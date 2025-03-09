#ifndef __FS_INODE_H
#define __FS_INODE_H
#include "include/library/types.h"
#include "include/library/list.h"
#include "include/device/ide.h"

/* inode structure */
typedef struct {
   uint32_t i_no;    // inode number, uniquely identifies a file in the filesystem

   /* When the inode represents a file, i_size is the file size in bytes.
      When the inode represents a directory, i_size is the total size of all directory entries in the directory. */
   uint32_t i_size;

   uint32_t i_open_cnts;   // Keeps track of the number of times this file has been opened by processes or threads

   bool write_deny;        // A flag to prevent concurrent write operations to this file. If true, new write operations are denied until the current write completes

   /* i_sectors[0-11] are direct block pointers, i_sectors[12] is used for storing the pointer to the first indirect block */
   uint32_t i_sectors[13]; // Array of block pointers, with the first 11 being direct pointers to data blocks and the 12th being a pointer to an indirect block containing more block pointers

   list_elem inode_tag;    // A list element to manage the inode in a linked list, typically used for inode management in the filesystem
} Inode;

#endif
