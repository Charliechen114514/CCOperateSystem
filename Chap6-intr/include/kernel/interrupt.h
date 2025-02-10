#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

typedef void*   intr_func_ptr;

void idt_init(void);

#endif