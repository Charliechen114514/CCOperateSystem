#include "include/device/keyboard.h"
#include "include/library/ccos_print.h"
#include "include/kernel/interrupt.h"
#include "include/device/configs/keyboard_ascii.h"
#include "include/device/configs/keyboard_mappings.h"
#include "include/io/io.h"

// keyboard ringbuffers here
IOQueue keyboard_ringbuffer;

/* Defines a structure to record the status of special keys */
static struct {
    bool ctrl_status;      // Indicates whether the Ctrl key is pressed
    bool shift_status;     // Indicates whether the Shift key is pressed
    bool alt_status;       // Indicates whether the Alt key is pressed
    bool caps_lock_status; // Indicates whether Caps Lock is active
    bool ext_scancode;     // Indicates if the scancode starts with 0xe0
} key_state;

static void keyboard_intr_handler(void)
{
    // fetch the recordings
    bool prev_shift_down = key_state.shift_status;
    bool prev_caps_lock = key_state.caps_lock_status;

    bool break_code;
    uint16_t scancode = inb(KEYBOARD_BUF_PORT);

    if (scancode == SCANCODE_EXT) {
        key_state.ext_scancode = true;
        return;
    }

    if(key_state.ext_scancode){
        scancode = ((0xe000) | scancode);
        key_state.ext_scancode = false;
    }

    // get break code
    break_code = ((scancode & 0x0080) != 0); 

    if (break_code) {
        // Extract the make code by masking the break code bit
        uint16_t make_code = (scancode &= 0xff7f);

        // Update the status of control keys if they are released
        if (make_code == CTRL_L_MAKE || make_code == CTRL_R_MAKE) {
            key_state.ctrl_status = false;
        } else if (make_code == SHIFT_L_MAKE || make_code == SHIFT_R_MAKE) {
            key_state.shift_status = false;
        } else if (make_code == ALT_L_MAKE || make_code == ALT_R_MAKE) {
            key_state.alt_status = false;
        }

        // Caps Lock does not toggle on release, so no update is needed here
        return;
    }

    // Handle make codes for defined keys
    if ((scancode > 0x00 && scancode < 0x3b) || (scancode == ALT_R_MAKE) ||
        (scancode == CTRL_R_MAKE)) {

        bool shift =
            false; // Used to index characters from keymap based on Shift status

        // Special key ranges that require Shift to access additional characters
        if ((scancode < 0x0e) || (scancode == 0x29) || (scancode == 0x1a) ||
            (scancode == 0x1b) || (scancode == 0x2b) || (scancode == 0x27) ||
            (scancode == 0x28) || (scancode == 0x33) || (scancode == 0x34) ||
            (scancode == 0x35)) {

            if (prev_shift_down) {
                shift = true;
            }
        } else {
            // For alphabetic keys, Shift and Caps Lock together cancel each
            // other
            if (prev_shift_down && prev_caps_lock) {
                shift = false;
            } else if (prev_shift_down || prev_caps_lock) {
                shift = true;
            } else {
                shift = false;
            }
        }

        // Mask the high byte for extended scancodes
        scancode &= 0x00ff;
        char cur_char =
            keymap[scancode][shift]; // Retrieve the character from the keymap

        // Add the character to the buffer if it is not null
        if (cur_char) {
            if (!ioq_full(&keyboard_ringbuffer)){
                // working as temp, test producer
                // __ccos_putchar(cur_char);
                ioq_putchar(&keyboard_ringbuffer, cur_char); 
            }
           return;
        }

        // Update the status of control keys for the next input
        if (scancode == CTRL_L_MAKE || scancode == CTRL_R_MAKE) {
            key_state.ctrl_status = true;
        } else if (scancode == SHIFT_L_MAKE || scancode == SHIFT_R_MAKE) {
            key_state.shift_status = true;
        } else if (scancode == ALT_L_MAKE || scancode == ALT_R_MAKE) {
            key_state.alt_status = true;
        } else if (scancode == CAPS_LOCK_MAKE) {
            // Toggle Caps Lock status on each press
            key_state.caps_lock_status = !key_state.caps_lock_status;
        }
    } else {
        ccos_puts("unknown key\n"); // Handle undefined keys
    }    

}

// init the subsystem of keyboard 
void init_basic_input_subsystem(void)
{
    ccos_puts("initing subsystem of input: from keyboard!\n");
    init_IOQueue(&keyboard_ringbuffer);
    register_intr_handler(KEYBOARD_INTERRUPT_N, keyboard_intr_handler);
    ccos_puts("init subsystem of input: from keyboard done!\n");
}