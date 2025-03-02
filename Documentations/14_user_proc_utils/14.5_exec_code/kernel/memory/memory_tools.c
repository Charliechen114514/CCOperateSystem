#include "include/memory/memory_tools.h"


// Function to get the pointer to the Page Table Entry (PTE) corresponding to a virtual address
uint32_t* pte_ptr(uint32_t vaddr) { 
    /* First, access the page table itself + 
    Then, use the Page Directory Entry (PDE) (which is an index in the page directory) to access the Page Table + 
    Finally, use the Page Table Entry (PTE) index to calculate the offset within the page. */
    
    // Calculate the PTE address: 
    // 1. Start from the base address for the page tables (0xffc00000).
    // 2. Shift the virtual address to extract the Page Directory index and use it to locate the page table.
    // 3. Use the PTE index to find the specific entry within the page table and multiply by 4 to account for 32-bit (4-byte) entries.
    return (uint32_t*)(0xffc00000 + 
            ((vaddr & 0xffc00000) >> 10) +   // Extract PDE index and shift it for the page table location
            PTE_IDX(vaddr) * 4);             // Use PTE index to find the exact entry in the page table
}

// Function to get the pointer to the Page Directory Entry (PDE) corresponding to a virtual address
uint32_t* pde_ptr(uint32_t vaddr) { 
    /* Use 0xfffff000 to access the base address of the page directory. */
    
    // Calculate the PDE address: 
    // 1. Start from the base address of the page directory (0xfffff000).
    // 2. Use the Page Directory Entry index to calculate the offset and multiply by 4 for the 32-bit entries.
    return (uint32_t*)((PG_FETCH_OFFSET) + PDE_IDX(vaddr) * 4); // Return the calculated PDE pointer
}


/* Get the physical address mapped to the virtual address */
uint32_t addr_v2p(uint32_t vaddr) {
    uint32_t *pte = pte_ptr(vaddr);
    /* (*pte) holds the physical page frame address of the page table,
     * remove the lower 12 bits of the page table entry attributes,
     * and add the lower 12 bits of the virtual address vaddr */
    return ((*pte & PG_FETCH_OFFSET) + (vaddr & PG_OFFSET_MASK));
}