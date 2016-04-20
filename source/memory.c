/*
*   memory.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/
#include "memory.h"

void memcpy32(u32 *dest, u32 *src, u32 size){
    for (u32 i = 0; i < size; i++) dest[i] = src[i];
}

void memcpy(void *dest, const void *src, u32 size){
    u8 *destc = (u8 *)dest;
    const u8 *srcc = (const u8 *)src;
    u32 i; for (i = 0; i < size; i++) {
        destc[i] = srcc[i];
    }
}

void memset(void *dest, int filler, u32 size){
    u8 *destc = (u8 *)dest;
    u32 i; for (i = 0; i < size; i++) {
        destc[i] = filler;
    }
}

int memcmp(const void *buf1, const void *buf2, u32 size){
    const u8 *buf1c = (const u8 *)buf1;
    const u8 *buf2c = (const u8 *)buf2;
    u32 i; for (i = 0; i < size; i++) {
        int cmp = buf1c[i] - buf2c[i];
        if (cmp) return cmp;
    }
    return 0;
}

void *memsearch(void *startPos, void *pattern, u32 searchSize, u32 patternSize){
    if(!searchSize) return NULL;
    for (void *pos = startPos + searchSize - patternSize; pos >= startPos; pos--) {
        if (memcmp(pos, pattern, patternSize) == 0) return pos;
    }
    return NULL;
}