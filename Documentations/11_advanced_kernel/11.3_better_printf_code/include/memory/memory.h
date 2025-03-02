#ifndef MEMORY_H
#define MEMORY_H

#include "include/library/types.h"
#include "include/library/bitmap.h"
#include "include/library/list.h"
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


// Pool flags tells to allocate from which pool flags :)
typedef enum { 
    PF_KERNEL = 1,  // allocate from kernel
    PF_USER = 2     // allocate from user
} PoolFlag;


// Declare the function 'memory_management_init', which initializes memory management systems
// This function is expected to set up kernel and user memory pools, initialize bitmaps, and configure memory address space
void memory_management_init(void);

// Function to allocate a specified number of pages and map them between virtual and physical memory
void* malloc_page(const PoolFlag pf, const uint32_t pg_cnt);
// Function to allocate a specified number of kernel pages and return their virtual addresses
void* get_kernel_pages(const uint32_t pg_cnt);
/* Allocate 4KB of memory from the user space and return the corresponding
 * virtual address */
void *get_user_pages(uint32_t pg_cnt);
/* Associate the address vaddr with the physical address in the pf pool,
 * supports only single page allocation */
void *get_a_page(PoolFlag pf, uint32_t vaddr);

#endif