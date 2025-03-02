#include "include/filesystem/file.h"
#include "include/FormatIO/stdio-kernel.h"
#include "include/filesystem/dir.h"
#include "include/filesystem/fs.h"
#include "include/filesystem/inode.h"
#include "include/filesystem/super_block.h"
#include "include/io/ioqueue.h"
#include "include/kernel/interrupt.h"
#include "include/kernel/thread.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/memory/memory.h"

#define DEFAULT_SECS 1

/* File table */
File file_table[MAX_FILE_OPEN];

/* Retrieves an available slot in the global file table (file_table).
 * Returns the index of the slot if successful, or -1 if all slots are in use.
 */
int32_t get_free_slot_in_global(void) {
    uint32_t fd_idx = 3; // Start from index 3, skipping stdin, stdout, stderr.
    while (fd_idx < MAX_FILE_OPEN) {
        if (file_table[fd_idx].fd_inode ==
            NULL) { // Check if the inode is NULL.
            break;
        }
        fd_idx++;
    }
    if (fd_idx == MAX_FILE_OPEN) { // All slots are in use.
        printk("exceed max open files\n");
        return -1;
    }
    return fd_idx;
}

/* Installs the global file descriptor index into the process or thread's local
 * file descriptor table (fd_table). Returns the local file descriptor index if
 * successful, or -1 if all local file descriptor slots are in use. */
int32_t pcb_fd_install(int32_t global_fd_idx) {
    TaskStruct *cur = running_thread(); // Get the current running thread.
    uint8_t local_fd_idx =
        3; // Start from index 3, skipping stdin, stdout, stderr.
    while (local_fd_idx < MAX_FILES_OPEN_PER_PROC) {
        if (cur->fd_table[local_fd_idx] == -1) { // -1 means the slot is free.
            cur->fd_table[local_fd_idx] =
                global_fd_idx; // Install the global fd index.
            break;
        }
        local_fd_idx++;
    }
    if (local_fd_idx == MAX_FILES_OPEN_PER_PROC) { // All slots are in use.
        printk("exceed max open files_per_proc\n");
        return -1;
    }
    return local_fd_idx;
}

/* Allocates an inode and returns its inode number. */
int32_t inode_bitmap_alloc(DiskPartition *part) {
    int32_t bit_idx =
        bitmap_scan(&part->inode_bitmap, 1); // Find an available bit.
    if (bit_idx == -1) {
        return -1;
    }
    bitmap_set(&part->inode_bitmap, bit_idx, 1); // Set the bit as used.
    return bit_idx;
}

/* Allocates a block and returns its block address. */
int32_t block_bitmap_alloc(DiskPartition *part) {
    int32_t bit_idx =
        bitmap_scan(&part->block_bitmap, 1); // Find an available bit.
    if (bit_idx == -1) {
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
void bitmap_sync(DiskPartition *part, uint32_t bit_idx, uint8_t btmp_type) {
    uint32_t off_sec = bit_idx / 4096; // Offset of the sector in the bitmap.
    uint32_t off_size =
        off_sec * BLOCK_SIZE; // Offset in bytes within the bitmap.
    uint32_t sec_lba;         // Sector address to be written.
    uint8_t *
        bitmap_off; // Pointer to the specific part of the bitmap to be written.

    /* Synchronize inode_bitmap or block_bitmap based on the btmp_type argument.
     */
    switch (btmp_type) {
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
int32_t file_create(Dir *parent_dir, char *filename, uint8_t flag) {
    void *io_buf = sys_malloc(1024); // Allocate memory for a temporary buffer.
    if (io_buf == NULL) {
        printk("in file_create: sys_malloc for io_buf failed\n");
        return -1;
    }

    uint8_t rollback_step =
        0; // Step counter for rolling back resources in case of failure.

    /* Allocate an inode for the new file. */
    int32_t inode_no = inode_bitmap_alloc(cur_part);
    if (inode_no == -1) {
        printk("in file_create: allocate inode failed\n");
        return -1;
    }

    /* Allocate memory for the inode and initialize it. */
    struct inode *new_file_inode =
        (struct inode *)sys_malloc(sizeof(struct inode));
    if (new_file_inode == NULL) {
        printk("file_create: sys_malloc for inode failed\n");
        rollback_step = 1;
        goto rollback;
    }
    inode_init(inode_no, new_file_inode); // Initialize the inode.

    /* Get an available slot in the global file table. */
    int fd_idx = get_free_slot_in_global();
    if (fd_idx == -1) {
        printk("exceed max open files\n");
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
    memset(&new_dir_entry, 0, sizeof(DirEntry));
    create_dir_entry(filename, inode_no, FT_REGULAR,
                     &new_dir_entry); // Create the directory entry.

    /* Synchronize the directory entry to the disk. */
    if (!sync_dir_entry(parent_dir, &new_dir_entry, io_buf)) {
        printk("sync dir_entry to disk failed\n");
        rollback_step = 3;
        goto rollback;
    }

    memset(io_buf, 0, 1024);
    /* Synchronize the parent directory's inode to the disk. */
    inode_sync(cur_part, parent_dir->inode, io_buf);

    memset(io_buf, 0, 1024);
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
    switch (rollback_step) {
    case 3:
        /* Clear the file table entry if synchronization fails. */
        memset(&file_table[fd_idx], 0, sizeof(File));
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
        printk("exceed max open files\n");
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
            disable_intr(); // Disable interrupts to enter a critical section.

        if (!(*write_deny)) {   // If no other process is currently writing to
                                // this file.
            *write_deny = true; // Mark the file as being written to prevent
                                // other processes from writing.
            set_intr_state(
                old_status); // Restore interrupts after the critical section.
        } else { // If another process is already writing to the file, return an
                 // error.
            set_intr_state(old_status);
            printk("file can't be written to now, try again later\n");
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

/* Writes 'count' bytes from 'buf' to the file.
 * Returns the number of bytes written on success,
 * or -1 if an error occurs. */
int32_t file_write(File *file, const void *buf, uint32_t count) {
    // Check if the file size after writing exceeds the maximum allowed size
    // (140 blocks, 71680 bytes).
    if ((file->fd_inode->i_size + count) > (BLOCK_SIZE * 140)) {
        printk("exceed max file_size 71680 bytes, write file failed\n");
        return -1;
    }

    // Allocate memory for the IO buffer and the block address array.
    uint8_t *io_buf = sys_malloc(BLOCK_SIZE);
    if (io_buf == NULL) {
        printk("file_write: sys_malloc for io_buf failed\n");
        return -1;
    }

    uint32_t *all_blocks = (uint32_t *)sys_malloc(
        BLOCK_SIZE + 48); // Array to store all block addresses.
    if (all_blocks == NULL) {
        printk("file_write: sys_malloc for all_blocks failed\n");
        return -1;
    }

    const uint8_t *src = buf; // Pointer to the data to be written.
    uint32_t bytes_written =
        0; // Variable to track the number of bytes written.
    uint32_t size_left =
        count; // Variable to track the remaining bytes to be written.
    int32_t block_lba = -1; // Block address.
    uint32_t block_bitmap_idx =
        0;            // Block bitmap index used for bitmap synchronization.
    uint32_t sec_idx; // Sector index.
    uint32_t sec_lba; // Sector address.
    uint32_t sec_off_bytes;       // Byte offset within the sector.
    uint32_t sec_left_bytes;      // Remaining bytes in the sector.
    uint32_t chunk_size;          // Size of the data chunk to be written.
    int32_t indirect_block_table; // Address of the indirect block table.
    uint32_t block_idx;           // Block index.

    // If this is the first time writing to the file, allocate the first block.
    if (file->fd_inode->i_sectors[0] == 0) {
        block_lba = block_bitmap_alloc(cur_part);
        if (block_lba == -1) {
            printk("file_write: block_bitmap_alloc failed\n");
            return -1;
        }
        file->fd_inode->i_sectors[0] = block_lba;

        // Synchronize the block bitmap to disk after allocation.
        block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
        ASSERT(block_bitmap_idx != 0);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
    }

    // Calculate the number of blocks the file has already occupied and will
    // occupy after writing.
    uint32_t file_has_used_blocks = file->fd_inode->i_size / BLOCK_SIZE + 1;
    uint32_t file_will_use_blocks =
        (file->fd_inode->i_size + count) / BLOCK_SIZE + 1;
    ASSERT(file_will_use_blocks <= 140);

    // Calculate the number of blocks to be added.
    uint32_t add_blocks = file_will_use_blocks - file_has_used_blocks;

    // Collect all the block addresses into 'all_blocks' array.
    if (add_blocks == 0) {
        // If no new blocks need to be allocated, just use the existing blocks.
        if (file_has_used_blocks <= 12) {
            block_idx = file_has_used_blocks - 1; // Last used block.
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        } else {
            // If indirect blocks are already used, load the indirect block
            // addresses.
            ASSERT(file->fd_inode->i_sectors[12] != 0);
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12,
                     1);
        }
    } else {
        // Allocate new blocks and handle the three cases of block allocation.
        if (file_will_use_blocks <= 12) {
            // Case 1: All new data fits within the first 12 blocks.
            block_idx = file_has_used_blocks - 1;
            ASSERT(file->fd_inode->i_sectors[block_idx] != 0);
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            // Allocate new blocks and add them to 'all_blocks'.
            block_idx = file_has_used_blocks;
            while (block_idx < file_will_use_blocks) {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1) {
                    printk("file_write: block_bitmap_alloc for situation 1 "
                           "failed\n");
                    return -1;
                }
                ASSERT(file->fd_inode->i_sectors[block_idx] == 0);
                file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] =
                    block_lba;
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                block_idx++;
            }
        } else if (file_has_used_blocks <= 12 && file_will_use_blocks > 12) {
            // Case 2: Existing data fits within the first 12 blocks, but new
            // data will use an indirect block.
            block_idx = file_has_used_blocks - 1;
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            // Allocate an indirect block.
            block_lba = block_bitmap_alloc(cur_part);
            if (block_lba == -1) {
                printk(
                    "file_write: block_bitmap_alloc for situation 2 failed\n");
                return -1;
            }
            ASSERT(file->fd_inode->i_sectors[12] == 0);
            indirect_block_table = file->fd_inode->i_sectors[12] = block_lba;

            block_idx = file_has_used_blocks;
            while (block_idx < file_will_use_blocks) {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1) {
                    printk("file_write: block_bitmap_alloc for situation 2 "
                           "failed\n");
                    return -1;
                }
                if (block_idx < 12) {
                    ASSERT(file->fd_inode->i_sectors[block_idx] == 0);
                    file->fd_inode->i_sectors[block_idx] =
                        all_blocks[block_idx] = block_lba;
                } else {
                    all_blocks[block_idx] = block_lba;
                }
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                block_idx++;
            }
            ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12,
                      1);
        } else if (file_has_used_blocks > 12) {
            // Case 3: New data will occupy indirect blocks.
            ASSERT(file->fd_inode->i_sectors[12] != 0);
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12,
                     1);

            block_idx = file_has_used_blocks;
            while (block_idx < file_will_use_blocks) {
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1) {
                    printk("file_write: block_bitmap_alloc for situation 3 "
                           "failed\n");
                    return -1;
                }
                all_blocks[block_idx++] = block_lba;
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
            }
            ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12,
                      1);
        }
    }

    // Begin writing data to the collected blocks.
    bool first_write_block = true;
    file->fd_pos = file->fd_inode->i_size - 1;
    while (bytes_written < count) {
        memset(io_buf, 0, BLOCK_SIZE);
        sec_idx = file->fd_inode->i_size / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_inode->i_size % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;

        // Determine the chunk size for this write operation.
        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
        if (first_write_block) {
            ide_read(cur_part->my_disk, sec_lba, io_buf, 1);
            first_write_block = false;
        }

        memcpy(io_buf + sec_off_bytes, src, chunk_size);
        ide_write(cur_part->my_disk, sec_lba, io_buf, 1);

        src += chunk_size;
        file->fd_inode->i_size += chunk_size;
        file->fd_pos += chunk_size;
        bytes_written += chunk_size;
        size_left -= chunk_size;
    }

    inode_sync(cur_part, file->fd_inode, io_buf);

    // Free allocated memory.
    sys_free(all_blocks);
    sys_free(io_buf);

    return bytes_written;
}

/* Reads 'count' bytes from the file into 'buf' and returns the number of bytes
   read. Returns -1 if the end of the file is reached. */
int32_t file_read(File *file, void *buf, uint32_t count) {
    uint8_t *buf_dst = (uint8_t *)buf;
    uint32_t size = count, size_left = size;

    /* If the requested number of bytes exceeds the file's readable size,
       use the remaining size as the number of bytes to read. */
    if ((file->fd_pos + count) > file->fd_inode->i_size) {
        size = file->fd_inode->i_size - file->fd_pos;
        size_left = size;
        if (size == 0) { // Return -1 if the end of the file is reached
            return -1;
        }
    }

    uint8_t *io_buf =
        sys_malloc(BLOCK_SIZE); // Allocate buffer for I/O operations
    if (io_buf == NULL) {
        printk("file_read: sys_malloc for io_buf failed\n");
    }
    uint32_t *all_blocks = (uint32_t *)sys_malloc(
        BLOCK_SIZE + 48); // Allocate space to store the addresses of all blocks
                          // used by the file
    if (all_blocks == NULL) {
        printk("file_read: sys_malloc for all_blocks failed\n");
        return -1;
    }

    uint32_t block_read_start_idx =
        file->fd_pos / BLOCK_SIZE; // Start block index of the data to be read
    uint32_t block_read_end_idx =
        (file->fd_pos + size) /
        BLOCK_SIZE; // End block index of the data to be read
    uint32_t read_blocks = block_read_start_idx -
                           block_read_end_idx; // If the difference is 0, data
                                               // is within the same sector
    ASSERT(block_read_start_idx < 139 && block_read_end_idx < 139);

    int32_t indirect_block_table; // To store the address of the indirect block
                                  // table
    uint32_t block_idx;           // To store the block address to be read

    /* Start constructing the 'all_blocks' array, which stores the block
     * addresses used by the file. */
    if (read_blocks ==
        0) { // Data is within the same sector, no need to read across sectors
        ASSERT(block_read_end_idx == block_read_start_idx);
        if (block_read_end_idx <
            12) { // If the data is within the first 12 direct blocks
            block_idx = block_read_end_idx;
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        } else { // If indirect block table is used, read the table into memory
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12,
                     1);
        }
    } else { // Multiple blocks need to be read
             /* First case: Start and end blocks are within direct blocks */
        if (block_read_end_idx <
            12) { // If the data ends within the direct blocks
            block_idx = block_read_start_idx;
            while (block_idx <= block_read_end_idx) {
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
        } else if (block_read_start_idx < 12 && block_read_end_idx >= 12) {
            /* Second case: Data spans direct and indirect blocks */
            /* First, copy the direct block addresses into 'all_blocks' */
            block_idx = block_read_start_idx;
            while (block_idx < 12) {
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
            ASSERT(
                file->fd_inode->i_sectors[12] !=
                0); // Ensure that the indirect block table has been allocated

            /* Then, copy the indirect block addresses into 'all_blocks' */
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12,
                     1); // Read the indirect block table into the memory
                         // starting from the 13th block
        } else {
            /* Third case: Data is entirely within the indirect blocks */
            ASSERT(
                file->fd_inode->i_sectors[12] !=
                0); // Ensure that the indirect block table has been allocated
            indirect_block_table =
                file->fd_inode->i_sectors[12]; // Get the address of the
                                               // indirect block table
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12,
                     1); // Read the indirect block table into the memory
                         // starting from the 13th block
        }
    }

    /* The addresses of the required blocks are now stored in 'all_blocks'.
     * Start reading the data. */
    uint32_t sec_idx, sec_lba, sec_off_bytes, sec_left_bytes, chunk_size;
    uint32_t bytes_read = 0;
    while (bytes_read < size) { // Continue reading until all data is read
        sec_idx = file->fd_pos / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx]; // Get the logical block address
        sec_off_bytes =
            file->fd_pos % BLOCK_SIZE; // Get the offset within the block
        sec_left_bytes =
            BLOCK_SIZE - sec_off_bytes; // Get the remaining bytes in the block
        chunk_size = size_left < sec_left_bytes
                         ? size_left
                         : sec_left_bytes; // Determine the chunk size to read

        memset(io_buf, 0, BLOCK_SIZE); // Clear the I/O buffer
        ide_read(cur_part->my_disk, sec_lba, io_buf,
                 1); // Read the block from disk
        memcpy(buf_dst, io_buf + sec_off_bytes,
               chunk_size); // Copy the relevant data into the buffer

        buf_dst += chunk_size;      // Update the destination pointer
        file->fd_pos += chunk_size; // Update the file position
        bytes_read += chunk_size;   // Update the number of bytes read
        size_left -= chunk_size;    // Update the remaining bytes to be read
    }

    sys_free(all_blocks); // Free the block addresses array
    sys_free(io_buf);     // Free the I/O buffer
    return bytes_read;    // Return the number of bytes successfully read
}
