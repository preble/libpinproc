/*
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
#include "pinproctest.h"



void ConfigureDrivers(PRHandle proc, PRMachineType machineType, YAML::Node& yamlDoc)
{
    int i;
    
    const int WPCDriverLoopTime = 4; // milliseconds
    const int SternDriverLoopTime = 2; // milliseconds
    
    const int mappedWPCDriverGroupEnableIndex[] = {0, 0, 0, 0, 0, 2, 4, 3, 1, 5, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0};
    const int mappedSternDriverGroupEnableIndex[] = {0, 0, 0, 0, 1, 0, 2, 3, 0, 0, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9};
    const int mappedWPCDriverGroupSlowTime[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 400, 400, 400, 400, 400, 400, 400, 400, 0, 0, 0, 0, 0, 0, 0, 0};
    const int mappedSternDriverGroupSlowTime[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 200, 0, 200, 0, 200, 0, 200, 0, 200, 0, 200, 0, 200, 0, 200};
    const int mappedWPCDriverGroupActivateIndex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0};
    const int mappedSternDriverGroupActivateIndex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7};
    
    const int watchdogResetTime = 1000; // milliseconds
    
    int mappedDriverGroupEnableIndex[kPRDriverGroupsMax];
    int mappedDriverGroupSlowTime[kPRDriverGroupsMax];
    int mappedDriverGroupActivateIndex[kPRDriverGroupsMax];
    int rowEnableIndex1;
    int rowEnableIndex0;
    bool tickleSternWatchdog;
    bool globalPolarity;
    bool activeLowMatrixRows;
    int driverLoopTime;
    int slowGroupTime;
    int numMatrixGroups;
    bool encodeEnables;
    int rowEnableSelect;
    
    switch (machineType) 
    {
        case kPRMachineWPC: 
        {
            memcpy(mappedDriverGroupEnableIndex,mappedWPCDriverGroupEnableIndex, 
                   sizeof(mappedDriverGroupEnableIndex)); 
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
            break;
        }
            
        case kPRMachineStern: 
        {
            memcpy(mappedDriverGroupEnableIndex,mappedSternDriverGroupEnableIndex, 
                   sizeof(mappedDriverGroupEnableIndex)); 
            rowEnableIndex1 = 6; // Unused in Stern
            rowEnableIndex0 = 10;
            tickleSternWatchdog = true;
            globalPolarity = true;
            activeLowMatrixRows = false;
            driverLoopTime = 2; // milliseconds
            memcpy(mappedDriverGroupSlowTime,mappedSternDriverGroupSlowTime, 
                   sizeof(mappedDriverGroupSlowTime)); 
            memcpy(mappedDriverGroupActivateIndex,mappedSternDriverGroupActivateIndex, 
                   sizeof(mappedDriverGroupActivateIndex)); 
            numMatrixGroups = 16;
            encodeEnables = true;
            rowEnableSelect = 0;
            break;
        }
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
    PRDriverUpdateGlobalConfig(proc, &globals);
    
    // Now enable the outputs:  (TODO: Why?)
    globals.enableOutputs = true;
    PRDriverUpdateGlobalConfig(proc, &globals);
    
    // Configure the groups.  Each group corresponds to 8 consecutive drivers, starting
    // with driver #32.  The following 6 groups are configured for coils/flashlamps.
    
    PRDriverGroupConfig group;
    for (i = 4; i < 10; i++)
    {
        PRDriverGetGroupConfig(proc, i, &group);
        group.slowTime = 0;
        group.enableIndex = mappedDriverGroupEnableIndex[i];
        group.rowActivateIndex = 0;
        group.rowEnableSelect = 0;
        group.matrixed = false;
        group.polarity = globalPolarity;
        group.active = 1;
        group.disableStrobeAfter = false;
        
        PRDriverUpdateGroupConfig(proc, &group);
    }
    
    // The following 8 groups are configured for the feature lamp matrix.
    for (i = 10; i < 10 + numMatrixGroups; i++) {
        PRDriverGetGroupConfig(proc, i, &group);
        group.slowTime = mappedDriverGroupSlowTime[i];
        group.enableIndex = mappedDriverGroupEnableIndex[i];
        group.rowActivateIndex = mappedDriverGroupActivateIndex[i];
        group.rowEnableSelect = rowEnableSelect;
        group.matrixed = 1;
        group.polarity = globalPolarity;
        group.active = 1;
        group.disableStrobeAfter = mappedDriverGroupSlowTime[i] != 0;
        PRDriverUpdateGroupConfig(proc, &group);
    }
}
