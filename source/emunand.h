/*
*   emunand.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef EMU_INC
#define EMU_INC

#include "types.h"

#define NCSD_MAGIC (0x4453434E)

void getEmunandSect(uPtr *off, uPtr *head);
void getSDMMC(const void *pos, Size size, uPtr *off);
void getEmuRW(const void *pos, Size size, uPtr *readOff, uPtr *writeOff);
void getMPU(const void *pos, Size size, uPtr *off);
void getEmuCode(const void *pos, Size size, uPtr *off);
#endif