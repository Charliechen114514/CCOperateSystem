#include "include/device/console_tty.h"
#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/library/assert.h"
#include "include/memory/memory.h"
#include "include/kernel/thread.h"
#include "include/kernel/interrupt.h"

#include "include/device/keyboard.h"
void kthread_demo(void* arg);
void kthread_demo_b(void* arg);

int main()
{
    ccos_set_console_cursor(5, 0);
    ccos_puts("Welcome Main!\nCCOS has been entering main!\n");
    init_all();
    console_puts("Start threadings...\n");
    // thread_start("A", 31, kthread_demo, "a_");
    // thread_start("B", 8, kthread_demo_b, "b_");
    enable_intr();
    while(1);
    // while (1){
    //     console_puts("Main");
    // }
}

// void kthread_demo(void* arg){
//     while(1){
//         Interrupt_Status s = disable_intr();
//         if(!ioq_empty(&keyboard_ringbuffer)){
//             console_puts(arg);
//             char byte = ioq_getchar(&keyboard_ringbuffer);
//             console_putchar(byte);
//             console_puts("! A has handle this!\n");
//         }
//         set_intr_state(s);
//     }
// } 

// void kthread_demo_b(void* agrs)
// {
//     while(1){
//         Interrupt_Status s = disable_intr();
//         if(!ioq_empty(&keyboard_ringbuffer)){
//             console_puts(agrs);
//             char byte = ioq_getchar(&keyboard_ringbuffer);
//             console_putchar(byte);
//             console_puts("! B has handle this!\n");
//         }
//         set_intr_state(s);
//     }  
// }