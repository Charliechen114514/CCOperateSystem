#ifndef CCOS_PRINT_H
#define CCOS_PRINT_H
#include "include/library/types.h"

#define CONSOLE_WIDTH           (80)
/* see the kernel/asm/_ccos_print.S for details */
void __ccos_putchar(uint8_t ascii_code);
void ccos_puts(char* message);
void __ccos_display_int(uint32_t integer);
/* console setter */
void ccos_set_console_cursor(uint16_t row, uint16_t col);
/* clean screen */
void clean_screen(void);
#endif