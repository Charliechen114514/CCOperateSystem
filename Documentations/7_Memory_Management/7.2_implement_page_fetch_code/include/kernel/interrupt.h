#ifndef INTERRUPT_H
#define INTERRUPT_H

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

#endif