#ifndef BITMAP_H
#define BITMAP_H
#include "include/library/types.h"

#define BITMAP_MASK 1
typedef struct {
   uint32_t btmp_bytes_len;
/* 在遍历位图时,整体上以字节为单位,细节上是以位为单位,所以此处位图的指针必须是单字节 */
   uint8_t* bits;
}Bitmap;

void bitmap_init(Bitmap* btmp);
bool bitmap_scan_test(Bitmap* btmp, uint32_t bit_idx);
int bitmap_scan(Bitmap* btmp, uint32_t cnt);
void bitmap_set(Bitmap* btmp, uint32_t bit_idx, int8_t value);


#endif