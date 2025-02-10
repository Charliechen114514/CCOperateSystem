#ifndef CCOS_PRINT_H
#define CCOS_PRINT_H

/* to local types.h */
#include "types.h"

/*  
    one should never call this at kernel, cause the 
    ioflow is unsafe!
*/

#define CONSOLE_WIDTH   (80)

void clean_screen(void);
void ccos_puts(char* message);
void ccos_set_console_cursor(uint16_t row, uint16_t col);

/* see arch/kernel/print.S for the implements*/
/* programmers are not supportive to call this directly */
void __ccos_putchar(uint8_t ascii_char);
void __ccos_display_int(uint32_t integer);


#endif