// Include necessary filesystem and library headers
#include "include/filesystem/dir.h"
#include "include/io/stdio-kernel.h"
#include "include/filesystem/file.h"
#include "include/filesystem/filesystem.h"
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
        ccos_printk("search_dir_entry: sys_malloc for all_blocks failed");
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
            if (!k_strcmp(p_de->filename, name)) {
                k_memcpy(dir_e, p_de, dir_entry_size);
                sys_free(buf);
                sys_free(all_blocks);
                return true;
            }
            dir_entry_idx++;
            p_de++;
        }
        block_idx++;
        p_de = (DirEntry *)buf;
        k_memset(buf, 0, SECTOR_SIZE);
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
    KERNEL_ASSERT(k_strlen(filename) <= MAX_FILE_NAME_LEN);
    k_memcpy(p_de->filename, filename, k_strlen(filename));
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
    Inode *dir_inode = parent_dir->inode;
    uint32_t dir_size[[maybe_unused]] = dir_inode->i_size;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;

    KERNEL_ASSERT(dir_size % dir_entry_size == 0);

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
                ccos_printk("alloc block bitmap for sync_dir_entry failed\n");
                return false;
            }
            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
            KERNEL_ASSERT(block_bitmap_idx != -1);
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
                    ccos_printk("alloc block bitmap for sync_dir_entry failed\n");
                    return false;
                }

                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                KERNEL_ASSERT(block_bitmap_idx != -1);
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                all_blocks[12] = block_lba;
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12],
                          all_blocks + 12, 1);
            } else {
                all_blocks[block_idx] = block_lba;
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12],
                          all_blocks + 12, 1);
            }

            k_memset(io_buf, 0, 512);
            k_memcpy(io_buf, p_de, dir_entry_size);
            ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
            dir_inode->i_size += dir_entry_size;
            return true;
        }

        ide_read(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
        uint8_t dir_entry_idx = 0;

        while (dir_entry_idx < dir_entrys_per_sec) {
            if ((dir_e + dir_entry_idx)->f_type == FT_UNKNOWN) {
                k_memcpy(dir_e + dir_entry_idx, p_de, dir_entry_size);
                ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
                dir_inode->i_size += dir_entry_size;
                return true;
            }
            dir_entry_idx++;
        }
        block_idx++;
    }
    ccos_printk("directory is full!\n");
    return false;
}