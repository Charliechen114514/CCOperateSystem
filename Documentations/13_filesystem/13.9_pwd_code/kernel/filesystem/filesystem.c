#include "include/filesystem/filesystem.h"
#include "include/io/stdio-kernel.h"
#include "include/device/console_tty.h"
#include "include/device/keyboard.h"
#include "include/filesystem/dir.h"
#include "include/filesystem/inode.h"
#include "include/filesystem/super_block.h"
#include "include/io/ioqueue.h"
#include "include/kernel/interrupt.h"
#include "include/thread/thread.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/memory/memory.h"
#include "include/tools/math_tools.h"
#include "include/filesystem/file.h"

DiskPartition *cur_part; // The partition currently being operated on by default

/* Finds the partition named 'part_name' in the partition list and assigns its
 * pointer to 'cur_part' */
static bool mount_partition(list_elem *pelem, int arg)
{
    // Convert the argument to a partition name string
    char *part_name = (char *)arg;

    // Retrieve the DiskPartition structure from the list element
    DiskPartition *part = elem2entry(DiskPartition, part_tag, pelem);

    // If the partition name matches, mount this partition
    if (!k_strcmp(part->name, part_name))
    {
        cur_part = part;              // Set the current partition to the found partition
        Disk *hd = cur_part->my_disk; // Get the disk that contains this partition

        // Allocate a buffer to temporarily store the superblock read from disk
        SuperBlock *sb_buf = (SuperBlock *)sys_malloc(SECTOR_SIZE);

        // Allocate memory for the superblock of the current partition
        cur_part->sb = (SuperBlock *)sys_malloc(sizeof(SuperBlock));
        if (!cur_part->sb)
        {
            KERNEL_PANIC_SPIN("alloc memory failed!"); // Kernel panic if allocation fails
        }

        // Read the superblock from disk into the buffer
        k_memset(sb_buf, 0, SECTOR_SIZE);
        ide_read(hd, cur_part->start_lba + 1, sb_buf, 1);

        // Copy the superblock information from the buffer to the partition's superblock
        k_memcpy(cur_part->sb, sb_buf, sizeof(SuperBlock));

        /********** Load the block bitmap from disk into memory **********/
        // Allocate memory for the block bitmap
        cur_part->block_bitmap.bits = (uint8_t *)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);
        if (cur_part->block_bitmap.bits == NULL)
        {
            KERNEL_PANIC_SPIN("alloc memory failed!"); // Kernel panic if allocation fails
        }
        // Set the length of the block bitmap in bytes
        cur_part->block_bitmap.btmp_bytes_len = sb_buf->block_bitmap_sects * SECTOR_SIZE;

        // Read the block bitmap from disk into memory
        ide_read(hd, sb_buf->block_bitmap_lba, cur_part->block_bitmap.bits, sb_buf->block_bitmap_sects);
        /****************************************************************/

        /********** Load the inode bitmap from disk into memory **********/
        // Allocate memory for the inode bitmap
        cur_part->inode_bitmap.bits = (uint8_t *)sys_malloc(sb_buf->inode_bitmap_sects * SECTOR_SIZE);
        if (cur_part->inode_bitmap.bits == NULL)
        {
            KERNEL_PANIC_SPIN("alloc memory failed!"); // Kernel panic if allocation fails
        }
        // Set the length of the inode bitmap in bytes
        cur_part->inode_bitmap.btmp_bytes_len = sb_buf->inode_bitmap_sects * SECTOR_SIZE;

        // Read the inode bitmap from disk into memory
        ide_read(hd, sb_buf->inode_bitmap_lba, cur_part->inode_bitmap.bits, sb_buf->inode_bitmap_sects);
        /****************************************************************/

        // Initialize the list of open inodes in this partition
        list_init(&cur_part->open_inodes);

        verbose_printk("disk mount %s done!\n", part->name); // Print a message indicating successful mounting

        // Return true to stop further traversal, as the partition has been found and mounted
        return true;
    }
    return false; // Continue traversing the list if the partition was not found
}

/* Format the partition, which involves initializing the partition's metadata
 * and creating the file system */
static void partition_format(DiskPartition *part)
{
    /* For simplicity, assume that one block size equals one sector */
    uint32_t boot_sector_sects = 1;
    uint32_t super_block_sects = 1;
    uint32_t inode_bitmap_sects =
        ROUNDUP(MAX_FILES_PER_PART,
                BITS_PER_SECTOR); // Number of sectors for the inode bitmap.
                                  // Supports a maximum of 4096 files
    uint32_t inode_table_sects =
        ROUNDUP(((sizeof(Inode) * MAX_FILES_PER_PART)), SECTOR_SIZE);
    uint32_t used_sects = boot_sector_sects + super_block_sects +
                          inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;

    /************** Simple calculation for the number of sectors occupied by the
     * block bitmap ***************/
    uint32_t block_bitmap_sects;
    block_bitmap_sects = ROUNDUP(free_sects, BITS_PER_SECTOR);
    /* block_bitmap_bit_len is the length of the bitmap in bits, which also
     * represents the number of available blocks */
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = ROUNDUP(block_bitmap_bit_len, BITS_PER_SECTOR);
    /*********************************************************/

    /* Initialize the superblock */
    SuperBlock sb;
    sb.magic = SUPER_BLOCK_MAGIC;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba =
        sb.part_lba_base +
        2; // Block 0 is the boot sector, block 1 is the superblock
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no = 0;
    sb.dir_entry_size = sizeof(DirEntry);

    ccos_printk("%s info:\n", part->name);
    ccos_printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   "
                "inode_cnt:0x%x\n   block_bitmap_lba:0x%x\n   "
                "block_bitmap_sectors:0x%x\n   inode_bitmap_lba:0x%x\n   "
                "inode_bitmap_sectors:0x%x\n   inode_table_lba:0x%x\n   "
                "inode_table_sectors:0x%x\n   data_start_lba:0x%x\n",
                sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt,
                sb.block_bitmap_lba, sb.block_bitmap_sects, sb.inode_bitmap_lba,
                sb.inode_bitmap_sects, sb.inode_table_lba, sb.inode_table_sects,
                sb.data_start_lba);

    Disk *hd = part->my_disk;
    /*******************************
     * 1. Write the superblock to the partition's second sector *
     ******************************/
    ide_write(hd, part->start_lba + 1, &sb, 1);
    ccos_printk("   super_block_lba:0x%x\n", part->start_lba + 1);

    /* Find the largest metadata element and use its size for the storage buffer
     */
    uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects
                             ? sb.block_bitmap_sects
                             : sb.inode_bitmap_sects);
    buf_size =
        (buf_size >= sb.inode_table_sects ? buf_size : sb.inode_table_sects) *
        SECTOR_SIZE;
    uint8_t *buf =
        (uint8_t *)sys_malloc(buf_size); // Allocate memory, which is cleared by
                                         // the memory management system

    /**************************************
     * 2. Initialize the block bitmap and write it to sb.block_bitmap_lba *
     *************************************/
    /* Initialize the block bitmap */
    buf[0] |= 0x01; // Reserve the first block for the root directory, mark it
                    // in the bitmap
    uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
    uint8_t block_bitmap_last_bit = block_bitmap_bit_len % 8;
    uint32_t last_size =
        SECTOR_SIZE - (block_bitmap_last_byte %
                       SECTOR_SIZE); // last_size is the remaining part in the
                                     // last sector of the bitmap

    /* 1. Set all the bits from the last byte in the bitmap to 1, marking them
     * as occupied */
    k_memset(&buf[block_bitmap_last_byte], 0xff, last_size);

    /* 2. Reset the valid bits in the last byte */
    uint8_t bit_idx = 0;
    while (bit_idx <= block_bitmap_last_bit)
    {
        buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
    }
    ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

    /***************************************
     * 3. Initialize the inode bitmap and write it to sb.inode_bitmap_lba *
     ***************************************/
    /* Clear the buffer */
    k_memset(buf, 0, buf_size);
    buf[0] |= 0x1; // Reserve the first inode for the root directory
    /* Since there are 4096 inodes in the inode table, the inode bitmap fits
     * exactly into one sector */
    ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

    /***************************************
     * 4. Initialize the inode array and write it to sb.inode_table_lba *
     ***************************************/
    /* Prepare the first inode in the inode table, which corresponds to the root
     * directory */
    k_memset(buf, 0, buf_size); // Clear the buffer
    Inode *i = (Inode *)buf;
    i->i_size = sb.dir_entry_size * 2; // For "." and ".."
    i->i_no = 0;                       // The root directory occupies the first inode
    i->i_sectors[0] =
        sb.data_start_lba; // Initialize the first sector for the root directory
    ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

    /***************************************
     * 5. Initialize the root directory and write it to sb.data_start_lba *
     ***************************************/
    /* Write the two directory entries for "." and ".." */
    k_memset(buf, 0, buf_size);
    DirEntry *p_de = (DirEntry *)buf;

    /* Initialize the current directory entry "." */
    k_memcpy(p_de->filename, ".", 1);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;

    /* Initialize the parent directory entry ".." */
    k_memcpy(p_de->filename, "..", 2);
    p_de->i_no = 0; // The parent directory of the root is still the root itself
    p_de->f_type = FT_DIRECTORY;

    /* Write the root directory data */
    ide_write(hd, sb.data_start_lba, buf, 1);

    ccos_printk("   root_dir_lba:0x%x\n", sb.data_start_lba);
    ccos_printk("%s format done\n", part->name);
    sys_free(buf);
}

/* Parse the top-level path name */
char *path_parse(char *pathname, char *name_store)
{
    if (pathname[0] == '/')
    { // No need to parse the root directory separately
        /* Skip consecutive '/' characters in the path, e.g., "///a/b" */
        while (*(++pathname) == '/')
            ;
    }

    /* Start general path parsing */
    while (*pathname != '/' && *pathname != 0)
    {
        *name_store++ = *pathname++;
    }

    if (pathname[0] == 0)
    { // If the path string is empty, return NULL
        return NULL;
    }
    return pathname;
}

/* Return the depth of the path, e.g., for /a/b/c, the depth is 3 */
int32_t path_depth_cnt(char *pathname)
{
    KERNEL_ASSERT(pathname);
    char *p = pathname;
    char name[MAX_FILE_NAME_LEN]; // Used as the parameter for path_parse to
                                  // parse the path
    uint32_t depth = 0;

    /* Parse the path and split it into individual names */
    p = path_parse(p, name);
    while (name[0])
    {
        depth++;
        k_memset(name, 0, MAX_FILE_NAME_LEN);
        if (p)
        { // If p is not NULL, continue analyzing the path
            p = path_parse(p, name);
        }
    }
    return depth;
}

/* Search for the file by pathname, return its inode number if found, otherwise
 * return -1 */
static int search_file(const char *pathname,
                       PathSearchRecordings *searched_record)
{
    /* If the root directory is being searched, return the known root directory
     * information */
    if (!k_strcmp(pathname, "/") || !k_strcmp(pathname, "/.") ||
        !k_strcmp(pathname, "/.."))
    {
        searched_record->parent_dir = &root_dir;
        searched_record->file_type = FT_DIRECTORY;
        searched_record->searched_path[0] = 0; // Clear the search path
        return 0;
    }

    uint32_t path_len [[maybe_unused]] = k_strlen(pathname);
    /* Ensure pathname is in the format /x and has a valid length */
    KERNEL_ASSERT(pathname[0] == '/' && path_len > 1 && path_len < MAX_PATH_LEN);
    char *sub_path = (char *)pathname;
    Dir *parent_dir = &root_dir;
    DirEntry dir_e;

    /* Record the parsed path names, e.g., for "/a/b/c", the array name will
     * hold "a", "b", "c" */
    char name[MAX_FILE_NAME_LEN] = {0};

    searched_record->parent_dir = parent_dir;
    searched_record->file_type = FT_UNKNOWN;
    uint32_t parent_inode_no = 0; // Parent directory's inode number

    sub_path = path_parse(sub_path, name);
    while (name[0])
    { // If the first character is the end of the string, stop
        // the loop
        /* Record the paths searched, but ensure the searched path does not
         * exceed 512 bytes */
        KERNEL_ASSERT(k_strlen(searched_record->searched_path) < 512);

        /* Record the existing parent directory */
        k_strcat(searched_record->searched_path, "/");
        k_strcat(searched_record->searched_path, name);

        /* Search for the file in the specified directory */
        if (search_dir_entry(cur_part, parent_dir, name, &dir_e))
        {
            k_memset(name, 0, MAX_FILE_NAME_LEN);
            /* If sub_path is not NULL, continue splitting the path */
            if (sub_path)
            {
                sub_path = path_parse(sub_path, name);
            }

            if (FT_DIRECTORY == dir_e.f_type)
            { // If the item is a directory
                parent_inode_no = parent_dir->inode->i_no;
                dir_close(parent_dir);
                parent_dir =
                    dir_open(cur_part, dir_e.i_no); // Update parent directory
                searched_record->parent_dir = parent_dir;
                continue;
            }
            else if (FT_REGULAR ==
                     dir_e.f_type)
            { // If the item is a regular file
                searched_record->file_type = FT_REGULAR;
                return dir_e.i_no;
            }
        }
        else
        { // If not found, return -1
            /* If the directory entry is not found, leave parent_dir open,
             * since creating a new file may require adding it to parent_dir */
            return -1;
        }
    }

    /* If we've reached this point, the path has been fully traversed and only a
     * directory with the same name exists */
    dir_close(searched_record->parent_dir);

    /* Save the direct parent directory of the found directory */
    searched_record->parent_dir = dir_open(cur_part, parent_inode_no);
    searched_record->file_type = FT_DIRECTORY;
    return dir_e.i_no;
}

/* Open or create a file, return the file descriptor if successful, otherwise
 * return -1 */
int32_t sys_open(const char *pathname, uint8_t flags)
{
    /* Directories should be opened with dir_open, here we only open files */
    if (pathname[k_strlen(pathname) - 1] == '/')
    {
        ccos_printk("can't open a directory %s\n", pathname);
        return -1;
    }
    KERNEL_ASSERT(flags <= 7);
    int32_t fd = -1; // Default value indicating file not found

    PathSearchRecordings searched_record;
    k_memset(&searched_record, 0, sizeof(PathSearchRecordings));

    /* Record the directory depth to help determine if a subdirectory doesn't
     * exist */
    uint32_t pathname_depth = path_depth_cnt((char *)pathname);

    /* First, check if the file exists */
    int inode_no = search_file(pathname, &searched_record);
    bool found = inode_no != -1 ? true : false;

    if (searched_record.file_type == FT_DIRECTORY)
    {
        ccos_printk("can't open a directory with open(), use opendir() instead\n");
        dir_close(searched_record.parent_dir);
        return -1;
    }

    uint32_t path_searched_depth =
        path_depth_cnt(searched_record.searched_path);

    /* Check if the entire pathname was accessed, meaning no directory in the
     * path was missing */
    if (pathname_depth != path_searched_depth)
    { // This means some intermediate
      // directory is missing
        ccos_printk("sorry! cannot access %s: Not a directory, subpath %s isn't exist\n",
                    pathname, searched_record.searched_path);
        dir_close(searched_record.parent_dir);
        return -1;
    }

    /* If the file wasn't found and we don't want to create it, return -1 */
    if (!found && !(flags & O_CREAT))
    {
        ccos_printk("however in path %s, file %s isn't exist\n",
                    searched_record.searched_path,
                    (k_strrchr(searched_record.searched_path, '/') + 1));
        dir_close(searched_record.parent_dir);
        return -1;
    }
    else if (found && flags & O_CREAT)
    { // If the file already exists but we
      // want to create it
        ccos_printk("but %s has already exist!\n", pathname);
        dir_close(searched_record.parent_dir);
        return -1;
    }

    switch (flags & O_CREAT)
    {
    case O_CREAT:
        ccos_printk("creating file!\n");
        fd = file_create(searched_record.parent_dir,
                         (k_strrchr(pathname, '/') + 1), flags);
        dir_close(searched_record.parent_dir);
        break;
    default:
        /* For other cases, open an existing file: O_RDONLY, O_WRONLY, O_RDWR */
        fd = file_open(inode_no, flags);
    }

    /* The returned fd is the index of the element in the task's fd_table array,
     * not the global file_table index */
    return fd;
}

/* Convert the file descriptor to the global file table index */
uint32_t fd_local2global(uint32_t local_fd)
{
    TaskStruct *cur = current_thread();
    int32_t global_fd = cur->fd_table[local_fd];
    KERNEL_ASSERT(global_fd >= 0 && global_fd < MAX_FILE_OPEN);
    return (uint32_t)global_fd;
}

/* Close the file associated with fd, return 0 on success, otherwise return -1
 */
int32_t sys_close(int32_t fd)
{
    int32_t ret = -1; // Default return value indicating failure
    if (fd > 2)
    {
        uint32_t _fd = fd_local2global(fd);
        ret = file_close(&file_table[_fd]);
        current_thread()->fd_table[fd] = -1;
    }
    return ret;
}

/* Write count bytes from buf to the file descriptor fd, return the number of
 * bytes written on success, otherwise return -1 */
int32_t sys_write(int32_t fd, const void *buf, uint32_t count)
{
    if (fd < 0)
    {
        ccos_printk("sys_write: fd error\n");
        return -1;
    }
    if (fd == stdout_no)
    {
        /* Standard output may be redirected to a pipe buffer, so we need to
         * check */
        char tmp_buf[1024] = {0};
        k_memcpy(tmp_buf, buf, count);
        console_ccos_puts(tmp_buf);
        return count;
    }
    uint32_t _fd = fd_local2global(fd);
    File *wr_file = &file_table[_fd];
    if (wr_file->fd_flag & O_WRONLY || wr_file->fd_flag & O_RDWR)
    {
        uint32_t bytes_written = file_write(wr_file, buf, count);
        return bytes_written;
    }
    else
    {
        console_ccos_puts("sys_write: not allowed to write file without "
                          "flag O_RDWR or O_WRONLY\n");
        return -1;
    }
}

/* Read count bytes from the file pointed to by file descriptor fd into buf,
 * return the number of bytes read on success, return -1 if end of file is
 * reached
 */
int32_t sys_read(int32_t fd, void *buf, uint32_t count)
{

    if (fd < 0 || fd == stdout_no || fd == stderr_no)
    {
        ccos_printk("sys_read: fd error\n");
    }
    KERNEL_ASSERT(buf);
    uint32_t _fd = fd_local2global(fd);
    return file_read(&file_table[_fd], buf, count);
}

/* Reset the offset pointer used for file read/write operations, return the new
 * offset on success, return -1 if an error occurs
 */
int32_t sys_lseek(int32_t fd, int32_t offset, uint8_t whence)
{
    if (fd < 0)
    {
        ccos_printk("sys_lseek: fd error\n");
        return -1;
    }
    KERNEL_ASSERT(whence > 0 && whence < 4);
    uint32_t _fd = fd_local2global(fd);
    File *pf = &file_table[_fd];
    int32_t new_pos = 0; // The new position must be within the file size
    int32_t file_size = (int32_t)pf->fd_inode->i_size;
    switch (whence)
    {
    /* SEEK_SET - The new read/write position is offset from the beginning of
     * the file */
    case SEEK_SET:
        new_pos = offset;
        break;

    /* SEEK_CUR - The new read/write position is offset from the current
     * position */
    case SEEK_CUR: // offset can be positive or negative
        new_pos = (int32_t)pf->fd_pos + offset;
        break;

    /* SEEK_END - The new read/write position is offset from the end of the file
     */
    case SEEK_END: // In this case, offset should be negative
        new_pos = file_size + offset;
    }
    if (new_pos < 0 || new_pos > (file_size - 1))
    {
        return -1;
    }
    pf->fd_pos = new_pos;
    return pf->fd_pos;
}

/* Delete a file (not a directory), return 0 on success, return -1 on failure */
int32_t sys_unlink(const char *pathname)
{
    KERNEL_ASSERT(strlen(pathname) < MAX_PATH_LEN);

    /* First, check if the file to delete exists */
    PathSearchRecordings searched_record;
    k_memset(&searched_record, 0, sizeof(PathSearchRecordings));
    int inode_no = search_file(pathname, &searched_record);
    KERNEL_ASSERT(inode_no != 0);
    if (inode_no == -1)
    {
        ccos_printk("file %s not found!\n", pathname);
        dir_close(searched_record.parent_dir);
        return -1;
    }
    if (searched_record.file_type == FT_DIRECTORY)
    {
        ccos_printk("can't delete a directory with unlink(), use rmdir() instead\n");
        dir_close(searched_record.parent_dir);
        return -1;
    }

    /* Check if the file is currently in use in the open file table */
    uint32_t file_idx = 0;
    while (file_idx < MAX_FILE_OPEN)
    {
        if (file_table[file_idx].fd_inode &&
            (uint32_t)inode_no == file_table[file_idx].fd_inode->i_no)
        {
            break;
        }
        file_idx++;
    }
    if (file_idx < MAX_FILE_OPEN)
    {
        dir_close(searched_record.parent_dir);
        ccos_printk("file %s is in use, cannot delete!\n", pathname);
        return -1;
    }
    KERNEL_ASSERT(file_idx == MAX_FILE_OPEN);

    /* Allocate buffer for delete_dir_entry */
    void *io_buf = sys_malloc(SECTOR_SIZE + SECTOR_SIZE);
    if (io_buf == NULL)
    {
        dir_close(searched_record.parent_dir);
        ccos_printk("sys_unlink: malloc for io_buf failed\n");
        return -1;
    }

    Dir *parent_dir = searched_record.parent_dir;
    delete_dir_entry(cur_part, parent_dir, inode_no, io_buf);
    inode_release(cur_part, inode_no);
    sys_free(io_buf);
    dir_close(searched_record.parent_dir);
    return 0; // Successfully deleted file
}

/* Create a directory at pathname, return 0 on success, return -1 on failure */
int32_t sys_mkdir(const char *pathname)
{
    uint8_t rollback_step =
        0; // Used to rollback resource states if an operation fails
    void *io_buf = sys_malloc(SECTOR_SIZE * 2);
    if (io_buf == NULL)
    {
        ccos_printk("sys_mkdir: sys_malloc for io_buf failed\n");
        return -1;
    }

    PathSearchRecordings searched_record;
    k_memset(&searched_record, 0, sizeof(PathSearchRecordings));
    int inode_no = -1;
    inode_no = search_file(pathname, &searched_record);
    if (inode_no != -1)
    { // If a file or directory with the same name exists,
      // return failure
        ccos_printk("sys_mkdir: file or directory %s exist!\n", pathname);
        rollback_step = 1;
        goto rollback;
    }
    else
    { // If not found, check whether the final directory exists or if any
      // intermediate directory is missing
        uint32_t pathname_depth = path_depth_cnt((char *)pathname);
        uint32_t path_searched_depth =
            path_depth_cnt(searched_record.searched_path);
        /* First, check if all directory levels of pathname are accessible,
         * i.e., whether a failure occurred in accessing any intermediate
         * directory
         */
        if (pathname_depth !=
            path_searched_depth)
        { // This means some intermediate directory is
          // missing
            ccos_printk("sys_mkdir: can`t access %s, subpath %s isn't exist\n",
                        pathname, searched_record.searched_path);
            rollback_step = 1;
            goto rollback;
        }
    }

    Dir *parent_dir = searched_record.parent_dir;
    /* The directory name might have a '/' character, so it is better to use
     * searched_record.searched_path directly without '/' */
    char *dirname = k_strrchr(searched_record.searched_path, '/') + 1;

    inode_no = inode_bitmap_alloc(cur_part);
    if (inode_no == -1)
    {
        ccos_printk("sys_mkdir: allocate inode failed\n");
        rollback_step = 1;
        goto rollback;
    }

    Inode new_dir_inode;
    inode_init(inode_no, &new_dir_inode); // Initialize the inode

    uint32_t block_bitmap_idx =
        0; // To record the index of block in block_bitmap
    int32_t block_lba = -1;
    /* Allocate a block for the directory to store '.' and '..' */
    block_lba = block_bitmap_alloc(cur_part);
    if (block_lba == -1)
    {
        ccos_printk("sys_mkdir: block_bitmap_alloc for create directory failed\n");
        rollback_step = 2;
        goto rollback;
    }
    new_dir_inode.i_sectors[0] = block_lba;
    /* Synchronize the bitmap to disk after each block allocation */
    block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
    KERNEL_ASSERT(block_bitmap_idx != 0);
    bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

    /* Write the directory entries for '.' and '..' */
    k_memset(io_buf, 0, SECTOR_SIZE * 2); // Clear io_buf
    DirEntry *p_de = (DirEntry *)io_buf;

    /* Initialize the current directory entry "." */
    k_memcpy(p_de->filename, ".", 1);
    p_de->i_no = inode_no;
    p_de->f_type = FT_DIRECTORY;

    p_de++;
    /* Initialize the parent directory entry ".." */
    k_memcpy(p_de->filename, "..", 2);
    p_de->i_no = parent_dir->inode->i_no;
    p_de->f_type = FT_DIRECTORY;
    ide_write(cur_part->my_disk, new_dir_inode.i_sectors[0], io_buf, 1);

    new_dir_inode.i_size = 2 * cur_part->sb->dir_entry_size;

    /* Add the new directory entry in the parent directory */
    DirEntry new_dir_entry;
    k_memset(&new_dir_entry, 0, sizeof(DirEntry));
    create_dir_entry(dirname, inode_no, FT_DIRECTORY, &new_dir_entry);
    k_memset(io_buf, 0, SECTOR_SIZE * 2); // Clear io_buf
    if (!sync_dir_entry(parent_dir, &new_dir_entry,
                        io_buf))
    { // sync_dir_entry will synchronize the block
      // bitmap to disk
        ccos_printk("sys_mkdir: sync_dir_entry to disk failed!\n");
        rollback_step = 2;
        goto rollback;
    }

    /* Synchronize the inode of the parent directory to disk */
    k_memset(io_buf, 0, SECTOR_SIZE * 2);
    inode_sync(cur_part, parent_dir->inode, io_buf);

    /* Synchronize the inode of the new directory to disk */
    k_memset(io_buf, 0, SECTOR_SIZE * 2);
    inode_sync(cur_part, &new_dir_inode, io_buf);

    /* Synchronize the inode bitmap to disk */
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    sys_free(io_buf);

    /* Close the parent directory of the newly created directory */
    dir_close(searched_record.parent_dir);
    return 0;

/* When creating a file or directory, multiple resources need to be created,
  if any step fails, the following rollback steps will be executed */
rollback: // Rollback due to failure at some step
    switch (rollback_step)
    {
    case 2:
        bitmap_set(&cur_part->inode_bitmap, inode_no,
                   0); // If the inode creation for the new directory fails,
                       // restore the previously allocated inode
        goto DIR_CLOSE;
        break;
    case 1:
    DIR_CLOSE:
        /* Close the parent directory of the newly created directory */
        dir_close(searched_record.parent_dir);
        break;
    }
    sys_free(io_buf);
    return -1;
}

/* Searches for the file system on the disk. If none is found, it formats the
 * partition and creates the file system */
void filesystem_init()
{
    uint8_t channel_no = 0, dev_no, part_idx = 0;

    /* sb_buf is used to store the super block read from the hard disk */
    SuperBlock *sb_buf = (SuperBlock *)sys_malloc(SECTOR_SIZE);

    if (!sb_buf)
    {
        KERNEL_PANIC_SPIN("alloc memory failed!");
    }
    verbose_printk("searching filesystem......\n");
    while (channel_no < channel_cnt)
    {
        dev_no = 0;
        while (dev_no < 2)
        {
            if (dev_no == 0)
            { // Skip raw disk hd60M.img
                dev_no++;
                continue;
            }
            Disk *hd = &channels[channel_no].devices[dev_no];
            DiskPartition *part = hd->prim_parts;
            while (part_idx < 12)
            { // 4 primary partitions + 8 logical partitions
                if (part_idx == 4)
                { // Start processing logical partitions
                    part = hd->logic_parts;
                }

                /* The channels array is a global variable, with a default value
                   of 0. Disk is a nested structure, and partition is a nested
                   structure within disk. Therefore, members in partition are
                   also defaulted to 0. We now process existing partitions. */
                if (part->sec_cnt != 0)
                { // If the partition exists
                    k_memset(sb_buf, 0, SECTOR_SIZE);

                    /* Read the super block of the partition to check if the
                     * magic number is correct and determine if the file system
                     * exists */
                    ide_read(hd, part->start_lba + 1, sb_buf, 1);

                    /* Only support our own file system. If the disk already has
                     * a file system, don't format it */
                    if (sb_buf->magic == SUPER_BLOCK_MAGIC)
                    {
                        verbose_printk("%s has filesystem\n", part->name);
                    }
                    else
                    { // Unsupported file systems are treated as having
                      // no file system
                        verbose_printk("formatting %s`s partition %s......\n", hd->name,
                                       part->name);
                        partition_format(part);
                    }
                }
                part_idx++;
                part++; // Next partition
            }
            dev_no++; // Next disk
        }
        channel_no++; // Next channel
    }
    sys_free(sb_buf);
    /* Determine the default partition for operations */
    char default_part[8] = "sdb1";
    /* Mount the partition */
    list_traversal(&partition_list, mount_partition, (int)default_part);

    /* Open the root directory of the current partition */
    open_root_dir(cur_part);

    /* Initialize the file table */
    uint32_t fd_idx = 0;
    while (fd_idx < MAX_FILE_OPEN)
    {
        file_table[fd_idx++].fd_inode = NULL;
    }
}

/* Opens the directory specified by name, returns a directory pointer on
   success, returns NULL on failure */
Dir *sys_opendir(const char *name)
{
    KERNEL_ASSERT(strlen(name) < MAX_PATH_LEN);
    /* If it's the root directory '/', directly return &root_dir */
    if (name[0] == '/' && (name[1] == 0 || name[0] == '.'))
    {
        return &root_dir;
    }

    /* Check if the directory to be opened exists */
    PathSearchRecordings searched_record;
    k_memset(&searched_record, 0, sizeof(PathSearchRecordings));
    int inode_no = search_file(name, &searched_record);
    Dir *ret = NULL;
    if (inode_no ==
        -1)
    { // If the directory is not found, print an error message
        ccos_printk("In %s, sub path %s not exist\n", name,
                    searched_record.searched_path);
    }
    else
    {
        if (searched_record.file_type == FT_REGULAR)
        {
            ccos_printk("%s is regular file!\n", name);
        }
        else if (searched_record.file_type == FT_DIRECTORY)
        {
            ret = dir_open(cur_part, inode_no);
        }
    }
    dir_close(searched_record.parent_dir);
    return ret;
}

/* Closes the directory dir, returns 0 on success, returns -1 on failure */
int32_t sys_closedir(Dir *dir)
{
    int32_t ret = -1;
    if (dir)
    {
        dir_close(dir);
        ret = 0;
    }
    return ret;
}

/* Reads one directory entry from the directory dir, returns the directory entry
   address on success, returns NULL at the end of the directory or on error */
DirEntry *sys_readdir(Dir *dir)
{
    KERNEL_ASSERT(dir);
    return dir_read(dir);
}

/* Resets the directory pointer dir_pos to 0 */
void sys_rewinddir(Dir *dir)
{
    dir->dir_pos = 0;
}

/* Deletes an empty directory, returns 0 on success, returns -1 on failure */
int32_t sys_rmdir(const char *pathname)
{
    /* Check if the directory to be deleted exists */
    PathSearchRecordings searched_record;
    k_memset(&searched_record, 0, sizeof(PathSearchRecordings));
    int32_t inode_no = search_file(pathname, &searched_record);
    KERNEL_ASSERT(inode_no != 0);
    int32_t retval = -1; // Default return value
    if (inode_no == -1)
    {
        ccos_printk("In %s, sub path %s not exist\n", pathname,
                    searched_record.searched_path);
    }
    else
    {
        if (searched_record.file_type == FT_REGULAR)
        {
            ccos_printk("%s is regular file!\n", pathname);
        }
        else
        {
            Dir *dir = dir_open(cur_part, inode_no);
            if (!dir_is_empty(dir))
            { // Non-empty directory cannot be deleted
                ccos_printk("dir %s is not empty, it is not allowed to delete a "
                            "nonempty directory!\n",
                            pathname);
            }
            else
            {
                if (!dir_remove(searched_record.parent_dir, dir))
                {
                    retval = 0;
                }
            }
            dir_close(dir);
        }
    }
    dir_close(searched_record.parent_dir);
    return retval;
}

/* Gets the inode number of the parent directory of a given directory */
static uint32_t get_parent_dir_inode_nr(uint32_t child_inode_nr, void *io_buf)
{
    Inode *child_dir_inode = inode_open(cur_part, child_inode_nr);
    /* The directory entry for ".." includes the parent directory inode number,
       ".." is located in the first block of the directory */
    uint32_t block_lba = child_dir_inode->i_sectors[0];
    KERNEL_ASSERT(block_lba >= cur_part->sb->data_start_lba);
    inode_close(child_dir_inode);
    ide_read(cur_part->my_disk, block_lba, io_buf, 1);
    DirEntry *dir_e = (DirEntry *)io_buf;
    /* The first directory entry is ".", the second directory entry is ".." */
    KERNEL_ASSERT(dir_e[1].i_no < 4096 && dir_e[1].f_type == FT_DIRECTORY);
    return dir_e[1]
        .i_no; // Return the inode number of "..", which is the parent directory
}

/* Finds the name of a child directory with inode number c_inode_nr in a parent
   directory with inode number p_inode_nr, stores the name in the path buffer.
   Returns 0 on success, returns -1 on failure */
static int get_child_dir_name(uint32_t p_inode_nr, uint32_t c_inode_nr,
                              char *path, void *io_buf)
{
    Inode *parent_dir_inode = inode_open(cur_part, p_inode_nr);
    /* Populate all_blocks and read all sectors occupied by the directory into
     * all_blocks */
    uint8_t block_idx = 0;
    uint32_t all_blocks[140] = {0}, block_cnt = 12;
    while (block_idx < 12)
    {
        all_blocks[block_idx] = parent_dir_inode->i_sectors[block_idx];
        block_idx++;
    }
    if (parent_dir_inode
            ->i_sectors[12])
    { // If there is a single indirect block table,
      // read it into all_blocks.
        ide_read(cur_part->my_disk, parent_dir_inode->i_sectors[12],
                 all_blocks + 12, 1);
        block_cnt = 140;
    }
    inode_close(parent_dir_inode);

    DirEntry *dir_e = (DirEntry *)io_buf;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;
    uint32_t dir_entrys_per_sec = (512 / dir_entry_size);
    block_idx = 0;
    /* Traverse all blocks */
    while (block_idx < block_cnt)
    {
        if (all_blocks[block_idx])
        { // If the block is not empty, read it
            ide_read(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
            uint8_t dir_e_idx = 0;
            /* Traverse each directory entry */
            while (dir_e_idx < dir_entrys_per_sec)
            {
                if ((dir_e + dir_e_idx)->i_no == c_inode_nr)
                {
                    k_strcat(path, "/");
                    k_strcat(path, (dir_e + dir_e_idx)->filename);
                    return 0;
                }
                dir_e_idx++;
            }
        }
        block_idx++;
    }
    return -1;
}

/* Writes the absolute path of the current working directory into the buffer
   buf, size is the size of buf. If buf is NULL, the operating system allocates
   memory for the working path and returns the address. Returns NULL on failure
 */
char *sys_getcwd(char *buf, uint32_t size)
{
    /* Ensure buf is not NULL, if the user's buf is NULL,
       the system call getcwd allocates memory for the user's process via malloc
     */
    KERNEL_ASSERT(buf != NULL);
    void *io_buf = sys_malloc(SECTOR_SIZE);
    if (io_buf == NULL)
    {
        return NULL;
    }
    TaskStruct *cur_thread = current_thread();
    int32_t parent_inode_nr = 0;
    int32_t child_inode_nr = cur_thread->cwd_inode_nr;
    KERNEL_ASSERT(child_inode_nr >= 0 &&
           child_inode_nr < 4096); // Maximum 4096 inodes supported
    /* If the current directory is the root directory, return '/' */
    if (child_inode_nr == 0)
    {
        buf[0] = '/';
        buf[1] = 0;
        sys_free(io_buf);
        return buf;
    }

    k_memset(buf, 0, size);
    char full_path_reverse[MAX_PATH_LEN] = {0}; // Buffer for the full path

    /* Traverse upwards through the parent directories until the root directory
     * is found. When child_inode_nr is the inode number of the root directory
     * (0), stop, meaning we have processed all directory entries in the root
     * directory */
    while ((child_inode_nr))
    {
        parent_inode_nr = get_parent_dir_inode_nr(child_inode_nr, io_buf);
        if (get_child_dir_name(
                parent_inode_nr, child_inode_nr, full_path_reverse,
                io_buf) == -1)
        { // If name not found, fail and exit
            sys_free(io_buf);
            return NULL;
        }
        child_inode_nr = parent_inode_nr;
    }
    KERNEL_ASSERT(k_strlen(full_path_reverse) <= size);
    /* Now full_path_reverse contains the path in reverse order,
     * i.e., child directory is at the front (left), parent directory at the
     * back (right), reverse the path */
    char *last_slash; // To store the address of the last slash in the string
    while ((last_slash = k_strrchr(full_path_reverse, '/')))
    {
        uint16_t len = k_strlen(buf);
        k_strcpy(buf + len, last_slash);
        /* Add the null terminator in full_path_reverse to act as the boundary
         * for the next strcpy */
        *last_slash = 0;
    }
    sys_free(io_buf);
    return buf;
}

/* Changes the current working directory to the absolute path specified by path.
   Returns 0 on success, -1 on failure */
int32_t sys_chdir(const char *path)
{
    int32_t ret = -1;
    PathSearchRecordings searched_record;
    k_memset(&searched_record, 0, sizeof(PathSearchRecordings));
    int inode_no = search_file(path, &searched_record);
    if (inode_no != -1)
    {
        if (searched_record.file_type == FT_DIRECTORY)
        {
            current_thread()->cwd_inode_nr = inode_no;
            ret = 0;
        }
        else
        {
            ccos_printk("sys_chdir: %s is regular file or other!\n", path);
        }
    }
    dir_close(searched_record.parent_dir);
    return ret;
}
