%include "boot.inc"
%include "gpt.inc"
%include "print.inc"
%include "detect_memory.inc"
%include "pagetable.inc"
section loader vstart=LOADER_BASE_ADDR
    jmp loader_start

GDT_BASE:
    dd 0x0000_0000
    dd 0x0000_0000
CODE_DESC:
    dd 0x0000_ffff
    dd DESC_CODE_HIGH4
DATA_STACK_DESC:
    dd 0x0000_ffff
    dd DESC_DATA_HIGH4

VIDEO_DESC:
    dd 0x8000_0007 ; limit is 7
    dd DESC_VIDEO_HIGH4

GDT_SIZE equ $ - GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1
times 60 dq 0
SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0
; defined at here, it is markable that 
; total_mem_bytes is location-related
; check the disasm for checking the real position

; total_mem_bytes locate for 4 bytes
total_mem_bytes dd 0x12345678

; GDT_PTR requires 6 bytes
GDT_PTR dw GDT_LIMIT
        dd GDT_BASE

; ARDS buffers
; at here we gain the 4 + 6 + 244 + 2 = 256bytes 
ards_buf times 244  db 0
ards_nr             dw 0

LOADER_STACK_TOP    equ LOADER_BASE_ADDR
enter_loader        db "Enter the Loader Sections Success!", 0
start_check_mem     db "Detect Memory operation start...", 0
using_e820          db "Using E820 Method to detect memory...", 0
using_e801          db "Using E801 Method to detect memory As previous failed...", 0
using_88            db "Using 88 Method to detect memory As previous E801 failed...", 0
memory_detect_error db "Can not Detect Memory... machine panic!", 0
memory_check_done   db "Memory Check Work Done!", 0


loader_start:
    mov si, enter_loader
    mov cx, 4
    call loader_print_strings

    mov si, start_check_mem
    mov cx, 5
    call loader_print_strings
    ; logging for using e820
    mov si, using_e820
    mov cx, 6
    call loader_print_strings
    ; ready to check the physical memory
    xor ebx, ebx    ; clear the ebx as required
    mov edx, E820Magic
    mov di,  ards_buf
.E820_get_phy_mem_loop:
    ; start
    mov eax, E820SubFunc
    mov ecx, 20
    int 0x15
    jc .try_e801
    add di, cx
    inc word [ards_nr]
    cmp ebx, 0
    jnz .E820_get_phy_mem_loop
    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx
.find_max_mem_area:
    mov eax, [ebx]
    add eax, [ebx + 8]
    add ebx, 20
    cmp edx, eax
    jge .next_ards
    mov edx, eax
.next_ards:
    loop .find_max_mem_area
    ; now we fetch the address at edx,
    ; this can be checked using bochs debug using 'r'
    jmp .check_phy_mem_done

.try_e801:
    ; logging for using e801
    mov si, using_e801
    mov cx, 6
    call loader_print_strings

    mov ax, E801Magic
    int 0x15
    jc .try88

    mov cx, 0x400
    mul cx
    shl edx, 16
    and eax, 0x0000_ffff
    or edx, eax
    add edx, 0x10_0000
    mov esi, edx
; convert to bytes counts
    xor eax, eax
    mov ax, bx
    mov ecx, 0x10_0000
    mul ecx
    add esi, eax
    mov edx, esi
    jmp .check_phy_mem_done

.try88:
    mov si, using_88
    mov cx, 6
    call loader_print_strings

    mov ah, 0x88
    int 0x15
    jc .error_occur
    and eax, 0x0000_ffff
    mov cx, 0x400
    mul cx
    shl edx, 16
    or edx, eax
    add edx, 0x10_0000
    jmp .check_phy_mem_done

.error_occur:
    ; logging for using e801
    mov si, memory_detect_error
    mov cx, 7
    call loader_print_strings
    Terminate

.check_phy_mem_done:
    mov [total_mem_bytes], edx
    ; logging for using e801
    mov si, memory_check_done
    mov cx, 7
    call loader_print_strings
    ; open the A20
    in al, 0x92
    or al, 0000_0010b
    out 0x92, al

    lgdt [GDT_PTR]
    mov eax, cr0
    or eax, 0x0000_0001
    mov cr0, eax
    jmp dword SELECTOR_CODE:protect_mode_start

; ------------------------------------------
; mbr_print_strings
; 函数名: mbr_print_strings
; 参数: si - 字符串地址
; 参数: cl - 行号
; ------------------------------------------
loader_print_strings:
    push di            ; 保存 di 寄存器
    push ax
    mov ax, 0xA0
    mov di, 0x00       ; 设置 gs 段的偏移地址为 0
    mul cl
    add di, ax

loader_store_loop:
    lodsb                   ; 加载一个字符到 al，并更新 si
    or al, al               ; 检查字符是否为 0（字符串结束符）
    jz loader_done_print    ; 如果字符为 0，结束循环

    ; 存储字符到 [gs:di]
    mov [gs:di], al    ; 将字符存储到 gs:di
    inc di             ; 更新 di，指向下一个存储位置

    ; 存储 0xA4 到 [gs:di]
    mov byte [gs:di], PRINT_STYLE
    inc di             ; 更新 di，指向下一个存储位置

    jmp loader_store_loop     ; 继续处理下一个字符

loader_done_print:
    pop ax             ; resume ax
    pop di             ; resume di
    ret                ; 返回

[bits 32]
protect_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax

    mov es, ax
    mov ss, ax

    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov esi, eax
    mov di, cx

    mov dx, 0x1f2
    mov al, cl
    out dx, al

    mov eax, esi

    mov dx, 0x1f3
    out dx, al

    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

.not_ready:
    nop
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .not_ready

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x1f0

.go_on_read:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .go_on_read

    call setup_page_table
    sgdt [GDT_PTR]
    mov ebx, [GDT_PTR + 2]
    or dword [ebx + 0x18 + 4], 0xc000_0000
    add dword [GDT_PTR + 2], 0xc000_0000
    add esp, 0xc000_0000
    mov eax, PAGE_TABLE_POSITION
    mov cr3, eax

    mov eax, cr0
    or  eax, 0x8000_0000
    mov cr0, eax

    lgdt [GDT_PTR]
    ; 初始化kernel
    jmp SELECTOR_CODE:enter_kernel

enter_kernel:
    call kernel_init
    mov esp, 0xc009f000
    jmp KERNEL_ENTRY_POINT

    Terminate

setup_page_table:
    mov ecx, PAGE_SIZE
    mov esi, 0
clear_up:
    ; clear the page table
    mov byte [PAGE_TABLE_POSITION + esi], 0
    inc esi
    loop clear_up
create_pde:
    mov eax, PAGE_TABLE_POSITION
    add eax, PAGE_SIZE
    mov ebx, eax
    or eax, PAGE_USER | PAGE_WRITE | PAGE_TABLE_P
    mov [PAGE_TABLE_POSITION + 0x0], eax
    mov [PAGE_TABLE_POSITION + 0xc00], eax
    sub eax, PAGE_SIZE
    mov [PAGE_TABLE_POSITION + 4092], eax

    mov ecx, 256
    mov esi, 0
    mov edx, PAGE_USER | PAGE_WRITE | PAGE_TABLE_P
create_pte:
    mov [ebx + esi * 4], edx
    add edx, PAGE_SIZE
    inc esi
    loop create_pte

    mov eax, PAGE_TABLE_POSITION
    add eax, PAGE_SIZE * 2
    or eax,  PAGE_USER | PAGE_WRITE | PAGE_TABLE_P
    mov ebx, PAGE_TABLE_POSITION
    mov ecx, 254
    mov esi, 769
.create_kernel_pde:
    mov [ebx + esi * 4], eax
    inc esi
    add eax, PAGE_SIZE
    loop .create_kernel_pde
    ret

kernel_init:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov dx, [KERNEL_BIN_BASE_ADDR + 42]
    mov ebx, [KERNEL_BIN_BASE_ADDR + 28]

    add ebx, KERNEL_BIN_BASE_ADDR
    mov cx, [KERNEL_BIN_BASE_ADDR + 44]

.each_segment:
    cmp byte [ebx], PT_NULL
    je .PTNULL

    ; 准备mem_cpy参数
    push dword [ebx + 16]
    mov eax, [ebx + 4]
    add eax, KERNEL_BIN_BASE_ADDR
    push eax
    push dword [ebx + 8]

    call mem_cpy
    add esp, 12

.PTNULL:
    add ebx, edx
    loop .each_segment
    ret

mem_cpy:
    cld
    push ebp
    mov ebp, esp
    push ecx

    mov edi, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    rep movsb

    pop ecx
    pop ebp
    ret