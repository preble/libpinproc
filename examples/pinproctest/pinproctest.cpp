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
/*
 *  pinproctest.cpp
 *  libpinproc
 */
#include "pinproctest.h"


/** Demonstration of the custom logging callback. */
void TestLogger(PRLogLevel level, const char *text)
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

    // Retrieve and store initial switch states. 
    LoadSwitchStates(proc);

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
            UpdateSwitchState( event );
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
    {
        fprintf(stderr, "Error during PRCreate: %s\n", PRGetLastErrorText());
        return 1;
    }
    
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
