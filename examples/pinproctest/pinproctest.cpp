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
 *  pinproctest.cpp
 *  libpinproc
 */
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <cmath>
#include "../../include/pinproc.h" // Include libpinproc's header.
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <sys/time.h>

#define kFlippersSection "PRFlippers"
#define kBumpersSection "PRBumpers"
#define kCoilsSection "PRCoils"
#define kSwitchesSection "PRSwitches"
#define kNumberField "number"

#define kFlipperPulseTime (34) // 34 ms
#define kBumperPulseTime (25) // 25 ms

#define kDMDColumns (128)
#define kDMDRows (32)
#define kDMDSubFrames (4) // For color depth of 16

/** Demonstration of the custom logging callback. */
void TestLogger(const char *text)
{
    fprintf(stderr, "TEST: %s", text);
}

PRResult LoadConfiguration(YAML::Node& yamlDoc, const char *yamlFilePath)
{
    try
    {
        std::ifstream fin(yamlFilePath);
        if (fin.is_open() == false)
        {
            fprintf(stderr, "YAML file not found: %s\n", yamlFilePath);
            return kPRFailure;
        }
        YAML::Parser parser(fin);
        
        while(parser) 
        {
            parser.GetNextDocument(yamlDoc);
        }
    }
    catch (YAML::ParserException& ex)
    {
        fprintf(stderr, "YAML parse error at line=%d col=%d: %s\n", ex.line, ex.column, ex.msg.c_str());
        return kPRFailure;
    }
    catch (YAML::RepresentationException& ex)
    {
        fprintf(stderr, "YAML representation error at line=%d col=%d: %s\n", ex.line, ex.column, ex.msg.c_str());
        return kPRFailure;
    }
    catch (...)
    {
        fprintf(stderr, "Unexpected exception while parsing YAML config.\n");
        return kPRFailure;
    }
    return kPRSuccess;
}

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
    int watchdogResetTime;
    int slowGroupTime;
    int numMatrixGroups;
    bool encodeEnables;

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

void ConfigureSwitches(PRHandle proc, YAML::Node& yamlDoc)
{
    // Configure switch controller registers (if the defaults aren't acceptable)
    PRSwitchConfig switchConfig;
    switchConfig.clear = false;
    switchConfig.directMatrixScanLoopTime = 2; // milliseconds
    switchConfig.pulsesBeforeCheckingRX = 10;
    switchConfig.inactivePulsesAfterBurst = 12;
    switchConfig.pulsesPerBurst = 6;
    switchConfig.pulseHalfPeriodTime = 13; // milliseconds
    PRSwitchUpdateConfig(proc, &switchConfig);
}

void ConfigureWPCFlipperSwitchRule (PRHandle proc, int swNum, int mainCoilNum, int holdCoilNum, int pulseTime)
{
    const int numDriverRules = 2;
    PRDriverState drivers[numDriverRules];
    PRSwitchRule sw;

    // Flipper on rules
    PRDriverGetState(proc, mainCoilNum, &drivers[0]);
    PRDriverStatePulse(&drivers[0],pulseTime); // Pulse coil for 34ms.
    PRDriverGetState(proc, holdCoilNum, &drivers[1]);
    PRDriverStatePulse(&drivers[1],0);  // Turn on indefintely (set pulse for 0ms)
    sw.notifyHost = false;
    PRSwitchUpdateRule(proc, swNum, kPREventTypeSwitchClosedNondebounced, &sw, drivers, numDriverRules);
    sw.notifyHost = true;
    PRSwitchUpdateRule(proc, swNum, kPREventTypeSwitchClosedDebounced, &sw, NULL, 0);

    // Flipper off rules
    PRDriverGetState(proc, mainCoilNum, &drivers[0]);
    PRDriverStateDisable(&drivers[0]); // Disable main coil
    PRDriverGetState(proc, holdCoilNum, &drivers[1]);
    PRDriverStateDisable(&drivers[1]); // Disable hold coil
    sw.notifyHost = false;
    PRSwitchUpdateRule(proc, swNum, kPREventTypeSwitchOpenNondebounced, &sw, drivers, numDriverRules);
    sw.notifyHost = true;
    PRSwitchUpdateRule(proc, swNum, kPREventTypeSwitchOpenDebounced, &sw, NULL, 0);
}

void ConfigureBumperRule (PRHandle proc, int swNum, int coilNum, int pulseTime)
{
    const int numDriverRules = 1;
    PRDriverState drivers[numDriverRules];
    PRSwitchRule sw;

    // Lower Right Flipper On
    PRDriverGetState(proc, coilNum, &drivers[0]);
    PRDriverStatePulse(&drivers[0],pulseTime); // Pulse coil for 34ms.
    sw.notifyHost = false;
    PRSwitchUpdateRule(proc, swNum, kPREventTypeSwitchClosedNondebounced, &sw, drivers, numDriverRules);
    sw.notifyHost = true;
    PRSwitchUpdateRule(proc, swNum, kPREventTypeSwitchClosedDebounced, &sw, NULL, 0);
}

void ConfigureSwitchRules(PRHandle proc, YAML::Node& yamlDoc)
{
    // WPC  Flippers
    const YAML::Node& flippers = yamlDoc[kFlippersSection];
    for (YAML::Iterator flippersIt = flippers.begin(); flippersIt != flippers.end(); ++flippersIt)
    {
        int swNum, coilMain, coilHold;
        std::string flipperName;
        *flippersIt >> flipperName;
        yamlDoc[kSwitchesSection][flipperName][kNumberField] >> swNum;
        yamlDoc[kCoilsSection][flipperName + "Main"][kNumberField] >> coilMain;
        yamlDoc[kCoilsSection][flipperName + "Hold"][kNumberField] >> coilHold;
        ConfigureWPCFlipperSwitchRule (proc, swNum, coilMain, coilHold, kFlipperPulseTime);
    }

    const YAML::Node& bumpers = yamlDoc[kBumpersSection];
    for (YAML::Iterator bumpersIt = bumpers.begin(); bumpersIt != bumpers.end(); ++bumpersIt)
    {
        int swNum, coilNum;
        // WPC  Slingshots
        std::string bumperName;
        *bumpersIt >> bumperName;
        yamlDoc[kSwitchesSection][bumperName][kNumberField] >> swNum;
        yamlDoc[kCoilsSection][bumperName][kNumberField] >> coilNum;
        ConfigureBumperRule (proc, swNum, coilNum, kBumperPulseTime);
    }
}

void ConfigureDMD(PRHandle proc)
{
    int i;

    // Create the structure that holds the DMD settings
    PRDMDConfig dmdConfig;
    memset(&dmdConfig, 0x0, sizeof(dmdConfig));
   
    dmdConfig.numRows = kDMDRows;
    dmdConfig.numColumns = kDMDColumns;
    dmdConfig.numSubFrames = kDMDSubFrames;
   
    for (i = 0; i < kDMDSubFrames; i++)
    {
        dmdConfig.rclkLowCycles[i] = 15;
        dmdConfig.latchHighCycles[i] = 15;
        dmdConfig.dotclkHalfPeriod[i] = 1;
    }
        
    dmdConfig.deHighCycles[0] = 250;
    dmdConfig.deHighCycles[1] = 400;
    dmdConfig.deHighCycles[2] = 180;
    dmdConfig.deHighCycles[3] = 800;

    PRDMDUpdateConfig(proc, &dmdConfig);
}

// Display a simple pattern to verify DMD functionality.
// 16 diagonal lines will rotate to the right.  Every two rows will get brighter, 
// starting with dim dots at the top.
void UpdateDots( unsigned char * dots, unsigned int dotOffset )
{
    int row,col,subFrame,color,mappedColor,loopCtr,byte_shifter;
    const int rate_reduction_divisor = 1;

    loopCtr = dotOffset/rate_reduction_divisor;
    color = pow(2,kDMDSubFrames) - 1;
    byte_shifter = 0x80;

    // Slow it down just a tad
    if (dotOffset%rate_reduction_divisor == 0)
    {
        // Set up byte_shifter to rotate pattern to the right.
        byte_shifter = pow(2,(loopCtr%8));

        // Clear the DMD dots every time the rotation occurs
        memset(dots,0,((kDMDColumns*kDMDRows)/8)*kDMDSubFrames);
    
        // Loop through all of the rows
        for (row = kDMDRows - 1; row >= 0; row--)
        {
            // Map the color index to the DMD's physical color map
            int mappedColors[] = {0, 2, 8, 10, 1, 3, 9, 11, 4, 6, 12, 14, 5, 7, 13, 15};
            mappedColor = mappedColors[color];

            // Loop through each of 16 bytes in a row
            for (col = 0; col < kDMDColumns / 8; col++)
            {
                // Loop through each subframe
                for (subFrame = 0; subFrame < kDMDSubFrames; subFrame++)
                {
                    // Turn on the byte in each sub-frame that's enabled 
                    // active for the color code.
                    if ((mappedColor >> subFrame) & 1 == 1) 
                        dots[subFrame*(kDMDColumns*kDMDRows/8)+((row%kDMDRows)*(kDMDColumns / 8))+col] = byte_shifter;
                }
            }
            // Determine where to change the color in order to progress from row 0 = color 0
            // to the last row being the last color.
            if (row % (int)((kDMDRows/pow(2,kDMDSubFrames))) == 0) color--;
            if (byte_shifter == 1) byte_shifter = 0x80;
            else byte_shifter = byte_shifter >> 1;
        }
    }
}

time_t startTime;
bool runLoopRun = true;

void RunLoop(PRHandle proc)
{
    const int maxEvents = 16;
    PREvent events[maxEvents];

    // Create dot array using an array of bytes.  Each byte holds 8 dots.  Need
    // space for 4 sub-frames of 128/32 dots.
    unsigned char dots[4*((128*32)/8)]; 
    unsigned int dotOffset = 0;

    while (runLoopRun)
    {
        PRDriverWatchdogTickle(proc);
         
        // Create a dot pattern to test the DMD
        UpdateDots(dots,dotOffset++);
        PRDMDDraw(proc,dots);

        int numEvents = PRGetEvents(proc, events, maxEvents);
        for (int i = 0; i < numEvents; i++)
        {
            PREvent *event = &events[i];
            const char *stateText = "Unknown";
            switch (event->type)
            {
                case kPREventTypeSwitchOpenDebounced: stateText = "open"; break;
                case kPREventTypeSwitchClosedDebounced: stateText = "closed"; break;
                case kPREventTypeSwitchOpenNondebounced: stateText = "open(ndb)"; break;
                case kPREventTypeSwitchClosedNondebounced: stateText = "closed(ndb)"; break;
            }
            struct timeval tv;
            gettimeofday(&tv, NULL);
            printf("%d.%03d switch % 3d: %s\n", tv.tv_sec-startTime, tv.tv_usec/1000, event->value, stateText);
        }
        PRFlushWriteData(proc);
        usleep(10*1000); // Sleep for 10ms so we aren't pegging the CPU.
    }
}

void sigint(int)
{
    runLoopRun = false;
    signal(SIGINT, SIG_DFL); // Re-install the default signal handler.
    printf("Exiting...\n");
}

int main(int argc, const char **argv)
{
    // Set a signal handler so that we can exit gracefully on Ctrl-C:
    signal(SIGINT, sigint);
    startTime = time(NULL);

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <yaml machine description>\n", argv[0]);
        return 1;
    }
    const char *yamlFilename = argv[1];
    
    // Assign a custom logging callback to demonstrate capturing log information from P-ROC:
    PRLogSetCallback(TestLogger);
    
    YAML::Node yamlDoc;
    if (LoadConfiguration(yamlDoc, yamlFilename) != kPRSuccess)
    {
        fprintf(stderr, "Failed to load configuration file %s\n", yamlFilename);
        return 1;
    }

    PRMachineType machineType = kPRMachineInvalid;
    std::string machineTypeString;
    yamlDoc["PRGame"]["machineType"] >> machineTypeString;
    if (machineTypeString == "wpc")
        machineType = kPRMachineWPC;
    else if(machineTypeString == "stern")
        machineType = kPRMachineStern;
    else
    {
        fprintf(stderr, "Unknown machine type: %s\n", machineTypeString.c_str());
        return 1;
    }
    
    // Finally instantiate the P-ROC device:
    PRHandle proc = PRCreate(machineType);
    if (proc == kPRHandleInvalid)
        return 1;
    
    PRReset(proc, kPRResetFlagUpdateDevice); // Reset the device structs and write them into the device.
    
    ConfigureDMD(proc); 
    ConfigureSwitches(proc, yamlDoc); // Notify host for all debounced switch events.
    ConfigureSwitchRules(proc, yamlDoc); // Flippers, slingshots

    // Make Drivers the last thing to configure so watchdog doesn't expire
    // before the RunLoop begins.
    ConfigureDrivers(proc, machineType, yamlDoc);

    printf("Running.  Hit Ctrl-C to exit.\n");

    // Pulse a coil for testing purposes.
    //PRDriverPulse(proc, 53, 100);
    // Schedule a feature lamp for testing purposes.
    PRDriverSchedule(proc, 80, 0xFF00FF00, 0, 0);
    // Pitter-patter a feature lamp for testing purposes.
    //PRDriverPatter(proc, 84, 127, 127, 0);

    RunLoop(proc);

    // Clean up P-ROC.
    printf("Disabling P-ROC drivers and switch rules...\n");
    PRReset(proc, kPRResetFlagUpdateDevice); // Reset the device structs and write them into the device.

    // Destroy the P-ROC device handle:
    PRDelete(proc);
    proc = kPRHandleInvalid;

    return 0;
}
