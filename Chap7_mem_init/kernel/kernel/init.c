#include "include/kernel/init.h"
#include "include/library/ccos_print.h"
#include "include/kernel/interrupt.h"
#include "include/device/timer.h"
#include "include/memory/memory.h"
void init_all(void)
{
    ccos_puts("start our work in initialize\n");
    idt_init();
    memory_management_init();
    init_system_timer();
    ccos_puts("initialize done!\n");
}
