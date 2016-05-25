/*
*   lcd.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "lcd.h"

vu32 *arm11Entry = (vu32*)0x1FFFFFF8;

void shutdownLCD(void){
    void __attribute__((naked)) ARM11(void){
        //Disable interrupts
        __asm(".word 0xF10C01C0");
        
        //Clear ARM11 entry offset and wait for ARM11 to set it, and jumps
        *arm11Entry = 0;
        
        //Clear LCDs
        *(vu32*)0x10202A44 = 0;
        *(vu32*)0x10202244 = 0;
        *(vu32*)0x10202014 = 0;
        
        while (!*arm11Entry);
        ((void (*)())*arm11Entry)();
    }
    if(GPU_PDN_CNT != 1){
        *arm11Entry = (u32)ARM11;
        while(*arm11Entry);
    }
}