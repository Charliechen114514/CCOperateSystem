#ifndef CONSOLE_TTY_H
#define CONSOLE_TTY_H
#include "include/library/types.h"

/*
    console interface!
    which is sync promised, so after the console init
    use console_puts for kernel and printf for user :)
*/
void    console_init(void);
void    console_acquire(void);
void    console_release(void);
void    console_ccos_puts(char* str);
void    console__ccos_putchar(uint8_t char_asci);
void    console__ccos_display_int(uint32_t num);

void sys_putchar(char char_asci);


#endif