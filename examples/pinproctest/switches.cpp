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