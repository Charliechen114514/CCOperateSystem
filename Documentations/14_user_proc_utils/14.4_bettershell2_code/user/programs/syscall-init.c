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
    return current_thread()->pid; // Retrieve the PID of the running thread
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
    syscall_table[SYS_READ] = sys_read;
    syscall_table[SYS_PUTCHAR] = sys_putchar;
    syscall_table[SYS_CLEAR] = clean_screen;
    syscall_table[SYS_GETCWD] = sys_getcwd;  // Get current working directory
    syscall_table[SYS_OPEN] = sys_open;      // Open a file
    syscall_table[SYS_CLOSE] = sys_close;    // Close a file
    syscall_table[SYS_LSEEK] = sys_lseek;    // Seek within a file
    syscall_table[SYS_UNLINK] = sys_unlink;  // Unlink a file
    syscall_table[SYS_MKDIR] = sys_mkdir;    // Make a new directory
    syscall_table[SYS_OPENDIR] = sys_opendir;     // Open a directory
    syscall_table[SYS_CLOSEDIR] = sys_closedir;   // Close a directory
    syscall_table[SYS_CHDIR] = sys_chdir;         // Change current directory
    syscall_table[SYS_RMDIR] = sys_rmdir;         // Remove a directory
    syscall_table[SYS_READDIR] = sys_readdir;     // Read a directory entry
    syscall_table[SYS_REWINDDIR] = sys_rewinddir; // Rewind a directory
    syscall_table[SYS_STAT] = sys_stat;   // Get file or directory status
    syscall_table[SYS_PS] = sys_ps;       // Show running processes
    verbose_ccputs("syscall_init done\n"); // Logging the completion of syscall
                                          // initialization
}
