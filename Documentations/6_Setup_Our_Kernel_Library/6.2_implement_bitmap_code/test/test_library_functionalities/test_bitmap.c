#include "include/library/bitmap.h"
#include "test/include/test.h"


#define NORMAL_BIT_MAP_LEN      (10)
#define BOUNDARY_BIT_MAP_LEN    (1)

void test_bitmap_init(void) {
    Bitmap btmp;
    uint8_t bits[NORMAL_BIT_MAP_LEN];
    btmp.btmp_bytes_len = NORMAL_BIT_MAP_LEN; // Bitmap has 10 bytes

    btmp.bits = bits; // Allocate memory for the bitmap

    bitmap_init(&btmp);

    // Check if each byte is 0
    for (uint32_t i = 0; i < btmp.btmp_bytes_len; i++) {
        TEST_ASSERT(btmp.bits[i] == 0); // All bits should be initialized to 0
    }
}

void test_bitmap_scan_test(void) {
    Bitmap btmp;
    uint8_t bits[NORMAL_BIT_MAP_LEN];
    btmp.btmp_bytes_len = NORMAL_BIT_MAP_LEN; // Bitmap has 10 bytes
    bitmap_init(&btmp);

    // Set a specific bit to 1
    bitmap_set(&btmp, 5, 1);

    // Test if the bit at index 5 is 1
    TEST_ASSERT(bitmap_scan_test(&btmp, 5) == true); // The 5th bit should be 1

    // Test other bits to ensure they are 0
    TEST_ASSERT(bitmap_scan_test(&btmp, 4) == false); // The 4th bit should be 0
}

void test_bitmap_scan(void) {
    Bitmap btmp;
    btmp.btmp_bytes_len = NORMAL_BIT_MAP_LEN; // Bitmap has 10 bytes
    uint8_t bits[NORMAL_BIT_MAP_LEN];
    bitmap_init(&btmp);

    // Set some bits
    bitmap_set(&btmp, 3, 1); // Set the 3rd bit to 1
    bitmap_set(&btmp, 5, 1); // Set the 5th bit to 1

    // Test if it can find 1 consecutive free bit
    int result = bitmap_scan(&btmp, 1);
    TEST_ASSERT(result == 0); // From position 0, there should be a free bit from 0 to 1

    // Test if it can find 2 consecutive free bits
    result = bitmap_scan(&btmp, 2);
    TEST_ASSERT(result == 0); // From position 2, there should be a free bit from 2 to 3

    // Test if it can find 4 consecutive free bits
    result = bitmap_scan(&btmp, 4);
    TEST_ASSERT(result == 6); // From position 7, there should be a free bit from 7 to 9
}

void test_bitmap_set(void) {
    Bitmap btmp;
    uint8_t bits[NORMAL_BIT_MAP_LEN];
    btmp.btmp_bytes_len = NORMAL_BIT_MAP_LEN; // Bitmap has 10 bytes
    bitmap_init(&btmp);

    // Set some bits
    bitmap_set(&btmp, 0, 1); // Set the 0th bit to 1
    bitmap_set(&btmp, 3, 1); // Set the 3rd bit to 1
    bitmap_set(&btmp, 5, 1); // Set the 5th bit to 1
    bitmap_set(&btmp, 7, 1); // Set the 7th bit to 1

    // Check if the bits are correctly set
    TEST_ASSERT(bitmap_scan_test(&btmp, 0) == true); // The 0th bit should be 1
    TEST_ASSERT(bitmap_scan_test(&btmp, 3) == true); // The 3rd bit should be 1
    TEST_ASSERT(bitmap_scan_test(&btmp, 5) == true); // The 5th bit should be 1
    TEST_ASSERT(bitmap_scan_test(&btmp, 7) == true); // The 7th bit should be 1
    TEST_ASSERT(bitmap_scan_test(&btmp, 1) == false); // The 1st bit should be 0

    // Set the 7th bit to 0
    bitmap_set(&btmp, 7, 0);
    TEST_ASSERT(bitmap_scan_test(&btmp, 7) == false); // The 7th bit should now be 0
}

void run_bitmap_test(void) {
    test_bitmap_init();
    TEST_PRINT("Bitmap Init Test Down");
    test_bitmap_scan_test();
    TEST_PRINT("Bitmap Scan test Test Down");
    test_bitmap_scan();
    TEST_PRINT("Bitmap Scan Test Down");
    test_bitmap_set();
    TEST_PRINT("Bitmap Set Test Down");
}
