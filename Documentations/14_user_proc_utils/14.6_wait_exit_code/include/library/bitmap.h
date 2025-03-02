#ifndef BITMAP_H
#define BITMAP_H
#include "include/library/types.h"

// Define the mask value used for bitmap operations
#define BITMAP_MASK 1

// Bitmap structure definition
// btmp_bytes_len: the total length in bytes of the bitmap
// bits: pointer to the actual bitmap array (a sequence of bits)
typedef struct {
    uint32_t btmp_bytes_len; // Length in bytes of the bitmap
    uint8_t *bits;           // Pointer to the bitmap array
} Bitmap;

// Function to initialize a bitmap structure
// Initializes the bitmap by setting all bits to 0 (cleared state).
void bitmap_init(Bitmap *btmp);

// Function to test the status of a specific bit in the bitmap
// btmp: the bitmap object
// bit_idx: the index of the bit to check
// Returns: true if the bit at bit_idx is set (1), false otherwise
bool bitmap_scan_test(Bitmap *btmp, uint32_t bit_idx);

// Function to find a continuous block of bits in the bitmap
// btmp: the bitmap object
// cnt: the number of consecutive bits required
// Returns: the index of the first bit in the block, or -1 if no block is found
int bitmap_scan(Bitmap *btmp, uint32_t cnt);

// Function to set the value of a specific bit in the bitmap
// btmp: the bitmap object
// bit_idx: the index of the bit to set
// value: the value to set the bit to (0 or 1)
void bitmap_set(Bitmap *btmp, uint32_t bit_idx, int8_t value);

#endif
