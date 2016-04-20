/*
*   draw.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "draw.h"
#include "fs.h"
#include "memory.h"

u8 clearScreen(void){
    //Only clear screens if they are initialized
    if(GPU_PDN_CNT == 1) return 0;
    memset(fb->top_left, 0, 0x38400);
    memset(fb->top_right, 0, 0x38400);
    memset(fb->bottom, 0, 0x38400);
    return 1;
}

void shutdownLCD(void){
    //Clear LCDs
    *(vu32*)0x10202A44 = 0;
    *(vu32*)0x10202244 = 0;
    *(vu32*)0x10202014 = 0;
    
    //Clear ARM11 entry offset and wait for ARM11 to set it, and jumps
    *(vu32*)0x1FFFFFF8 = 0;
    while (!*(vu32*)0x1FFFFFF8);
    ((void (*)())*(vu32*)0x1FFFFFF8)();
}

void loadSplash(void){
    //If FB was clear, and the image exists, display
    if(clearScreen()){
        fopen("/rei/splash.bin", "rb");
        fread(fb->top_left, 1, fsize());
        fclose();
        u64 i = 0xFFFFFF; while(--i) __asm("mov r0, r0"); //Less Ghetto sleep func
    }
}