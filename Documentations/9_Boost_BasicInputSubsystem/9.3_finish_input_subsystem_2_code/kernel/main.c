#include "include/device/console_tty.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory.h"
#include "include/thread/thread.h"
#include "include/kernel/interrupt.h"
#include "include/device/keyboard.h"
void thread_a(void *args);
void thread_b(void *args);
int main(void)
{
    init_all();
    thread_start("k_thread_a", 31, thread_a, "In thread A:");
    thread_start("k_thread_b", 16, thread_b, "In thread B:");
    interrupt_enabled();
    while (1)
    {}
}

void thread_a(void* args){
    while(1){
        Interrupt_Status old_status = set_intr_status(INTR_OFF); 
        char* arg = (char*)args;
        if (!ioq_empty(&keyboard_ringbuffer)) {
            console_ccos_puts(arg);
            char byte = ioq_getchar(&keyboard_ringbuffer);
            console__ccos_putchar(byte);
            console__ccos_putchar('\n');
        }
        set_intr_status(old_status);
    }
    
}

void thread_b(void* args){
    while(1){
        Interrupt_Status old_status = set_intr_status(INTR_OFF); 
        char* arg = (char*)args;
        if (!ioq_empty(&keyboard_ringbuffer)) {
            console_ccos_puts(arg);
            char byte = ioq_getchar(&keyboard_ringbuffer);
            console__ccos_putchar(byte);
            console__ccos_putchar('\n');
        }
        set_intr_status(old_status);
    }
}