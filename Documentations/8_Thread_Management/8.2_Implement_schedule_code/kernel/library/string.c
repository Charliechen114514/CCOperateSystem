#include "include/library/kernel_assert.h"
#include "include/library/string.h"

// Set 'k_size' bytes of memory at 'k_dst' to the value 'k_value'.
void k_memset(void *k_dst, const uint8_t k_value, const uint32_t k_size)
{
    KERNEL_ASSERT(k_dst);
    uint8_t *destinations = (uint8_t *)k_dst;
    uint32_t index = k_size;
    while (index-- > 0)
    {
        *destinations++ = k_value;
    }
}

// Copy 'k_size' bytes of memory from 'k_src' to 'k_dst'.
void k_memcpy(void *k_dst, const void *k_src, uint32_t k_size)
{
    KERNEL_ASSERT(k_dst && k_src);
    uint8_t *dstinations = k_dst;
    const uint8_t *source = k_src;
    while (k_size-- > 0)
    {
        *dstinations++ = *source++;
    }
}

// Compare 'k_size' bytes of memory between 'k_a' and 'k_b'.
// Returns 0 if they are equal, a negative value if k_a < k_b, or a positive value if k_a > k_b.
int k_memcmp(const void *k_a, const void *k_b, uint32_t k_size)
{
    const char *a = k_a;
    const char *b = k_b;
    KERNEL_ASSERT(a || b);
    while (k_size-- > 0)
    {
        if (*a != *b)
        {
            return *a > *b ? 1 : -1;
        }
        a++;
        b++;
    }
    return 0;
}

// Copy the null-terminated string from 'k_src' to 'k_dst'.
// Returns a pointer to the destination string 'k_dst'.
char *k_strcpy(char *k_dst, const char *k_src)
{
    KERNEL_ASSERT(k_dst && k_src);
    char *r = k_dst;
    while ((*k_dst++ = *k_src++))
        ;
    return r;
}

// Calculate and return the length of the null-terminated string 'k_str'.
// Excludes the null-terminator.
uint32_t k_strlen(const char *k_str)
{
    KERNEL_ASSERT(k_str);
    const char *p = k_str;
    while (*p++)
        ;
    return (p - k_str - 1);
}

// Compare two null-terminated strings 'k_a' and 'k_b'.
// Returns 0 if they are equal, a negative value if k_a < k_b, or a positive value if k_a > k_b.
int8_t k_strcmp(const char *k_a, const char *k_b)
{
    KERNEL_ASSERT(k_a && k_b);
    while (*k_a != 0 && *k_a == *k_b)
    {
        k_a++;
        k_b++;
    }
    return *k_a < *k_b ? -1 : *k_a > *k_b;
}

// Find the first occurrence of 'k_ch' in the string 'k_string'.
// Returns a pointer to the first occurrence of 'k_ch', or NULL if not found.
char *k_strchr(const char *k_string, const uint8_t k_ch)
{
    KERNEL_ASSERT(k_string);
    while (*k_string != 0)
    {
        if (*k_string == k_ch)
        {
            return (char *)k_string;
        }
        k_string++;
    }
    return NULL;
}

// Find the last occurrence of 'k_ch' in the string 'k_string'.
// Returns a pointer to the last occurrence of 'k_ch', or NULL if not found.
char *k_strrchr(const char *k_string, const uint8_t k_ch)
{
    KERNEL_ASSERT(k_string);
    const char *last_char = NULL;
    while (*k_string != 0)
    {
        if (*k_string == k_ch)
        {
            last_char = k_string;
        }
        k_string++;
    }
    return (char *)last_char;
}

// Concatenate the null-terminated string 'k_src' to the end of 'k_dst'.
// Returns a pointer to the resulting string 'k_dst'.
char *k_strcat(char *k_dst, const char *k_src)
{
    KERNEL_ASSERT(k_dst && k_src);
    char *str = k_dst;
    while (*str++)
        ; // until we find the end
    --str;
    while ((*str++ = *k_src++))
        ;
    return k_dst;
}

// Count the number of occurrences of 'k_ch' in the string 'k_filename'.
// Returns the count of occurrences.
uint32_t k_strchrs(const char *k_filename, uint8_t k_ch)
{
    KERNEL_ASSERT(k_filename);
    uint32_t ch_cnt = 0;
    const char *p = k_filename;
    while (*p != 0)
    {
        if (*p == k_ch)
        {
            ch_cnt++;
        }
        p++;
    }
    return ch_cnt;
}
