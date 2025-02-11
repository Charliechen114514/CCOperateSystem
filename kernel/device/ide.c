#include "include/device/ide.h"
#include "include/FormatIO/stdio-kernel.h"
#include "include/device/console_tty.h"
#include "include/device/timer.h"
#include "include/io/io.h"
#include "include/kernel/interrupt.h"
#include "include/kernel/lock.h"
#include "include/library/kernel_assert.h"
#include "include/library/list.h"
#include "include/library/string.h"
#include "include/memory/memory.h"
#include "include/user/stdio/stdio.h"
#include "include/utils/fast_utils.h"

/* Define port numbers for IDE registers */
#define reg_data(channel) (channel->port_base + 0)
#define reg_error(channel) (channel->port_base + 1)
#define reg_sect_cnt(channel) (channel->port_base + 2)
#define reg_lba_l(channel) (channel->port_base + 3)
#define reg_lba_m(channel) (channel->port_base + 4)
#define reg_lba_h(channel) (channel->port_base + 5)
#define reg_dev(channel) (channel->port_base + 6)
#define reg_status(channel) (channel->port_base + 7)
#define reg_cmd(channel) (reg_status(channel))
#define reg_alt_status(channel) (channel->port_base + 0x206)
#define reg_ctl(channel) reg_alt_status(channel)

/* Key bits in the reg_status register */
#define BIT_STAT_BSY 0x80  // Hard disk is busy
#define BIT_STAT_DRDY 0x40 // Drive is ready
#define BIT_STAT_DRQ 0x8   // Data transfer is ready

/* Key bits in the device register */
#define BIT_DEV_MBS 0xa0 // Bits 7 and 5 are fixed to 1
#define BIT_DEV_LBA 0x40 // LBA (Logical Block Addressing) mode
#define BIT_DEV_DEV 0x10 // Device selection (0 for master, 1 for slave)

/* Hard disk operation commands */
#define CMD_IDENTIFY 0xec     // Identify command
#define CMD_READ_SECTOR 0x20  // Read sector command
#define CMD_WRITE_SECTOR 0x30 // Write sector command

/* Maximum number of sectors for read/write (debugging purposes) */
#define max_lba ((80 * 1024 * 1024 / 512) - 1) // Supports only 80MB hard disk

uint8_t channel_cnt;    // Number of IDE channels based on the number of disks
IDEChannel channels[2]; // There are two IDE channels

/* Records the starting LBA of the extended partition, initially 0 */
int32_t ext_lba_base = 0;

uint8_t p_no = 0, l_no = 0; // Indexes for primary and logical partitions

list partition_list; // List of partitions

/* A 16-byte structure to store partition table entries */
struct partition_table_entry {
    uint8_t bootable;      // Whether the partition is bootable
    uint8_t start_head;    // Starting head number
    uint8_t start_sec;     // Starting sector number
    uint8_t start_chs;     // Starting cylinder number
    uint8_t fs_type;       // Partition type
    uint8_t end_head;      // Ending head number
    uint8_t end_sec;       // Ending sector number
    uint8_t end_chs;       // Ending cylinder number
    uint32_t start_lba;    // Starting LBA address of the partition
    uint32_t sec_cnt;      // Number of sectors in the partition
} __attribute__((packed)); // Ensure the structure is 16 bytes

/* Boot sector, where MBR or EBR resides */
struct boot_sector {
    uint8_t other[446]; // Boot code
    struct partition_table_entry
        partition_table[4]; // Partition table with 4 entries (64 bytes)
    uint16_t signature;     // End signature of the boot sector (0x55, 0xAA)
} __attribute__((packed));

/* Select the disk for read/write operations */
static void select_disk(Disk *hd) {
    uint8_t reg_device = BIT_DEV_MBS | BIT_DEV_LBA;
    if (hd->dev_no == 1) { // If it's the slave disk, set the DEV bit to 1
        reg_device |= BIT_DEV_DEV;
    }
    outb(reg_dev(hd->my_channel), reg_device);
}

/* Write the starting sector address and sector count to the disk controller */
static void select_sector(Disk *hd, uint32_t lba, uint8_t sec_cnt) {
    ASSERT(lba <= max_lba);
    IDEChannel *channel = hd->my_channel;

    /* Write the number of sectors to read/write */
    outb(reg_sect_cnt(channel),
         sec_cnt); // If sec_cnt is 0, it means 256 sectors

    /* Write the LBA address (sector number) */
    outb(reg_lba_l(channel), lba);       // Lower 8 bits of LBA address
    outb(reg_lba_m(channel), lba >> 8);  // Bits 8-15 of LBA address
    outb(reg_lba_h(channel), lba >> 16); // Bits 16-23 of LBA address

    /* Re-write the device register to store bits 24-27 of the LBA address */
    outb(reg_dev(channel), BIT_DEV_MBS | BIT_DEV_LBA |
                               (hd->dev_no == 1 ? BIT_DEV_DEV : 0) | lba >> 24);
}

/* Send a command to the IDE channel */
static void cmd_out(IDEChannel *channel, uint8_t cmd) {
    /* Set this flag to true whenever a command is issued to the disk */
    channel->expecting_intr = true;
    outb(reg_cmd(channel), cmd);
}

/* Read sec_cnt sectors of data from the disk into buf */
static void read_from_sector(Disk *hd, void *buf, uint8_t sec_cnt) {
    uint32_t size_in_byte;
    if (sec_cnt == 0) {
        /* If sec_cnt is 0, it means 256 sectors */
        size_in_byte = 256 * 512;
    } else {
        size_in_byte = sec_cnt * 512;
    }
    insw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

/* Write sec_cnt sectors of data from buf to the disk */
static void write2sector(Disk *hd, void *buf, uint8_t sec_cnt) {
    uint32_t size_in_byte;
    if (sec_cnt == 0) {
        /* If sec_cnt is 0, it means 256 sectors */
        size_in_byte = 256 * 512;
    } else {
        size_in_byte = sec_cnt * 512;
    }
    outsw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

/* Wait for 30 seconds for the disk to become ready */
static bool busy_wait(Disk *hd) {
    IDEChannel *channel = hd->my_channel;
    uint16_t time_limit = 30 * 1000; // Wait for up to 30,000 milliseconds
    while (time_limit -= 10 >= 0) {
        if (!(inb(reg_status(channel)) & BIT_STAT_BSY)) {
            return (inb(reg_status(channel)) & BIT_STAT_DRQ);
        } else {
            mtime_sleep(10); // Sleep for 10 milliseconds
        }
    }
    return false;
}

/* Read sec_cnt sectors from the disk into buf */
void ide_read(Disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    ASSERT(lba <= max_lba);
    ASSERT(sec_cnt > 0);
    lock_acquire(&hd->my_channel->lock);

    /* 1. Select the disk */
    select_disk(hd);

    uint32_t secs_op;       // Number of sectors to operate on
    uint32_t secs_done = 0; // Number of sectors already processed
    while (secs_done < sec_cnt) {
        if ((secs_done + 256) <= sec_cnt) {
            secs_op = 256;
        } else {
            secs_op = sec_cnt - secs_done;
        }

        /* 2. Write the sector count and starting sector number */
        select_sector(hd, lba + secs_done, secs_op);

        /* 3. Issue the read command */
        cmd_out(hd->my_channel, CMD_READ_SECTOR);

        /* Block the thread until the disk operation completes */
        sema_down(&hd->my_channel->disk_done);

        /* 4. Check if the disk is ready */
        if (!busy_wait(hd)) { // If failed
            char error[64];
            sprintf(error, "%s read sector %d failed!!!!!!\n", hd->name, lba);
            KERNEL_PANIC(error);
        }

        /* 5. Read data from the disk's buffer */
        read_from_sector(hd, (void *)((uint32_t)buf + secs_done * 512),
                         secs_op);
        secs_done += secs_op;
    }
    lock_release(&hd->my_channel->lock);
}

/* Write sec_cnt sectors of data from buf to the disk */
void ide_write(Disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    ASSERT(lba <= max_lba);
    ASSERT(sec_cnt > 0);
    lock_acquire(&hd->my_channel->lock);

    /* 1. Select the disk */
    select_disk(hd);

    uint32_t secs_op;       // Number of sectors to operate on
    uint32_t secs_done = 0; // Number of sectors already processed
    while (secs_done < sec_cnt) {
        if ((secs_done + 256) <= sec_cnt) {
            secs_op = 256;
        } else {
            secs_op = sec_cnt - secs_done;
        }

        /* 2. Write the sector count and starting sector number */
        select_sector(hd, lba + secs_done, secs_op);

        /* 3. Issue the write command */
        cmd_out(hd->my_channel, CMD_WRITE_SECTOR);

        /* 4. Check if the disk is ready */
        if (!busy_wait(hd)) { // If failed
            char error[64];
            sprintf(error, "%s write sector %d failed!!!!!!\n", hd->name, lba);
            KERNEL_PANIC(error);
        }

        /* 5. Write data to the disk */
        write2sector(hd, (void *)((uint32_t)buf + secs_done * 512), secs_op);

        /* Block the thread until the disk operation completes */
        sema_down(&hd->my_channel->disk_done);
        secs_done += secs_op;
    }
    lock_release(&hd->my_channel->lock);
}

/* Swap adjacent bytes in dst and store the result in buf */
static void swap_pairs_bytes(const char *dst, char *buf, uint32_t len) {
    uint8_t idx;
    for (idx = 0; idx < len; idx += 2) {
        buf[idx + 1] = *dst++;
        buf[idx] = *dst++;
    }
    buf[idx] = '\0';
}

/* Retrieve disk parameter information */
static void identify_disk(Disk *hd) {
    char id_info[512];
    select_disk(hd);
    cmd_out(hd->my_channel, CMD_IDENTIFY);
    /* Block the thread until the disk operation completes */
    sema_down(&hd->my_channel->disk_done);

    /* Check if the disk is ready */
    if (!busy_wait(hd)) { // If failed
        char error[64];
        sprintf(error, "%s identify failed!!!!!!\n", hd->name);
        KERNEL_PANIC(error);
    }
    read_from_sector(hd, id_info, 1);

    char buf[64];
    uint8_t sn_start = 10 * 2, sn_len = 20, md_start = 27 * 2, md_len = 40;
    swap_pairs_bytes(&id_info[sn_start], buf, sn_len);
    verbose_printk("     disk %s info:\n      SN: %s\n", hd->name, buf);
    memset(buf, 0, sizeof(buf));
    swap_pairs_bytes(&id_info[md_start], buf, md_len);
    verbose_printk("      MODULE: %s\n", buf);
    uint32_t sectors = *(uint32_t *)&id_info[60 * 2];
    verbose_printk("      SECTORS: %d\n", sectors);
    verbose_printk("      CAPACITY: %dMB\n", sectors * 512 / 1024 / 1024);
}

/* Scan all partitions in the sector at ext_lba on disk hd */
static void partition_scan(Disk *hd, uint32_t ext_lba) {
    struct boot_sector *bs = sys_malloc(sizeof(struct boot_sector));
    ide_read(hd, ext_lba, bs, 1);
    uint8_t part_idx = 0;
    struct partition_table_entry *p = bs->partition_table;

    /* Traverse the 4 partition table entries */
    while (part_idx++ < 4) {
        if (p->fs_type == 0x5) { // If it's an extended partition
            if (ext_lba_base != 0) {
                /* The start_lba of the child extended partition is relative to
                 * the base */
                partition_scan(hd, p->start_lba + ext_lba_base);
            } else { // First time reading the boot block (MBR)
                /* Record the base LBA of the extended partition */
                ext_lba_base = p->start_lba;
                partition_scan(hd, p->start_lba);
            }
        } else if (p->fs_type != 0) { // If it's a valid partition type
            if (ext_lba == 0) {       // Primary partitions
                hd->prim_parts[p_no].start_lba = ext_lba + p->start_lba;
                hd->prim_parts[p_no].sec_cnt = p->sec_cnt;
                hd->prim_parts[p_no].my_disk = hd;
                list_append(&partition_list, &hd->prim_parts[p_no].part_tag);
                sprintf(hd->prim_parts[p_no].name, "%s%d", hd->name, p_no + 1);
                p_no++;
                ASSERT(p_no < 4); // Only 4 primary partitions (0,1,2,3)
            } else {              // Logical partitions
                hd->logic_parts[l_no].start_lba = ext_lba + p->start_lba;
                hd->logic_parts[l_no].sec_cnt = p->sec_cnt;
                hd->logic_parts[l_no].my_disk = hd;
                list_append(&partition_list, &hd->logic_parts[l_no].part_tag);
                sprintf(hd->logic_parts[l_no].name, "%s%d", hd->name,
                        l_no + 5); // Logical partitions start from 5
                l_no++;
                if (l_no >= 8) // Only support up to 8 logical partitions
                    return;
            }
        }
        p++;
    }
    sys_free(bs);
}

/* Print partition information */
static bool partition_info(list_elem *pelem, int arg) {
    (void)arg;
    DiskPartition *part = elem2entry(DiskPartition, part_tag, pelem);
    printk("   %s start_lba:0x%x, sec_cnt:0x%x\n", part->name, part->start_lba,
           part->sec_cnt);

    /* Return false to continue traversal */
    return false;
}

/* Hard disk interrupt handler */
void intr_hd_handler(uint8_t irq_no) {
    ASSERT(irq_no == 0x2e || irq_no == 0x2f);
    uint8_t ch_no = irq_no - 0x2e;
    IDEChannel *channel = &channels[ch_no];
    ASSERT(channel->irq_no == irq_no);
    /* Handle the interrupt if it was expected */
    if (channel->expecting_intr) {
        channel->expecting_intr = false;
        sema_up(&channel->disk_done);

        /* Read the status register to acknowledge the interrupt */
        inb(reg_status(channel));
    }
}

/* Initialize the IDE subsystem */
void ide_init() {
    verbose_printk("ide_init start\n");
    uint8_t hd_cnt = *((uint8_t *)(0x475)); // Get the number of hard disks
    ASSERT(hd_cnt > 0);
    list_init(&partition_list);
    channel_cnt = ROUNDUP(hd_cnt, 2); // Each IDE channel supports 2 disks
    IDEChannel *channel;
    uint8_t channel_no = 0, dev_no = 0;

    /* Initialize each IDE channel and its disks */
    while (channel_no < channel_cnt) {
        channel = &channels[channel_no];
        sprintf(channel->name, "ide%d", channel_no);

        /* Set the base port and interrupt vector for each IDE channel */
        switch (channel_no) {
        case 0:
            channel->port_base = 0x1f0;  // IDE0 channel base port
            channel->irq_no = 0x20 + 14; // IRQ 14 for IDE0
            break;
        case 1:
            channel->port_base = 0x170;  // IDE1 channel base port
            channel->irq_no = 0x20 + 15; // IRQ 15 for IDE1
            break;
        }

        channel->expecting_intr = false; // No interrupt expected initially
        lock_init(&channel->lock);
        sema_init(&channel->disk_done,
                  0); // Initialize the semaphore for disk operations

        register_intr_handler(channel->irq_no, intr_hd_handler);

        /* we fetch the devices info */
        while (dev_no < 2) {
            Disk *hd = &channel->devices[dev_no];
            hd->my_channel = channel;
            hd->dev_no = dev_no;
            sprintf(hd->name, "sd%c", 'a' + channel_no * 2 + dev_no);
            identify_disk(hd); // verify the disk
            if (dev_no != 0) {
                partition_scan(hd, 0); //	scan the disk
            }
            p_no = 0, l_no = 0;
            dev_no++;
        }
        dev_no = 0;   // set the dev as 0
        channel_no++; // so the next
    }

    verbose_printk("\n   all partition info\n");
    list_traversal(&partition_list, partition_info, (int)NULL);
    verbose_printk("ide_init done\n");
}
