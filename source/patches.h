/*
*   patches.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef PATCHES_INC
#define PATCHES_INC

#include "types.h"

void getSigChecks(const void *pos, Size size, uPtr *off, uPtr *off2);
void getFirmWrite(const void *pos, Size size, uPtr *off);
void getLoader(const void *pos, Size *ldrSize, uPtr *ldrOff);
void getFOpen(void *pos, u32 size, uPtr *off);
void getReboot(void *pos, u32 size, uPtr *off);

#endif