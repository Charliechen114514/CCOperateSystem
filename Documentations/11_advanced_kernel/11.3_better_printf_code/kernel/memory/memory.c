#include "include/memory/memory_settings.h"
#include "include/memory/memory_tools.h"
#include "include/memory/memory.h"
#include "include/tools/math_tools.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/library/ccos_print.h"
#include "include/kernel/lock.h"

// Physical Memory Pools definition
// The 'MemoryPool' structure represents a physical memory pool, managing the pool's bitmap,
// the starting address of the physical memory, and the pool's total size in bytes.
typedef struct __mem_pool
{
    Bitmap pool_bitmap;      // Bitmap used by this pool to track allocated and free memory pages
    uint32_t phy_addr_start; // The start address of the physical memory managed by this pool
    uint32_t pool_size;      // Total size of this memory pool in bytes
    CCLocker lock;
} MemoryPool;

// Declare the kernel and user memory pools, and the structure for virtual address mappings
MemoryPool kernel_pool, user_pool;   // Kernel and user memory pools
VirtualAddressMappings kernel_vaddr; // Virtual address mapping for kernel space

/* Allocate virtual memory pages from the specified pool (kernel or user).
 * If successful, return the starting virtual address; otherwise, return NULL. */
static void *vaddr_get(const PoolFlag pf, const uint32_t pg_cnt)
{
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;

    switch (pf) // Determine which pool to allocate from based on the PoolFlag
    {
        case PF_KERNEL: // If the pool is the kernel memory pool
        {
            bit_idx_start = bitmap_scan(&kernel_vaddr.virtual_mem_bitmap, pg_cnt); // Find the first free block of 'pg_cnt' pages
            if (bit_idx_start == -1)
            {
                return NULL; // If no free block is found, return NULL
            }
            while (cnt < pg_cnt)
            {
                bitmap_set(&kernel_vaddr.virtual_mem_bitmap, bit_idx_start + cnt, 1); // Mark the pages as allocated in the bitmap
                cnt++;                                                                // Increment the page count
            }
            vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE; // Calculate the starting virtual address of the allocated pages                                                               
        }break;// Exit the case block
        case PF_USER: // If the pool is the user memory pool
        {
            TaskStruct *cur = current_thread();
            bit_idx_start = bitmap_scan(&cur->userprog_vaddr.virtual_mem_bitmap, pg_cnt);
            if (bit_idx_start == -1)
            {
                return NULL;
            }

            while (cnt < pg_cnt)
            {
                bitmap_set(&cur->userprog_vaddr.virtual_mem_bitmap, bit_idx_start + cnt++, 1);
            }
            vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;

            /* (KERNEL_V_START - PG_SIZE) is already allocated as the user-level 3rd
            * stack in start_process */
            KERNEL_ASSERT((uint32_t)vaddr_start < (KERNEL_V_START - PG_SIZE));
    }
    }
    return (void *)vaddr_start; // Return the starting virtual address of the allocated pages
}

/* Allocate 1 physical page from the memory pool m_pool.
 * If successful, returns the physical address of the page; otherwise, returns
 * NULL */
static void *palloc(MemoryPool *m_pool)
{
    /* Bitmap scan or setting should be atomic */
    int bit_idx =
        bitmap_scan(&m_pool->pool_bitmap, 1); // Find an available physical page
    if (bit_idx == -1)
    {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap, bit_idx,
               1); // Set the bit at index bit_idx to 1
    return (void *)((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
}

// Function to add a mapping between a virtual address and a physical address in the page table
static void page_table_add(const void *_vaddr, const void *_page_phyaddr)
{
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t *pde = pde_ptr(vaddr); // Get the pointer to the page directory entry
    uint32_t *pte = pte_ptr(vaddr); // Get the pointer to the page table entry

    /************************   Attention   *************************
     * When accessing *pte, it may refer to an empty pde.
     * Therefore, ensure that the pde is created before accessing *pte.
     * Otherwise, a page fault will occur.
     * If *pde is 0, the *pte can only be accessed after *pde is properly initialized.
     ************************************************************/

    // First, check the existence of the page directory entry (PDE)
    // If the PDE exists (P bit is 1), we can proceed to work with the PTE
    if (*pde & PG_P_1)
    {
        // Check if the PTE exists. If it does, no error; otherwise, create it
        if (*pte & PG_P_1)
        {
            KERNEL_PANIC_SPIN("pte repeat");
        }
        else
        {
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); // Create the PTE with necessary flags
        }
    }
    else
    { // If the PDE does not exist, we need to create it first
        // Allocate physical memory for the page directory entry (PDE) from the kernel space
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);

        // Initialize the PDE with the allocated physical address, setting appropriate flags
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

        // Clear the physical memory allocated for the PDE to avoid any old data being interpreted as page table entries
        k_memset((void *)((int)pte & PG_FETCH_OFFSET), 0, PG_SIZE);

        // Ensure that the PTE is not set already before proceeding to write
        KERNEL_ASSERT(!(*pte & PG_P_1));

        // Now create the PTE with the appropriate flags (US=1, RW=1, P=1)
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

static bool accept_allocate_policy(const uint32_t pg_cnt)
{
    return pg_cnt > 0 &&
           pg_cnt < (MAX_ACCEPT_SINGLE_ALLOCATE_MB * MB / PG_SIZE);
}

// Function to allocate a specified number of pages and map them between virtual and physical memory
void *malloc_page(const PoolFlag pf, const uint32_t pg_cnt)
{
    KERNEL_ASSERT(accept_allocate_policy(pg_cnt)); // Ensure the requested page count is valid

    /*********** The malloc_page function performs three actions: ***********
    1. Allocates virtual addresses through vaddr_get.
    2. Allocates physical pages through palloc.
    3. Maps the virtual and physical addresses in the page table via page_table_add.
    ********************************************************************/

    // Get the starting virtual address for the requested number of pages
    void *vaddr_start = vaddr_get(pf, pg_cnt);
    if (vaddr_start == NULL)
    {
        return NULL; // Return NULL if the virtual address allocation fails
    }

    uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
    MemoryPool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool; // Select the appropriate memory pool

    // The virtual address is contiguous, but physical addresses may not be. Therefore, handle the mapping one page at a time.
    while (cnt-- > 0)
    {
        // Allocate a physical page from the chosen memory pool
        void *page_phyaddr = palloc(mem_pool);
        if (!page_phyaddr)
        { // If allocation fails, revert previously allocated pages
            return NULL;
        }

        // Add the mapping between the virtual address and physical page in the page table
        page_table_add((void *)vaddr, page_phyaddr);

        vaddr += PG_SIZE; // Move to the next virtual page
    }

    return vaddr_start; // Return the starting virtual address of the allocated space
}

// Function to allocate a specified number of kernel pages and return their virtual addresses
void *get_kernel_pages(const uint32_t kpage_count)
{
    lock_acquire(&kernel_pool.lock);
    void *vaddr = malloc_page(PF_KERNEL, kpage_count); // Request kernel pages
    if (vaddr)
    { // If successful, clear the allocated pages and return the address
        k_memset(vaddr, 0, kpage_count * PG_SIZE);
    }
    lock_release(&kernel_pool.lock);
    return vaddr; // Return the virtual address of the allocated kernel pages
}

/* Allocate 4KB of memory from the user space and return the corresponding
 * virtual address */
void *get_user_pages(uint32_t pg_cnt) {
    lock_acquire(&user_pool.lock);
    void *vaddr = malloc_page(PF_USER, pg_cnt);
    if (vaddr != NULL) { // If the allocated address is not NULL, clear the
                         // pages before returning
        k_memset(vaddr, 0, pg_cnt * PG_SIZE);
    }
    lock_release(&user_pool.lock);
    return vaddr;
}


/* Associate the address vaddr with the physical address in the pf pool,
 * supports only single page allocation */
void *get_a_page(PoolFlag pf, uint32_t vaddr) {

    MemoryPool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);

    /* First, set the corresponding bit in the virtual address bitmap */
    TaskStruct *cur = current_thread();
    int32_t bit_idx = -1;

    /* If the current thread is a user process requesting user memory, modify
     * the user process's virtual address bitmap */
    if (cur->pg_dir != NULL && pf == PF_USER) {
        bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PG_SIZE;
        KERNEL_ASSERT(bit_idx >= 0);
        bitmap_set(&cur->userprog_vaddr.virtual_mem_bitmap, bit_idx, 1);

    } else if (cur->pg_dir == NULL && pf == PF_KERNEL) {
        /* If a kernel thread is requesting kernel memory, modify kernel_vaddr.
         */
        bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
        KERNEL_ASSERT(bit_idx > 0);
        bitmap_set(&kernel_vaddr.virtual_mem_bitmap, bit_idx, 1);
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





// Function to initialize the physical memory pools
static void init_memory_pool(const uint32_t all_memory)
{
    verbose_ccputs("     memory pool init process start\n");

    // Calculate the size of a single page table in bytes (256 entries * 4 bytes each)
    const uint32_t PAGETABLE_SZ = PG_SIZE * 256;

    // Reserved memory for initial kernel setup (includes page table and low memory)
    const uint32_t used_memory = PAGETABLE_SZ + KERNEL_LOW_1M_MEM;

    // Calculate the free memory available for the pools (total memory - used memory)
    const uint32_t free_memory = all_memory - used_memory;

    // Calculate the number of free pages available
    const uint16_t all_free_pages = free_memory / PG_SIZE;

    // Split the available free memory between kernel and user spaces (50/50 split here)
    const uint16_t all_free_kernel_pages = all_free_pages / 2;
    const uint16_t all_free_user_pages = all_free_pages - all_free_kernel_pages;

    // Calculate the size of the bitmaps for the kernel and user memory pools
    const uint32_t kbm_length = all_free_kernel_pages / 8;
    const uint32_t ubm_length = all_free_user_pages / 8;

    // Set the starting physical address for the kernel and user memory pools
    const uint32_t kernel_pool_start = used_memory;
    const uint32_t user_pool_start = kernel_pool_start + all_free_kernel_pages * PG_SIZE;

    // Initialize the kernel pool's properties
    kernel_pool.pool_size = all_free_kernel_pages * PG_SIZE;   // Size in bytes
    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;       // Bitmap size
    kernel_pool.pool_bitmap.bits = (void *)MEMORY_BITMAP_BASE; // Bitmap location in memory
    kernel_pool.phy_addr_start = kernel_pool_start;            // Starting physical address

    // Initialize the user pool's properties
    user_pool.pool_size = all_free_user_pages * PG_SIZE;                    // Size in bytes
    user_pool.pool_bitmap.bits = (void *)(MEMORY_BITMAP_BASE + kbm_length); // Bitmap location in memory
    user_pool.pool_bitmap.btmp_bytes_len = ubm_length;                      // Bitmap size
    user_pool.phy_addr_start = user_pool_start;                             // Starting physical address

    // Initialize the bitmaps for both kernel and user memory pools to 0 (free all memory pages)
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);
    // init lockers
    lock_init(&kernel_pool.lock);
    lock_init(&user_pool.lock);
    /******************** Output memory pool information **********************/
    verbose_ccputs("         kernel_pool_bitmap_start:");
    verbose_ccint((int)kernel_pool.pool_bitmap.bits); // Print the address of the kernel pool bitmap
    verbose_ccputs("\n");

    verbose_ccputs("         kernel_pool_phy_addr_start:");
    verbose_ccint(kernel_pool.phy_addr_start); // Print the starting physical address of the kernel pool
    verbose_ccputs("\n");

    verbose_ccputs("         user_pool_bitmap_start:");
    verbose_ccint((int)user_pool.pool_bitmap.bits); // Print the address of the user pool bitmap
    verbose_ccputs("\n");

    verbose_ccputs("         user_pool_phy_addr_start:");
    verbose_ccint(user_pool.phy_addr_start); // Print the starting physical address of the user pool
    verbose_ccputs("\n");

    // Initialize the kernel's virtual address mappings bitmap and set the starting address
    kernel_vaddr.virtual_mem_bitmap.btmp_bytes_len = kbm_length;
    kernel_vaddr.virtual_mem_bitmap.bits = (void *)(MEMORY_BITMAP_BASE + kbm_length + ubm_length); // Start of virtual memory bitmap
    kernel_vaddr.vaddr_start = KERNEL_HEAP_V_START;                                                // Starting virtual address for kernel heap
    bitmap_init(&kernel_vaddr.virtual_mem_bitmap);                                                 // Initialize the virtual address mapping bitmap

    verbose_ccputs("     memory pool init done\n");
}


// Function to initialize memory management system
void memory_management_init(void)
{
    verbose_ccputs("mem_init start\n");

    // Fetch the total memory size from the system
    const uint32_t mem_bytes_total = MEMORY_SZ_DETECT;

    // Initialize memory pools based on the total available memory size
    init_memory_pool(mem_bytes_total);

    verbose_ccputs("mem_init done\n");
}
