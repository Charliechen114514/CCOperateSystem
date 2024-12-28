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
times 59 dq 0; 60 decriptors ;DQ 定义的变量为4字型（8字节）Define Quadra Word
times 5 db 0
total_mem_bytes dd 0 ;保存内存中的总量大小，而我们默认bin vstart = 0x900 + 0x200 = 0xb00
; why 0x200 = 512? 60 x 8(discriptor) + 32btye(Current GDT Size: 0 for ununsable, code, video and data)

GDT_PTR                 dw GDT_LIMIT; GDT_Reg 48 bits == 1 w + 1 dw
                        dd GDT_BASE
                        
; tol_mem_bytes 4 + gdt_ptr 6 + buf 244 + ards_nr 2 = 256
ards_buf times 244      db 0 ; Auto Filled the bytes
ards_nr                 dw 0 ; record the number of ARDS

SELECTOR_CODE           equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA           equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO          equ (0x0003 << 3) + TI_GDT + RPL0
; gdt pointer
loader_start:
; int 15h eax = 0000_E820, edx = 534D4150h "SMAP" as the signature to check
xor ebx, ebx
mov edx, 0x534D4150
mov di, ards_buf
.e820_mem_get_loop:
    mov eax, 0x0000_E820; caller
    mov ecx, 20 ;ARDS is 20 bytes
    int 0x15 ; call
    jc .e820_mem_failed_try_e801 ; cf is 1 to signal the error, jmp to the second method
    add di, cx
    inc word [ards_nr] ; ards_nr++
    cmp ebx, 0 ; is 0 means finish all and ready return
    jnz .e820_mem_get_loop
    ; find max 
    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx; clear edx to store the largest
.find_max_mem_area:
    mov eax, [ebx];base add low
    add eax, [ebx + 8];length_low
    add ebx, 20; next
    cmp edx, eax
; we always store largest in edx
    jge .next_ards
    mov edx, eax
.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok

; --- E801H --------
.e820_mem_failed_try_e801:
    mov eax, 0xe801
    int 0x15
    jc .e801_failed_try_88
    mov cx, 0x400
    mul cx
    shl edx, 16
    and eax, 0x0000FFFF
    or edx, eax
    add edx, 0x10_0000; ax = 15MB
    mov esi, edx; store

    ; 16MB+
    xor eax, eax
    mov ax, bx
    mov ecx, 0x10000 ; 64KB
    mul ecx     ; High in edx, low in eax
    add esi, eax
    mov edx, esi
    jmp .mem_get_ok

; ----0x88---
.e801_failed_try_88:
    mov ah, 0x88
    int 0x15
    jc .error_hlt
    and eax, 0x0000_FFFF
    mov cx, 0x400; change it to byte 
    mul cx
    shl edx, 16; dx left move to higher place
    or edx, eax; then we get dx | ax as 32bits
    add edx, 0x100000; add the bonus 1MB
.mem_get_ok:
    mov [total_mem_bytes], edx



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

.error_hlt:
	hlt


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





