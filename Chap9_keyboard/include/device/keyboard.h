#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "include/io/ioqueue.h"

// expose the keybuffer!
extern IOQueue keyboard_ringbuffer;
void keyboard_init();

#endif