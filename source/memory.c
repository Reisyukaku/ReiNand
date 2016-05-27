/*
*   memory.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/
#include "memory.h"

uPtr memsearch(const void *startPos, const void *pattern, u32 searchSize, u32 patternSize){
    if(!searchSize) return 0;
    for (void *pos = startPos + searchSize - patternSize; pos >= startPos; pos--) {
        if (memcmp(pos, pattern, patternSize) == 0) return (uPtr)pos;
    }
    return 0;
}