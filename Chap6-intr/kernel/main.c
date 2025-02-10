#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
int main()
{
    ccos_set_console_cursor(5, 0);
    ccos_puts("CCOS has been entering main!\n");
    init_all();
    asm volatile("sti");
    while(1);
    return 0;
}