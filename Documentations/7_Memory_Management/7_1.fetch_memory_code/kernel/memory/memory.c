#include "include/memory/memory_settings.h"
#include "include/memory/memory.h"
#include "include/library/ccos_print.h"

// Physical Memory Pools definition
// The 'MemoryPool' structure represents a physical memory pool, managing the pool's bitmap,
// the starting address of the physical memory, and the pool's total size in bytes.
typedef struct __mem_pool
{
    Bitmap pool_bitmap;      // Bitmap used by this pool to track allocated and free memory pages
    uint32_t phy_addr_start; // The start address of the physical memory managed by this pool
    uint32_t pool_size;      // Total size of this memory pool in bytes
} MemoryPool;

// Declare the kernel and user memory pools, and the structure for virtual address mappings
MemoryPool kernel_pool, user_pool;   // Kernel and user memory pools
VirtualAddressMappings kernel_vaddr; // Virtual address mapping for kernel space

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
