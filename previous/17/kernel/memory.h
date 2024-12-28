#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

struct virtual_addr{
    struct bitmap vddr_bitmap;
    uint32_t vaddr_start;
};

enum pool_flag{
    PF_KERNEL   = 1,
    PF_USER     = 2
};

#define PG_P_1  1
#define PG_P_0  0
#define PG_RW_R 1
#define PG_RW_W 2
#define PG_US_S 0
#define PG_US_U 4

extern struct pool kernel_pool, user_pool;
void* vaddr_get(enum pool_flag pf,uint32_t pg_cnt);
uint32_t* pte_ptr(uint32_t vaddr);
uint32_t* pde_ptr(uint32_t vaddr);
void* palloc(struct pool* m_pool);
void page_table_add(void* _vaddr,void* _page_phyaddr);
void* malloc_page(enum pool_flag pf,uint32_t pg_cnt);
void* get_kernel_pages(uint32_t pg_cnt);
void mem_init(void);
#endif