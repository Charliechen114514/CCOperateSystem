#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"

/*
 * kernel_panic_spin function is used to handle kernel panic scenarios, where
 * the kernel encounters a critical error that requires immediate attention.
 * This function will print detailed information about the panic and enter an
 * infinite loop to prevent further execution, essentially halting the kernel.
 *
 * Parameters:
 * - filename: The name of the source file where the panic occurred.
 * - line: The line number where the panic was triggered.
 * - func_name: The name of the function where the panic occurred.
 * - condition: The condition that caused the panic (usually a failed assertion).
 */
void kernel_panic_spin(char *filename, int line,
                       const char *func_name,
                       const char *condition)
{
    // Disable interrupts to prevent further interruptions during panic handling.
    set_intr_status(INTR_OFF);
    
    // Print a message indicating a panic has occurred.
    ccos_puts("------------------- HOLY SHIT -------------------\n");
    ccos_puts("Kernel has been died due to some reason... messages are dumped:\n");

    // Print the filename where the panic happened.
    ccos_puts("At filename:");
    ccos_puts(filename);
    ccos_puts("\n");

    // Print the line number where the panic happened.
    ccos_puts("At line:0x");
    __ccos_display_int(line); // Display the line number in hexadecimal format.
    ccos_puts("\n");

    // Print the function name where the panic happened.
    ccos_puts("At function:");
    ccos_puts((char *)func_name);
    ccos_puts("\n");

    // Print the condition that caused the panic (e.g., failed assertion).
    ccos_puts("At condition:");
    ccos_puts((char *)condition);
    ccos_puts("\n");

    // Print a message encouraging the user to check the issue and not to worry.
    ccos_puts("Check you business dude!\n");
    ccos_puts("Don't mind! For Bugs, WE SAY FUCK YOU!\n");

    // Print another line to mark the end of the panic message.
    ccos_puts("------------------- HOLY SHIT -------------------\n");

    // Enter an infinite loop to halt further kernel execution.
    // This is where the kernel stays in a "panic" state indefinitely.
    while (1)
        ;
}
