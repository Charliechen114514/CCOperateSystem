#ifndef __FS_FS_H
#define __FS_FS_H
#include "include/library/types.h"
#include "include/device/ide.h"
#include "include/filesystem/filesystem_settings.h"

typedef struct __dir Dir;
typedef struct __dirEntry DirEntry;

/* File types */
enum file_types
{
   FT_UNKNOWN,  // Unsupported file type
   FT_REGULAR,  // Regular file
   FT_DIRECTORY // Directory
};

/* File open options */
enum oflags
{
   O_RDONLY,   // Read-only
   O_WRONLY,   // Write-only
   O_RDWR,     // Read and write
   O_CREAT = 4 // Create
};

/* File seek position offset */
enum whence
{
   SEEK_SET = 1, // Seek from the beginning of the file
   SEEK_CUR,     // Seek from the current position
   SEEK_END      // Seek from the end of the file
};

/* Structure to record the search path during file lookup */
typedef struct
{
   char searched_path[MAX_PATH_LEN]; // Parent path during file lookup
   Dir *parent_dir;                  // Direct parent directory of the file or directory
   enum file_types file_type;        // File type found (regular file, directory, or unknown if not found)
} PathSearchRecordings;

/* File attributes structure */
typedef struct
{
   uint32_t st_ino;             // Inode number
   uint32_t st_size;            // File size
   enum file_types st_filetype; // File type
}Stat;

extern DiskPartition *cur_part; // Pointer to the current disk partition

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
int32_t sys_unlink(const char *pathname);
/* Create a directory at pathname, return 0 on success, return -1 on failure */
int32_t sys_mkdir(const char *pathname);
/* Opens the directory specified by name, returns a directory pointer on
   success, returns NULL on failure */
Dir *sys_opendir(const char *name);
/* Closes the directory dir, returns 0 on success, returns -1 on failure */

/* Reads one directory entry from the directory dir, returns the directory entry
   address on success, returns NULL at the end of the directory or on error */
DirEntry *sys_readdir(Dir *dir);
// close a dirent
int32_t sys_closedir(Dir *dir);

/* Resets the directory pointer dir_pos to 0 */
void sys_rewinddir(Dir *dir);
/* Deletes an empty directory, returns 0 on success, returns -1 on failure */
int32_t sys_rmdir(const char *pathname);
/* Writes the absolute path of the current working directory into the buffer
   buf, size is the size of buf. If buf is NULL, the operating system allocates
   memory for the working path and returns the address. Returns NULL on failure
 */
char *sys_getcwd(char *buf, uint32_t size);
/* Changes the current working directory to the absolute path specified by path.
   Returns 0 on success, -1 on failure */
int32_t sys_chdir(const char *path);
// init filesystem
void filesystem_init(void);
/* Return the depth of the path, e.g., for /a/b/c, the depth is 3 */
int32_t path_depth_cnt(char *pathname);
/* Parse the top-level path name */
char *path_parse(char *pathname, char *name_store);
/* Convert the file descriptor to the global file table index */
uint32_t fd_local2global(uint32_t local_fd);
/* Fills the buf with the file structure information. Returns 0 on success, -1
 * on failure */
int32_t sys_stat(const char *path, Stat *buf);
/* Displays the system's supported internal commands */
void sys_help(void);
#endif
