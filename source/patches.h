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
u8 mpu[0x2C];       
u8 nandRedir[0x08];
u8 sigPatch1[2];
u8 sigPatch2[4];
u8 threadHook1[4];
u8 threadHook2[4];

/**************************************************
*                   Functions
**************************************************/
void getThreadCode(u32 *off);
void getThreadHooks(void *pos, u32 size, u32 *off, u32 *off2);
void getSigChecks(void *pos, u32 size, u32 *off, u32 *off2);

#endif