#ifndef MEMORY_H
#define MEMORY_H

#include "include/library/types.h"
#include "include/library/bitmap.h"

// Define a forward declaration for the 'MemoryPool' structure, which will manage memory pools for the kernel and user spaces
// MemoryPool represents a pool of memory blocks, managing allocations and deallocations
typedef struct __mem_pool MemoryPool;

// Declare two global memory pool variables: one for kernel space and one for user space
// These will be used to manage memory allocations in the respective address spaces
extern MemoryPool kernel_pool, user_pool;

// Define a structure to hold information about virtual address mappings
// The structure includes a bitmap to track the allocation of virtual addresses and the starting address for virtual memory
typedef struct
{
    Bitmap virtual_mem_bitmap; // A bitmap for tracking allocated and free virtual memory pages
    uint32_t vaddr_start;      // The starting address of the virtual memory region
} VirtualAddressMappings;

// Declare the function 'memory_management_init', which initializes memory management systems
// This function is expected to set up kernel and user memory pools, initialize bitmaps, and configure memory address space
void memory_management_init(void);

#endif