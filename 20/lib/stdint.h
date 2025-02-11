/**
 * 仿照/usr/include/stdint.h定义我们自己的数据类型.
 */
#ifndef _LIB_STDINT_H
#define _LIB_STDINT_H

#define NULL 0
#define true 1
#define false 0
#define bool char
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

#endif
