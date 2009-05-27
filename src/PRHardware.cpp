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
 *  PRHardware.cpp
 *  libpinproc
 */


#include "PRHardware.h"
#include "PRCommon.h"
#include "pinproc.h"


uint32_t CreateRegRequestWord( uint32_t select, uint32_t addr, uint32_t num_words ) {
    return ( (P_ROC_READ << P_ROC_COMMAND_SHIFT) |
            (num_words << P_ROC_HEADER_LENGTH_SHIFT) |
            (select << P_ROC_MODULE_SELECT_SHIFT) |
            (addr << P_ROC_ADDR_SHIFT) );
};


uint32_t CreateBurstCommand ( uint32_t select, uint32_t addr, uint32_t num_words ) {
    return ( (P_ROC_WRITE << P_ROC_COMMAND_SHIFT) |
            (num_words << P_ROC_HEADER_LENGTH_SHIFT) |
            (select << P_ROC_MODULE_SELECT_SHIFT) |
            (addr << P_ROC_ADDR_SHIFT) );
}

int32_t CreateDriverUpdateGlobalConfigBurst ( uint32_t * burst, PRDriverGlobalConfig *driver_globals) {
    uint32_t addr;

    addr = 0;
    addr = (P_ROC_DRIVER_CTRL_REG_DECODE << P_ROC_DRIVER_CTRL_DECODE_SHIFT);

    burst[0] = CreateBurstCommand (P_ROC_BUS_DRIVER_CTRL_SELECT, addr, 1 );
    burst[1] = ( (driver_globals->enableOutputs <<
                  P_ROC_DRIVER_GLOBAL_ENABLE_DIRECT_OUTPUTS_SHIFT) |
                (driver_globals->globalPolarity <<
                 P_ROC_DRIVER_GLOBAL_GLOBAL_POLARITY_SHIFT) |
                (driver_globals->useClear <<
                 P_ROC_DRIVER_GLOBAL_USE_CLEAR_SHIFT) |
                (driver_globals->strobeStartSelect <<
                 P_ROC_DRIVER_GLOBAL_STROBE_START_SELECT_SHIFT) |
                (driver_globals->startStrobeTime <<
                 P_ROC_DRIVER_GLOBAL_START_STROBE_TIME_SHIFT) |
                (driver_globals->matrixRowEnableIndex1 <<
                 P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_1_SHIFT) |
                (driver_globals->matrixRowEnableIndex0 <<
                 P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_0_SHIFT) |
                (driver_globals->activeLowMatrixRows <<
                 P_ROC_DRIVER_GLOBAL_ACTIVE_LOW_MATRIX_ROWS_SHIFT) |
                (driver_globals->encodeEnables <<
                 P_ROC_DRIVER_GLOBAL_ENCODE_ENABLES_SHIFT) |
                (driver_globals->tickleSternWatchdog <<
                 P_ROC_DRIVER_GLOBAL_TICKLE_WATCHDOG_SHIFT) );

    return kPRSuccess;
}

int32_t CreateDriverUpdateGroupConfigBurst ( uint32_t * burst, PRDriverGroupConfig *driver_group) {
    uint32_t addr;

    addr = 0;
    addr = (P_ROC_DRIVER_CTRL_REG_DECODE << P_ROC_DRIVER_CTRL_DECODE_SHIFT) |
    driver_group->groupNum;

    burst[0] = CreateBurstCommand (P_ROC_BUS_DRIVER_CTRL_SELECT, addr, 1 );
    burst[1] = ( (driver_group->slowTime <<
                  P_ROC_DRIVER_GROUP_SLOW_TIME_SHIFT) |
                (driver_group->disableStrobeAfter <<
                 P_ROC_DRIVER_GROUP_DISABLE_STROBE_AFTER_SHIFT) |
                (driver_group->enableIndex <<
                 P_ROC_DRIVER_GROUP_ENABLE_INDEX_SHIFT) |
                (driver_group->rowActivateIndex <<
                 P_ROC_DRIVER_GROUP_ROW_ACTIVATE_INDEX_SHIFT) |
                (driver_group->rowEnableSelect <<
                 P_ROC_DRIVER_GROUP_ROW_ENABLE_SELECT_SHIFT) |
                (driver_group->matrixed <<
                 P_ROC_DRIVER_GROUP_MATRIXED_SHIFT) |
                (driver_group->polarity <<
                 P_ROC_DRIVER_GROUP_POLARITY_SHIFT) |
                (driver_group->active <<
                 P_ROC_DRIVER_GROUP_ACTIVE_SHIFT) );
    return kPRSuccess;
}

int32_t CreateDriverUpdateBurst ( uint32_t * burst, PRDriverState *driver) {
    uint32_t addr;

    addr = 0;
    addr = (P_ROC_DRIVER_CONFIG_TABLE_DECODE << P_ROC_DRIVER_CTRL_DECODE_SHIFT) |
    (driver->driverNum << P_ROC_DRIVER_CONFIG_TABLE_DRIVER_NUM_SHIFT);

    burst[0] = CreateBurstCommand (P_ROC_BUS_DRIVER_CTRL_SELECT, addr, 2 );
    burst[1] = ( (driver->outputDriveTime << P_ROC_DRIVER_CONFIG_OUTPUT_DRIVE_TIME_SHIFT) |
                (driver->polarity << P_ROC_DRIVER_CONFIG_POLARITY_SHIFT) |
                (driver->state << P_ROC_DRIVER_CONFIG_STATE_SHIFT) |
                (1 << P_ROC_DRIVER_CONFIG_UPDATE_SHIFT) |
                (driver->waitForFirstTimeSlot << P_ROC_DRIVER_CONFIG_WAIT_4_1ST_SLOT_SHIFT) |
                (driver->timeslots << P_ROC_DRIVER_CONFIG_TIMESLOT_SHIFT) );
    burst[2] = (driver->timeslots >> P_ROC_DRIVER_CONFIG_TIMESLOT_SHIFT) |
    (driver->patterOnTime << P_ROC_DRIVER_CONFIG_PATTER_ON_TIME_SHIFT) |
    (driver->patterOffTime << P_ROC_DRIVER_CONFIG_PATTER_OFF_TIME_SHIFT) |
    (driver->patterEnable << P_ROC_DRIVER_CONFIG_PATTER_ENABLE_SHIFT);
    return kPRSuccess;
}

int32_t CreateWatchdogConfigBurst ( uint32_t * burst, bool_t watchdogExpired,
                                   bool_t watchdogEnable, uint16_t watchdogResetTime) {
    uint32_t addr;
    
    addr = P_ROC_REG_WATCHDOG_ADDR;
    burst[0] = CreateBurstCommand (P_ROC_MANAGER_SELECT, addr, 1 );
    burst[1] = ( (watchdogExpired << P_ROC_MANAGER_WATCHDOG_EXPIRED_SHIFT) |
                (watchdogEnable << P_ROC_MANAGER_WATCHDOG_ENABLE_SHIFT) |
                (watchdogResetTime << P_ROC_MANAGER_WATCHDOG_RESET_TIME_SHIFT) );
    
    return kPRSuccess;
}

int16_t CreateSwitchRuleIndex(uint8_t switchNum, PREventType eventType) 
{
    uint32_t debounce = (eventType == kPREventTypeSwitchOpenDebounced) || (eventType == kPREventTypeSwitchClosedDebounced) ? 1 : 0;
    uint32_t state    = (eventType == kPREventTypeSwitchOpenDebounced) || (eventType == kPREventTypeSwitchOpenNondebounced) ? 1 : 0;
	
    uint32_t index = ((debounce << P_ROC_SWITCH_RULE_NUM_DEBOUNCE_SHIFT) |
                      (state << P_ROC_SWITCH_RULE_NUM_STATE_SHIFT) |
                      (switchNum << P_ROC_SWITCH_RULE_NUM_SWITCH_NUM_SHIFT) );
    return index;
}

int32_t CreateSwitchRuleAddr(uint8_t switchNum, PREventType eventType) 
{
    uint16_t number = CreateSwitchRuleIndex( switchNum, eventType );
    uint32_t addr = number << P_ROC_SWITCH_RULE_NUM_TO_ADDR_SHIFT;
    return addr;
}

void ParseSwitchRuleIndex(uint16_t index, uint8_t *switchNum, PREventType *eventType)
{
    *switchNum = (index >> P_ROC_SWITCH_RULE_NUM_SWITCH_NUM_SHIFT) & 0xff;

    bool open = ((index >> P_ROC_SWITCH_RULE_NUM_STATE_SHIFT) & 0x1) == 0x1;
    bool debounce = ((index >> P_ROC_SWITCH_RULE_NUM_DEBOUNCE_SHIFT) & 0x1) == 0x1;
    if (open)
        *eventType = debounce ? kPREventTypeSwitchOpenDebounced : kPREventTypeSwitchOpenNondebounced;
    else
        *eventType = debounce ? kPREventTypeSwitchClosedDebounced : kPREventTypeSwitchClosedNondebounced;
}

int32_t CreateSwitchesUpdateRulesBurst ( uint32_t * burst, PRSwitchRuleInternal *rule_record) {
    uint32_t addr = CreateSwitchRuleAddr(rule_record->switchNum, rule_record->eventType);
    uint32_t driver_command[3];

    CreateDriverUpdateBurst ( driver_command, &(rule_record->driver));

    burst[0] = CreateBurstCommand (P_ROC_BUS_STATE_CHANGE_PROC_SELECT, addr, 3 );
    burst[1] = driver_command[1];
    burst[2] = driver_command[2];

    burst[3] = (rule_record->changeOutput << P_ROC_SWITCH_RULE_CHANGE_OUTPUT_SHIFT) |
    (rule_record->driver.driverNum << P_ROC_SWITCH_RULE_DRIVER_NUM_SHIFT) |
    (rule_record->linkActive << P_ROC_SWITCH_RULE_LINK_ACTIVE_SHIFT) |
    (rule_record->linkIndex << P_ROC_SWITCH_RULE_LINK_ADDRESS_SHIFT) |
    (rule_record->notifyHost << P_ROC_SWITCH_RULE_NOTIFY_HOST_SHIFT);
    return kPRSuccess;

}

int32_t CreateDMDUpdateConfigBurst ( uint32_t * burst, PRDMDConfig *dmd_config)
{
    uint32_t addr;
    uint32_t i;

    addr = 0;
    burst[0] = CreateBurstCommand (P_ROC_BUS_DMD_SELECT, addr, 1 );
    burst[1] = (1 << P_ROC_DMD_ENABLE_SHIFT) |
               (dmd_config->numSubFrames << P_ROC_DMD_NUM_SUB_FRAMES_SHIFT) |
               (dmd_config->numRows << P_ROC_DMD_NUM_ROWS_SHIFT) |
               (dmd_config->numColumns << P_ROC_DMD_NUM_COLUMNS_SHIFT); 

    addr = 8;
    burst[2] = CreateBurstCommand (P_ROC_BUS_DMD_SELECT, addr, 4 );

    for (i=0; i<4; i++) {
        burst[i+3] = (dmd_config->rclkLowCycles[i] << P_ROC_DMD_RCLK_LOW_CYCLES_SHIFT) |
        (dmd_config->latchHighCycles[i] << P_ROC_DMD_LATCH_HIGH_CYCLES_SHIFT) |
        (dmd_config->deHighCycles[i] << P_ROC_DMD_DE_HIGH_CYCLES_SHIFT) |
        (dmd_config->dotclkHalfPeriod[i] << P_ROC_DMD_DOTCLK_HALF_PERIOD_SHIFT);
    }
    return kPRSuccess;
}


/**
 * This is where all FTDI driver-specific code should go.
 * As we add support for other drivers (such as D2xx on Windows), we will add more implementations of the PRHardware*() functions here.
 */

#if !defined(USE_LIBFTDI)
#define USE_LIBFTDI 1
#endif

#if USE_LIBFTDI

#include <ftdi.h>

static bool ftdiInitialized;
static ftdi_context ftdic;


PRResult PRHardwareOpen()
{
    int32_t i=0;
    PRResult rc;
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128];
    
    ftdiInitialized = false;
    
    // Open the FTDI device
    if (ftdi_init(&ftdic) != 0)
    {
        DEBUG(PRLog("Failed to initialize FTDI.\n"));
        return kPRFailure;
    }
    
    // Find all FTDI devices
    // This is very basic and really only expects to see 1 device.  It needs to be
    // smarter.  At the very least, it should check some register on the P-ROC versus
    // an input parameter to ensure the software is set up for the same architecture as
    // the P-ROC (Stern vs WPC).  Otherwise, it's possible to drive the coils the wrong
    // polarity and blow fuses or fry transistors and all other sorts of badness.
    
    // We first enumerate all of the devices:
    int numDevices = ftdi_usb_find_all(&ftdic, &devlist, FTDI_VENDOR_ID, FTDI_FT245RL_PRODUCT_ID);
    if (numDevices < 0) {
        DEBUG(PRLog("ftdi_usb_find_all failed: %d: %s\n", numDevices, ftdi_get_error_string(&ftdic)));
        ftdi_deinit(&ftdic);
        return kPRFailure;
    }
    else {
        DEBUG(PRLog("Number of FTDI devices found: %d\n", numDevices));
        
        for (curdev = devlist; curdev != NULL; i++) {
            DEBUG(PRLog("Checking device %d\n", i));
            if ((rc = (int32_t)ftdi_usb_get_strings(&ftdic, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0) {
                DEBUG(PRLog("  ftdi_usb_get_strings failed: %d: %s\n", rc, ftdi_get_error_string(&ftdic)));
            }
            else {
                DEBUG(PRLog("  Device #%d:\n", i));
                DEBUG(PRLog("  Manufacturer: %s\n", manufacturer));
                DEBUG(PRLog("  Description: %s\n", description));
            }
            curdev = curdev->next;
        }
        
    }
    
    // Don't need the device list anymore
    ftdi_list_free (&devlist);
    
    if ((rc = (int32_t)ftdi_usb_open(&ftdic, FTDI_VENDOR_ID, FTDI_FT245RL_PRODUCT_ID)) < 0)
    {
        DEBUG(PRLog("ERROR: Unable to open ftdi device: %d: %s\n", rc, ftdi_get_error_string(&ftdic)));
        return kPRFailure;
    }
    else
    {
        rc = kPRSuccess;
        if (ftdic.type == TYPE_R) {
            uint32_t chipid;
            ftdi_read_chipid(&ftdic,&chipid);
            DEBUG(PRLog("FTDI chip_id = 0x%x\n", chipid));
            ftdiInitialized = true;
            return kPRSuccess;
        }
        else
        {
            DEBUG(PRLog("FTDI type != TYPE_R: 0x%x\n", ftdic.type));
            return kPRFailure;
        }
    }
}
void PRHardwareClose()
{
    if (ftdiInitialized)
    {
        ftdi_usb_close(&ftdic);
        ftdi_deinit(&ftdic);
    }
}
int PRHardwareRead(uint8_t *buffer, int maxBytes)
{
    return ftdi_read_data(&ftdic, buffer, maxBytes);
}
int PRHardwareWrite(uint8_t *buffer, int bytes)
{
    return ftdi_write_data(&ftdic, buffer, bytes);
}

#endif // USE_LIBFTDI
