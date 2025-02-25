#ifndef INTERRUPT_H
#define INTERRUPT_H

// idt initings
void init_idt(void);

void sti(void);
void cli(void);

#endif