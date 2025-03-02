#include "include/device/console_tty.h"
#include "include/filesystem/dir.h"
#include "include/filesystem/fs.h"
#include "include/kernel/init.h"
#include "include/kernel/interrupt.h"
#include "include/kernel/thread.h"
#include "include/library/ccos_print.h"
#include "include/memory/memory.h"
#include "include/shell/shell.h"
#include "include/syscall/syscall.h"
#include "include/user/library/user_assert.h"
#include "include/user/program/process.h"
#include "include/user/program/syscall-init.h"
#include "include/user/stdio/stdio.h"

#include "include/FormatIO/stdio-kernel.h"
#include "include/device/ide.h"

void init(void);

int main(void) {
    early_console_init();                // Initialize the early console
    init_all();                          // Initialize the entire system
    thread_exit(running_thread(), true); // Exit the current thread
    return 0;
}

/* init process */
void init(void) {
    uint32_t ret_pid = fork(); // Fork a new process
    if (ret_pid) {             // Parent process
        int status;
        int child_pid;
        /* The init process keeps reaping zombie processes here */
        while (1) {
            child_pid = wait(&status); // Wait for the child process to finish
            printf(
                "Initializing a child process. Its pid is %d, status is %d\n",
                child_pid, status);
        }
    } else {        // Child process
        ccos_baseshell(); // Start the shell for the child process
    }
    panic("init: Why are you here?:("); // If this is reached, panic
}
