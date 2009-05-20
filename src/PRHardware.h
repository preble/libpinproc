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
#ifndef _PROC_HARDWARE_H_
#define _PROC_HARDWARE_H_

#include <stdint.h>
#include "../include/pinproc.h"

const int32_t FTDI_VENDOR_ID = 0x0403;
const int32_t FTDI_FT245RL_PRODUCT_ID = 0x6001;

const int32_t FTDI_BUFFER_SIZE = 2048;
const uint32_t P_ROC_INIT_PATTERN_A         = 0x801F1122;
const uint32_t P_ROC_INIT_PATTERN_B         = 0x345678AB;
const uint32_t P_ROC_CHIP_ID                = 0xFEEDBEEF;

const uint32_t P_ROC_ADDR_MASK              = 0x000FFFFF;
const uint32_t P_ROC_HEADER_LENGTH_MASK     = 0x7FF00000;
const uint32_t P_ROC_COMMAND_MASK           = 0x80000000;

const uint32_t P_ROC_ADDR_SHIFT             = 0;
const uint32_t P_ROC_HEADER_LENGTH_SHIFT    = 20;
const uint32_t P_ROC_COMMAND_SHIFT          = 31;

const uint32_t P_ROC_READ                   = 0;
const uint32_t P_ROC_WRITE                  = 1;
const uint32_t P_ROC_REQUESTED_DATA         = 0;
const uint32_t P_ROC_UNREQUESTED_DATA       = 1;

const uint32_t P_ROC_REG_ADDR_MASK          = 0x0000FFFF;
const uint32_t P_ROC_MODULE_SELECT_MASK     = 0x000F0000;

const uint32_t P_ROC_REG_ADDR_SHIFT         = 0;
const uint32_t P_ROC_MODULE_SELECT_SHIFT    = 16;

const uint32_t P_ROC_MANAGER_SELECT               = 0;
const uint32_t P_ROC_BUS_MASTER_SELECT            = 1;
const uint32_t P_ROC_BUS_SWITCH_CTRL_SELECT       = 2;
const uint32_t P_ROC_BUS_DRIVER_CTRL_SELECT       = 3;
const uint32_t P_ROC_BUS_STATE_CHANGE_PROC_SELECT = 4;
const uint32_t P_ROC_BUS_DMD_SELECT               = 5;
const uint32_t P_ROC_BUS_UNASSOCIATED_SELECT      = 15;

const uint32_t P_ROC_REG_CHIP_ID_ADDR             = 0;
const uint32_t P_ROC_REG_VERSION_ADDR             = 1;
const uint32_t P_ROC_REG_WATCHDOG_ADDR            = 2;
const uint32_t P_ROC_REG_DIPSWITCH_ADDR           = 3;

const uint32_t P_ROC_EVENT_SWITCH_NUM_MASK                 = 0xFF;
const uint32_t P_ROC_EVENT_SWITCH_STATE_MASK               = 0x100;
const uint32_t P_ROC_EVENT_SWITCH_STATE_SHIFT              = 8;

const uint32_t P_ROC_EVENT_SWITCH_DEBOUNCED_MASK           = 0x200;
const uint32_t P_ROC_EVENT_SWITCH_DEBOUNCED_SHIFT          = 9;

const uint32_t P_ROC_DRIVER_CTRL_DECODE_SHIFT     = 10;
const uint32_t P_ROC_DRIVER_CTRL_REG_DECODE       = 0;
const uint32_t P_ROC_DRIVER_CONFIG_TABLE_DECODE   = 1;
const uint32_t P_ROC_DRIVER_STATE_TABLE_DECODE    = 2;
const uint32_t P_ROC_DRIVER_CATCHALL_DECODE       = 3;

const uint32_t P_ROC_DRIVER_GLOBAL_ENABLE_DIRECT_OUTPUTS_SHIFT       = 31;
const uint32_t P_ROC_DRIVER_GLOBAL_GLOBAL_POLARITY_SHIFT             = 30;
const uint32_t P_ROC_DRIVER_GLOBAL_USE_CLEAR_SHIFT                   = 28;
const uint32_t P_ROC_DRIVER_GLOBAL_STROBE_START_SELECT_SHIFT         = 27;
const uint32_t P_ROC_DRIVER_GLOBAL_START_STROBE_TIME_SHIFT           = 20;
const uint32_t P_ROC_DRIVER_GLOBAL_START_STROBE_TIME_MASK            = 0x07F00000;
const uint32_t P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_1_SHIFT   = 16;
const uint32_t P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_1_MASK    = 0x000F0000;
const uint32_t P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_0_SHIFT   = 12;
const uint32_t P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_0_MASK    = 0x0000F000;
const uint32_t P_ROC_DRIVER_GLOBAL_ACTIVE_LOW_MATRIX_ROWS_SHIFT      = 11;
const uint32_t P_ROC_DRIVER_GLOBAL_ENCODE_ENABLES_SHIFT              = 10;
const uint32_t P_ROC_DRIVER_GLOBAL_TICKLE_WATCHDOG_SHIFT             = 9;

const uint32_t P_ROC_DRIVER_GROUP_SLOW_TIME_SHIFT                    = 12;
const uint32_t P_ROC_DRIVER_GROUP_DISABLE_STROBE_AFTER_SHIFT         = 11;
const uint32_t P_ROC_DRIVER_GROUP_ENABLE_INDEX_SHIFT                 = 7;
const uint32_t P_ROC_DRIVER_GROUP_ROW_ACTIVATE_INDEX_SHIFT           = 4;
const uint32_t P_ROC_DRIVER_GROUP_ROW_ENABLE_SELECT_SHIFT            = 3;
const uint32_t P_ROC_DRIVER_GROUP_MATRIXED_SHIFT                     = 2;
const uint32_t P_ROC_DRIVER_GROUP_POLARITY_SHIFT                     = 1;
const uint32_t P_ROC_DRIVER_GROUP_ACTIVE_SHIFT                       = 0;

const uint32_t P_ROC_DRIVER_CONFIG_OUTPUT_DRIVE_TIME_SHIFT = 0;
const uint32_t P_ROC_DRIVER_CONFIG_POLARITY_SHIFT          = 8;
const uint32_t P_ROC_DRIVER_CONFIG_STATE_SHIFT             = 9;
const uint32_t P_ROC_DRIVER_CONFIG_UPDATE_SHIFT            = 10;
const uint32_t P_ROC_DRIVER_CONFIG_WAIT_4_1ST_SLOT_SHIFT   = 11;
const uint32_t P_ROC_DRIVER_CONFIG_TIMESLOT_SHIFT          = 16;
const uint32_t P_ROC_DRIVER_CONFIG_PATTER_ON_TIME_SHIFT    = 16;
const uint32_t P_ROC_DRIVER_CONFIG_PATTER_OFF_TIME_SHIFT   = 23;
const uint32_t P_ROC_DRIVER_CONFIG_PATTER_ENABLE_SHIFT     = 30;

const uint32_t P_ROC_DRIVER_CONFIG_TABLE_DRIVER_NUM_SHIFT = 1;

const uint32_t P_ROC_SWITCH_RULE_ADDR_DEBOUNCE_SHIFT = 11;
const uint32_t P_ROC_SWITCH_RULE_ADDR_STATE_SHIFT = 10;
const uint32_t P_ROC_SWITCH_RULE_ADDR_SWITCH_NUM_SHIFT = 2;

const uint32_t P_ROC_SWITCH_RULE_NOTIFY_HOST_SHIFT = 23;
const uint32_t P_ROC_SWITCH_RULE_LINK_ACTIVE_SHIFT = 10;
const uint32_t P_ROC_SWITCH_RULE_LINK_ADDRESS_SHIFT = 11;
const uint32_t P_ROC_SWITCH_RULE_CHANGE_OUTPUT_SHIFT = 9;
const uint32_t P_ROC_SWITCH_RULE_DRIVER_NUM_SHIFT    = 0;

const uint32_t P_ROC_DMD_NUM_COLUMNS_SHIFT           = 0;
const uint32_t P_ROC_DMD_NUM_ROWS_SHIFT              = 8;
const uint32_t P_ROC_DMD_NUM_SHADES_SHIFT            = 16;
const uint32_t P_ROC_DMD_CYCLES_PER_ROW_SHIFT        = 21;
const uint32_t P_ROC_DMD_ENABLE_SHIFT                = 31;

const uint32_t P_ROC_DMD_DOTCLK_HALF_PERIOD_SHIFT    = 0;
const uint32_t P_ROC_DMD_DE_HIGH_CYCLES_SHIFT        = 8;
const uint32_t P_ROC_DMD_LATCH_HIGH_CYCLES_SHIFT     = 16;
const uint32_t P_ROC_DMD_RCLK_LOW_CYCLES_SHIFT       = 24;

const uint32_t P_ROC_DMD_DOT_TABLE_BASE_ADDR         = 0x1000;



uint32_t CreateRegRequestWord( uint32_t select, uint32_t addr, uint32_t num_words);
uint32_t CreateBurstCommand ( uint32_t select, uint32_t addr, uint32_t num_words);
int32_t CreateDriverUpdateGlobalConfigBurst ( uint32_t * burst, PRDriverGlobalConfig *driver_globals);
int32_t CreateDriverUpdateGroupConfigBurst ( uint32_t * burst, PRDriverGroupConfig *driver_group);
int32_t CreateDriverUpdateBurst ( uint32_t * burst, PRDriverState *driver);
int32_t CreateSwitchesUpdateRulesBurst ( uint32_t * burst, PRSwitchRule *rule_record);
int32_t CreateDMDUpdateGlobalConfigBurst ( uint32_t * burst, PRDMDConfig *dmd_config);

#endif // _PROC_HARDWARE_H_
