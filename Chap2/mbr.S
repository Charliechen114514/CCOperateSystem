%define MBR_VSTART          0x7c00
%define STOP                jmp $
%define PLACE_MBR_MAGIC     db 0x55, 0xaa

SECTION MBR vstart=MBR_VSTART
    ; Load Segments
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, MBR_VSTART
    mov ax, 0xb800
    mov gs, ax

; clear the screen by using 0x06

    mov ax, 0x0600
    mov bx, 0x0700
    xor cx, cx
    mov dx, 0x18f4
    int 0x10

; 设置光标到第一行、第一列 (DH = 0, DL = 0)
    mov ah, 0x02     ; BIOS中断0x10的设置光标位置功能
    mov bh, 0        ; 页码0
    mov dh, 0        ; 行号0 (第一行)
    mov dl, 0        ; 列号0 (第一列)
    int 0x10         ; 调用中断

; get the cursor
    mov ah, 0x02
    mov bh, 0
    int 0x10
    ; 设置 di 为目标存储地址，假设存储从 0x00 开始
    mov di, 0x00        ; 设置 gs 段的偏移地址为 0

    ; 字符串数据
    mov si, welcome     ; 将字符串的地址传递给 si
    
store_loop:
    lodsb               ; 加载一个字符到 al，并更新 si
    or al, al           ; 检查字符是否为 0（字符串结束符）
    jz done             ; 如果字符为 0，结束循环

    ; 存储字符到 [gs:di]
    mov [gs:di], al     ; 将字符存储到 gs:di
    inc di              ; 更新 di，指向下一个存储位置

    ; 存储 0xA4 到 [gs:di]
    mov byte [gs:di], 0xA4
    inc di              ; 更新 di，指向下一个存储位置

    jmp store_loop      ; 继续处理下一个字符

done:
    STOP                ; 程序结束

welcome db "Hello, Charlie's OS", 0
welcome_len equ $ - welcome
times 510 - ($ - $$) db 0
PLACE_MBR_MAGIC