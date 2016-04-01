/*
*   fs.h
*/

#ifndef __fs_h__
#define __fs_h__

#include "types.h"

int mountSD();
int unmountSD();
int fileReadOffset(void *dest, const char *path, Size size, u32 offset);
int fileRead(void *dest, const char *path, Size size);
int fileWrite(const void *buffer, const char *path, Size size);
int fileSize(const char* path);

#endif
