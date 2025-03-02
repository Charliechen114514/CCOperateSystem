#ifndef KEYBOARD_MAPPINGS_H
#define KEYBOARD_MAPPINGS_H
#include "include/device/configs/keyboard_ascii.h"

// Key mappings table
static char keymap[][2] = {
    /* 扫描码   未与shift组合  与shift组合*/
    /* ---------------------------------- */
    /* 0x00 */	{0,	0},		
    /* 0x01 */	{ESC,	ESC},		
    /* 0x02 */	{'1',	'!'},		
    /* 0x03 */	{'2',	'@'},		
    /* 0x04 */	{'3',	'#'},		
    /* 0x05 */	{'4',	'$'},		
    /* 0x06 */	{'5',	'%'},		
    /* 0x07 */	{'6',	'^'},		
    /* 0x08 */	{'7',	'&'},		
    /* 0x09 */	{'8',	'*'},		
    /* 0x0A */	{'9',	'('},		
    /* 0x0B */	{'0',	')'},		
    /* 0x0C */	{'-',	'_'},		
    /* 0x0D */	{'=',	'+'},		
    /* 0x0E */	{BACKSPACE, BACKSPACE},	
    /* 0x0F */	{TAB,	TAB},		
    /* 0x10 */	{'q',	'Q'},		
    /* 0x11 */	{'w',	'W'},		
    /* 0x12 */	{'e',	'E'},		
    /* 0x13 */	{'r',	'R'},		
    /* 0x14 */	{'t',	'T'},		
    /* 0x15 */	{'y',	'Y'},		
    /* 0x16 */	{'u',	'U'},		
    /* 0x17 */	{'i',	'I'},		
    /* 0x18 */	{'o',	'O'},		
    /* 0x19 */	{'p',	'P'},		
    /* 0x1A */	{'[',	'{'},		
    /* 0x1B */	{']',	'}'},		
    /* 0x1C */	{ENTER,  ENTER},
    /* 0x1D */	{CTRL_L_CHAR, CTRL_L_CHAR},
    /* 0x1E */	{'a',	'A'},		
    /* 0x1F */	{'s',	'S'},		
    /* 0x20 */	{'d',	'D'},		
    /* 0x21 */	{'f',	'F'},		
    /* 0x22 */	{'g',	'G'},		
    /* 0x23 */	{'h',	'H'},		
    /* 0x24 */	{'j',	'J'},		
    /* 0x25 */	{'k',	'K'},		
    /* 0x26 */	{'l',	'L'},		
    /* 0x27 */	{';',	':'},		
    /* 0x28 */	{'\'',	'"'},		
    /* 0x29 */	{'`',	'~'},		
    /* 0x2A */	{SHIFT_L_CHAR, SHIFT_L_CHAR},	
    /* 0x2B */	{'\\',	'|'},		
    /* 0x2C */	{'z',	'Z'},		
    /* 0x2D */	{'x',	'X'},		
    /* 0x2E */	{'c',	'C'},		
    /* 0x2F */	{'v',	'V'},		
    /* 0x30 */	{'b',	'B'},		
    /* 0x31 */	{'n',	'N'},		
    /* 0x32 */	{'m',	'M'},		
    /* 0x33 */	{',',	'<'},		
    /* 0x34 */	{'.',	'>'},		
    /* 0x35 */	{'/',	'?'},
    /* 0x36	*/	{SHIFT_R_CHAR, SHIFT_R_CHAR},	
    /* 0x37 */	{'*',	'*'},    	
    /* 0x38 */	{ALT_L_CHAR, ALT_L_CHAR},
    /* 0x39 */	{' ',	' '},		
    /* 0x3A */	{CAPS_LOCK_CHAR, CAPS_LOCK_CHAR}
};

#endif