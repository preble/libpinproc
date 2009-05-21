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
 *  PRDevice.cpp
 *  libpinproc
 */

#include "PRDevice.h"

PRDevice::PRDevice(PRMachineType machineType) : machineType(machineType), ftdiInitialized(false)
{
    Reset();
}

PRDevice::~PRDevice()
{
    Close();
}

PRDevice* PRDevice::Create(PRMachineType machineType)
{
    PRDevice *dev = new PRDevice(machineType);

    if (dev == NULL)
    {
        DEBUG(PRLog("Error allocating memory for P-ROC device\n"));
        return NULL;
    }

    if (!dev->Open())
    {
        DEBUG(PRLog("Error opening P-ROC device.\n"));
        delete dev;
        return NULL;
    }

    dev->Reset();

    return dev;
}

void PRDevice::Reset()
{
    bool defaultPolarity = machineType != kPRMachineWPC;
    int i;
    memset(&driverGlobalConfig, 0x00, sizeof(PRDriverGlobalConfig));
    for (i = 0; i < maxDrivers; i++)
    {
        PRDriverState *driver = &drivers[i];
        memset(driver, 0x00, sizeof(PRDriverState));
        driver->driverNum = i;
        driver->polarity = defaultPolarity;
    }
    for (i = 0; i < maxDriverGroups; i++)
    {
        PRDriverGroupConfig *group = &driverGroups[i];
        memset(group, 0x00, sizeof(PRDriverGroupConfig));
        group->groupNum = i;
        group->polarity = defaultPolarity;
    }
    for (i = 0; i < maxSwitchRules; i++)
    {
        PRSwitchRuleInternal *switchRule = &switchRules[i];
        memset(switchRule, 0x00, sizeof(PRSwitchRule));
        uint32_t addr = (i << P_ROC_SWITCH_RULE_ADDR_SWITCH_NUM_SHIFT);
        ParseSwitchAddress(addr, &switchRule->switchNum, &switchRule->eventType);
        switchRule->driver.polarity = defaultPolarity;
        if (switchRule->switchNum >= kPRSwitchVirtualFirst && switchRule->switchNum <= kPRSwitchVirtualLast)
            freeSwitchRules.push(addr);
    }

    unrequestedDataQueue.empty();
    requestedDataQueue.empty();
    num_collected_bytes = 0;

    // TODO: Assign defaults based on machineType.  Some may have already been done above.
}


int PRDevice::GetEvents(PREvent *events, int maxEvents)
{
    SortReturningData();

    // The unrequestedDataQueue only has unrequested switch event data.  Pop
    // events out 1 at a time, interpret them, and populate the outgoing list with them.
    int i;
    for (i = 0; (i < maxEvents) && !unrequestedDataQueue.empty(); i++)
    {
        uint32_t event_data = unrequestedDataQueue.front();
        unrequestedDataQueue.pop();

        events[i].value = event_data & P_ROC_EVENT_SWITCH_NUM_MASK;
        bool open = (event_data & P_ROC_EVENT_SWITCH_STATE_MASK) >> P_ROC_EVENT_SWITCH_STATE_SHIFT;
        bool debounced = (event_data & P_ROC_EVENT_SWITCH_DEBOUNCED_MASK) >> P_ROC_EVENT_SWITCH_DEBOUNCED_SHIFT;
        if (open)
            events[i].type = debounced ? kPREventTypeSwitchOpenDebounced : kPREventTypeSwitchOpenNondebounced;
        else
            events[i].type = debounced ? kPREventTypeSwitchClosedDebounced : kPREventTypeSwitchOpenNondebounced;
    }
    return i;
}


PRResult PRDevice::DriverUpdateGlobalConfig(PRDriverGlobalConfig *driverGlobalConfig)
{
    const int burstWords = 2;
    uint32_t burst[burstWords];
    int32_t rc;

    DEBUG(PRLog("Installing driver globals\n"));

    this->driverGlobalConfig = *driverGlobalConfig;
    rc = CreateDriverUpdateGlobalConfigBurst(burst, driverGlobalConfig);

    DEBUG(PRLog("Words: %x %x\n", burst[0], burst[1]));
    return WriteData(burst, burstWords);
}

PRResult PRDevice::DriverGetGroupConfig(uint8_t groupNum, PRDriverGroupConfig *driverGroupConfig)
{
    *driverGroupConfig = driverGroups[groupNum];
    return kPRSuccess;
}

PRResult PRDevice::DriverUpdateGroupConfig(PRDriverGroupConfig *driverGroupConfig)
{
    const int burstWords = 2;
    uint32_t burst[burstWords];
    int32_t rc;

    driverGroups[driverGroupConfig->groupNum] = *driverGroupConfig;
    DEBUG(PRLog("Installing driver group\n"));
    rc = CreateDriverUpdateGroupConfigBurst(burst, driverGroupConfig);

    DEBUG(PRLog("Words: %x %x\n", burst[0], burst[1]));
    return WriteData(burst, burstWords);
}

PRResult PRDevice::DriverGetState(uint8_t driverNum, PRDriverState *driverState)
{
    *driverState = drivers[driverNum];
    return kPRSuccess;
}

PRResult PRDevice::DriverUpdateState(PRDriverState *driverState)
{
    const int burstWords = 3;
    uint32_t burst[burstWords];
    int32_t rc;

    DEBUG(PRLog("Updating driver #%d\n", driverState->driverNum));

    if (driverState->polarity != drivers[driverState->driverNum].polarity && machineType != kPRMachineCustom)
    {
        DEBUG(PRLog("Refusing to update driver #%d; polarity differs on non-custom machine.\n", driverState->driverNum));
        return kPRFailure;
    }

    drivers[driverState->driverNum] = *driverState;

    rc = CreateDriverUpdateBurst(burst, &drivers[driverState->driverNum]);
    DEBUG(PRLog("Words: %x %x %x\n", burst[0], burst[1], burst[2]));

    return WriteData(burst, burstWords);
}




PRSwitchRuleInternal *PRDevice::GetSwitchRuleByAddress(uint32_t addr)
{
    return &switchRules[addr>>P_ROC_SWITCH_RULE_ADDR_SWITCH_NUM_SHIFT];
}

PRResult PRDevice::SwitchesUpdateRule(uint8_t switchNum, PREventType eventType, PRSwitchRule *rule, PRDriverState *linkedDrivers, int numDrivers)
{
    // Updates a single rule with the associated linked driver state changes.
    const int burstSize = 4;
    uint32_t burst[burstSize];
    
    if (switchNum > kPRSwitchPhysicalLast) // Always true due to data type.
    {
        DEBUG(PRLog("Switch rule out of range 0-%d\n", kPRSwitchPhysicalLast));
        return kPRFailure;
    }
    if (freeSwitchRules.size() < numDrivers-1) // -1 because the first switch rule holds the first driver.
    {
        DEBUG(PRLog("Not enough free switch rules: %d available, need %d\n", freeSwitchRules.size(), numDrivers));
        return kPRFailure;
    }
    
    PRResult res = kPRSuccess;
    uint32_t newRuleAddr = CreateSwitchRuleAddr(switchNum, eventType);
    
    // First we need to check the linked rule to see if the indicated switchNum has any rules that need to be freed:
    PRSwitchRuleInternal *oldRule = GetSwitchRuleByAddress(newRuleAddr);
    while (oldRule->linkActive)
    {
        oldRule = GetSwitchRuleByAddress(oldRule->linkAddress);
        freeSwitchRules.push(oldRule->linkAddress);
    }
    
    // Now let's setup the first actual rule:
    uint32_t firstRuleAddr = newRuleAddr;
    PRSwitchRuleInternal *newRule = GetSwitchRuleByAddress(newRuleAddr);
    if (newRule->eventType != eventType)
        DEBUG(PRLog("Unexpected state: switch rule at 0x%x has event type 0x%x (expected 0x%x).\n", newRuleAddr, newRule->eventType, eventType));
    newRule->notifyHost = rule->notifyHost;
    newRule->changeOutput = false;
    newRule->linkActive = false;
    
    while (numDrivers > 0)
    {
        newRule->changeOutput = true;
        newRule->driver = linkedDrivers[0];
        
        if (numDrivers > 1)
        {
            newRule->linkActive = true;
            newRule->linkAddress = freeSwitchRules.front();
            freeSwitchRules.pop();
            
            CreateSwitchesUpdateRulesBurst(burst, newRule);
            
            // Prepare for the next rule:
            newRule = GetSwitchRuleByAddress(newRule->linkAddress);
        }
        else
        {
            newRule->linkActive = false;
            CreateSwitchesUpdateRulesBurst(burst, newRule);
        }
        
        // Write the actual rule:
        res = WriteData(burst, burstSize);
        if (res != kPRSuccess)
        {
            DEBUG(PRLog("Error while writing switch update, attempting to revert switch rule to a safe state..."));
            newRule = GetSwitchRuleByAddress(firstRuleAddr);
            newRule->changeOutput = false;
            newRule->linkActive = false;
            CreateSwitchesUpdateRulesBurst(burst, newRule);
            if (WriteData(burst, burstSize) == kPRSuccess)
                DEBUG(PRLog("Disabled successfully.\n"));
            else
                DEBUG(PRLog("Failed to disable.\n"));
            return res;
        }
        
        linkedDrivers++;
        numDrivers--;
    }
    
    return res;
}

int32_t PRDevice::DMDUpdateGlobalConfig(PRDMDGlobalConfig *dmdGlobalConfig)
{
    uint32_t rc;
    uint32_t burst[10];

    CreateDMDUpdateGlobalConfigBurst(burst, dmdGlobalConfig);

    DEBUG(PRLog("DMD config packet: "));
    for (int i=0; i<10; i++) {
        DEBUG(PRLog("%d ", burst[i]));
    }
    DEBUG(PRLog("\n"));

    rc = WriteData(burst, 9);
    return rc;
}


PRResult PRDevice::DMDDraw(uint8_t * dots, uint16_t columns, uint8_t rows, uint8_t numSubFrames)
{
    int32_t k; //i,x,y,j,k,m;
    //uint8_t color;
    uint16_t words_per_sub_frame = (columns*rows) / 32;
    uint16_t words_per_frame = words_per_sub_frame * numSubFrames;
    uint32_t dmd_command_buffer[1024];
    uint32_t * p_dmd_frame_buffer_words;

    p_dmd_frame_buffer_words = (uint32_t *)dots;

    dmd_command_buffer[0] = CreateBurstCommand(P_ROC_BUS_DMD_SELECT, P_ROC_DMD_DOT_TABLE_BASE_ADDR, words_per_frame);
    for (k=0; k<words_per_frame; k++) {
        dmd_command_buffer[k+1] = p_dmd_frame_buffer_words[k];
    }

    return WriteData(dmd_command_buffer, words_per_frame+1);

    // The following code prints out the init lines for the 4 Xilinx BlockRAMs
    // in the FPGA.  It's used to make an image for the P-ROC to display on power-up.
    //if (print_dots) {
    //print_dots = false;

    //for (i=0; i<4; i++) {
    //  std::cout << "For memory: "<< i << "\n";
    //  // Need 4 lines to get 1 frame (4*256*4 = 4096)
    //  // The rest will be all 0.
    //  for (y=0; y<4; y++) {
    //    std::cout << "defparam blockram.INIT_00 = 256'b";
    //    for (j=31; j>=0; j--) {
    //      for (x=7; x>=0; x--) {
    //        std::cout << ((dmd_frame_buffer[(y*32)+j] >> ((i*8)+x)) & 1);
    //      }
    //    }
    //    std::cout << ";\n";
    //  }
    //  std::cout << "\n\n\n";
    //}
    //}
}



/////////////////////////////////////////////////////////////////////////////////////////////
// Device I/O

PRResult PRDevice::Open()
{
    int32_t i=0;
    PRResult rc;
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128];
    uint32_t temp_word;

    ftdiInitialized = false;

    // Open the FTDI device
    if (ftdi_init(&ftdic) != 0)
    {
        DEBUG(PRLog("Failed to initialize FTDI.\n"));
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
        DEBUG(PRLog("ftdi_usb_find_all failed: %d: %s\n", numDevices, ftdi_get_error_string(&ftdic)));
        ftdi_deinit(&ftdic);
        return kPRFailure;
    }
    else {
        DEBUG(PRLog("Number of FTDI devices found: %d\n", numDevices));

        for (curdev = devlist; curdev != NULL; i++) {
            DEBUG(PRLog("Checking device %d\n", i));
            if ((rc = (int32_t)ftdi_usb_get_strings(&ftdic, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0) {
                DEBUG(PRLog("  ftdi_usb_get_strings failed: %d: %s\n", rc, ftdi_get_error_string(&ftdic)));
            }
            else {
                DEBUG(PRLog("  Device #%d:\n", i));
                DEBUG(PRLog("  Manufacturer: %s\n", manufacturer));
                DEBUG(PRLog("  Description: %s\n", description));
            }
            curdev = curdev->next;
        }

    }

    // Don't need the device list anymore
    ftdi_list_free (&devlist);
    // Did previous logic leave ftdic clean? Probably
    // Need to de-init and re-init before opening usb?  Doubtful.
    //ftdi_deinit(&ftdic);
    //ftdi_init(&ftdic);


    if ((rc = (int32_t)ftdi_usb_open(&ftdic, FTDI_VENDOR_ID, FTDI_FT245RL_PRODUCT_ID)) < 0)
    {
        DEBUG(PRLog("ERROR: Unable to open ftdi device: %d: %s\n", rc, ftdi_get_error_string(&ftdic)));
        return kPRFailure;
    }
    else
    {
        rc = kPRSuccess;
        if (ftdic.type == TYPE_R) {
            uint32_t chipid;
            ftdi_read_chipid(&ftdic,&chipid);
            DEBUG(PRLog("FTDI chip_id = 0x%x\n", chipid));

            // Try to verify the P-ROC IS in the FPGA before initializing the FPGA's FTDI interface
            // just in case it was already initialized from a previous application execution.
            DEBUG(PRLog("Verifying P-ROC ID: \n"));
            if (VerifyChipID() == kPRFailure) {
                // Since the FPGA didn't appear to be responding properly, send the FPGA's FTDI
                // initialization sequence.  This is a set of bytes the FPGA is waiting to receive
                // before it allows access deeper into the chip.  This keeps garbage from getting
                // in and wreaking havoc before software is up and running.
                DEBUG(PRLog("Initializing P-ROC...\n"));
                rc = FlushReadBuffer();
                temp_word = P_ROC_INIT_PATTERN_A;
                rc = WriteData(&temp_word, 1);
                temp_word = P_ROC_INIT_PATTERN_B;
                rc = WriteData(&temp_word, 1);
                rc = VerifyChipID();
            }
            else
            {
                DEBUG(PRLog("Failed to verify chip ID."));
                rc = kPRFailure;
            }
        }
    }

    if (rc == kPRSuccess)
        ftdiInitialized = true;

    return rc;
}

PRResult PRDevice::Close()
{
    // TODO: Add protection against closing a not-open ftdic.
    if (ftdiInitialized)
    {
        ftdi_usb_close(&ftdic);
        ftdi_deinit(&ftdic);
    }
    return kPRSuccess;
}

PRResult PRDevice::VerifyChipID()
{
    PRResult rc;
    const int bufferWords = 5;
    uint32_t buffer[bufferWords];
    //uint32_t temp_word;
    uint32_t max_count;

    //std::cout << "Requesting FPGA Chip ID: ";
    rc = RequestData(P_ROC_MANAGER_SELECT, P_ROC_REG_CHIP_ID_ADDR, 4);

    max_count = 0;
    //std::cout << "Waiting for read data ";
    while (num_collected_bytes < (bufferWords*4) && max_count < 10) {
        sleep(.01);
        //std::cout << ". ";
        rc = CollectReadData();
        max_count++;
    }
    //std::cout << "\n";

    if (max_count != 10) {
        int wordsRead = ReadData(buffer, bufferWords);

        if (wordsRead == 5) {
            //std::cout << rc << " words read.  \n"
            DEBUG(PRLog("FPGA Chip ID: 0x%x\n", buffer[1]));
            DEBUG(PRLog("FPGA Chip Version/Rev: %d.%d\n", buffer[2] >> 16, buffer[2] & 0xffff));
            DEBUG(PRLog("Watchdog Settings: 0x%x\n", buffer[3]));
            DEBUG(PRLog("Switches: 0x%x\n", buffer[4]));
            rc = kPRSuccess;
        }
        else {
            DEBUG(PRLog("Error reading Chip IP and Version.  Incorrect number of bytes received from read_data().\n"));
            rc = kPRFailure;
        }
    }
    else {
        DEBUG(PRLog("Unable to read Chip ID - P-ROC not yet initialized.\n"));
        rc = kPRFailure;
    }
    return (rc);
}

PRResult PRDevice::RequestData(uint32_t module_select, uint32_t start_addr, int32_t num_words)
{
    uint32_t requestWord = CreateRegRequestWord(module_select, start_addr, num_words);
    return WriteData(&requestWord, 1);
}

PRResult PRDevice::WriteData(uint32_t * words, int32_t numWords)
{
    int32_t j,k;
//  int32_t item;

    if (numWords == 0)
        return kPRSuccess;

    // The 32-bit words coming in are in the same byte order they need to be in the P-ROC.
    // However, due to Intel endian-ness, simply casting the words to 4 bytes changes the
    // byte order.  So, the conversion to bytes is done here manually to preserve the byte
    // order.  Might want to add a parameter for endian-ness at some point to make it
    // work on big endian architectures.
    for (j = 0; j < numWords; j++) {
        uint32_t temp_word = words[j];
        for (k = 3; k >= 0; k--)
        {
            wr_buffer[(j*4)+k] = (uint8_t)(temp_word & 0x000000ff);
            temp_word = temp_word >> 8;
        }
//      for (k=0; k<4; k++)
//      {
//          item = wr_buffer[(j*4)+k];
//      }
    }

    int bytesToWrite = numWords * 4;
    int bytesWritten = (int32_t)ftdi_write_data(&ftdic, wr_buffer, bytesToWrite);

    if (bytesWritten != bytesToWrite)
    {
        DEBUG(PRLog("Error in WriteData: wrote %d of %d bytes\n", bytesWritten, bytesToWrite));
        return kPRFailure;
    }
    else
    {
        return kPRSuccess;
    }
}


int32_t PRDevice::ReadData(uint32_t *buffer, int32_t num_words)
{
    int32_t rc,i,j;

    // Just like in the write_data method, the bytes are ordered here manually to put
    // them in the right order.  They are pulled from the collected_bytes_fifo 1 at a time
    // and stuffed into 32-bit words, high byte to low byte.
    if ((num_words * 4) <= num_collected_bytes) {
        for (j=0; j<num_words; j++) {
            // Initialize buffer position
            buffer[j] = 0;
            for (i=0; i<4; i++) {
                buffer[j] = (collected_bytes_fifo[collected_bytes_rd_addr] << (24-(i*8))) |
                buffer[j];
                if (collected_bytes_rd_addr == (FTDI_BUFFER_SIZE-1))
                    collected_bytes_rd_addr = 0;
                else
                    collected_bytes_rd_addr++;
            }
        }
        num_collected_bytes -= (num_words * 4);

        rc = num_words;
    }
    else {
        rc = 0;
    }
    DEBUG(PRLog("Read num bytes: %d\n", rc));
    return (rc);
}

PRResult PRDevice::FlushReadBuffer()
{
    int32_t numBytes,rc,k;
    uint32_t rd_buffer[3];
    numBytes = CollectReadData();
    k = 0;
    //std::cout << "Flushing rd buffer of " << num_words << "words\n";
    while (k < numBytes) {
        rc = ReadData(rd_buffer, 1);
        k++;
    }
    return rc;
}

int32_t PRDevice::CollectReadData()
{
    int32_t rc,i;
    rc = ftdi_read_data(&ftdic, collect_buffer, FTDI_BUFFER_SIZE-num_collected_bytes);
    for (i=0; i<rc; i=i++) {
        collected_bytes_fifo[collected_bytes_wr_addr] = collect_buffer[i];
        if (collected_bytes_wr_addr == (FTDI_BUFFER_SIZE-1))
            collected_bytes_wr_addr = 0;
        else
            collected_bytes_wr_addr++;
    }
    num_collected_bytes += rc;
    if (rc > 0)
    {
        DEBUG(PRLog("Collected bytes: %d\n", rc));
    }
    return (rc);
}

PRResult PRDevice::SortReturningData()
{
    uint32_t num_bytes, num_words, rc;
    uint32_t rd_buffer[512];

    num_bytes = CollectReadData();
    num_words = num_collected_bytes/4;

    while (num_words >= 2) {
        rc = ReadData(rd_buffer, 1);
        DEBUG(PRLog("New returning word: 0x%x\n", rd_buffer[0]));

        switch ( (rd_buffer[0] & P_ROC_COMMAND_MASK) >> P_ROC_COMMAND_SHIFT)
        {
                // Must be a bug in the P-ROC.  Unrequested packets are returning looking
                // like requested packets.  Commenting out requested packets for now.
            case P_ROC_REQUESTED_DATA: {
                int bytesRead = ReadData(rd_buffer,
                                         (rd_buffer[0] & P_ROC_HEADER_LENGTH_MASK) >>
                                         P_ROC_HEADER_LENGTH_SHIFT);
                for (int i = 0; i < bytesRead; i++)
                    requestedDataQueue.push(rd_buffer[i]);
                break;
            }
            case P_ROC_UNREQUESTED_DATA: {
                ReadData(rd_buffer,1);
                DEBUG(PRLog("Pushing onto unreq Q 0x%x\n", rd_buffer[0]));
                unrequestedDataQueue.push(rd_buffer[0]);
                break;
            }
        }
        num_words = num_collected_bytes/4;
    }
    return kPRSuccess;
}
