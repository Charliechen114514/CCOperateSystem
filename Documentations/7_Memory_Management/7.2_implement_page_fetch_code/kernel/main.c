#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory.h"
int main(void)
{
    init_all();
    void* addr = get_kernel_pages(3);
    ccos_puts("\nget_kernel_page start vaddr is "); 
    __ccos_display_int((uint32_t)addr); 
    ccos_puts("\n");
    ccos_puts("\nlet's do another query: start vaddr is "); 
    addr = get_kernel_pages(3);
    __ccos_display_int((uint32_t)addr); 
    ccos_puts("\n");
    // interrupt_enabled();
    while(1);
    return 0;
}