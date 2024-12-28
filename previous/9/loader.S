%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

; gdt create
GDT_BASE:               dd 0x0000_0000
                        dd 0x0000_0000


CODE_DESCRIPTOR         dd 0x0000_FFFF
                        dd DESCRIBTOR_CODE_HIGH4

DATA_STACK_DESCRIPTOR:  dd 0x0000_FFFF
                        dd DESCRIBTOR_DATA_HIGH4

VIDEO_DESCRIPTOR:       dd 0x8000_0007; limit = 0xbffff - 0xb8000/4K = 7; text_mode specific
                        dd DESCRIBTOR_VIDEO_HIGH4

GDT_SIZE                equ $ - GDT_BASE
GDT_LIMIT               equ GDT_SIZE - 1
times 60 dq 0; 60 decriptors ;DQ 定义的变量为4字型（8字节）Define Quadra Word
SELECTOR_CODE           equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA           equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO          equ (0x0003 << 3) + TI_GDT + RPL0
; gdt pointer
GDT_PTR                 dw GDT_LIMIT; GDT_Reg 48 bits == 1 w + 1 dw
                        dd GDT_BASE

loader_msg              db '2 loader in real mode!'

loader_start:
;--------------------------------------------------------------------------
;INT 10; | 0x13 functional | print string!
;--------------------------------------------------------------------------
; AH = 13H
; BH = page code
; BL =  attribute
; CX = string length
;(DH, DL):POS
; ES:BP string addr
; AL: ways of output
mov sp, LOADER_BASE_ADDR
mov bp, loader_msg
mov cx, 22
mov ax, 0x1301
mov bx, 0x001f
mov dx, 0x1800
int 0x10

; ----------------LOAD_TO_PRESERVED_MODE--------------------------------
; Open A20
; Load GDT
; CR0 set first bet Page Enable 1
; ----------------LOAD_TO_PRESERVED_MODE--------------------------------
in al, 0x92
or al, 0000_0010B
out 0x92, al

lgdt [GDT_PTR]

mov eax, cr0
or  eax, 0x0000_00001
mov cr0, eax

jmp dword   SELECTOR_CODE:p_mode_start


[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax
    mov byte [gs:0xB0], 'P'
    jmp $





