%include "boot.inc"
%include "gpt.inc"
%include "print.inc"
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

GDT_PTR dw GDT_LIMIT
        dd GDT_BASE

LOADER_STACK_TOP    equ LOADER_BASE_ADDR
enter_loader        db "Enter the Loader Sections Success!", 0

loader_start:
    mov si, enter_loader
    mov cx, 4
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

    push di
    SET_PROTECT_PRINT_LINE 5
    print_hello_os_protect_mode
    SET_PROTECT_PRINT_LINE 6
    print_protect_mode_state

    pop di
    Terminate
