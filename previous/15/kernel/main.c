#include "print.h"
#include "init.h"
int main(void)
{
	put_str("Enter the kernel\n");
	init_all();
	asm volatile("sti");
	while(1);
}
