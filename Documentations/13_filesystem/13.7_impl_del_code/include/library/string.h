#ifndef STRING_H
#define STRING_H
#include    "include/library/types.h"
/* Kernel operations string op */
void        k_memset(void* dst_, uint8_t value, uint32_t size);
void        k_memcpy(void* dst_, const void* src_, uint32_t size);
int         k_memcmp(const void* a_, const void* b_, uint32_t size);
char*       k_strcpy(char* dst_, const char* src_);
uint32_t    k_strlen(const char* str);
int8_t      k_strcmp (const char *a, const char *b); 
char*       k_strchr(const char* string, const uint8_t ch);
char*       k_strrchr(const char* string, const uint8_t ch);
char*       k_strcat(char* dst_, const char* src_);
uint32_t    k_strchrs(const char* filename, uint8_t ch);

#endif