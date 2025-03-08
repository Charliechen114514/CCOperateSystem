#include "include/user/process/process.h"
#include "include/device/console_tty.h"
#include "include/kernel/interrupt.h"
#include "include/property/selectors.h"
#include "include/library/kernel_assert.h"
#include "include/library/list.h"
#include "include/library/string.h"
#include "include/user/tss/tss.h"
#include "include/memory/memory_settings.h"
#include "include/tools/math_tools.h"
#include "include/memory/memory_tools.h"

extern void intr_exit(void);


/* Build the initial context for a user process */
static void start_process(void *filename_) {
    void *function = filename_; // The entry point of the user process
    TaskStruct *cur = current_thread();
    cur->self_kstack += sizeof(ThreadStack); // Move the stack pointer to the correct location
    Interrupt_Stack *proc_stack =
        (Interrupt_Stack *)cur->self_kstack; // Set up the interrupt stack
    proc_stack->edi = proc_stack->esi = proc_stack->ebp =
        proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0; // User mode does not use the gs register, set to 0
    proc_stack->ds = proc_stack->es = proc_stack->fs =
        SELECTOR_U_DATA;              // Set the data segment selector
    proc_stack->eip = function;       // The address of the function to execute
    proc_stack->cs = SELECTOR_U_CODE; // Set the code segment selector
    proc_stack->eflags =
        (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1); // Set flags for user mode
    proc_stack->esp =
        (void *)((uint32_t)get_a_page(PF_USER, USER_STACK3_VADDR) +
                 PG_SIZE);             // Set up user stack
    proc_stack->ss = SELECTOR_U_DATA; // Set the stack segment selector
    asm volatile("movl %0, %%esp; \
         jmp intr_exit"
                 :
                 : "g"(proc_stack)
                 : "memory"); // Switch to the user process context
}

/* Activate the page directory */
void page_dir_activate(TaskStruct *p_thread) {
    /********************************************************
     * This function may be called while the current task is a thread.
     * The reason we also need to reload the page table for threads is that
     * the last task switched might have been a process, and without restoring
     * the page table, the thread will mistakenly use the process's page table.
     ********************************************************/

    /* If it's a kernel thread, set the page directory to 0x100000 (kernel
     * space) */
    uint32_t pagedir_phy_addr = KERNEL_PAGE_MAPPINGS; // Default for kernel thread page directory
    if (p_thread->pg_dir) { // If the thread has its own page directory (user process)
        pagedir_phy_addr = addr_v2p((uint32_t)p_thread->pg_dir);
    }

    /* Update the CR3 register to activate the new page table */
    asm volatile("movl %0, %%cr3" : : "r"(pagedir_phy_addr) : "memory");
}

/* Activate the page table for a process or thread and update the TSS's esp0 for
 * privilege level 0 stack */
void activate_process_settings(TaskStruct *p_thread) {
    KERNEL_ASSERT(p_thread);
    /* Activate the page table for the process or thread */
    page_dir_activate(p_thread);

    /* If it's a kernel thread, its privilege level is already 0,
       so the processor doesn't need to fetch the stack address from the TSS
       during interrupts. */
    if (p_thread->pg_dir) {
        /* Update esp0 in the TSS for the process's privilege level 0 stack
         * during interrupts */
        update_tss_esp(p_thread);
    }
}

/* Create a new page directory, copying the kernel space entries from the
   current page directory. Returns the virtual address of the new page
   directory, or NULL on failure. */
uint32_t *create_page_dir(void) {

    /* The user process's page table cannot be directly accessed, so we allocate
     * memory in the kernel space */
    uint32_t *page_dir_vaddr = get_kernel_pages(1);
    if (!page_dir_vaddr) {
        console_ccos_puts("create_page_dir: get_kernel_page failed!");
        return NULL;
    }

    /************************** 1. Copy the current page table entries for
     * kernel space *************************************/
    /* Copy the kernel page directory entries starting at 0x300 (768th entry in
     * the page directory) */
    k_memcpy((uint32_t *)((uint32_t)page_dir_vaddr + KERNEL_PGTB_INDEX * PDE_BYTES_LEN),
           (uint32_t *)(PG_FETCH_OFFSET + KERNEL_PGTB_INDEX * PDE_BYTES_LEN), PDE_ENTRY_NR);
    /*****************************************************************************/

    /************************** 2. Update the page directory address
     * **********************************/
    uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);
    /* The page directory address is stored in the last entry of the page table.
       Update it with the new page directory's physical address */
    page_dir_vaddr[PDE_ENTRY_NR - 1] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
    /*****************************************************************************/

    return page_dir_vaddr; // Return the virtual address of the newly created
                           // page directory
}

/* Create the user process's virtual address bitmap */
static void create_user_vaddr_bitmap(TaskStruct *user_prog) {
    user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;
    uint32_t bitmap_pg_cnt =
        ROUNDUP((KERNEL_V_START - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    user_prog->userprog_vaddr.virtual_mem_bitmap.bits =
        get_kernel_pages(bitmap_pg_cnt);
    user_prog->userprog_vaddr.virtual_mem_bitmap.btmp_bytes_len =
        (KERNEL_V_START - USER_VADDR_START) / PG_SIZE / 8;
    bitmap_init(&user_prog->userprog_vaddr.virtual_mem_bitmap);
}

/* Execute a user process */
void create_process(void *filename, char *name) {
    /* Allocate memory for the process control block (PCB) in the kernel memory
     * pool */
    TaskStruct *thread = get_kernel_pages(PCB_SZ_PG_CNT);
    init_thread(thread, name, DEFAULT_PRIO); // Initialize the thread's PCB
    create_user_vaddr_bitmap(
        thread); // Create a virtual address bitmap for the user process
    create_thread(thread, start_process,
                  filename); // Create the thread to start the user process
    thread->pg_dir =
        create_page_dir(); // Create a page directory for the process
    block_desc_init(thread->u_block_desc);
    /* Add the thread to the ready list and all thread list */
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    KERNEL_ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);

    KERNEL_ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);
    set_intr_status(old_status); // Restore the interrupt state
}
