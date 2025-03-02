#include "include/filesystem/file.h"
#include "include/io/stdio-kernel.h"
#include "include/filesystem/dir.h"
#include "include/filesystem/filesystem.h"
#include "include/filesystem/inode.h"
#include "include/filesystem/super_block.h"
#include "include/io/ioqueue.h"
#include "include/kernel/interrupt.h"
#include "include/thread/thread.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/memory/memory.h"

#define DEFAULT_SECS 1

/* File table */
File file_table[MAX_FILE_OPEN];

/* Retrieves an available slot in the global file table (file_table).
 * Returns the index of the slot if successful, or -1 if all slots are in use.
 */
int32_t get_free_slot_in_global(void)
{
    uint32_t fd_idx = 3; // Start from index 3, skipping stdin, stdout, stderr.
    while (fd_idx < MAX_FILE_OPEN)
    {
        if (file_table[fd_idx].fd_inode ==
            NULL)
        { // Check if the inode is NULL.
            break;
        }
        fd_idx++;
    }
    if (fd_idx == MAX_FILE_OPEN)
    { // All slots are in use.
        ccos_printk("exceed max open files\n");
        return -1;
    }
    return fd_idx;
}

/* Installs the global file descriptor index into the process or thread's local
 * file descriptor table (fd_table). Returns the local file descriptor index if
 * successful, or -1 if all local file descriptor slots are in use. */
int32_t pcb_fd_install(int32_t global_fd_idx)
{
    TaskStruct *cur = running_thread(); // Get the current running thread.
    uint8_t local_fd_idx =
        3; // Start from index 3, skipping stdin, stdout, stderr.
    while (local_fd_idx < MAX_FILES_OPEN_PER_PROC)
    {
        if (cur->fd_table[local_fd_idx] == -1)
        { // -1 means the slot is free.
            cur->fd_table[local_fd_idx] =
                global_fd_idx; // Install the global fd index.
            break;
        }
        local_fd_idx++;
    }
    if (local_fd_idx == MAX_FILES_OPEN_PER_PROC)
    { // All slots are in use.
        ccos_printk("exceed max open files_per_proc\n");
        return -1;
    }
    return local_fd_idx;
}

/* Allocates an inode and returns its inode number. */
int32_t inode_bitmap_alloc(DiskPartition *part)
{
    int32_t bit_idx =
        bitmap_scan(&part->inode_bitmap, 1); // Find an available bit.
    if (bit_idx == -1)
    {
        return -1;
    }
    bitmap_set(&part->inode_bitmap, bit_idx, 1); // Set the bit as used.
    return bit_idx;
}

/* Allocates a block and returns its block address. */
int32_t block_bitmap_alloc(DiskPartition *part)
{
    int32_t bit_idx =
        bitmap_scan(&part->block_bitmap, 1); // Find an available bit.
    if (bit_idx == -1)
    {
        return -1;
    }
    bitmap_set(&part->block_bitmap, bit_idx, 1); // Set the bit as used.
    /* Unlike inode_bitmap_alloc, this function returns the actual block
     * address, not the bitmap index. */
    return (part->sb->data_start_lba + bit_idx); // Calculate the block address.
}

/* Synchronizes the bitmap data of the given type (inode_bitmap or block_bitmap)
 * to the disk. The synchronization is performed by writing the relevant section
 * of the bitmap to the disk. */
void bitmap_sync(DiskPartition *part, uint32_t bit_idx, uint8_t btmp_type)
{
    uint32_t off_sec = bit_idx / 4096; // Offset of the sector in the bitmap.
    uint32_t off_size =
        off_sec * BLOCK_SIZE; // Offset in bytes within the bitmap.
    uint32_t sec_lba;         // Sector address to be written.
    uint8_t *
        bitmap_off; // Pointer to the specific part of the bitmap to be written.

    /* Synchronize inode_bitmap or block_bitmap based on the btmp_type argument.
     */
    switch (btmp_type)
    {
    case INODE_BITMAP:
        sec_lba = part->sb->inode_bitmap_lba + off_sec;
        bitmap_off = part->inode_bitmap.bits + off_size;
        break;

    case BLOCK_BITMAP:
        sec_lba = part->sb->block_bitmap_lba + off_sec;
        bitmap_off = part->block_bitmap.bits + off_size;
        break;
    }
    ide_write(part->my_disk, sec_lba, bitmap_off,
              1); // Write the bitmap data to disk.
}

/* Creates a new file in the given parent directory.
 * Returns the file descriptor if successful, or -1 if an error occurs during
 * creation. */
int32_t file_create(Dir *parent_dir, char *filename, uint8_t flag)
{
    void *io_buf = sys_malloc(1024); // Allocate memory for a temporary buffer.
    if (io_buf == NULL)
    {
        ccos_printk("in file_create: sys_malloc for io_buf failed\n");
        return -1;
    }

    uint8_t rollback_step =
        0; // Step counter for rolling back resources in case of failure.

    /* Allocate an inode for the new file. */
    int32_t inode_no = inode_bitmap_alloc(cur_part);
    if (inode_no == -1)
    {
        ccos_printk("in file_create: allocate inode failed\n");
        return -1;
    }

    /* Allocate memory for the inode and initialize it. */
    Inode *new_file_inode =
        (Inode *)sys_malloc(sizeof(Inode));
    if (new_file_inode == NULL)
    {
        ccos_printk("file_create: sys_malloc for inode failed\n");
        rollback_step = 1;
        goto rollback;
    }
    inode_init(inode_no, new_file_inode); // Initialize the inode.

    /* Get an available slot in the global file table. */
    int fd_idx = get_free_slot_in_global();
    if (fd_idx == -1)
    {
        ccos_printk("exceed max open files\n");
        rollback_step = 2;
        goto rollback;
    }

    /* Initialize the file table entry for the new file. */
    file_table[fd_idx].fd_inode = new_file_inode;
    file_table[fd_idx].fd_pos = 0;
    file_table[fd_idx].fd_flag = flag;
    file_table[fd_idx].fd_inode->write_deny = false;

    /* Create a directory entry for the new file. */
    DirEntry new_dir_entry;
    k_memset(&new_dir_entry, 0, sizeof(DirEntry));
    create_dir_entry(filename, inode_no, FT_REGULAR,
                     &new_dir_entry); // Create the directory entry.

    /* Synchronize the directory entry to the disk. */
    if (!sync_dir_entry(parent_dir, &new_dir_entry, io_buf))
    {
        ccos_printk("sync dir_entry to disk failed\n");
        rollback_step = 3;
        goto rollback;
    }

    k_memset(io_buf, 0, 1024);
    /* Synchronize the parent directory's inode to the disk. */
    inode_sync(cur_part, parent_dir->inode, io_buf);

    k_memset(io_buf, 0, 1024);
    /* Synchronize the new file's inode to the disk. */
    inode_sync(cur_part, new_file_inode, io_buf);

    /* Synchronize the inode bitmap to the disk. */
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    /* Add the newly created file's inode to the open_inodes list. */
    list_push(&cur_part->open_inodes, &new_file_inode->inode_tag);
    new_file_inode->i_open_cnts = 1;

    sys_free(io_buf);
    return pcb_fd_install(
        fd_idx); // Install the file descriptor in the process's fd_table.

rollback:
    /* If any step fails, roll back the allocated resources. */
    switch (rollback_step)
    {
    case 3:
        /* Clear the file table entry if synchronization fails. */
        k_memset(&file_table[fd_idx], 0, sizeof(File));
        goto FREE_SRC;
        break;
    case 2:
    FREE_SRC:
        sys_free(new_file_inode); // Free the inode memory.
        goto RESET_MAP;
        break;
    case 1:
    RESET_MAP:
        /* Roll back the inode allocation if it failed. */
        bitmap_set(&cur_part->inode_bitmap, inode_no, 0);
        break;
    }
    sys_free(io_buf);
    return -1;
}

/* Opens the file corresponding to the inode number (inode_no).
 * If successful, returns the file descriptor; otherwise, returns -1. */
int32_t file_open(uint32_t inode_no, uint8_t flag) {
    // Get an available slot in the global file table.
    int fd_idx = get_free_slot_in_global();
    if (fd_idx == -1) { // Check if no free slot is available.
        ccos_printk("exceed max open files\n");
        return -1;
    }

    // Open the inode and associate it with the file table entry.
    file_table[fd_idx].fd_inode = inode_open(cur_part, inode_no);
    file_table[fd_idx].fd_pos =
        0; // Reset the file pointer to the beginning of the file.
    file_table[fd_idx].fd_flag = flag; // Set the file access flag.

    bool *write_deny = &file_table[fd_idx].fd_inode->write_deny;

    // If the file is being opened for writing (write-only or read-write), check
    // if it is already being written by another process.
    if (flag == O_WRONLY || flag == O_RDWR) { // Writing to the file.
        // Only check for write access conflicts if the file is being written.
        Interrupt_Status old_status =
            set_intr_status(INTR_OFF); // Disable interrupts to enter a critical section.

        if (!(*write_deny)) {   // If no other process is currently writing to
                                // this file.
            *write_deny = true; // Mark the file as being written to prevent
                                // other processes from writing.
            set_intr_status(old_status); // Restore interrupts after the critical section.
        } else { // If another process is already writing to the file, return an
                 // error.
            set_intr_status(old_status);
            ccos_printk("file can't be written to now, try again later\n");
            return -1;
        }
    }

    // If the file is being opened for reading or creation, ignore write_deny
    // (default behavior).
    return pcb_fd_install(fd_idx); // Install the file descriptor in the
                                   // process's file descriptor table.
}

/* Closes the given file.
 * If the file is successfully closed, returns 0; otherwise, returns -1. */
int32_t file_close(File *file) {
    if (file == NULL) { // Check if the file is null.
        return -1;
    }

    // Release the write deny lock and close the inode.
    file->fd_inode->write_deny =
        false; // Allow other processes to write to the file.
    inode_close(
        file->fd_inode); // Close the inode and release associated resources.

    // Set the file's inode pointer to NULL, making the file structure available
    // for reuse.
    file->fd_inode = NULL;
    return 0;
}