#include "include/device/keyboard.h"
#include "include/library/ccos_print.h"
#include "include/kernel/interrupt.h"
#include "include/device/configs/keyboard_ascii.h"
#include "include/device/configs/keyboard_mappings.h"
#include "include/io/io.h"

static void keyboard_intr_handler(void)
{
    // hey don't use puts here, gs is not switched, else we will visit
    // wrong place
    // __ccos_putchar('C');
    uint8_t scancode = 
        inb(KEYBOARD_BUF_PORT);
    __ccos_display_int(scancode);
    __ccos_putchar(' ');
    return;
}

// register the interrupt here
void init_basic_input_subsystem(void)
{
    ccos_puts("initing subsystem of input: from keyboard!\n");
    register_intr_handler(KEYBOARD_INTERRUPT_N, keyboard_intr_handler);
    ccos_puts("init subsystem of input: from keyboard done!\n");
}