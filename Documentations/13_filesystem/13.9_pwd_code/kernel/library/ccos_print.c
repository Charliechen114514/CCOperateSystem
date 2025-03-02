#include "include/library/ccos_print.h"
extern void __ccos_set_console_cursor(uint16_t row, uint16_t col);

void ccos_set_console_cursor(uint16_t row, uint16_t col)
{
    uint8_t __col = col + CONSOLE_WIDTH * row; 
    uint8_t __row = (CONSOLE_WIDTH * row + col - __col) % UINT8_MAX;
    __ccos_set_console_cursor(__row, __col);
}

void verbose_ccputchar(uint8_t ascii_code){
    (void)ascii_code;
#ifdef VERBOSE
    __ccos_putchar(ascii_code);
#endif
}
void verbose_ccputs(char* message){
    (void)message;
#ifdef VERBOSE
    ccos_puts(message);
#endif    
}

void verbose_ccint(uint32_t integer){
    (void)integer;
#ifdef VERBOSE
    __ccos_display_int(integer);
#endif  
}