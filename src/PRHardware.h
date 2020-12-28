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
#ifndef PINPROC_PRHARDWARE_H
#define PINPROC_PRHARDWARE_H
#if !defined(__GNUC__) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)	// GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <stdint.h>
#include "pinproc.h"

#if defined(__WIN32__) || defined(_WIN32)
    #include <windows.h>
    #define PRSleep(milliseconds) Sleep(milliseconds)
#else
    #define PRSleep(milliseconds) usleep(milliseconds*1000)
#endif

const int32_t FTDI_VENDOR_ID = 0x0403;
const int32_t FTDI_FT245RL_PRODUCT_ID = 0x6001;
const int32_t FTDI_FT240X_PRODUCT_ID = 0x6015;

//const int32_t FTDI_BUFFER_SIZE = 2048;
const int32_t FTDI_BUFFER_SIZE = 8192;


typedef enum PRLEDRegisterType {
    kPRLEDRegisterTypeLEDIndex        = 0,
    kPRLEDRegisterTypeColor           = 1,
    kPRLEDRegisterTypeFadeColor       = 2,
    kPRLEDRegisterTypeFadeRateLow     = 3,
    kPRLEDRegisterTypeFadeRateHigh    = 4
} PRPDLEDRegisterType;

typedef struct PRSwitchRuleInternal {
    uint8_t switchNum;    /**< Number of the physical switch, or for linked driver changes the virtual switch number (224 and up). */
    PREventType eventType; /**< The event type that this rule generates.  Determines closed/open, debounced/non-debounced. */
    bool_t reloadActive;
    bool_t notifyHost;
    bool_t changeOutput;   /**< True if this switch rule should affect a driver output change. */
    bool_t linkActive;     /**< True if this switch rule has additional linked driver updates. */
    uint16_t linkIndex;  /**< Switch rule index ({debounce,state,switchNum}) of the linked driver update rule. */
    PRDriverState driver;  /**< Driver state change to affect once this rule is triggered. */
} PRSwitchRuleInternal;


bool_t IsStern (uint32_t hardware_data);
uint32_t CreateRegRequestWord( uint32_t select, uint32_t addr, uint32_t num_words);
uint32_t CreateBurstCommand ( uint32_t select, uint32_t addr, uint32_t num_words);
int32_t CreateManagerUpdateConfigBurst ( uint32_t * burst, PRManagerConfig *manager_config);
int32_t CreateDriverUpdateGlobalConfigBurst ( uint32_t * burst, PRDriverGlobalConfig *driver_globals);
int32_t CreateDriverUpdateGroupConfigBurst ( uint32_t * burst, PRDriverGroupConfig *driver_group);
int32_t CreateDriverUpdateBurst ( uint32_t * burst, PRDriverState *driver);
uint32_t CreateDriverAuxCommand ( PRDriverAuxCommand command);

int32_t CreateWatchdogConfigBurst ( uint32_t * burst, bool_t watchdogExpired,
                                   bool_t watchdogEnable, uint16_t watchdogResetTime);

int32_t CreateDMDUpdateConfigBurst ( uint32_t * burst, PRDMDConfig *dmd_config);

int32_t CreateSwitchUpdateConfigBurst ( uint32_t * burst, PRSwitchConfig *switchConfig);
int32_t CreateSwitchUpdateRulesBurst ( uint32_t * burst, PRSwitchRuleInternal *rule_record, bool_t drive_outputs_now);

void ParseSwitchRuleIndex(uint16_t index, uint8_t *switchNum, PREventType *eventType);
int16_t CreateSwitchRuleIndex(uint8_t switchNum, PREventType eventType);
int32_t CreateSwitchRuleAddr(uint8_t switchNum, PREventType eventType, bool_t drive_outputs_now);

int32_t CreateJTAGLatchOutputsBurst ( uint32_t * burst, PRJTAGOutputs *jtagOutputs);
int32_t CreateJTAGForceOutputsBurst ( uint32_t * burst, PRJTAGOutputs *jtagOutputs);
int32_t CreateJTAGShiftTDODataBurst ( uint32_t * burst, uint16_t numBits, bool_t dataBlockComplete);

void FillPDBCommand(uint8_t command, uint8_t boardAddr, PRLEDRegisterType reg, uint8_t value, uint32_t * pData);

PRResult PRHardwareOpen();
void PRHardwareClose();
int PRHardwareRead(uint8_t *buffer, int maxBytes);
int PRHardwareWrite(uint8_t *buffer, int bytes);

#endif /* PINPROC_PRHARDWARE_H */
