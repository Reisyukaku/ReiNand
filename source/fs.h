/*
*   fs.h
*/

#ifndef __fs_h__
#define __fs_h__

#include "types.h"

int mountSD();
int unmountSD();
int fileReadOffset(void *dest, const char *path, u32 size, u32 offset);
int fileRead(void *dest, const char *path, u32 size);
int fileWrite(const void *buffer, const char *path, u32 size);
int fileSize(const char* path);

#endif
