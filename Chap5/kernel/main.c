#include "include/library/ccos_print.h"

int main()
{
    ccos_set_console_cursor(5, 0);
    ccos_puts("Kernel!\nCCOS has been entering main!");
    ccos_set_console_cursor(7, 0);
    ccos_puts("Kernel now can display the number!");
    ccos_set_console_cursor(8, 0);
    __ccos_display_int(10);
    ccos_set_console_cursor(9, 0);
    __ccos_display_int(0x1111);
    ccos_set_console_cursor(10, 0);
    __ccos_display_int(0b1111);
    // ccos_putchar('k');
    // ccos_putchar('e');
    // ccos_putchar('r');
    // ccos_putchar('n');
    // ccos_putchar('e');
    // ccos_putchar('l');
    // ccos_putchar('i');
    // ccos_putchar('\b');
    // ccos_putchar('!');
    // ccos_putchar('\n');
    // ccos_putchar('1');
    // ccos_putchar('2');
    // ccos_putchar('3');
    while(1);
    return 0;
}