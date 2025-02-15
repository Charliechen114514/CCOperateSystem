%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
    mov si, enter_loader
    mov cx, 3
    call loader_print_strings
    STOP

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

enter_loader db "Enter the Loader Sections Success!", 0