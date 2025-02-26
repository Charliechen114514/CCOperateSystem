#ifndef _IO_IO_H
#define _IO_IO_H
#include "include/library/types.h"

/* IO Operation level */
/* To make things fast, we need to define as inline */

/* write port a byte*/
// Inline function to write a byte to the specified I/O port
static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile(
        "outb %b0, %w1" // Assembly instruction to output a byte (%b0) to a port (%w1)
        :               // No outputs
        : "a"(data),    // Input: data in the AL register (8-bit, hence %b0 corresponds to the byte)
          "Nd"(port)    // Input: port address in the DX register (%w1 corresponds to the 16-bit port)
    );
}

/* write words */
// Inline function to write a word (16-bit) to the specified I/O port multiple times
static inline void outsw(uint16_t port, const void *addr, uint32_t word_cnt)
{
    asm volatile(
        "cld;               // Clear direction flag to ensure string operations go forward (increasing address)"
        "rep outsw"      // Repeat the 'outsw' instruction (write word) for word_cnt times
        : "+S"(addr),    // Input and output: address of the data to be written (register SI)
          "+c"(word_cnt) // Input and output: word count (register CX)
        : "d"(port)      // Input: port address (register DX)
    );
}

// read bytes
// Inline function to read a byte from the specified I/O port
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile(
        "inb %w1, %b0" // Assembly instruction to input a byte from port (%w1) into the AL register (%b0)
        : "=a"(data)   // Output: the value read from the port is stored in the data variable (AL register)
        : "Nd"(port)   // Input: port address (DX register)
    );
    return data;
}

/* read words */
// Inline function to read words (16-bit) from the specified I/O port multiple times
static inline void insw(uint16_t port, void *addr, uint32_t word_cnt)
{
    asm volatile(
        "cld;               // Clear direction flag to ensure string operations go forward (increasing address)"
        "rep insw"       // Repeat the 'insw' instruction (input word) for word_cnt times
        : "+D"(addr),    // Input and output: address where the data will be stored (register DI)
          "+c"(word_cnt) // Input and output: word count (register CX)
        : "d"(port)      // Input: port address (register DX)
        : "memory"       // Clobber memory: inform the compiler that memory may have been modified
    );
}

#endif
