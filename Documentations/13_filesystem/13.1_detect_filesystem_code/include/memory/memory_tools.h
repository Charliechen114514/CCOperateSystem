#ifndef MEMORY_TOOLS_H
#define MEMORY_TOOLS_H
#include "include/memory/memory_settings.h"
#include "include/library/types.h"

// Define the macro to extract the Page Directory Entry (PDE) index from a virtual address
// The virtual address is divided into multiple parts for paging: 
// The top 10 bits (bits 22–31) correspond to the Page Directory index.
// The macro uses bitwise operations to mask and shift the address to isolate these bits.
#define PDE_IDX(addr)   ((addr & 0xffc00000) >> 22)  // Extract the top 10 bits for the PDE index

// Define the macro to extract the Page Table Entry (PTE) index from a virtual address
// The next 10 bits (bits 12–21) correspond to the Page Table index.
// The macro uses bitwise operations to mask and shift the address to isolate these bits.
#define PTE_IDX(addr)   ((addr & 0x003ff000) >> 12)  // Extract bits 12–21 for the PTE index

#define MB              (1024 * 1024)

// Function to get the pointer to the Page Table Entry (PTE) corresponding to a virtual address
uint32_t* pte_ptr(uint32_t vaddr);
// Function to get the pointer to the Page Directory Entry (PDE) corresponding to a virtual address
uint32_t* pde_ptr(uint32_t vaddr);
/* Get the physical address mapped to the virtual address */
uint32_t addr_v2p(uint32_t vaddr);
#endif