#include "include/device/timer.h"
#include "include/device/timer_settings.h"
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

/* init PIT8253 */
void init_system_timer(void)
{
    ccos_puts("   timer is initing...\n");
    frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    ccos_puts("   timer is initing! done!\n");
}