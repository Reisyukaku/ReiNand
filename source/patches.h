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

/**************************************************
*                   Functions
**************************************************/
void getSigChecks(void *pos, u32 size, u32 *off, u32 *off2);
void getExe(void *pos, u32 size, u32 *off);

#endif