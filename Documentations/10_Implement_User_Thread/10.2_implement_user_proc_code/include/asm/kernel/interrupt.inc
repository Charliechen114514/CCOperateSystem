; Followings are the operetions the Interrupt enter
; behaviour done
%define DO_NOP      nop
%define PUSH_ZERO   push 0

; EOI command
%define EOI         0x20
%define SLAVE_PORT  0xa0
%define MASTER_PORT 0x20

%macro INTR_VECTOR 2
section .text
intr%1enrty:
    %2

    push ds
    push es
    push fs
    push gs
    pushad    

    ; write the 8259A
    mov al, EOI
    out SLAVE_PORT, al
    out MASTER_PORT,al

    push %1              ; Regardless of whether the target function in idt_table needs parameters, always push the interrupt vector number for debugging convenience.
    call [internal_exception_callback + %1 * 4]  ; Call the C version interrupt handler function in idt_table.
    jmp intr_exit

section .data
    dd  intr%1enrty
%endmacro

