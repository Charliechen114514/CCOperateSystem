#ifndef PROPERTY_SELECTOR_H
#define PROPERTY_SELECTOR_H

#include "include/library/types.h"

// --------------- Selectors ----------------------
// These definitions are used for segment selectors in the GDT (Global Descriptor Table).
// --------------- Selectors ----------------------

// ----------------  GDT Descriptor Attributes  ----------------

#define	DESC_G_4K    1  // Granularity bit, 1 means 4K-byte granularity.
#define	DESC_D_32    1  // Default operand size, 1 means 32-bit protected mode.
#define DESC_L	     0  // Long mode indicator, 0 means not using 64-bit code.
#define DESC_AVL     0  // Available bit, not used by CPU, set to 0.
#define DESC_P	     1  // Present bit, must be 1 for valid descriptors.
#define DESC_DPL_0   0  // Descriptor Privilege Level 0 (highest privilege).
#define DESC_DPL_1   1  // Descriptor Privilege Level 1.
#define DESC_DPL_2   2  // Descriptor Privilege Level 2.
#define DESC_DPL_3   3  // Descriptor Privilege Level 3 (lowest privilege).

/* 
   Code segments and data segments belong to storage segments,
   while TSS and various gate descriptors belong to system segments.
   S = 1 indicates a storage segment, while S = 0 indicates a system segment.
*/
#define DESC_S_CODE	1  // Code segment descriptor.
#define DESC_S_DATA	DESC_S_CODE  // Data segment descriptor (same as code).
#define DESC_S_SYS	0  // System segment descriptor.

#define DESC_TYPE_CODE	8  // Code segment: executable, non-conforming, not readable, accessed bit cleared.
#define DESC_TYPE_DATA  2  // Data segment: non-executable, expanding upward, writable, accessed bit cleared.
#define DESC_TYPE_TSS   9  // TSS (Task State Segment), B-bit is 0 (not busy).

// Ring privilege levels (RPL).
#define	 RPL0  0  // Highest privilege (kernel).
#define	 RPL1  1  
#define	 RPL2  2  
#define	 RPL3  3  // Lowest privilege (user mode).

// Table Indicator (TI) values.
#define TI_GDT 0  // Refers to GDT.
#define TI_LDT 1  // Refers to LDT.

#define SELECTOR_K_CODE	   ((1 << 3) + (TI_GDT << 2) + RPL0)  // Kernel code segment selector.
#define SELECTOR_K_DATA	   ((2 << 3) + (TI_GDT << 2) + RPL0)  // Kernel data segment selector.
#define SELECTOR_K_STACK   SELECTOR_K_DATA  // Kernel stack segment selector (same as data).
#define SELECTOR_K_GS	   ((3 << 3) + (TI_GDT << 2) + RPL0)  // Kernel GS segment selector.

/* The third descriptor is for video memory, and the fourth is for TSS */
#define SELECTOR_U_CODE	   ((5 << 3) + (TI_GDT << 2) + RPL3)  // User code segment selector.
#define SELECTOR_U_DATA	   ((6 << 3) + (TI_GDT << 2) + RPL3)  // User data segment selector.
#define SELECTOR_U_STACK   SELECTOR_U_DATA  // User stack segment selector (same as data).

// GDT descriptor attributes.
#define GDT_ATTR_HIGH		 ((DESC_G_4K << 7) + (DESC_D_32 << 6) + (DESC_L << 5) + (DESC_AVL << 4))
#define GDT_CODE_ATTR_LOW_DPL3	 ((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_CODE << 4) + DESC_TYPE_CODE)
#define GDT_DATA_ATTR_LOW_DPL3	 ((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_DATA << 4) + DESC_TYPE_DATA)

//--------------   IDT Descriptor Attributes  ------------

#define	 IDT_DESC_P	 1  // Present bit, must be 1 for valid IDT entries.
#define	 IDT_DESC_DPL0   0  // DPL 0 (kernel-level interrupt).
#define	 IDT_DESC_DPL3   3  // DPL 3 (user-level interrupt).

#define	 IDT_DESC_32_TYPE     0xE  // 32-bit interrupt gate.
#define	 IDT_DESC_16_TYPE     0x6  // 16-bit interrupt gate (not used, defined for distinction).

// IDT descriptor attributes.
#define	 IDT_DESC_ATTR_DPL0  ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)
#define	 IDT_DESC_ATTR_DPL3  ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)

//---------------    EFLAGS Register Attributes    ---------------- 

/********************************************************
--------------------------------------------------------------
		  Intel 8086 EFLAGS Register
--------------------------------------------------------------
*
*     15|14|13|12|11|10|F|E|D C|B|A|9|8|7|6|5|4|3|2|1|0|
*      |  |  |  |  |  | | |  |  | | | | | | | | | | '---  CF……Carry Flag
*      |  |  |  |  |  | | |  |  | | | | | | | | | '---  1 MBS (must be set)
*      |  |  |  |  |  | | |  |  | | | | | | | | '---  PF……Parity Flag
*      |  |  |  |  |  | | |  |  | | | | | | | '---  0 (Reserved)
*      |  |  |  |  |  | | |  |  | | | | | | '---  AF……Auxiliary Flag
*      |  |  |  |  |  | | |  |  | | | | | '---  0 (Reserved)
*      |  |  |  |  |  | | |  |  | | | | '---  ZF……Zero Flag
*      |  |  |  |  |  | | |  |  | | | '---  SF……Sign Flag
*      |  |  |  |  |  | | |  |  | | '---  TF……Trap Flag
*      |  |  |  |  |  | | |  |  | '---  IF……Interrupt Flag
*      |  |  |  |  |  | | |  |  '---  DF……Direction Flag
*      |  |  |  |  |  | | |  '----  OF……Overflow flag
*      |  |  |  |  |  | | '-----  IOPL……I/O Privilege Level
*      |  |  |  |  |  | '-----  NT……Nested Task Flag
*      |  |  |  |  |  '-----  0 (Reserved)
*      |  |  |  |  '------  RF……Resume Flag
*      |  |  |  '-----  VM……Virtual Mode Flag
*      |  |  '-----  AC……Alignment Check
*      |  '-----  VIF……Virtual Interrupt Flag  
*      '-----  VIP……Virtual Interrupt Pending
*
**********************************************************/

#define EFLAGS_MBS	(1 << 1)	// Must be set to 1.
#define EFLAGS_IF_1	(1 << 9)	// IF = 1, enables interrupts.
#define EFLAGS_IF_0	0		// IF = 0, disables interrupts.
#define EFLAGS_IOPL_3	(3 << 12)	// IOPL = 3, allows user-mode I/O (for testing).
#define EFLAGS_IOPL_0	(0 << 12)	// IOPL = 0, restricts I/O access to kernel mode.

#endif
