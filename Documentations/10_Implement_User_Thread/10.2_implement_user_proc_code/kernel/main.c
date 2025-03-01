#include "include/device/console_tty.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/thread/thread.h"
#include "include/user/process/process.h"
void thread_a(void *args);
void thread_b(void *args);
void u_prog_a(void);
void u_prog_b(void);
int test_var_a = 0, test_var_b = 0; 

int main(void)
{
    init_all();
    thread_start("k_thread_a", 31, thread_a, "In thread A:");
    thread_start("k_thread_b", 16, thread_b, "In thread B:");
    process_execute(u_prog_a, "user_prog_a");
    process_execute(u_prog_b, "user_prog_b");
    interrupt_enabled();
    while (1)
    {
    }
}

void thread_a(void *arg[[maybe_unused]])
{
    while (1)
    {
        console_ccos_puts("v_a:0x");
        console__ccos_display_int(test_var_a);
    }
}
void thread_b(void *arg[[maybe_unused]])
{
    while (1)
    {
        console_ccos_puts(" v_b:0x");
        console__ccos_display_int(test_var_b);
    }
}

void u_prog_a(void)
{
    while (1)
    {
        test_var_a++;
    }
}

void u_prog_b(void) { 
    while(1) { 
        test_var_b++; 
    } 
} 