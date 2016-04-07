/*
*   patches.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "patches.h"
#include "memory.h"

/**************************************************
*                   Patches
**************************************************/

/*
*   MPU
*/
const u8 mpu[0x2C] = {    //MPU shit
    0x03, 0x00, 0x36, 0x00, 0x00, 0x00, 0x10, 0x10, 0x01, 0x00, 0x00, 0x01, 0x03, 0x00, 0x36, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x01, 0x01, 0x01, 0x01, 0x03, 0x06, 0x20, 0x00, 0x00, 0x00, 0x00, 0x08, 
    0x01, 0x01, 0x01, 0x01, 0x03, 0x06, 0x1C, 0x00, 0x00, 0x00, 0x02, 0x08
    };
const u8 nandRedir[0x08] = {0x00, 0x4C, 0xA0, 0x47, 0xC0, 0xA5, 0x01, 0x08};    //Branch to emunand function

/*
*   Sig checks
*/
const u8 sigPatch1[2] = {0x00, 0x20};
const u8 sigPatch2[4] = {0x00, 0x20, 0x70, 0x47};


/**************************************************
*                   Functions
**************************************************/

//Offsets to redirect to thread code
void getSigChecks(void *pos, u32 size, u32 *off, u32 *off2){
    const u8 pattern[] = {0xC0, 0x1C, 0x76, 0xE7};
    const u8 pattern2[] = {0xB5, 0x22, 0x4D, 0x0C};

    *off = (u32)memsearch(pos, (void*)pattern, size, 4);
    *off2 = (u32)memsearch(pos, (void*)pattern2, size, 4) - 1;
}

//Offset to exe: protocol
void getExe(void *pos, u32 size, u32 *off){
    const char *pattern = "exe:";

    *off = memsearch(pos, (void*)pattern, size, 4);
}