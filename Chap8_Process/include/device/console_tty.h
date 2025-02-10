#ifndef CONSOLE_TTY_H
#define CONSOLE_TTY_H
#include "include/library/types.h"


void init_console(void);
void acquire_console(void);
void release_console(void);
void console_puts(char* str);
void console_putchar(uint8_t char_asci);
void console_putint(uint32_t num);
void console_setcursor(uint16_t row, uint16_t col);
#endif