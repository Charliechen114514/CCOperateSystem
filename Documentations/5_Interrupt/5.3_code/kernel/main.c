#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/kernel/interrupt.h"
int main(void)
{
    init_all();
    sti();
    while(1);
    return 0;
}