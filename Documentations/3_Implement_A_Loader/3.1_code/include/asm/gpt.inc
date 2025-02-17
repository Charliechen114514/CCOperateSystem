; -----------------------------------
;   gpt.inc promised the gpt settings
; -----------------------------------
; the section's size per is 4K
DESC_G_4K       equ     (1 << 23)

; Running in the 32bits mode
DESC_D_32       equ     (1 << 22)
; Disabling as we run in 32bits mode, code page is 32 bits
DESC_L          equ     (0 << 21)
; For fun, no usage here :)
DESC_AVL        equ     (0 << 20)
; (4GB / 4KB) - 1 = 0xFFFFF，Segment limits is 0xFFFF
DESC_LIMIT_CODE2 equ    11110_00000_00000_00000b
DESC_LIMIT_DATA2 equ    DESC_LIMIT_CODE2
; base addr of Protect mode is 0xb8000, set as here
DESC_LIMIT_VIDEO2 equ   1011b

; the property of the description tables
; TIPS, MY EYES SUCKS FOR COUNTING THE 0 REQUIRED
; So I put it at here :)
DESC_P          equ     1000_0000_0000_0000b
DESC_DPL_0      equ     000_000000000000b
DESC_DPL_1      equ     010_000000000000b
DESC_DPL_2      equ     100_000000000000b
DESC_DPL_3      equ     110_000000000000b
DESC_S_CODE     equ     1_0000_0000_0000b
DESC_S_DATA     equ     DESC_S_CODE
DESC_S_SYS      equ     00000_0000_0000b
DESC_TYPE_CODE  equ     1000_0000_0000b
DESC_TYPE_DATA  equ     0010_0000_0000b

; Compose the type for the usage in loader.S
; Define segment descriptor properties for code, data, and video
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
    DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
; Code segment descriptor:
; - The base is shifted left 24 bits (0x00 << 24)
; - Granularity is set to 4K (DESC_G_4K)
; - 32-bit operand size (DESC_D_32)
; - 32-bit segment limit (DESC_L)
; - Available for system use (DESC_AVL)
; - Code limit set to 2 (DESC_LIMIT_CODE2)
; - Present flag (DESC_P)
; - Descriptor privilege level 0 (DESC_DPL_0)
; - Code segment (DESC_S_CODE)
; - Type of segment is code (DESC_TYPE_CODE)

DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
    DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
; Data segment descriptor:
; - Similar structure as the code segment but for data segment (DESC_S_DATA and DESC_TYPE_DATA)

DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + \
    DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
; Video segment descriptor:
; - Similar structure to the data segment but with a different limit (DESC_LIMIT_VIDEO2)

; Selector Property Definitions
RPL0    equ 00b        ; Requestor Privilege Level 0 (highest privilege)
RPL1    equ 01b        ; Requestor Privilege Level 1
RPL2    equ 10b        ; Requestor Privilege Level 2
RPL3    equ 11b        ; Requestor Privilege Level 3 (lowest privilege)

TI_GDT  equ 000b       ; Table Indicator for GDT (Global Descriptor Table)
TI_LDT  equ 100b       ; Table Indicator for LDT (Local Descriptor Table)

; Selector values for different segments
SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
; Code segment selector (index 1 in GDT, TI_GDT, RPL0)

SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
; Data segment selector (index 2 in GDT, TI_GDT, RPL0)

SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0
; Video segment selector (index 3 in GDT, TI_GDT, RPL0)


%macro GPT_TABLE 0  
    GDT_BASE:
        dd 0x0000_0000                      ; GDT Base Must be dummy
        dd 0x0000_0000                      ; I say it at bonus/ProtectMode.md
    CODE_DESC:
        dd 0x0000_ffff                      ; Set code descriptor limit to 0xffff
        dd DESC_CODE_HIGH4                  ; Set code descriptor attributes
    DATA_STACK_DESC:
        dd 0x0000_ffff                      ; Set data and stack descriptor limit to 0xffff
        dd DESC_DATA_HIGH4                  ; Set data and stack descriptor attributes
    
    VIDEO_DESC:
        dd 0x8000_0007                      ; Set video descriptor limit to 7
        dd DESC_VIDEO_HIGH4                 ; Set video descriptor attributes
    
    GDT_SIZE equ $ - GDT_BASE               ; Calculate the size of the GDT table
    GDT_LIMIT equ GDT_SIZE - 1              ; Set the GDT limit (size - 1)
    times 60 dq 0                           ; Reserve 60 bytes (data)
    ; Lastly, the GDT Table
    GDT_PTR dw GDT_LIMIT                    ; Set the pointer to the GDT limit
            dd GDT_BASE                     ; Set the base address of the GDT    
%endmacro

