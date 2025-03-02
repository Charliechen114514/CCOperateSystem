#include "include/device/keyboard.h"
#include "include/device/configs/keyboard_ascii.h"
#include "include/device/configs/keyboard_mappings.h"
#include "include/io/io.h"
#include "include/io/ioqueue.h"
#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/library/types.h"

IOQueue keyboard_ringbuffer; // Defines the keyboard buffer to store keyboard
                             // input data

/* Defines a structure to record the status of special keys */
static struct {
    bool ctrl_status;      // Indicates whether the Ctrl key is pressed
    bool shift_status;     // Indicates whether the Shift key is pressed
    bool alt_status;       // Indicates whether the Alt key is pressed
    bool caps_lock_status; // Indicates whether Caps Lock is active
    bool ext_scancode;     // Indicates if the scancode starts with 0xe0
} key_state;

/* Keyboard interrupt handler */
static void intr_keyboard_handler(void) {
    // Save the status of control keys before the current interrupt
    bool ctrl_down_last = key_state.ctrl_status;
    bool shift_down_last = key_state.shift_status;
    bool caps_lock_last = key_state.caps_lock_status;

    bool break_code;
    uint16_t scancode =
        inb(KEYBOARD_BUF_PORT); // Read the scancode from the keyboard port

    // Handle scancodes that start with 0xe0, indicating extended scancodes
    if (scancode == 0xe0) {
        key_state.ext_scancode = true; // Mark the start of an extended scancode
        return;
    }

    // Merge the scancode if the previous one started with 0xe0
    if (key_state.ext_scancode) {
        scancode = ((0xe000) | scancode);
        key_state.ext_scancode = false; // Reset the extended scancode flag
    }

    // Check if it is a break code (key release)
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

            if (shift_down_last) {
                shift = true;
            }
        } else {
            // For alphabetic keys, Shift and Caps Lock together cancel each
            // other
            if (shift_down_last && caps_lock_last) {
                shift = false;
            } else if (shift_down_last || caps_lock_last) {
                shift = true;
            } else {
                shift = false;
            }
        }

        // Mask the high byte for extended scancodes
        uint8_t index = (scancode &= 0x00ff);
        char cur_char =
            keymap[index][shift]; // Retrieve the character from the keymap

        // Add the character to the buffer if it is not null
        if (cur_char) {

            // Handle shortcut keys Ctrl+L and Ctrl+U
            if ((ctrl_down_last && cur_char == 'l') ||
                (ctrl_down_last && cur_char == 'u')) {
                cur_char -= 'a'; // Map to non-printable ASCII values
            }

            // Add the character to the keyboard buffer if it's not full
            if (!ioq_full(&keyboard_ringbuffer)) {
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

/* Initialize the keyboard */
void keyboard_init(void) {
    verbose_write("keyboard init start\n");
    init_IOQueue(&keyboard_ringbuffer); // Initialize the keyboard ring buffer
    register_intr_handler(
        0x21, intr_keyboard_handler); // Register the keyboard interrupt handler
    verbose_write("keyboard init done\n");
}
