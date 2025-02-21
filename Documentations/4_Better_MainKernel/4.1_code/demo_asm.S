section .data

; the string we will print
demo_str:       db "and this is is just a simple greetings!",0xA, 0
demo_str_len    equ $-demo_str


section .text
; on the demo_c.c file
extern c_print

global _start
_start:
    push demo_str             
    call c_print        
    add esp,4           
    mov eax,1           
    int 0x80           

; tell the entern so ld could find the definitions
global assembly_print

assembly_print:
    push ebp            
    mov ebp,esp

    mov eax,4           
    mov ebx,1           
    mov ecx, [ebp+8]    
    mov edx, [ebp+12]   
    int 0x80           

    pop ebp            
    ret
