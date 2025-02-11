section .data
STR_C_LIB: db "c lib says: hello world!", 0xa
STR_C_LIB_LEN equ $ - STR_C_LIB

STR_SYSCALL: db "syscall says: hello world!", 0xa
STR_SYSCALL_LEN equ $ - STR_SYSCALL

section .text
global _start
_start:
; ------- C Like Caller -------------
    push STR_C_LIB_LEN
    push STR_C_LIB
    push 1

    call simu_write
    add esp, 12

; use syscall directly
    mov eax, 4 ; write's syscall num is 4
    mov ebx, 1
    mov ecx, STR_SYSCALL
    mov edx, STR_SYSCALL_LEN
    int 0x80
;;;-----Exit then
    mov eax, 1; 0x80 of 1 is exit
    int 0x80

simu_write:
    push ebp
    mov ebp, esp
    mov eax, 4
    mov ebx, [ebp + 8]
    mov ecx, [ebp + 12]
    mov edx, [ebp + 16]
    int 0x80
    pop ebp
    ret