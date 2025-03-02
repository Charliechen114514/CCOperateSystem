#include "include/memory/memory.h"
#include "include/kernel/interrupt.h"
#include "include/kernel/lock.h"
#include "include/library/bitmap.h"
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/memory/memory_settings.h"
#include "include/utils/fast_utils.h"
/*************** Bitmap Addresses ********************
 * 0xc009f000 is the top of the kernel main thread stack, and 0xc009e000 is the
 *PCB of the kernel main thread. A bitmap of the size of one page can represent
 *128M of memory. The bitmap is placed at address 0xc009a000. This system can
 *support up to 4 page frames worth of bitmap, representing up to 512M of
 *memory.
 *************************************/

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

/* Memory pool structure, creating two instances for managing the kernel and
 * user memory pools */
struct pool {
    Bitmap pool_bitmap; // Bitmap used by this pool to manage physical memory
    uint32_t phy_addr_start; // The start address of the physical memory managed
                             // by this pool
    uint32_t pool_size;      // The size of this memory pool in bytes
    CCLocker lock;           // Mutex for memory allocation requests
};

/* Arena metadata for memory allocation */
struct arena {
    MemoryBlockDescriptor
        *desc; // Memory block descriptor associated with this arena
    /* If 'large' is true, 'cnt' represents the number of page frames.
     * Otherwise, 'cnt' represents the number of free memory blocks. */
    uint32_t cnt;
    bool large;
};

MemoryBlockDescriptor
    k_block_descs[DESC_CNT]; // Array of kernel memory block descriptors
struct pool kernel_pool,
    user_pool; // Instances for managing kernel and user memory pools
VirtualMemoryHandle kernel_vaddr; // Structure used for allocating virtual
                                  // addresses for the kernel

/* Request pg_cnt virtual pages from the virtual memory pool specified by pf.
 * If successful, returns the start address of the virtual pages; otherwise,
 * returns NULL. */
static void *vaddr_get(PoolFlag pf, uint32_t pg_cnt) {
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    if (pf == PF_KERNEL) { // Kernel memory pool
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1) {
            return NULL;
        }
        while (cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PGSIZE;
    } else { // User memory pool
        TaskStruct *cur = running_thread();
        bit_idx_start = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1) {
            return NULL;
        }

        while (cnt < pg_cnt) {
            bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++,
                       1);
        }
        vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PGSIZE;

        /* (0xc0000000 - PGSIZE) is already allocated as the user-level 3rd
         * stack in start_process */
        KERNEL_ASSERT((uint32_t)vaddr_start < (0xc0000000 - PGSIZE));
    }
    return (void *)vaddr_start;
}

/* Get the pointer to the PTE for the virtual address vaddr */
uint32_t *pte_ptr(uint32_t vaddr) {
    /* Access the page table itself first + \
     * Then use the PDE index to access the PTE within the page table + \
     * Finally, use the PTE index to get the page offset */
    uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) +
                                 PTE_IDX(vaddr) * 4);
    return pte;
}

/* Get the pointer to the PDE for the virtual address vaddr */
uint32_t *pde_ptr(uint32_t vaddr) {
    /* 0xfffff is used to access the page table's location */
    uint32_t *pde = (uint32_t *)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}

/* Allocate 1 physical page from the memory pool m_pool.
 * If successful, returns the physical address of the page; otherwise, returns
 * NULL */
static void *palloc(struct pool *m_pool) {
    /* Bitmap scan or setting should be atomic */
    int bit_idx =
        bitmap_scan(&m_pool->pool_bitmap, 1); // Find an available physical page
    if (bit_idx == -1) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap, bit_idx,
               1); // Set the bit at index bit_idx to 1
    uint32_t page_phyaddr = ((bit_idx * PGSIZE) + m_pool->phy_addr_start);
    return (void *)page_phyaddr;
}

/* Add a mapping between the virtual address _vaddr and the physical address
 * _page_phyaddr in the page table */
static void page_table_add(void *_vaddr, void *_page_phyaddr) {
    uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t *pde = pde_ptr(vaddr);
    uint32_t *pte = pte_ptr(vaddr);

    /************************   Important   *************************
     * Accessing *pte will access *pde. Therefore, ensure that pde is created
     * before accessing *pte, otherwise it will cause a page fault. Hence, when
     * pde is not created, *pte should only appear after *pde in the following
     * outer else block.
     * *********************************************************/
    /* Check the P bit of the directory entry, if it is 1, it indicates the
     * table already exists */
    if (*pde & PG_P_1) {
        // Check if the PTE exists. If it does, no error; otherwise, create it
        if (*pte & PG_P_1) {
            KERNEL_PANIC_SPIN("pte repeat");
        } else {
            *pte = (page_phyaddr | PG_US_U | PG_RW_W |
                    PG_P_1); // Create the PTE with necessary flags
        }
    } else { // If the page directory entry does not exist, create it first,
             // then create the page table entry.
        /* The page table must be allocated from kernel space */
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

        /*******************   
         * Ensure the page table is cleared The physical memory corresponding to the
         * pde_phyaddr should be cleared to avoid outdated data turning into
         * page table entries, which would cause confusion in the page table.
         * The high 20 bits of pte will map to the physical start address of the
         * page table pointed by pde. */
        k_memset((void *)((int)pte & 0xfffff000), 0, PGSIZE);
        /************************************************************/
        KERNEL_ASSERT(!(*pte & PG_P_1));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); // US=1, RW=1, P=1
    }
}

/* Allocate pg_cnt pages of memory. If successful, return the starting virtual
 * address, if failed, return NULL */
void *malloc_page(PoolFlag pf, uint32_t pg_cnt) {
    KERNEL_ASSERT(pg_cnt > 0 && pg_cnt < 3840);
    /***********   The principle of malloc_page consists of three actions:
    ***********
          1. Use vaddr_get to allocate virtual addresses from the virtual memory
    pool
          2. Use palloc to allocate physical pages from the physical memory pool
          3. Use page_table_add to map the allocated virtual and physical
    addresses in the page table
    ***************************************************************/

    void *vaddr_start = vaddr_get(pf, pg_cnt);
    if (vaddr_start == NULL) {
        return NULL;
    }
    uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
    struct pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

    /* Because virtual addresses are contiguous, but physical addresses can be
     * non-contiguous, map them one by one */
    while (cnt-- > 0) {
        void *page_phyaddr = palloc(mem_pool);

        /* If allocation fails, rollback all previously allocated virtual and
         * physical pages, to be handled later during memory recovery */
        if (page_phyaddr == NULL) {
            return NULL;
        }

        page_table_add((void *)vaddr, page_phyaddr); // Map in the page table
        vaddr += PGSIZE; // Move to the next virtual page
    }
    return vaddr_start;
}

/* Allocate pg_cnt pages of memory from the kernel's physical memory pool,
 * return the corresponding virtual address if successful, NULL if failed */
void *get_kernel_pages(uint32_t pg_cnt) {
    lock_acquire(&kernel_pool.lock);
    void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if (vaddr != NULL) { // If the allocated address is not NULL, clear the
                         // pages before returning
        k_memset(vaddr, 0, pg_cnt * PGSIZE);
    }
    lock_release(&kernel_pool.lock);
    return vaddr;
}

/* Allocate 4KB of memory from the user space and return the corresponding
 * virtual address */
void *get_user_pages(uint32_t pg_cnt) {
    lock_acquire(&user_pool.lock);
    void *vaddr = malloc_page(PF_USER, pg_cnt);
    if (vaddr != NULL) { // If the allocated address is not NULL, clear the
                         // pages before returning
        k_memset(vaddr, 0, pg_cnt * PGSIZE);
    }
    lock_release(&user_pool.lock);
    return vaddr;
}

/* Associate the address vaddr with the physical address in the pf pool,
 * supports only single page allocation */
void *get_a_page(PoolFlag pf, uint32_t vaddr) {

    struct pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);

    /* First, set the corresponding bit in the virtual address bitmap */
    TaskStruct *cur = running_thread();
    int32_t bit_idx = -1;

    /* If the current thread is a user process requesting user memory, modify
     * the user process's virtual address bitmap */
    if (cur->pgdir != NULL && pf == PF_USER) {
        bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PGSIZE;
        KERNEL_ASSERT(bit_idx >= 0);
        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx, 1);

    } else if (cur->pgdir == NULL && pf == PF_KERNEL) {
        /* If a kernel thread is requesting kernel memory, modify kernel_vaddr.
         */
        bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PGSIZE;
        KERNEL_ASSERT(bit_idx > 0);
        bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
    } else {
        KERNEL_PANIC_SPIN("get_a_page: not allowed to allocate user space in kernel "
                     "or kernel space in user mode");
    }

    void *page_phyaddr = palloc(mem_pool);
    if (page_phyaddr == NULL) {
        lock_release(&mem_pool->lock);
        return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr);
    lock_release(&mem_pool->lock);
    return (void *)vaddr;
}

/* Install a single page of vaddr, specifically for cases where the virtual
 * address bitmap does not need to be modified during a fork */
void *get_a_page_without_opvaddrbitmap(PoolFlag pf, uint32_t vaddr) {
    struct pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);
    void *page_phyaddr = palloc(mem_pool);
    if (page_phyaddr == NULL) {
        lock_release(&mem_pool->lock);
        return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr);
    lock_release(&mem_pool->lock);
    return (void *)vaddr;
}

/* Get the physical address mapped to the virtual address */
uint32_t addr_v2p(uint32_t vaddr) {
    uint32_t *pte = pte_ptr(vaddr);
    /* (*pte) holds the physical page frame address of the page table,
     * remove the lower 12 bits of the page table entry attributes,
     * and add the lower 12 bits of the virtual address vaddr */
    return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

/* Return the address of the idx-th memory block in the arena */
static MemoryBlock *arena2block(struct arena *a, uint32_t idx) {
    return (MemoryBlock *)((uint32_t)a + sizeof(struct arena) +
                           idx * a->desc->block_size);
}

/* Return the arena address where the memory block b is located */
static struct arena *block2arena(MemoryBlock *b) {
    return (struct arena *)((uint32_t)b & 0xfffff000);
}

/* Allocate memory of size 'size' bytes from the heap */
void *sys_malloc(uint32_t size) {
    PoolFlag PF;
    struct pool *mem_pool;
    uint32_t pool_size;
    MemoryBlockDescriptor *descs;
    TaskStruct *cur_thread = running_thread();

    /* Determine which memory pool to use */
    if (cur_thread->pgdir == NULL) { // Kernel thread
        PF = PF_KERNEL;
        pool_size = kernel_pool.pool_size;
        mem_pool = &kernel_pool;
        descs = k_block_descs;
    } else { // User process, its pgdir is created when allocating page table
        PF = PF_USER;
        pool_size = user_pool.pool_size;
        mem_pool = &user_pool;
        descs = cur_thread->u_block_desc;
    }

    /* Return NULL if requested memory size exceeds the pool size */
    if (!(size > 0 && size < pool_size)) {
        return NULL;
    }
    struct arena *a;
    MemoryBlock *b;
    lock_acquire(&mem_pool->lock);

    /* If the size is larger than 1024 bytes, allocate a page frame */
    if (size > 1024) {
        uint32_t page_cnt =
            ROUNDUP(size + sizeof(struct arena),
                    PGSIZE); // Round up to the number of page frames needed

        a = malloc_page(PF, page_cnt);

        if (a != NULL) {
            k_memset(a, 0, page_cnt * PGSIZE); // Clear the allocated memory

            /* For large allocations, set desc to NULL, cnt to the number of
             * pages, and large to true */
            a->desc = NULL;
            a->cnt = page_cnt;
            a->large = true;
            lock_release(&mem_pool->lock);
            return (void *)(a + 1); // Skip the arena header and return the
                                    // remaining memory
        } else {
            lock_release(&mem_pool->lock);
            return NULL;
        }
    } else { // If the requested memory is 1024 bytes or less, fit into
             // available block descriptors
        uint8_t desc_idx;

        /* Find the matching block size from the memory block descriptors */
        for (desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
            if (size <=
                descs[desc_idx].block_size) { // From small to large, stop once
                                              // a match is found
                break;
            }
        }

        /* If no available memory blocks in the free list, allocate a new arena
         */
        if (list_empty(&descs[desc_idx].free_list)) {
            a = malloc_page(PF, 1); // Allocate 1 page frame for the arena
            if (a == NULL) {
                lock_release(&mem_pool->lock);
                return NULL;
            }
            k_memset(a, 0, PGSIZE);

            /* For small allocations, set desc to the matching memory block
             * descriptor, cnt to the number of available blocks in the arena,
             * and large to false */
            a->desc = &descs[desc_idx];
            a->large = false;
            a->cnt = descs[desc_idx].blocks_per_arena;
            uint32_t block_idx;

            Interrupt_Status old_status = set_intr_status(INTR_OFF);

            /* Start splitting the arena into memory blocks and adding them to
             * the free list */
            for (block_idx = 0; block_idx < descs[desc_idx].blocks_per_arena;
                 block_idx++) {
                b = arena2block(a, block_idx);
                KERNEL_ASSERT(!elem_find(&a->desc->free_list, &b->free_elem));
                list_append(&a->desc->free_list, &b->free_elem);
            }
            set_intr_status(old_status);
        }
        /* Start allocating memory blocks */
        b = elem2entry(MemoryBlock, free_elem,
                       list_pop(&(descs[desc_idx].free_list)));
        k_memset(b, 0, descs[desc_idx].block_size);

        a = block2arena(b); // Get the arena of the memory block b
        a->cnt--;           // Decrease the count of free blocks in the arena
        lock_release(&mem_pool->lock);
        return (void *)b;
    }
}

/* Return the physical address pg_phy_addr back to the physical memory pool */
void pfree(uint32_t pg_phy_addr) {
    struct pool *mem_pool;
    uint32_t bit_idx = 0;
    if (pg_phy_addr >= user_pool.phy_addr_start) { // User physical memory pool
        mem_pool = &user_pool;
        bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PGSIZE;
    } else { // Kernel physical memory pool
        mem_pool = &kernel_pool;
        bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PGSIZE;
    }
    bitmap_set(&mem_pool->pool_bitmap, bit_idx, 0); // Set the bitmap bit to 0
}

/* Remove the mapping for the virtual address vaddr in the page table,
 * only removes the page table entry (PTE) for the given virtual address */
static void page_table_pte_remove(uint32_t vaddr) {
    uint32_t *pte = pte_ptr(vaddr);
    *pte &= ~PG_P_1; // Set the present bit of the page table entry to 0
    asm volatile("invlpg %0" ::"m"(vaddr) : "memory"); // Update TLB
}

/* Free the contiguous pg_cnt virtual pages starting from vaddr in the virtual
 * address pool */
static void vaddr_remove(PoolFlag pf, void *_vaddr, uint32_t pg_cnt) {
    uint32_t bit_idx_start = 0, vaddr = (uint32_t)_vaddr, cnt = 0;

    if (pf == PF_KERNEL) { // Kernel virtual memory pool
        bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PGSIZE;
        while (cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
        }
    } else { // User virtual memory pool
        TaskStruct *cur_thread = running_thread();
        bit_idx_start =
            (vaddr - cur_thread->userprog_vaddr.vaddr_start) / PGSIZE;
        while (cnt < pg_cnt) {
            bitmap_set(&cur_thread->userprog_vaddr.vaddr_bitmap,
                       bit_idx_start + cnt++, 0);
        }
    }
}

/* Free the physical pages starting from the virtual address vaddr and spanning
 * pg_cnt pages */
void mfree_page(PoolFlag pf, void *_vaddr, uint32_t pg_cnt) {
    uint32_t pg_phy_addr;
    uint32_t vaddr = (int32_t)_vaddr, page_cnt = 0;
    KERNEL_ASSERT(pg_cnt >= 1 && vaddr % PGSIZE == 0);
    pg_phy_addr =
        addr_v2p(vaddr); // Get the physical address corresponding to vaddr

    /* Ensure that the physical memory to be freed is outside the lower 1M + 1K
     * page directory + 1K page table address range */
    KERNEL_ASSERT((pg_phy_addr % PGSIZE) == 0 && pg_phy_addr >= 0x102000);

    /* Determine if the physical address belongs to the user or kernel pool */
    if (pg_phy_addr >= user_pool.phy_addr_start) { // User pool
        vaddr -= PGSIZE;
        while (page_cnt < pg_cnt) {
            vaddr += PGSIZE;
            pg_phy_addr = addr_v2p(vaddr);

            /* Ensure the physical address belongs to the user physical memory
             * pool */
            KERNEL_ASSERT((pg_phy_addr % PGSIZE) == 0 &&
                   pg_phy_addr >= user_pool.phy_addr_start);

            /* Return the corresponding physical page frame to the pool */
            pfree(pg_phy_addr);

            /* Remove the page table entry for the corresponding virtual address
             */
            page_table_pte_remove(vaddr);

            page_cnt++;
        }
        /* Clear the corresponding bits in the virtual address bitmap */
        vaddr_remove(pf, _vaddr, pg_cnt);

    } else { // Kernel pool
        vaddr -= PGSIZE;
        while (page_cnt < pg_cnt) {
            vaddr += PGSIZE;
            pg_phy_addr = addr_v2p(vaddr);
            /* Ensure that the physical memory to be freed is only within the
             * kernel physical memory pool */
            KERNEL_ASSERT((pg_phy_addr % PGSIZE) == 0 &&
                   pg_phy_addr >= kernel_pool.phy_addr_start &&
                   pg_phy_addr < user_pool.phy_addr_start);

            /* Return the corresponding physical page frame to the pool */
            pfree(pg_phy_addr);

            /* Remove the page table entry for the corresponding virtual address
             */
            page_table_pte_remove(vaddr);

            page_cnt++;
        }
        /* Clear the corresponding bits in the virtual address bitmap */
        vaddr_remove(pf, _vaddr, pg_cnt);
    }
}

/* Free the memory pointed to by ptr */
void sys_free(void *ptr) {
    KERNEL_ASSERT(ptr != NULL);
    if (ptr != NULL) {
        PoolFlag PF;
        struct pool *mem_pool;

        /* Determine whether it's a thread or a process */
        if (running_thread()->pgdir == NULL) {
            KERNEL_ASSERT((uint32_t)ptr >= KERNEL_HEAP_START);
            PF = PF_KERNEL;
            mem_pool = &kernel_pool;
        } else {
            PF = PF_USER;
            mem_pool = &user_pool;
        }

        lock_acquire(&mem_pool->lock);
        MemoryBlock *b = ptr;
        struct arena *a = block2arena(
            b); // Convert the memory block to an arena to get metadata
        KERNEL_ASSERT(a->large == 0 || a->large == 1);
        if (a->desc == NULL &&
            a->large == true) { // Memory larger than 1024 bytes
            mfree_page(PF, a, a->cnt);
        } else { // Memory block smaller than or equal to 1024 bytes
            /* Add the memory block back to the free list */
            list_append(&a->desc->free_list, &b->free_elem);

            /* Check if all blocks in this arena are freed, if so, release the
             * arena */
            if (++a->cnt == a->desc->blocks_per_arena) {
                uint32_t block_idx;
                for (block_idx = 0; block_idx < a->desc->blocks_per_arena;
                     block_idx++) {
                    MemoryBlock *b = arena2block(a, block_idx);
                    KERNEL_ASSERT(elem_find(&a->desc->free_list, &b->free_elem));
                    list_remove(&b->free_elem);
                }
                mfree_page(PF, a, 1);
            }
        }
        lock_release(&mem_pool->lock);
    }
}

/* Initialize the memory pools */
static void mem_pool_init(uint32_t all_mem) {
    verbose_ccputs("   mem_pool_init start\n");
    uint32_t page_table_size =
        PGSIZE * 256; // Page table size = 1 page for the page directory + page
                      // tables for entries 0 and 768, and 254 page tables for
                      // entries 769 to 1022, totaling 256 pages
    uint32_t used_mem =
        page_table_size + 0x100000; // 0x100000 for the lower 1MB memory
    uint32_t free_mem = all_mem - used_mem;
    uint16_t all_free_pages =
        free_mem / PGSIZE; // Total free pages, assuming memory is allocated in
                           // page-size chunks
    uint16_t kernel_free_pages = all_free_pages / 2;
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;

    /* For simplicity, ignore the remainder of free memory and do not check for
       out-of-bound memory, as this would require additional checks, potentially
       losing memory. */
    uint32_t kbm_length =
        kernel_free_pages /
        8; // Length of the Kernel BitMap, 1 bit represents 1 page, in bytes
    uint32_t ubm_length = user_free_pages / 8; // Length of the User BitMap.

    uint32_t kp_start = used_mem; // Kernel Pool start address
    uint32_t up_start =
        kp_start + kernel_free_pages * PGSIZE; // User Pool start address

    kernel_pool.phy_addr_start = kp_start;
    user_pool.phy_addr_start = up_start;

    kernel_pool.pool_size = kernel_free_pages * PGSIZE;
    user_pool.pool_size = user_free_pages * PGSIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
    user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

    /*********    Kernel and User Pool Bitmaps   ***********/
    // The bitmaps are global data structures whose lengths are not fixed.
    // Instead of statically defining the array size, we calculate the required
    // length based on the total memory size. The kernel's highest address is
    // 0xc009f000, which is the stack address of the main thread. The kernel
    // memory size is approximately 70KB, and the bitmap for 32MB memory takes
    // 2KB. The kernel memory pool's bitmap is set to MEM_BITMAP_BASE
    // (0xc009a000).
    kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;

    /* The User memory pool's bitmap starts right after the kernel's bitmap */
    user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length);
    /******************** Output memory pool information **********************/
    verbose_ccputs("      kernel_pool_bitmap_start:");
    verbose_ccint((int)kernel_pool.pool_bitmap.bits);
    verbose_ccputs(" kernel_pool_phy_addr_start:");
    verbose_ccint(kernel_pool.phy_addr_start);
    verbose_ccputs("\n");
    verbose_ccputs("      user_pool_bitmap_start:");
    verbose_ccint((int)user_pool.pool_bitmap.bits);
    verbose_ccputs(" user_pool_phy_addr_start:");
    verbose_ccint(user_pool.phy_addr_start);
    verbose_ccputs("\n");

    /* Initialize the bitmaps to 0 */
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    lock_init(&kernel_pool.lock);
    lock_init(&user_pool.lock);

    /* Initialize the kernel virtual address bitmap */
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len =
        kbm_length; // Used to manage virtual addresses for the kernel heap, so
                    // it matches the kernel memory pool size

    /* The bitmap array points to unused memory located outside the kernel and
     * user pools */
    kernel_vaddr.vaddr_bitmap.bits =
        (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);

    kernel_vaddr.vaddr_start = KERNEL_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    verbose_ccputs("   mem_pool_init done\n");
}

/* Prepare for malloc by initializing memory block descriptors */
void block_desc_init(MemoryBlockDescriptor *desc_array) {
    uint16_t block_size = 16;

    /* Initialize each memory block descriptor */
    for (uint16_t desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
        desc_array[desc_idx].block_size = block_size;

        /* Initialize the number of memory blocks in an arena */
        desc_array[desc_idx].blocks_per_arena =
            (PGSIZE - sizeof(struct arena)) / block_size;

        list_init(&desc_array[desc_idx].free_list);

        block_size *= 2; // Update to the next memory block size
    }
}

/* Clear the bitmap for a physical page address (pg_phy_addr) in the
   corresponding memory pool, without modifying the page table */
void free_a_phy_page(uint32_t pg_phy_addr) {
    struct pool *mem_pool;
    uint32_t bit_idx = 0;
    if (pg_phy_addr >= user_pool.phy_addr_start) {
        mem_pool = &user_pool;
        bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PGSIZE;
    } else {
        mem_pool = &kernel_pool;
        bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PGSIZE;
    }
    bitmap_set(&mem_pool->pool_bitmap, bit_idx, 0);
}

/* Memory management initialization entry point */
void memory_management_init() {
    verbose_ccputs("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
    mem_pool_init(mem_bytes_total); // Initialize the memory pool
    /* Initialize the memory block descriptor array descs, preparing for malloc
     */
    block_desc_init(k_block_descs);
    verbose_ccputs("mem_init done\n");
}
