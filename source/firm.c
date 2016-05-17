/*
*   firm.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include <string.h>
#include "firm.h"
#include "patches.h"
#include "memory.h"
#include "fs.h"
#include "emunand.h"
#include "draw.h"

//Firm vars
const void *firmLocation = (void*)0x24000000;
firmHeader *firm = NULL;
firmSectionHeader *section = NULL;
Size firmSize = 0;

//Emu vars
u32 emuOffset = 0,
    emuHeader = 0,
    emuRead = 0,
    emuWrite = 0,
    sdmmcOffset = 0,
    mpuOffset = 0,
    emuCodeOffset = 0;
    
//Patch vars
u32 sigPatchOffset1 = 0,
    sigPatchOffset2 = 0,
    threadOffset1 = 0,
    threadOffset2 = 0,
    threadCodeOffset = 0,
    firmWriteOffset = 0;

//Load firm into FCRAM
void loadFirm(void){
    //Read FIRM from SD card and write to FCRAM
    fopen("/rei/firmware.bin", "rb");
    firmSize = fsize();
    fread(firmLocation, 1, firmSize);
    fclose();
    decryptFirm(firmLocation, firmSize);
    
    //Initial setup
    firm = firmLocation;
    section = firm->section;
    k9loader(firmLocation + section[2].offset);
    
    //Set MPU for emu/thread code region
    getMPU(firmLocation, firmSize, &mpuOffset);
    memcpy((u8*)mpuOffset, mpu, sizeof(mpu));
    
    //Dont boot emu if AGB game was just played, or if START was held.
    getEmunandSect(&emuOffset, &emuHeader);
    if((HID & 0xFFF) == (1 << 3) || CFG_BOOTENV == 0x7 || !(emuOffset | emuHeader))
        loadSys();
    else
        loadEmu();
}

//Setup for Sysnand
void loadSys(void){
    //Disable firm partition update if a9lh is installed
    if(!PDN_SPI_CNT){
        const u16 fwPatch[] = {0x2000, 0x46C0};
        getFirmWrite(firmLocation, firmSize, &firmWriteOffset);
        memcpy((u8*)firmWriteOffset, fwPatch, 4);
    }
}

//Nand redirection
void loadEmu(void){ 

    //Read emunand code from SD
    fopen("/rei/emunand/emunand.bin", "rb");
    Size emuSize = fsize();
    getEmuCode(firmLocation, firmSize, &emuCodeOffset);
    fread(emuCodeOffset, 1, emuSize);
    fclose();
    
    //Setup Emunand code
    uPtr *pos_sdmmc = memsearch(emuCodeOffset, "SDMC", emuSize, 4);
    uPtr *pos_offset = memsearch(emuCodeOffset, "NAND", emuSize, 4);
    uPtr *pos_header = memsearch(emuCodeOffset, "NCSD", emuSize, 4);
    getSDMMC(firmLocation, firmSize, &sdmmcOffset);
    getEmuRW(firmLocation, firmSize, &emuRead, &emuWrite);
    *pos_sdmmc = sdmmcOffset;
    *pos_offset = emuOffset;
    *pos_header = emuHeader;
    
    //Add Emunand hooks
    memcpy((u8*)emuRead, nandRedir, sizeof(nandRedir));
    memcpy((u8*)emuWrite, nandRedir, sizeof(nandRedir));
}

//Patches arm9 things on Sys/Emu
void patchFirm(){ 
    //Disable signature checks
    getSigChecks(firmLocation, firmSize, &sigPatchOffset1, &sigPatchOffset2);
    memcpy((u8*)sigPatchOffset1, sigPatch1, sizeof(sigPatch1));
    memcpy((u8*)sigPatchOffset2, sigPatch2, sizeof(sigPatch2));
    
    //Inject custom loader
    fopen("/rei/loader.cxi", "rb");
    fread(firmLocation + 0x26600, 1, fsize());
    fclose();
}

void launchFirm(void){
    //Copy firm partitions to respective memory locations
    memcpy(section[0].address, firmLocation + section[0].offset, section[0].size);
    memcpy(section[1].address, firmLocation + section[1].offset, section[1].size);
    memcpy(section[2].address, firmLocation + section[2].offset, section[2].size);
    
    //Run ARM11 screen stuff
    vu32 *arm11 = (vu32*)0x1FFFFFF8;
    *arm11 = (u32)shutdownLCD;
    while (*arm11);
    
    //Set ARM11 kernel
    *arm11 = (u32)firm->arm11Entry;
    
    //Final jump to arm9 binary
    u32 entry = PDN_MPCORE_CFG != 1 ? 0x801B01C : firm->arm9Entry;
    ((void (*)())entry)();
}