#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/kernel/init.h"

void init_all(void)
{
    ccos_set_console_cursor(5, 0);
    ccos_puts("Initializing the system...\n");
    init_idt();
    ccos_puts("Initializing the system done!\n");
}
