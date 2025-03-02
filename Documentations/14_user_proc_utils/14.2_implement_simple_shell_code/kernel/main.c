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
#include "include/user/ccshell/ccshell.h"
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
        while(1);
    }
    else
    {
        ccshell();
    }
    while (1)
        ;
}
