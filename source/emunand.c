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

void getEmunandSect(uPtr *off, uPtr *head){
    Size nandSize = getMMCDevice(0)->total_size;
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

void getSDMMC(const void *pos, Size size, uPtr *off){
    //Look for struct code
    const u8 pattern[] = {0x21, 0x20, 0x18, 0x20};
    uPtr sdmmc = memsearch(pos, pattern, size, 4) - 1;

    *off = *(uPtr*)(sdmmc + 0x0A) + *(uPtr*)(sdmmc + 0x0E);
}


void getEmuRW(const void *pos, Size size, uPtr *readOff, uPtr *writeOff){
    //Look for read/write code
    const u8 pattern[] = {0x04, 0x00, 0x0D, 0x00, 0x17, 0x00, 0x1E, 0x00, 0xC8, 0x05};
    *writeOff = memsearch(pos, pattern, size, 10);
    *readOff = memsearch((void*)(*writeOff - 0x1000), pattern, 0x1000, 10);
}

void getMPU(const void *pos, Size size, uPtr *off){
    //Look for MPU pattern
    const u8 pattern[] = {0x03, 0x00, 0x24, 0x00, 0x00};
    *off = memsearch(pos, pattern, size, 5);
}

void getEmuCode(const void *pos, Size size, uPtr *off){
    //Finds start of 0xFF field
    const u8 pattern[] = {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};
    uPtr proc9 = memsearch(pos, "Proc", size, 4);
 
    //Looking for the last spot before Process9
    *off = memsearch(pos, pattern, size - (size - (uPtr)(proc9 - (uPtr)pos)), 6) + 0xF + (PDN_MPCORE_CFG == 1 ? 0x100 : 0);
}