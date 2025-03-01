#ifndef USR_SYSCALL_NR
#define USR_SYSCALL_NR
#include "include/library/types.h"
typedef enum {
    SYS_GETPID,
    SYS_WRITE
}Syscall_NR;

uint32_t getpid(void);
uint32_t write(char* str);
#endif