#include "include/device/console_tty.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/thread/thread.h"
#include "include/user/process/process.h"
#include "include/user/syscall/syscall.h"
#include "include/user/syscall/syscall_init.h"
#include "include/user/stdio/stdio.h"
#include "include/memory/memory.h"
void thread_a(void *args);
void thread_b(void *args);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0, prog_b_pid = 0; 

int main(void)
{
    init_all();
    // process_execute(u_prog_a, "user_prog_a");
    // process_execute(u_prog_b, "user_prog_b");
    interrupt_enabled();
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
    void* addr = sys_malloc(33);
    console_ccos_puts("thread a malloc 33 bytes: ");
    console__ccos_display_int((uint32_t)addr);
    console__ccos_putchar('\n');
    while (1);
}
void thread_b(void *arg)
{
    char* para = arg;
    (void)para;
    void* addr = sys_malloc(63);
    console_ccos_puts("thread a malloc 63 bytes: ");
    console__ccos_display_int((uint32_t)addr);
    console__ccos_putchar('\n');
    while (1);
}

// void u_prog_a(void)
// {
//     char* name = "user_prog_a"; 
//     printf("in proc %s, prog_a_pid:0x%x%c", name, getpid(), '\n'); 
//     while (1);
// }

// void u_prog_b(void) { 
//     char* name = "user_prog_b"; 
//     printf("in proc %s, prog_b_pid:0x%x%c", name, getpid(), '\n'); 
//     while(1);
// } 