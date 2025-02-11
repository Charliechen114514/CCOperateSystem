; --------------------LOADER and KERNEL---------------
LOADER_BASE_ADDR            equ 0x900
LOADER_START_SECTOR         equ 0x2

;---------------------Global Describtor Table---------

; 4k set 1 as per sector multiply 4K
DESCRIBTOR_G_4K            equ 1000_0000_0000_0000_0000_0000b
; set 1 as to indicate the operator is 32bits instead of 16 bits
DESCRIBTOR_D_32            equ  100_0000_0000_0000_0000_0000b
; No, not 64 bits but 32!
DESCRIBTOR_L               equ   00_0000_0000_0000_0000_0000b
; Currently we don't use the AVL bits, as we don't have to make it meaningful!
DESCRIBTOR_AVL             equ    0_0000_0000_0000_0000_0000b 
; the second segments limitations in high 32 bits
DESCRIBTOR_LIMIT_CODE_2    equ      1111_0000_0000_0000_0000b
; else for the data if the descriptor indicates the data seg
DESCRIBTOR_LIMIT_DATA_2    equ      DESCRIBTOR_LIMIT_CODE_2
; for Video(显存) one:
DESCRIBTOR_LIMIT_VIDEO_2   equ       000_0000_0000_0000_0000b
; P for the present, of course it must be present :)
DESCRIBTOR_P               equ           1000_0000_0000_0000b
; Following macro sets the Privilage Level of the kernel
DESCRIBTOR_DPL_0           equ            000_0000_0000_0000b
DESCRIBTOR_DPL_1           equ            010_0000_0000_0000b
DESCRIBTOR_DPL_2           equ            100_0000_0000_0000b
DESCRIBTOR_DPL_3           equ            110_0000_0000_0000b
; S stands for the basic type of the descriptor
; 1 for the user des, 0 for the system des 
DESCRIBTOR_S_CODE          equ              1_0000_0000_0000b
DESCRIBTOR_S_DATA          equ              DESCRIBTOR_S_CODE
DESCRIBTOR_S_SYS           equ              0_0000_0000_0000b
; set the type now! remember it is the X C R A for CODE X E W A for data
; for code it is executable, not the consistence, unable to read , a is for \
; having read, we haven't read it yet, set 0
DESCRIBTOR_TYPE_CODE       equ                1000_0000_0000b
; for the data. not executable, extended upper and writable
DESCRIBTOR_TYPE_DATA       equ                0010_0000_0000b

; make a sum and get the HIGH32bits
DESCRIBTOR_CODE_HIGH4      equ                (0x00 << 24) + DESCRIBTOR_G_4K + \
                                              DESCRIBTOR_D_32 + DESCRIBTOR_L + \
                                              DESCRIBTOR_AVL + DESCRIBTOR_LIMIT_CODE_2 + \
                                              DESCRIBTOR_P + DESCRIBTOR_DPL_0 + DESCRIBTOR_S_CODE + \
                                              DESCRIBTOR_TYPE_CODE + 0x00

DESCRIBTOR_DATA_HIGH4      equ                (0x00 << 24) + DESCRIBTOR_G_4K + \
                                              DESCRIBTOR_D_32 + DESCRIBTOR_L + \
                                              DESCRIBTOR_AVL + DESCRIBTOR_LIMIT_DATA_2 + \
                                              DESCRIBTOR_P + DESCRIBTOR_DPL_0 + DESCRIBTOR_S_CODE + \
                                              DESCRIBTOR_TYPE_DATA + 0x00

DESCRIBTOR_VIDEO_HIGH4     equ                (0x00 << 24) + DESCRIBTOR_G_4K + \
                                              DESCRIBTOR_D_32 + DESCRIBTOR_L + \
                                              DESCRIBTOR_AVL + DESCRIBTOR_LIMIT_VIDEO_2 + \
                                              DESCRIBTOR_P + DESCRIBTOR_DPL_0 + DESCRIBTOR_S_CODE + \
                                              DESCRIBTOR_TYPE_DATA + 0x0B  
; --------------------------Sector-------------------------
RPL0                       equ                00b
RPL1                       equ                01b
RPL2                       equ                10b
RPL3                       equ                11b
TI_GDT                     equ                000b
TI_LDT                     equ                100b                                                                               
