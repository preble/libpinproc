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
 *  PRDevice.h
 *  libpinproc
 */
#ifndef PINPROC_PRDEVICE_H
#define PINPROC_PRDEVICE_H
#if !defined(__GNUC__) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)	// GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "pinproc.h"
#include "PRCommon.h"
#include "PRHardware.h"
#include <queue>

using namespace std;

#define maxDriverGroups (26)
#define maxDrivers (256)
#define maxSwitchRules (256<<2) // 8 bits of switchNum indicies plus bits for debounced and state.
#define maxWriteWords (1536) // Hardware supports 2048 word bursts, but restrict to 1536 for margin.

class PRDevice
{
public:
    static PRDevice *Create(PRMachineType machineType);
    ~PRDevice();
    PRResult Reset(uint32_t resetFlags);
protected:
    PRDevice(PRMachineType machineType);

public:
    // public libpinproc API:
    int GetEvents(PREvent *events, int maxEvents);

    PRResult FlushWriteData();
    PRResult WriteDataRaw(uint32_t moduleSelect, uint32_t startingAddr, int32_t numWriteWords, uint32_t * buffer);
    PRResult WriteDataRawUnbuffered(uint32_t moduleSelect, uint32_t startingAddr, int32_t numWriteWords, uint32_t * buffer);
    PRResult ReadDataRaw(uint32_t moduleSelect, uint32_t startingAddr, int32_t numReadWords, uint32_t * readBuffer);

    PRResult ManagerUpdateConfig(PRManagerConfig *managerConfig);

    PRResult DriverUpdateGlobalConfig(PRDriverGlobalConfig *driverGlobalConfig);
    PRResult DriverGetGroupConfig(uint8_t groupNum, PRDriverGroupConfig *driverGroupConfig);
    PRResult DriverUpdateGroupConfig(PRDriverGroupConfig *driverGroupConfig);
    PRResult DriverGetState(uint8_t driverNum, PRDriverState *driverState);
    PRResult DriverUpdateState(PRDriverState *driverState);
    PRResult DriverLoadMachineTypeDefaults(PRMachineType machineType, uint32_t resetFlags = kPRResetFlagDefault);
    PRResult DriverAuxSendCommands( PRDriverAuxCommand *commands, uint8_t numCommands, uint8_t startingAddr);
    PRResult DriverWatchdogTickle();

    PRResult SwitchUpdateConfig(PRSwitchConfig *switchConfig);
    PRResult SwitchUpdateRule(uint8_t switchNum, PREventType eventType, PRSwitchRule *rule, PRDriverState *linkedDrivers, int numDrivers, bool_t drive_outputs_now);
    PRResult SwitchGetStates(PREventType * switchStates, uint16_t numSwitches);

    PRResult DMDUpdateConfig(PRDMDConfig *dmdConfig);
    PRResult DMDDraw(uint8_t * dots);

    PRResult PRJTAGDriveOutputs(PRJTAGOutputs * jtagOutputs, bool_t toggleClk);
    PRResult PRJTAGWriteTDOMemory(uint16_t tableOffset, uint16_t numWords, uint32_t * tdoData);
    PRResult PRJTAGShiftTDOData(uint16_t numBits, bool_t dataBlockComplete);
    PRResult PRJTAGReadTDIMemory(uint16_t tableOffset, uint16_t numWords, uint32_t * tdiData);
    PRResult PRJTAGGetStatus(PRJTAGStatus * status);

    PRResult PRLEDColor(PRLED * pLED, uint8_t color);
    PRResult PRLEDFade(PRLED * pLED, uint8_t fadeColor, uint16_t fadeRate);
    PRResult PRLEDFadeColor(PRLED * pLED, uint8_t fadeColor);
    PRResult PRLEDFadeRate(uint8_t boardAddr, uint16_t fadeRate);
    PRResult PRLEDRGBColor(PRLEDRGB * pLED, uint32_t color);
    PRResult PRLEDRGBFade(PRLEDRGB * pLED, uint32_t fadeColor, uint16_t fadeRate);
    PRResult PRLEDRGBFadeColor(PRLEDRGB * pLED, uint32_t fadeColor);

    int GetVersionInfo(uint16_t *verPtr, uint16_t *revPtr, uint32_t *combinedPtr);

protected:

    // Device I/O

    PRResult Open();
    PRResult Close();

    PRResult VerifyChipID();
    PRMachineType GetReadMachineType();

    // Raw write and read methods
    //

    /** Schedules data to be written to the P-ROC.  */
    PRResult PrepareWriteData(uint32_t * buffer, int32_t numWords);

    /** Writes data to the P-ROC immediately. */
    PRResult WriteData(uint32_t * buffer, int32_t numWords);

    /**
     * Reads data from the buffer that was previously collected by CollectReadData().
     * Returns the number of bytes read.
     */
    int32_t ReadData(uint32_t *buffer, int32_t maxWords);

    // Collection of methods to get data returning from the P-ROC
    /**
     * Request a block of data from the P-ROC.
     */
    PRResult RequestData(uint32_t module_select, uint32_t start_addr, int32_t num_words);
    /**
     * Actually reads the data off of the FTDI chip.
     * This is called by SortReturningData() in order to get some data to process.
     */
    int32_t CollectReadData();
    /**
     * Processes data into unrequestedDataQueue and requestedDataQueue.
     * Calls CollectReadData() to obtain the data and then uses ReadData() to read it out.
     */
    PRResult SortReturningData();
    /**
     * Empties out the read buffer.
     * Calls CollectReadData() and then ReadData() until it's empty.
     */
    PRResult FlushReadBuffer();

    queue<uint32_t> unrequestedDataQueue; /**< Queue of words received from the device that were not requested via RequestData().  Usually switch events. */
    queue<uint32_t> requestedDataQueue; /**< Queue of words received from the device as the result of a call to RequestData(). */

    uint16_t version;
    uint16_t revision;
    uint32_t chip_id;
    uint32_t combinedVersionRevision;
    /**
     * Calculated combined Version/Revision number.
     */
    int CalcCombinedVerRevision();

    uint32_t preparedWriteWords[maxWriteWords];
    int32_t numPreparedWriteWords;

    uint8_t collected_bytes_fifo[FTDI_BUFFER_SIZE];
    int32_t collected_bytes_rd_addr;
    int32_t collected_bytes_wr_addr;
    int32_t num_collected_bytes;

    uint8_t wr_buffer[16384];
    uint8_t collect_buffer[FTDI_BUFFER_SIZE];
    PRMachineType readMachineType;


    // Local Device State
    PRMachineType machineType;
    PRManagerConfig managerConfig;
    PRDriverGlobalConfig driverGlobalConfig;
    PRDriverGroupConfig driverGroups[maxDriverGroups];
    PRDriverState drivers[maxDrivers];
    PRDMDConfig dmdConfig;

    PRSwitchConfig switchConfig;
    PRSwitchRuleInternal switchRules[maxSwitchRules];
	queue<uint32_t> freeSwitchRuleIndexes; /**< Indexes of available switch rules. */
    PRSwitchRuleInternal *GetSwitchRuleByIndex(uint16_t index);
};

#endif	/* PINPROC_PRDEVICE_H */
