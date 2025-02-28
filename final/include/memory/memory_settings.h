#ifndef MEMORY_SETTINGS_H
#define MEMORY_SETTINGS_H

#define PGSIZE              (4096)
#define MEM_BITMAP_BASE     (0xc009a000)

/* 0xc0000000 is the starting address of the kernel's virtual memory, starting
 * at 3GB. 0x100000 skips the lower 1M of memory, ensuring that the virtual
 * address space is logically contiguous. */
#define KERNEL_HEAP_START   (0xc0100000)

#define	    PG_P_1	    1	// 页表项或页目录项存在属性位
#define	    PG_P_0	    0	// 页表项或页目录项存在属性位
#define	    PG_RW_R     0	// R/W 属性位值, 读/执行
#define	    PG_RW_W     2	// R/W 属性位值, 读/写/执行
#define	    PG_US_S     0	// U/S 属性位值, 系统级
#define	    PG_US_U     4	// U/S 属性位值, 用户级

#define     DESC_CNT 7


#endif