#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/device/timer.h"
#include "include/kernel/init.h"
#include "include/memory/memory.h"
#include "include/thread/thread.h"
#include "include/device/console_tty.h"
#ifdef REQ_CHECK_LIB
#include "test/include/test.h"
#endif

static inline void __check_library(void){
#ifdef REQ_CHECK_LIB
    involk_test();
#endif
}


void init_all(void)
{
    ccos_set_console_cursor(5, 0);
    ccos_puts("Initializing the system...\n");
    init_idt();
    init_system_timer();
    memory_management_init();
    thread_subsystem_init();
    console_init();
    ccos_puts("Initializing the system done!\n");
}

void interrupt_enabled(void)
{
    set_intr_status(INTR_ON);
}