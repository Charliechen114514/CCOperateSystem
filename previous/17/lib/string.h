#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#include "stdint.h"
#define NULL 0

void memset(void* dst_,uint8_t value,uint32_t size);
void memcpy(void* dst_,const void* src_,uint32_t size);
int memcmp(const void* a_,const void* b_, uint32_t size);
char* strcpy(char* dsc_,const char* src_);
uint32_t strlen(const char* str);
int8_t strcmp(const char* a,const char* b);
char* strchr(const char* str,const uint8_t ch);
char* strrchr(const char* str,const uint8_t ch);
char* strcat(char* dsc_,const char* src_);
uint32_t strchrs(const char* str,uint8_t ch);
#endif
