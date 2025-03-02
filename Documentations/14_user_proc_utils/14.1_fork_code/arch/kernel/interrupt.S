[bits 32]
%include "asm/kernel/interrupt.inc"

[bits 32]
%define DO_NOP nop         ; If the CPU has already pushed an error code in the relevant exception, do nothing here to keep the stack format unified.
%define PUSH_ZERO push 0   ; If the CPU did not push an error code in the relevant exception, manually push a 0 to unify the stack format.

extern internal_exception_callback ; idt_table is an array of interrupt handler functions registered in C.

section .data
global asm_intr_vector_table
asm_intr_vector_table:
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

; -------------------- Table Page for the interrupt -----------------------
;   contains two part
;   Part 1 is the intel usage, see the intel cpu reference for details
;   Part 2 is the future hook our operating system use!
; -------------------------------------------------------------------------
;   Part 1 Table Page for the reserved intel cpu
; -------------------------------------------------------------------------
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
; -------------------------------------------------------------------------
;   Part 2 Table Page for the interrupt for kernel
; -------------------------------------------------------------------------
INTR_VECTOR 0x20, PUSH_ZERO  ; Entry for the timer interrupt.
INTR_VECTOR 0x21, PUSH_ZERO  ; Entry for the keyboard interrupt.
INTR_VECTOR 0x22, PUSH_ZERO  ; Cascade interrupt.
INTR_VECTOR 0x23, PUSH_ZERO  ; Entry for serial port 2.
INTR_VECTOR 0x24, PUSH_ZERO  ; Entry for serial port 1.
INTR_VECTOR 0x25, PUSH_ZERO  ; Entry for parallel port 2.
INTR_VECTOR 0x26, PUSH_ZERO  ; Entry for the floppy disk.
INTR_VECTOR 0x27, PUSH_ZERO  ; Entry for parallel port 1.
INTR_VECTOR 0x28, PUSH_ZERO  ; Entry for the real-time clock.
INTR_VECTOR 0x29, PUSH_ZERO  ; Redirect.
INTR_VECTOR 0x2a, PUSH_ZERO  ; Reserved.
INTR_VECTOR 0x2b, PUSH_ZERO  ; Reserved.
INTR_VECTOR 0x2c, PUSH_ZERO  ; PS/2 mouse.
INTR_VECTOR 0x2d, PUSH_ZERO  ; FPU floating-point unit exception.
INTR_VECTOR 0x2e, PUSH_ZERO  ; Hard disk.
INTR_VECTOR 0x2f, PUSH_ZERO  ; Reserved.

;;;;;;;;;;;;;;;;   Interrupt 0x80   ;;;;;;;;;;;;;;;;
[bits 32]
; specific for 0x80, we don't need to mappings to the that end!
extern syscall_table
section .text
global syscall_handler
syscall_handler:
; 1. Save the context environment.
   push 0                 ; Push 0 to unify the stack format.

   push ds
   push es
   push fs
   push gs
   pushad                 ; The PUSHAD instruction pushes 32-bit registers in the order:
                          ; EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI.
				 
   push 0x80              ; Push 0x80 here to maintain a unified stack format.

; 2. Pass parameters to the syscall sub-function.
   push edx               ; The third parameter in the syscall.
   push ecx               ; The second parameter in the syscall.
   push ebx               ; The first parameter in the syscall.

; 3. Call the sub-function handler.
   call [syscall_table + eax * 4]  ; The compiler will match the correct number of parameters in the stack based on the C function declaration.
   add esp, 12            ; Skip the three parameters above.

; 4. Store the return value of the call in the current kernel stack's EAX position.
   mov [esp + 8 * 4], eax	
   jmp intr_exit          ; Return via intr_exit, restoring the context.