#include "include/kernel/init.h"
#include "include/device/console_tty.h"
#include "include/device/ide.h"
#include "include/device/keyboard.h"
#include "include/device/timer.h"
#include "include/filesystem/fs.h"
#include "include/kernel/interrupt.h"
#include "include/kernel/thread.h"
#include "include/library/ccos_print.h"
#include "include/memory/memory.h"
#include "include/user/program/syscall-init.h"
#include "include/user/tss/tss.h"

/* Responsible for initializing all modules */
void init_all() {
    idt_init();               // Initialize interrupts
    memory_management_init(); // Initialize memory management system
    thread_init();            // Initialize thread-related structures
    init_system_timer();      // Initialize PIT (Programmable Interrupt Timer)
    console_init(); // Console initialization, should be done before enabling
                    // interrupts
    post_console_init();
    keyboard_init(); // Initialize keyboard
    tss_init();      // Initialize Task State Segment (TSS)
    syscall_init();  // Initialize system calls
    enable_intr();   // Enable interrupts, required for ide_init
    ide_init();      // Initialize hard disk
    filesys_init();  // Initialize file system
}
