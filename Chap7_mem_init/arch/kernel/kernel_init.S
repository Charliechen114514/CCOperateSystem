[bits 32]
; defines of the work when entering intr
%define DO_NOP      nop
%define PUSH_ZERO   push 0

extern idt_table
section .data
global interrupt_entry_tbl

; Author's here: to debug the target interrupt, it requires
; you to know the when interrupt happens
; 1. set breakpoint in kernel main(vb 0008:0xc0001500)
; 2. trace the intr (show int)
; 3. s 70000 or any to let the target intr hanppens
; 4. remembers the instructions number the current pass, let assume it's n
; 5. restart the kernel, now, do: sba n
; 6. continue as type 'c'
; 7. holy!
; ------------- Interrupt Vector Creator -------------
; defines the interrupt vector
; ------------- Interrupt Vector Creator -------------
%macro INTR_VEC 2
section .text  
intr%1entry:
    %2
    push ds
    push es
    push fs
    push gs
    pushad	

    mov al, 0x20
    out 0xa0, al
    out 0x20, al
    push %1			                ; we push the idt number for the debug! it's conv!
    call [idt_table + %1*4]         ; call the idt table hook
    jmp intr_exit                   ; goto the stack cleaner
section .data
    dd intr%1entry
%endmacro
; defines the interrupt vector end

interrupt_entry_tbl:
section .text
global intr_exit
intr_exit:	     
; 以下是恢复上下文环境
   add esp, 4			   
   popad
   pop gs
   pop fs
   pop es
   pop ds
   add esp, 4			   
   iretd
; -------------------- Table Page for the interrupt -----------------------
;   contains two part
;   Part 1 is the intel usage, see the intel cpu reference for details
;   Part 2 is the future hook our operating system use!
; -------------------------------------------------------------------------
;   Part 1
; -------------------- Table Page for the reserved intel cpu ---------------
INTR_VEC 0x00,PUSH_ZERO
INTR_VEC 0x01,PUSH_ZERO
INTR_VEC 0x02,PUSH_ZERO
INTR_VEC 0x03,PUSH_ZERO 
INTR_VEC 0x04,PUSH_ZERO
INTR_VEC 0x05,PUSH_ZERO
INTR_VEC 0x06,PUSH_ZERO
INTR_VEC 0x07,PUSH_ZERO 
INTR_VEC 0x08,DO_NOP
INTR_VEC 0x09,PUSH_ZERO
INTR_VEC 0x0a,DO_NOP
INTR_VEC 0x0b,DO_NOP 
INTR_VEC 0x0c,PUSH_ZERO
INTR_VEC 0x0d,DO_NOP
INTR_VEC 0x0e,DO_NOP
INTR_VEC 0x0f,PUSH_ZERO 
INTR_VEC 0x10,PUSH_ZERO
INTR_VEC 0x11,DO_NOP
INTR_VEC 0x12,PUSH_ZERO
INTR_VEC 0x13,PUSH_ZERO 
INTR_VEC 0x14,PUSH_ZERO
INTR_VEC 0x15,PUSH_ZERO
INTR_VEC 0x16,PUSH_ZERO
INTR_VEC 0x17,PUSH_ZERO 
INTR_VEC 0x18,DO_NOP
INTR_VEC 0x19,PUSH_ZERO
INTR_VEC 0x1a,DO_NOP
INTR_VEC 0x1b,DO_NOP 
INTR_VEC 0x1c,PUSH_ZERO
INTR_VEC 0x1d,DO_NOP
INTR_VEC 0x1e,DO_NOP
INTR_VEC 0x1f,PUSH_ZERO 
INTR_VEC 0x20,PUSH_ZERO 
; -------------------------------------------------------------------------
;   Part 2
; -------------------- Table Page for the interrupt for kernel ---------------

; -------------------- Table Page for the interrupt End ----------------------
