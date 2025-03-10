#ifndef __FS_INODE_H
#define __FS_INODE_H
#include "include/library/types.h"
#include "include/library/list.h"
#include "include/device/ide.h"

/* inode结构 */
typedef struct __inode{
   uint32_t i_no;    // inode编号

/* 当此inode是文件时,i_size是指文件大小,
若此inode是目录,i_size是指该目录下所有目录项大小之和*/
   uint32_t i_size;

   uint32_t i_open_cnts;   // 记录此文件被打开的次数
   bool write_deny;	   // 写文件不能并行,进程写文件前检查此标识

/* i_sectors[0-11]是直接块, i_sectors[12]用来存储一级间接块指针 */
   uint32_t i_sectors[13];
   list_elem inode_tag;
}Inode;

Inode* inode_open(DiskPartition* part, uint32_t inode_no);
void inode_sync(DiskPartition* part, Inode* inode, void* io_buf);
void inode_init(uint32_t inode_no, Inode* new_inode);
void inode_close(Inode* inode);
void inode_release(DiskPartition* part, uint32_t inode_no);
void inode_delete(DiskPartition* part, uint32_t inode_no, void* io_buf);
#endif
