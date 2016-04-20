/*
*   fc.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef __fs_h__
#define __fs_h__

#include "types.h"
#include "fatfs/ff.h"

u8 mountSD(void);
u8 unmountSD(void);
void fopen(const char *filename, const char *mode);
void fclose(void);
void fseek(u32 offset);
u8 eof(void);
Size fsize(void);
Size fwrite(const char *buffer, Size elementSize, Size elementCnt);
Size fread(const char *buffer, Size elementSize, Size elementCnt);

#endif
