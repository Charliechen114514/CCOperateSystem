#include "include/memory/memory.h"
#include "include/library/ccos_print.h"
#include "include/memory/memory_settings.h"
#include "include/library/assert.h"
#include "include/library/string.h"

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

struct pool{
    Bitmap      pool_map;
    uint32_t    phy_addr_start;
    uint32_t    pool_size;
};

struct pool kernel_pool, user_pool;
VirtualMemoryHandle kernel_vaddr;

/*
    returns the virtual pages start addr   
*/
static void* vaddr_get(PoolFlag pf, uint32_t pg_cnt)
{
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    // we consider the kernel case
    if (pf == PF_KERNEL) { 
        // are there anything left?
        bit_idx_start  = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1) {
            return NULL;
        }
        while(cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PGSIZE;
    } else {	     
        // TODO
    }
 
    return (void*)vaddr_start;
}

/* from vaddr to pte */
uint32_t* pte_ptr(uint32_t vaddr) {
    /* fast access */
    uint32_t* pte = (uint32_t*)(0xffc00000 + \
      ((vaddr & 0xffc00000) >> 10) + \
      PTE_IDX(vaddr) * 4);
    return pte;
 }
 
/* from vaddr to pde */
uint32_t* pde_ptr(uint32_t vaddr) {
    /* 0xfffff是用来访问到页表本身所在的地址 */
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
 }

/*
    allocate pages from physical memory pool
*/
static void* palloc(struct pool* m_pool) {
    int bit_idx = bitmap_scan(&m_pool->pool_map, 1);    // 找一个物理页面
    if (bit_idx == -1 ) {
       return NULL;
    }
    bitmap_set(&m_pool->pool_map, bit_idx, 1);	// 将此位bit_idx置1
    uint32_t page_phyaddr = ((bit_idx * PGSIZE) + m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}


static void page_table_add(void* _vaddr, void* _page_phyaddr) {
    uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t* pde = pde_ptr(vaddr);
    uint32_t* pte = pte_ptr(vaddr);
    __ccos_display_int(vaddr);
    __ccos_putchar('\n');
    __ccos_display_int((uint32_t)pde);
    __ccos_putchar('\n');
    __ccos_display_int((uint32_t)pte);
    while(1);
    // Ensure the pde is exsited!
    if (*pde & 0x00000001) {
        ASSERT(!(*pte & 0x00000001));
 
        if (!(*pte & 0x00000001)) {   
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        } else {	  
            KERNEL_PANIC("pte repeat");
        }
    } else {	   
        // we have to allocate first
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
       *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
 
        // We must clear the phyaddr's memory to zero, prevent the messy up!
        memset((void*)((int)pte & 0xfffff000), 0, PGSIZE); 
        ASSERT(!(*pte & 0x00000001));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);      // US=1,RW=1,P=1
    }
 }
 


/***********   To finish allocate page:   ***********
 * 1. allocate the virtual
 * 2. fetch the phisycal
 * 3. then make mappings
 ***************************************************************/

 // allocate from target memory pool with given page cnt
static void* malloc_page(PoolFlag pf, uint32_t pg_cnt) {
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);

    void* vaddr_start = vaddr_get(pf, pg_cnt);
    if (vaddr_start == NULL) {
       return NULL;
    }
    uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
 
    // we need to map as each, cause there are no consistence in phy addr!
    while (cnt-- > 0) {
        void* page_phyaddr = palloc(mem_pool);
 
        if (page_phyaddr == NULL) {  
            return NULL;
        }
 
        page_table_add((void*)vaddr, page_phyaddr); 
        vaddr += PGSIZE; // goto next
    }
    return vaddr_start;
 }
 
/* interface of the memory page fetcher */
void* get_kernel_pages(uint32_t pg_cnt) {
    void* vaddr =  malloc_page(PF_KERNEL, pg_cnt);
    if (vaddr != NULL) {	   // 若分配的地址不为空,将页框清0后返回
       memset(vaddr, 0, pg_cnt * PGSIZE);
    }
    return vaddr;
}
 

// memroy pool initialize
static void mem_pool_init(uint32_t all_mem)
{
    ccos_puts("     initing memory pools\n");
    const uint32_t pagetable_sz = PGSIZE * 256;
    const uint32_t used_memory = pagetable_sz + 0x100000;
    const uint32_t free_memory = all_mem - used_memory;
    const uint16_t all_free_pg = free_memory / PGSIZE;

    const uint16_t kernel_free_page = all_free_pg / 2;
    const uint16_t user_free_page = all_free_pg - kernel_free_page;

    const uint32_t    kbm_len = kernel_free_page / 8;
    const uint32_t    ubm_len = user_free_page / 8;

    const uint32_t kp_start = used_memory;
    const uint32_t up_start = kp_start + kernel_free_page * PGSIZE;

    kernel_pool.phy_addr_start = kp_start;
    user_pool.phy_addr_start = up_start;

    kernel_pool.pool_size = kernel_free_page * PGSIZE;
    user_pool.pool_size = user_free_page * PGSIZE;

    kernel_pool.pool_map.btmp_bytes_len = kbm_len;
    user_pool.pool_map.btmp_bytes_len = ubm_len;

    kernel_pool.pool_map.bits = (void*)MEM_BITMAP_BASE;
    user_pool.pool_map.bits = (void*)(MEM_BITMAP_BASE + kbm_len);

    ccos_puts("      kernel_pool_map_start:");__ccos_display_int((int)kernel_pool.pool_map.bits);
    ccos_puts(" kernel_pool_phy_addr_start:");__ccos_display_int(kernel_pool.phy_addr_start);
    ccos_puts("\n");
    ccos_puts("      user_pool_map_start:");__ccos_display_int((int)user_pool.pool_map.bits);
    ccos_puts(" user_pool_phy_addr_start:");__ccos_display_int(user_pool.phy_addr_start);
    ccos_puts("\n");


    bitmap_init(&kernel_pool.pool_map);
    bitmap_init(&user_pool.pool_map);

    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_len;
    kernel_vaddr.vaddr_bitmap.bits = (void*) \
        (MEM_BITMAP_BASE + kbm_len + ubm_len);
    kernel_vaddr.vaddr_start = KERNEL_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    ccos_puts("     init memory pools done!\n");
}

void memory_management_init(void)
{
    ccos_puts(" memery management initing...\n");
    uint32_t mem_pool_tool = (*(uint32_t*)(0xb00));
    mem_pool_init(mem_pool_tool);
    ccos_puts(" memery management init done!\n");
}