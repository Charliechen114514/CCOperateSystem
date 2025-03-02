#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H
#include "include/kernel/lock.h"
#include "include/library/bitmap.h"
#include "include/library/list.h"
#include "include/library/types.h"

/*
   Hard Disk Drive
*/
typedef struct __diskpartition DiskPartition;
typedef struct __diskhandle Disk;
typedef struct __idechannel IDEChannel;

/* Partition structure */
typedef struct __diskpartition{
    uint32_t start_lba;     // Starting sector
    uint32_t sec_cnt;       // Number of sectors
    Disk *my_disk;   // The disk to which this partition belongs
    list_elem part_tag;     // Tag for use in a queue
    char name[8];           // Partition name
    struct super_block *sb; // Superblock of this partition
    Bitmap block_bitmap;    // Block bitmap
    Bitmap inode_bitmap;    // Inode bitmap
    list open_inodes;       // List of open inodes in this partition
}DiskPartition;

/* Disk structure */
typedef struct __diskhandle{
    char name[8];                   // Name of the disk, e.g., sda
    IDEChannel *my_channel; // IDE channel to which this disk belongs
    uint8_t dev_no; // Whether this disk is master (0) or slave (1)
    DiskPartition prim_parts[4]; // Primary partitions (up to 4)
    DiskPartition
        logic_parts[8]; // Logical partitions (theoretically unlimited, but
                        // capped at 8 for practicality)
}Disk;

/* ATA channel structure */
typedef struct __idechannel{
    char name[8]; // Name of the ATA channel, e.g., ata0 (also known as ide0).
                  // Refer to the Bochs configuration file for hard disk
                  // settings.
    uint16_t port_base; // Base port number for this channel
    uint8_t irq_no;     // Interrupt number used by this channel
    CCLocker lock;
    bool expecting_intr; // Waiting for an interrupt from the hard disk after
                         // sending a command
    Semaphore disk_done; // Disk operation completed. Threads use this semaphore
                         // to block themselves, and the interrupt generated by
                         // the disk wakes them up.
    Disk devices[2]; // Two disks connected to one channel: one master
                            // and one slave
}IDEChannel;

void intr_hd_handler(uint8_t irq_no); // Hard disk interrupt handler
void ide_init(void);                  // Initialize IDE
extern uint8_t channel_cnt;           // Number of IDE channels
extern IDEChannel channels[]; // Array of IDE channels
extern list partition_list;           // List of partitions
void ide_read(Disk *hd, uint32_t lba, void *buf,
              uint32_t sec_cnt); // Read from disk
void ide_write(Disk *hd, uint32_t lba, void *buf,
               uint32_t sec_cnt); // Write to disk
#endif