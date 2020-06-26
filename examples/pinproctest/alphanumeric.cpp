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

void display(PRHandle proc, char * string_1, char * string_2)
{
    // Start at ASCII table offset 32: ' ' 
    const int asciiSegments[] = {0x0000,  // ' '
                                 0x0000,  // '!'
                                 0x0000,  // '"'
                                 0x0000,  // '#'
                                 0x0000,  // '$'
                                 0x0000,  // '%'
                                 0x0000,  // '&'
                                 0x0200,  // '''
                                 0x1400,  // '('
                                 0x4100,  // ')'
                                 0x7f40,  // '*'
                                 0x2a40,  // '+'
                                 0x8080,  // ','
                                 0x0840,  // '-'
                                 0x8000,  // '.'
                                 0x4400,  // '/'

                                 0x003f,  // '0'
                                 0x0006,  // '1'
                                 0x085b,  // '2'
                                 0x084f,  // '3'
                                 0x0866,  // '4'
                                 0x086d,  // '5'
                                 0x087d,  // '6'
                                 0x0007,  // '7'
                                 0x087f,  // '8'
                                 0x086f,  // '9'

                                 0x0000,  // '1'
                                 0x0000,  // '1'
                                 0x0000,  // '1'
                                 0x0000,  // '1'
                                 0x0000,  // '1'
                                 0x0000,  // '1'
                                 0x0000,  // '1'

                                 0x0877,  // 'A'
                                 0x2a4f,  // 'B'
                                 0x0039,  // 'C'
                                 0x220f,  // 'D'
                                 0x0879,  // 'E'
                                 0x0871,  // 'F'
                                 0x083d,  // 'G'
                                 0x0876,  // 'H'
                                 0x2209,  // 'I'
                                 0x001e,  // 'J'
                                 0x1470,  // 'K'
                                 0x0038,  // 'L'
                                 0x0536,  // 'M'
                                 0x1136,  // 'N'
                                 0x003f,  // 'O'
                                 0x0873,  // 'P'
                                 0x103f,  // 'Q'
                                 0x1873,  // 'R'
                                 0x086d,  // 'S'
                                 0x2201,  // 'T'
                                 0x003e,  // 'U'
                                 0x4430,  // 'V'
                                 0x5036,  // 'W'
                                 0x5500,  // 'X'
                                 0x2500,  // 'Y'
                                 0x4409   // 'Z'
                                };

    const int DIS_STB = 8;
    const int STB_1   = 9;
    const int STB_2   = 10;
    const int STB_3   = 11;
    const int STB_4   = 12;

    int i, cmd_index=0;
    char char_a, char_b; 
    int segs_a, segs_b;

    PRDriverAuxCommand auxCommands[256];

    // Disable the first entry so the Aux logic won't begin immediately.
    PRDriverAuxPrepareDisable(&auxCommands[cmd_index++]);

    for (i=0; i<16; i++) {
        // Assert the STB line
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), i, 0, DIS_STB, false, 0);

        char_a = string_1[i]; 
        char_b = string_2[i]; 

        segs_a = asciiSegments[char_a - 32];
        segs_b = asciiSegments[char_b - 32];

        printf("ASCII Chars and associated segment values: %d\n", i);
        printf("char_a: %x, segs_a: %x\n", char_a, segs_a);
        printf("char_b: %x, segs_b: %x\n", char_b, segs_b);

        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), segs_a & 0xff, 0, STB_1, false, 0);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), (segs_a >> 8) & 0xff, 0, STB_2, false, 0);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), segs_b & 0xff, 0, STB_3, false, 0);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), (segs_b >> 8) & 0xff, 0, STB_4, false, 0);

        PRDriverAuxPrepareDelay(&auxCommands[cmd_index++],350);

        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_1, false, 0);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_2, false, 0);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_3, false, 0);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_4, false, 0);

        PRDriverAuxPrepareDelay(&auxCommands[cmd_index++],40);
    }
    
    PRDriverAuxPrepareJump(&auxCommands[cmd_index++],1);

    printf("Aux commands being sent:\n");
    for (i=0; i<cmd_index; i++) {
        printf("Command: %d\tdata: %8x\tenables: %4d\tdelay_time: %10d\tjumpAddr: %4d\n",
               auxCommands[i].command, auxCommands[i].data, auxCommands[i].enables,
               auxCommands[i].delayTime, auxCommands[i].jumpAddr);
    }

    // Send the commands.
    PRDriverAuxSendCommands(proc, auxCommands, cmd_index, 0);

    cmd_index = 0;
    // Jump from addr 0 to 1 to begin.
    PRDriverAuxPrepareJump(&auxCommands[cmd_index++],1);
    PRDriverAuxSendCommands(proc, auxCommands, cmd_index, 0);

    printf("Aux commands being sent:\n");
    for (i=0; i<cmd_index; i++) {
        printf("Command: %d\tdata: %x\tenables: %d\tdelay_time: %d\tjumpAddr: %d\n",
                auxCommands[i].command, auxCommands[i].data, auxCommands[i].enables,
                auxCommands[i].delayTime, auxCommands[i].jumpAddr);
    }
    
} 

// Display a simple pattern to verify DMD functionality.
// 16 diagonal lines will rotate to the right.  Every two rows will get brighter, 
// starting with dim dots at the top.
void UpdateAlphaDisplay(PRHandle proc, int counter)
{

    char string_1a[] = "     P-ROC      ";
    char string_1b[] = "    V1.17 D3    ";

    display(proc, string_1a, string_1b);
}

