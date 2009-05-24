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
#include "pinproc.h" // Include libpinproc's header.

/** Demonstration of the custom logging callback. */
void TestLogger(const char *text)
{
    fprintf(stderr, "TEST: %s", text);
}

void ConfigureDrivers(PRHandle proc)
{
    int i;

    PRDriverGlobalConfig globals;
    globals.enableOutputs = false;
    globals.globalPolarity = false;
    globals.useClear = false;
    globals.strobeStartSelect = false;
    globals.startStrobeTime = 4; // milliseconds per output loop
    globals.matrixRowEnableIndex1 = 12;
    globals.matrixRowEnableIndex0 = 6;
    globals.activeLowMatrixRows = true;
    globals.tickleSternWatchdog = false;
    globals.encodeEnables = false;
    globals.watchdogExpired = false;
    globals.watchdogEnable = true;
    globals.watchdogResetTime = 1000; // milliseconds

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
    for (i = 0; i < 6; i++)
    {
        PRDriverGetGroupConfig(proc, i + 4, &group);
        group.slowTime = 0;
        group.enableIndex = i;
        group.rowActivateIndex = i;
        group.rowEnableSelect = 0;
        group.matrixed = false;
        group.polarity = false;
        group.active = 1;
        group.disableStrobeAfter = false;

        PRDriverUpdateGroupConfig(proc, &group);
    }

    // The following 8 groups are configured for the feature lamp matrix.
    for (i = 6; i < 14; i++) {
        PRDriverGetGroupConfig(proc, i + 4, &group);
        group.slowTime = 400;
        group.enableIndex = 7;
        group.rowActivateIndex = i - 6;
        group.rowEnableSelect = 0;
        group.matrixed = 1;
        group.polarity = 0;
        group.active = 1;
        group.disableStrobeAfter = 1;
        PRDriverUpdateGroupConfig(proc, &group);
    }
}

void ConfigureSwitches(PRHandle proc)
{
    int i;

    // Configures rules to notify the host for every debounced switch event.
    for (i = 0; i <= kPRSwitchPhysicalLast; i++)
    {
        PRSwitchRule sw;
        sw.notifyHost = true;
        PRSwitchesUpdateRule(proc, i, kPREventTypeSwitchClosedDebounced, &sw, NULL, 0);
        PRSwitchesUpdateRule(proc, i, kPREventTypeSwitchOpenDebounced, &sw, NULL, 0);
    }
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
    PRSwitchesUpdateRule(proc,swNum, kPREventTypeSwitchClosedNondebounced, &sw, drivers, numDriverRules);

    // Flipper off rules
    PRDriverGetState(proc, mainCoilNum, &drivers[0]);
    PRDriverStateDisable(&drivers[0]); // Disable main coil
    PRDriverGetState(proc, holdCoilNum, &drivers[1]);
    PRDriverStateDisable(&drivers[1]); // Disable hold coil
    sw.notifyHost = false;
    PRSwitchesUpdateRule(proc,swNum, kPREventTypeSwitchOpenNondebounced, &sw, drivers, numDriverRules);
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
    PRSwitchesUpdateRule(proc,swNum, kPREventTypeSwitchClosedNondebounced, &sw, drivers, numDriverRules);
}

void ConfigureSwitchRules(PRHandle proc)
{
    // WPC  Flippers
    ConfigureWPCFlipperSwitchRule (proc, 1, 32, 33, 34); // Lower Right WPC Flipper
    ConfigureWPCFlipperSwitchRule (proc, 3, 34, 35, 34); // Lower Left WPC Flipper
    ConfigureWPCFlipperSwitchRule (proc, 5, 36, 37, 34); // Upper Right WPC Flipper
    ConfigureWPCFlipperSwitchRule (proc, 7, 38, 39, 34); // Upper Left WPC Flipper

    // WPC  Slingshots
    ConfigureBumperRule (proc, 97, 71, 25); // WPC Right Slingshot
    ConfigureBumperRule (proc, 96, 70, 25); // WPC Left Slingshot
}

void ConfigureDMD (PRHandle proc)
{
    int i;

    // Create the structure that holds the DMD settings
    PRDMDConfig dmdConfig;
    memset(&dmdConfig, 0x0, sizeof(dmdConfig));
   
    dmdConfig.numRows = 32;
    dmdConfig.numColumns = 128;
    dmdConfig.numSubFrames = 4;
   
    for (i = 0; i < 4; i++)
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
void UpdateDots( unsigned char * dots, unsigned int dotPointer )
{
    int i,j,k,dot_byte,color,mappedColor,loopCtr,shifter;

    loopCtr = dotPointer/1;
    color = 0xf;
    shifter = 0x80;

    // Slow it down just a tad
    if (dotPointer%1 == 0)
    {
        // Set up shifter to rotate pattern to the right.
        shifter = pow(2,(loopCtr%8));

        // Clear the DMD dots every time the rotation occurs
        memset(dots,0,((128*32)/8)*4);
    
        // Loop through all of the rows
        //for (i = (loopCtr%32)+32; i >= loopCtr%32; i--)
        for (i = 31; i >= 0; i--)
        {
            // Map the color index to the DMD's physical color map
            switch (color) 
            {
                case 0: mappedColor = 0; break;
                case 1: mappedColor = 2; break;
                case 2: mappedColor = 8; break;
                case 3: mappedColor = 1; break;
                case 4: mappedColor = 10; break;
                case 5: mappedColor = 3; break;
                case 6: mappedColor = 9; break;
                case 7: mappedColor = 4; break;
                case 8: mappedColor = 11; break;
                case 9: mappedColor = 6; break;
                case 10: mappedColor = 12; break;
                case 11: mappedColor = 5; break;
                case 12: mappedColor = 14; break;
                case 13: mappedColor = 7; break;
                case 14: mappedColor = 13; break;
                case 15: mappedColor = 15; break;
            }

            // Loop through each of 16 bytes in a row
            for (j = 0; j < 16; j++)
            {
                // Loop through each subframe
                for (k = 0; k < 4; k++)
                {
                    // Turn on the byte in each sub-frame that's enabled 
                    // active for the color code.
                    if ((mappedColor >> k) & 1 == 1) dots[k*(128*32/8)+((i%32)*16)+j] = shifter;
                }
            }
            if (i%2 == 0) color--;
            if (shifter == 1) shifter = 0x80;
            else shifter = shifter >> 1;
        }
    }

}

bool runLoopRun = true;

void RunLoop(PRHandle proc)
{
    const int maxEvents = 16;
    PREvent events[maxEvents];

    // Create dot array using an array of bytes.  Each byte holds 8 dots.  Need
    // space for 4 sub-frames of 128/32 dots.
    unsigned char dots[4*((128*32)/8)]; 
    unsigned int dotPointer = 0;

    while (runLoopRun)
    {
        PRDriverWatchdogTickle(proc);
         
        // Create a dot pattern to test the DMD
        UpdateDots(dots,dotPointer++);
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
            printf("switch % 3d: %s\n", event->value, stateText);
        }
        usleep(10*1000); // Sleep for 10ms so we aren't pegging the CPU.
    }
}

void sigint(int)
{
    runLoopRun = false;
    signal(SIGINT, SIG_DFL); // Re-install the default signal handler.
    printf("Exiting...\n");
}

int main(const char **argv, int argc)
{
    // Set a signal handler so that we can exit gracefully on Ctrl-C:
    signal(SIGINT, sigint);

    // Assign a custom logging callback to demonstrate capturing log information from P-ROC:
    PRLogSetCallback(TestLogger);

    // Finally instantiate the P-ROC device:
    PRHandle proc = PRCreate(kPRMachineWPC);
    if (proc == kPRHandleInvalid)
        return 1;

    printf("Configuring P-ROC...\n");
    PRLoadDefaultsFromYAML(proc, "../../examples/pinproctest/Example.yaml");

    ConfigureDMD(proc); 
    ConfigureSwitches(proc); // Notify host for all debounced switch events.
    ConfigureSwitchRules(proc); // Flippers, slingshots

    // Make Drivers the last thing to configure so watchdog doesn't expire
    // before the RunLoop begins.
    ConfigureDrivers(proc);

    printf("Running.  Hit Ctrl-C to exit.\n");

    // Pulse a coil for testing purposes.
    //PRDriverPulse(proc, 53, 100);
    // Schedule a feature lamp for testing purposes.
    //PRDriverSchedule(proc, 80, 0xFF00FF00, 0, 0);
    // Pitter-patter a feature lamp for testing purposes.
    //PRDriverPatter(proc, 84, 127, 127, 0);

    RunLoop(proc);

    // Destroy the P-ROC device handle:
    PRDelete(proc);
    proc = kPRHandleInvalid;

    return 0;
}
