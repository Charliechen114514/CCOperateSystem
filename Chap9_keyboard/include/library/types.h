#ifndef CCOS_TYPES_H
#define CCOS_TYPES_H

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

/* kernel usage */
typedef uint8_t             bool;

#ifndef true
#define true                    (1)
#endif

#ifndef false
#define false                   (0)
#endif

#define UINT8_MAX               (0xff)
#define NULL                    ((void*)0)

#endif