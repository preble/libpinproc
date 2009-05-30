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

typedef void (*PRLogCallback)(const char *text);

PRLogCallback logCallback = NULL;

void PRLog(const char *format, ...)
{
    const int maxLogLineLength = 1024;
    char line[maxLogLineLength];
    va_list ap;
    va_start(ap, format);
    vsnprintf(line, maxLogLineLength, format, ap);
    if (logCallback)
        logCallback(line);
    else
        fprintf(stderr, line);
}

void PRLogSetCallback(PRLogCallback callback)
{
    logCallback = callback;
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

// Driver Helper functions:
PR_EXPORT PRResult PRDriverDisable(PRHandle handle, uint16_t driverNum)
{
    PRDriverState driver;
    handleAsDevice->DriverGetState(driverNum, &driver);
    PRDriverStateDisable(&driver);
    return handleAsDevice->DriverUpdateState(&driver);
}
PR_EXPORT PRResult PRDriverPulse(PRHandle handle, uint16_t driverNum, int milliseconds)
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
PR_EXPORT void PRDriverStatePulse(PRDriverState *driver, int milliseconds)
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
    driver->state = originalOnTime != 0;
    driver->timeslots = 0;
    driver->waitForFirstTimeSlot = false;
    driver->outputDriveTime = originalOnTime;
    driver->patterOnTime = millisecondsOn;
    driver->patterOffTime = millisecondsOff;
    driver->patterEnable = true;
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

PR_EXPORT int32_t PRDMDUpdateConfig(PRHandle handle, PRDMDConfig *dmdConfig)
{
    return handleAsDevice->DMDUpdateConfig(dmdConfig);
}
PR_EXPORT PRResult PRDMDDraw(PRHandle handle, uint8_t * dots)
{
    return handleAsDevice->DMDDraw(dots);
}

