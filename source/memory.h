/*
*   memory.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef MEM_INC
#define MEM_INC

#include <string.h>
#include "types.h"

uPtr memsearch(const void *startPos, const void *pattern, u32 searchSize, u32 patternSize);

#endif