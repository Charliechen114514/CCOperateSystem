#include "include/library/ccos_print.h"
void __ccos_set_console_cursor(uint16_t row, uint16_t col);
void ccos_set_console_cursor(uint16_t row, uint16_t col)
{
    uint8_t __col = col + CONSOLE_WIDTH * row; 
    uint8_t __row = (CONSOLE_WIDTH * row + col - __col) % UINT8_MAX;
    __ccos_set_console_cursor(__row, __col);
}