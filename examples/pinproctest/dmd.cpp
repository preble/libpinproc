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


void ConfigureDMD(PRHandle proc)
{
    int i;
    
    // Create the structure that holds the DMD settings
    PRDMDConfig dmdConfig;
    memset(&dmdConfig, 0x0, sizeof(dmdConfig));
    
    dmdConfig.numRows = kDMDRows;
    dmdConfig.numColumns = kDMDColumns;
    dmdConfig.numSubFrames = kDMDSubFrames;
    dmdConfig.numFrameBuffers = kDMDFrameBuffers;
    dmdConfig.autoIncBufferWrPtr = true;
    dmdConfig.enableFrameEvents = true;
    
    for (i = 0; i < kDMDSubFrames; i++)
    {
        dmdConfig.rclkLowCycles[i] = 15;
        dmdConfig.latchHighCycles[i] = 15;
        dmdConfig.dotclkHalfPeriod[i] = 1;
    }
    
    dmdConfig.deHighCycles[0] = 250;
    dmdConfig.deHighCycles[1] = 400;
    dmdConfig.deHighCycles[2] = 180;
    dmdConfig.deHighCycles[3] = 800;
    
    PRDMDUpdateConfig(proc, &dmdConfig);
}

// Display a simple pattern to verify DMD functionality.
// 16 diagonal lines will rotate to the right.  Every two rows will get brighter, 
// starting with dim dots at the top.
void UpdateDots( unsigned char * dots, unsigned int dotOffset )
{
    int row,col,subFrame,color,mappedColor,loopCtr,byte_shifter;
    const int rate_reduction_divisor = 1;
    
    loopCtr = dotOffset/rate_reduction_divisor;
    color = pow(2,kDMDSubFrames) - 1;
    byte_shifter = 0x80;
    
    // Slow it down just a tad
    if (dotOffset%rate_reduction_divisor == 0)
    {
        // Set up byte_shifter to rotate pattern to the right.
        byte_shifter = pow(2,(loopCtr%8));
        
        // Clear the DMD dots every time the rotation occurs
        memset(dots,0,((kDMDColumns*kDMDRows)/8)*kDMDSubFrames);
        
        // Loop through all of the rows
        for (row = kDMDRows - 1; row >= 0; row--)
        {
            // Map the color index to the DMD's physical color map
            int mappedColors[] = {0, 2, 8, 10, 1, 3, 9, 11, 4, 6, 12, 14, 5, 7, 13, 15};
            mappedColor = mappedColors[color];
            
            // Loop through each of 16 bytes in a row
            for (col = 0; col < kDMDColumns / 8; col++)
            {
                // Loop through each subframe
                for (subFrame = 0; subFrame < kDMDSubFrames; subFrame++)
                {
                    // Turn on the byte in each sub-frame that's enabled 
                    // active for the color code.
                    if ((mappedColor >> subFrame) & 1 == 1) 
                        dots[subFrame*(kDMDColumns*kDMDRows/8)+((row%kDMDRows)*(kDMDColumns / 8))+col] = byte_shifter;
                }
            }
            // Determine where to change the color in order to progress from row 0 = color 0
            // to the last row being the last color.
            if (row % (int)((kDMDRows/pow(2,kDMDSubFrames))) == 0) color--;
            if (byte_shifter == 1) byte_shifter = 0x80;
            else byte_shifter = byte_shifter >> 1;
        }
    }
}

