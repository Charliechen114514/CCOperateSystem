; mbr.S

; 主引导程序
; --------------------------------------------------
%include "boot.inc";
;LOADER_BASE_ADDR equ 0x900  将loader加载到内存0x900
;LOADER_START_SECTOR equ 0x2  loader位于磁盘第2块扇区

SECTION MBR vstart=0x7c00 ; 把起始地址编译为 0x7c00
    mov ax, cs     ; cs 代码段寄存器
    mov ds, ax     ; dx 数据段寄存器
    mov es, ax     ; es 附加段寄存器
    mov ss, ax     ; ss 堆栈段寄存器
    mov fs, ax     ; fs 80386 后添加的寄存器，无全称
    mov sp, 0x7c00 ; sp 堆栈指针寄存器
     
    mov ax, 0xb800;
    mov gs, ax;
	
; 清屏
; --------------------------------------------------
; INT 0x10    功能号: 0x06    功能描述：上卷窗口
; --------------------------------------------------
; 输入：
; AH 功能号 = 0x06
; AL = 上卷的行数(如果为0，表示全部)
; BH = 上卷行属性
; (CL, CH) = 窗口左上角的 (X, Y) 位置
; (DL, DH) = 窗口右下角的 (X, Y) 位置
; 无返回值:
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0
    mov dx, 0x184f ; 右下角: (80, 25)
                   ; VGA 文本模式种，一行只能容纳 80 个字符，共 25 行
                   ; 下标从 0 开始，所以 0x18=24, 0x4f=79
        
    int 0x10       ; int 0x10


	;;;;;下面代码是新增功能;;;;;;;;;;;;;;;;;;;;
	mov eax, LOADER_START_SECTOR; 磁盘中loader的LBA地址
	mov bx, LOADER_BASE_ADDR;      loader加入内存的起始地址	
	mov cx, 4; 待读入内存的扇区数
	call rd_disk_m_16;

	jmp LOADER_BASE_ADDR;
;--------------------------------------------------------------
;功能：读取硬盘n个扇区
	rd_disk_m_16:
;----------------------------------------------------------
	mov esi, eax;   备份eax  al在in/out指令会被使用	
	mov di, cx;    备份cx cl会在接下来代码中频繁使用
;读写硬盘:
;第一步：设置要读取的扇区数 1
	mov dx, 0x1f2; 配置的硬盘是ata0-master 是Primary通道  主盘
		       ;sector count寄存器是0x1f2端口
	mov al, cl;
	out dx, al; 从内存把扇区数1输出到端口号0x1f2; 
	mov eax, esi; 恢复eax

;第二步 将LBA地址存入0x1f3~0x1f6	
;LBA地址7~0位写入端口0x1f3
	mov dx, 0x1f3;
	out dx, al;

;LBA地址15~8位写入端口0x1f4
	mov cl, 8;
	shr eax, cl;
	mov dx, 0x1f4;
	out dx, al;

;LBA地址23~16位写入端口0x1f5
	shr eax, cl;
	mov dx, 0x1f5;
	out dx, al;

;LBA地址24~27位写入端口0x1f6
	shr eax, cl;
	and al, 0x0f;  与运算al中低四位为LBA地址24~27位
	or al, 0xe0; 或运算拼出0x1f6的高四位1110 第6位为1表示LBA
	mov dx, 0x1f6; 
	out dx, al;

;第三步 向0x1f7端口写入读命令,0x20  
	mov dx, 0x1f7;
	mov al, 0x20;
    out dx, al;  命令 写入端口0x1f7后，硬盘立即开始工作，将数据放入硬	
;	盘控制器的缓冲区

;第四步 检测硬盘状态,判断loader是否已经读入0x1f0端口中
  .not_ready:
	nop;
	in al, dx; 将端口0x1f7的status写入al;
	and al, 0x88; 获得status的第3位和第7位;
	cmp al, 0x08; 与第3位相减作比较 会影响ZF CF PF
  	jnz .not_ready;   ZF不等于0就跳,相当于循环等缓冲区中的数据准备好为止

;第5步 将0x1f0端口的数据搬向内存
  ;5.1 计算搬运次数
	mov ax, di;	
	mov dx, 256;	0x1f0端口是16比特
	mul dx;	    di*512字节/2字节=搬运次数  16位乘法乘积32位
		;高16位在dx,低16位在ax;
	mov cx, ax;   dx=1 乘积高16位是0,故把低16位移入cx
  ;5.2 循环搬运至内存         
	mov dx, 0x1f0;
  .go_on_read:     ;我们的loader只有一个扇区512字节
    in ax, dx;		;bx的寻址范围位64KB 65536字节	
	mov [bx], ax;	该循环不能加载大于64KB的程序于内存
	add bx, 2;
	loop .go_on_read; cx不等于0 就回到循环处继续搬
	ret    ;搬运完loader 回到LOADER_BASE_ADDR;

  
  times 510-($-$$) db 0 ; 填充文件末尾的魔数 0xaa55 和当前位置之间的空间
                          ; 保证编译后生成的文件大小为 512 字节（硬盘一个扇区的大小）
    db 0x55, 0xaa


