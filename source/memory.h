/*
*   memory.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/
#ifndef MEM_INC
#define MEM_INC

#include "types.h"

void memcpy(void *dest, const void *src, u32 size);
void memcpy32(u32 *dest, u32 *src, u32 size);
void memset(void *dest, int filler, u32 size);
int memcmp(const void *buf1, const void *buf2, u32 size);
void *memsearch(void *startPos, void *pattern, u32 searchSize, u32 patternSize);

#endif