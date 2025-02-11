;一一一一一一loader和 kernel

LOADER_BASE_ADDR equ 0x900 
LOADER_START_SECTOR equ  0x2 
 
;一一一一一一－ gdt 描述符属性 一一一一一一 
DESC_G_4K equ 1000_0000_0000_0000_0000_0000b   ;G位为第23位,置1代
												;表
;段界限为单位4k
DESC_D_32 equ 1_00_0000_0000_0000_0000_0000b ;D/B 宇段,第22位
							;对代码段来说是D位,置1表示指令中的有效地址及
							;操作数是32位，指令有效地址用EIP寄存器。

DESC_L equ 0_0000_0000_0000_0000_0000_0000b ; 64位代码标记,我们在32位CPU下编程，
;标记为0便可
DESC_AVL equ 0_0000_0000_0000_0000_0000b ;CPU不用此位，暂置为
DESC_LIMIT_CODE2 equ 1111_0000_0000_0000_0000b  ;段界限16~19位
				;全设为1，它在下面代码中会与段界限的0~15位拼成0xFFFF，
				;0xFFFF*4k等于4G，段基址设为0，采用平坦模型

DESC_LIMIT_DATA2 equ DESC_LIMIT_CODE2 ;
DESC_LIMIT_VIDEO2 equ 0000_0000_0000_0000_0000b ;
DESC_P equ 1_000_0000_0000_0000b   ;第15位，表示段存在
DESC_DPL_0 equ 00_0_0000_0000_0000b ;DPL在13~14位 0为最高特权级
DESC_DPL_1 equ 01_0_0000_0000_0000b 
DESC_DPL_2 equ 10_0_0000_0000_0000b 
DESC_DPL_3 equ 11_0_0000_0000_0000b 
DESC_S_CODE equ 1_0000_0000_0000b ; S为0时表示系统段， S为1时表示非系统段。
DESC_S_DATA equ DESC_S_CODE 
DESC_S_sys equ 0_0000_0000_0000b 
DESC_TYPE_CODE equ 1000_0000_0000b ;x=1，c=0, r=0,a=0 ，即代码段是可执行的，非一致
;性，不可读，己访问位a清0 配合S使用
DESC_TYPE_DATA equ 0010_0000_0000b ;
;x=0,e=0,w=1,a=0 数据段是不可执行的，向上扩展的，可写，己访问位a清0。

DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
DESC_P + DESC_DPL_0 + DESC_S_CODE +\
DESC_TYPE_CODE + 0x00 

DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 +\
DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
DESC_P + DESC_DPL_0 + DESC_S_DATA + \
DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 +\
DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + \
DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B   ;注意书上这里写的是0x00，写错了
;显存起始地址应该是0xB8000

 
;一一一一一一 选择子属性一一一一一一一
RPL0 equ 00b 
RPL1 equ 01b 
RPL2 equ 10b 
RPL3 equ 11b 
TI_GDT equ 000b 
TI_LDT equ 100b

