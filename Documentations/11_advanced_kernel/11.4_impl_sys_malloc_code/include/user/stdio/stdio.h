#ifndef USER_STDIO_H
#define USER_STDIO_H

#include "include/library/types.h"

#define PRINTF_IO_BUFFER_LENGTH (1024)

typedef char* va_list;
// Formats and prints a string based on a format specifier
uint32_t printf(const char* format_string, ...);
// Formats a string and stores the result in the provided buffer
uint32_t vsprintf(char *output_buffer, const char *format_string, va_list args);
// Formats a string and stores it in the given buffer
uint32_t sprintf(char *buffer, const char *format_string, ...);
#endif