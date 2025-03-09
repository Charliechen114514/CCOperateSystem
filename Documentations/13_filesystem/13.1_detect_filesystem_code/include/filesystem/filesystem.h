#ifndef __FS_FS_H
#define __FS_FS_H
#include "include/library/types.h"
#include "include/device/ide.h"
#include "include/filesystem/filesystem_settings.h"

/* File types */
enum file_types {
   FT_UNKNOWN,    // Unsupported file type
   FT_REGULAR,    // Regular file
   FT_DIRECTORY   // Directory
};

void filesystem_init(void);

#endif
