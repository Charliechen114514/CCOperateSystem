#ifndef PROPERTY_SELECTOR_H
#define PROPERTY_SELECTOR_H

// RPL Settings for IDT (Request Privilege Level)
#define  RPL0  0   // Ring 0: Kernel mode
#define  RPL1  1   // Ring 1: Reserved for device drivers
#define  RPL2  2   // Ring 2: Reserved for certain kernel functions
#define  RPL3  3   // Ring 3: User mode

// TI (Table Indicator) values
#define TI_GDT 0   // GDT (Global Descriptor Table)
#define TI_LDT 1   // LDT (Local Descriptor Table)

// Selectors for various segments (Code, Data, Stack, GS)
#define SELECTOR_K_CODE    ((1 << 3) + (TI_GDT << 2) + RPL0)  // Kernel Code Segment Selector
#define SELECTOR_K_DATA    ((2 << 3) + (TI_GDT << 2) + RPL0)  // Kernel Data Segment Selector
#define SELECTOR_K_STACK   SELECTOR_K_DATA                 // Kernel Stack Segment Selector (same as Data)
#define SELECTOR_K_GS      ((3 << 3) + (TI_GDT << 2) + RPL0)  // Kernel GS Segment Selector

//--------------   IDT Property Table ------------

// IDT descriptor properties
#define  IDT_DESC_P        1   // Present flag (descriptor is present)
#define  IDT_DESC_DPL0     0   // Descriptor Privilege Level 0 (Kernel)
#define  IDT_DESC_DPL3     3   // Descriptor Privilege Level 3 (User)
#define  IDT_DESC_32_TYPE  0xE // 32-bit Gate Type (Interrupt Gate)
#define  IDT_DESC_16_TYPE  0x6 // 16-bit Gate Type (Trap Gate)

// IDT descriptor attributes with privilege levels
#define  IDT_DESC_ATTR_DPL0  ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)  // 32-bit interrupt gate with DPL 0
#define  IDT_DESC_ATTR_DPL3  ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)  // 32-bit interrupt gate with DPL 3

#endif
