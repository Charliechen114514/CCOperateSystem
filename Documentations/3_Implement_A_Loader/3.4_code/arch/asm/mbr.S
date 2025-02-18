%include "asm/boot.inc"
%include "asm/print.inc"
section mbr vstart=MBR_VSTART
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

; Print the Hello Strings on the screen!
; 设置光标到第一行、第一列 (DH = 0, DL = 0)
    mov si, welcome     ; 将字符串的地址传递给 si
    mov cx, 0
    call mbr_print_strings  ; string printer
    ; After this, we shell check the hard disk for setup
    ; set the cursor to the next line
    mov si, tell_check_disk
    mov cx, 1
    call mbr_print_strings

; check the disk
    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, 4
    call check_disk
    
    mov si, tell_check_disk_done
    ; check more disk
    mov cx, 2
    call mbr_print_strings
    
    ; goto the loader sectors
    jmp JMP_TO_LOADER
; ------------------------------------------
; declear functions
; ------------------------------------------

; --------------------------
; check_disk
; eax, LOADER BASE ADDRESS
; bx, waiting write address
; cx, read sectors size
; --------------------------
check_disk:
    ; convinient save by register operations
    push esi
    push di
    
    mov esi, eax
    mov di, cx

    ; tell the hard disk port to read the sector
    mov dx, 0x01f2
    mov al, cl
    out dx, al

    ; resume the eax
    mov eax, esi

    ; save the LBA to the 0x1f3 ~ 0x1f6

    mov dx, 0x01f3
    out dx, al

    mov cl, 8
    shr eax, cl
    mov dx, 0x01f4
    out dx, al

    shr eax, cl
    mov dx, 0x01f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or al, 0xe0 ; ser the lba mode
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x01f7
    mov al, 0x20    ; read the data
    out dx, al

.disk_not_ready:
    nop ; terminate for 1 clock 
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .disk_not_ready

    ; now ready

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x01f0

.read_data:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .read_data

    pop di
    pop esi
    ret

; ------------------------------------------
; mbr_print_strings
; 函数名: mbr_print_strings
; 参数: si - 字符串地址
; 参数: cl - 行号
; ------------------------------------------
mbr_print_strings:
    push di            ; 保存 di 寄存器
    push ax
    mov ax, NEW_LINE_LENGTH
    mov di, 0x00       ; 设置 gs 段的偏移地址为 0
    mul cl
    add di, ax

store_loop:
    lodsb              ; 加载一个字符到 al，并更新 si
    or al, al          ; 检查字符是否为 0（字符串结束符）
    jz done_print      ; 如果字符为 0，结束循环

    ; load chars
    mov [gs:di], al    ; 将字符存储到 gs:di
    inc di             ; 更新 di，指向下一个存储位置

    ; load styles
    mov byte [gs:di], PRINT_STYLE
    inc di             ; 更新 di，指向下一个存储位置

    jmp store_loop     ; 继续处理下一个字符

done_print:
    pop ax             ; resume ax
    pop di             ; resume di
    ret                ; 返回
; ------------------------------------
; tell the data
; ------------------------------------
welcome                 db "Booting From MBR...", 0
tell_check_disk         db "Ready to check the disk!", 0
tell_check_disk_done    db "Disk Check work done", 0
times 510 - ($ - $$) db 0
PLACE_MBR_MAGIC
