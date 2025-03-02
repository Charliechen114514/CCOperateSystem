// Include necessary filesystem and library headers
#include "include/filesystem/dir.h"
#include "include/FormatIO/stdio-kernel.h"
#include "include/filesystem/file.h"
#include "include/filesystem/fs.h"
#include "include/filesystem/inode.h"
#include "include/filesystem/super_block.h"
#include "include/kernel/interrupt.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/memory/memory.h"

// Root directory structure
Dir root_dir;

/**
 * Opens the root directory on the specified disk partition.
 * @param part The disk partition containing the root directory.
 */
void open_root_dir(DiskPartition *part) {
    root_dir.inode = inode_open(part, part->sb->root_inode_no);
    root_dir.dir_pos = 0;
}

/**
 * Opens a directory by its inode number on the specified partition.
 * @param part The disk partition.
 * @param inode_no The inode number of the directory.
 * @return A pointer to the opened directory structure.
 */
Dir *dir_open(DiskPartition *part, uint32_t inode_no) {
    Dir *pdir = (Dir *)sys_malloc(sizeof(Dir));
    pdir->inode = inode_open(part, inode_no);
    pdir->dir_pos = 0;
    return pdir;
}

/**
 * Searches for a directory entry with the specified name in the given
 * directory.
 * @param part The disk partition.
 * @param pdir The directory to search.
 * @param name The name of the target file or directory.
 * @param dir_e A pointer to store the found directory entry.
 * @return True if the directory entry is found, false otherwise.
 */
bool search_dir_entry(DiskPartition *part, Dir *pdir, const char *name,
                      DirEntry *dir_e) {
    uint32_t block_cnt =
        140; // Total of 12 direct blocks and 128 indirect blocks
    uint32_t *all_blocks = (uint32_t *)sys_malloc(48 + 512);
    if (all_blocks == NULL) {
        printk("search_dir_entry: sys_malloc for all_blocks failed");
        return false;
    }

    uint32_t block_idx = 0;
    while (block_idx < 12) {
        all_blocks[block_idx] = pdir->inode->i_sectors[block_idx];
        block_idx++;
    }

    if (pdir->inode->i_sectors[12] != 0) {
        ide_read(part->my_disk, pdir->inode->i_sectors[12], all_blocks + 12, 1);
    }

    uint8_t *buf = (uint8_t *)sys_malloc(SECTOR_SIZE);
    DirEntry *p_de = (DirEntry *)buf;
    uint32_t dir_entry_size = part->sb->dir_entry_size;
    uint32_t dir_entry_cnt = SECTOR_SIZE / dir_entry_size;

    while (block_idx < block_cnt) {
        if (all_blocks[block_idx] == 0) {
            block_idx++;
            continue;
        }

        ide_read(part->my_disk, all_blocks[block_idx], buf, 1);
        uint32_t dir_entry_idx = 0;

        while (dir_entry_idx < dir_entry_cnt) {
            if (!strcmp(p_de->filename, name)) {
                memcpy(dir_e, p_de, dir_entry_size);
                sys_free(buf);
                sys_free(all_blocks);
                return true;
            }
            dir_entry_idx++;
            p_de++;
        }
        block_idx++;
        p_de = (DirEntry *)buf;
        memset(buf, 0, SECTOR_SIZE);
    }
    sys_free(buf);
    sys_free(all_blocks);
    return false;
}

/**
 * Closes a directory. Note that the root directory should never be closed.
 * @param dir The directory to close.
 */
void dir_close(Dir *dir) {
    if (dir == &root_dir) {
        return;
    }
    inode_close(dir->inode);
    sys_free(dir);
}

/**
 * Initializes a directory entry in memory.
 * @param filename The name of the file or directory.
 * @param inode_no The inode number.
 * @param file_type The type of the file (directory or regular file).
 * @param p_de A pointer to the directory entry to be initialized.
 */
void create_dir_entry(char *filename, uint32_t inode_no, uint8_t file_type,
                      DirEntry *p_de) {
    ASSERT(strlen(filename) <= MAX_FILE_NAME_LEN);
    memcpy(p_de->filename, filename, strlen(filename));
    p_de->i_no = inode_no;
    p_de->f_type = file_type;
}

/**
 * Synchronizes a directory entry by writing it to disk.
 * @param parent_dir The parent directory.
 * @param p_de The directory entry to write.
 * @param io_buf A buffer used for I/O operations.
 * @return True if the operation succeeds, false otherwise.
 */
bool sync_dir_entry(Dir *parent_dir, DirEntry *p_de, void *io_buf) {
    struct inode *dir_inode = parent_dir->inode;
    uint32_t dir_size = dir_inode->i_size;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;

    ASSERT(dir_size % dir_entry_size == 0);

    uint32_t dir_entrys_per_sec = (512 / dir_entry_size);
    int32_t block_lba = -1;
    uint8_t block_idx = 0;
    uint32_t all_blocks[140] = {0};

    while (block_idx < 12) {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }

    DirEntry *dir_e = (DirEntry *)io_buf;
    int32_t block_bitmap_idx = -1;
    block_idx = 0;

    while (block_idx < 140) {
        block_bitmap_idx = -1;
        if (all_blocks[block_idx] == 0) {
            block_lba = block_bitmap_alloc(cur_part);
            if (block_lba == -1) {
                printk("alloc block bitmap for sync_dir_entry failed\n");
                return false;
            }
            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
            ASSERT(block_bitmap_idx != -1);
            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

            if (block_idx < 12) {
                dir_inode->i_sectors[block_idx] = all_blocks[block_idx] =
                    block_lba;
            } else if (block_idx == 12) {
                dir_inode->i_sectors[12] = block_lba;
                block_lba = block_bitmap_alloc(cur_part);
                if (block_lba == -1) {
                    block_bitmap_idx =
                        dir_inode->i_sectors[12] - cur_part->sb->data_start_lba;
                    bitmap_set(&cur_part->block_bitmap, block_bitmap_idx, 0);
                    dir_inode->i_sectors[12] = 0;
                    printk("alloc block bitmap for sync_dir_entry failed\n");
                    return false;
                }

                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx != -1);
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                all_blocks[12] = block_lba;
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12],
                          all_blocks + 12, 1);
            } else {
                all_blocks[block_idx] = block_lba;
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12],
                          all_blocks + 12, 1);
            }

            memset(io_buf, 0, 512);
            memcpy(io_buf, p_de, dir_entry_size);
            ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
            dir_inode->i_size += dir_entry_size;
            return true;
        }

        ide_read(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
        uint8_t dir_entry_idx = 0;

        while (dir_entry_idx < dir_entrys_per_sec) {
            if ((dir_e + dir_entry_idx)->f_type == FT_UNKNOWN) {
                memcpy(dir_e + dir_entry_idx, p_de, dir_entry_size);
                ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
                dir_inode->i_size += dir_entry_size;
                return true;
            }
            dir_entry_idx++;
        }
        block_idx++;
    }
    printk("directory is full!\n");
    return false;
}

/*
 * Deletes a directory entry in the specified partition directory.
 *
 * Parameters:
 * part      - Pointer to the partition where the directory resides.
 * pdir      - Pointer to the parent directory structure containing the entry to
 * delete. inode_no  - Inode number of the directory entry to be deleted. io_buf
 * - Buffer for temporary storage of read/write operations.
 *
 * This function attempts to remove a directory entry by:
 * 1. Collecting all block addresses of the directory.
 * 2. Reading each block to search for the directory entry.
 * 3. When the entry is found:
 *    a) It clears the entry and updates metadata, or
 *    b) Frees the block if it contains only the entry itself and is not the
 * first block.
 *
 * Return:
 * true      - Directory entry was successfully deleted.
 * false     - Directory entry was not found or deletion failed.
 */
bool delete_dir_entry(DiskPartition *part, Dir *pdir, uint32_t inode_no,
                      void *io_buf) {
    struct inode *dir_inode = pdir->inode;
    uint32_t block_idx = 0, all_blocks[140] = {0};

    /* Collect all direct and indirect block addresses */
    while (block_idx < 12) {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }
    if (dir_inode->i_sectors[12]) {
        ide_read(part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1);
    }

    uint32_t dir_entry_size = part->sb->dir_entry_size;
    uint32_t dir_entrys_per_sec = SECTOR_SIZE / dir_entry_size;
    DirEntry *dir_e = (DirEntry *)io_buf;
    DirEntry *dir_entry_found = NULL;
    uint8_t dir_entry_idx, dir_entry_cnt;
    bool is_dir_first_block = false;

    block_idx = 0;

    /* Search all blocks for the directory entry */
    while (block_idx < 140) {
        is_dir_first_block = false;
        if (all_blocks[block_idx] == 0) {
            block_idx++;
            continue;
        }

        dir_entry_idx = dir_entry_cnt = 0;
        memset(io_buf, 0, SECTOR_SIZE);
        ide_read(part->my_disk, all_blocks[block_idx], io_buf, 1);

        /* Traverse all entries in the current block */
        while (dir_entry_idx < dir_entrys_per_sec) {
            if ((dir_e + dir_entry_idx)->f_type != FT_UNKNOWN) {
                if (!strcmp((dir_e + dir_entry_idx)->filename, ".")) {
                    is_dir_first_block = true;
                } else if (strcmp((dir_e + dir_entry_idx)->filename, ".") &&
                           strcmp((dir_e + dir_entry_idx)->filename, "..")) {
                    dir_entry_cnt++;
                    if ((dir_e + dir_entry_idx)->i_no == inode_no) {
                        ASSERT(dir_entry_found == NULL);
                        dir_entry_found = dir_e + dir_entry_idx;
                    }
                }
            }
            dir_entry_idx++;
        }

        if (dir_entry_found == NULL) {
            block_idx++;
            continue;
        }

        ASSERT(dir_entry_cnt >= 1);

        if (dir_entry_cnt == 1 && !is_dir_first_block) {
            uint32_t block_bitmap_idx =
                all_blocks[block_idx] - part->sb->data_start_lba;
            bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

            if (block_idx < 12) {
                dir_inode->i_sectors[block_idx] = 0;
            } else {
                uint32_t indirect_blocks = 0;
                uint32_t indirect_block_idx = 12;
                while (indirect_block_idx < 140) {
                    if (all_blocks[indirect_block_idx] != 0) {
                        indirect_blocks++;
                    }
                }
                ASSERT(indirect_blocks >= 1);

                if (indirect_blocks > 1) {
                    all_blocks[block_idx] = 0;
                    ide_write(part->my_disk, dir_inode->i_sectors[12],
                              all_blocks + 12, 1);
                } else {
                    block_bitmap_idx =
                        dir_inode->i_sectors[12] - part->sb->data_start_lba;
                    bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
                    bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                    dir_inode->i_sectors[12] = 0;
                }
            }
        } else {
            memset(dir_entry_found, 0, dir_entry_size);
            ide_write(part->my_disk, all_blocks[block_idx], io_buf, 1);
        }

        ASSERT(dir_inode->i_size >= dir_entry_size);
        dir_inode->i_size -= dir_entry_size;
        memset(io_buf, 0, SECTOR_SIZE * 2);
        inode_sync(part, dir_inode, io_buf);

        return true;
    }

    return false;
}

/*
 * Reads a directory entry from the specified directory.
 *
 * Parameters:
 * dir - Pointer to the directory structure to read from.
 *
 * This function iterates over the blocks of the directory,
 * reading the next valid entry. It skips empty blocks and
 * invalid entries while updating the directory's read position.
 *
 * Return:
 * Pointer to a valid directory entry if successful.
 * NULL if no more entries are found or an error occurs.
 */
DirEntry *dir_read(Dir *dir) {
    DirEntry *dir_e = (DirEntry *)dir->dir_buf;
    struct inode *dir_inode = dir->inode;
    uint32_t all_blocks[140] = {0}, block_cnt = 12;
    uint32_t block_idx = 0, dir_entry_idx = 0;

    /* Collect all direct block addresses */
    while (block_idx < 12) {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }

    /* If there are indirect blocks, read them */
    if (dir_inode->i_sectors[12] != 0) {
        ide_read(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12,
                 1);
        block_cnt = 140;
    }
    block_idx = 0;

    uint32_t cur_dir_entry_pos = 0;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;
    uint32_t dir_entrys_per_sec = SECTOR_SIZE / dir_entry_size;

    /* Iterate over all blocks to find a valid directory entry */
    while (block_idx < block_cnt) {
        if (dir->dir_pos >= dir_inode->i_size) {
            return NULL;
        }

        if (all_blocks[block_idx] == 0) {
            block_idx++;
            continue;
        }

        memset(dir_e, 0, SECTOR_SIZE);
        ide_read(cur_part->my_disk, all_blocks[block_idx], dir_e, 1);
        dir_entry_idx = 0;

        /* Traverse all entries in the current block */
        while (dir_entry_idx < dir_entrys_per_sec) {
            if ((dir_e + dir_entry_idx)->f_type) {
                if (cur_dir_entry_pos < dir->dir_pos) {
                    cur_dir_entry_pos += dir_entry_size;
                    dir_entry_idx++;
                    continue;
                }

                ASSERT(cur_dir_entry_pos == dir->dir_pos);
                dir->dir_pos += dir_entry_size;
                return dir_e + dir_entry_idx;
            }
            dir_entry_idx++;
        }
        block_idx++;
    }

    return NULL;
}

/*
 * Checks if a directory is empty.
 *
 * Parameters:
 * dir - Pointer to the directory to check.
 *
 * A directory is considered empty if it contains only the
 * entries for "." and "..".
 *
 * Return:
 * true  - Directory is empty.
 * false - Directory contains other entries.
 */
bool dir_is_empty(Dir *dir) {
    struct inode *dir_inode = dir->inode;
    return (dir_inode->i_size == cur_part->sb->dir_entry_size * 2);
}

/*
 * Removes a subdirectory from its parent directory.
 *
 * Parameters:
 * parent_dir - Pointer to the parent directory structure.
 * child_dir  - Pointer to the child directory structure to be removed.
 *
 * This function ensures that the child directory is empty and
 * safely deletes it by:
 * 1. Removing its entry from the parent directory.
 * 2. Releasing all blocks and the inode associated with the child directory.
 *
 * Return:
 * 0  - Directory was successfully removed.
 * -1 - Directory removal failed (e.g., memory allocation error).
 */
int32_t dir_remove(Dir *parent_dir, Dir *child_dir) {
    struct inode *child_dir_inode = child_dir->inode;

    /* Verify that the directory has no extra blocks beyond the first one */
    int32_t block_idx = 1;
    while (block_idx < 13) {
        ASSERT(child_dir_inode->i_sectors[block_idx] == 0);
        block_idx++;
    }

    void *io_buf = sys_malloc(SECTOR_SIZE * 2);
    if (io_buf == NULL) {
        printk("dir_remove: malloc for io_buf failed\n");
        return -1;
    }

    /* Remove the directory entry in the parent directory */
    delete_dir_entry(cur_part, parent_dir, child_dir_inode->i_no, io_buf);

    /* Release the blocks and inode associated with the child directory */
    inode_release(cur_part, child_dir_inode->i_no);

    sys_free(io_buf);
    return 0;
}
