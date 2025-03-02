#include "include/user/program/syscall-init.h"
#include "include/device/console_tty.h"
#include "include/filesystem/filesystem.h"
#include "include/thread/thread.h"
#include "include/library/ccos_print.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/memory/memory.h"
#include "include/syscall/syscall.h"
#include "include/user/program/fork.h"
#define SYSCALL_SUM_NR (32) // Number of system calls
typedef void *syscall;
syscall syscall_table[SYSCALL_SUM_NR]; // System call table to hold the addresses of
                                   // system call functions

/* Return the pid of the current task */
uint32_t sys_getpid(void) {
    return running_thread()->pid; // Retrieve the PID of the running thread
}


extern void sys_free(void *ptr);
extern void *sys_malloc(uint32_t size);

/* Initialize the system call table */
void syscall_init(void) {
    verbose_ccputs(
        "syscall_init start\n"); // Logging the start of syscall initialization
    /* Set the system call table entries for various system call numbers */
    syscall_table[SYS_GETPID] = sys_getpid; // Get process ID
    syscall_table[SYS_WRITE] = sys_write;   // Write to console
    syscall_table[SYS_MALLOC] = sys_malloc; // Memory allocation
    syscall_table[SYS_FREE] = sys_free;     // Free allocated memory
    syscall_table[SYS_FORK] = sys_fork;     // Fork a new process
    verbose_ccputs("syscall_init done\n"); // Logging the completion of syscall
                                          // initialization
}
