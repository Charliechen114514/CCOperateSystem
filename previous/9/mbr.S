; Main Guide
%include "boot.inc"
SECTION MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax

; --------CLEAR SCREEN-----------------
; INT 10, functional 0x06 
; -------------------------------------
mov ax, 0600
mov bx, 0700
mov cx, 0
mov dx, 184f
int 10h

; outPutBytes
mov byte [gs:0x00], 'W'
mov byte [gs:0x01], 0xA4
mov byte [gs:0x02], 'e'
mov byte [gs:0x03], 0xA4
mov byte [gs:0x04], 'l'
mov byte [gs:0x05], 0xA4
mov byte [gs:0x06], 'c'
mov byte [gs:0x07], 0xA4
mov byte [gs:0x08], 'o'
mov byte [gs:0x09], 0xA4
mov byte [gs:0x0a], 'm'
mov byte [gs:0x0b], 0xA4
mov byte [gs:0x0c], 'e'
mov byte [gs:0x0d], 0xA4
mov byte [gs:0x0e], ' '
mov byte [gs:0x0f], 0xA4
mov byte [gs:0x10], 'P'
mov byte [gs:0x11], 0xA4
mov byte [gs:0x12], 'r'
mov byte [gs:0x13], 0xA4
mov byte [gs:0x14], 'e'
mov byte [gs:0x15], 0xA4
mov byte [gs:0x16], 's'
mov byte [gs:0x17], 0xA4
mov byte [gs:0x18], 'e'
mov byte [gs:0x19], 0xA4
mov byte [gs:0x1a], 'r'
mov byte [gs:0x1b], 0xA4
mov byte [gs:0x1c], 'v'
mov byte [gs:0x1d], 0xA4
mov byte [gs:0x1e], 'e'
mov byte [gs:0x1f], 0xA4
mov byte [gs:0x20], 'd'
mov byte [gs:0x21], 0xA4
mov byte [gs:0x22], ' '
mov byte [gs:0x23], 0xA4
mov byte [gs:0x24], 'M'
mov byte [gs:0x25], 0xA4
mov byte [gs:0x26], 'o'
mov byte [gs:0x27], 0xA4
mov byte [gs:0x28], 'd'
mov byte [gs:0x29], 0xA4
mov byte [gs:0x2a], 'u'
mov byte [gs:0x2b], 0xA4
mov byte [gs:0x2c], 'l'
mov byte [gs:0x2d], 0xA4
mov byte [gs:0x2e], 'e'
mov byte [gs:0x2f], 0xA4
mov byte [gs:0x30], '!'
mov byte [gs:0x31], 0xA4

mov eax, LOADER_START_SECTOR;起始扇区地址
mov bx, LOADER_BASE_ADDR; 写入的地址

mov cx, 4;待读入的扇区个数
call rd_disk_m_16
jmp LOADER_BASE_ADDR

;---------------------------
;LOAD DISK
;---------------------------
rd_disk_m_16:
;eax LBA ADDR； BX where shall we write into CX : amounts of disks
    mov esi, eax
    mov di, cx; Save the important index
    ; First:set the read amount
    mov dx, 0x1f2
    mov al, cl
    out dx, al
    mov eax, esi

    ;LBA 0-7 bits to 0x1f3
    mov dx, 0x1f3
    out dx, al

    ; LBA 8-15 to 0x1f4
    mov cl, 8
    shr eax, cl ; shr means shift right cl bits
    mov dx, 0x1f4
    out dx, al

    ; LBA 16-23
    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or  al, 0xe0; 1110
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x1f7
    mov al, 0x20;输入写指令
    out dx, al

    ;check the state
.not_ready:
    nop
    in al, dx
    and al, 0x88; 1000 1000检查第三位是不是1,是1表示准备结束
    cmp al, 0x08
    jnz .not_ready

    ; ready to ready from 0x1f0
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
    ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa
    
