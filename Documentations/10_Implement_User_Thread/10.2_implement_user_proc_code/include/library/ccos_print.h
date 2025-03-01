#ifndef CCOS_PRINT_H
#define CCOS_PRINT_H
#include "include/library/types.h"
#include "include/settings.h"

#define CONSOLE_WIDTH           (80)
/* see the kernel/asm/_ccos_print.S for details */
void __ccos_putchar(uint8_t ascii_code);
void ccos_puts(char* message);
void __ccos_display_int(uint32_t integer);
/* console setter */
void ccos_set_console_cursor(uint16_t row, uint16_t col);
/* clean screen */
void clean_screen(void);

void verbose_ccputchar(uint8_t ascii_code);
void verbose_ccputs(char* message);
void verbose_ccint(uint32_t integer);

#endif