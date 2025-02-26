#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/device/timer.h"
#include "include/kernel/init.h"

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
    ccos_puts("Initializing the system done!\n");
}
