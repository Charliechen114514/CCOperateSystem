#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/kernel/interrupt.h"
int main(void)
{
    init_all();
    set_intr_status(INTR_ON);
    while(1);
    return 0;
}