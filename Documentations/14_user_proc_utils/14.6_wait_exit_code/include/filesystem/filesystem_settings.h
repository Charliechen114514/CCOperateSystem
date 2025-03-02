#ifndef FILESYSTEM_SETTINGS
#define FILESYSTEM_SETTINGS

#define MAX_FILES_PER_PART (4096)     // Maximum number of files that can be created in a partition
#define BITS_PER_SECTOR (4096)        // Number of bits per sector
#define SECTOR_SIZE (512)             // Size of a sector in bytes
#define BLOCK_SIZE SECTOR_SIZE      // Size of a block in bytes

#define MAX_PATH_LEN (512)            // Maximum length of a file path


#endif