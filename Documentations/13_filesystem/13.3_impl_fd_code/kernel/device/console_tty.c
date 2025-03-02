#include "include/device/console_tty.h"
#include "include/kernel/lock.h"
#include "include/thread/thread.h"
#include "include/library/ccos_print.h"
#include "include/library/types.h"

// Static locker for console synchronization
// this is acceptable, for we only have one actually console
static CCLocker console_lock; // Lock for controlling access to the console

/**
 * Initializes the console.
 * This function sets up the console lock to ensure thread-safe operations.
 */
void console_init() {
    lock_init(&console_lock); // Initialize the console lock
}

/**
 * Acquires the console lock.
 * This function ensures exclusive access to the console by acquiring the lock.
 * If the lock is already held by another thread, the current thread will block
 * until the lock is released.
 */
void console_acquire() {
    lock_acquire(&console_lock); // Acquire the console lock
}

/**
 * Releases the console lock.
 * This function releases the lock, allowing other threads to acquire it and
 * access the console.
 */
void console_release() {
    lock_release(&console_lock); // Release the console lock
}

/**
 * Prints a string to the console.
 * This function ensures thread-safe printing by acquiring the console lock
 * before calling the underlying `ccos_puts` function and releasing the lock
 * afterward.
 *
 * @param str The string to be printed to the console.
 */
void console_ccos_puts(char *str) {
    console_acquire(); // Acquire the console lock
    ccos_puts(str);    // Print the string to the console
    console_release(); // Release the console lock
}

/**
 * Prints a character to the console.
 * This function ensures thread-safe printing by acquiring the console lock
 * before calling the underlying `__ccos_putchar` function and releasing the
 * lock afterward.
 *
 * @param char_asci The ASCII value of the character to be printed.
 */
void console__ccos_putchar(uint8_t char_asci) {
    console_acquire();         // Acquire the console lock
    __ccos_putchar(char_asci); // Print the character to the console
    console_release();         // Release the console lock
}

/**
 * Prints a 32-bit integer in hexadecimal format to the console.
 * This function ensures thread-safe printing by acquiring the console lock
 * before calling the underlying `__ccos_display_int` function and releasing the
 * lock afterward.
 *
 * @param num The 32-bit integer to be printed in hexadecimal format.
 */
void console__ccos_display_int(uint32_t num) {
    console_acquire();       // Acquire the console lock
    __ccos_display_int(num); // Print the integer in hexadecimal format
    console_release();       // Release the console lock
}