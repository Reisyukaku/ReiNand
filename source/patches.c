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

// MPU
const u8 mpu[0x2C] = {
    0x03, 0x00, 0x36, 0x00, 0x00, 0x00, 0x10, 0x10, 0x01, 0x00, 0x00, 0x01, 0x03, 0x00, 0x36, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x01, 0x01, 0x01, 0x01, 0x03, 0x06, 0x20, 0x00, 0x00, 0x00, 0x00, 0x08, 
    0x01, 0x01, 0x01, 0x01, 0x03, 0x06, 0x1C, 0x00, 0x00, 0x00, 0x02, 0x08
    };
const u8 nandRedir[0x08] = {0x00, 0x4C, 0xA0, 0x47, 0xC0, 0xA5, 0x01, 0x08};    //Branch to emunand function

// Sig checks
const u8 sigPatch1[2] = {0x00, 0x20};
const u8 sigPatch2[4] = {0x00, 0x20, 0x70, 0x47};

// Firmware update protection
const u16 fwPatch[] = {0x2000, 0x46C0};


/**************************************************
*                   Functions
**************************************************/

//Offsets to redirect to thread code
void getSigChecks(const void *pos, Size size, uPtr *off, uPtr *off2){
    const u8 pattern[] = {0xC0, 0x1C, 0x76, 0xE7};
    const u8 pattern2[] = {0xB5, 0x22, 0x4D, 0x0C};

    *off = memsearch(pos, (void*)pattern, size, 4);
    *off2 = memsearch(pos, (void*)pattern2, size, 4) - 1;
}

//Offset to exe: protocol
void getFirmWrite(const void *pos, Size size, uPtr *off){
    const u8 pattern[] = {0x00, 0x28, 0x01, 0xDA};
    uPtr exe = memsearch(pos, "exe:", size, 4);

    *off = memsearch((void*)exe, (void*)pattern, 0x100, 4);
}

void getLoader(const void *pos, Size *ldrSize, uPtr *ldrOff){
    u8 *off = (u8*)pos;
    Size s;

    while(1){
        s = *(u32*)(off + 0x104) * 0x200;
        if(*(u32*)(off + 0x200) == 0x64616F6C) break;
        off += s;
    }

    *ldrSize = s;
    *ldrOff = (uPtr)(off - (u8*)pos);
}