#ifndef KEYBOARD_ASCII_H
#define KEYBOARD_ASCII_H

/* Port and interrupt number for keyboard input */
#define KEYBOARD_BUF_PORT       (0x60)  // I/O port for keyboard buffer
#define KEYBOARD_INTERRUPT_N    (0x21)  // Keyboard interrupt number

#define SCANCODE_EXT            (0xe0)

/* ASCII control characters */
#define ESC		    ('\x1b')  // Escape key
#define BACKSPACE	('\b')    // Backspace key
#define TAB		    ('\t')    // Tab key
#define ENTER		('\r')    // Enter (carriage return)
#define DELETE		('\x7f')  // Delete key

/* Invisible control characters (do not produce a visible output) */
#define INVISIBLE	       (0)   // Represents an invisible keypress
#define CTRL_L_CHAR        INVISIBLE  // Left Control key
#define CTRL_R_CHAR        INVISIBLE  // Right Control key
#define SHIFT_L_CHAR       INVISIBLE  // Left Shift key
#define SHIFT_R_CHAR       INVISIBLE  // Right Shift key
#define ALT_L_CHAR         INVISIBLE  // Left Alt key
#define ALT_R_CHAR         INVISIBLE  // Right Alt key
#define CAPS_LOCK_CHAR     INVISIBLE  // Caps Lock key

/* Make codes for modifier keys (sent when key is pressed) */
#define SHIFT_L_MAKE       (0x2A)   // Left Shift key make code
#define SHIFT_R_MAKE       (0x36)   // Right Shift key make code
#define ALT_L_MAKE         (0x38)   // Left Alt key make code
#define ALT_R_MAKE         (0xE038) // Right Alt key make code (extended)
#define CTRL_L_MAKE        (0x1D)   // Left Control key make code
#define CTRL_R_MAKE        (0xE01D) // Right Control key make code (extended)
#define CAPS_LOCK_MAKE     (0x3A)   // Caps Lock key make code

/* Break codes for modifier keys (sent when key is released) */
#define ALT_R_BREAK        (0xE0B8) // Right Alt key break code (extended)
#define CTRL_R_BREAK       (0xE09D) // Right Control key break code (extended)

#endif
