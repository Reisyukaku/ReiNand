/*
*   memory.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/
#include "memory.h"

void *memsearch(void *startPos, void *pattern, u32 searchSize, u32 patternSize){
    if(!searchSize) return NULL;
    for (void *pos = startPos + searchSize - patternSize; pos >= startPos; pos--) {
        if (memcmp(pos, pattern, patternSize) == 0) return pos;
    }
    return NULL;
}