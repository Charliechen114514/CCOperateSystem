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
    intr%1entry:            ; Each interrupt handler pushes the interrupt vector number, so each interrupt type has its own handler, knowing its own interrupt vector number.
    
       %2                   ; If the interrupt has an error code, it will be pushed after EIP.
                            
       ; Save the context environment below.
       ; these will be pack back later in the intr_exit!
       push ds
       push es
       push fs
       push gs
       pushad               ; The PUSHAD instruction pushes 32-bit registers in the order: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI.
    
       ; If the interrupt is from the slave chip, send EOI to both the slave and master chips.
       mov al, 0x20         ; End of Interrupt (EOI) command.
       out 0xa0, al         ; Send to the slave chip.
       out 0x20, al         ; Send to the master chip.
    
       push %1              ; Regardless of whether the target function in idt_table needs parameters, always push the interrupt vector number for debugging convenience.
       call [internal_exception_callback + %1 * 4]  ; Call the C version interrupt handler function in idt_table.
       jmp intr_exit
    
 section .data
    dd intr%1entry          ; Store the addresses of each interrupt entry program to form the intr_entry_table array.
 %endmacro
 
