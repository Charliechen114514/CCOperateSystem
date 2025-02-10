#include "include/kernel/interrupt.h"
#include "include/kernel/kernel_table_property.h"
#include "include/library/types.h"
#include "include/library/ccos_print.h"
#include "include/io/io.h"

#define IDT_DESC_SZ     0x21
typedef struct {
    uint16_t    func_off_set_low;
    uint16_t    selector;
    uint8_t     desp_cnt;
    uint8_t     attr;
    uint16_t    func_off_set_high;
}Gate_Descriptor;

#define PCI_MASTER_CONTROL_PORT     (0x20)
#define PCI_MASTER_DATA_PORT        (0x21)
#define PCI_SLAVE_CONTROL_PORT      (0xa0)
#define PCI_SLAVE_DATA_PORT         (0xa1)

#define EFLAGS_IF   0x00000200       

// we push the eflags into the register
#define GET_EFLAGS(EFLAG_VAR) asm volatile(" \
    pushfl; \
    popl %0" : "=g" (EFLAG_VAR))


char* interrupt_name[IDT_DESC_SZ];
// for kernel_init.S
intr_func_ptr idt_table[IDT_DESC_SZ];

static Gate_Descriptor  idt_table_internel[IDT_DESC_SZ];
extern intr_func_ptr    interrupt_entry_tbl[IDT_DESC_SZ]; 

Interrupt_Status get_intr_state(void)
{
    uint32_t eflags = 0; 
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

Interrupt_Status enable_intr(void)
{
    Interrupt_Status old;
    if (INTR_ON == get_intr_state()) {
        old = INTR_ON;
    } else {
        old = INTR_OFF;
        asm volatile("sti");
    }
    return old;
}

Interrupt_Status disable_intr(void)
{
    Interrupt_Status old;
    if (INTR_ON == get_intr_state()) {
        old = INTR_ON;
        asm volatile("cli" : : : "memory"); // 关中断,cli指令将IF位置0
     } else {
        old = INTR_OFF;
     }
    return old;
}

Interrupt_Status set_intr_state(Interrupt_Status status){
    return status & INTR_ON ? enable_intr() : disable_intr();
}

static void pic_init(void)
{
    ccos_puts("     start init pic\n");
    /* 初始化主片 */
    outb (PCI_MASTER_CONTROL_PORT, 0x11);   // ICW1
    outb (PCI_MASTER_DATA_PORT, 0x20);   // ICW2
    outb (PCI_MASTER_DATA_PORT, 0x04);   // ICW3
    outb (PCI_MASTER_DATA_PORT, 0x01);   // ICW4

    /* 初始化从片 */
    outb (PCI_SLAVE_CONTROL_PORT, 0x11);    // ICW1
    outb (PCI_SLAVE_DATA_PORT, 0x28);    // ICW2
    outb (PCI_SLAVE_DATA_PORT, 0x02);    // ICW3
    outb (PCI_SLAVE_DATA_PORT, 0x01);    // ICW4

    outb (PCI_MASTER_DATA_PORT, 0xfe);
    outb (PCI_SLAVE_DATA_PORT, 0xff);
    ccos_puts("     pic init done\n");
}

static void init_gate_descriptor(
    Gate_Descriptor*    blank, 
    uint8_t             attr,
    intr_func_ptr       handle_ptr)
{
    blank->func_off_set_low = (uint32_t)handle_ptr & 0x0000FFFF;  
    blank->selector         = SELECTOR_K_CODE;
    blank->desp_cnt         = 0;
    blank->attr             = attr;
    blank->func_off_set_high= ((uint32_t)handle_ptr & 0xFFFF0000) >> 16;
}

static void init_gdt_tbl(void)
{
    uint16_t i = 0;
    ccos_puts("     gdt table start!\n");
    for (i = 0; i < IDT_DESC_SZ; i++)
    {
        init_gate_descriptor(&idt_table_internel[i], IDT_DESC_ATTR_DPL0, interrupt_entry_tbl[i]);
    }
    ccos_puts("     gdt table done!\n");
}

static void generate_interrupt_handler(uint8_t nvec)
{
    if(nvec == 0x27 || nvec == 0x2f){
        return;
    }
// you can enable this if you wanna see the intr number
// ccos_puts("receive interrupt: 0x");
// __ccos_display_int(nvec);
// __ccos_putchar('\n');
}

static void exception_init(void)
{
    uint16_t i;
    for(i = 0; i < IDT_DESC_SZ; i++){
        idt_table[i] = generate_interrupt_handler;
        interrupt_name[i] = "unknown interrupt";
    }
    interrupt_name[0] = "#DE Divide Error";
    interrupt_name[1] = "#DB Debug Exception";
    interrupt_name[2] = "NMI Interrupt";
    interrupt_name[3] = "#BP Breakpoint Exception";
    interrupt_name[4] = "#OF Overflow Exception";
    interrupt_name[5] = "#BR BOUND Range Exceeded Exception";
    interrupt_name[6] = "#UD Invalid Opcode Exception";
    interrupt_name[7] = "#NM Device Not Available Exception";
    interrupt_name[8] = "#DF Double Fault Exception";
    interrupt_name[9] = "Coprocessor Segment Overrun";
    interrupt_name[10] = "#TS Invalid TSS Exception";
    interrupt_name[11] = "#NP Segment Not Present";
    interrupt_name[12] = "#SS Stack Fault Exception";
    interrupt_name[13] = "#GP General Protection Exception";
    interrupt_name[14] = "#PF Page-Fault Exception";
    // interrupt_name[15] is unused
    interrupt_name[16] = "#MF x87 FPU Floating-Point Error";
    interrupt_name[17] = "#AC Alignment Check Exception";
    interrupt_name[18] = "#MC Machine-Check Exception";
    interrupt_name[19] = "#XF SIMD Floating-Point Exception";
}

void idt_init(void)
{
    ccos_puts("start initialize the idt!\n");
    exception_init();       // exception hook init
    init_gdt_tbl();         // gdt init
    pic_init();             // pic init
    uint64_t idt_operand = 
            ((sizeof(idt_table_internel) - 1) | 
            ((uint64_t)(uint32_t)idt_table_internel << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
    ccos_puts("idt_init done\n");
}