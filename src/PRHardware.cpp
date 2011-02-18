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
 *  PRHardware.cpp
 *  libpinproc
 */


#include "PRHardware.h"
#include "PRCommon.h"

bool_t IsStern (uint32_t hardware_data) {
//    if ( ((hardware_data & P_ROC_BOARD_VERSION_MASK) >> P_ROC_BOARD_VERSION_SHIFT) == 0x1) 
 //       return ( ((hardware_data & P_ROC_AUTO_STERN_DETECT_MASK) >> P_ROC_AUTO_STERN_DETECT_SHIFT) == P_ROC_AUTO_STERN_DETECT_VALUE);
//    else 
        return ( ((hardware_data & P_ROC_MANUAL_STERN_DETECT_MASK) >> P_ROC_MANUAL_STERN_DETECT_SHIFT) == P_ROC_MANUAL_STERN_DETECT_VALUE);
}

uint32_t CreateRegRequestWord( uint32_t select, uint32_t addr, uint32_t num_words ) {
    return ( (P_ROC_READ << P_ROC_COMMAND_SHIFT) |
            (num_words << P_ROC_HEADER_LENGTH_SHIFT) |
            (select << P_ROC_MODULE_SELECT_SHIFT) |
            (addr << P_ROC_ADDR_SHIFT) );
};


uint32_t CreateBurstCommand ( uint32_t select, uint32_t addr, uint32_t num_words ) {
    return ( (P_ROC_WRITE << P_ROC_COMMAND_SHIFT) |
            (num_words << P_ROC_HEADER_LENGTH_SHIFT) |
            (select << P_ROC_MODULE_SELECT_SHIFT) |
            (addr << P_ROC_ADDR_SHIFT) );
}

int32_t CreateManagerUpdateConfigBurst ( uint32_t * burst, PRManagerConfig *manager_config) {
    uint32_t addr;

    addr = P_ROC_REG_DIPSWITCH_ADDR;
    burst[0] = CreateBurstCommand (P_ROC_MANAGER_SELECT, addr, 1 );
    burst[1] = ( (manager_config->reuse_dmd_data_for_aux << 
                        P_ROC_MANAGER_REUSE_DMD_DATA_FOR_AUX_SHIFT) |
                 (manager_config->invert_dipswitch_1 << 
                        P_ROC_MANAGER_INVERT_DIPSWITCH_1_SHIFT) );
    
    return kPRSuccess;
}

int32_t CreateDriverUpdateGlobalConfigBurst ( uint32_t * burst, PRDriverGlobalConfig *driver_globals) {
    uint32_t addr;

    addr = 0;
    addr = (P_ROC_DRIVER_CTRL_REG_DECODE << P_ROC_DRIVER_CTRL_DECODE_SHIFT);

    burst[0] = CreateBurstCommand (P_ROC_BUS_DRIVER_CTRL_SELECT, addr, 1 );
    burst[1] = ( (driver_globals->enableOutputs <<
                  P_ROC_DRIVER_GLOBAL_ENABLE_DIRECT_OUTPUTS_SHIFT) |
                (driver_globals->globalPolarity <<
                 P_ROC_DRIVER_GLOBAL_GLOBAL_POLARITY_SHIFT) |
                (driver_globals->useClear <<
                 P_ROC_DRIVER_GLOBAL_USE_CLEAR_SHIFT) |
                (driver_globals->strobeStartSelect <<
                 P_ROC_DRIVER_GLOBAL_STROBE_START_SELECT_SHIFT) |
                (driver_globals->startStrobeTime <<
                 P_ROC_DRIVER_GLOBAL_START_STROBE_TIME_SHIFT) |
                (driver_globals->matrixRowEnableIndex1 <<
                 P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_1_SHIFT) |
                (driver_globals->matrixRowEnableIndex0 <<
                 P_ROC_DRIVER_GLOBAL_MATRIX_ROW_ENABLE_INDEX_0_SHIFT) |
                (driver_globals->activeLowMatrixRows <<
                 P_ROC_DRIVER_GLOBAL_ACTIVE_LOW_MATRIX_ROWS_SHIFT) |
                (driver_globals->encodeEnables <<
                 P_ROC_DRIVER_GLOBAL_ENCODE_ENABLES_SHIFT) |
                (driver_globals->tickleSternWatchdog <<
                 P_ROC_DRIVER_GLOBAL_TICKLE_WATCHDOG_SHIFT) );

    return kPRSuccess;
}

int32_t CreateDriverUpdateGroupConfigBurst ( uint32_t * burst, PRDriverGroupConfig *driver_group) {
    uint32_t addr;

    addr = 0;
    addr = (P_ROC_DRIVER_CTRL_REG_DECODE << P_ROC_DRIVER_CTRL_DECODE_SHIFT) |
    driver_group->groupNum;

    burst[0] = CreateBurstCommand (P_ROC_BUS_DRIVER_CTRL_SELECT, addr, 1 );
    burst[1] = ( (driver_group->slowTime <<
                  P_ROC_DRIVER_GROUP_SLOW_TIME_SHIFT) |
                (driver_group->disableStrobeAfter <<
                 P_ROC_DRIVER_GROUP_DISABLE_STROBE_AFTER_SHIFT) |
                (driver_group->enableIndex <<
                 P_ROC_DRIVER_GROUP_ENABLE_INDEX_SHIFT) |
                (driver_group->rowActivateIndex <<
                 P_ROC_DRIVER_GROUP_ROW_ACTIVATE_INDEX_SHIFT) |
                (driver_group->rowEnableSelect <<
                 P_ROC_DRIVER_GROUP_ROW_ENABLE_SELECT_SHIFT) |
                (driver_group->matrixed <<
                 P_ROC_DRIVER_GROUP_MATRIXED_SHIFT) |
                (driver_group->polarity <<
                 P_ROC_DRIVER_GROUP_POLARITY_SHIFT) |
                (driver_group->active <<
                 P_ROC_DRIVER_GROUP_ACTIVE_SHIFT) );
    return kPRSuccess;
}

int32_t CreateDriverUpdateBurst ( uint32_t * burst, PRDriverState *driver) {
    uint32_t addr;

    addr = 0;
    addr = (P_ROC_DRIVER_CONFIG_TABLE_DECODE << P_ROC_DRIVER_CTRL_DECODE_SHIFT) |
    (driver->driverNum << P_ROC_DRIVER_CONFIG_TABLE_DRIVER_NUM_SHIFT);

    burst[0] = CreateBurstCommand (P_ROC_BUS_DRIVER_CTRL_SELECT, addr, 2 );
    burst[1] = ( (driver->outputDriveTime << P_ROC_DRIVER_CONFIG_OUTPUT_DRIVE_TIME_SHIFT) |
                (driver->polarity << P_ROC_DRIVER_CONFIG_POLARITY_SHIFT) |
                (driver->state << P_ROC_DRIVER_CONFIG_STATE_SHIFT) |
                (1 << P_ROC_DRIVER_CONFIG_UPDATE_SHIFT) |
                (driver->waitForFirstTimeSlot << P_ROC_DRIVER_CONFIG_WAIT_4_1ST_SLOT_SHIFT) |
                (driver->timeslots << P_ROC_DRIVER_CONFIG_TIMESLOT_SHIFT) );
    burst[2] = (driver->timeslots >> P_ROC_DRIVER_CONFIG_TIMESLOT_SHIFT) |
    (driver->patterOnTime << P_ROC_DRIVER_CONFIG_PATTER_ON_TIME_SHIFT) |
    (driver->patterOffTime << P_ROC_DRIVER_CONFIG_PATTER_OFF_TIME_SHIFT) |
    (driver->patterEnable << P_ROC_DRIVER_CONFIG_PATTER_ENABLE_SHIFT);
    return kPRSuccess;
}

uint32_t CreateDriverAuxCommand ( PRDriverAuxCommand command) {
    switch (command.command) {
        case (kPRDriverAuxCmdOutput) : {
            return ((command.active & 1) << P_ROC_DRIVER_AUX_ENTRY_ACTIVE_SHIFT) |
                   ((command.delayTime & P_ROC_DRIVER_AUX_OUTPUT_DELAY_MASK) <<
                                       P_ROC_DRIVER_AUX_OUTPUT_DELAY_SHIFT) |
                   ((command.muxEnables & 1) << P_ROC_DRIVER_AUX_MUX_ENABLES_SHIFT) |
                   ((command.command & P_ROC_DRIVER_AUX_COMMAND_MASK) <<
                                       P_ROC_DRIVER_AUX_COMMAND_SHIFT) |
                   ((command.enables & P_ROC_DRIVER_AUX_ENABLES_MASK) <<
                                       P_ROC_DRIVER_AUX_ENABLES_SHIFT) |
                   ((command.extraData & P_ROC_DRIVER_AUX_EXTRA_DATA_MASK) <<
                                         P_ROC_DRIVER_AUX_EXTRA_DATA_SHIFT) |
                   ((command.data & P_ROC_DRIVER_AUX_DATA_MASK) <<
                                    P_ROC_DRIVER_AUX_DATA_SHIFT);
        }
        break;
        case (kPRDriverAuxCmdDelay) : {
            return (command.active << P_ROC_DRIVER_AUX_ENTRY_ACTIVE_SHIFT) |
                   ((command.command & P_ROC_DRIVER_AUX_COMMAND_MASK) <<
                                       P_ROC_DRIVER_AUX_COMMAND_SHIFT) |
                   ((command.delayTime & P_ROC_DRIVER_AUX_DELAY_TIME_MASK) <<
                                        P_ROC_DRIVER_AUX_DELAY_TIME_SHIFT);
        }
        break;
        case (kPRDriverAuxCmdJump) : {
            return (command.active << P_ROC_DRIVER_AUX_ENTRY_ACTIVE_SHIFT) |
                   ((command.command & P_ROC_DRIVER_AUX_COMMAND_MASK) <<
                                       P_ROC_DRIVER_AUX_COMMAND_SHIFT) |
                   ((command.jumpAddr & P_ROC_DRIVER_AUX_JUMP_ADDR_MASK) <<
                                        P_ROC_DRIVER_AUX_JUMP_ADDR_SHIFT);
        }
        break;
        default : {
            return (false << P_ROC_DRIVER_AUX_ENTRY_ACTIVE_SHIFT); 
        }
    }
}

int32_t CreateWatchdogConfigBurst ( uint32_t * burst, bool_t watchdogExpired,
                                   bool_t watchdogEnable, uint16_t watchdogResetTime) {
    uint32_t addr;
    
    addr = P_ROC_REG_WATCHDOG_ADDR;
    burst[0] = CreateBurstCommand (P_ROC_MANAGER_SELECT, addr, 1 );
    burst[1] = ( (watchdogExpired << P_ROC_MANAGER_WATCHDOG_EXPIRED_SHIFT) |
                (watchdogEnable << P_ROC_MANAGER_WATCHDOG_ENABLE_SHIFT) |
                (watchdogResetTime << P_ROC_MANAGER_WATCHDOG_RESET_TIME_SHIFT) );
    
    return kPRSuccess;
}

int32_t CreateSwitchUpdateConfigBurst ( uint32_t * burst, PRSwitchConfig *switchConfig)
{
    uint32_t addr;

    addr = 0;
    burst[0] = CreateBurstCommand (P_ROC_BUS_SWITCH_CTRL_SELECT, addr, 1 );
    burst[1] = (switchConfig->clear << P_ROC_SWITCH_CONFIG_CLEAR_SHIFT) |
               (switchConfig->directMatrixScanLoopTime << 
                   P_ROC_SWITCH_CONFIG_MS_PER_DM_SCAN_LOOP_SHIFT) |
               (switchConfig->pulsesBeforeCheckingRX << 
                   P_ROC_SWITCH_CONFIG_PULSES_BEFORE_CHECKING_RX_SHIFT) |
               (switchConfig->inactivePulsesAfterBurst << 
                   P_ROC_SWITCH_CONFIG_INACTIVE_PULSES_AFTER_BURST_SHIFT) |
               (switchConfig->pulsesPerBurst << 
                   P_ROC_SWITCH_CONFIG_PULSES_PER_BURST_SHIFT) |
               (switchConfig->pulseHalfPeriodTime << 
                   P_ROC_SWITCH_CONFIG_MS_PER_PULSE_HALF_PERIOD_SHIFT) |
               (switchConfig->use_column_8 << 
                   P_ROC_SWITCH_CONFIG_USE_COLUMN_8) |
               (switchConfig->use_column_9 << 
                   P_ROC_SWITCH_CONFIG_USE_COLUMN_9);
    burst[2] = CreateBurstCommand (P_ROC_BUS_STATE_CHANGE_PROC_SELECT, 
                                   P_ROC_STATE_CHANGE_CONFIG_ADDR, 1 );
    burst[3] = switchConfig->hostEventsEnable;

    return kPRSuccess;
}

int16_t CreateSwitchRuleIndex(uint8_t switchNum, PREventType eventType) 
{
    uint32_t debounce = (eventType == kPREventTypeSwitchOpenDebounced) || (eventType == kPREventTypeSwitchClosedDebounced) ? 1 : 0;
    uint32_t state    = (eventType == kPREventTypeSwitchOpenDebounced) || (eventType == kPREventTypeSwitchOpenNondebounced) ? 1 : 0;
	
    uint32_t index = ((debounce << P_ROC_SWITCH_RULE_NUM_DEBOUNCE_SHIFT) |
                      (state << P_ROC_SWITCH_RULE_NUM_STATE_SHIFT) |
                      (switchNum << P_ROC_SWITCH_RULE_NUM_SWITCH_NUM_SHIFT) );
    return index;
}

int32_t CreateSwitchRuleAddr(uint8_t switchNum, PREventType eventType, bool_t drive_outputs_now) 
{
    uint16_t number = CreateSwitchRuleIndex( switchNum, eventType );
    uint32_t addr = (number << P_ROC_SWITCH_RULE_NUM_TO_ADDR_SHIFT) |
                    (drive_outputs_now << P_ROC_SWITCH_RULE_DRIVE_OUTPUTS_NOW);
    return addr;
}

void ParseSwitchRuleIndex(uint16_t index, uint8_t *switchNum, PREventType *eventType)
{
    *switchNum = (index >> P_ROC_SWITCH_RULE_NUM_SWITCH_NUM_SHIFT) & 0xff;

    bool open = ((index >> P_ROC_SWITCH_RULE_NUM_STATE_SHIFT) & 0x1) == 0x1;
    bool debounce = ((index >> P_ROC_SWITCH_RULE_NUM_DEBOUNCE_SHIFT) & 0x1) == 0x1;
    if (open)
        *eventType = debounce ? kPREventTypeSwitchOpenDebounced : kPREventTypeSwitchOpenNondebounced;
    else
        *eventType = debounce ? kPREventTypeSwitchClosedDebounced : kPREventTypeSwitchClosedNondebounced;
}

int32_t CreateSwitchUpdateRulesBurst ( uint32_t * burst, PRSwitchRuleInternal *rule_record, bool_t drive_outputs_now) {
    uint32_t addr = CreateSwitchRuleAddr(rule_record->switchNum, rule_record->eventType, drive_outputs_now);
    uint32_t driver_command[3];

    CreateDriverUpdateBurst ( driver_command, &(rule_record->driver));

    burst[0] = CreateBurstCommand (P_ROC_BUS_STATE_CHANGE_PROC_SELECT, addr, 3 );
    burst[1] = driver_command[1];
    burst[2] = driver_command[2];

    burst[3] = (rule_record->changeOutput << P_ROC_SWITCH_RULE_CHANGE_OUTPUT_SHIFT) |
    (rule_record->driver.driverNum << P_ROC_SWITCH_RULE_DRIVER_NUM_SHIFT) |
    (rule_record->linkActive << P_ROC_SWITCH_RULE_LINK_ACTIVE_SHIFT) |
    (rule_record->linkIndex << P_ROC_SWITCH_RULE_LINK_ADDRESS_SHIFT) |
    (rule_record->notifyHost << P_ROC_SWITCH_RULE_NOTIFY_HOST_SHIFT) |
    (rule_record->reloadActive << P_ROC_SWITCH_RULE_RELOAD_ACTIVE_SHIFT);
    return kPRSuccess;

}

int32_t CreateDMDUpdateConfigBurst ( uint32_t * burst, PRDMDConfig *dmd_config)
{
    uint32_t addr;
    uint32_t i;

    addr = 0;
    burst[0] = CreateBurstCommand (P_ROC_BUS_DMD_SELECT, addr, 1 );
    burst[1] = (1 << P_ROC_DMD_ENABLE_SHIFT) |
               (dmd_config->enableFrameEvents << P_ROC_DMD_ENABLE_FRAME_EVENTS_SHIFT) |
               (dmd_config->autoIncBufferWrPtr << P_ROC_DMD_AUTO_INC_WR_POINTER_SHIFT) |
               (dmd_config->numFrameBuffers << P_ROC_DMD_NUM_FRAME_BUFFERS_SHIFT) |
               (dmd_config->numSubFrames << P_ROC_DMD_NUM_SUB_FRAMES_SHIFT) |
               (dmd_config->numRows << P_ROC_DMD_NUM_ROWS_SHIFT) |
               (dmd_config->numColumns << P_ROC_DMD_NUM_COLUMNS_SHIFT); 

    addr = 8;
    burst[2] = CreateBurstCommand (P_ROC_BUS_DMD_SELECT, addr, 4 );

    for (i=0; i<4; i++) {
        burst[i+3] = (dmd_config->rclkLowCycles[i] << P_ROC_DMD_RCLK_LOW_CYCLES_SHIFT) |
        (dmd_config->latchHighCycles[i] << P_ROC_DMD_LATCH_HIGH_CYCLES_SHIFT) |
        (dmd_config->deHighCycles[i] << P_ROC_DMD_DE_HIGH_CYCLES_SHIFT) |
        (dmd_config->dotclkHalfPeriod[i] << P_ROC_DMD_DOTCLK_HALF_PERIOD_SHIFT);
    }
    return kPRSuccess;
}

int32_t CreateJTAGForceOutputsBurst ( uint32_t * burst, PRJTAGOutputs *jtagOutputs) {
    burst[0] = CreateBurstCommand (P_ROC_BUS_JTAG_SELECT, P_ROC_JTAG_COMMAND_REG_BASE_ADDR, 1 );
    burst[1] = 0;
    burst[1] = 1 << P_ROC_JTAG_CMD_START_SHIFT |
               1 << P_ROC_JTAG_CMD_OE_SHIFT |
               P_ROC_JTAG_CMD_SET_PORTS << P_ROC_JTAG_CMD_CMD_SHIFT |
               jtagOutputs->tckMask << P_ROC_JTAG_TRANSITION_TCK_MASK_SHIFT | 
               jtagOutputs->tdoMask << P_ROC_JTAG_TRANSITION_TDO_MASK_SHIFT | 
               jtagOutputs->tmsMask << P_ROC_JTAG_TRANSITION_TMS_MASK_SHIFT | 
               jtagOutputs->tck << P_ROC_JTAG_TRANSITION_TCK_SHIFT | 
               jtagOutputs->tdo << P_ROC_JTAG_TRANSITION_TCK_SHIFT | 
               jtagOutputs->tms << P_ROC_JTAG_TRANSITION_TCK_SHIFT; 
    return kPRSuccess;

}
int32_t CreateJTAGLatchOutputsBurst ( uint32_t * burst, PRJTAGOutputs *jtagOutputs) {
    burst[0] = CreateBurstCommand (P_ROC_BUS_JTAG_SELECT, P_ROC_JTAG_COMMAND_REG_BASE_ADDR, 1 );
    burst[1] = 0;
    burst[1] = 1 << P_ROC_JTAG_CMD_START_SHIFT |
               1 << P_ROC_JTAG_CMD_OE_SHIFT |
               P_ROC_JTAG_CMD_TRANSITION << P_ROC_JTAG_CMD_CMD_SHIFT |
               jtagOutputs->tdoMask << P_ROC_JTAG_TRANSITION_TDO_MASK_SHIFT | 
               jtagOutputs->tmsMask << P_ROC_JTAG_TRANSITION_TMS_MASK_SHIFT | 
               jtagOutputs->tdo << P_ROC_JTAG_TRANSITION_TCK_SHIFT | 
               jtagOutputs->tms << P_ROC_JTAG_TRANSITION_TMS_SHIFT; 
    return kPRSuccess;

}
int32_t CreateJTAGShiftTDODataBurst ( uint32_t * burst, uint16_t numBits, bool_t dataBlockComplete) {
    burst[0] = CreateBurstCommand (P_ROC_BUS_JTAG_SELECT, P_ROC_JTAG_COMMAND_REG_BASE_ADDR, 1 );
    burst[1] = 0;
    burst[1] = 1 << P_ROC_JTAG_CMD_START_SHIFT |
               1 << P_ROC_JTAG_CMD_OE_SHIFT |
               P_ROC_JTAG_CMD_SHIFT << P_ROC_JTAG_CMD_CMD_SHIFT |
               dataBlockComplete << P_ROC_JTAG_SHIFT_EXIT_SHIFT |
               numBits << P_ROC_JTAG_SHIFT_NUM_BITS_SHIFT; 
    return kPRSuccess;
}

/**
 * This is where all FTDI driver-specific code should go.
 * As we add support for other drivers (such as D2xx on Windows), we will add more implementations of the PRHardware*() functions here.
 */

#if defined(__WIN32__) || defined(_WIN32)
#include "ftd2xx.h"

#define BUF_SIZE 16
#define MAX_DEVICES 1

// Globals
static FT_HANDLE ftHandles[MAX_DEVICES];
static FT_HANDLE ftHandle;

PRResult PRHardwareOpen()
{
    char 	cBufWrite[BUF_SIZE];
    char * 	pcBufLD[MAX_DEVICES + 1];
    char 	cBufLD[MAX_DEVICES][64];
    FT_STATUS	ftStatus;
    int	iNumDevs = 0;
    int	i, j;
    int	iDevicesOpen = 0;

    for(i = 0; i < MAX_DEVICES; i++) {
        pcBufLD[i] = cBufLD[i];
        ftHandles[i] = NULL;
    }
    pcBufLD[MAX_DEVICES] = NULL;
    
    ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
    
    if(ftStatus != FT_OK) {
        DEBUG(PRLog(kPRLogInfo,"Error: FT_ListDevices(%d)\n", ftStatus));
        return kPRFailure;
    }
    
    for(j = 0; j < BUF_SIZE; j++) {
        cBufWrite[j] = j;
    }
    
    for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
        DEBUG(PRLog(kPRLogInfo,"Device %d Serial Number - %s\n", i, cBufLD[i]));
    }

    for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ) ; i++) {
        /* Setup */
        if((ftStatus = FT_OpenEx(cBufLD[i], FT_OPEN_BY_SERIAL_NUMBER, &ftHandles[i])) != FT_OK){
            /* 
                This can fail if the ftdi_sio driver is loaded
                use lsmod to check this and rmmod ftdi_sio to remove
                also rmmod usbserial
            */
            DEBUG(PRLog(kPRLogInfo,"Error FT_OpenEx(%d), device\n", ftStatus, i));
            return kPRFailure;
        }
    
        DEBUG(PRLog(kPRLogInfo,"Opened device %s\n", cBufLD[i]));
        ftHandle = ftHandles[i];

        if((ftStatus = FT_SetBaudRate(ftHandles[i], 1228800)) != FT_OK) {
            DEBUG(PRLog(kPRLogInfo,"Error FT_SetBaudRate(%d), cBufLD[i] = %s\n", ftStatus, cBufLD[i]));
        }
    
        iDevicesOpen++;
    }

    if (iDevicesOpen > 0) 
    {
      FT_ResetDevice(ftHandle);
      DEBUG(PRLog(kPRLogInfo,"FTDI Device Opened\n"));
      return kPRSuccess;
    }
    else return kPRFailure;
}
    
void PRHardwareClose()
{
    int i;

    for(i = 0; i < MAX_DEVICES; i++) {
        if(ftHandles[i] != NULL) {
            FT_Close(ftHandles[i]);
            ftHandles[i] = NULL;
            DEBUG(PRLog(kPRLogInfo,"Closed device\n"));
        }
    }
}

int PRHardwareRead(uint8_t *buffer, int maxBytes)
{
    FT_STATUS ftStatus; 
    DWORD bytesToRead;
    DWORD bytesRead;
    int i;

    ftStatus = FT_GetQueueStatus(ftHandle,&bytesToRead);
    if (ftStatus != FT_OK) return 0;
   
    if ((DWORD)maxBytes < bytesToRead) bytesToRead = maxBytes;
    ftStatus = FT_Read(ftHandle, buffer, bytesToRead, &bytesRead);
    if (ftStatus == FT_OK) {
        DEBUG(PRLog(kPRLogVerbose,"Read %d bytes:\n",bytesRead));
        for (i=0; (DWORD)i<bytesRead; i++) {
            DEBUG(PRLog(kPRLogVerbose,"Read byte: %x\n",buffer[i]));
        }
        return (int)bytesRead;
    }
    else return 0;
}

int PRHardwareWrite(uint8_t *buffer, int bytes)
{
    FT_STATUS ftStatus=0; 
    DWORD bytesWritten=0;
    int i;

    DEBUG(PRLog(kPRLogVerbose,"Writing %d bytes:\n",bytes));
    ftStatus = FT_Write(ftHandle, buffer, (DWORD)bytes, &bytesWritten);
    if (ftStatus == FT_OK) 
    {
        DEBUG(PRLog(kPRLogVerbose,"Wrote %d bytes:\n",bytesWritten));
        if (bytesWritten != DWORD(bytes)) DEBUG(PRLog(kPRLogVerbose,"Wrote %d bytes, should have written %d bytes",bytesWritten,bytes));
        else {
            for (i=0; (DWORD)i<bytesWritten; i++) {
                DEBUG(PRLog(kPRLogVerbose,"Wrote byte: %x\n",buffer[i]));
            }
        }
        return (int)bytesWritten;
    }
    else return 0;
}

#else // WIN32

#include <ftdi.h>

static bool ftdiInitialized;
static ftdi_context ftdic;


PRResult PRHardwareOpen()
{
    int32_t i=0;
    PRResult rc;
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128];
    
    ftdiInitialized = false;
    
    // Open the FTDI device
    if (ftdi_init(&ftdic) != 0)
    {
        PRSetLastErrorText("Failed to initialize FTDI.");
        return kPRFailure;
    }
    
    // Find all FTDI devices
    // This is very basic and really only expects to see 1 device.  It needs to be
    // smarter.  At the very least, it should check some register on the P-ROC versus
    // an input parameter to ensure the software is set up for the same architecture as
    // the P-ROC (Stern vs WPC).  Otherwise, it's possible to drive the coils the wrong
    // polarity and blow fuses or fry transistors and all other sorts of badness.
    
    // We first enumerate all of the devices:
    int numDevices = ftdi_usb_find_all(&ftdic, &devlist, FTDI_VENDOR_ID, FTDI_FT245RL_PRODUCT_ID);
    if (numDevices < 0) {
        PRSetLastErrorText("ftdi_usb_find_all failed: %d: %s", numDevices, ftdi_get_error_string(&ftdic));
        ftdi_deinit(&ftdic);
        return kPRFailure;
    }
    else {
        DEBUG(PRLog(kPRLogInfo, "Number of FTDI devices found: %d\n", numDevices));
        
        for (curdev = devlist; curdev != NULL; i++) {
            DEBUG(PRLog(kPRLogInfo, "Checking device %d\n", i));
            if ((rc = (int32_t)ftdi_usb_get_strings(&ftdic, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0) {
                DEBUG(PRLog(kPRLogInfo, "  ftdi_usb_get_strings failed: %d: %s\n", rc, ftdi_get_error_string(&ftdic)));
            }
            else {
                DEBUG(PRLog(kPRLogInfo, "  Device #%d:\n", i));
                DEBUG(PRLog(kPRLogInfo, "  Manufacturer: %s\n", manufacturer));
                DEBUG(PRLog(kPRLogInfo, "  Description: %s\n", description));
            }
            curdev = curdev->next;
        }
        
    }
    
    // Don't need the device list anymore
    ftdi_list_free (&devlist);
    
    if ((rc = (int32_t)ftdi_usb_open(&ftdic, FTDI_VENDOR_ID, FTDI_FT245RL_PRODUCT_ID)) < 0)
    {
        PRSetLastErrorText("Unable to open ftdi device: %d: %s", rc, ftdi_get_error_string(&ftdic));
        return kPRFailure;
    }
    else
    {
        rc = kPRSuccess;
        if (ftdic.type == TYPE_R) {
            uint32_t chipid;
            ftdi_read_chipid(&ftdic,&chipid);
            DEBUG(PRLog(kPRLogInfo, "FTDI chip_id = 0x%x\n", chipid));
            // Set some defaults:
            ftdi_read_data_set_chunksize(&ftdic, 4096);
            ftdi_set_latency_timer(&ftdic, 2); // This helps make reads much faster.  16 appeared to be the default.
            ftdiInitialized = true;
            return kPRSuccess;
        }
        else
        {
            PRSetLastErrorText("FTDI type != TYPE_R: 0x%x", ftdic.type);
            return kPRFailure;
        }
    }
}
void PRHardwareClose()
{
    if (ftdiInitialized)
    {
        ftdi_usb_close(&ftdic);
        ftdi_deinit(&ftdic);
    }
}
int PRHardwareRead(uint8_t *buffer, int maxBytes)
{
    return ftdi_read_data(&ftdic, buffer, maxBytes);
}
int PRHardwareWrite(uint8_t *buffer, int bytes)
{
    return ftdi_write_data(&ftdic, buffer, bytes);
}

#endif
