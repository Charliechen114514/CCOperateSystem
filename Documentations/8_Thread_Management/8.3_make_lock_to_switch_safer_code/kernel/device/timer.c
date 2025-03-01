#include "include/device/timer.h"          // Include the header for timer-related definitions
#include "include/device/timer_settings.h" // Include the header for timer settings
#include "include/library/ccos_print.h"    // Include the header for printing functions
#include "include/io/io.h"                 // Include the header for I/O functions
#include "include/kernel/interrupt.h"
#include "include/thread/thread.h"
#include "include/library/kernel_assert.h"
uint32_t ticks; // 'ticks' variable registers the time passed since system start

/* Timer interrupt handler */
static void intr_timer_handler(void) {
    TaskStruct *cur_thread = running_thread();

    KERNEL_ASSERT(cur_thread->stack_magic == TASK_MAGIC); // Check for stack overflow

    cur_thread->elapsed_ticks++; // Record the CPU time consumed by this thread
    ticks++; // Total ticks since the first kernel timer interrupt, including both kernel and user mode

    if (cur_thread->ticks == 0) { // If the process time slice is exhausted, schedule a new process
        schedule();
    } else { // Decrease the remaining time slice of the current process
        cur_thread->ticks--;
    }
}

/**
 * frequency_set function configures a counter in the PIT (Programmable Interval Timer)
 * @param counter_port The port of the counter to configure
 * @param counter_no The counter number (0, 1, or 2)
 * @param rwl The read/write latch mode (0 for write, 1 for read latch)
 * @param counter_mode The mode of the counter (e.g., 0 for interrupt on terminal count)
 * @param counter_value The value to load into the counter (16-bit value)
 */
static void frequency_set(
    uint8_t counter_port, uint8_t counter_no,
    uint8_t rwl, uint8_t counter_mode, uint16_t counter_value)
{
    // Send the control word to configure the counter
    // The control word consists of:
    // - counter_no: 2 bits for the counter number (shifted to bits 6 and 7)
    // - rwl: 1 bit for read/write latch (shifted to bit 4)
    // - counter_mode: 3 bits for the counter mode (shifted to bits 1, 2, and 3)
    // The control word is sent to the PIT control port
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));

    // Write the lower 8 bits of the counter value to the counter port
    outb(counter_port, (uint8_t)counter_value);

    // Write the upper 8 bits of the counter value to the counter port
    outb(counter_port, (uint8_t)(counter_value >> 8));
}

/**
 * init_system_timer initializes the system timer (PIT8253)
 * This function sets up the timer and configures counter 0 with the appropriate settings.
 */
void init_system_timer(void)
{
    // Print message indicating the timer is being initialized
    verbose_ccputs("   timer is initing...\n");

    // Configure the timer's counter 0 with appropriate settings
    // The function frequency_set will configure the counter 0 port, set the mode,
    // and initialize it with the value defined in COUNTER0_VALUE.
    frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_intr_handler(SCHED_INTERRUPT_CALLBACK_N, intr_timer_handler);
    // Print message indicating the timer has been successfully initialized
    verbose_ccputs("   timer is initing! done!\n");
}
