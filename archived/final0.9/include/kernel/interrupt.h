#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H
#include "include/library/types.h"
/* functors for the interrupt callback */
typedef void*   intr_func_ptr;
void idt_init(void);

/* enum states of the interrupt */
typedef enum {
    INTR_OFF,       // < flags here shows the intr closed
    INTR_ON         // < flags here shows the intr on
}Interrupt_Status;

Interrupt_Status intr_get_status(void);
Interrupt_Status enable_intr(void);
Interrupt_Status disable_intr(void);
Interrupt_Status set_intr_state(Interrupt_Status status);

// register the intr handler, for other low kernel level use!
void register_intr_handler(uint8_t nvec, intr_func_ptr func);
#endif