#ifndef KEYBOARD_ASCII_H
#define KEYBOARD_ASCII_H

#define KEYBOARD_BUF_PORT   (0x60)

/* 用转义字符定义部分控制字符 */
#define ESC		    ('\x1b')
#define BACKSPACE	('\b')
#define TAB		    ('\t')
#define ENTER		('\r')
#define DELETE		('\x7f')	

/* 以上不可见字符一律定义为0 */
#define INVISIBLE	        0
#define CTRL_L_CHAR        INVISIBLE
#define CTRL_R_CHAR        INVISIBLE
#define SHIFT_L_CHAR       INVISIBLE
#define SHIFT_R_CHAR       INVISIBLE
#define ALT_L_CHAR         INVISIBLE
#define ALT_R_CHAR         INVISIBLE
#define CAPS_LOCK_CHAR     INVISIBLE

/* 定义控制字符的通码和断码 */
#define SHIFT_L_MAKE       0x2A
#define SHIFT_R_MAKE       0x36
#define ALT_L_MAKE         0x38
#define ALT_R_MAKE         0xE038
#define ALT_R_BREAK        0xE0B8
#define CTRL_L_MAKE        0x1D
#define CTRL_R_MAKE        0xE01D
#define CTRL_R_BREAK       0xE09D
#define CAPS_LOCK_MAKE     0x3A

#endif