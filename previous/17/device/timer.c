#include "timer.h"

static void set_frequency(
    uint8_t     counter_port,
    uint8_t     counter_no,
    uint8_t     rwl,
    uint8_t     counter_mode,
    uint16_t    counter_value)
{
    outb(PIT_CONTROL_PORT,\
        (uint8_t)(counter_no << 6 | \
                  rwl << 4 | \
                  counter_mode << 1));

    outb(counter_port,(uint8_t)counter_value);
    outb(counter_port,(uint8_t)counter_value >> 8);
}

void timer_init()
{
    put_str("timer_init start!");
    set_frequency(  COUNTER0_PORT,
                    COUNTER0_NO,
                    READ_WRITE_LATCH,
                    COUNTER0_MODE,
                    COUNTER0_VALUE);

    put_str("timer_init finish!");
}
