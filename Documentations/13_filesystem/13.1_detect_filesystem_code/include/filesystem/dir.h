#ifndef __FS_DIR_H
#define __FS_DIR_H
#include "include/device/ide.h"
#include "include/filesystem/filesystem.h"
#include "include/filesystem/inode.h"
#include "include/library/types.h"

#define MAX_FILE_NAME_LEN (16)     // Maximum length of a file name
#define DIRENT_LEN_BUFFER_SZ (512) // Directory entry buffer size

/* Directory structure */
typedef struct __dir{
    Inode *inode; // Pointer to the inode of the directory
    uint32_t dir_pos;    // Offset within the directory
    uint8_t dir_buf[DIRENT_LEN_BUFFER_SZ]; // Buffer to hold directory data
}Dir;

/* Directory entry structure */
typedef struct __dirEntry{
    char filename[MAX_FILE_NAME_LEN]; // Name of the file or directory
    uint32_t i_no;                    // Inode number of the file or directory
    enum file_types f_type; // File type (e.g., regular file, directory, etc.)
}DirEntry;

#endif
