#ifndef TIMER_SETTINGS_H
#define TIMER_SETTINGS_H

#define IRQ0_FREQUENCY	    (100)
#define INPUT_FREQUENCY	    (1193180)
#define COUNTER0_VALUE	    INPUT_FREQUENCY / IRQ0_FREQUENCY
#define CONTRER0_PORT	    (0x40)
#define COUNTER0_NO	        (0)
#define COUNTER_MODE	    (2)
#define READ_WRITE_LATCH    (3)
#define PIT_CONTROL_PORT    (0x43)
#define MILISEC_PER_INT     (1000 / IRQ0_FREQUENCY)

// timely interrupt settings
#define SCHED_INTERRUPT_CALLBACK_N  (0x20)


#endif

