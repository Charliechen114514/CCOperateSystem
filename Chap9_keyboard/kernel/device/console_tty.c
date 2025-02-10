#include "include/device/console_tty.h"
#include "include/kernel/lock.h"
#include "include/library/ccos_print.h"

static CCLocker console_lock;

void init_console(void){
    ccos_puts("console init start...\n");
    init_locker(&console_lock);
    ccos_puts("console init done!\n");
}

void acquire_console(void){
    acquire_lock(&console_lock);
}
void release_console(void){
    release_lock(&console_lock);
}

void console_puts(char* str){
    acquire_lock(&console_lock);
    ccos_puts(str);
    release_lock(&console_lock);
}

void console_putchar(uint8_t ascii){
    acquire_lock(&console_lock);
    __ccos_putchar(ascii);
    release_lock(&console_lock);
}

void console_putint(uint32_t num){
    acquire_lock(&console_lock);
    __ccos_display_int(num);
    release_lock(&console_lock);
}

void console_setcursor(uint16_t row, uint16_t col){
    acquire_lock(&console_lock);
    ccos_set_console_cursor(row, col);
    release_lock(&console_lock);
}