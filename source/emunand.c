/*
*   emunand.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "emunand.h"
#include "memory.h"
#include "fatfs/ff.h"
#include "fatfs/sdmmc/sdmmc.h"

static u8 *temp = (u8*)0x24300000;

void getEmunandSect(u32 *off, u32 *head){
    u32 nandSize = getMMCDevice(0)->total_size;
    if (sdmmc_sdcard_readsectors(1, 1, temp) == 0){   //Rednand
        if (*(u32*)(temp + 0x100) == NCSD_MAGIC) {
            *off = 1;
            *head = 1;
            return;
        }
    }
    if (sdmmc_sdcard_readsectors(nandSize, 1, temp) == 0) {   //GW Emu
        if (*(u32*)(temp + 0x100) == NCSD_MAGIC) {
            *off = 0;
            *head = nandSize;
            return;
        }
    }
    //No Emunand detected
    *off = 0;
    *head = 0;
}

void getSDMMC(void *pos, Size size, u32 *off){
    //Look for struct code
    const u8 pattern[] = {0x21, 0x20, 0x18, 0x20};
    u8 *sdmmc = memsearch(pos, pattern, size, 4) - 1;

    *off = *(u32 *)(sdmmc + 0x0A) + *(u32 *)(sdmmc + 0x0E);
}


void getEmuRW(void *pos, Size size, u32 *readOff, u32 *writeOff){
    //Look for read/write code
    unsigned char pattern[] = {0x04, 0x00, 0x0D, 0x00, 0x17, 0x00, 0x1E, 0x00, 0xC8, 0x05};
    *writeOff = (u32)memsearch(pos, pattern, size, 10);
    *readOff = (u32)memsearch((void *)(*writeOff - 0x1000), pattern, 0x1000, 10);
}

void getMPU(void *pos, Size size, u32 *off){
    //Look for MPU pattern
    unsigned char pattern[] = {0x03, 0x00, 0x24, 0x00, 0x00};
    *off = (u32)memsearch(pos, pattern, size, 5);
}

void getEmuCode(void *pos, Size size, u32 *off){
    //Finds start of 0xFF field
    unsigned char pattern[] = {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};
    void *proc9 = memsearch(pos, "Process9", size, 8);
 
    //Looking for the last spot before Process9
    *off = (u32)memsearch(pos, pattern, size - (size - (u32)(proc9 - pos)), 6) + 0xF + (PDN_MPCORE_CFG == 1 ? 0x100 : 0);
}