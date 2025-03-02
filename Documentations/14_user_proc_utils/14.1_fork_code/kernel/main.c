#include "include/device/console_tty.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/thread/thread.h"
#include "include/user/stdio/stdio.h"
#include "include/memory/memory.h"
#include "include/library/ccos_print.h"
#include "include/filesystem/filesystem.h"
#include "include/library/string.h"
#include "include/filesystem/dir.h"
#include "include/syscall/syscall.h"
// void thread_a(void *args);
// void thread_b(void *args);
// void u_prog_a(void);
// void u_prog_b(void);
// int prog_a_pid = 0, prog_b_pid = 0;
// init process here
void init(void);

int main(void)
{
    init_all();
    while(1);
}

// init process here
void init(void)
{
    uint32_t ret_pid = fork();
    if (ret_pid)
    {
        printf("i am father, my pid is %d, child pid is %d\n", getpid(), ret_pid);
    }
    else
    {
        printf("i am child, my pid is %d, ret pid is %d\n", getpid(), ret_pid);
    }
    while (1)
        ;
}
