; Provides basic includes
; As Intel Programming scripts infers
; MBR is placed in 0x7c00 at physical address
; so I Place it at here, as the boot code
%define MBR_VSTART              0x7c00

; Set Terminate
%define Terminate               jmp $

; Magic numbers of MBR is 0xaa55 for human readable
%define PLACE_MBR_MAGIC         db 0x55, 0xaa

; Boot Real Mode Print Mode
;   ---------------------------------
;   | K | R | G | B | I | R | G | B |
;   ---------------------------------
%define PRINT_STYLE             0x09

; sector macros
%define LOADER_BASE_ADDR        0x900
%define LOADER_START_SECTOR     0x2

%define KERNEL_BIN_SECTOR       0x9
%define KERNEL_ENTRY_POINT      0xc0001500
%define KERNEL_BIN_BASE_ADDR    0x70000
%define PT_NULL                 0   