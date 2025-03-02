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

/* File open options */
enum oflags {
   O_RDONLY,      // Read-only
   O_WRONLY,      // Write-only
   O_RDWR,        // Read and write
   O_CREAT = 4    // Create
};

/* File seek position offset */
enum whence {
   SEEK_SET = 1,  // Seek from the beginning of the file
   SEEK_CUR,      // Seek from the current position
   SEEK_END       // Seek from the end of the file
};

/* Structure to record the search path during file lookup */
typedef struct {
   char searched_path[MAX_PATH_LEN]; // Parent path during file lookup
   Dir* parent_dir;                  // Direct parent directory of the file or directory
   enum file_types file_type;        // File type found (regular file, directory, or unknown if not found)
}PathSearchRecordings;

/* File attributes structure */
struct stat {
   uint32_t st_ino;        // Inode number
   uint32_t st_size;       // File size
   enum file_types st_filetype; // File type
};

extern DiskPartition* cur_part; // Pointer to the current disk partition

/* Open or create a file, return the file descriptor if successful, otherwise
 * return -1 */
int32_t sys_open(const char *pathname, uint8_t flags);
// close file
int32_t sys_close(int32_t fd);
// write file
int32_t sys_write(int32_t fd, const void *buf, uint32_t count);
// read file
int32_t sys_read(int32_t fd, void *buf, uint32_t count);
/* Reset the offset pointer used for file read/write operations, return the new
 * offset on success, return -1 if an error occurs */
int32_t sys_lseek(int32_t fd, int32_t offset, uint8_t whence);
// unlink or del the file
int32_t sys_unlink(const char* pathname);
void filesystem_init(void);
/* Return the depth of the path, e.g., for /a/b/c, the depth is 3 */
int32_t path_depth_cnt(char *pathname);
/* Parse the top-level path name */
char *path_parse(char *pathname, char *name_store);
/* Convert the file descriptor to the global file table index */
uint32_t fd_local2global(uint32_t local_fd);
#endif
