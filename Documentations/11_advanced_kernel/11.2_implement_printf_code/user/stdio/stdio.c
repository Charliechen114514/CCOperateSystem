#include "include/user/stdio/stdio.h"
#include "include/kernel/interrupt.h"
#include "include/library/ccos_print.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/user/syscall/syscall.h"

#define va_start(ap, v) ap = (va_list) & v // the start of the vargs
#define va_arg(ap, t) *((t *)(ap += 4))    // let next
#define va_end(ap) ap = NULL               // clear varg

// Converts an unsigned integer to a string representation in the specified base
static void itoa(uint32_t number, char **buffer_address, uint8_t base) {
    uint32_t remainder = number % base;  // Get the least significant digit
    uint32_t quotient = number / base;   // Get the remaining part of the number

    // Recursively process the higher digits
    if (quotient) {
        itoa(quotient, buffer_address, base);
    }

    // Convert remainder to a character and store it in the buffer
    if (remainder < 10) {
        *((*buffer_address)++) = remainder + '0';  // Convert 0-9 to '0'-'9'
    } else {
        *((*buffer_address)++) = remainder - 10 + 'A';  // Convert 10-15 to 'A'-'F' (for base > 10)
    }
}
// Formats a string and stores the result in the provided buffer
uint32_t vsprintf(char* output_buffer, const char* format_string, va_list args) { 
    char* buffer_ptr = output_buffer; 
    const char* format_ptr = format_string; 
    char current_char = *format_ptr; 
    int32_t argument_integer; 

    while (current_char) { 
        if (current_char != '%') { 
            *(buffer_ptr++) = current_char; 
            current_char = *(++format_ptr); 
            continue; 
        } 

        current_char = *(++format_ptr);  // Get the character after '%'

        switch (current_char) { 
            case 'x': 
                argument_integer = va_arg(args, int); 
                itoa(argument_integer, &buffer_ptr, 16); 
                current_char = *(++format_ptr);  // Skip format specifier and update current_char 
                break; 
        } 
    } 

    return k_strlen(output_buffer); 
} 

// Formats and prints a string based on a format specifier
uint32_t printf(const char* format_string, ...) { 
    va_list args; 
    va_start(args, format_string); // Initialize args to store all additional arguments
    char buffer[PRINTF_IO_BUFFER_LENGTH] = {0};       // Buffer to hold the formatted string
    vsprintf(buffer, format_string, args); 
    va_end(args); 
    return write(buffer); 
} 
