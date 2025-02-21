#include "include/library/ccos_print.h"

int main()
{
    // test setcursor
    ccos_set_console_cursor(5, 0);
    // test the putchar
    // __ccos_putchar('C');
    // __ccos_putchar('h');
    // __ccos_putchar('a');
    // __ccos_putchar('r');
    // __ccos_putchar('l');
    // __ccos_putchar('i');
    // __ccos_putchar('e');
    // clean screen
    // clean_screen();
    // test puts
    ccos_puts("Charlies! Heri\be\n");
    ccos_puts("Let's test a number print!: ");
    __ccos_display_int(0x114514);
    __ccos_putchar('\n');
    int a = 0;
    ccos_puts("Address of a is: ");
    __ccos_display_int(&a);
    __ccos_putchar('\n');
    while(1);
    return 0;
}