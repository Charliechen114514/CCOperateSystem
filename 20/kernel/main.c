#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "memory.h"
#include "../thread/thread.h"
#include "interrupt.h"
#include "../device/console.h"
#include "../device/ioqueue.h"
#include "../device/keyboard.h"
#include "../userprog/process.h"

int a = 0,b = 0;
void test_thread1(void* arg);
void test_thread2(void* arg);
void u_prog_a(void);
void u_prog_b(void);

int main(void) {
   put_str("I am kernel\n");
   init_all();
   thread_start("kernel_thread_a",31,test_thread1,"argA: ");
   thread_start("kernel_thread_b",31,test_thread2,"argB: ");
   process_execute(u_prog_a,"user_prog_a");
   process_execute(u_prog_b,"user_prog_b");
   intr_enable();
   
   while(1);
   return 0;
}

void test_thread1(void* arg)
{
    while(1)
    {
        console_put_str((char*)arg);
        console_put_int(a);
        console_put_char(' ');
    }
}

void test_thread2(void* arg)
{
    while(1)
    {
        console_put_str((char*)arg);
        console_put_int(b);
        console_put_char(' ');
    }
}

void u_prog_a(void)
{
    while(1)
    {
    	++a;
    }
}

void u_prog_b(void)
{
    while(1)
    {
    	++b;
    }
}

