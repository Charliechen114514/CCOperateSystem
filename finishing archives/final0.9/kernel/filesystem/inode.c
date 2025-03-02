#include "include/filesystem/inode.h"
#include "include/filesystem/fs.h"
#include "include/filesystem/file.h"
#include "include/library/types.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory.h"
#include "include/kernel/interrupt.h"
#include "include/library/list.h"
#include "include/FormatIO/stdio-kernel.h"
#include "include/library/string.h"
#include "include/filesystem/super_block.h"

/* Structure to store the inode location */
struct inode_position {
   bool	 two_sec;	// Indicates if the inode spans two sectors
   uint32_t sec_lba;	// The sector number where the inode is located
   uint32_t off_size;	// The byte offset of the inode within the sector
};

/* Locate the sector and offset where the inode is located */
static void inode_locate(DiskPartition* part, uint32_t inode_no, struct inode_position* inode_pos) {
   /* The inode table is contiguous on the disk */
   ASSERT(inode_no < 4096);
   uint32_t inode_table_lba = part->sb->inode_table_lba;

   uint32_t inode_size = sizeof(struct inode);
   uint32_t off_size = inode_no * inode_size;	    // The byte offset of inode_no within the inode table
   uint32_t off_sec  = off_size / 512;		    // The sector offset of inode_no within the inode table
   uint32_t off_size_in_sec = off_size % 512;	    // The start address of the inode within the sector

   /* Check if the inode spans across two sectors */
   uint32_t left_in_sec = 512 - off_size_in_sec;
   if (left_in_sec < inode_size ) {	// If the remaining space in the sector is insufficient to hold the inode, it spans two sectors
      inode_pos->two_sec = true;
   } else {				  // Otherwise, the inode fits within a single sector
      inode_pos->two_sec = false;
   }
   inode_pos->sec_lba = inode_table_lba + off_sec;
   inode_pos->off_size = off_size_in_sec;
}

/* Synchronize the inode to the disk partition */
void inode_sync(DiskPartition* part, struct inode* inode, void* io_buf) {	 // io_buf is the buffer for disk IO
   uint8_t inode_no = inode->i_no;
   struct inode_position inode_pos;
   inode_locate(part, inode_no, &inode_pos);	       // The inode position is stored in inode_pos
   ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));
   
   /* Members inode_tag and i_open_cnts in the inode are not needed on the disk,
    * They are used in memory for tracking list positions and shared processes */
   struct inode pure_inode;
   memcpy(&pure_inode, inode, sizeof(struct inode));

   /* These three members only exist in memory, we need to clear them before synchronizing the inode to disk */
   pure_inode.i_open_cnts = 0;
   pure_inode.write_deny = false;	 // Set to false to ensure it is writable when read from disk
   pure_inode.inode_tag.prev = pure_inode.inode_tag.next = NULL;

   char* inode_buf = (char*)io_buf;
   if (inode_pos.two_sec) {	    // If the inode spans two sectors, read and write two sectors
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);	// Read two sectors since the inode was written continuously during formatting

   /* Start writing the inode into the two sectors at the appropriate positions */
      memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(struct inode));
   
   /* Write the modified data back to disk */
      ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
   } else {			    // If it's just one sector
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
      memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(struct inode));
      ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
   }
}

/* Open the inode corresponding to the inode number */
struct inode* inode_open(DiskPartition* part, uint32_t inode_no) {
   /* First, try to find the inode in the opened inode list for faster access */
   list_elem* elem = part->open_inodes.head.next;
   struct inode* inode_found;
   while (elem != &part->open_inodes.tail) {
      inode_found = elem2entry(struct inode, inode_tag, elem);
      if (inode_found->i_no == inode_no) {
	 inode_found->i_open_cnts++;
	 return inode_found;
      }
      elem = elem->next;
   }

   /* If the inode is not found in the opened list, read it from the disk and add it to the list */
   struct inode_position inode_pos;

   /* The inode location information will be stored in inode_pos, including the sector address and byte offset */
   inode_locate(part, inode_no, &inode_pos);

   /* To ensure that the inode created by sys_malloc is shared by all tasks,
    * it needs to be placed in kernel space, so temporarily set cur_pbc->pgdir to NULL */
   TaskStruct* cur = running_thread();
   uint32_t* cur_pagedir_bak = cur->pgdir;
   cur->pgdir = NULL;
   /* After these three lines of code, the following memory allocation will be in kernel space */
   inode_found = (struct inode*)sys_malloc(sizeof(struct inode));
   /* Restore pgdir */
   cur->pgdir = cur_pagedir_bak;

   char* inode_buf;
   if (inode_pos.two_sec) {	// Handle the case where the inode spans two sectors
      inode_buf = (char*)sys_malloc(1024);

   /* The inode table is written continuously by partition_format, so we can read it consecutively */
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
   } else {	// Otherwise, the inode does not span sectors, so a single sector buffer is enough
      inode_buf = (char*)sys_malloc(512);
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
   }
   memcpy(inode_found, inode_buf + inode_pos.off_size, sizeof(struct inode));

   /* Since this inode is likely to be used soon, insert it at the head of the list for quick retrieval */
   list_push(&part->open_inodes, &inode_found->inode_tag);
   inode_found->i_open_cnts = 1;

   sys_free(inode_buf);
   return inode_found;
}

/* Close the inode or decrement its open count */
void inode_close(struct inode* inode) {
   /* If no processes are using the inode anymore, remove it from the list and free the memory */
   Interrupt_Status old_status = disable_intr();
   if (--inode->i_open_cnts == 0) {
      list_remove(&inode->inode_tag);	  // Remove the inode from part->open_inodes

   /* The inode was allocated in kernel space via sys_malloc when opened,
      so free the inode's memory in kernel space */
      TaskStruct* cur = running_thread();
      uint32_t* cur_pagedir_bak = cur->pgdir;
      cur->pgdir = NULL;
      sys_free(inode);		         // Free the inode's memory
      cur->pgdir = cur_pagedir_bak;
   }
   set_intr_state(old_status);
}

/* Clear the inode from the disk partition */
void inode_delete(DiskPartition* part, uint32_t inode_no, void* io_buf) {
   ASSERT(inode_no < 4096);
   struct inode_position inode_pos;
   inode_locate(part, inode_no, &inode_pos);     // The inode location information is stored in inode_pos
   ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));
   
   char* inode_buf = (char*)io_buf;
   if (inode_pos.two_sec) {   // If the inode spans two sectors, read two sectors
      /* Read the original content from the disk */
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
      /* Clear the inode in the buffer */
      memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
      /* Write the cleared data back to disk */
      ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
   } else {    // If it does not span sectors, just read one sector
      /* Read the original content from the disk */
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
      /* Clear the inode in the buffer */
      memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
      /* Write the cleared data back to disk */
      ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
   }
}

/* Reclaim the data blocks and the inode itself */
void inode_release(DiskPartition* part, uint32_t inode_no) {
   struct inode* inode_to_del = inode_open(part, inode_no);
   ASSERT(inode_to_del->i_no == inode_no);

   /* 1 Reclaim all blocks occupied by the inode */
   uint8_t block_idx = 0, block_cnt = 12;
   uint32_t block_bitmap_idx;
   uint32_t all_blocks[140] = {0};	  // 12 direct blocks + 128 indirect blocks

   /* a First, collect the first 12 direct blocks */
   while (block_idx < 12) {
      all_blocks[block_idx] = inode_to_del->i_sectors[block_idx];
      block_idx++;
   }

   /* b If the first level indirect block table exists, read its 128 blocks into all_blocks[12~], 
      and free the space occupied by the first level indirect block table */
   if (inode_to_del->i_sectors[12] != 0) {
      ide_read(part->my_disk, inode_to_del->i_sectors[12], all_blocks + 12, 1);
      block_cnt = 140;

      /* Reclaim the space occupied by the first level indirect block table */
      block_bitmap_idx = inode_to_del->i_sectors[12] - part->sb->data_start_lba;
      ASSERT(block_bitmap_idx > 0);
      bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
      bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
   }
   
   /* c All blocks of the inode are now in all_blocks, reclaim each one */
   block_idx = 0;
   while (block_idx < block_cnt) {
      if (all_blocks[block_idx] != 0) {
	 block_bitmap_idx = 0;
	 block_bitmap_idx = all_blocks[block_idx] - part->sb->data_start_lba;
	 ASSERT(block_bitmap_idx > 0);
	 bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
	 bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
      }
      block_idx++; 
   }

   /* 2 Reclaim the inode itself */
   bitmap_set(&part->inode_bitmap, inode_no, 0);  
   bitmap_sync(cur_part, inode_no, INODE_BITMAP);

   /******     The following inode_delete is for debugging purposes   ******/
   /* This function will clear the inode in the inode_table, but it is not actually needed.
      Inode allocation is controlled by the inode bitmap, and the data on the disk does not need to be cleared.
      It can be directly overwritten. */
   void* io_buf = sys_malloc(1024);
   inode_delete(part, inode_no, io_buf);
   sys_free(io_buf);
   /***********************************************/

   inode_close(inode_to_del);
}

/* Initialize a new inode */
void inode_init(uint32_t inode_no, struct inode* new_inode) {
   new_inode->i_no = inode_no;
   new_inode->i_size = 0;
   new_inode->i_open_cnts = 0;
   new_inode->write_deny = false;

   /* Initialize the block index array i_sectors */
   uint8_t sec_idx = 0;
   while (sec_idx < 13) {
      /* i_sectors[12] is the address of the first level indirect block */
      new_inode->i_sectors[sec_idx] = 0;
      sec_idx++;
   }
}
