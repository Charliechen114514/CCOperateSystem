#ifndef __USERPROG_TSS_H
#define __USERPROG_TSS_H
#include "include/kernel/thread.h"

//---------------  TSS描述符属性  ------------
#define TSS_DESC_D  0 

#define TSS_ATTR_HIGH ((DESC_G_4K << 7) + (TSS_DESC_D << 6) + (DESC_L << 5) + (DESC_AVL << 4) + 0x0)
#define TSS_ATTR_LOW ((DESC_P << 7) + (DESC_DPL_0 << 5) + (DESC_S_SYS << 4) + DESC_TYPE_TSS)
#define SELECTOR_TSS ((4 << 3) + (TI_GDT << 2 ) + RPL0)

void update_tss_esp(TaskStruct* pthread);
void tss_init(void);
#endif
