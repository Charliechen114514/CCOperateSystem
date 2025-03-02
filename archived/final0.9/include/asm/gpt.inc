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
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
    DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
    DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + \
    DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

; Selector Property
RPL0    equ 00b
RPL1    equ 01b
RPL2    equ 10b
RPL3    equ 11b
TI_GDT  equ 000b
TI_LDT  equ 100b

SELECTOR_CODE           equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA           equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO          equ (0x0003 << 3) + TI_GDT + RPL0


; This macro controls the generations of GPT Table
%macro      GPT_TABLE  0
;构建gdt及其内部的描述符
GDT_BASE:       
            dd    0x00000000 
            dd    0x00000000
CODE_DESC:  
            dd    0x0000FFFF 
            dd    DESC_CODE_HIGH4

DATA_STACK_DESC:  
            dd    0x0000FFFF
            dd    DESC_DATA_HIGH4

VIDEO_DESC: 
            dd    0x80000007	        ; limit=(0xbffff-0xb8000)/4k=0x7
            dd    DESC_VIDEO_HIGH4      ; 此时dpl为0

GDT_SIZE    equ   $ - GDT_BASE
GDT_LIMIT   equ   GDT_SIZE -	1 

; reserved 60 empty slots
times 60 dq 0					 
%endmacro

%macro  KERNEL_INFO 0
; memorize the memory we owns, we will setup user pool then
total_mem_bytes dd 0					 
; gpt ptr base
GDT_PTR  
    dw  GDT_LIMIT 
    dd  GDT_BASE
    
; kernel infos
; buf for the 244 and record how many ards buffer actually
ards_buf times 244 db 0
ards_nr dw 0		         
%endmacro

