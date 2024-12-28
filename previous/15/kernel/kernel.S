[bits 32]
%define ERROR_CODE nop      ;若在相关异常中CPU已经自动压入了错误码，为保持栈中格式统一，这里不做操作
%define ZERO push 0        ;CPU没有压入错误码，为了统一栈中格式，手工压入0

extern idt_table                ;idt_table是C中注册的中断处理程序数组

section .data

; intr_entry_table位于data段, 之后会和宏中的data段组合在一起(注意: 宏中的text段与intr_entry_table不是同一个段)
global intr_entry_table
intr_entry_table:

;--------------- 宏 VECTOR 开始, 参数数目为2, 第一个参数为中断号, 第二个参数为该中断对 ERROR_CODE 的操作 -----------
%macro VECTOR 2
section .text
intr%1entry:                ;每个中断处理程序都要压入中断向量号
                            ;所以一个终端类型一个中断处理程序
                            ;自己知道自己的中断向量号是多少
    %2
    ;以下是保存上下文环境
    push ds
    push es
    push fs
    push gs
    pushad


    ;如果是  从片 上进入的中断，除了往 从片发生EOI外，还要往主片发EOI
    mov al, 0x20             ;中断结束命令 EOI
    out 0xa0, al             ;向 从片发生
    out 0x20, al             ;向 主片发送

    push %1                     ;;不管idt_table中的目标程序是否需要参数，一律压入中断向量号
    call [idt_table + %1*4]   ;调用idt_table中的c版本中断处理函数

    jmp intr_exit

section .data           ;这个 section .data 的作用就是让数组里全都是地址，编译器会将属性相同的 Section 合成一个大的 Segmengt，所以这里就是紧凑排列的数组了
        dd intr%1entry  ;存储各个中断入口程序的地址，形成 intr_entry_table 数组

%endmacro

section .text
global intr_exit
intr_exit:
;以下是恢复上下文环境
        add esp,4       ;跳过中断号
        popad
        pop gs
        pop fs
        pop es
        pop ds
        add esp,4       ;跳过error_code
        iretd


VECTOR 0x00, ZERO
VECTOR 0x01, ZERO
VECTOR 0x02, ZERO
VECTOR 0x03, ZERO 
VECTOR 0x04, ZERO

VECTOR 0x05, ZERO
VECTOR 0x06, ZERO
VECTOR 0x07, ZERO 
VECTOR 0x08, ERROR_CODE
VECTOR 0x09, ZERO

VECTOR 0x0a, ERROR_CODE
VECTOR 0x0b, ERROR_CODE 
VECTOR 0x0c, ZERO
VECTOR 0x0d, ERROR_CODE
VECTOR 0x0e, ERROR_CODE

VECTOR 0x0f, ZERO 
VECTOR 0x10, ZERO
VECTOR 0x11, ERROR_CODE
VECTOR 0x12, ZERO
VECTOR 0x13, ZERO 

VECTOR 0x14, ZERO
VECTOR 0x15, ZERO
VECTOR 0x16, ZERO
VECTOR 0x17, ZERO 
VECTOR 0x18, ERROR_CODE

VECTOR 0x19, ZERO
VECTOR 0x1a, ERROR_CODE
VECTOR 0x1b, ERROR_CODE 
VECTOR 0x1c, ZERO
VECTOR 0x1d, ERROR_CODE

VECTOR 0x1e, ERROR_CODE
VECTOR 0x1f, ZERO 
VECTOR 0x20, ZERO


