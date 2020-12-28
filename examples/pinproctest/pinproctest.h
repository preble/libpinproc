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
#ifndef PINPROCTEST_PINPROCTEST_H
#define PINPROCTEST_PINPROCTEST_H
#if !defined(__GNUC__) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)	// GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <string.h>
#include <stdio.h>
#include <signal.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <cmath>
#include "pinproc.h" // Include libpinproc's header.
#include <fstream>

#ifdef _MSC_VER
  #include <time.h>
  #include <sys/timeb.h>
  #include <windows.h>
#else
  #include <sys/time.h>
#endif

#define kFlippersSection "PRFlippers"
#define kBumpersSection "PRBumpers"
#define kCoilsSection "PRCoils"
#define kSwitchesSection "PRSwitches"
#define kNumberField "number"

#define kFlipperPulseTime (34) // 34 ms
#define kFlipperPatterOnTime (2) // 2 ms
#define kFlipperPatterOffTime (18) // 2 ms
#define kBumperPulseTime (25) // 25 ms

#define kDMDColumns (128)
#define kDMDRows (32)
#define kDMDSubFrames (4) // For color depth of 16
#define kDMDFrameBuffers (3) // 3 is the max

void ConfigureDrivers(PRHandle proc);

void ConfigureSwitches(PRHandle proc);
void UpdateSwitchState (PREvent * event);
void LoadSwitchStates (PRHandle proc);

void ConfigureDMD(PRHandle proc);
void UpdateDots(unsigned char * dots, unsigned int dotOffset);

void UpdateAlphaDisplay(PRHandle, int);

#endif	/* PINPROCTEST_PINPROCTEST_H */
