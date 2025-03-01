#ifndef KERBOARD_H
#define KERBOARD_H
#include "include/io/ioqueue.h"

// expose the keybuffer!
extern IOQueue keyboard_ringbuffer;

void init_basic_input_subsystem(void);

#endif