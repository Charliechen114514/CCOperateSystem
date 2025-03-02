#ifndef __FS_FS_H
#define __FS_FS_H
#include "include/library/types.h"
#include "include/device/ide.h"
#include "include/filesystem/filesystem_settings.h"


typedef struct __dir Dir;
typedef struct __dirEntry DirEntry;

/* File types */
enum file_types {
   FT_UNKNOWN,    // Unsupported file type
   FT_REGULAR,    // Regular file
   FT_DIRECTORY   // Directory
};

extern DiskPartition* cur_part; // Pointer to the current disk partition

void filesystem_init(void);

#endif
