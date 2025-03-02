#include "include/library/string.h"
// #include "include/library/types.h"
#include "include/library/kernel_assert.h"

/* Set 'size' number of bytes of memory starting at 'dst_' to the value 'value'
 */
void k_memset(void *dst_, uint8_t value, uint32_t size) {
    KERNEL_ASSERT(dst_ != NULL); // Assert that the destination is not NULL
    uint8_t *dst =
        (uint8_t *)dst_; // Cast to uint8_t pointer for byte-wise manipulation
    while (size-- > 0)   // Loop until all bytes are set
        *dst++ = value;  // Set each byte to 'value'
}

/* Copy 'size' number of bytes from 'src_' to 'dst_' */
void k_memcpy(void *dst_, const void *src_, uint32_t size) {
    KERNEL_ASSERT(dst_ != NULL &&
           src_ !=
               NULL);    // Assert that both source and destination are not NULL
    uint8_t *dst = dst_; // Cast destination to byte pointer
    const uint8_t *src = src_; // Cast source to byte pointer
    while (size-- > 0)         // Loop until all bytes are copied
        *dst++ = *src++;       // Copy each byte from source to destination
}

/* Compare 'size' number of bytes from 'a_' and 'b_'.
   Return 0 if equal, 1 if 'a_' > 'b_', -1 if 'a_' < 'b_' */
int k_memcmp(const void *a_, const void *b_, uint32_t size) {
    const char *a = a_; // Cast a_ to char pointer for byte-wise comparison
    const char *b = b_; // Cast b_ to char pointer
    KERNEL_ASSERT(a != NULL || b != NULL); // Assert at least one is not NULL
    while (size-- > 0) {
        if (*a != *b) {
            return *a > *b
                       ? 1
                       : -1; // Return 1 if 'a' is greater, -1 if 'b' is greater
        }
        a++;
        b++;
    }
    return 0; // Return 0 if all bytes are equal
}

/* Copy the string from 'src_' to 'dst_' */
char *k_strcpy(char *dst_, const char *src_) {
    KERNEL_ASSERT(dst_ != NULL &&
           src_ !=
               NULL); // Assert that both source and destination are not NULL
    char *r = dst_;   // Save the starting address of the destination string
    while ((*dst_++ = *src_++))
        ;     // Copy each character from source to destination
    return r; // Return the destination string
}

/* Return the length of the string */
uint32_t k_strlen(const char *str) {
    KERNEL_ASSERT(str != NULL); // Assert that the string is not NULL
    const char *p = str;
    while (*p++)
        ;                 // Loop through the string until the null-terminator
    return (p - str - 1); // Return the length excluding the null-terminator
}

/* Compare two strings. Return 1 if 'a_' > 'b_', 0 if equal, -1 if 'a_' < 'b_'
 */
int8_t k_strcmp(const char *a, const char *b) {
    KERNEL_ASSERT(a != NULL && b != NULL); // Assert that both strings are not NULL
    while (*a != 0 && *a == *b) {   // Loop until a mismatch or end of string
        a++;
        b++;
    }
    return *a < *b ? -1 : *a > *b; // Return -1, 0, or 1 based on the comparison
}

/* Find the first occurrence of character 'ch' in the string 'str' and return
 * its address */
char *k_strchr(const char *str, const uint8_t ch) {
    KERNEL_ASSERT(str != NULL); // Assert that the string is not NULL
    while (*str != 0) {  // Loop through the string until the null-terminator
        if (*str == ch) {
            return (char *)str; // Return the address of the found character
        }
        str++;
    }
    return NULL; // Return NULL if the character is not found
}

/* Find the last occurrence of character 'ch' in the string 'str' and return its
 * address */
char *k_strrchr(const char *str, const uint8_t ch) {
    KERNEL_ASSERT(str != NULL); // Assert that the string is not NULL
    const char *last_char = NULL;
    while (*str != 0) { // Loop through the string
        if (*str == ch) {
            last_char = str; // Update last_char with the address of each
                             // occurrence of 'ch'
        }
        str++;
    }
    return (
        char *)last_char; // Return the address of the last occurrence of 'ch'
}

/* Concatenate the string 'src_' to the end of string 'dst_' and return the
 * concatenated string */
char *k_strcat(char *dst_, const char *src_) {
    KERNEL_ASSERT(dst_ != NULL &&
           src_ !=
               NULL); // Assert that both source and destination are not NULL
    char *str = dst_;
    while (*str++)
        ;  // Find the end of the destination string
    --str; // Move back one step to the null-terminator
    while ((*str++ = *src_++))
        ;        // Copy each character from source to destination
    return dst_; // Return the concatenated string
}

/* Count the occurrences of character 'ch' in the string 'str' */
uint32_t k_strchrs(const char *str, uint8_t ch) {
    KERNEL_ASSERT(str != NULL); // Assert that the string is not NULL
    uint32_t ch_cnt = 0;
    const char *p = str;
    while (*p != 0) { // Loop through the string
        if (*p == ch) {
            ch_cnt++; // Increment the count for each occurrence of 'ch'
        }
        p++;
    }
    return ch_cnt; // Return the total count of 'ch' in the string
}
