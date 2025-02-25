#ifndef _IO_IO_H
#define _IO_IO_H
#include "include/library/types.h"
/* IO Operation level */
/* To make things fast, we need to define as inline */

/* write port a byte*/
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile ( "outb %b0, %w1" : : "a" (data), "Nd" (port));    
}
    
// write words
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {
   asm volatile ("  cld; \
                    rep outsw" : 
                    "+S" (addr), "+c" (word_cnt) : "d" (port));
}
// read bytes 
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}
    
// read words
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt) {
    asm volatile (" cld; \
                    rep insw" : "+D" (addr), "+c" (word_cnt) : "d" (port) : "memory");
}
    
#endif