#ifndef MEMORY_H
#define MEMORY_H

#include "include/library/types.h"
#include "include/library/bitmap.h"

typedef struct {
    Bitmap      vaddr_bitmap;
    uint32_t    vaddr_start;
}VirtualMemoryHandle;

extern struct pool kernel_pool, user_pool;

typedef enum {
    PF_KERNEL   = 1,
    PF_USER     = 2
}PoolFlag;


void        memory_management_init(void);
void*       get_kernel_pages(uint32_t pg_cnt);
uint32_t*   pte_ptr(uint32_t vaddr);
uint32_t*   pde_ptr(uint32_t vaddr);
#endif