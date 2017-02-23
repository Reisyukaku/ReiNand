/*
*   patches.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "patches.h"
#include "memory.h"

//Offsets to redirect to thread code
void getSigChecks(const void *pos, Size size, uPtr *off, uPtr *off2){
    const u8 pattern[] = {0xC0, 0x1C, 0x76, 0xE7};
    const u8 pattern2[] = {0xB5, 0x22, 0x4D, 0x0C};

    *off = memsearch(pos, (void*)pattern, size, sizeof(pattern));
    *off2 = memsearch(pos, (void*)pattern2, size, sizeof(pattern2)) - 1;
}

//Offset to exe: protocol
void getFirmWrite(const void *pos, Size size, uPtr *off){
    const u8 pattern[] = {0x00, 0x28, 0x01, 0xDA};
    uPtr exe = memsearch(pos, "exe:", size, sizeof(pattern));

    *off = memsearch((void*)exe, (void*)pattern, 0x100, sizeof(pattern));
}

void getFOpen(void *pos, u32 size, uPtr *off){
    //Calculate fOpen (Kidnapped from AUREINAND)
    uPtr p9addr = *(uPtr*)(memsearch(pos, "ess9", size, 4) + 0xC);
    uPtr p9off = memsearch(pos, "code", size, 4) + 0x1FF;
    unsigned char pattern[] = {0xB0, 0x04, 0x98, 0x0D};
    *off = (uPtr)memsearch(pos, pattern, size, sizeof(pattern)) - 2 - p9off + p9addr;
}

void getReboot(void *pos, u32 size, uPtr *off){
    //Look for FIRM reboot code
    unsigned char pattern[] = {0xE2, 0x20, 0x20, 0x90};

    *off = memsearch(pos, pattern, size, sizeof(pattern)) - 0x13;
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