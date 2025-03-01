#ifndef K_STRING_H
#define K_STRING_H

#include "include/library/types.h"
/* Kernel operations string op */
// Set 'k_size' bytes of memory at 'k_dst' to the value 'k_value'.
void k_memset(void *k_dst, const uint8_t k_value, const uint32_t k_size);

// Copy 'k_size' bytes of memory from 'k_src' to 'k_dst'.
void k_memcpy(void *k_dst, const void *k_src, uint32_t k_size);

// Compare 'k_size' bytes of memory between 'k_a' and 'k_b'.
// Returns 0 if they are equal, a negative value if k_a < k_b, or a positive value if k_a > k_b.
int k_memcmp(const void *k_a, const void *k_b, uint32_t k_size);

// Copy the null-terminated string from 'k_src' to 'k_dst'.
// Returns a pointer to the destination string 'k_dst'.
char *k_strcpy(char *k_dst, const char *k_src);

// Calculate and return the length of the null-terminated string 'k_str'.
// Excludes the null-terminator.
uint32_t k_strlen(const char *k_str);

// Compare two null-terminated strings 'k_a' and 'k_b'.
// Returns 0 if they are equal, a negative value if k_a < k_b, or a positive value if k_a > k_b.
int8_t k_strcmp(const char *k_a, const char *k_b);

// Find the first occurrence of 'k_ch' in the string 'k_string'.
// Returns a pointer to the first occurrence of 'k_ch', or NULL if not found.
char *k_strchr(const char *k_string, const uint8_t k_ch);

// Find the last occurrence of 'k_ch' in the string 'k_string'.
// Returns a pointer to the last occurrence of 'k_ch', or NULL if not found.
char *k_strrchr(const char *k_string, const uint8_t k_ch);

// Concatenate the null-terminated string 'k_src' to the end of 'k_dst'.
// Returns a pointer to the resulting string 'k_dst'.
char *k_strcat(char *k_dst, const char *k_src);

// Count the number of occurrences of 'k_ch' in the string 'k_filename'.
// Returns the count of occurrences.
uint32_t k_strchrs(const char *k_filename, uint8_t k_ch);

#endif