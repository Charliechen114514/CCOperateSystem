/**************	 机器模式   ***************
	 b -- 输出寄存器QImode名称,即寄存器中的最低8位:[a-d]l。
	 w -- 输出寄存器HImode名称,即寄存器中2个字节的部分,如[a-d]x。

	 HImode
	     “Half-Integer”模式，表示一个两字节的整数。 
	 QImode
	     “Quarter-Integer”模式，表示一个一字节的整数。 
*******************************************/ 

#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

/*向端口port写入一个字节*/
static inline void outb(uint16_t port, uint8_t data) {
    /*********************************************************
        对端口指定 N 表示0-255, d表示用dx存储端口号，
        %b0表示对应al，%w1表示对应dx */
    asm volatile ("outb %b0, %w1" : : "a"(data), "Nd"(port));
    /******************************************************/
        // 这里是 AT&T 语法的汇编语言，相当于： mov al. data
        //                                   mov dx, port
        //                                   out dx, al

}


/*将addr处其实的word_cnt 个字写入端口port */
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {
       /*********************************************************
        + 表示此限制既做输入，又做输出,
        outsw 是把 ds:esi 处的 16 位的内容写入 port 端口,
        我们在设置段描述符时，已经将ds,es,ss段的选择子都设置为相同的值了， 此时不用担心数据错乱 */
    asm volatile ("cld; rep outsw" : "+S"(addr), "+c"(word_cnt) : "d"(port));
    /*********************************************************/
    // 这里是 AT&T 语法的汇编语言，相当于： cld
    //                                   mov esi, addr
    //                                   mov ecx, word_cnt
    //                                   mov edx, port
}


/* 将从端口port 读入一个字节返回*/
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
    return data;
}

/* 将从端口port 读入的word_cnt 个字写入addr */
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt){
    //insw是将端口port处读入的16位内容写入es:edi 指向的内存
    asm volatile("cld; rep insw" : "+D"(addr), "+c"(word_cnt) : "d"(port): "memory");
}

#endif

