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
    const int asciiSegments[] = {0b0000000000000000,  // ' '
                                 0b0000000000000000,  // '!'
                                 0b0000000000000000,  // '"'
                                 0b0000000000000000,  // '#'
                                 0b0000000000000000,  // '$'
                                 0b0000000000000000,  // '%'
                                 0b0000000000000000,  // '&'
                                 0b0000001000000000,  // '''
                                 0b0001010000000000,  // '('
                                 0b0100000100000000,  // ')'
                                 0b0111111101000000,  // '*'
                                 0b0010101001000000,  // '+'
                                 0b1000000010000000,  // ','
                                 0b0000100001000000,  // '-'
                                 0b1000000000000000,  // '.'
                                 0b0100010000000000,  // '/'

                                 0b0000000000111111,  // '0'
                                 0b0000000000000110,  // '1'
                                 0b0000100001011011,  // '2'
                                 0b0000100001001111,  // '3'
                                 0b0000100001100110,  // '4'
                                 0b0000100001101101,  // '5'
                                 0b0000100001111101,  // '6'
                                 0b0000000000000111,  // '7'
                                 0b0000100001111111,  // '8'
                                 0b0000100001101111,  // '9'

                                 0b0000000000000000,  // '1'
                                 0b0000000000000000,  // '1'
                                 0b0000000000000000,  // '1'
                                 0b0000000000000000,  // '1'
                                 0b0000000000000000,  // '1'
                                 0b0000000000000000,  // '1'
                                 0b0000000000000000,  // '1'

                                 0b0000100001110111,  // 'A'
                                 0b0010101001001111,  // 'B'
                                 0b0000000000111001,  // 'C'
                                 0b0010001000001111,  // 'D'
                                 0b0000100001111001,  // 'E'
                                 0b0000100001110001,  // 'F'
                                 0b0000100000111101,  // 'G'
                                 0b0000100001110110,  // 'H'
                                 0b0010001000001001,  // 'I'
                                 0b0000000000011110,  // 'J'
                                 0b0001010001110000,  // 'K'
                                 0b0000000000111000,  // 'L'
                                 0b0000010100110110,  // 'M'
                                 0b0001000100110110,  // 'N'
                                 0b0000000000111111,  // 'O'
                                 0b0000100001110011,  // 'P'
                                 0b0001000000111111,  // 'Q'
                                 0b0001000001110011,  // 'R'
                                 0b0000100001101101,  // 'S'
                                 0b0010001000000001,  // 'T'
                                 0b0000000000111110,  // 'U'
                                 0b0100010000110000,  // 'V'
                                 0b0101000000110110,  // 'W'
                                 0b0101010100000000,  // 'X'
                                 0b0010010100000000,  // 'Y'
                                 0b0100010000001001   // 'Z'
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
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), i, 0, DIS_STB, false);

        char_a = string_1[i]; 
        char_b = string_2[i]; 

        segs_a = asciiSegments[char_a - 32];
        segs_b = asciiSegments[char_b - 32];

        printf("\nASCII Chars and associated segment values: %d", i);
        printf("\nchar_a: %x, segs_a: %x", char_a, segs_a);
        printf("\nchar_b: %x, segs_b: %x", char_b, segs_b);

        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), segs_a & 0xff, 0, STB_1, false);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), (segs_a >> 8) & 0xff, 0, STB_2, false);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), segs_b & 0xff, 0, STB_3, false);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), (segs_b >> 8) & 0xff, 0, STB_4, false);

        PRDriverAuxPrepareDelay(&auxCommands[cmd_index++],350);

        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_1, false);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_2, false);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_3, false);
        PRDriverAuxPrepareOutput(&(auxCommands[cmd_index++]), 0, 0, STB_4, false);

        PRDriverAuxPrepareDelay(&auxCommands[cmd_index++],40);
    }
    
    PRDriverAuxPrepareJump(&auxCommands[cmd_index++],1);

    printf("\nAux commands being sent:");
    for (i=0; i<cmd_index; i++) {
        printf("\nCommand: %d\tdata: %8x\tenables: %4d\tdelay_time: %10d\tjumpAddr: %4d",auxCommands[i].command, auxCommands[i].data, auxCommands[i].enables, auxCommands[i].delayTime, auxCommands[i].jumpAddr);
    }

    // Send the commands.
    PRDriverAuxSendCommands(proc, auxCommands, cmd_index, 0);

    cmd_index = 0;
    // Jump from addr 0 to 1 to begin.
    PRDriverAuxPrepareJump(&auxCommands[cmd_index++],1);
    PRDriverAuxSendCommands(proc, auxCommands, cmd_index, 0);

    printf("\nAux commands being sent:");
    for (i=0; i<cmd_index; i++) {
        printf("\nCommand: %d\tdata: %x\tenables: %d\tdelay_time: %d\tjumpAddr: %d",auxCommands[i].command, auxCommands[i].data, auxCommands[i].enables, auxCommands[i].delayTime, auxCommands[i].jumpAddr);
    }
    
} 

// Display a simple pattern to verify DMD functionality.
// 16 diagonal lines will rotate to the right.  Every two rows will get brighter, 
// starting with dim dots at the top.
void UpdateAlphaDisplay(PRHandle proc, int counter)
{

    char string_1a[] = "P-ROC CAN DRIVE ";
    char string_1b[] = "ALPHANUMERICS   ";

    display(proc, string_1a, string_1b);
}

