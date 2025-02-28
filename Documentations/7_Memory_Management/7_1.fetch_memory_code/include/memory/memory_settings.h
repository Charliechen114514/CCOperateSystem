#ifndef MEMORY_SETTINGS_H
#define MEMORY_SETTINGS_H

// Define the page size to 4096 bytes (4KB)
#define PG_SIZE                 (4096)

// Define the base address of the memory bitmap, which is a data structure used for managing memory blocks
// The bitmap will be used to track memory allocations and deallocations
#define MEMORY_BITMAP_BASE      (0xc009a000)

// Define the starting virtual address of the kernel space
// Kernel address space typically starts from a high memory region, often above the user-space address range
#define KERNEL_V_START          (0xc0000000)

// Define the size of the first 1MB of memory that is reserved for the kernel
// The kernel occupies this area initially to set up the environment, often including early memory mappings
#define KERNEL_LOW_1M_MEM       (0x100000)  // 1MB in hexadecimal

// Define the starting virtual address for the kernel heap
// The heap is used for dynamic memory allocation during the kernel's runtime
// This address is calculated by adding the size of the 1MB low kernel memory region to the starting kernel virtual address
#define KERNEL_HEAP_V_START     (KERNEL_V_START + KERNEL_LOW_1M_MEM)

// Define a memory address where a pointer to the memory size record is stored
// This is used to track the total size of available memory in the system
// you should have seen this at previous bochs debug
// steps should be:
//  1. place the memory fetch using e820 method in an address anywhere you like
//  2. caculate the address offset, which will be used later like here
//  3. validate this by using bochs debug with xp <target addr>
//  4. thats the case, modify here.
#define MEMORY_SIZE_RECORD_PTR  (0xb00)

// Dereference the pointer stored at MEMORY_SIZE_RECORD_PTR to obtain the memory size in bytes
// MEMORY_SZ_DETECT is a 32-bit unsigned integer that will contain the total amount of available memory in the system
#define MEMORY_SZ_DETECT        (*(uint32_t *)(MEMORY_SIZE_RECORD_PTR))  // Fetch the memory size value stored at the given address

#endif