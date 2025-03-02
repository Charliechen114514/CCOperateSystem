#ifndef BIT_TOOLS_H
#define BIT_TOOLS_H

#define LOW_16BITS(value)   ((uint32_t)value & 0x0000FFFF)
#define HIGH_16BITS(value)  (((uint32_t)value & 0xFFFF0000) >> 16)

#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

#endif