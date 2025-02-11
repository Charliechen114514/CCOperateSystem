#include "init.h"
#include "print.h"
#include "interrupt.h"

void init_all()
{
    put_str("initialization start\n");
    idt_init();
}