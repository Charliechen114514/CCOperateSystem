#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory.h"
#include "include/thread/thread.h"

void thread_a(void* args);

int main(void)
{
    init_all();
    thread_start("thread_a", 31, thread_a, "Welp I fired! ");
    while(1);
    return 0;
}

// this is the thread we shell fired!
// remember to put it after the main, else the main will be
// missed at 0xc0001500 :) these will be sad :(
void thread_a(void* args){
    char* arg = (char*)args;
    while(1){
        ccos_puts(arg);
    }
}