#include "include/user/program/fork.h"
#include "include/filesystem/file.h"
#include "include/filesystem/inode.h"
#include "include/kernel/interrupt.h"
#include "include/thread/thread.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/memory/memory.h"
#include "include/memory/memory_settings.h"
#include "include/user/program/process.h"

extern void intr_exit(void);

/* Copy parent process's PCB, virtual address bitmap, and stack to child process
 */
static int32_t copy_pcb_vaddrbitmap_stack0(TaskStruct *child_thread,
                                           TaskStruct *parent_thread) {
    /* Copy the entire page containing PCB information and level 0 stack, then
     * modify individual parts */
    k_memcpy(child_thread, parent_thread,
           PG_SIZE);                 // Copy parent process's PCB information
    child_thread->pid = fork_pid(); // Assign a PID to the child process
    child_thread->elapsed_ticks =
        0; // Initialize the child process's elapsed ticks
    child_thread->status = TASK_READY; // Set the child process status to ready
    child_thread->ticks =
        child_thread->priority; // Allocate the time slice to the child process
    child_thread->parent_pid =
        parent_thread->pid; // Set the parent PID for the child process
    child_thread->general_tag.prev = child_thread->general_tag.next =
        NULL; // Initialize task list links
    child_thread->all_list_tag.prev = child_thread->all_list_tag.next = NULL;
    block_desc_init(
        child_thread->u_block_desc); // Initialize memory block descriptors for
                                     // the child process

    /* Copy the virtual address bitmap of the parent process */
    uint32_t bitmap_pg_cnt =
        ROUNDUP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    void *vaddr_btmp =
        get_kernel_pages(bitmap_pg_cnt); // Allocate kernel pages for the bitmap
    if (vaddr_btmp == NULL)
        return -1; // Return error if memory allocation fails

    /* Copy the virtual address bitmap to the child process's allocated memory
     */
    k_memcpy(vaddr_btmp, child_thread->userprog_vaddr.vaddr_bitmap.bits,
           bitmap_pg_cnt * PG_SIZE);
    child_thread->userprog_vaddr.vaddr_bitmap.bits = vaddr_btmp;

    return 0;
}

/* Copy the child process's code, data, and user stack from the parent process
 */
static void copy_body_stack3(TaskStruct *child_thread,
                             TaskStruct *parent_thread, void *buf_page) {
    uint8_t *vaddr_btmp = parent_thread->userprog_vaddr.vaddr_bitmap.bits;
    uint32_t btmp_bytes_len =
        parent_thread->userprog_vaddr.vaddr_bitmap.btmp_bytes_len;
    uint32_t vaddr_start = parent_thread->userprog_vaddr.vaddr_start;
    uint32_t idx_byte = 0;
    uint32_t idx_bit = 0;
    uint32_t prog_vaddr = 0;

    /* Scan the parent process's user space for allocated pages */
    while (idx_byte < btmp_bytes_len) {
        if (vaddr_btmp[idx_byte]) { // If the page is allocated in the bitmap
            idx_bit = 0;
            while (idx_bit < 8) {
                if ((BITMAP_MASK << idx_bit) & vaddr_btmp[idx_byte]) {
                    prog_vaddr =
                        (idx_byte * 8 + idx_bit) * PG_SIZE + vaddr_start;

                    /* Copy data from the parent process's user space to the
                    kernel buffer buf_page so that it can be copied to the child
                    process's space after page table switch */
                    k_memcpy(buf_page, (void *)prog_vaddr, PG_SIZE);

                    /* Switch to the child process's page table to prevent
                     * modifying the parent process's page table */
                    page_dir_activate(child_thread);

                    /* Allocate virtual address prog_vaddr for the child process
                     */
                    get_a_page_without_opvaddrbitmap(PF_USER, prog_vaddr);

                    /* Copy data from the kernel buffer to the child process's
                     * user space */
                    k_memcpy((void *)prog_vaddr, buf_page, PG_SIZE);

                    /* Switch back to the parent process's page table */
                    page_dir_activate(parent_thread);
                }
                idx_bit++;
            }
        }
        idx_byte++;
    }
}

/* Build the child process's thread stack and set return value */
static int32_t build_child_stack(TaskStruct *child_thread) {
    /* a Set the child process's PID return value to 0 */
    Interrupt_Stack *intr_0_stack =
        (Interrupt_Stack *)((uint32_t)child_thread + PG_SIZE -
                            sizeof(Interrupt_Stack));
    intr_0_stack->eax = 0; // Set the return value of the child process to 0

    /* b Build the ThreadStack for switch_to, placed below the interrupt stack
     */
    uint32_t *ret_addr_in_thread_stack = (uint32_t *)intr_0_stack - 1;

    /***   These lines are not necessary but help to clarify the thread_stack
     * structure ***/
    uint32_t *esi_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 2;
    uint32_t *edi_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 3;
    uint32_t *ebx_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 4;
    /**********************************************************/

    /* Set the stack pointer (ebp) in the thread stack as the top of the stack
     * for switch_to */
    uint32_t *ebp_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 5;

    /* Update the return address for switch_to to intr_exit, so the program
     * returns from interrupt */
    *ret_addr_in_thread_stack = (uint32_t)intr_exit;

    /* These assignments are just to make the thread_stack clearer; they are not
    strictly needed, as they will be overwritten by subsequent pops during
    interrupt return */
    *ebp_ptr_in_thread_stack = *ebx_ptr_in_thread_stack =
        *edi_ptr_in_thread_stack = *esi_ptr_in_thread_stack = 0;
    /*********************************************************/

    /* Set the top of the thread_stack as the stack pointer to be restored
     * during switch_to */
    child_thread->self_kstack = ebp_ptr_in_thread_stack;
    return 0;
}

/* Update inode open count */
static void update_inode_open_cnts(TaskStruct *thread) {
    int32_t local_fd = 3, global_fd = 0;
    while (local_fd < MAX_FILES_OPEN_PER_PROC) {
        global_fd = thread->fd_table[local_fd];
        KERNEL_ASSERT(global_fd < MAX_FILE_OPEN);
        if (global_fd != -1) {
            file_table[global_fd].fd_inode->i_open_cnts++; // Increment inode open count for
                                      // regular files
        }
        local_fd++;
    }
}

/* Copy the resources occupied by the parent process to the child process */
static int32_t copy_process(TaskStruct *child_thread,
                            TaskStruct *parent_thread) {
    /* Kernel buffer to temporarily hold data from parent process's user space
     * to be copied to the child process */
    void *buf_page = get_kernel_pages(1);
    if (buf_page == NULL) {
        return -1;
    }

    /* a Copy the parent's PCB, virtual address bitmap, and kernel stack to the
     * child process */
    if (copy_pcb_vaddrbitmap_stack0(child_thread, parent_thread) == -1) {
        return -1;
    }

    /* b Create a page table for the child process, which only includes kernel
     * space */
    child_thread->pg_dir = create_page_dir();
    if (child_thread->pg_dir == NULL) {
        return -1;
    }

    /* c Copy the parent process's code and user stack to the child process */
    copy_body_stack3(child_thread, parent_thread, buf_page);

    /* Continue with other operations such as setting up the child thread's
     * stack and updating inode counts */
    build_child_stack(child_thread);
    update_inode_open_cnts(child_thread);

    mfree_page(PF_KERNEL, buf_page, 1);
    return 0;
}

/* Fork the child process. Kernel threads cannot directly call this function */
pid_t sys_fork(void) {
    TaskStruct *parent_thread = current_thread();
    TaskStruct *child_thread =
        get_kernel_pages(1); // Create PCB for the child process
    if (child_thread == NULL) {
        return -1; // Return error if memory allocation fails
    }
    KERNEL_ASSERT(INTR_OFF == get_intr_status() && parent_thread->pg_dir != NULL);

    /* Copy resources from the parent process to the child process */
    if (copy_process(child_thread, parent_thread) == -1) {
        return -1;
    }

    /* Add the child process to the ready thread list and all thread list,
     * allowing the scheduler to manage it */
    KERNEL_ASSERT(!elem_find(&thread_ready_list, &child_thread->general_tag));
    list_append(&thread_ready_list, &child_thread->general_tag);
    KERNEL_ASSERT(!elem_find(&thread_all_list, &child_thread->all_list_tag));
    list_append(&thread_all_list, &child_thread->all_list_tag);

    return child_thread
        ->pid; // Return the PID of the child process to the parent
}
