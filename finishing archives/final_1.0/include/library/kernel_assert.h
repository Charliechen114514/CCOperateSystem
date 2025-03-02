#ifndef __ASSERT_H
#define __ASSERT_H

void kernel_die(char* filename, int line, const char* func_name, const char* condition);

#define KERNEL_PANIC(...) kernel_die(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define ASSERT(COND)    do{\
    if(!(COND)){\
        KERNEL_PANIC(#COND);\
    }}while(0)


#endif