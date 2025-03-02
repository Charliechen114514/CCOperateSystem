#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H
#include "include/library/types.h"

#define SUPER_BLOCK_MAGIC (0x20040303)

/* Superblock structure */
typedef struct __super_block{
   uint32_t magic;                // Identifier for the file system type, used by operating systems supporting multiple file systems to recognize the type
   uint32_t sec_cnt;              // Total number of sectors in this partition
   uint32_t inode_cnt;            // Number of inodes in this partition
   uint32_t part_lba_base;        // Starting LBA address of this partition

   uint32_t block_bitmap_lba;     // Starting sector address of the block bitmap
   uint32_t block_bitmap_sects;   // Number of sectors occupied by the block bitmap

   uint32_t inode_bitmap_lba;     // Starting LBA address of the inode bitmap
   uint32_t inode_bitmap_sects;   // Number of sectors occupied by the inode bitmap

   uint32_t inode_table_lba;      // Starting LBA address of the inode table
   uint32_t inode_table_sects;    // Number of sectors occupied by the inode table

   uint32_t data_start_lba;       // First sector number of the data area
   uint32_t root_inode_no;        // Inode number of the root directory
   uint32_t dir_entry_size;       // Size of a directory entry

   uint8_t  pad[460];             // Padding to make the structure 512 bytes (one sector) in size
} __attribute__ ((packed)) SuperBlock;

#endif
