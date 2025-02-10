#include "include/library/ccos_print.h"
#include "include/kernel/init.h"
#include "include/library/assert.h"
#include "include/memory/memory.h"

int main()
{
    ccos_set_console_cursor(5, 0);
    ccos_puts("Welcome Main!\nCCOS has been entering main!\n");
    init_all();

    void* addr = get_kernel_pages(3);
    ccos_puts("get the kernel page vaddr: ");
    __ccos_display_int((uint32_t)addr);
    ccos_puts("\n");
    addr = get_kernel_pages(3);
    ccos_puts("get another kernel page vaddr: ");
    __ccos_display_int((uint32_t)addr);
    ccos_puts("\n");
    while(1);
}