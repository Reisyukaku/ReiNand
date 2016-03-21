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
u8 mpu[0x2C] = {    //MPU shit
    0x03, 0x00, 0x36, 0x00, 0x00, 0x00, 0x10, 0x10, 0x01, 0x00, 0x00, 0x01, 0x03, 0x00, 0x36, 0x00, 
    0x00, 0x00, 0x00, 0x20, 0x01, 0x01, 0x01, 0x01, 0x03, 0x06, 0x20, 0x00, 0x00, 0x00, 0x00, 0x08, 
    0x01, 0x01, 0x01, 0x01, 0x03, 0x06, 0x1C, 0x00, 0x00, 0x00, 0x02, 0x08
    };
u8 nandRedir[0x08] = {0x00, 0x4C, 0xA0, 0x47, 0xC0, 0xA4, 0x01, 0x08};    //Branch to emunand function

/*
*   Sig checks
*/
u8 sigPatch1[2] = {0x00, 0x20};
u8 sigPatch2[4] = {0x00, 0x20, 0x70, 0x47};

/*
*   Arm9 thread
*/
u8 threadHook1[4] = {0x2C, 0xF0, 0x9F, 0xE5};   //ldr pc, =threadHook2
u8 threadHook2[4] = {0xE0, 0xA7, 0x01, 0x08};   //threadHook2: .word 0x0801A7E0


/**************************************************
*                   Functions
**************************************************/

//Where thread code is stored in firm
void getThreadCode(u32 *off){
    *off = 0x2407A3E0;
}

//Offsets to redirect to thread code
void getThreadHooks(void *pos, u32 size, u32 *off, u32 *off2){
    const u8 pattern[] = {0x3A, 0x2C, 0x00, 0x9F, 0xE5};

    *off = memsearch(pos, (void*)pattern, size, 5) + 1;
    *off2 = *off + 0x34;
}

//Offsets to redirect to thread code
void getSigChecks(void *pos, u32 size, u32 *off, u32 *off2){
    const u8 pattern[] = {0xC0, 0x1C, 0x76, 0xE7};
    const u8 pattern2[] = {0xB5, 0x22, 0x4D, 0x0C};

    *off = (u32)memsearch(pos, (void*)pattern, size, 4);
    *off2 = (u32)memsearch(pos, (void*)pattern2, size, 4) - 1;
}