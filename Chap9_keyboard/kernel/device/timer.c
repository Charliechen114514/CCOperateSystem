#include "include/device/timer.h"
#include "include/kernel/interrupt.h"
#include "include/device/timer_settings.h"
#include "include/kernel/thread.h"
#include "include/library/assert.h"
#include "include/library/ccos_print.h"
#include "include/io/io.h"

uint32_t ticks;          // ticks registers the time pass

static void frequency_set(
    uint8_t counter_port, uint8_t counter_no, 
    uint8_t rwl,  uint8_t counter_mode, uint16_t counter_value) 
{

    outb(PIT_CONTROL_PORT, 
        (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    outb(counter_port, (uint8_t)counter_value);
    outb(counter_port, (uint8_t)counter_value >> 8);
}

/* 时钟的中断处理函数 */
static void intr_timer_handler(void) {
    TaskStruct* cur_thread = running_thread();
 
    ASSERT(cur_thread->stack_magic == TASK_MAGIC);         // 检查栈是否溢出
 
    cur_thread->elapsed_ticks++;	  // 记录此线程占用的cpu时间嘀
    ticks++;	  //从内核第一次处理时间中断后开始至今的滴哒数,内核态和用户态总共的嘀哒数
 
    if (cur_thread->ticks_in_cpu == 0) {	  // 若进程时间片用完就开始调度新的进程上cpu
        schedule(); 
    } else {				  // 将当前进程的时间片-1
        cur_thread->ticks_in_cpu--;
    }
 }

/* init PIT8253 */
void init_system_timer(void)
{
    ccos_puts("   timer is initing...\n");
    frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_intr_handler(0x20, intr_timer_handler);
    ccos_puts("   timer is initing! done!\n");
}