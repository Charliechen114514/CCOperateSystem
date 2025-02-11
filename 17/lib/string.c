#include "string.h"
#include "global.h"
#include "debug.h"

void memset(void* _dst, uint8_t value, uint32_t size)
{
    ASSERT(_dst != NULL);
    uint8_t* dst = (uint8_t*)_dst;
    while(size-- > 0)
        *dst++ = value;
}

void memcpy(void* _dst, const void* _src, uint32_t size){
    ASSERT(_dst != NULL && _src != NULL);
    uint8_t* dst = (uint8_t*)_dst;
    const uint8_t* src = _src;
    while(size-->0)
        *dst++ = *src++;
}

int memcmp(const void* _a, const void* _b, uint32_t size)
{
    const char* a = _a;
    const char* b = _b;
    ASSERT(a != NULL && b != NULL);
    while(size-->0)
    {
        if(*a != *b)
            return *a > *b ? 1 : -1;
        a++;
        b++;
    }

    return 0;
}

char* strcpy(char* _dst, const char* _src)
{
    ASSERT(_dst != NULL && _src != NULL);
    char* r = _dst;
    while ((*_dst++ = *_src++));
    return r;
}

uint32_t strlen(const char* str)
{
    ASSERT(str != NULL);
    const char* p = str;
    while(*p++);
    return p - str - 1; 
}

int8_t strcmp(const char* a, const char* b){
    ASSERT(a != NULL && b != NULL)
    while(*a != 0 && *a == *b)
    {
        a++;
        b++;
    }
    return *a < *b ? -1 : *a > *b;
}

char* strchr(const char* str, const uint8_t ch)
{
    ASSERT(str != NULL);
    while(*str != 0)
    {
        if(*str == ch)
            return (char*)str;
        str++;
    }
    return (char*)NULL;
}

char* strrchr(const char* str, const uint8_t ch)
{
    ASSERT(str != NULL);
    char* last_char = NULL;
    while(*str != 0)
    {
        if(*str == ch)
            last_char = (char*)str;
        str++;
    }
    return (char*)last_char;
}


char* strcat(char* dsc_,const char* src_)
{
    ASSERT(dsc_ != NULL && src_ != NULL);
    char* str = dsc_;
    while(*(str++));
    str--;
    while(*(str++) = *((char*)src_++));
    return dsc_;
}

uint32_t strchrs(const char* str,uint8_t ch)
{
    ASSERT(str != NULL);
    uint32_t ch_cnt = 0;
    while(*str)
    {
    	if(*str == ch) ++ch_cnt;
    	++str;
    }
    return ch_cnt;
}
