#include "include/filesystem/filesystem.h"
#include "include/io/stdio-kernel.h"
#include "include/device/console_tty.h"
#include "include/device/keyboard.h"
#include "include/filesystem/dir.h"
#include "include/filesystem/inode.h"
#include "include/filesystem/super_block.h"
#include "include/io/ioqueue.h"
#include "include/kernel/interrupt.h"
#include "include/thread/thread.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/memory/memory.h"
#include "include/tools/math_tools.h"

/* Format the partition, which involves initializing the partition's metadata
 * and creating the file system */
static void partition_format(DiskPartition *part)
{
    /* For simplicity, assume that one block size equals one sector */
    uint32_t boot_sector_sects = 1;
    uint32_t super_block_sects = 1;
    uint32_t inode_bitmap_sects =
        ROUNDUP(MAX_FILES_PER_PART,
                BITS_PER_SECTOR); // Number of sectors for the inode bitmap.
                                  // Supports a maximum of 4096 files
    uint32_t inode_table_sects =
        ROUNDUP(((sizeof(struct inode) * MAX_FILES_PER_PART)), SECTOR_SIZE);
    uint32_t used_sects = boot_sector_sects + super_block_sects +
                          inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;

    /************** Simple calculation for the number of sectors occupied by the
     * block bitmap ***************/
    uint32_t block_bitmap_sects;
    block_bitmap_sects = ROUNDUP(free_sects, BITS_PER_SECTOR);
    /* block_bitmap_bit_len is the length of the bitmap in bits, which also
     * represents the number of available blocks */
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = ROUNDUP(block_bitmap_bit_len, BITS_PER_SECTOR);
    /*********************************************************/

    /* Initialize the superblock */
    SuperBlock sb;
    sb.magic = SUPER_BLOCK_MAGIC;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba =
        sb.part_lba_base +
        2; // Block 0 is the boot sector, block 1 is the superblock
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no = 0;
    sb.dir_entry_size = sizeof(DirEntry);

    printk("%s info:\n", part->name);
    printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   "
           "inode_cnt:0x%x\n   block_bitmap_lba:0x%x\n   "
           "block_bitmap_sectors:0x%x\n   inode_bitmap_lba:0x%x\n   "
           "inode_bitmap_sectors:0x%x\n   inode_table_lba:0x%x\n   "
           "inode_table_sectors:0x%x\n   data_start_lba:0x%x\n",
           sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt,
           sb.block_bitmap_lba, sb.block_bitmap_sects, sb.inode_bitmap_lba,
           sb.inode_bitmap_sects, sb.inode_table_lba, sb.inode_table_sects,
           sb.data_start_lba);

    Disk *hd = part->my_disk;
    /*******************************
     * 1. Write the superblock to the partition's second sector *
     ******************************/
    ide_write(hd, part->start_lba + 1, &sb, 1);
    printk("   super_block_lba:0x%x\n", part->start_lba + 1);

    /* Find the largest metadata element and use its size for the storage buffer
     */
    uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects
                             ? sb.block_bitmap_sects
                             : sb.inode_bitmap_sects);
    buf_size =
        (buf_size >= sb.inode_table_sects ? buf_size : sb.inode_table_sects) *
        SECTOR_SIZE;
    uint8_t *buf =
        (uint8_t *)sys_malloc(buf_size); // Allocate memory, which is cleared by
                                         // the memory management system

    /**************************************
     * 2. Initialize the block bitmap and write it to sb.block_bitmap_lba *
     *************************************/
    /* Initialize the block bitmap */
    buf[0] |= 0x01; // Reserve the first block for the root directory, mark it
                    // in the bitmap
    uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
    uint8_t block_bitmap_last_bit = block_bitmap_bit_len % 8;
    uint32_t last_size =
        SECTOR_SIZE - (block_bitmap_last_byte %
                       SECTOR_SIZE); // last_size is the remaining part in the
                                     // last sector of the bitmap

    /* 1. Set all the bits from the last byte in the bitmap to 1, marking them
     * as occupied */
    k_memset(&buf[block_bitmap_last_byte], 0xff, last_size);

    /* 2. Reset the valid bits in the last byte */
    uint8_t bit_idx = 0;
    while (bit_idx <= block_bitmap_last_bit)
    {
        buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
    }
    ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

    /***************************************
     * 3. Initialize the inode bitmap and write it to sb.inode_bitmap_lba *
     ***************************************/
    /* Clear the buffer */
    k_memset(buf, 0, buf_size);
    buf[0] |= 0x1; // Reserve the first inode for the root directory
    /* Since there are 4096 inodes in the inode table, the inode bitmap fits
     * exactly into one sector */
    ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

    /***************************************
     * 4. Initialize the inode array and write it to sb.inode_table_lba *
     ***************************************/
    /* Prepare the first inode in the inode table, which corresponds to the root
     * directory */
    k_memset(buf, 0, buf_size); // Clear the buffer
    struct inode *i = (struct inode *)buf;
    i->i_size = sb.dir_entry_size * 2; // For "." and ".."
    i->i_no = 0;                       // The root directory occupies the first inode
    i->i_sectors[0] =
        sb.data_start_lba; // Initialize the first sector for the root directory
    ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

    /***************************************
     * 5. Initialize the root directory and write it to sb.data_start_lba *
     ***************************************/
    /* Write the two directory entries for "." and ".." */
    k_memset(buf, 0, buf_size);
    DirEntry *p_de = (DirEntry *)buf;

    /* Initialize the current directory entry "." */
    k_memcpy(p_de->filename, ".", 1);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;

    /* Initialize the parent directory entry ".." */
    k_memcpy(p_de->filename, "..", 2);
    p_de->i_no = 0; // The parent directory of the root is still the root itself
    p_de->f_type = FT_DIRECTORY;

    /* Write the root directory data */
    ide_write(hd, sb.data_start_lba, buf, 1);

    printk("   root_dir_lba:0x%x\n", sb.data_start_lba);
    printk("%s format done\n", part->name);
    sys_free(buf);
}

/* Searches for the file system on the disk. If none is found, it formats the
 * partition and creates the file system */
void filesystem_init()
{
    uint8_t channel_no = 0, dev_no, part_idx = 0;

    /* sb_buf is used to store the super block read from the hard disk */
    SuperBlock *sb_buf = (SuperBlock *)sys_malloc(SECTOR_SIZE);

    if (!sb_buf)
    {
        KERNEL_PANIC_SPIN("alloc memory failed!");
    }
    verbose_printk("searching filesystem......\n");
    while (channel_no < channel_cnt)
    {
        dev_no = 0;
        while (dev_no < 2)
        {
            if (dev_no == 0)
            { // Skip raw disk hd60M.img
                dev_no++;
                continue;
            }
            Disk *hd = &channels[channel_no].devices[dev_no];
            DiskPartition *part = hd->prim_parts;
            while (part_idx < 12)
            { // 4 primary partitions + 8 logical partitions
                if (part_idx == 4)
                { // Start processing logical partitions
                    part = hd->logic_parts;
                }

                /* The channels array is a global variable, with a default value
                   of 0. Disk is a nested structure, and partition is a nested
                   structure within disk. Therefore, members in partition are
                   also defaulted to 0. We now process existing partitions. */
                if (part->sec_cnt != 0)
                { // If the partition exists
                    k_memset(sb_buf, 0, SECTOR_SIZE);

                    /* Read the super block of the partition to check if the
                     * magic number is correct and determine if the file system
                     * exists */
                    ide_read(hd, part->start_lba + 1, sb_buf, 1);

                    /* Only support our own file system. If the disk already has
                     * a file system, don't format it */
                    if (sb_buf->magic == SUPER_BLOCK_MAGIC)
                    {
                        verbose_printk("%s has filesystem\n", part->name);
                    }
                    else
                    { // Unsupported file systems are treated as having
                      // no file system
                        verbose_printk("formatting %s`s partition %s......\n", hd->name,
                               part->name);
                        partition_format(part);
                    }
                }
                part_idx++;
                part++; // Next partition
            }
            dev_no++; // Next disk
        }
        channel_no++; // Next channel
    }
    sys_free(sb_buf);
}