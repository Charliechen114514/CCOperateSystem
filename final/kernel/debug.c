#include "debug.h"
#include "print.h"
#include "interrupt.h"
#include "syscall.h"
/* 打印文件名,行号,函数名,条件并使程序悬停 */
void panic_spin(char* filename,	       \
	        int line,	       \
		const char* func,      \
		const char* condition) \
{
   intr_disable();	// 因为有时候会单独调用panic_spin,所以在此处关中断。
   clear();
   put_str("\n\n\n!!!!! PANIC !!!!!\n");
   put_str("filename:");put_str(filename);put_str("\n");
   put_str("line:0x");put_int(line);put_str("\n");
   put_str("function:");put_str((char*)func);put_str("\n");
   put_str("condition:");put_str((char*)condition);put_str("\n");
   put_str("\n\n\n!!!!! PANIC_END !!!!!\n");
   while(1);
}
