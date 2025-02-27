[bits 32]
%include "asm/kernel/interrupt.inc"

; using in test the interrupt issue

extern internal_exception_callback ; idt_table is an array of interrupt handler functions registered in C.

section .text
global intr_exit
intr_exit:	     
; Restore the context environment below.
    add esp, 4              ; Skip the interrupt number.
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4              ; Skip DO_NOP.
    iretd

section .data
; This is the asm intr table,
; using in indicate for the lower level intr controls call
global asm_intr_vector_table
asm_intr_vector_table:

INTR_VECTOR 0x00, PUSH_ZERO
INTR_VECTOR 0x01, PUSH_ZERO
INTR_VECTOR 0x02, PUSH_ZERO
INTR_VECTOR 0x03, PUSH_ZERO 
INTR_VECTOR 0x04, PUSH_ZERO
INTR_VECTOR 0x05, PUSH_ZERO
INTR_VECTOR 0x06, PUSH_ZERO
INTR_VECTOR 0x07, PUSH_ZERO 
INTR_VECTOR 0x08, DO_NOP
INTR_VECTOR 0x09, PUSH_ZERO
INTR_VECTOR 0x0a, DO_NOP
INTR_VECTOR 0x0b, DO_NOP 
INTR_VECTOR 0x0c, PUSH_ZERO
INTR_VECTOR 0x0d, DO_NOP
INTR_VECTOR 0x0e, DO_NOP
INTR_VECTOR 0x0f, PUSH_ZERO 
INTR_VECTOR 0x10, PUSH_ZERO
INTR_VECTOR 0x11, DO_NOP
INTR_VECTOR 0x12, PUSH_ZERO
INTR_VECTOR 0x13, PUSH_ZERO 
INTR_VECTOR 0x14, PUSH_ZERO
INTR_VECTOR 0x15, PUSH_ZERO
INTR_VECTOR 0x16, PUSH_ZERO
INTR_VECTOR 0x17, PUSH_ZERO 
INTR_VECTOR 0x18, DO_NOP
INTR_VECTOR 0x19, PUSH_ZERO
INTR_VECTOR 0x1a, DO_NOP
INTR_VECTOR 0x1b, DO_NOP 
INTR_VECTOR 0x1c, PUSH_ZERO
INTR_VECTOR 0x1d, DO_NOP
INTR_VECTOR 0x1e, DO_NOP
INTR_VECTOR 0x1f, PUSH_ZERO 
INTR_VECTOR 0x20, PUSH_ZERO 