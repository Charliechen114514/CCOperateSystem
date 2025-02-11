#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "include/io/ioqueue.h"

// expose the keybuffer!
extern IOQueue keyboard_ringbuffer;

// do the input standard
void keyboard_init(void);

#endif