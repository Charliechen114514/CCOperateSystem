#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "memory.h"
#include "../thread/thread.h"
#include "interrupt.h"
#include "../device/console.h"

void test_thread1(void* arg);
void test_thread2(void* arg);

int main(void) {
   put_str("I am kernel\n");
   init_all();
   //thread_start("kernel_thread_a",8,test_thread1,"Arga ");
   // thread_start("kernel_thread_b",8,test_thread2,"Argb ");
   intr_enable();
   
   while(1)
   {
   	console_put_str("Main ");
   }
   return 0;
}

void test_thread1(void* arg)
{
    while(1)
    {
   	console_put_str((char*)arg);
    }
}

void test_thread2(void* arg)
{
    while(1)
    {
   	console_put_str((char*)arg);
    }
}
