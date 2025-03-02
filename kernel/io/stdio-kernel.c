#include "include/io/stdio-kernel.h"
#include "include/device/console_tty.h"
#include "include/library/ccos_print.h"
#include "include/library/types.h"
#include "include/user/stdio/stdio.h"

#define va_start(args, first_fix) args = (va_list) & first_fix
#define va_end(args) args = NULL

#ifndef va_list
typedef char *va_list;
#endif

/* this is the kernel format stdio... */
void ccos_printk(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buf[1024] = {0};
    vsprintf(buf, format, args);
    va_end(args);
    console_ccos_puts(buf);
}

void verbose_printk(const char *format, ...) {
    (void)format;
#ifdef VERBOSE
va_list args;
    va_start(args, format);
    char buf[1024] = {0};
    vsprintf(buf, format, args);
    va_end(args);
    console_ccos_puts(buf);
#endif
}