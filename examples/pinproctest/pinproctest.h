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
#define kDMDFrameBuffers (3) // 3 is the max

void ConfigureDrivers(PRHandle proc, PRMachineType machineType, YAML::Node& yamlDoc);

void ConfigureSwitches(PRHandle proc, YAML::Node& yamlDoc);
void ConfigureSwitchRules(PRHandle proc, YAML::Node& yamlDoc);
void UpdateSwitchState (PREvent * event);
void LoadSwitchStates (PRHandle proc);

void ConfigureDMD(PRHandle proc);
void UpdateDots(unsigned char * dots, unsigned int dotOffset);
