#include "include/device/console_tty.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/thread/thread.h"
#include "include/user/process/process.h"
#include "include/user/syscall/syscall.h"
#include "include/user/syscall/syscall_init.h"
#include "include/user/stdio/stdio.h"
void thread_a(void *args);
void thread_b(void *args);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0, prog_b_pid = 0; 

int main(void)
{
    init_all();
    create_process(u_prog_a, "user_prog_a");
    create_process(u_prog_b, "user_prog_b");
    interrupt_enabled();
    console_ccos_puts("main_pid: 0x");
    console__ccos_display_int(sys_getpid());
    console__ccos_putchar('\n');
    thread_start("k_thread_a", 31, thread_a, "In thread A:");
    thread_start("k_thread_b", 31, thread_b, "In thread B:");
    
    while (1)
    {
    }
}

void thread_a(void *arg)
{
    char* para = arg;
    (void)para;
    console_ccos_puts("thread a pid: 0x");
    console__ccos_display_int(sys_getpid());
    console__ccos_putchar('\n');
    while (1);
}
void thread_b(void *arg)
{
    char* para = arg;
    (void)para;
    console_ccos_puts("thread b pid: 0x");
    console__ccos_display_int(sys_getpid());
    console__ccos_putchar('\n');
    while (1);
}

void u_prog_a(void)
{
    printf("prog_a_pid:0x%x\n", getpid()); 
    while (1);
}

void u_prog_b(void) { 
    printf("prog_b_pid:0x%x\n", getpid()); 
    while(1);
} 