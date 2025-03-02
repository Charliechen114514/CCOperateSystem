#include "include/user/program/wait_exit.h"
#include "include/io/stdio-kernel.h"
#include "include/filesystem/file.h"
#include "include/filesystem/filesystem.h"
#include "include/thread/thread.h"
#include "include/library/bitmap.h"
#include "include/library/kernel_assert.h"
#include "include/library/list.h"
#include "include/library/types.h"
#include "include/memory/memory.h"
#include "include/memory/memory_settings.h"
#include "include/user/ccshell/pipe.h"

/* Release user process resources:
 * 1. Physical pages corresponding to page tables
 * 2. Virtual memory pool that occupies physical page frames
 * 3. Close open files */
static void release_prog_resource(TaskStruct *release_thread) {
    uint32_t *pgdir_vaddr = release_thread->pg_dir;
    uint16_t user_pde_nr = 768, pde_idx = 0;
    uint32_t pde = 0;
    uint32_t *v_pde_ptr =
        NULL; // v represents var, distinguishing it from the function pde_ptr

    uint16_t user_pte_nr = 1024, pte_idx = 0;
    uint32_t pte = 0;
    uint32_t *v_pte_ptr =
        NULL; // Adding 'v' to distinguish it from the function pte_ptr

    uint32_t *first_pte_vaddr_in_pde =
        NULL; // Used to record the address of the first pte in pde
    uint32_t pg_phy_addr = 0;

    /* Reclaim physical pages in the user space page tables */
    while (pde_idx < user_pde_nr) {
        v_pde_ptr = pgdir_vaddr + pde_idx;
        pde = *v_pde_ptr;
        if (pde & PG_P_1) { // If the page directory entry's 'present' bit is set
            first_pte_vaddr_in_pde = pte_ptr(
                pde_idx *
                0x400000); // A page table represents 4MB, i.e., 0x400000
            pte_idx = 0;
            while (pte_idx < user_pte_nr) {
                v_pte_ptr = first_pte_vaddr_in_pde + pte_idx;
                pte = *v_pte_ptr;
                if (pte & PG_P_1) {
                    /* Clear the corresponding physical page frame in the memory
                     * pool bitmap */
                    pg_phy_addr = pte & PG_FETCH_OFFSET;
                    free_a_phy_page(pg_phy_addr);
                }
                pte_idx++;
            }
            /* Clear the physical page frame recorded in pde from the memory
             * pool bitmap */
            pg_phy_addr = pde & PG_FETCH_OFFSET;
            free_a_phy_page(pg_phy_addr);
        }
        pde_idx++;
    }

    /* Reclaim physical memory occupied by the user virtual address pool */
    uint32_t bitmap_pg_cnt =
        (release_thread->userprog_vaddr.vaddr_bitmap.btmp_bytes_len) / PG_SIZE;
    uint8_t *user_vaddr_pool_bitmap =
        release_thread->userprog_vaddr.vaddr_bitmap.bits;
    mfree_page(PF_KERNEL, user_vaddr_pool_bitmap, bitmap_pg_cnt);

    /* Close files opened by the process */
    uint8_t local_fd = 3;
    while (local_fd < MAX_FILES_OPEN_PER_PROC) {
        if (release_thread->fd_table[local_fd] != -1) {
            if (is_pipe(local_fd)) {
                uint32_t global_fd = fd_local2global(local_fd);
                if (--file_table[global_fd].fd_pos == 0) {
                    mfree_page(PF_KERNEL, file_table[global_fd].fd_inode, 1);
                    file_table[global_fd].fd_inode = NULL;
                }
            } else {
                sys_close(local_fd);
            }
        }
        local_fd++;
    }
}

/* list_traversal callback function,
 * Find if the parent_pid of pelem is ppid, return true if found, otherwise
 * return false */
static bool find_child(list_elem *pelem, int32_t ppid)
{
    TaskStruct *pthread = elem2entry(TaskStruct, all_list_tag, pelem);
    if (pthread->parent_pid ==
        ppid)
    {                // If the parent_pid of the thread matches the ppid
        return true; // list_traversal stops if the callback function returns
                     // true, so we return true here
    }
    return false; // Let list_traversal continue to the next element
}

/* list_traversal callback function,
 * Find tasks in TASK_HANGING state */
static bool find_hanging_child(list_elem *pelem, int32_t ppid)
{
    TaskStruct *pthread = elem2entry(TaskStruct, all_list_tag, pelem);
    if (pthread->parent_pid == ppid && pthread->status == TASK_HANGING)
    {
        return true;
    }
    return false;
}

/* list_traversal callback function,
 * Adopt a child process to init */
static bool init_adopt_a_child(list_elem *pelem, int32_t pid)
{
    TaskStruct *pthread = elem2entry(TaskStruct, all_list_tag, pelem);
    if (pthread->parent_pid ==
        pid)
    {                            // If the parent_pid of the process matches pid
        pthread->parent_pid = 1; // Adopt by init process
    }
    return false; // Let list_traversal continue to the next element
}

/* Wait for child process to call exit, save the exit status to the variable
 * pointed by status. Return child process pid on success, -1 on failure */
pid_t sys_wait(int32_t *status)
{
    TaskStruct *parent_thread = running_thread();

    while (1)
    {
        /* Handle tasks that are already in TASK_HANGING state first */
        list_elem *child_elem = list_traversal(
            &thread_all_list, find_hanging_child, parent_thread->pid);
        /* If there is a hanging child process */
        if (child_elem != NULL)
        {
            TaskStruct *child_thread =
                elem2entry(TaskStruct, all_list_tag, child_elem);
            *status = child_thread->exit_status;

            /* Get the pid before thread_exit as pcb will be reclaimed after
             * thread_exit */
            uint16_t child_pid = child_thread->pid;

            /* Remove the process from the ready and all thread lists */
            thread_exit(
                child_thread,
                false); // Pass false to return to this place after thread_exit
            /* At this point, the process has completely disappeared */

            return child_pid;
        }

        /* Check if there are any child processes */
        child_elem =
            list_traversal(&thread_all_list, find_child, parent_thread->pid);
        if (child_elem == NULL)
        { // If no child processes, return -1
            return -1;
        }
        else
        {
            /* If the child process has not exited yet, block the parent process
             * until the child process exits */
            thread_block(TASK_WAITING);
        }
    }
}

/* Used by the child process to terminate itself */
void sys_exit(int32_t status)
{
    TaskStruct *child_thread = running_thread();
    child_thread->exit_status = status;
    if (child_thread->parent_pid == -1)
    {
        KERNEL_PANIC_SPIN("sys_exit: child_thread->parent_pid is -1\n");
    }

    /* Adopt all the child processes of child_thread to init */
    list_traversal(&thread_all_list, init_adopt_a_child, child_thread->pid);

    /* Reclaim resources of child_thread */
    release_prog_resource(child_thread);

    /* If the parent process is waiting for the child process to exit, wake it
     * up */
    TaskStruct *parent_thread = pid2thread(child_thread->parent_pid);
    if (parent_thread->status == TASK_WAITING)
    {
        thread_unblock(parent_thread);
    }

    /* Block itself until the parent process retrieves its status and reclaims
     * its pcb */
    thread_block(TASK_HANGING);
}
