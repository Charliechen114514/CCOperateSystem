#ifndef __FS_DIR_H
#define __FS_DIR_H
#include "include/device/ide.h"
#include "include/filesystem/fs.h"
#include "include/filesystem/inode.h"
#include "include/library/types.h"

#define MAX_FILE_NAME_LEN (16)     // Maximum length of a file name
#define DIRENT_LEN_BUFFER_SZ (512) // Directory entry buffer size

/* Directory structure */
typedef struct __dir{
    struct inode *inode; // Pointer to the inode of the directory
    uint32_t dir_pos;    // Offset within the directory
    uint8_t dir_buf[DIRENT_LEN_BUFFER_SZ]; // Buffer to hold directory data
}Dir;

/* Directory entry structure */
typedef struct __dirEntry{
    char filename[MAX_FILE_NAME_LEN]; // Name of the file or directory
    uint32_t i_no;                    // Inode number of the file or directory
    enum file_types f_type; // File type (e.g., regular file, directory, etc.)
}DirEntry;

extern Dir root_dir; // Root directory
void open_root_dir(
    DiskPartition *part); // Function to open the root directory

Dir *dir_open(DiskPartition *part,
                     uint32_t inode_no); // Open a directory by inode number
void dir_close(Dir *dir);         // Close a directory
bool search_dir_entry(
    DiskPartition *part, Dir *pdir, const char *name,
    DirEntry *dir_e); // Search for a directory entry by name
void create_dir_entry(char *filename, uint32_t inode_no, uint8_t file_type,
                      DirEntry *p_de); // Create a new directory entry
bool sync_dir_entry(Dir *parent_dir, DirEntry *p_de,
                    void *io_buf); // Synchronize directory entry with disk
bool delete_dir_entry(DiskPartition *part, Dir *pdir,
                      uint32_t inode_no,
                      void *io_buf);         // Delete a directory entry
DirEntry *dir_read(Dir *dir); // Read a directory entry
bool dir_is_empty(Dir *dir);          // Check if a directory is empty
int32_t dir_remove(Dir *parent_dir,
                   Dir *child_dir); // Remove a directory entry
#endif
