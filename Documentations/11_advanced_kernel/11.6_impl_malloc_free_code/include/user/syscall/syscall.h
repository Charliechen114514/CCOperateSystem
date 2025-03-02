#ifndef USR_SYSCALL_NR
#define USR_SYSCALL_NR
#include "include/library/types.h"
typedef enum {
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE
}Syscall_NR;

uint32_t getpid(void);
uint32_t write(char* str);
void* malloc(uint32_t size); 
void free(void* ptr);
#endif