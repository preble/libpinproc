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
    globals.tickleWatchdog = false;
    globals.encodeEnables = false;

    // We want to start up safely, so we'll update the global driver config twice.
    // When we toggle enableOutputs like this P-ROC will reset the polarity:

    // Enable now without the outputs enabled:
    PRDriverUpdateGlobalConfig(proc, &globals);

    // Now enable the outputs:  (TODO: Why?)
    globals.enableOutputs = true;
    PRDriverUpdateGlobalConfig(proc, &globals);

    // Configure the groups:

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

    for (i = 0; i <= kPRSwitchPhysicalLast; i++)
    {
        PRSwitchRule sw;
        sw.notifyHost = true;
        PRSwitchesUpdateRule(proc, i, kPREventTypeSwitchClosedDebounced, &sw, NULL, 0);
        PRSwitchesUpdateRule(proc, i, kPREventTypeSwitchOpenDebounced, &sw, NULL, 0);
    }
}

bool runLoopRun = true;

void RunLoop(PRHandle proc)
{
    const int maxEvents = 16;
    PREvent events[maxEvents];
    while (runLoopRun)
    {
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

    ConfigureDrivers(proc);
    ConfigureSwitches(proc);

    printf("Running.  Hit Ctrl-C to exit.\n");

    // Pulse a coil to test:
//	PRDriverDisable(proc, 80);
//	PRDriverPulse(proc, 53, 100);

    RunLoop(proc);

    // Destroy the P-ROC device handle:
    PRDelete(proc);
    proc = kPRHandleInvalid;

    return 0;
}
