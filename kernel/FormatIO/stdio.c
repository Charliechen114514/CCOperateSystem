#include "include/user/stdio/stdio.h"
#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/syscall/syscall.h"

#define va_start(ap, v) ap = (va_list) & v // the start of the vargs
#define va_arg(ap, t) *((t *)(ap += 4))    // let next
#define va_end(ap) ap = NULL               // clear varg

// itoa make a transform from interger to string
static void itoa(uint32_t value, char **buf_ptr_addr, uint8_t base) {
    uint32_t m = value % base;
    uint32_t i = value / base;
    if (i) {
        itoa(i, buf_ptr_addr, base);
    }
    if (m < 10) {
        *((*buf_ptr_addr)++) = m + '0';
    } else {
        *((*buf_ptr_addr)++) = m - 10 + 'A';
    }
}

uint32_t vsprintf(char *str, const char *format, va_list ap) {
    char *buf_ptr = str;
    const char *index_ptr = format;
    char index_char = *index_ptr;
    int32_t arg_int;
    char *arg_str;
    while (index_char) {
        if (index_char != '%') {
            *(buf_ptr++) = index_char;
            index_char = *(++index_ptr);
            continue;
        }
        index_char = *(++index_ptr);
        switch (index_char) {
        case 's':
            arg_str = va_arg(ap, char *);
            strcpy(buf_ptr, arg_str);
            buf_ptr += strlen(arg_str);
            index_char = *(++index_ptr);
            break;

        case 'c':
            *(buf_ptr++) = va_arg(ap, char);
            index_char = *(++index_ptr);
            break;

        case 'd':
            arg_int = va_arg(ap, int);
            if (arg_int < 0) {
                arg_int = 0 - arg_int;
                *buf_ptr++ = '-';
            }
            itoa(arg_int, &buf_ptr, 10);
            index_char = *(++index_ptr);
            break;

        case 'x':
            arg_int = va_arg(ap, int);
            itoa(arg_int, &buf_ptr, 16);
            index_char = *(++index_ptr);
            break;
        }
    }
    return strlen(str);
}

uint32_t sprintf(char *buf, const char *format, ...) {
    va_list args;
    uint32_t retval;
    va_start(args, format);
    retval = vsprintf(buf, format, args);
    va_end(args);
    return retval;
}

// printf as the stdio version
uint32_t printf(const char *format, ...) {
    va_list args;
    va_start(args, format); // 使args指向format
    char buf[1024] = {0};   // 用于存储拼接后的字符串
    vsprintf(buf, format, args);
    va_end(args);
    return write(1, buf, strlen(buf));
}
