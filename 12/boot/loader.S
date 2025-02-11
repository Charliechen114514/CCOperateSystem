%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

; gdt create
GDT_BASE:               dd 0x0000_0000
                        dd 0x0000_0000


CODE_DESCRIPTOR         dd 0x0000_FFFF
                        dd DESCRIBTOR_CODE_HIGH4

DATA_STACK_DESCRIPTOR:  dd 0x0000_FFFF
                        dd DESCRIBTOR_DATA_HIGH4

VIDEO_DESCRIPTOR:       dd 0x8000_0007; limit = 0xbffff - 0xb8000/4K = 7; text_mode specific
                        dd DESCRIBTOR_VIDEO_HIGH4

GDT_SIZE                equ $ - GDT_BASE
GDT_LIMIT               equ GDT_SIZE - 1
times 60 dq 0; 60 decriptors ;DQ 定义的变量为4字型（8字节）Define Quadra Word
total_mem_bytes dd 0 ;保存内存中的总量大小，而我们默认bin vstart = 0x900 + 0x200 = 0xb00
; why 0x200 = 512? 60 x 8(discriptor) + 32btye(Current GDT Size: 0 for ununsable, code, video and data)

GDT_PTR                 dw GDT_LIMIT; GDT_Reg 48 bits == 1 w + 1 dw
                        dd GDT_BASE
                        
; tol_mem_bytes 4 + gdt_ptr 6 + buf 244 + ards_nr 2 = 256
ards_buf times 244      db 0 ; Auto Filled the bytes
ards_nr                 dw 0 ; record the number of ARDS

SELECTOR_CODE           equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA           equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO          equ (0x0003 << 3) + TI_GDT + RPL0
; gdt pointer
loader_start:
; int 15h eax = 0000_E820, edx = 534D4150h "SMAP" as the signature to check
xor ebx, ebx
mov edx, 0x534D4150
mov di, ards_buf
.e820_mem_get_loop:
    mov eax, 0x0000_E820; caller
    mov ecx, 20 ;ARDS is 20 bytes
    int 0x15 ; call
    jc .e820_mem_failed_try_e801 ; cf is 1 to signal the error, jmp to the second method
    add di, cx
    inc word [ards_nr] ; ards_nr++
    cmp ebx, 0 ; is 0 means finish all and ready return
    jnz .e820_mem_get_loop
    ; find max 
    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx; clear edx to store the largest
.find_max_mem_area:
    mov eax, [ebx];base add low
    add eax, [ebx + 8];length_low
    add ebx, 20; next
    cmp edx, eax
; we always store largest in edx
    jge .next_ards
    mov edx, eax
.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok

; --- E801H --------
.e820_mem_failed_try_e801:
    mov eax, 0xe801
    int 0x15
    jc .e801_failed_try_88
    mov cx, 0x400
    mul cx
    shl edx, 16
    and eax, 0x0000FFFF
    or edx, eax
    add edx, 0x10_0000; ax = 15MB
    mov esi, edx; store

    ; 16MB+
    xor eax, eax
    mov ax, bx
    mov ecx, 0x10000 ; 64KB
    mul ecx     ; High in edx, low in eax
    add esi, eax
    mov edx, esi
    jmp .mem_get_ok

; ----0x88---
.e801_failed_try_88:
    mov ah, 0x88
    int 0x15
    jc .error_hlt
    and eax, 0x0000_FFFF
    mov cx, 0x400; change it to byte 
    mul cx
    shl edx, 16; dx left move to higher place
    or edx, eax; then we get dx | ax as 32bits
    add edx, 0x100000; add the bonus 1MB
.mem_get_ok:
    mov [total_mem_bytes], edx



; ----------------LOAD_TO_PRESERVED_MODE--------------------------------
; Open A20
; Load GDT
; CR0 set first bet Page Enable 1
; ----------------LOAD_TO_PRESERVED_MODE--------------------------------
in al, 0x92
or al, 0000_0010B
out 0x92, al

lgdt [GDT_PTR]

mov eax, cr0
or  eax, 0x0000_00001
mov cr0, eax

jmp dword   SELECTOR_CODE:p_mode_start

.error_hlt:
	hlt


[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov eax, KERNEL_BIN_SECTOR
    mov ebx, KERNEL_BIN_BASE_ADDR
    
    mov ecx,200
    call rd_disk_m_32
; -----------PAGE--------------
    call setup_page
    sgdt [GDT_PTR]
    mov ebx, [GDT_PTR + 2]
    or dword [ebx + 0x18 + 4], 0xc000_0000
    add dword [GDT_PTR + 2], 0xc000_0000
    add esp, 0xc0000000
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax
    
    mov eax, cr0
    or eax, 0x8000_0000
    mov cr0, eax

    mov eax, SELECTOR_VIDEO
    mov gs, ax
    lgdt [GDT_PTR]
    mov byte [gs:160], 'V'

    jmp SELECTOR_CODE:enter_kernel
; ----------------Enter kernel
enter_kernel:
    call kernel_init
    mov esp, 0xc009_f000
    jmp KERNEL_ENTRY_ADDR


; -------------------setup page table ---------
setup_page:
    mov ecx, 4096
    mov esi, 0
.clear_page_dir: ; 先把页表空间清0
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir

;PDE
.create_pde: ; Create Page Directory Entry
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000;First PDE's attr
    mov ebx, eax    ; ready for PTE

    ; kernel one and usr one

    or eax, PG_US_U | PG_RW_W | PG_P
    ; init the basic attr
    mov [PAGE_DIR_TABLE_POS + 0x0], eax
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax ; kernel
    sub eax, 0x1000
    mov [PAGE_DIR_TABLE_POS + 4092], eax; final one points to itself
; create PTE
    mov ecx, 256
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P
.create_pte:
    mov [ebx + esi*4], edx
    add edx, 4096
    inc esi
    loop .create_pte

; kernel pte
    mov eax, PAGE_DIR_TABLE_POS
    mov eax, 0x2000
    or eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 769
.create_kernel_pde:
    mov [ebx + esi*4], eax
    inc esi
    add eax, 0x1000
    loop .create_kernel_pde
    ret


kernel_init:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov ebx,[KERNEL_BIN_BASE_ADDR+28]
    add ebx,KERNEL_BIN_BASE_ADDR                               ;ebx当前位置为程序段表
    mov dx,[KERNEL_BIN_BASE_ADDR+42]		         ;获取程序段表每个条目描述符字节大小
    mov cx,[KERNEL_BIN_BASE_ADDR+44]                         ;一共有几个段
    
     
 .get_each_segment:
    cmp dword [ebx+0],PT_NULL
    je .PTNULL                                                 ;空即跳转即可 不进行mem_cpy
    
    mov eax,[ebx+8]
    cmp eax,0xc0001500
    jb .PTNULL
    
        
    push dword [ebx+16]                                        ;ebx+16在存储的数是filesz  可以翻到Loader刚开始
                                                               
    mov eax,[ebx+4]                                            
    add eax,KERNEL_BIN_BASE_ADDR
    push eax                                                   ;p_offset 在文件中的偏移位置    源位置         
    push dword [ebx+8]                                         ;目标位置
     
    call mem_cpy
    add esp,12                                                 ;把三个参数把栈扔出去 等于恢复栈指针
    
 .PTNULL:
    add  ebx,edx                                               ;edx是一个描述符字节大小
    loop .get_each_segment                                     ;继续进行外层循环    
    ret
                                        
mem_cpy:
    cld                                                        ;向高地址自动加数字 cld std 向低地址自动移动
    push ebp                                                   ;保存ebp 因为访问的时候通过ebp 良好的编程习惯保存相关寄存器
    mov  ebp,esp 
    push ecx                                                   ;外层循环还要用 必须保存 外层eax存储着还有几个段
    
                                                               ;分析一下为什么是 8 因为进入的时候又重新push了ebp 所以相对应的都需要+4
                                                               ;并且进入函数时 还Push了函数返回地址 所以就那么多了
    mov edi,[ebp+8]                                            ;目的指针 edi存储的是目的位置 4+4
    mov esi,[ebp+12]                                           ;源指针   源位置             8+4
    mov ecx,[ebp+16]                                           ;与Movsb好兄弟 互相搭配      12+4
    
    
    rep movsb                                                  ;一个一个字节复制
       
    pop ecx 
    pop ebp
    ret

;------------------------ rd_disk_m_32 
rd_disk_m_32:
;1 写入待操作磁盘数
;2 写入LBA 低24位寄存器 确认扇区
;3 device 寄存器 第4位主次盘 第6位LBA模式 改为1
;4 command 写指令
;5 读取status状态寄存器 判断是否完成工作
;6 完成工作 取出数据
 
 ;;;;;;;;;;;;;;;;;;;;;
 ;1 写入待操作磁盘数
 ;;;;;;;;;;;;;;;;;;;;;
    mov esi,eax   ; !!! 备份eax
    mov di,cx     ; !!! 备份cx
    
    mov dx,0x1F2  ; 0x1F2为Sector Count 端口号 送到dx寄存器中
    mov al,cl     ; !!! 忘了只能由ax al传递数据
    out dx,al     ; !!! 这里修改了 原out dx,cl
    
    mov eax,esi   ; !!!袄无! 原来备份是这个用 前面需要ax来传递数据 麻了
    
;;;;;;;;;;;;;;;;;;;;;
;2 写入LBA 24位寄存器 确认扇区
;;;;;;;;;;;;;;;;;;;;;
    mov cl,0x8    ; shr 右移8位 把24位给送到 LBA low mid high 寄存器中

    mov dx,0x1F3  ; LBA low
    out dx,al 
    
    mov dx,0x1F4  ; LBA mid
    shr eax,cl    ; eax为32位 ax为16位 eax的低位字节 右移8位即8~15
    out dx,al
    
    mov dx,0x1F5
    shr eax,cl
    out dx,al
    
;;;;;;;;;;;;;;;;;;;;;
;3 device 寄存器 第4位主次盘 第6位LBA模式 改为1
;;;;;;;;;;;;;;;;;;;;;

    		 
    		  ; 24 25 26 27位 尽管我们知道ax只有2 但还是需要按规矩办事 
    		  ; 把除了最后四位的其他位置设置成0
    shr eax,cl
    
    and al,0x0f 
    or al,0xe0   ;!!! 把第四-七位设置成0111 转换为LBA模式
    mov dx,0x1F6 ; 参照硬盘控制器端口表 Device 
    out dx,al

;;;;;;;;;;;;;;;;;;;;;
;4 向Command写操作 Status和Command一个寄存器
;;;;;;;;;;;;;;;;;;;;;

    mov dx,0x1F7 ; Status寄存器端口号
    mov ax,0x20  ; 0x20是读命令
    out dx,al
    
;;;;;;;;;;;;;;;;;;;;;
;5 向Status查看是否准备好惹 
;;;;;;;;;;;;;;;;;;;;;
    
		   ;设置不断读取重复 如果不为1则一直循环
  .not_ready:     
    nop           ; !!! 空跳转指令 在循环中达到延时目的
    in al,dx      ; 把寄存器中的信息返还出来
    and al,0x88   ; !!! 0100 0100 0x88
    cmp al,0x08
    jne .not_ready ; !!! jump not equal == 0
    
    
;;;;;;;;;;;;;;;;;;;;;
;6 读取数据
;;;;;;;;;;;;;;;;;;;;;

    mov ax,di      ;把 di 储存的cx 取出来
    mov dx,256
    mul dx        ;与di 与 ax 做乘法 计算一共需要读多少次 方便作循环 低16位放ax 高16位放dx
    mov cx,ax      ;loop 与 cx相匹配 cx-- 当cx == 0即跳出循环
    mov dx,0x1F0
 .go_read_loop:
    in ax,dx      ;两字节dx 一次读两字
    mov [ebx],ax
    add ebx,2
    loop .go_read_loop
    ret ;与call 配对返回原来的位置 跳转到call下一条指令
