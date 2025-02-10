#include "include/kernel/init.h"
#include "include/library/ccos_print.h"
#include "include/kernel/interrupt.h"
#include "include/device/timer.h"
#include "include/memory/memory.h"
#include "include/kernel/thread.h"
#include "include/device/console_tty.h"

void init_all(void)
{
    ccos_puts("start our work in initialize\n");
    idt_init();
    memory_management_init();
    thread_init();
    init_system_timer();
    init_console();
    ccos_puts("initialize done!\n");
}
