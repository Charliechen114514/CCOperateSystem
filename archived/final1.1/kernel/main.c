#include "include/kernel/init.h"
#include "include/user/stdio/stdio.h"
#include "include/user/ccshell/ccshell.h"
#include "include/user/library/user_assertion.h"
#include "include/user/program/wait_exit.h"
#include "include/library/ccos_print.h"
#include "include/syscall/syscall.h"
void init(void);

int main(void) {
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
        ccshell(); // Start the shell for the child process
    }
    user_panic("init: Why are you here?:("); // If this is reached, panic
}
