#include "include/thread/thread.h"
#include "include/user/syscall/syscall.h"
#include "include/library/ccos_print.h"
#include "include/user/syscall/syscall_init.h"
#include "include/library/string.h"
#include "include/device/console_tty.h"
#define SYSCALL_SUM_NR (32) // Number of system calls
typedef void *syscall;
syscall syscall_table[SYSCALL_SUM_NR]; // System call table to hold the addresses of
                                   // system call functions

/* Return the pid of the current task */
uint32_t sys_getpid(void) {
    return current_thread()->pid; // Retrieve the PID of the running thread
}

uint32_t sys_write(char* str) {
    console_ccos_puts(str);
    return k_strlen(str);
}

/* Initialize the system call table */
void syscall_init(void) {
    verbose_ccputs("syscall_init start\n"); // Logging the start of syscall initialization
    /* Set the system call table entries for various system call numbers */
    syscall_table[SYS_GETPID] = sys_getpid; // Get process ID
    syscall_table[SYS_WRITE] = sys_write;
    verbose_ccputs("syscall_init done\n"); // Logging the completion of syscall
                                          // initialization
}
