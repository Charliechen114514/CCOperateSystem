#include "include/library/string.h"
#include "test/include/test.h"

static void test_k_memset(void)
{
    uint8_t buf[10];

    // Normal case: Set memory to a specific value
    k_memset(buf, 0xAA, 10);
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT(buf[i] == 0xAA);
    }

    // Edge case: Set 0 bytes of memory
    k_memset(buf, 0x00, 0);
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT(buf[i] == 0xAA); // buf should not be changed
    }
}

static void test_k_memcpy(void)
{
    uint8_t src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t dst[10] = {0};

    // Normal case: Copy data from source to destination
    k_memcpy(dst, src, 10);
    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT(dst[i] == src[i]);
    }

    // Edge case: Source and destination are the same
    k_memcpy(src, src, 10);
    TEST_ASSERT(src[0] == 1); // Data should remain unchanged

    // Exception case: Copy 0 bytes
    k_memcpy(dst, src, 0);
    TEST_ASSERT(dst[0] == 1); // Data should not be changed
}

static void test_k_memcmp(void)
{
    uint8_t a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t b[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t c[10] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

    // Normal case: Memory is identical
    TEST_ASSERT(k_memcmp(a, b, 10) == 0);

    // Special case: Memory is different
    TEST_ASSERT(k_memcmp(a, c, 10) < 0);

    // Edge case: Empty memory
    TEST_ASSERT(k_memcmp(a, c, 0) == 0);
}

static void test_k_strcpy(void)
{
    char src[] = "Hello, World!";
    char dst[20] = {0};

    // Normal case: Copy string
    TEST_ASSERT(k_strcpy(dst, src) == dst);
    TEST_ASSERT(k_strcmp(dst, src) == 0);

    // Edge case: Copy empty string
    TEST_ASSERT(k_strcpy(dst, "") == dst);
    TEST_ASSERT(k_strcmp(dst, "") == 0);
}

static void test_k_strlen(void)
{
    const char *str1 = "Hello";
    const char *str2 = "";

    // Normal case: Non-empty string
    TEST_ASSERT(k_strlen(str1) == 5);

    // Edge case: Empty string
    TEST_ASSERT(k_strlen(str2) == 0);
}

static void test_k_strcmp(void)
{
    const char *str1 = "Hello";
    const char *str2 = "World";
    const char *str3 = "Hello";

    // Normal case: Different strings
    TEST_ASSERT(k_strcmp(str1, str2) != 0);

    // Edge case: Same strings
    TEST_ASSERT(k_strcmp(str1, str3) == 0);
}

static void test_k_strchr(void)
{
    const char *str = "Hello, World!";

    // Normal case: Character is found
    TEST_ASSERT(k_strchr(str, 'o') == &str[4]);

    // Edge case: Character is not found
    TEST_ASSERT(k_strchr(str, 'x') == NULL);
}

static void test_k_strrchr(void)
{
    const char *str = "Hello, World!";

    // Normal case: Character is found
    TEST_ASSERT(k_strrchr(str, 'o') == &str[8]);

    // Edge case: Character is not found
    TEST_ASSERT(k_strrchr(str, 'x') == NULL);
}

static void test_k_strcat(void)
{
    char str1[20] = "Hello";
    const char *str2 = " World!";

    // Normal case: Concatenate strings
    TEST_ASSERT(k_strcat(str1, str2) == str1);
    TEST_ASSERT(k_strcmp(str1, "Hello World!") == 0);

    // Edge case: Concatenate empty string
    TEST_ASSERT(k_strcat(str1, "") == str1);
}

static void test_k_strchrs(void)
{
    const char *str = "Hello, World!";

    // Normal case: Count occurrences of a character
    TEST_ASSERT(k_strchrs(str, 'o') == 2);

    // Edge case: Character is not found
    TEST_ASSERT(k_strchrs(str, 'x') == 0);
}

void run_strings_test(void)
{
    // Call each test function one by one
    TEST_PRINT("String func tests starts");
    test_k_memset();
    TEST_PRINT("Test test_k_memset done!");
    test_k_memcpy();
    TEST_PRINT("Test test_k_memcpy done!");
    test_k_memcmp();
    TEST_PRINT("Test test_k_memcmp done!");
    test_k_strcpy();
    TEST_PRINT("Test test_k_strcpy done!");
    test_k_strlen();
    TEST_PRINT("Test test_k_strlen done!");
    test_k_strcmp();
    TEST_PRINT("Test test_k_strcmp done!");
    test_k_strchr();
    TEST_PRINT("Test test_k_strchr done!");
    test_k_strrchr();
    TEST_PRINT("Test test_k_strrchr done!");
    test_k_strcat();
    TEST_PRINT("Test test_k_strcat done!");
    test_k_strchrs();
    TEST_PRINT("Test test_k_strchrs done!");
}
