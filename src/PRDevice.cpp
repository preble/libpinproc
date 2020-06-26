/*
 * The MIT License
 * Copyright (c) 2009 Gerry Stellenberg, Adam Preble
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 *  PRDevice.cpp
 *  libpinproc
 */

#include "PRDevice.h"
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdio.h>

PRDevice::PRDevice(PRMachineType machineType) : machineType(machineType)
{
    // Reset internally maintainted driver and switch structures, but do not update the device.
    Reset(kPRResetFlagDefault);
}

PRDevice::~PRDevice()
{
    Close();
}

PRDevice* PRDevice::Create(PRMachineType machineType)
{
    PRDevice *dev = new PRDevice(machineType);

    if (dev == NULL)
    {
        DEBUG(PRLog(kPRLogError, "Error allocating memory for P-ROC device\n"));
		PRSetLastErrorText("Error allocating memory for P-ROC device");
        return NULL;
    }

    if (!dev->Open())
    {
        DEBUG(PRLog(kPRLogError, "Error opening P-ROC device.\n"));
        delete dev;
        return NULL;
    }

    PRMachineType readMachineType = dev->GetReadMachineType();

    // Custom is always accepted
    if (machineType != kPRMachineCustom && machineType != kPRMachinePDB &&

	// Don't accept if requested type is WPC/WPC95 but read machine is not.
        ( (((machineType == kPRMachineWPC) ||
            (machineType == kPRMachineWPC95) ||
            (machineType == kPRMachineWPCAlphanumeric)) &&
           (readMachineType != kPRMachineWPC &&
            readMachineType != kPRMachineWPC95 &&
            readMachineType != kPRMachineWPCAlphanumeric)) ||
	  // Also don't accept if the requested is not WPC/WPC95 but the P-ROC is.
          (machineType != kPRMachineWPC &&
           machineType != kPRMachineWPC95 &&
           machineType != kPRMachineWPCAlphanumeric &&
           readMachineType == kPRMachineWPC) ) )
    {
        dev->Close();
        DEBUG(PRLog(kPRLogError, "Machine type 0x%x invalid for P-ROC board settings 0x%x.\n", machineType, readMachineType));
		PRSetLastErrorText("Machine type error.");
        return NULL;
    }

    return dev;
}

PRResult PRDevice::Reset(uint32_t resetFlags)
{
    int i;

    // Initialize buffer pointers
    collected_bytes_rd_addr = 0;
    collected_bytes_wr_addr = 0;
    num_collected_bytes = 0;

    // Make sure the data queues are empty.
    while (!unrequestedDataQueue.empty()) unrequestedDataQueue.pop();
    while (!requestedDataQueue.empty()) requestedDataQueue.pop();
    num_collected_bytes = 0;
    numPreparedWriteWords = 0;

    if (machineType != kPRMachineCustom && machineType != kPRMachinePDB) DriverLoadMachineTypeDefaults(machineType, resetFlags);

    // Disable dmd events if updating the device.
#if 0
    if (resetFlags & kPRResetFlagUpdateDevice)
    {
        PRDMDConfig *dmdConfig = &(this->dmdConfig);
        dmdConfig->enableFrameEvents = false;
        DMDUpdateConfig(dmdConfig);
    }
#endif

    // Make sure the free list is empty.
    while (!freeSwitchRuleIndexes.empty()) freeSwitchRuleIndexes.pop();

	memset(switchRules, 0x00, sizeof(PRSwitchRuleInternal) * maxSwitchRules);

    for (i = 0; i < kPRSwitchRulesCount; i++)
    {
        PRSwitchRuleInternal *switchRule = &switchRules[i];

        uint16_t ruleIndex = i;
        ParseSwitchRuleIndex(ruleIndex, &switchRule->switchNum, &switchRule->eventType);
        switchRule->driver.polarity = driverGlobalConfig.globalPolarity;

        // All of the base switch numbers in the P-ROC are used; so there are no
        // full sets of switch rule resources that are available for the freelist for linked rules.
        // However, some of the switches are always optos and don't need to be debounced.
        // So the debounced rule resources for those switches are available for linked rules.
        if (switchRule->switchNum >= kPRSwitchNeverDebounceFirst &&
            (switchRule->eventType == kPREventTypeSwitchClosedDebounced ||
             switchRule->eventType == kPREventTypeSwitchOpenDebounced))
            freeSwitchRuleIndexes.push(ruleIndex);
    }

    // Create empty switch rule for clearing the rules in the device.
    PRSwitchRule emptySwitchRule;
    memset(&emptySwitchRule, 0x00, sizeof(PRSwitchRule));

    for (i = 0; i < kPRSwitchCount; i++)
    {
        // Send blank rule for each event type to Device if necessary
        if (resetFlags & kPRResetFlagUpdateDevice)
        {
            SwitchUpdateRule(i, kPREventTypeSwitchOpenDebounced, &emptySwitchRule, NULL, 0, false);
            SwitchUpdateRule(i, kPREventTypeSwitchClosedDebounced, &emptySwitchRule, NULL, 0, false);
            SwitchUpdateRule(i, kPREventTypeSwitchOpenNondebounced, &emptySwitchRule, NULL, 0, false);
            SwitchUpdateRule(i, kPREventTypeSwitchClosedNondebounced, &emptySwitchRule, NULL, 0, false);
        }
    }

    return kPRSuccess;
}

int PRDevice::GetEvents(PREvent *events, int maxEvents)
{
    if (SortReturningData() != kPRSuccess)
    {
        PRSetLastErrorText("GetEvents ERROR: Error in CollectReadData");
	    return -1;
    }

    // The unrequestedDataQueue only has unrequested switch event data.  Pop
    // events out 1 at a time, interpret them, and populate the outgoing list with them.
    int i;
    for (i = 0; (i < maxEvents) && !unrequestedDataQueue.empty(); i++)
    {
        uint32_t event_data = unrequestedDataQueue.front();
        unrequestedDataQueue.pop();

        int type;
        bool open, debounced;

        if (version >= 2) {
            events[i].value = event_data & P_ROC_V2_EVENT_SWITCH_NUM_MASK;
            type = (event_data & P_ROC_V2_EVENT_TYPE_MASK) >> P_ROC_V2_EVENT_TYPE_SHIFT;
            open = (event_data & P_ROC_V2_EVENT_SWITCH_STATE_MASK) >> P_ROC_V2_EVENT_SWITCH_STATE_SHIFT;
            debounced = (event_data & P_ROC_V2_EVENT_SWITCH_DEBOUNCED_MASK) >> P_ROC_V2_EVENT_SWITCH_DEBOUNCED_SHIFT;
            events[i].time = (event_data & P_ROC_V2_EVENT_SWITCH_TIMESTAMP_MASK) >> P_ROC_V2_EVENT_SWITCH_TIMESTAMP_SHIFT;
        }
        else {
            type = (event_data & P_ROC_V1_EVENT_TYPE_MASK) >> P_ROC_V1_EVENT_TYPE_SHIFT;
            events[i].value = event_data & P_ROC_V1_EVENT_SWITCH_NUM_MASK;
            open = (event_data & P_ROC_V1_EVENT_SWITCH_STATE_MASK) >> P_ROC_V1_EVENT_SWITCH_STATE_SHIFT;
            debounced = (event_data & P_ROC_V1_EVENT_SWITCH_DEBOUNCED_MASK) >> P_ROC_V1_EVENT_SWITCH_DEBOUNCED_SHIFT;
            events[i].time = (event_data & P_ROC_V1_EVENT_SWITCH_TIMESTAMP_MASK) >> P_ROC_V1_EVENT_SWITCH_TIMESTAMP_SHIFT;
        }


        //fprintf(stderr, "\nLibpinproc: event type: %d", type);
        switch (type)
        {
            case P_ROC_EVENT_TYPE_SWITCH:
            {
                if (open)
                    events[i].type = debounced ? kPREventTypeSwitchOpenDebounced : kPREventTypeSwitchOpenNondebounced;
                else
                    events[i].type = debounced ? kPREventTypeSwitchClosedDebounced : kPREventTypeSwitchClosedNondebounced;
                break;
            }

            case P_ROC_EVENT_TYPE_DMD:
            {
                events[i].type = kPREventTypeDMDFrameDisplayed;
                break;
            }

            case P_ROC_EVENT_TYPE_BURST_SWITCH:
            {
                //fprintf(stderr, "\nBurst event");
                if (open) events[i].type = kPREventTypeBurstSwitchOpen;
                else events[i].type = kPREventTypeBurstSwitchClosed;
                break;
            }

            case P_ROC_EVENT_TYPE_ACCELEROMETER:
            {
                events[i].time = events[i].time >> 2;
                events[i].value = event_data & 0x00003FFF;
                int accel_type = (event_data & 0x00030000) >> 16;
                switch (accel_type)
                {
                    case 0:
                    {
                        events[i].type = kPREventTypeAccelerometerX;
                        break;
                    }
                    case 1:
                    {
                        events[i].type = kPREventTypeAccelerometerY;
                        break;
                    }
                    case 2:
                    {
                        events[i].type = kPREventTypeAccelerometerZ;
                        break;
                    }
                    case 3:
                    {
                        events[i].type = kPREventTypeAccelerometerIRQ;
                        break;
                    }
                    default: events[i].type = kPREventTypeInvalid;
                }
                break;
            }

            default: events[i].type = kPREventTypeInvalid;

        }
    }
    return i;
}

PRResult PRDevice::ManagerUpdateConfig(PRManagerConfig *managerConfig)
{
    const int burstWords = 2;
    uint32_t burst[burstWords];
    DEBUG(PRLog(kPRLogInfo, "Setting Manager Config Register\n"));
    this->managerConfig = *managerConfig;
    CreateManagerUpdateConfigBurst(burst, managerConfig);
    return PrepareWriteData(burst, burstWords);
}

PRResult PRDevice::DriverUpdateGlobalConfig(PRDriverGlobalConfig *driverGlobalConfig)
{
    const int burstWords = 4;
    uint32_t burst[burstWords];

    DEBUG(PRLog(kPRLogInfo, "Installing driver globals\n"));

    this->driverGlobalConfig = *driverGlobalConfig;
    CreateDriverUpdateGlobalConfigBurst(burst, driverGlobalConfig);
    CreateWatchdogConfigBurst(burst+2, driverGlobalConfig->watchdogExpired,
                                       driverGlobalConfig->watchdogEnable,
                                       driverGlobalConfig->watchdogResetTime);

    DEBUG(PRLog(kPRLogVerbose, "Driver Global words: %x %x\n", burst[0], burst[1]));
    DEBUG(PRLog(kPRLogVerbose, "Watchdog words: %x %x\n", burst[2], burst[3]));
    return PrepareWriteData(burst, burstWords);
}

PRResult PRDevice::DriverGetGroupConfig(uint8_t groupNum, PRDriverGroupConfig *driverGroupConfig)
{
    *driverGroupConfig = driverGroups[groupNum];
    return kPRSuccess;
}

PRResult PRDevice::DriverUpdateGroupConfig(PRDriverGroupConfig *driverGroupConfig)
{
    const int burstWords = 2;
    uint32_t burst[burstWords];

    driverGroups[driverGroupConfig->groupNum] = *driverGroupConfig;
    DEBUG(PRLog(kPRLogInfo, "Installing driver group\n"));
    CreateDriverUpdateGroupConfigBurst(burst, driverGroupConfig);

    DEBUG(PRLog(kPRLogVerbose, "Words: %x %x\n", burst[0], burst[1]));
    return PrepareWriteData(burst, burstWords);
}

PRResult PRDevice::DriverGetState(uint8_t driverNum, PRDriverState *driverState)
{
    *driverState = drivers[driverNum];
    return kPRSuccess;
}

PRResult PRDevice::DriverUpdateState(PRDriverState *driverState)
{
    const int burstWords = 3;
    uint32_t burst[burstWords];

    // Don't allow Constant Pulse (non-schedule with time = 0) for known high current drivers.
    // Note, the driver numbers depend on the driver group settings from DriverLoadMachineTypeDefaults.
    // TODO: Create some constants that are used both here and in DriverLoadMachineTypeDefaults.

    DEBUG(PRLog(kPRLogInfo, "Updating driver #%d\n", driverState->driverNum));

    if (driverState->polarity != drivers[driverState->driverNum].polarity && machineType != kPRMachineCustom && machineType != kPRMachinePDB)
    {
        PRSetLastErrorText("Refusing to update driver #%d; polarity differs on non-custom machine.", driverState->driverNum);
        return kPRFailure;
    }

    drivers[driverState->driverNum] = *driverState;

    CreateDriverUpdateBurst(burst, &drivers[driverState->driverNum]);
    DEBUG(PRLog(kPRLogVerbose, "Words: %x %x %x\n", burst[0], burst[1], burst[2]));

    return PrepareWriteData(burst, burstWords);
}

PRResult PRDevice::DriverLoadMachineTypeDefaults(PRMachineType machineType, uint32_t resetFlags)
{
    int i;
    PRResult res = kPRSuccess;

    //const int WPCDriverLoopTime = 4; // milliseconds
    //const int SternDriverLoopTime = 2; // milliseconds

    const int mappedWPCDriverGroupEnableIndex[] = {0, 0, 0, 0, 0, 2, 4, 3, 1, 5, 7, 7, 7, 7, 7, 7, 7, 7, 8, 0, 0, 0, 0, 0, 0, 0};
    const int mappedSternDriverGroupEnableIndex[] = {0, 0, 0, 0, 1, 0, 2, 3, 0, 0, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9};
    const bool mappedWPCDriverGroupPolarity[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
    const bool mappedSternDriverGroupPolarity[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    const int lastWPCCoilDriverGroup = 9;
    const int lastSternCoilDriverGroup = 7;
    const int mappedWPCDriverGroupSlowTime[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 400, 400, 400, 400, 400, 400, 400, 400, 0, 0, 0, 0, 0, 0, 0, 0};
    const int mappedSternDriverGroupSlowTime[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400};
    const int mappedWPCDriverGroupActivateIndex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0};
    const int mappedSternDriverGroupActivateIndex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7};

    const int watchdogResetTime = 1000; // milliseconds

    int mappedDriverGroupEnableIndex[kPRDriverGroupsMax];
    bool mappedDriverGroupPolarity[kPRDriverGroupsMax];
    int mappedDriverGroupSlowTime[kPRDriverGroupsMax];
    int mappedDriverGroupActivateIndex[kPRDriverGroupsMax];
    int rowEnableIndex1;
    int rowEnableIndex0;
    bool tickleSternWatchdog;
    bool globalPolarity;
    bool activeLowMatrixRows;
    int driverLoopTime;
    //int slowGroupTime;
    int numMatrixGroups;
    bool encodeEnables;
    int rowEnableSelect;
    int lastCoilDriverGroup;


    switch (machineType)
    {
        case kPRMachineWPC:
        case kPRMachineWPC95:
        case kPRMachineWPCAlphanumeric:
        {
            memcpy(mappedDriverGroupEnableIndex,mappedWPCDriverGroupEnableIndex,
                   sizeof(mappedDriverGroupEnableIndex));
            memcpy(mappedDriverGroupPolarity,mappedWPCDriverGroupPolarity,
                   sizeof(mappedDriverGroupPolarity));
            rowEnableIndex1 = 6; // Unused in WPC
            rowEnableIndex0 = 6;
            tickleSternWatchdog = false;
            globalPolarity = false;
            activeLowMatrixRows = true;
            driverLoopTime = 4; // milliseconds
            memcpy(mappedDriverGroupSlowTime,mappedWPCDriverGroupSlowTime,
                   sizeof(mappedDriverGroupSlowTime));
            memcpy(mappedDriverGroupActivateIndex,mappedWPCDriverGroupActivateIndex,
                   sizeof(mappedDriverGroupActivateIndex));
            numMatrixGroups = 8;
            encodeEnables = false;
            rowEnableSelect = 0;
            lastCoilDriverGroup = lastWPCCoilDriverGroup;
            break;
        }

        case kPRMachineSternWhitestar:
        case kPRMachineSternSAM:
        {
            memcpy(mappedDriverGroupEnableIndex,mappedSternDriverGroupEnableIndex,
                   sizeof(mappedDriverGroupEnableIndex));
            memcpy(mappedDriverGroupPolarity,mappedSternDriverGroupPolarity,
                   sizeof(mappedDriverGroupPolarity));
            rowEnableIndex1 = 6; // Unused in Stern
            rowEnableIndex0 = 10;
            tickleSternWatchdog = true;
            globalPolarity = true;
            activeLowMatrixRows = false;
            driverLoopTime = 1; // milliseconds
            memcpy(mappedDriverGroupSlowTime,mappedSternDriverGroupSlowTime,
                   sizeof(mappedDriverGroupSlowTime));
            memcpy(mappedDriverGroupActivateIndex,mappedSternDriverGroupActivateIndex,
                   sizeof(mappedDriverGroupActivateIndex));
            numMatrixGroups = 16;
            encodeEnables = true;
            rowEnableSelect = 0;
            lastCoilDriverGroup = lastSternCoilDriverGroup;
            break;
        }

        default:
            // Don't do anything for non-specific machine types.  Enabling
            // drivers and/or groups could be dangerous, especially if polarities
            // are wrong.
            return kPRSuccess;

    }

    memset(&driverGlobalConfig, 0x00, sizeof(PRDriverGlobalConfig));
    for (i = 0; i < kPRDriverCount; i++)
    {
        PRDriverState *driver = &drivers[i];
        memset(driver, 0x00, sizeof(PRDriverState));
        driver->driverNum = i;
        driver->polarity = mappedDriverGroupPolarity[i/8];
        DEBUG(PRLog(kPRLogInfo,"\nDriver Polarity for Driver: %d is %x.", i,driver->polarity));
        if (resetFlags & kPRResetFlagUpdateDevice)
            res = DriverUpdateState(driver);
    }
    for (i = 0; i < kPRDriverGroupsMax; i++)
    {
        PRDriverGroupConfig *group = &driverGroups[i];
        memset(group, 0x00, sizeof(PRDriverGroupConfig));
        group->groupNum = i;
        group->polarity = mappedDriverGroupPolarity[i];
    }


    // Configure the groups.  Each group corresponds to 8 consecutive drivers, starting
    // with driver #32.  The following 6 groups are configured for coils/flashlamps.

    PRDriverGroupConfig group;
    for (i = 4; i <= lastCoilDriverGroup; i++)
    {
        DriverGetGroupConfig(i, &group);
        group.slowTime = 0;
        group.enableIndex = mappedDriverGroupEnableIndex[i];
        group.rowActivateIndex = 0;
        group.rowEnableSelect = 0;
        group.matrixed = false;
        group.polarity = mappedDriverGroupPolarity[i];
        group.active = 1;
        group.disableStrobeAfter = false;

        if (resetFlags & kPRResetFlagUpdateDevice) {
            res = DriverUpdateGroupConfig(&group);
            DEBUG(PRLog(kPRLogInfo,"\nDriver Polarity for Group: %d is %x.", i,group.polarity));
        }
        else
            driverGroups[i] = group;
    }


    // The following 8 groups are configured for the feature lamp matrix.
    for (i = 10; i < 10 + numMatrixGroups; i++) {
        DriverGetGroupConfig(i, &group);
        group.slowTime = mappedDriverGroupSlowTime[i];
        group.enableIndex = mappedDriverGroupEnableIndex[i];
        group.rowActivateIndex = mappedDriverGroupActivateIndex[i];
        group.rowEnableSelect = rowEnableSelect;
        group.matrixed = 1;
        group.polarity = mappedDriverGroupPolarity[i];
        group.active = 1;
        group.disableStrobeAfter = mappedDriverGroupSlowTime[i] != 0;

        if (resetFlags & kPRResetFlagUpdateDevice) {
            res = DriverUpdateGroupConfig(&group);
            DEBUG(PRLog(kPRLogInfo,"\nDriver Polarity for Group: %d is %x.", i,group.polarity));
        }
        else
            driverGroups[i] = group;
    }

    // Special case for WPC machines.  Enable group 18 for the 8-driver board.
    if (machineType == kPRMachineWPC) {
        i = 18;
        DriverGetGroupConfig(i, &group);
        group.slowTime = 0;
        group.enableIndex = mappedDriverGroupEnableIndex[i];
        group.rowActivateIndex = 0;
        group.rowEnableSelect = 0;
        group.matrixed = false;
        group.polarity = mappedDriverGroupPolarity[i];
        group.active = 1;
        group.disableStrobeAfter = false;

        if (resetFlags & kPRResetFlagUpdateDevice) {
            res = DriverUpdateGroupConfig(&group);
            DEBUG(PRLog(kPRLogInfo,"\nDriver Polarity for Group: %d is %x.\n", i,group.polarity));
        }
        else
            driverGroups[i] = group;
    }

    PRDriverGlobalConfig globals;
    globals.enableOutputs = false;
    globals.globalPolarity = globalPolarity;
    globals.useClear = false;
    globals.strobeStartSelect = false;
    globals.startStrobeTime = driverLoopTime; // milliseconds per output loop
    globals.matrixRowEnableIndex1 = rowEnableIndex1;
    globals.matrixRowEnableIndex0 = rowEnableIndex0;
    globals.activeLowMatrixRows = activeLowMatrixRows;
    globals.tickleSternWatchdog = tickleSternWatchdog;
    globals.encodeEnables = encodeEnables;
    globals.watchdogExpired = false;
    globals.watchdogEnable = true;
    globals.watchdogResetTime = watchdogResetTime;

    // We want to start up safely, so we'll update the global driver config twice.
    // When we toggle enableOutputs like this P-ROC will reset the polarity:

    // Enable now without the outputs enabled:
    if (resetFlags & kPRResetFlagUpdateDevice)
        res = DriverUpdateGlobalConfig(&globals);
    else
        driverGlobalConfig = globals;

    // Now enable the outputs to protect against the polarity being driven incorrectly:
    globals.enableOutputs = true;
    if (resetFlags & kPRResetFlagUpdateDevice)
        res = DriverUpdateGlobalConfig(&globals);
    else
        driverGlobalConfig = globals;

    // If WPCAlphanumeric, select Aux functionality for the dual-purpose Aux/DMD
    // pins.

    managerConfig.reuse_dmd_data_for_aux = (machineType == kPRMachineWPCAlphanumeric);
    managerConfig.invert_dipswitch_1 = false;
    ManagerUpdateConfig(&managerConfig);

    return res;
}

PRResult PRDevice::DriverAuxSendCommands(PRDriverAuxCommand * commands, uint8_t numCommands, uint8_t startingAddr)
{
    int32_t k;
    uint32_t commandBuffer[513];
    uint32_t convertedCommand;
    uint32_t addr;

    if (chip_id == P_ROC_CHIP_ID)
    {
        addr = (P_ROC_DRIVER_AUX_MEM_DECODE << P_ROC_DRIVER_CTRL_DECODE_SHIFT) | startingAddr;
        commandBuffer[0] = CreateBurstCommand(P_ROC_BUS_DRIVER_CTRL_SELECT, addr, numCommands);
    }
    else // chip == P3_ROC_CHIP_ID)
    {
        addr = 0;
        commandBuffer[0] = CreateBurstCommand(P3_ROC_BUS_AUX_CTRL_SELECT, addr, numCommands);
    }

    for (k=0; k<numCommands; k++) {
        convertedCommand = CreateDriverAuxCommand(commands[k]);
        commandBuffer[k+1] = convertedCommand;
    }

    return PrepareWriteData(commandBuffer, numCommands+1);

}

PRResult PRDevice::DriverWatchdogTickle()
{
    const int burstWords = 2;
    uint32_t burst[burstWords];

    CreateWatchdogConfigBurst(burst, driverGlobalConfig.watchdogExpired,
                              driverGlobalConfig.watchdogEnable,
                              driverGlobalConfig.watchdogResetTime);

    return PrepareWriteData(burst, burstWords);
}



PRSwitchRuleInternal *PRDevice::GetSwitchRuleByIndex(uint16_t index)
{
    return &switchRules[index];
}

PRResult PRDevice::SwitchUpdateConfig(PRSwitchConfig *switchConfig)
{
    uint32_t rc;
    const int burstWords = 4;
    uint32_t burst[burstWords];

    this->switchConfig = *switchConfig;
    CreateSwitchUpdateConfigBurst(burst, switchConfig);

    DEBUG(PRLog(kPRLogInfo, "Configuring Switch Logic\n"));
    DEBUG(PRLog(kPRLogVerbose, "Words: %x %x\n",burst[0],burst[1]));

    rc = PrepareWriteData(burst, burstWords);
    return rc;
}

PRResult PRDevice::SwitchUpdateRule(uint8_t switchNum, PREventType eventType, PRSwitchRule *rule, PRDriverState *linkedDrivers, int numDrivers, bool_t drive_outputs_now )
{
    // Updates a single rule with the associated linked driver state changes.
    const int burstSize = 4;
    uint32_t burst[burstSize];

    // If more the base rule will link to others, ensure free indexes exists for
    // the links.
    if (numDrivers > 0 && freeSwitchRuleIndexes.size() < (uint32_t)(numDrivers-1)) // -1 because the first switch rule holds the first driver.
    {
        PRSetLastErrorText("Not enough free switch rule indexes: %d available, need %d", freeSwitchRuleIndexes.size(), numDrivers);
        return kPRFailure;
    }

    PRResult res = kPRSuccess;
    uint32_t newRuleIndex = CreateSwitchRuleIndex(switchNum, eventType);

    // Because we're redefining the rule chain, we need to remove all previously existing links and return the indexes to the free list.
    PRSwitchRuleInternal *oldRule = GetSwitchRuleByIndex(newRuleIndex);

    uint16_t oldLinkIndex;
    while (oldRule->linkActive)
    {
	// Save old link index so it can freed after the linked rule is retrieved.
	oldLinkIndex = oldRule->linkIndex;
        oldRule = GetSwitchRuleByIndex(oldRule->linkIndex);
        freeSwitchRuleIndexes.push(oldLinkIndex);

        if (freeSwitchRuleIndexes.size() > 128) // Detect a corrupted link-related values before it eats up all of the memory.
        {
			PRSetLastErrorText("Too many free switch rule indicies!");
            return kPRFailure;
        }
    }

    // Create a pointer for new rules.
    PRSwitchRuleInternal *newRule;

    // Process each driver who's state should change in response to the switch event.
    if (numDrivers > 0)
    {
        uint32_t ruleIndex, savedRuleIndex;

        // Need to program the main rule last just in case drive_outputs_now is true.
        // Otherwise, the hardware could try to access the linked rules before they're
        // programmed.  So, program the rules in reverse order.

        // Move to last driver
        linkedDrivers += (numDrivers - 1);
        int totalNumDrivers = numDrivers;

        while (numDrivers > 0)
        {
            if (numDrivers > 1)
            {
                ruleIndex = freeSwitchRuleIndexes.front();
                freeSwitchRuleIndexes.pop();
                newRule = GetSwitchRuleByIndex(ruleIndex);
                newRule->driver = linkedDrivers[0];
                newRule->changeOutput = true;

                if (totalNumDrivers == numDrivers) newRule->linkActive = false;
                else
                {
                    newRule->linkActive = true;
                    newRule->linkIndex = savedRuleIndex;
                }

                savedRuleIndex = ruleIndex;
                // Set the 3rd param (drive_outputs_now) to false to keep the hardware
                // from evaluating the state of the rule index and possibly activating the
                // driver.  The evaluation will happen later when the primary rule is
                // written.
                CreateSwitchUpdateRulesBurst(burst, newRule, false);
            }
            else
            {
                // This is the primary rule.
                newRule = GetSwitchRuleByIndex(newRuleIndex);
                if (newRule->eventType != eventType) {
                    DEBUG(PRLog(kPRLogWarning, "Unexpected state: switch rule at 0x%x has event type 0x%x (expected 0x%x).\n", newRuleIndex, newRule->eventType, eventType));
                }
                newRule->notifyHost = rule->notifyHost;
                newRule->reloadActive = rule->reloadActive;
                newRule->changeOutput = true;
                newRule->driver = linkedDrivers[0];
                if (totalNumDrivers > 1)
                {
                    newRule->linkActive = true;
                    newRule->linkIndex = savedRuleIndex;
                }
                else newRule->linkActive = false;
                CreateSwitchUpdateRulesBurst(burst, newRule, drive_outputs_now);
            }

            DEBUG(PRLog(kPRLogVerbose, "Rule Words: %x %x %x %x\n", burst[0],burst[1],burst[2],burst[3]));
            // Write the rule:
            res = PrepareWriteData(burst, burstSize);
            if (res != kPRSuccess)
            {
                DEBUG(PRLog(kPRLogError, "Error while writing switch update, attempting to revert switch rule to a safe state..."));
                newRule = GetSwitchRuleByIndex(newRuleIndex);
                newRule->changeOutput = false;
                newRule->linkActive = false;
                CreateSwitchUpdateRulesBurst(burst, newRule, false);
                if (PrepareWriteData(burst, burstSize) == kPRSuccess)
                    DEBUG(PRLog(kPRLogError, "Disabled successfully.\n"));
                else
                    DEBUG(PRLog(kPRLogError, "Failed to disable.\n"));
                return res;
            }

            linkedDrivers--;
            numDrivers--;
        }
    }
    else {
        newRule = GetSwitchRuleByIndex(newRuleIndex);
        if (newRule->eventType != eventType) {
            DEBUG(PRLog(kPRLogWarning, "Unexpected state: switch rule at 0x%x has event type 0x%x (expected 0x%x).\n", newRuleIndex, newRule->eventType, eventType));
        }
        newRule->notifyHost = rule->notifyHost;
        newRule->reloadActive = rule->reloadActive;
        newRule->changeOutput = false;
        newRule->linkActive = false;

        CreateSwitchUpdateRulesBurst(burst, newRule, false);
        DEBUG(PRLog(kPRLogVerbose, "Rule Words: %x %x %x %x\n", burst[0],burst[1],burst[2],burst[3]));

        // Write the rule:
        res = PrepareWriteData(burst, burstSize);
    }

    return res;
}

PRResult PRDevice::SwitchGetStates( PREventType * switchStates, uint16_t numSwitches )
{
    uint32_t stateWord, debounceWord;
    uint8_t i, j;
    PREventType eventType;

    // Request one state word and one debounce word at a time.  Could make more efficient
    // use of the USB bus by requesting a burst of state words and then a burst of debounce
    // words, but doing one word at a time makes it easier to process each switch when the
    // data returns.  Also, this function shouldn't be called during timing sensitive
    // situations; so the inefficiencies are acceptable.
    for (i = 0; i < numSwitches / 32; i++)
    {

        if (chip_id == P_ROC_CHIP_ID)
        {
            RequestData(P_ROC_BUS_SWITCH_CTRL_SELECT,
                        P_ROC_SWITCH_CTRL_STATE_BASE_ADDR + i, 1);

            if (combinedVersionRevision < P_ROC_VER_REV_FIXED_SWITCH_STATE_READS) {
                RequestData(P_ROC_BUS_SWITCH_CTRL_SELECT,
                            P_ROC_SWITCH_CTRL_OLD_DEBOUNCE_BASE_ADDR + i, 1);
            }
            else {
                RequestData(P_ROC_BUS_SWITCH_CTRL_SELECT,
                            P_ROC_SWITCH_CTRL_DEBOUNCE_BASE_ADDR + i, 1);
            }
        }
        else // chip == P3_ROC_CHIP_ID)
        {
            RequestData(P_ROC_BUS_SWITCH_CTRL_SELECT,
                        P3_ROC_SWITCH_CTRL_STATE_BASE_ADDR + i, 1);

            RequestData(P_ROC_BUS_SWITCH_CTRL_SELECT,
                        P3_ROC_SWITCH_CTRL_DEBOUNCE_BASE_ADDR + i, 1);
        }
    }

    // Expect 4 words for each 32 switches.  The state and debounce words,
    // and the address words for both.
    uint16_t numWords = 4 * (numSwitches / 32);

    i = 0; // Reset i so it can be used to prevent an infinite loop below

    // Wait for data to return.  Give it 10 loops before giving up.
    while (requestedDataQueue.size() < numWords && i++ < 10)
    {
        PRSleep (10); // 10 milliseconds should be plenty of time.
		if (SortReturningData() != kPRSuccess)
		{
			return kPRFailure;
        }
    }

    // Make sure all of the requested words are available before processing them.
    // Too many words is just as bad as not enough words.
    // If too many come back, can they be trusted?
    if (requestedDataQueue.size() == numWords)
    {
        // Process the returning words.
        for (i = 0; i < numSwitches / 32; i++)
        {
            requestedDataQueue.pop(); // Ignore address word.  TODO: Verify this address word.
            stateWord = requestedDataQueue.front(); // This is the switch state word.
            requestedDataQueue.pop();
            requestedDataQueue.pop(); // Ignore address word.  TODO: Verify this address word.
            debounceWord = requestedDataQueue.front(); // This is the debounce word.
            requestedDataQueue.pop();

            // Loop through each bit of the words, combining them into an eventType
            for (j = 0; j < 32; j++)
            {
                // Only process the number of switches requested via numSwitches
                if ((i * 32) + j < numSwitches)
                {
                    if (stateWord >> j & 1)
                        if (debounceWord >> j & 1) eventType = kPREventTypeSwitchOpenDebounced;
                        else eventType = kPREventTypeSwitchOpenNondebounced;
                    else if (debounceWord >> j & 1) eventType = kPREventTypeSwitchClosedDebounced;
                    else eventType = kPREventTypeSwitchClosedNondebounced;
                    switchStates[(i * 32) + j] = eventType;
                }
            }
        }
        return kPRSuccess;
    }
    else
    {
        PRSetLastErrorText("Switch response length does not match.");
        return kPRFailure;
    }
}

int32_t PRDevice::DMDUpdateConfig(PRDMDConfig *dmdConfig)
{
    uint32_t rc;
    const int burstWords = 7;
    uint32_t burst[burstWords];

    this->dmdConfig = *dmdConfig;
    CreateDMDUpdateConfigBurst(burst, dmdConfig);

    DEBUG(PRLog(kPRLogInfo, "Configuring DMD\n"));
    DEBUG(PRLog(kPRLogVerbose, "Words: %x %x %x %x %x %x %x\n",burst[0],burst[1],burst[2],burst[3],
                burst[4],burst[5],burst[6]));

    rc = PrepareWriteData(burst, burstWords);
    return rc;
}


PRResult PRDevice::DMDDraw(uint8_t * dots)
{
    int32_t k; //i,x,y,j,k,m;
    //uint8_t color;
    uint16_t words_per_sub_frame = (dmdConfig.numColumns*dmdConfig.numRows) / 32;
    uint16_t words_per_frame = words_per_sub_frame * dmdConfig.numSubFrames;
    uint32_t dmd_command_buffer[1024];
    uint32_t * p_dmd_frame_buffer_words;

    p_dmd_frame_buffer_words = (uint32_t *)dots;

    dmd_command_buffer[0] = CreateBurstCommand(P_ROC_BUS_DMD_SELECT, P_ROC_DMD_DOT_TABLE_BASE_ADDR, words_per_frame);
    for (k=0; k<words_per_frame; k++) {
        dmd_command_buffer[k+1] = p_dmd_frame_buffer_words[k];
    }

    return PrepareWriteData(dmd_command_buffer, words_per_frame+1);

    // The following code prints out the init lines for the 4 Xilinx BlockRAMs
    // in the FPGA.  It's used to make an image for the P-ROC to display on power-up.
#if 0
    // This is the original version... needs to be deleted.
    for (i=0; i<4; i++) {
      std::cout << "For memory: "<< i << "\n";
      // Need 4 lines to get 1 frame (4*256*4 = 4096)
      // The rest will be all 0.
      for (y=0; y<4; y++) {
        std::cout << "defparam blockram.INIT_00 = 256'b";
        for (j=31; j>=0; j--) {
          for (x=7; x>=0; x--) {
            std::cout << ((dmd_frame_buffer[(y*32)+j] >> ((i*8)+x)) & 1);
          }
        }
        std::cout << ";\n";
      }
      std::cout << "\n\n\n";
    }
#endif
#if 0
    for (i=0; i<4; i++) {
      std::cout << "For memory: "<< i << "\n";
      // Need 4 lines to get 1 frame (4*256*4 = 4096)
      // The rest will be all 0.
      for (y=0; y<4; y++) {
        std::cout << "defparam blockram.INIT_00 = 256'b";
        for (j=8; j>=0; j--) {
          for (x=31; x>=0; x--) {
            std::cout << ((dmd_frame_buffer[(y*32)+j] >> ((i*8)+x)) & 1);
          }
        }
        std::cout << ";\n";
      }
      std::cout << "\n\n\n";
    }
#endif
}

PRResult PRDevice::PRJTAGDriveOutputs(PRJTAGOutputs * jtagOutputs, bool_t toggleClk)
{
    const int burstSize = 2;
    uint32_t burst[burstSize];

    if (toggleClk) CreateJTAGLatchOutputsBurst( burst, jtagOutputs );
    else CreateJTAGForceOutputsBurst( burst, jtagOutputs );
    return WriteData(burst, burstSize);
}

PRResult PRDevice::PRJTAGWriteTDOMemory(uint16_t tableOffset, uint16_t numWords, uint32_t * tdoData)
{
    int32_t i;
    const int maxBurstSize = 513;
    uint32_t burst[maxBurstSize];

    burst[0] = CreateBurstCommand(P_ROC_BUS_JTAG_SELECT, P_ROC_JTAG_TDO_MEMORY_BASE_ADDR + tableOffset, numWords);
    for (i=0; i<numWords; i++) {
        burst[i+1] = tdoData[i];
    }

    return WriteData(burst, numWords + 1);
}

PRResult PRDevice::PRJTAGShiftTDOData(uint16_t numBits, bool_t dataBlockComplete)
{
    const int burstSize = 2;
    uint32_t burst[burstSize];

    CreateJTAGShiftTDODataBurst( burst, numBits, dataBlockComplete );
    return WriteData(burst, burstSize);
}

PRResult PRDevice::PRJTAGReadTDIMemory(uint16_t tableOffset, uint16_t numWords, uint32_t * tdiData)
{
    return ReadDataRaw (P_ROC_BUS_JTAG_SELECT, P_ROC_JTAG_TDI_MEMORY_BASE_ADDR + tableOffset, numWords, tdiData);
}

PRResult PRDevice::PRJTAGGetStatus(PRJTAGStatus * status)
{
    uint32_t rdBuffer[1];
	PRResult res = ReadDataRaw(P_ROC_BUS_JTAG_SELECT, P_ROC_JTAG_STATUS_REG_BASE_ADDR, 1, rdBuffer);
	if (res == kPRFailure)
		return res;
    status->commandComplete = rdBuffer[0] >> P_ROC_JTAG_STATUS_DONE_SHIFT;
    status->tdi = rdBuffer[0] >> P_ROC_JTAG_STATUS_TDI_SHIFT;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Device I/O

PRResult PRDevice::Open()
{
    uint32_t temp_word;
    PRResult res = PRHardwareOpen();
    if (res == kPRSuccess)
    {
        // Try to verify the P-ROC IS in the FPGA before initializing the FPGA's FTDI interface
        // just in case it was already initialized from a previous application execution.
        DEBUG(PRLog(kPRLogInfo, "Verifying P-ROC ID: \n"));

        // Attempt to turn off events.  This is necessary if P-ROC wasn't shut down
        // properly previously.  If the P-ROC isn't initialized, this request will
        // be ignored.

        PRDMDConfig dmdConfig;
        dmdConfig.numRows = 32; // Doesn't matter.
        dmdConfig.numColumns = 128; // Doesn't matter
        dmdConfig.numSubFrames = 4; // Doesn't matter
        dmdConfig.numFrameBuffers = 3; // Doesn't matter
        dmdConfig.autoIncBufferWrPtr = false;
        dmdConfig.enableFrameEvents = false;
        DMDUpdateConfig(&dmdConfig);

        PRSwitchConfig switchConfig;
        switchConfig.clear = false;
        switchConfig.use_column_9 = false;
        switchConfig.use_column_8 = false;
        switchConfig.hostEventsEnable = false;
        switchConfig.directMatrixScanLoopTime = 2; // milliseconds
        switchConfig.pulsesBeforeCheckingRX = 10;
        switchConfig.inactivePulsesAfterBurst = 12;
        switchConfig.pulsesPerBurst = 6;
        switchConfig.pulseHalfPeriodTime = 13; // milliseconds
        SwitchUpdateConfig(&switchConfig);

        // Flush read data to ensure VerifyChipID starts with clean buffer.
        // It's possible the P-ROC has a lot of data stored up in internal buffers.  So if
        // the verify still fails, do a bunch of flushes.
        res = FlushReadBuffer();
        uint32_t verify_ctr = 0;
        res = VerifyChipID();
        while (res == kPRFailure && verify_ctr++ < 5) {
            // Only send init pattern once.
            if (verify_ctr == 1) {
                temp_word = P_ROC_INIT_PATTERN_A;
                res = WriteData(&temp_word, 1);
                temp_word = P_ROC_INIT_PATTERN_B;
                res = WriteData(&temp_word, 1);
            }
            // Since the FPGA didn't appear to be responding properly, send the FPGA's FTDI
            // initialization sequence.  This is a set of bytes the FPGA is waiting to receive
            // before it allows access deeper into the chip.  This keeps garbage from getting
            // in and wreaking havoc before software is up and running.
            DEBUG(PRLog(kPRLogError, "Verification of chip ID failed.  Flushing read buffer and re-verifying chip ID.\n"));
            DEBUG(PRLog(kPRLogInfo, "Initializing P-ROC...\n"));
            res = FlushReadBuffer();
            PRSleep(100);
            res = VerifyChipID();
            if (res == kPRFailure) {
                DEBUG(PRLog(kPRLogWarning, "Unable to read Chip ID - P-ROC could not be initialized.\n"));
            }
        }
    }

    return res;
}

PRResult PRDevice::Close()
{
    // TODO: Add protection against closing a not-open ftdic.
    PRHardwareClose();
    return kPRSuccess;
}

PRMachineType PRDevice::GetReadMachineType()
{
    return readMachineType;
}

PRResult PRDevice::VerifyChipID()
{
    PRResult rc;
    const int bufferWords = 5;
    uint32_t buffer[bufferWords] = {0};
    //uint32_t temp_word;
    uint32_t max_count, i;
    const uint32_t max_count_limit = 10;

    //std::cout << "Requesting FPGA Chip ID: ";
    rc = RequestData(P_ROC_MANAGER_SELECT, P_ROC_REG_CHIP_ID_ADDR, 4);

    max_count = 0;
    // Wait for data to return.  Give it 10 loops before giving up.
    while (requestedDataQueue.size() < 5 && max_count++ < max_count_limit)
    {
        PRSleep (10); // 10 milliseconds should be plenty of time.
		if (SortReturningData() != kPRSuccess)
			return kPRFailure;
    }

    if (max_count != max_count_limit+1) {

        if (requestedDataQueue.size() == 5) {
            for (i = 0; i < bufferWords; i++) {
                buffer[i] = requestedDataQueue.front();
                requestedDataQueue.pop(); // Ignore address word.  TODO: Verify the address.
            }
            if (buffer[1] != P_ROC_CHIP_ID && buffer[1] != P3_ROC_CHIP_ID)
            {
                DEBUG(PRLog(kPRLogError, "Error in VerifyID(): Dumping buffer\n"));
                for (i = 0; i < bufferWords; i++)
                    DEBUG(PRLog(kPRLogError, "buffer[%d]: 0x%x\n", i, buffer[i]));
                PRSetLastErrorText("Chip ID does not match.");
                rc = kPRFailure;
            }
            else rc = kPRSuccess;
            //std::cout << rc << " words read.  \n"
            DEBUG(PRLog(kPRLogError, "FPGA Chip ID: 0x%x\n", buffer[1]));
            chip_id = buffer[1];
            revision = buffer[2] & 0xffff;
            version = buffer[2] >> 16;
            CalcCombinedVerRevision();
            DEBUG(PRLog(kPRLogError, "FPGA Chip Version/Rev: %d.%d\n", version, revision));
            DEBUG(PRLog(kPRLogInfo, "Watchdog Settings: 0x%x\n", buffer[3]));
            DEBUG(PRLog(kPRLogInfo, "Switches: 0x%x\n", buffer[4]));

            if (IsStern(buffer[4])) readMachineType = kPRMachineSternWhitestar; // Choose SAM or Whitestar, doesn't matter.
            else readMachineType = kPRMachineWPC; // Choose WPC or WPC95, doesn't matter.
        }
        else {
            DEBUG(PRLog(kPRLogError, "Error reading Chip IP and Version. Read %d words instead of 5. The first 2 were: 0x%x and 0x%x.\n", requestedDataQueue.size(), buffer[0], buffer[1]));
            PRSetLastErrorText("Error reading Chip IP and Version. Read %d words instead of 5. The first 2 were: 0x%x and 0x%x.", requestedDataQueue.size(), buffer[0], buffer[1]);
            rc = kPRFailure;
        }
    }
    else
    {
        // Return failure without logging; calling function must log.
        DEBUG(PRLog(kPRLogError, "Verify Chip ID took too long to receive data\n"));
        PRSetLastErrorText("Verify Chip ID took too long to receive data");
        rc = kPRFailure;
    }
    return (rc);
}

PRResult PRDevice::RequestData(uint32_t module_select, uint32_t start_addr, int32_t num_words)
{
    uint32_t requestWord = CreateRegRequestWord(module_select, start_addr, num_words);
    return WriteData(&requestWord, 1);
}

PRResult PRDevice::PrepareWriteData(uint32_t * words, int32_t numWords)
{
    if (numWords > maxWriteWords)
    {
        PRSetLastErrorText("%d words Exceeds write capabilities.  Restrict write requests to %d words.", numWords, maxWriteWords);
        return kPRFailure;
    }

    // If there are already some words prepared to be written and the addition of the new
    // words will be too many, flush the currently prepared words to the P-ROC now.
    if (numPreparedWriteWords + numWords > maxWriteWords)
    {
        if (FlushWriteData() == kPRFailure)
            return kPRFailure;
    }

    memcpy(preparedWriteWords + numPreparedWriteWords, words, numWords * 4);
    numPreparedWriteWords += numWords;

    return kPRSuccess;
}

PRResult PRDevice::FlushWriteData()
{
    PRResult res;
    res = WriteData(preparedWriteWords, numPreparedWriteWords);
    numPreparedWriteWords = 0; // Reset word counter
    return res;
}

PRResult PRDevice::WriteData(uint32_t * words, int32_t numWords)
{
    int32_t j,k;
//  int32_t item;

    if (numWords == 0)
        return kPRSuccess;

    // The 32-bit words coming in are in the same byte order they need to be in the P-ROC.
    // However, due to Intel endian-ness, simply casting the words to 4 bytes changes the
    // byte order.  So, the conversion to bytes is done here manually to preserve the byte
    // order.  Might want to add a parameter for endian-ness at some point to make it
    // work on big endian architectures.
    for (j = 0; j < numWords; j++) {
        uint32_t temp_word = words[j];
        for (k = 3; k >= 0; k--)
        {
            wr_buffer[(j*4)+k] = (uint8_t)(temp_word & 0x000000ff);
            temp_word = temp_word >> 8;
        }
    }

    int bytesToWrite = numWords * 4;
    int bytesWritten = PRHardwareWrite(wr_buffer, bytesToWrite);

    if (bytesWritten != bytesToWrite)
    {
        PRSetLastErrorText("Error in WriteData: wrote %d of %d bytes", bytesWritten, bytesToWrite);
        return kPRFailure;
    }
    else
    {
        return kPRSuccess;
    }
}

PRResult PRDevice::WriteDataRawUnbuffered(uint32_t moduleSelect, uint32_t startingAddr, int32_t numWriteWords, uint32_t * writeBuffer)
{
	PRResult res;
    uint32_t * buffer;

    buffer = (uint32_t *)malloc((numWriteWords * 4) + 4);
    buffer[0] = CreateBurstCommand(moduleSelect, startingAddr, numWriteWords);
    memcpy(buffer+1, writeBuffer, numWriteWords * 4);
    res = PrepareWriteData(buffer, numWriteWords + 1);
    free (buffer);
	return res;
}

PRResult PRDevice::WriteDataRaw(uint32_t moduleSelect, uint32_t startingAddr, int32_t numWriteWords, uint32_t * writeBuffer)
{
	PRResult res;
    uint32_t * buffer;

    buffer = (uint32_t *)malloc((numWriteWords * 4) + 4);
    buffer[0] = CreateBurstCommand(moduleSelect, startingAddr, numWriteWords);
    memcpy(buffer+1, writeBuffer, numWriteWords * 4);
    res = WriteData(buffer, numWriteWords + 1);
    free (buffer);
	return res;
}

PRResult PRDevice::ReadDataRaw(uint32_t moduleSelect, uint32_t startingAddr, int32_t numReadWords, uint32_t * readBuffer)
{
    int32_t i;

    // Send out the request.
    RequestData(moduleSelect, startingAddr, numReadWords);

    i = 0; // Reset i so it can be used to prevent an infinite loop below

    // Wait for data to return.  Give it 10 loops before giving up.
    // Expect numReadWords + 1 word with the address.
    while (requestedDataQueue.size() < (uint32_t)((numReadWords + 1)) && i++ < 10)
    {
        PRSleep (10); // 10 milliseconds should be plenty of time.
		if (SortReturningData() != kPRSuccess)
			return kPRFailure;
    }

    // Make sure all of the requested words are available before processing them.
    // Too many words is just as bad as not enough words.
    // If too many come back, can they be trusted?
    if (requestedDataQueue.size() == (uint32_t)(numReadWords + 1))
    {
        requestedDataQueue.pop(); // Ignore address word.  TODO: Verify the address.
        for (i = 0; i < numReadWords; i++)
        {
            readBuffer[i] =  requestedDataQueue.front();
            requestedDataQueue.pop();
        }
        return kPRSuccess;
    }
    else
    {
        PRSetLastErrorText("Response length did not match.");
        return kPRFailure;
    }
}


int32_t PRDevice::ReadData(uint32_t *buffer, int32_t num_words)
{
    int32_t rc,i,j;

    // Just like in the write_data method, the bytes are ordered here manually to put
    // them in the right order.  They are pulled from the collected_bytes_fifo 1 at a time
    // and stuffed into 32-bit words, high byte to low byte.
    if ((num_words * 4) <= num_collected_bytes) {
        for (j=0; j<num_words; j++) {
            // Initialize buffer position
            buffer[j] = 0;
            for (i=0; i<4; i++) {
                buffer[j] = (collected_bytes_fifo[collected_bytes_rd_addr] << (24-(i*8))) |
                buffer[j];
                if (collected_bytes_rd_addr == (FTDI_BUFFER_SIZE-1))
                    collected_bytes_rd_addr = 0;
                else
                    collected_bytes_rd_addr++;
            }
        }
        num_collected_bytes -= (num_words * 4);

        rc = num_words;
    }
    else {
        PRSetLastErrorText("Read length did not match.");
        rc = 0;
    }
    DEBUG(PRLog(kPRLogVerbose, "Read num bytes: %d\n", rc));
    return (rc);
}

PRResult PRDevice::FlushReadBuffer()
{
    int32_t numBytes,rc=0;
    //uint32_t rd_buffer[3];
    numBytes = CollectReadData();
    DEBUG(PRLog(kPRLogError, "Flushing Read Buffer: %d bytes trashed\n", numBytes));

    //while (k < numBytes) {
    //    rc = ReadData(rd_buffer, 1);
    //    k++;
    //}
    collected_bytes_rd_addr = 0;
    collected_bytes_wr_addr = 0;
    num_collected_bytes = 0;
    return rc;
}

int32_t PRDevice::CollectReadData()
{
    int32_t rc,i;
    rc = PRHardwareRead(collect_buffer, FTDI_BUFFER_SIZE-num_collected_bytes);
    if (rc < 0)
        return rc;
    for (i=0; i<rc; i++) {
        collected_bytes_fifo[collected_bytes_wr_addr] = collect_buffer[i];
        if (collected_bytes_wr_addr == (FTDI_BUFFER_SIZE-1))
            collected_bytes_wr_addr = 0;
        else
            collected_bytes_wr_addr++;
    }
    num_collected_bytes += rc;
    if (rc > 0)
    {
        DEBUG(PRLog(kPRLogVerbose, "Collected bytes: %d\n", rc));
    }
    return (rc);
}

PRResult PRDevice::SortReturningData()
{
    int32_t num_bytes, num_words;
    uint32_t rd_buffer[FTDI_BUFFER_SIZE/4];

    num_bytes = CollectReadData();
    if (num_bytes < 0)
    {
        PRSetLastErrorText("Error in CollectReadData: %d", num_bytes);
        return kPRFailure;
    }
    num_words = num_collected_bytes/4;

    while (num_words >= 2) {
        ReadData(rd_buffer, 1);
        DEBUG(PRLog(kPRLogVerbose, "New returning word: 0x%x\n", rd_buffer[0]));

        switch ( (rd_buffer[0] & P_ROC_COMMAND_MASK) >> P_ROC_COMMAND_SHIFT)
        {
            case P_ROC_REQUESTED_DATA: {
                // Push the address word so it can be used to identify the subsequent data.
                requestedDataQueue.push(rd_buffer[0]);
                int wordsRead = ReadData(rd_buffer,
                                         (rd_buffer[0] & P_ROC_HEADER_LENGTH_MASK) >>
                                         P_ROC_HEADER_LENGTH_SHIFT);
                for (int i = 0; i < wordsRead; i++)
                {
                    DEBUG(PRLog(kPRLogVerbose, "Pushing onto unreq Q 0x%x\n", rd_buffer[i]));
                    requestedDataQueue.push(rd_buffer[i]);
                }
                break;
            }
            case P_ROC_UNREQUESTED_DATA: {
                ReadData(rd_buffer,1);
                DEBUG(PRLog(kPRLogVerbose, "Pushing onto unreq Q 0x%x\n", rd_buffer[0]));
                unrequestedDataQueue.push(rd_buffer[0]);
                break;
            }
        }
        num_words = num_collected_bytes/4;
    }
    return kPRSuccess;
}

int PRDevice::CalcCombinedVerRevision()
{
    combinedVersionRevision = (version * 0x10000) + revision;
    return 0;
}

int PRDevice::GetVersionInfo(uint16_t *verPtr, uint16_t *revPtr, uint32_t *combinedPtr)
{
    *verPtr = version;
    *revPtr = revision;
    *combinedPtr = combinedVersionRevision;
    return 0;
}

PRResult PRDevice::PRLEDColor(PRLED * pLED, uint8_t color)
{
    const int bufferWords = 2;
    uint32_t buffer[bufferWords];

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeColor, color, buffer);

    return PrepareWriteData(buffer, bufferWords);
}

PRResult PRDevice::PRLEDFade(PRLED * pLED, uint8_t fadeColor, uint16_t fadeRate)
{
    const int bufferWords = 2;
    uint32_t buffer[bufferWords];

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeFadeRateLow, fadeRate & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeFadeRateHigh, (fadeRate >> 8) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeFadeColor, fadeColor, buffer);
    return PrepareWriteData(buffer, bufferWords);

}

PRResult PRDevice::PRLEDFadeColor(PRLED * pLED, uint8_t fadeColor)
{
    const int bufferWords = 2;
    uint32_t buffer[bufferWords];

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->boardAddr, kPRLEDRegisterTypeFadeColor, fadeColor, buffer);
    return PrepareWriteData(buffer, bufferWords);

}

PRResult PRDevice::PRLEDFadeRate(uint8_t boardAddr, uint16_t fadeRate)
{
    const int bufferWords = 2;
    uint32_t buffer[bufferWords];

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, boardAddr, kPRLEDRegisterTypeFadeRateLow, fadeRate & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, boardAddr, kPRLEDRegisterTypeFadeRateHigh, (fadeRate >> 8) & 0xFF, buffer);
    return PrepareWriteData(buffer, bufferWords);
}

PRResult PRDevice::PRLEDRGBColor(PRLEDRGB * pLED, uint32_t color)
{
    const int bufferWords = 2;
    uint32_t buffer[bufferWords];

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pRedLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeColor, (color >> 16) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pGreenLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeColor, (color >> 8) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pBlueLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeColor, color & 0xFF, buffer);
    return PrepareWriteData(buffer, bufferWords);
}

PRResult PRDevice::PRLEDRGBFade(PRLEDRGB * pLED, uint32_t fadeColor, uint16_t fadeRate)
{
    const int bufferWords = 12;
    uint32_t buffer[bufferWords];

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeFadeRateLow, fadeRate & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeFadeRateHigh, (fadeRate >> 8) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pRedLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeFadeColor, (fadeColor >> 16) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeFadeRateLow, fadeRate & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeFadeRateHigh, (fadeRate >> 8) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pGreenLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeFadeColor, (fadeColor >> 8) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeFadeRateLow, fadeRate & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeFadeRateHigh, (fadeRate >> 8) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pBlueLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeFadeColor, fadeColor & 0xFF, buffer);
    return PrepareWriteData(buffer, bufferWords);
}

PRResult PRDevice::PRLEDRGBFadeColor(PRLEDRGB * pLED, uint32_t fadeColor)
{
    const int bufferWords = 2;
    uint32_t buffer[bufferWords];

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pRedLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pRedLED->boardAddr, kPRLEDRegisterTypeFadeColor, (fadeColor >> 16) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pGreenLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pGreenLED->boardAddr, kPRLEDRegisterTypeFadeColor, (fadeColor >> 8) & 0xFF, buffer);
    PrepareWriteData(buffer, bufferWords);

    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeLEDIndex, pLED->pBlueLED->LEDIndex, buffer);
    PrepareWriteData(buffer, bufferWords);
    FillPDBCommand(P_ROC_DRIVER_PDB_WRITE_COMMAND, pLED->pBlueLED->boardAddr, kPRLEDRegisterTypeFadeColor, fadeColor & 0xFF, buffer);
    return PrepareWriteData(buffer, bufferWords);
}
