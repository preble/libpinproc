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
 *  pinproc.cpp
 *  libpinproc
 */

#include "../include/pinproc.h"
#include "PRDevice.h"
#include <stdarg.h>

#define MAX_TEXT (1024)

typedef void (*PRLogCallback)(PRLogLevel level, const char *text);

PRLogCallback logCallback = NULL;
PRLogLevel logLevel = kPRLogError;

void PRLog(PRLogLevel level, const char *format, ...)
{
    if (level < logLevel)
        return;
    
    char line[MAX_TEXT];
    va_list ap;
    va_start(ap, format);
    vsnprintf(line, MAX_TEXT, format, ap);
    if (logCallback)
        logCallback(level, line);
    else
        fprintf(stderr, "%s", line);
}

void PRLogSetCallback(PRLogCallback callback)
{
    logCallback = callback;
}

void PRLogSetLevel(PRLogLevel level)
{
    logLevel = level;
}

char lastErrorText[MAX_TEXT];

void PRSetLastErrorText(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(lastErrorText, MAX_TEXT, format, ap);
    PRLog(kPRLogError, "%s\n", lastErrorText);
}

PR_EXPORT const char *PRGetLastErrorText()
{
    return lastErrorText;
}

#define handleAsDevice ((PRDevice*)handle)

/** Create a new P-ROC device handle.  Only one handle per device may be created. This handle must be destroyed with PRDelete() when it is no longer needed. */
PR_EXPORT PRHandle PRCreate(PRMachineType machineType)
{
    PRDevice *device = PRDevice::Create(machineType);
    if (device == NULL)
        return kPRHandleInvalid;
    else
        return device;
}
/** Destroys an existing P-ROC device handle. */
PR_EXPORT void PRDelete(PRHandle handle)
{
    if (handle != kPRHandleInvalid)
        delete (PRDevice*)handle;
}

/** Resets internally maintained driver and switch rule structures and optionally writes those to the P-ROC device. */
PR_EXPORT PRResult PRReset(PRHandle handle, uint32_t resetFlags)
{
    return handleAsDevice->Reset(resetFlags);
}

// I/O

/** Flush all pending write data out to the P-ROC */
PR_EXPORT PRResult PRFlushWriteData(PRHandle handle)
{
    return handleAsDevice->FlushWriteData();
}

/** Write data out to the P-ROC immediately (does not require a call to PRFlushWriteData */
PR_EXPORT PRResult PRWriteData(PRHandle handle, uint32_t moduleSelect, uint32_t startingAddr, int32_t numWriteWords, uint32_t * writeBuffer)
{
    return handleAsDevice->WriteDataRaw(moduleSelect, startingAddr, numWriteWords, writeBuffer);
}

/** Read data from the P-ROC. */
PR_EXPORT PRResult PRReadData(PRHandle handle, uint32_t moduleSelect, uint32_t startingAddr, int32_t numReadWords, uint32_t * readBuffer)
{
    return handleAsDevice->ReadDataRaw(moduleSelect, startingAddr, numReadWords, readBuffer);
}

// Events

/** Get all of the available events that have been received. */
PR_EXPORT int PRGetEvents(PRHandle handle, PREvent *eventsOut, int maxEvents)
{
    return handleAsDevice->GetEvents(eventsOut, maxEvents);
}

// Drivers
PR_EXPORT PRResult PRDriverUpdateGlobalConfig(PRHandle handle, PRDriverGlobalConfig *driverGlobalConfig)
{
    return handleAsDevice->DriverUpdateGlobalConfig(driverGlobalConfig);
}
PR_EXPORT PRResult PRDriverGetGroupConfig(PRHandle handle, uint8_t groupNum, PRDriverGroupConfig *driverGroupConfig)
{
    return handleAsDevice->DriverGetGroupConfig(groupNum, driverGroupConfig);
}
PR_EXPORT PRResult PRDriverUpdateGroupConfig(PRHandle handle, PRDriverGroupConfig *driverGroupConfig)
{
    return handleAsDevice->DriverUpdateGroupConfig(driverGroupConfig);
}
PR_EXPORT PRResult PRDriverGetState(PRHandle handle, uint8_t driverNum, PRDriverState *driverState)
{
    return handleAsDevice->DriverGetState(driverNum, driverState);
}
PR_EXPORT PRResult PRDriverUpdateState(PRHandle handle, PRDriverState *driverState)
{
    return handleAsDevice->DriverUpdateState(driverState);
}
PR_EXPORT PRResult PRDriverLoadMachineTypeDefaults(PRHandle handle, PRMachineType machineType)
{
    return handleAsDevice->DriverLoadMachineTypeDefaults(machineType);
}

// Driver Helper functions:
PR_EXPORT PRResult PRDriverDisable(PRHandle handle, uint16_t driverNum)
{
    PRDriverState driver;
    handleAsDevice->DriverGetState(driverNum, &driver);
    PRDriverStateDisable(&driver);
    return handleAsDevice->DriverUpdateState(&driver);
}
PR_EXPORT PRResult PRDriverPulse(PRHandle handle, uint16_t driverNum, uint8_t milliseconds)
{
    PRDriverState driver;
    handleAsDevice->DriverGetState(driverNum, &driver);
    PRDriverStatePulse(&driver, milliseconds);
    return handleAsDevice->DriverUpdateState(&driver);
}
PR_EXPORT PRResult PRDriverSchedule(PRHandle handle, uint16_t driverNum, uint32_t schedule, uint8_t cycleSeconds, bool_t now)
{
    PRDriverState driver;
    handleAsDevice->DriverGetState(driverNum, &driver);
    PRDriverStateSchedule(&driver, schedule, cycleSeconds, now);
    return handleAsDevice->DriverUpdateState(&driver);
}
PR_EXPORT PRResult PRDriverPatter(PRHandle handle, uint16_t driverNum, uint16_t millisecondsOn, uint16_t millisecondsOff, uint16_t originalOnTime)
{
    PRDriverState driver;
    handleAsDevice->DriverGetState(driverNum, &driver);
    PRDriverStatePatter(&driver, millisecondsOn, millisecondsOff, originalOnTime);
    return handleAsDevice->DriverUpdateState(&driver);
}
PR_EXPORT PRResult PRDriverAuxSendCommands(PRHandle handle, PRDriverAuxCommand * commands, uint8_t numCommands, uint8_t startingAddr)
{
    return handleAsDevice->DriverAuxSendCommands(commands, numCommands, startingAddr);
}

PR_EXPORT void PRDriverAuxPrepareOutput(PRDriverAuxCommand *auxCommand, uint8_t data, uint8_t extraData, bool_t useExtraData, uint8_t enables, bool_t muxEnables)
{
    auxCommand->active = true;
    auxCommand->data = data;
    auxCommand->extraData = extraData;
    auxCommand->enables = enables;
    auxCommand->muxEnables = muxEnables;
    auxCommand->useExtraData = useExtraData;
    auxCommand->command = kPRDriverAuxCmdOutput;
}

PR_EXPORT void PRDriverAuxPrepareDelay(PRDriverAuxCommand *auxCommand, uint16_t delayTime)
{
    auxCommand->active = true;
    auxCommand->delayTime = delayTime;
    auxCommand->command = kPRDriverAuxCmdDelay;
}

PR_EXPORT void PRDriverAuxPrepareJump(PRDriverAuxCommand *auxCommand, uint8_t jumpAddr)
{
    auxCommand->active = true;
    auxCommand->jumpAddr = jumpAddr;
    auxCommand->command = kPRDriverAuxCmdJump;
}

PR_EXPORT void PRDriverAuxPrepareDisable(PRDriverAuxCommand *auxCommand)
{
    auxCommand->active = false;
}

PR_EXPORT PRResult PRDriverWatchdogTickle(PRHandle handle)
{
    return handleAsDevice->DriverWatchdogTickle();
}

PR_EXPORT void PRDriverStateDisable(PRDriverState *driver)
{
    driver->state = 0;
    driver->timeslots = 0;
    driver->waitForFirstTimeSlot = false;
    driver->outputDriveTime = 0;
    driver->patterOnTime = 0;
    driver->patterOffTime = 0;
    driver->patterEnable = false;
}
PR_EXPORT void PRDriverStatePulse(PRDriverState *driver, uint8_t milliseconds)
{
    driver->state = 1;
    driver->timeslots = 0;
    driver->waitForFirstTimeSlot = false;
    driver->outputDriveTime = milliseconds;
    driver->patterOnTime = 0;
    driver->patterOffTime = 0;
    driver->patterEnable = false;
}
PR_EXPORT void PRDriverStateSchedule(PRDriverState *driver, uint32_t schedule, uint8_t cycleSeconds, bool_t now)
{
    driver->state = 1;
    driver->timeslots = schedule;
    driver->waitForFirstTimeSlot = !now;
    driver->outputDriveTime = cycleSeconds;
    driver->patterOnTime = 0;
    driver->patterOffTime = 0;
    driver->patterEnable = false;
}
PR_EXPORT void PRDriverStatePatter(PRDriverState *driver, uint16_t millisecondsOn, uint16_t millisecondsOff, uint16_t originalOnTime)
{
    driver->state = true;
    driver->timeslots = 0;
    driver->waitForFirstTimeSlot = false;
    driver->outputDriveTime = originalOnTime;
    driver->patterOnTime = millisecondsOn;
    driver->patterOffTime = millisecondsOff;
    driver->patterEnable = true;
}

PR_EXPORT void PRDriverStatePulsedPatter(PRDriverState *driver, uint16_t millisecondsOn, uint16_t millisecondsOff, uint16_t patterTime)
{
    driver->state = false;
    driver->timeslots = 0;
    driver->waitForFirstTimeSlot = false;
    driver->outputDriveTime = patterTime;
    driver->patterOnTime = millisecondsOn;
    driver->patterOffTime = millisecondsOff;
    driver->patterEnable = true;
}

PR_EXPORT uint16_t PRDecode(PRMachineType machineType, const char *str)
{
    uint16_t x;

    if (str == NULL)
        return 0;

    if ( strlen(str) == 3 )
        x = (str[1]-'0') * 10 + (str[2]-'0');
    else if ( strlen(str) == 4)
        x = (str[2]-'0') * 10 + (str[3]-'0');
    else return atoi(str);
    
    if ((machineType == kPRMachineWPC) || (machineType == kPRMachineWPC95))
    {
        switch (str[0])
        {
            case 'F':
            case 'f':
                switch (str[1])
                {
                    case 'L':
                    case 'l':
                        switch (str[2])
                        {
                            case 'R':
                            case 'r':
                                switch (str[3])
                                {
                                    case 'M':
                                    case 'm':
                                        return 32;
                                    default:
                                        return 33; 
                                }
                            default:
                                switch (str[3])
                                {
                                    case 'M':
                                    case 'm':
                                        return 34;
                                    default:
                                        return 35; 
                                }
                        }
                    default:
                        switch (str[2])
                        {
                            case 'R':
                            case 'r':
                                switch (str[3])
                                {
                                    case 'M':
                                    case 'm':
                                        return 36;
                                    default:
                                        return 37; 
                                }
                            default:
                                switch (str[3])
                                {
                                    case 'M':
                                    case 'm':
                                        return 38;
                                    default:
                                        return 39; 
                                }
                        }
                }

            case 'L':
            case 'l':
                return 80 + 8 * ((x / 10) - 1) + ((x % 10) -1);
            case 'C':
            case 'c':
                if (x <= 28)
                    return x + 39;
                else if (x <= 36)
                    return x + 3;
                else if (x <= 44)
                {
                    if (machineType == kPRMachineWPC95)
                        return x + 7;
                    else 
                        return x + 107; // WPC 37-44 use 8-driver board (mapped to drivers 144-151)
                }
                else return x + 108;
            case 'G':
            case 'g':
                return x + 71;
            case 'S':
            case 's':
            {
                switch (str[1])
                {
                    case 'D':
                    case 'd':
                        return 8 + ((str[2]-'0') - 1);
                    case 'F':
                    case 'f':
                        return (str[2]-'0') - 1;
                    default:
                        return 32 + 16 * ((x / 10) - 1) + ((x % 10) - 1);
                }
            }
        }
    }
    else if (machineType == kPRMachineSternSAM)
    {
        switch (str[0])
        {
            case 'L':
            case 'l':
                return 80 + 16 * (7 - ((x - 1) % 8)) + (x - 1) / 8;
            case 'C':
            case 'c':
                return x + 31;
            case 'S':
            case 's':
            {
                switch (str[1])
                {
                    case 'D':
                    case 'd':
                        if (strlen(str) == 3) 
                            return (str[2]-'0') + 7;
                        else return x + 7;
                    default:
                        if ((x - 1) % 16 < 8)
                           return 32 + 8 * ((x - 1) / 8)  + (7 - ((x - 1) % 8));
                        else
                           return 32 + (x - 1);
                }
            }
        }
    }
    else if (machineType == kPRMachineSternWhitestar)
    {
        switch (str[0])
        {
            case 'L':
            case 'l':
                return 80 + 16 * (7 - ((x - 1) % 8)) + (x - 1) / 8;
            case 'C':
            case 'c':
                return x + 31;
            case 'S':
            case 's':
            {
                switch (str[1])
                {
                    case 'D':
                    case 'd':
                        if (strlen(str) == 3) 
                            return (str[2]-'0') + 7;
                        else return x + 7;
                    default:
                        return 32 + 16 * (((x-1) / 8)) + (7-((x-1) % 8));
                }
            }
        }
    }
    return atoi(str);
}

// Switches

PR_EXPORT PRResult PRSwitchUpdateConfig(PRHandle handle, PRSwitchConfig *switchConfig)
{
    return handleAsDevice->SwitchUpdateConfig(switchConfig);
}

PR_EXPORT PRResult PRSwitchUpdateRule(PRHandle handle, uint8_t switchNum, PREventType eventType, PRSwitchRule *rule, PRDriverState *linkedDrivers, int numDrivers)
{
    return handleAsDevice->SwitchUpdateRule(switchNum, eventType, rule, linkedDrivers, numDrivers);
}

PR_EXPORT PRResult PRSwitchGetStates(PRHandle handle, PREventType * switchStates, uint16_t numSwitches)
{
    return handleAsDevice->SwitchGetStates(switchStates, numSwitches);
}

// DMD

PR_EXPORT int32_t PRDMDUpdateConfig(PRHandle handle, PRDMDConfig *dmdConfig)
{
    return handleAsDevice->DMDUpdateConfig(dmdConfig);
}
PR_EXPORT PRResult PRDMDDraw(PRHandle handle, uint8_t * dots)
{
    return handleAsDevice->DMDDraw(dots);
}

// JTAG

PR_EXPORT PRResult PRJTAGDriveOutputs(PRHandle handle, PRJTAGOutputs * jtagOutputs, bool_t toggleClk)
{
    return handleAsDevice->PRJTAGDriveOutputs(jtagOutputs, toggleClk);
}

PR_EXPORT PRResult PRJTAGWriteTDOMemory(PRHandle handle, uint16_t tableOffset, uint16_t numWords, uint32_t * tdoData)
{
    return handleAsDevice->PRJTAGWriteTDOMemory(tableOffset, numWords, tdoData);
}

PR_EXPORT PRResult PRJTAGShiftTDOData(PRHandle handle, uint16_t numBits, bool_t dataBlockComplete)
{
    return handleAsDevice->PRJTAGShiftTDOData(numBits, dataBlockComplete);
}

PR_EXPORT PRResult PRJTAGReadTDIMemory(PRHandle handle, uint16_t tableOffset, uint16_t numWords, uint32_t * tdiData)
{
    return handleAsDevice->PRJTAGReadTDIMemory(tableOffset, numWords, tdiData);
}

PR_EXPORT PRResult PRJTAGGetStatus(PRHandle handle, PRJTAGStatus * status)
{
    return handleAsDevice->PRJTAGGetStatus(status);
}
