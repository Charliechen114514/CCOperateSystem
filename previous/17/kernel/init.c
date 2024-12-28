# include "init.h"
# include "kernel/print.h"
# include "timer.h"

void init_all() {
    put_str("initialized start!\n");
    // init interrupt descriptor table
    idt_init();
    // init timer
    timer_init();
    // init memorys
    mem_init();
    put_str("Init finished...!\n");
}