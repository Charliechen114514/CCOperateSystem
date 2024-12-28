;------------------------
%include "boot2.inc" 
section loader vstart=LOADER_BASE_ADDR 
LOADER_STACK_TOP equ LOADER_BASE_ADDR  ;相同内存地址，地址之下便是栈
jmp loader_start 

;构建 gdt 及其内部的描述符
GDT_BASE: dd 0x00000000       ;第0个段描述符不可用
		dd 0x00000000  

CODE_DESC: dd 0x0000FFFF   ;代码段描述符
		 dd DESC_CODE_HIGH4 
 
DATA_STACK_DESC: dd 0x0000FFFF ;栈段描述符 栈段和数据段共用一个描述符 均向上扩展
		 dd DESC_DATA_HIGH4  
 
VIDEO_DESC: dd 0x80000007   ; limit=(0xbffff-0xb8000)/4k=0x7  故段界限为7
			dd  DESC_VIDEO_HIGH4  ;此时dpl为0
			 
GDT_SIZE equ $ - GDT_BASE ; 先是通过地址差来获得 GDT的大小，进而用 GDT大小减1得到了段界限
GDT_LIMIT equ GDT_SIZE - 1   ;用于构建GDTR的段界限
times 60 dq 0 ;此处预留 60 个描述符的空位

;以下是构建代码段、数据段、显存段选择子
SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0 
;相当于[(CODE_DESC - GDT_BASE) /8 ]<<3+ TI_GDT + RPL0 
;内存地址的编号是一个存储单元8比特，这里CODE_DESC - GDT_BASE应该等于8
;书里的备注应该写掉了"<<3"
SELECTOR_DATA equ (0x0002<< 3) + TI_GDT + RPL0 
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0 

;以下是 gdt 的指针即GDTR，前2字节是gdt界限，后4字节是gdt起始地址 后面代码使用lgdt指令时会用上
gdt_ptr dw GDT_LIMIT 
		dd GDT_BASE 

 loadermsg db '2 loader in real.' 

 loader_start: 
 
;------------------------------------------------------------
; INT 0x10 功能号： 0x13 功能描述：打印字符串
;------------------------------------------------------------
;输入：
;AH 子功能号＝13H
;BH ＝页码
;BL ＝属性（若 AL=OOH OlH)
;CX ＝字符串长度
; (DH DL ）＝坐标｛行、列）
;ES:BP＝字符串地址
;AL＝显示输出方式
; 一一字符串中只含显示字符，其显示属性在 BL
;显示后，光标位置不变
; 一一字符串中只含显示字符，其显示属性在 BL
;显示后，光标位置改变
; 一一字符事中含显示字符和显示属性。显示后，光标位置不变
; 一一字符串中含显示字符和显示属性。显示后，光标位置改变
;无返回值
mov sp, LOADER_BASE_ADDR 
mov bp, loadermsg ; ES:BP ＝字符串地址
mov cx, 17 ; cx ＝字符串长度
mov ax, 0x1301 ; AH = 13, AL = 01h 
mov bx, 0x001f ;页号为0(BH = 0）蓝底粉红字（ BL = 1fh) 
mov dx, 0x1800  ;  dh=0x18 十进制为24,代表行数；dl=0x00 表示列数。使用显存的文本模式下，一共25行，所以2 loader in real 会出现在屏幕最后一行
int 0x10 ; 10h号中断 由于AH=0x13，所以该BIOS中断后会执行打印字符串的中断处理程序。
 
; 一一一一一一一一一一 准备进入保护模式 一一一一一一一一一一一一一一－
;1 打开 A20
;2 加载 gdt
;3 将cr0 的 pe 位置1 

;一一一一一一一－打开 A20 一一一一一
in al,0x92
or al, 0000_0010B 
out 0x92,al 
 
;一一一一一一一一加载 GDT (也就是设置好gdtr，gdtr记录着gdt的起始地址)一一一一一一一－
lgdt [gdt_ptr ] 

;一一一一一一一一 cr0位置1 一一一一一一一－
mov eax, cr0 
or eax, 0x00000001 
mov cr0, eax 

jmp dword SELECTOR_CODE:p_mode_start  ;刷新流水线

[bits 32] 
p_mode_start: 
;;;;用选择子初始化段寄存器
	mov ax, SELECTOR_DATA 
	mov ds, ax 
	mov es, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP 
	mov ax, SELECTOR_VIDEO 
	mov gs, ax 
	
	
	mov byte [gs:0xA0], 'P' 
 
	jmp $


