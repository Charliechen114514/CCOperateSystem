#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "include/library/types.h"
typedef void *intr_functor; // Define a function pointer type for interrupt handlers

// idt initings
void init_idt(void);

// this is the enum of the interrupt status
typedef enum {
    INTR_OFF,   // interrupt disabled
    INTR_ON     // interrupt enabled
}Interrupt_Status;

// interface of interrupt subsystem library
// fetch the status
Interrupt_Status get_intr_status(void);
// set the status, and we will also fetch the old status, if we want:)
Interrupt_Status set_intr_status(const Interrupt_Status status);

// register the intr handler, for other low kernel level use!
void register_intr_handler(uint8_t nvec, intr_functor func);

#endif