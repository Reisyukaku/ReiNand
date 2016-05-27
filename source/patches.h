/*
*   patches.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef PATCHES_INC
#define PATCHES_INC

#include "types.h"

/**************************************************
*                   Patches
**************************************************/
const u8 mpu[0x2C];       
const u8 nandRedir[0x08];
const u8 sigPatch1[2];
const u8 sigPatch2[4];
const u16 fwPatch[2];

/**************************************************
*                   Functions
**************************************************/
void getSigChecks(const void *pos, Size size, uPtr *off, uPtr *off2);
void getFirmWrite(const void *pos, Size size, uPtr *off);
void getLoader(const void *pos, Size *ldrSize, uPtr *ldrOff);

#endif