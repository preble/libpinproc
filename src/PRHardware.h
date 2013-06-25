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
const uint32_t P_ROC_INIT_PATTERN_A         = 0x801F1122;
const uint32_t P_ROC_INIT_PATTERN_B         = 0x345678AB;
const uint32_t P_ROC_CHIP_ID                = 0xfeedbeef;
const uint32_t P3_ROC_CHIP_ID                = 0xf33db33f;

const uint32_t P_ROC_VER_REV_FIXED_SWITCH_STATE_READS = 0x10013; // 1.19

const uint32_t P_ROC_AUTO_STERN_DETECT_SHIFT     = 8;
const uint32_t P_ROC_AUTO_STERN_DETECT_MASK      = 0x00000100;
const uint32_t P_ROC_AUTO_STERN_DETECT_VALUE     = 0x1;
const uint32_t P_ROC_MANUAL_STERN_DETECT_SHIFT   = 0;
const uint32_t P_ROC_MANUAL_STERN_DETECT_MASK    = 0x00000001;
const uint32_t P_ROC_MANUAL_STERN_DETECT_VALUE   = 0x00000000;
const uint32_t P_ROC_BOARD_VERSION_SHIFT    = 7;
const uint32_t P_ROC_BOARD_VERSION_MASK     = 0x00000080;

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
const uint32_t P_ROC_BUS_JTAG_SELECT              = 1;
const uint32_t P_ROC_BUS_SWITCH_CTRL_SELECT       = 2;
const uint32_t P_ROC_BUS_DRIVER_CTRL_SELECT       = 3;
const uint32_t P_ROC_BUS_STATE_CHANGE_PROC_SELECT = 4;
const uint32_t P_ROC_BUS_DMD_SELECT               = 5;
const uint32_t P_ROC_BUS_UNASSOCIATED_SELECT      = 15;

const uint32_t P3_ROC_MANAGER_SELECT               = 0;
const uint32_t P3_ROC_BUS_SPI_SELECT               = 1;
const uint32_t P3_ROC_BUS_SWITCH_CTRL_SELECT       = 2;
const uint32_t P3_ROC_BUS_DRIVER_CTRL_SELECT       = 3;
const uint32_t P3_ROC_BUS_STATE_CHANGE_PROC_SELECT = 4;
const uint32_t P3_ROC_BUS_AUX_CTRL_SELECT          = 5;
const uint32_t P3_ROC_BUS_ACCELEROMETER_SELECT     = 6;
const uint32_t P3_ROC_BUS_I2C_SELECT               = 7;
const uint32_t P3_ROC_BUS_UNASSOCIATED_SELECT      = 15;

const uint32_t P_ROC_REG_CHIP_ID_ADDR             = 0;
const uint32_t P_ROC_REG_VERSION_ADDR             = 1;
const uint32_t P_ROC_REG_WATCHDOG_ADDR            = 2;
const uint32_t P_ROC_REG_DIPSWITCH_ADDR           = 3;

const uint32_t P_ROC_MANAGER_WATCHDOG_EXPIRED_SHIFT        = 30;
const uint32_t P_ROC_MANAGER_WATCHDOG_ENABLE_SHIFT         = 14;
const uint32_t P_ROC_MANAGER_WATCHDOG_RESET_TIME_SHIFT     = 0;
const uint32_t P_ROC_MANAGER_REUSE_DMD_DATA_FOR_AUX_SHIFT  = 10;
const uint32_t P_ROC_MANAGER_INVERT_DIPSWITCH_1_SHIFT      = 9;

const uint32_t P3_ROC_SPI_OPCODE_SHIFT            = 24;

const uint32_t P3_ROC_SPI_OPCODE_WR_ENABLE        = 0;
const uint32_t P3_ROC_SPI_OPCODE_WR_DISABLE       = 1;
const uint32_t P3_ROC_SPI_OPCODE_RD_ID            = 2;
const uint32_t P3_ROC_SPI_OPCODE_RD_STATUS        = 3;
const uint32_t P3_ROC_SPI_OPCODE_WR_STATUS        = 4;
const uint32_t P3_ROC_SPI_OPCODE_RD_DATA          = 5;
const uint32_t P3_ROC_SPI_OPCODE_FRD_DATA         = 6;
const uint32_t P3_ROC_SPI_OPCODE_PP               = 7;
const uint32_t P3_ROC_SPI_OPCODE_SECTOR_ERASE     = 8;
const uint32_t P3_ROC_SPI_OPCODE_BULK_ERASE       = 9;
const uint32_t P3_ROC_SPI_OPCODE_DEEP_POWERDN     = 10;
const uint32_t P3_ROC_SPI_OPCODE_RELEASE          = 11;

const uint32_t P_ROC_JTAG_SHIFT_EXIT_SHIFT                 = 16;
const uint32_t P_ROC_JTAG_SHIFT_NUM_BITS_SHIFT             = 0;

const uint32_t P_ROC_JTAG_CMD_CHANGE_STATE                 = 0;
const uint32_t P_ROC_JTAG_CMD_SHIFT                        = 1;
const uint32_t P_ROC_JTAG_CMD_TRANSITION                   = 2;
const uint32_t P_ROC_JTAG_CMD_SET_PORTS                    = 3;

const uint32_t P_ROC_JTAG_CMD_START_SHIFT                  = 31;
const uint32_t P_ROC_JTAG_CMD_OE_SHIFT                     = 30;
const uint32_t P_ROC_JTAG_CMD_CMD_SHIFT                    = 24;

const uint32_t P_ROC_JTAG_TRANSITION_TCK_MASK_SHIFT        = 6;
const uint32_t P_ROC_JTAG_TRANSITION_TDO_MASK_SHIFT        = 5;
const uint32_t P_ROC_JTAG_TRANSITION_TMS_MASK_SHIFT        = 4;
const uint32_t P_ROC_JTAG_TRANSITION_TCK_SHIFT             = 2;
const uint32_t P_ROC_JTAG_TRANSITION_TDO_SHIFT             = 1;
const uint32_t P_ROC_JTAG_TRANSITION_TMS_SHIFT             = 0;

const uint32_t P_ROC_JTAG_STATUS_DONE_SHIFT                = 31;
const uint32_t P_ROC_JTAG_STATUS_TDI_SHIFT                 = 16;

const uint32_t P_ROC_JTAG_COMMAND_REG_BASE_ADDR            = 0x0;
const uint32_t P_ROC_JTAG_STATUS_REG_BASE_ADDR             = 0x1;
const uint32_t P_ROC_JTAG_TDO_MEMORY_BASE_ADDR             = 0x400;
const uint32_t P_ROC_JTAG_TDI_MEMORY_BASE_ADDR             = 0x800;

const uint32_t P_ROC_SWITCH_CTRL_STATE_BASE_ADDR           = 4;
const uint32_t P_ROC_SWITCH_CTRL_OLD_DEBOUNCE_BASE_ADDR    = 11;
const uint32_t P_ROC_SWITCH_CTRL_DEBOUNCE_BASE_ADDR        = 12;

const uint32_t P_ROC_EVENT_TYPE_SWITCH                     = 0;
const uint32_t P_ROC_EVENT_TYPE_DMD                        = 1;
const uint32_t P_ROC_EVENT_TYPE_BURST_SWITCH               = 2;
const uint32_t P_ROC_EVENT_TYPE_ACCELEROMETER              = 3;

const uint32_t P_ROC_V1_EVENT_TYPE_MASK                    = 0xC00;
const uint32_t P_ROC_V1_EVENT_TYPE_SHIFT                   = 10;
const uint32_t P_ROC_V2_EVENT_TYPE_MASK                    = 0xC000;
const uint32_t P_ROC_V2_EVENT_TYPE_SHIFT                   = 14;

const uint32_t P_ROC_V1_EVENT_SWITCH_NUM_MASK              = 0xFF;
const uint32_t P_ROC_V2_EVENT_SWITCH_NUM_MASK              = 0x7FF;
const uint32_t P_ROC_V1_EVENT_SWITCH_STATE_MASK            = 0x100;
const uint32_t P_ROC_V2_EVENT_SWITCH_STATE_MASK            = 0x1000;
const uint32_t P_ROC_V1_EVENT_SWITCH_STATE_SHIFT           = 8;
const uint32_t P_ROC_V2_EVENT_SWITCH_STATE_SHIFT           = 12;
const uint32_t P_ROC_V1_EVENT_SWITCH_DEBOUNCED_MASK        = 0x200;
const uint32_t P_ROC_V2_EVENT_SWITCH_DEBOUNCED_MASK        = 0x2000;
const uint32_t P_ROC_V1_EVENT_SWITCH_DEBOUNCED_SHIFT       = 9;
const uint32_t P_ROC_V2_EVENT_SWITCH_DEBOUNCED_SHIFT       = 13;
const uint32_t P_ROC_V1_EVENT_SWITCH_TIMESTAMP_MASK        = 0xFFFFF000;
const uint32_t P_ROC_V1_EVENT_SWITCH_TIMESTAMP_SHIFT       = 12;
const uint32_t P_ROC_V2_EVENT_SWITCH_TIMESTAMP_MASK        = 0xFFFF0000;
const uint32_t P_ROC_V2_EVENT_SWITCH_TIMESTAMP_SHIFT       = 16;
const uint32_t P_ROC_V2_EVENT_ACCEL_TIMESTAMP_MASK         = 0xFFFC0000;
const uint32_t P_ROC_V2_EVENT_ACCEL_TIMESTAMP_SHIFT        = 18;


const uint32_t P_ROC_DRIVER_CTRL_DECODE_SHIFT     = 10;
const uint32_t P_ROC_DRIVER_CTRL_REG_DECODE       = 0;
const uint32_t P_ROC_DRIVER_CONFIG_TABLE_DECODE   = 1;
const uint32_t P_ROC_DRIVER_AUX_MEM_DECODE        = 2;
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

const uint32_t P_ROC_DRIVER_CONFIG_OUTPUT_DRIVE_TIME_SHIFT           = 0;
const uint32_t P_ROC_DRIVER_CONFIG_POLARITY_SHIFT                    = 8;
const uint32_t P_ROC_DRIVER_CONFIG_STATE_SHIFT                       = 9;
const uint32_t P_ROC_DRIVER_CONFIG_UPDATE_SHIFT                      = 10;
const uint32_t P_ROC_DRIVER_CONFIG_WAIT_4_1ST_SLOT_SHIFT             = 11;
const uint32_t P_ROC_DRIVER_CONFIG_TIMESLOT_SHIFT                    = 16;
const uint32_t P_ROC_DRIVER_CONFIG_PATTER_ON_TIME_SHIFT              = 16;
const uint32_t P_ROC_DRIVER_CONFIG_PATTER_OFF_TIME_SHIFT             = 23;
const uint32_t P_ROC_DRIVER_CONFIG_PATTER_ENABLE_SHIFT               = 30;
const uint32_t P_ROC_DRIVER_CONFIG_FUTURE_ENABLE_SHIFT               = 31;

const uint32_t P_ROC_DRIVER_CONFIG_TABLE_DRIVER_NUM_SHIFT = 1;

const uint32_t P_ROC_DRIVER_AUX_ENTRY_ACTIVE_SHIFT                   = 31;
const uint32_t P_ROC_DRIVER_AUX_OUTPUT_DELAY_SHIFT                   = 20;
const uint32_t P_ROC_DRIVER_AUX_OUTPUT_DELAY_MASK                    = 0x7ff;
const uint32_t P_ROC_DRIVER_AUX_MUX_ENABLES_SHIFT                    = 19;
const uint32_t P_ROC_DRIVER_AUX_COMMAND_SHIFT                        = 16;
const uint32_t P_ROC_DRIVER_AUX_COMMAND_MASK                         = 0x3;
const uint32_t P_ROC_DRIVER_AUX_ENABLES_SHIFT                        = 12;
const uint32_t P_ROC_DRIVER_AUX_ENABLES_MASK                         = 0xF;
const uint32_t P_ROC_DRIVER_AUX_EXTRA_DATA_SHIFT                     = 8;
const uint32_t P_ROC_DRIVER_AUX_EXTRA_DATA_MASK                      = 0xF;
const uint32_t P_ROC_DRIVER_AUX_DATA_SHIFT                           = 0;
const uint32_t P_ROC_DRIVER_AUX_DATA_MASK                            = 0xFF;
const uint32_t P_ROC_DRIVER_AUX_DELAY_TIME_SHIFT                     = 0;
const uint32_t P_ROC_DRIVER_AUX_DELAY_TIME_MASK                      = 0x3FFF;
const uint32_t P_ROC_DRIVER_AUX_JUMP_ADDR_SHIFT                      = 0;
const uint32_t P_ROC_DRIVER_AUX_JUMP_ADDR_MASK                       = 0xFF;

const uint32_t P_ROC_DRIVER_AUX_CMD_OUTPUT                           = 2;
const uint32_t P_ROC_DRIVER_AUX_CMD_DELAY                            = 1;
const uint32_t P_ROC_DRIVER_AUX_CMD_JUMP                             = 0;

const uint32_t P_ROC_SWITCH_CONFIG_CLEAR_SHIFT                       = 31;
const uint32_t P_ROC_SWITCH_CONFIG_USE_COLUMN_9                      = 30;
const uint32_t P_ROC_SWITCH_CONFIG_USE_COLUMN_8                      = 29;
const uint32_t P_ROC_SWITCH_CONFIG_MS_PER_DM_SCAN_LOOP_SHIFT         = 24;
const uint32_t P_ROC_SWITCH_CONFIG_PULSES_BEFORE_CHECKING_RX_SHIFT   = 18;
const uint32_t P_ROC_SWITCH_CONFIG_INACTIVE_PULSES_AFTER_BURST_SHIFT = 12;
const uint32_t P_ROC_SWITCH_CONFIG_PULSES_PER_BURST_SHIFT            = 6;
const uint32_t P_ROC_SWITCH_CONFIG_MS_PER_PULSE_HALF_PERIOD_SHIFT    = 0;

const uint32_t P_ROC_SWITCH_RULE_DRIVE_OUTPUTS_NOW    = 13;
const uint32_t P_ROC_SWITCH_RULE_NUM_DEBOUNCE_SHIFT   = 9;
const uint32_t P_ROC_SWITCH_RULE_NUM_STATE_SHIFT      = 8;
const uint32_t P_ROC_SWITCH_RULE_NUM_SWITCH_NUM_SHIFT = 0;
const uint32_t P_ROC_SWITCH_RULE_NUM_TO_ADDR_SHIFT    = 2;

const uint32_t P_ROC_SWITCH_RULE_RELOAD_ACTIVE_SHIFT   = 31;
const uint32_t P_ROC_SWITCH_RULE_NOTIFY_HOST_SHIFT     = 23;
const uint32_t P_ROC_SWITCH_RULE_LINK_ACTIVE_SHIFT     = 10;
const uint32_t P_ROC_SWITCH_RULE_LINK_ADDRESS_SHIFT    = 11;
const uint32_t P_ROC_SWITCH_RULE_CHANGE_OUTPUT_SHIFT   = 9;
const uint32_t P_ROC_SWITCH_RULE_DRIVER_NUM_SHIFT      = 0;

const uint32_t P_ROC_STATE_CHANGE_CONFIG_ADDR        = 0x1000;

const uint32_t P_ROC_DMD_NUM_COLUMNS_SHIFT           = 0;
const uint32_t P_ROC_DMD_NUM_ROWS_SHIFT              = 8;
const uint32_t P_ROC_DMD_NUM_SUB_FRAMES_SHIFT        = 16;
const uint32_t P_ROC_DMD_NUM_FRAME_BUFFERS_SHIFT     = 24;
const uint32_t P_ROC_DMD_AUTO_INC_WR_POINTER_SHIFT   = 29;
const uint32_t P_ROC_DMD_ENABLE_FRAME_EVENTS_SHIFT   = 30;
const uint32_t P_ROC_DMD_ENABLE_SHIFT                = 31;

const uint32_t P_ROC_DMD_DOTCLK_HALF_PERIOD_SHIFT    = 0;
const uint32_t P_ROC_DMD_DE_HIGH_CYCLES_SHIFT        = 6;
const uint32_t P_ROC_DMD_LATCH_HIGH_CYCLES_SHIFT     = 16;
const uint32_t P_ROC_DMD_RCLK_LOW_CYCLES_SHIFT       = 24;

const uint32_t P_ROC_DMD_DOT_TABLE_BASE_ADDR         = 0x1000;

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

PRResult PRHardwareOpen();
void PRHardwareClose();
int PRHardwareRead(uint8_t *buffer, int maxBytes);
int PRHardwareWrite(uint8_t *buffer, int bytes);

#endif /* PINPROC_PRHARDWARE_H */
