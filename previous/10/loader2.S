；正确的代码
%include "boot2.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

; construct gdt and its inner-descriptor
GDT_BASE: dd 0x00000000
		  dd 0x00000000

CODE_DESC: dd 0x0000FFFF
		   dd DESC_CODE_HIGH4

DATA_STACK_DESC: dd 0x0000FFFF
				 dd DESC_DATA_HIGH4

VIDEO_DESC: dd 0x80000007
			dd DESC_VIDEO_HIGH4

GDT_SIZE equ $ - GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1
times 59 dq 0
times 5 db 0

total_mem_bytes dd 0	; save memory capacity

; gdt's pointer
gdt_ptr dw GDT_LIMIT
		dd GDT_BASE

ards_buf times 244 db 0 ; buffer_size
ards_nr dw 0 ; buffer_num

SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

loader_start:

xor ebx, ebx
mov di, ards_buf
.e820_mem_get_loop:
	mov eax, 0x0000e820
	mov edx, 0x534d4150
	mov ecx, 20
	int 0x15
	add di, cx
	inc word [ards_nr]
	cmp ebx, 0
	jne .e820_mem_get_loop

	mov cx, [ards_nr]
	mov ebx, ards_buf
	xor edx, edx
.find_max_mem_area:
	mov eax, [ebx]
	add eax, [ebx+8]
	add ebx, 20
	cmp edx, eax	; if ebx >= eax: continue, else ebx = eax
	jge .next_ards
	mov edx, eax
.next_ards:
	loop .find_max_mem_area
	jmp .mem_get_ok

.mem_get_ok:
	mov [total_mem_bytes], edx

; ========= ready to enter protection mode ? =========
; =========           1. open A20            =========
; =========           2. load gdt            =========
; =========           3. set pe = 1         =========

	;================ 1. open A20 ===============
	in al, 0x92
	or al, 0000_0010b
	out 0x92, al

	;================ 2. load GDT ===============
	lgdt [gdt_ptr]

	; =============== 3. set pe = 1 =============
	mov eax, cr0
	or eax, 0x00000001
	mov cr0, eax

	jmp dword SELECTOR_CODE:p_mode_start

[bits 32]
p_mode_start:
	mov ax, SELECTOR_DATA
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov esp, LOADER_STACK_TOP
	mov ax, SELECTOR_VIDEO
	mov gs, ax

	mov byte [gs:160], 'P'	

jmp $

