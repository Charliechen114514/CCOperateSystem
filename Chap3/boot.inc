; Provides basic includes
%define MBR_VSTART              0x7c00
%define STOP                    jmp $
%define PLACE_MBR_MAGIC         db 0x55, 0xaa
%define PRINT_STYLE             0x09

; sector macros
%define LOADER_BASE_ADDR        0x900
%define LOADER_START_SECTOR     0x2


%macro SET_CURSOR 2
    mov ah, 0x02     ; BIOS中断0x10的设置光标位置功能
    mov bh, 0        ; 页码0
    mov dh, %1        ; 行号0 (第一行)
    mov dl, %2        ; 列号0 (第一列)
    int 0x10         ; 调用中断
%endmacro


