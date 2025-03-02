#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include "include/library/types.h"
#define PRINTF_IO_BUFFER_LENGTH (1024)
typedef char* va_list;
uint32_t printf(const char* str, ...);
uint32_t vsprintf(char* str, const char* format, va_list ap);
uint32_t sprintf(char* buf, const char* format, ...);
#endif
