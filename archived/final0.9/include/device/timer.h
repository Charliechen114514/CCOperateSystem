#ifndef TIMER_H
#define TIMER_H
#include "include/library/types.h"

// about global timer, see the timer setttings for the clocks
void init_system_timer(void);
void mtime_sleep(uint32_t m_seconds);
#endif