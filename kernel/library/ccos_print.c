#include "include/library/ccos_print.h"
#include "include/device/console_tty.h"
#include "include/utils/early_init_console_settings.h"

// set the _ccos_print.s for details
void __ccos_set_console_cursor(uint16_t row, uint16_t col);
void ccos_set_console_cursor(uint16_t row, uint16_t col) {
    uint8_t __col = col + CONSOLE_WIDTH * row;
    uint8_t __row = (CONSOLE_WIDTH * row + col - __col) % UINT8_MAX;
    __ccos_set_console_cursor(__row, __col);
}

void early_console_init(void) {
    ccos_set_console_cursor(5, 0);
    pretty_write(READY_INIT_STR);
}

void post_console_init(void) {
    ccos_puts(MY_ANNOUNCE);
}

// We support colorful print, for performace, we disabled it
// static Color pretty_color = COLOR_BRIGHT_BLUE;
void pretty_write(char *message) {
    // set_console_color(pretty_color);
    ccos_puts(message);
    // set_console_color(CONSOLE_PEN_COLOR);
}

void verbose_write(char *message) {
    (void)message;
#ifdef VERBOSE
    ccos_puts(message);
#endif
}

void verbose_write_int(uint32_t interger) {
    (void)interger;
#ifdef VERBOSE
    __ccos_display_int(interger);
#endif
}