#include "include/filesystem/inode.h"
#include "include/filesystem/filesystem.h"
#include "include/library/types.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory.h"
#include "include/kernel/interrupt.h"
#include "include/library/list.h"
#include "include/io/stdio-kernel.h"
#include "include/library/string.h"
#include "include/filesystem/super_block.h"

/* Structure to store the inode location */
typedef struct  {
   bool	 two_sec;	// Indicates if the inode spans two sectors
   uint32_t sec_lba;	// The sector number where the inode is located
   uint32_t off_size;	// The byte offset of the inode within the sector
}InodePosition;

/* Locate the sector and offset where the inode is located */
static void inode_locate(DiskPartition* part, uint32_t inode_no, InodePosition* inode_pos) {
    /* The inode table is contiguous on the disk */
    KERNEL_ASSERT(inode_no < 4096);
    uint32_t inode_table_lba = part->sb->inode_table_lba;
 
    uint32_t inode_size = sizeof(Inode);
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
 void inode_sync(DiskPartition* part, Inode* inode, void* io_buf) {	 // io_buf is the buffer for disk IO
    uint8_t inode_no = inode->i_no;
    InodePosition inode_pos;
    inode_locate(part, inode_no, &inode_pos);	       // The inode position is stored in inode_pos
    KERNEL_ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));
    
    /* Members inode_tag and i_open_cnts in the inode are not needed on the disk,
     * They are used in memory for tracking list positions and shared processes */
    Inode pure_inode;
    k_memcpy(&pure_inode, inode, sizeof(Inode));
 
    /* These three members only exist in memory, we need to clear them before synchronizing the inode to disk */
    pure_inode.i_open_cnts = 0;
    pure_inode.write_deny = false;	 // Set to false to ensure it is writable when read from disk
    pure_inode.inode_tag.prev = pure_inode.inode_tag.next = NULL;
 
    char* inode_buf = (char*)io_buf;
    if (inode_pos.two_sec) {	    // If the inode spans two sectors, read and write two sectors
       ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);	// Read two sectors since the inode was written continuously during formatting
 
    /* Start writing the inode into the two sectors at the appropriate positions */
       k_memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(Inode));
    
    /* Write the modified data back to disk */
       ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    } else {			    // If it's just one sector
       ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
       k_memcpy((inode_buf + inode_pos.off_size), &pure_inode, sizeof(Inode));
       ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
 }

/* Open the inode corresponding to the inode number */
Inode* inode_open(DiskPartition* part, uint32_t inode_no) {
    /* First, try to find the inode in the opened inode list for faster access */
    list_elem* elem = part->open_inodes.head.next;
    Inode* inode_found;
    while (elem != &part->open_inodes.tail) {
       inode_found = elem2entry(Inode, inode_tag, elem);
       if (inode_found->i_no == inode_no) {
      inode_found->i_open_cnts++;
      return inode_found;
       }
       elem = elem->next;
    }
 
    /* If the inode is not found in the opened list, read it from the disk and add it to the list */
    InodePosition inode_pos;
 
    /* The inode location information will be stored in inode_pos, including the sector address and byte offset */
    inode_locate(part, inode_no, &inode_pos);
 
    /* To ensure that the inode created by sys_malloc is shared by all tasks,
     * it needs to be placed in kernel space, so temporarily set cur_pbc->pg_dir to NULL */
    TaskStruct* cur = running_thread();
    uint32_t* cur_pagedir_bak = cur->pg_dir;
    cur->pg_dir = NULL;
    /* After these three lines of code, the following memory allocation will be in kernel space */
    inode_found = (Inode*)sys_malloc(sizeof(Inode));
    /* Restore pg_dir */
    cur->pg_dir = cur_pagedir_bak;
 
    char* inode_buf;
    if (inode_pos.two_sec) {	// Handle the case where the inode spans two sectors
       inode_buf = (char*)sys_malloc(1024);
 
    /* The inode table is written continuously by partition_format, so we can read it consecutively */
       ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    } else {	// Otherwise, the inode does not span sectors, so a single sector buffer is enough
       inode_buf = (char*)sys_malloc(512);
       ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
    k_memcpy(inode_found, inode_buf + inode_pos.off_size, sizeof(Inode));
 
    /* Since this inode is likely to be used soon, insert it at the head of the list for quick retrieval */
    list_push(&part->open_inodes, &inode_found->inode_tag);
    inode_found->i_open_cnts = 1;
 
    sys_free(inode_buf);
    return inode_found;
 }
 
 /* Close the inode or decrement its open count */
 void inode_close(Inode* inode) {
    /* If no processes are using the inode anymore, remove it from the list and free the memory */
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    if (--inode->i_open_cnts == 0) {
       list_remove(&inode->inode_tag);	  // Remove the inode from part->open_inodes
 
    /* The inode was allocated in kernel space via sys_malloc when opened,
       so free the inode's memory in kernel space */
       TaskStruct* cur = running_thread();
       uint32_t* cur_pagedir_bak = cur->pg_dir;
       cur->pg_dir = NULL;
       sys_free(inode);		         // Free the inode's memory
       cur->pg_dir = cur_pagedir_bak;
    }
    set_intr_status(old_status);
 }

 
 /* Initialize a new inode */
 void inode_init(uint32_t inode_no, Inode* new_inode) {
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
 
