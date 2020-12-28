/*
 * Copyright (c) 3009 Gerry Stellenberg, Adam Preble
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

// This is an example configuration for a custom machine showing
// the steps to configure drivers.
// Custom machine devs will almost certainly need to use different settings
// tailored to their hardware setup.

// For this example, we're using the following driver group configuration:
// Group 0  (Drivers 0-7)     : Unused
// Group 1  (Drivers 8-15)    : Unused
// Group 2  (Drivers 16-23)   : Unused
// Group 3  (Drivers 24-31)   : Unused
// Group 4  (Drivers 32-39)   : Coils 0-7
// Group 5  (Drivers 40-47)   : Coils 8-15
// Group 6  (Drivers 48-55)   : Coils 16-23
// Group 7  (Drivers 56-63)   : Coils 24-31
// Group 8  (Drivers 64-71)   : Coils 32-39
// Group 9  (Drivers 72-79)   : Coils 40-47
// Group 10 (Drivers 80-87)   : Matrix 0 0-7
// Group 11 (Drivers 88-95)   : Matrix 0 8-15
// Group 12 (Drivers 96-103)  : Matrix 0 16-23
// Group 13 (Drivers 104-111) : Matrix 0 24-31
// Group 14 (Drivers 112-119) : Matrix 0 32-39
// Group 15 (Drivers 120-127) : Matrix 0 40-47
// Group 16 (Drivers 128-135) : Matrix 0 48-55
// Group 17 (Drivers 136-143) : Matrix 0 56-63
// Group 18 (Drivers 144-151) : Matrix 1 0-7
// Group 19 (Drivers 152-159) : Matrix 1 8-15
// Group 20 (Drivers 160-167) : Matrix 1 16-23
// Group 21 (Drivers 168-175) : Matrix 1 24-31
// Group 22 (Drivers 176-183) : Matrix 1 32-39
// Group 23 (Drivers 184-191) : Matrix 1 40-47
// Group 24 (Drivers 192-199) : Matrix 1 48-55
// Group 25 (Drivers 300-207) : Matrix 1 56-63

// The Driver Board setup is the following:
// Address 0 : Sink-16
// Address 1 : Sink-16
// Address 2 : Sink-16
// Address 3 : Source-sink-8
// Address 4 : Source-sink-8


    // First create individual driver records for each driver, and reset them to
    // zero except for the polarity, which is set according to driverPolarity from
    // above.  
void InitializeDrivers(PRHandle proc, bool driverPolarity)
{
    int i;
    for (i = 0; i < kPRDriverCount; i++)
    {
        PRDriverState driver;
        memset(&driver, 0x00, sizeof(PRDriverState));
        driver.driverNum = i;
        driver.polarity = driverPolarity;
        PRDriverUpdateState(proc, &driver);
    }
}

void ConfigureDriverGroups(PRHandle proc, bool driverPolarity)
{
    int i;

    // Each entry in the mappedDriverGroupEnableIndex is the enable line that 
    // will be asserted when the data belonging to the group is serviced.  
    // The enable line maps to a Driver Board and bank.  
    // Bits [3:1] of the index represent the Driver
    // Board address, and bit [0] represents the bank (A vs B).  For example,
    // Enable 0 maps to Board 0, Bank A.  Enable 1 maps to Board 0, Bank B.
    // Enable 2 maps to Board 1, Bank A.  Etc, etc.

    // The groups being serviced correspond directly to the P-ROC's drivers.
    // Group 0 is for drivers 0-7 group 1 is for drivers 15-8, and so on.
    // Groups 0-3 correspond to the P-ROC's direct driver outputs.  
    // The remaining groups correspond to the P-ROC's multiplexed drivers,
    // and are therefore the drivers used on existing P/D boards and now
    // on the new Driver Boards.  These start at group 4 (drivers 32-39).
    // Therefore, the enable indexes given to groups 0-3 are all 0 in
    // this case and don't matter.  Group 4 is also 0, meaning drivers
    // 32-39 map to Driver Board 0, Bank A.

    // Notice groups 10-17 all use index 7, which maps to Driver Board 3,
    // Bank B.  This means that drivers 80-143 all use Driver Board 3,
    // Bank B.  In the example setup, this is a Source-Sink-8 Board where
    // Bank B is the row outputs.  Similarly, groups 18-25, or drivers
    // 144-207, go to index 9 or Board 4, Bank B.  Board 4 is also a
    // source-sink-8 board.
    const int mappedDriverGroupEnableIndex[] = {0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 7, 7, 7, 7, 7, 7, 7, 7, 9, 9, 9, 9, 9, 9, 9, 9};

    // Slow times are used to allow matrix groups to turn on the matrix lamp or led.
    // They're in microseconds.  Here, all matrixed groups turn on for 300us.
    const int mappedDriverGroupSlowTime[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300};

    // GroupActivateIndexes indicate which column to strobe while a matrix group
    // is being serviced.  For instance, group 10 is 0, so data bit [0] is asserted
    // for the matrix column when drivers 80-87 are driven onto the rows of the 
    // matrix.  Next, group 11 is 1, so data bit [1] represents the active column
    // when drivers 88-95 are driven onto the Rows.  How these columns map to the
    // Driver Boards is determined by the matrixRowEnableIndex below.
    const int mappedDriverGroupActivateIndex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};

    // There are 2 matrixRowEnableIndexes because the P-ROC can control 2 separate
    // matrixes, and in fact this example uses 2 matrixes.  This next
    // variable mappedDriverGroupRowEnableSelect specifies which of the 2 indexes
    // to use when the groups are serviced.  Groups 10-17 use index 0, and 
    // groups 18-25 use index 1.  The value of indexes 0 and 1 is defined below
    // in the matrixRowEnableIndex variables.
    const int mappedDriverGroupRowEnableSelect[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
    
    for (i = 0; i < kPRDriverGroupsMax; i++)
    {
        PRDriverGroupConfig group;
        //memset(&group, 0x00, sizeof(PRDriverGroupConfig));

        group.groupNum = i;

        group.slowTime = mappedDriverGroupSlowTime[i];
        group.enableIndex = mappedDriverGroupEnableIndex[i];
        group.rowActivateIndex = mappedDriverGroupActivateIndex[i];
        group.rowEnableSelect = mappedDriverGroupRowEnableSelect[i];

        // All of the groups with slow times are in matrixes.
        group.matrixed = mappedDriverGroupSlowTime[i] != 0;

        group.polarity = driverPolarity;

        // Not using groups 0-3 in this example because we're not using the direct
        // drivers (drivers 0-31).  So, start activating the groups at group 4.
        group.active = i >= 4;

        // Matrix rows should be disabled after they are driven so they don't
        // overlap into the next row.
        group.disableStrobeAfter = mappedDriverGroupSlowTime[i] != 0;
        
        PRDriverUpdateGroupConfig(proc, &group);
    }
}

void ConfigureDriverGlobals(PRHandle proc, bool driverPolarity)
{
    PRDriverGlobalConfig globals;

    // Set up the watchdog time for when the global settings are programmed.  1000
    // means the drivers will disable automatically if the watchdog isn't updated
    // for 1 second (1000ms).  This could happen if software crashes or if the USB
    // cable is unplugged.
    const int watchdogResetTime = 1000; // milliseconds

    // Start with outputs disabled so the P-ROC can initialize all of its
    // logic without actually driving anything.
    globals.enableOutputs = false;
    globals.globalPolarity = driverPolarity;
    globals.useClear = false;
    globals.strobeStartSelect = false;

    // startStrobeTime is obselete.  It used to mean cycle through the drivers
    // every x ms.  Now the logic loops as fast as it can, which is as fast
    // as the combination of all of the slowTime combined.
    globals.startStrobeTime = 1; 

    // Here's where the enable lines are set for the matrix columns.  
    // All of the groups in Matrix 0 were configured to use index 0, which
    // is set to 6 here.  6 maps to Driver Board 3, Bank A, which is the 
    // source bank on the source-sink-8 board.
    // All of the groups in Matrix 1 were configured to use index 1, which
    // is set to 8 here.  8 maps to Driver Board 4, Bank A, which is the 
    // source bank on the source-sink-8 board.
    globals.matrixRowEnableIndex0 = 6;
    globals.matrixRowEnableIndex1 = 8;

    // Choose the polarity for the Column strobes.  PDBs are active high.
    // So rows should NOT be active low.
    globals.activeLowMatrixRows = false;

    // Not using the Stern P/D board.  So disable the Stern tickle logic.
    globals.tickleSternWatchdog = false;

    // Encoding enables isn't entirely necessary since we're not using
    // the multiplexed data/enables bus, but it seems nice that the encoded
    // enables map to the Driver Board addresses/banks.
    globals.encodeEnables = true;

    // Enable and reset the watchdog logic.
    globals.watchdogExpired = false;
    globals.watchdogEnable = true;
    globals.watchdogResetTime = watchdogResetTime;
    
    // Send these globals to the P-ROC.  Remember, we aren't enabling the outputs
    // yet.  As soon as this command goes through, the P-ROC will initialize its
    // driver logic.
    PRDriverUpdateGlobalConfig(proc, &globals);
    
    // Now that the P-ROC driver logic is initialized, enable the outputs and 
    // resend the command.
    globals.enableOutputs = true;
    PRDriverUpdateGlobalConfig(proc, &globals);
}
    
void ConfigureDrivers(PRHandle proc)
{
    // First set up a bunch of constants to use later:

    // The driverPolarity determines when the drivers go high or low when
    // they are supposed to be active.  PDBs are active high.  So
    // this should be true.
    const bool driverPolarity = true;

    // Now start actually programming thing in the P-ROC.

    // First reset each individual driver.
    InitializeDrivers(proc, driverPolarity);

    // Now configure all of the groups.  
    ConfigureDriverGroups(proc, driverPolarity);

    // Now take care of the globals.
    ConfigureDriverGlobals(proc, driverPolarity);

    // The P-ROC should now be configured to use our chain of Driver Boards. 
    // You can now change the outputs by issuing the normal PRDriver commands,
    // like:

    // Pulse driver 40 for 30ms.  Driver 40 is in group 5, which we
    // configured to go to enableIndex 1, which is Board 0, bank B.
    // PRDriverPulse(proc, 40, 30);  Pulse driver 40 for 30ms.  

    // Disable driver 32, which we configured to be on Board 0, Bank A.
    // PRDriverDisable(proc, 32); 

    // Other commands like PRDriverSchedule, PRDriverPatter, 
    // PRDriverPulsedPatter, etc will all work too.

    // Note - Another way to control Driver Board outputs is to write
    // to them directly.  Using this scheme, configuring the individual
    // drivers and groups, as shown above, is unnecessary.
    // Here's how to write directly to the Driver Boards:

    // uint8_t command = 0x1; // Command 1 is Write
    // uint8_t brd_addr = 0x2;
    // uint8_t reg_addr = 0x0; // 0 for Bank A, 1 for Bank B
    // uint8_t reg_data = 0xa5; // Turn on every other driver.
    // uint32_t data= (command << 24) | (brd_addr << 16) | 
    //                (reg_addr << 8) | reg_data;
    
    // Now issue the write to Module 3, address 0xc00.  These 
    // hardcoded values map to the P-ROC's serial bus output.  So
    // don't change them.
    // PRWriteData(proc, 3, 0xc00, 1, &data);
}

