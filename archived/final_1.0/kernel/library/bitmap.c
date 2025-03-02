#include "include/library/bitmap.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"

/* Initialize the bitmap btmp */
void bitmap_init(Bitmap *btmp) {
    memset(btmp->bits, 0,
           btmp->btmp_bytes_len); // Set all bits to 0 (clear the bitmap)
}

/* Check if the bit at bit_idx is set to 1. Return the bytes if it is, otherwise
 * false */
bool bitmap_scan_test(Bitmap *btmp, uint32_t bit_idx) {
    uint32_t byte_idx = bit_idx / 8; // Integer division to get the byte index
    uint32_t bit_odd =
        bit_idx % 8; // Remainder gives the bit position within the byte
    return !!(btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd)); // Test the bit
}

/* Find cnt consecutive free bits in the bitmap. Return the starting bit index
 */
int bitmap_scan(Bitmap *btmp, uint32_t cnt) {
    uint32_t idx_byte = 0; // To track the byte where free bits are found

    /* First, scan byte by byte using a brute-force approach */
    while ((0xff == btmp->bits[idx_byte]) &&
           (idx_byte < btmp->btmp_bytes_len)) {
        /* 0xff means all bits in this byte are set (1), so move to the next
         * byte */
        idx_byte++;
    }

    ASSERT(idx_byte < btmp->btmp_bytes_len);
    if (idx_byte == btmp->btmp_bytes_len) { // If no free space is found in the
                                            // entire bitmap
        return -1;
    }

    /* Once a byte with free bits is found, scan bit by bit within this byte */
    int idx_bit = 0;
    while ((uint8_t)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]) {
        idx_bit++;
    }

    int bit_idx_start =
        idx_byte * 8 + idx_bit; // Starting index of the free bit
    if (cnt == 1) {
        return bit_idx_start; // If only 1 free bit is requested, return
                              // immediately
    }

    uint32_t bit_left =
        (btmp->btmp_bytes_len * 8 - bit_idx_start); // Bits left to check
    uint32_t next_bit = bit_idx_start + 1;
    uint32_t count = 1; // Count of consecutive free bits found

    bit_idx_start =
        -1; // Initialize to -1; if no consecutive bits are found, return -1
    while (bit_left-- > 0) {
        if (!(bitmap_scan_test(btmp,
                               next_bit))) { // If the next bit is free (0)
            count++;
        } else {
            count = 0; // Reset count if a used bit (1) is encountered
        }
        if (count ==
            cnt) { // If the required number of consecutive free bits is found
            bit_idx_start = next_bit - cnt + 1;
            break;
        }
        next_bit++;
    }
    return bit_idx_start;
}

/* Set the bit at bit_idx to the given value (0 or 1) */
void bitmap_set(Bitmap *btmp, uint32_t bit_idx, int8_t value) {
    ASSERT((value == 0) || (value == 1)); // Assert that value is either 0 or 1
    uint32_t byte_idx = bit_idx / 8;      // Get the byte index
    uint32_t bit_odd = bit_idx % 8; // Get the bit position within the byte

    /* Use bitwise operations to set or clear the bit */
    if (value) {                                           // If value is 1
        btmp->bits[byte_idx] |= (BITMAP_MASK << bit_odd);  // Set the bit to 1
    } else {                                               // If value is 0
        btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_odd); // Clear the bit to 0
    }
}
