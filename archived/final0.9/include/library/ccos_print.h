#ifndef CCOS_PRINT_H
#define CCOS_PRINT_H

/* to local types.h */
#include "include/library/types.h"
#include "include/library/console_pen.h"
/*  
    one should never call this at kernel, cause the 
    ioflow is unsafe!
*/
#define CONSOLE_WIDTH           (80)
#define CONSOLE_PEN_COLOR       (COLOR_WHITE)

void clean_screen(void);
void ccos_puts(char* message);
void ccos_set_console_cursor(uint16_t row, uint16_t col);
void set_console_color(Color color);

/* see arch/kernel/print.S for the implements*/
/* programmers are not supportive to call this directly */
void __ccos_putchar(uint8_t ascii_char);
void __ccos_display_int(uint32_t integer);

// for pro :)
void early_console_init(void);
void post_console_init(void);
void pretty_write(char* message);

// for the kernel verbose
void verbose_write(char* message);
void verbose_write_int(uint32_t interger);
#endif