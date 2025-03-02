; page table is placed at 1MB
%define PAGE_TABLE_POSITION    0x100000
%define PAGE_SIZE              4096
; --------- Page Table Properties --------
PAGE_TABLE_P        equ     1b
PAGE_READ           equ     0000_0000b
PAGE_WRITE          equ     0000_0010b
PAGE_USER           equ     0000_0100b
PAGE_SYSTEM         equ     0000_0000b
