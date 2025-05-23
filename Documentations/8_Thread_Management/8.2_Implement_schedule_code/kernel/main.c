#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory.h"
#include "include/thread/thread.h"

void thread_a(void* args);
void thread_b(void* args);
int main(void)
{
    init_all();
    thread_start("k_thread_a", 31, thread_a, "argA "); 
    thread_start("k_thread_b", 16, thread_b, "argB "); 
    interrupt_enabled();
    // code is baddy! we need LOCK!!!!
    while(1);
}

void thread_a(void* args){
    char* arg = (char*)args;
    while(1){
        ccos_puts(arg);
    }
}

void thread_b(void* args){
    char* arg = (char*)args;
    while(1){
        ccos_puts(arg);
    }
}