#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/kernel/interrupt.h"
int main(void)
{
    init_all();
    KERNEL_ASSERT(1 == 2);
    while(1);
    return 0;
}