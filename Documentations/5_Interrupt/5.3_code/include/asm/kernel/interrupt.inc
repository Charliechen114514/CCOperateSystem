; Followings are the operetions the Interrupt enter
; behaviour done
%define DO_NOP      nop
%define PUSH_ZERO   push 0

; EOI command
%define EOI         0x20
%define SLAVE_PORT  0x0a
%define MASTER_PORT 0x20

%macro INTR_VECTOR 2
section .text
intr%1enrty:
    %2
    push intr_str
    call ccos_puts
    add esp, 4
    mov al, EOI
    out SLAVE_PORT, al
    out MASTER_PORT,al

    add esp, 4
    iret
section .data
    dd  intr%1enrty
%endmacro

