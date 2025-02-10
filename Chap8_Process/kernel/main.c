#include "include/device/console_tty.h"
#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/library/assert.h"
#include "include/memory/memory.h"
#include "include/kernel/thread.h"
#include "include/kernel/interrupt.h"
void kthread_demo(void* arg);
void kthread_demo_b(void* arg);

int main()
{
    ccos_set_console_cursor(5, 0);
    ccos_puts("Welcome Main!\nCCOS has been entering main!\n");
    init_all();
    ccos_puts("Start threadings...\n");
    thread_start("A", 31, kthread_demo, "arga");
    thread_start("B", 8, kthread_demo_b, "argb");
    enable_intr();
    while (1){
        console_puts("Main");
    }
}

void kthread_demo(void* arg){
    char* para = arg;
    while (1)
    {
        console_puts(para);
    }
} 

void kthread_demo_b(void* agrs)
{
    char* para = agrs;
    while (1)
    {
        console_puts(para);
    }   
}