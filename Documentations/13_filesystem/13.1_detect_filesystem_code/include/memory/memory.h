#ifndef MEMORY_H
#define MEMORY_H

#include "include/library/bitmap.h"
#include "include/library/types.h"
#include "include/library/list.h"
typedef struct {
    Bitmap vaddr_bitmap;
    uint32_t vaddr_start;
} VirtualMemoryHandle;

/* MemoryBlock */
typedef struct {
    list_elem free_elem;
}MemoryBlock;

/* MemoryBlockDescriptor */
typedef struct {
    uint32_t    block_size;       // block size of the memory
    uint32_t    blocks_per_arena; // current memblock contains
    list        free_list;     // current available mem blocks
}MemoryBlockDescriptor;

typedef struct __mem_pool MemoryPool;

extern MemoryPool kernel_pool, user_pool;

typedef enum { PF_KERNEL = 1, PF_USER = 2 } PoolFlag;

void memory_management_init(void);
void *get_kernel_pages(uint32_t pg_cnt);
uint32_t *pte_ptr(uint32_t vaddr);
uint32_t *pde_ptr(uint32_t vaddr);
void* malloc_page(PoolFlag pf, uint32_t pg_cnt);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
void* get_a_page(PoolFlag pf, uint32_t vaddr);
void* get_user_pages(uint32_t pg_cnt);
void block_desc_init(MemoryBlockDescriptor* desc_array);
void* sys_malloc(uint32_t size);
void mfree_page(PoolFlag pf, void* _vaddr, uint32_t pg_cnt);
void pfree(uint32_t pg_phy_addr);
void sys_free(void* ptr);
void* get_a_page_without_opvaddrbitmap(PoolFlag pf, uint32_t vaddr);
void free_a_phy_page(uint32_t pg_phy_addr);
#endif