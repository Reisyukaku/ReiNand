/*
*   main.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*
*   Minimalist CFW for N3DS
*/

#include "fs.h"
#include "firm.h"
#include "draw.h"

#ifndef BOOT_SYS
int mode = 1;
#else
int mode = 0;
#endif

int main(){
    mountSD();
    #if !defined BOOT_EMU && !defined BOOT_SYS
    loadSplash();
    while(1){
        if(((~*(unsigned *)0x10146000) & 0xFFF) == (1 << 3)) break;
        else if(((~*(unsigned *)0x10146000) & 0xFFF) == ((1 << 3) | (1 << 1))) {mode = 0; break;}
    } //Start = emu; Start+B = sys
    #endif
    loadFirm(mode);
    patchFirm();
    launchFirm();
    return 0;
}