#include "include/kernel/init.h"
#include "include/device/console_tty.h"
#include "include/device/ide.h"
#include "include/device/keyboard.h"
#include "include/device/timer.h"
#include "include/kernel/interrupt.h"
#include "include/thread/thread.h"
#include "include/library/ccos_print.h"
#include "include/memory/memory.h"
#include "include/user/program/syscall-init.h"
#include "include/user/tss/tss.h"
#include "include/filesystem/filesystem.h"

/* Responsible for initializing all modules */
void init_all() {
    early_console_init();     // Initialize the early console
    init_idt();               // Initialize interrupts
    memory_management_init(); // Initialize memory management system
    post_console_init();      // post output before intr is enabled
    thread_init();            // Initialize thread-related structures
    init_system_timer();      // Initialize PIT (Programmable Interrupt Timer)
    console_init(); // Console initialization, should be done before enabling
                    // interrupts
    keyboard_init(); // Initialize keyboard
    tss_init();      // Initialize Task State Segment (TSS)
    syscall_init();  // Initialize system calls
    set_intr_status(INTR_ON);   // Enable interrupts, required for ide_init
    ide_init();      // Initialize hard disk
    filesystem_init();
}
