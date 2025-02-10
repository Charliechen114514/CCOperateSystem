#include "include/library/assert.h"
#include "include/library/ccos_print.h"
#include "include/kernel/interrupt.h"
void kernel_die(
    char* filename, int line, 
    const char* func_name, const char* condition)
{
    // close the interrupt, kernel died :(
    disable_intr();
    ccos_puts("------------------- HOLY SHIT -------------------\n");
    ccos_puts("Kernel has been died due to some reason... messages are dumped:\n");
    ccos_puts("At filename:");ccos_puts(filename);ccos_puts("\n");
    ccos_puts("At line:0x");__ccos_display_int(line);ccos_puts("\n");
    ccos_puts("At function:");ccos_puts((char*)func_name);ccos_puts("\n");
    ccos_puts("At condition:");ccos_puts((char*)condition);ccos_puts("\n");
    ccos_puts("Check you business dude!\n");
    ccos_puts("Don't mind! For Bugs, WE SAY FUCK YOU!\n");
    ccos_puts("------------------- HOLY SHIT -------------------\n");
    while(1);
}