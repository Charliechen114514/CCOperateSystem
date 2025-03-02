#include "include/device/timer.h"
#include "include/io/io.h"
#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"
#include "include/utils/fast_utils.h"
#include "include/kernel/thread.h"
#include "include/device/timer_settings.h"

uint32_t ticks; // Total number of ticks since the kernel interrupt started

/* Write the counter number (counter_no), read-write lock attributes (rwl),
 * and counter mode (counter_mode) into the mode control register, and 
 * set the initial counter value (counter_value).
 */
static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl,
                          uint8_t counter_mode, uint16_t counter_value) {
    /* Write the control word to the control register port 0x43 */
    outb(PIT_CONTROL_PORT,
         (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    /* Write the lower 8 bits of counter_value first */
    outb(counter_port, (uint8_t)counter_value);
    /* Write the upper 8 bits of counter_value next */
    outb(counter_port, (uint8_t)counter_value >> 8);
}

/* Timer interrupt handler */
static void intr_timer_handler(void) {
    TaskStruct *cur_thread = running_thread();

    ASSERT(cur_thread->stack_magic == TASK_MAGIC); // Check for stack overflow

    cur_thread->elapsed_ticks++; // Record the CPU time consumed by this thread
    ticks++; // Total ticks since the first kernel timer interrupt, including both kernel and user mode

    if (cur_thread->ticks == 0) { // If the process time slice is exhausted, schedule a new process
        schedule();
    } else { // Decrease the remaining time slice of the current process
        cur_thread->ticks--;
    }
}

// Sleep in tick units. Any sleep function in other time formats will be converted to this.
static void ticks_to_sleep(uint32_t sleep_ticks) {
    uint32_t start_tick = ticks;

    while (ticks - start_tick < sleep_ticks) { // If the required ticks have not elapsed, yield the CPU
        thread_yield();
    }
}

// Sleep in milliseconds. 1 second = 1000 milliseconds
void mtime_sleep(uint32_t m_seconds) {
    uint32_t sleep_ticks = ROUNDUP(m_seconds, MILISEC_PER_INT);
    ASSERT(sleep_ticks > 0);
    ticks_to_sleep(sleep_ticks);
}

/* Initialize PIT8253 */
void init_system_timer() {
    verbose_write("open up the 8253 as timer ...\n");
    /* Set the timing cycle of 8253, which determines the interrupt interval */
    frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE,
                  COUNTER0_VALUE);
    register_intr_handler(0x20, intr_timer_handler);
    verbose_write("finished!\n");
}
