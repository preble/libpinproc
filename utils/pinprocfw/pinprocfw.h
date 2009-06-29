/*****************************************************************************
* File:         micro.h
* Description:  This header file contains the function prototype to the
*               primary interface function for the XSVF player.
* Usage:        FIRST - PORTS.C
*               Customize the ports.c function implementations to establish
*               the correct protocol for communicating with your JTAG ports
*               (setPort() and readTDOBit()) and tune the waitTime() delay
*               function.  Also, establish access to the XSVF data source
*               in the readByte() function.
*               FINALLY - Call xsvfExecute().
*****************************************************************************/
#ifndef PINPROCFW_H
#define PINPROCFW_H

/* Legacy error codes for xsvfExecute from original XSVF player v2.0 */
#define XSVF_LEGACY_SUCCESS 1
#define XSVF_LEGACY_ERROR   0

/* 4.04 [NEW] Error codes for xsvfExecute. */
/* Must #define XSVF_SUPPORT_ERRORCODES in micro.c to get these codes */
#define XSVF_ERROR_NONE         0
#define XSVF_ERROR_UNKNOWN      1
#define XSVF_ERROR_TDOMISMATCH  2
#define XSVF_ERROR_MAXRETRIES   3   /* TDO mismatch after max retries */
#define XSVF_ERROR_ILLEGALCMD   4
#define XSVF_ERROR_ILLEGALSTATE 5
#define XSVF_ERROR_DATAOVERFLOW 6   /* Data > lenVal MAX_LEN buffer size*/
/* Insert new errors here */
#define XSVF_ERROR_LAST         7

/*****************************************************************************
* Function:     xsvfExecute
* Description:  Process, interpret, and apply the XSVF commands.
*               See port.c:readByte for source of XSVF data.
* Parameters:   none.
* Returns:      int - For error codes see above.
*****************************************************************************/
extern int xsvfExecute();

/* these constants are used to send the appropriate ports to setPort */
/* they should be enumerated types, but some of the microcontroller  */
/* compilers don't like enumerated types */
#define TCK (short) 0
#define TMS (short) 1
#define TDI (short) 2

/* set the port "p" (TCK, TMS, or TDI) to val (0 or 1) */
void setPort(short p, short val);

/* read the TDO bit and store it in val */
unsigned char readTDOBit();

/* make clock go down->up->down*/
void pulseClock();

/* read the next byte of data from the xsvf file */
void readByte(unsigned char *data);

void waitTime(long microsec);


#endif  /* PINPROCFW_H */

