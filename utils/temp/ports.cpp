/*******************************************************/
/* file: ports.c                                       */
/* abstract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/* Revisions:                                          */
/* 12/01/2008:  Same code as before (original v5.01).  */
/*              Updated comments to clarify instructions.*/
/*              Add print in setPort for xapp058_example.exe.*/
/*******************************************************/
#include "ports.h"
/*#include "prgispx.h"*/

#include "stdio.h"
extern FILE *in;
static int  g_iTCK = 0; /* For xapp058_example .exe */
static int  g_iTMS = 0; /* For xapp058_example .exe */
static int  g_iTDI = 0; /* For xapp058_example .exe */

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "../../include/pinproc.h" // Include libpinproc's header.
extern PRHandle proc;

/* setPort:  Implement to set the named JTAG signal (p) to the new value (v).*/
/* if in debugging mode, then just set the variables */
void setPort(short p,short val)
{
    PRJTAGOutputs jtagOutputs;
    /* Printing code for the xapp058_example.exe.  You must set the specified
       JTAG signal (p) to the new value (v).  See the above, old Win95 code
       as an implementation example. */
    if (p==TMS)
        g_iTMS = val;
    if (p==TDI)
        g_iTDI = val;
    if (p==TCK) {
        g_iTCK = val;
    }
    uint32_t buffer[1];
    
    // Set up the data and mask bits depending on which bit is being changed.
    jtagOutputs.tckMask = p==TCK;
    jtagOutputs.tdoMask = p==TDI;
    jtagOutputs.tmsMask = p==TMS;
    jtagOutputs.tck = g_iTCK;
    jtagOutputs.tdo = g_iTDI;
    jtagOutputs.tms = g_iTMS;

    //if (p==TMS) buffer[0] = 0xC3000010 | g_iTMS;
    //if (p==TDI) buffer[0] = 0xC3000020 | g_iTDI << 1;
    //if (p==TCK) buffer[0] = 0xC3000040 | g_iTCK << 2;
    //PRWriteData(proc, 1, 0, 1, buffer);
    //const uint8_t tckMask = p==TCK;
    //const uint8_t tdoMask = p==TDI;
    //const uint8_t tmsMask = p==TMS;
    PRJTAGDriveOutputs(proc, &jtagOutputs, false);
}


// Toggle TCK.
void pulseClock()
{
    setPort(TCK,0);  /* set the TCK port to low  */
    setPort(TCK,1);  /* set the TCK port to high */
}


void readByte(unsigned char *data)
{
    // read in a byte of data from the xsvf file
    *data   = (unsigned char)fgetc( in );
}

unsigned char readTDOBit()
{
    PRJTAGStatus jtagStatus;

    // Read the TDO bit.
    // The JTAG module's status register contains the TDO bit. (It's TDI from the 
    // perspective of the JTAG module.)
    PRJTAGGetStatus(proc, &jtagStatus);

    if (jtagStatus.tdi) return ( (unsigned char) 1 );
    return( (unsigned char) 0 );
}

void waitTime(long microsec)
{
    long i;
    PRJTAGStatus jtagStatus;
    // Estimation: each PRReadData request takes approx 2ms.
    // Always do at least one.  This means the minimum delay time is approx 2ms.
    // This makes it impossible for a USB write before and after the delay
    // to be squeezed together by USB burst logic.

    // Use the system sleep() if needing a 50ms delay or more due to timing 
    // error of PRReadData loops adding up.
    if ( microsec >= 50000L )
    {
        // Make sure TCK is low during wait for XC18V00/XCFxxS 
        setPort( TCK, 0 );
        
        // Read the JTAG status register to exercise the USB bus
        PRJTAGGetStatus(proc, &jtagStatus);

        sleep( ( microsec - 2000L ) / 1000000L);
    }
    else    /* Satisfy FPGA JTAG configuration, startup TCK cycles */
    {
    
        setPort( TCK, 0 );

        // Always do at least 1 cycle.  So minimum delay time is approx 2 ms.
        for ( i = 0; i < (microsec-1)/2000 + 1;  ++i )
        {
          PRJTAGGetStatus(proc, &jtagStatus);
        }
        //{
        //sleep( ( microsec + 19999L ) / 1000000L );
        //sleep( 1 );
            //pulseClock();
        //}
    }
}
