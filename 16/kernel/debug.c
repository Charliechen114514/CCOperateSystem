# include "debug.h"
# include "kernel/print.h"
# include "interrupt.h"

void panic_spin(char* filename, int line, const char* func, const char* condition) {
    intr_disable();

    put_str("\n\n\n*** PANIC ***\n\n\n");
    
    put_str("FileName: ");put_str(filename);put_char('\n');
    put_str("Line: ");put_int(line);put_char('\n');
    put_str("Function: ");put_str(func);put_char('\n');
    put_str("Condition: ");put_str(condition);put_char('\n');

    put_str("\n\n\n*** ***\n\n\n");

    while (1);
}
