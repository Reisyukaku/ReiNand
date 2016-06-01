/*
*   firm.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "firm.h"
#include "patches.h"
#include "memory.h"
#include "fs.h"
#include "emunand.h"
#include "lcd.h"
#include "crypto.h"
#include "../build/payloads.h"

//Firm vars
const void *firmLocation = (void*)0x24000000;
firmHeader *firm = NULL;
firmSectionHeader *section = NULL;
Size firmSize = 0;

//Emu vars
uPtr emuOffset = 0,
     emuHeader = 0,
     emuRead = 0,
     emuWrite = 0,
     sdmmcOffset = 0,
     mpuOffset = 0,
     emuCodeOffset = 0;
    
//Patch vars
uPtr sigPatchOffset1 = 0,
     sigPatchOffset2 = 0,
     threadOffset1 = 0,
     threadOffset2 = 0,
     threadCodeOffset = 0,
     firmWriteOffset = 0,
     ldrOffset = 0;

//Load firm into FCRAM
void loadFirm(void){
    //Read FIRM from SD card and write to FCRAM
    fopen("/rei/firmware.bin", "rb");
    firmSize = fsize()/2;
    if(PDN_MPCORE_CFG == 1) fseek(firmSize);
    fread(firmLocation, 1, firmSize);
    fclose();
    decryptFirm(firmLocation, firmSize);
    
    //Initial setup
    firm = firmLocation;
    section = firm->section;
    keyInit(firmLocation + section[2].offset);
    
    //Set MPU for emu/thread code region
    getMPU(firmLocation, firmSize, &mpuOffset);
    memcpy((u8*)mpuOffset, mpu, sizeof(mpu));
    
    //Inject custom loader
    fopen("/rei/loader.cxi", "rb");
    u8 *arm11SysMods = (u8 *)firm + section[0].offset;
    Size ldrInFirmSize;
    Size ldrFileSize = fsize();
    getLoader(arm11SysMods, &ldrInFirmSize, &ldrOffset);
    memcpy(section[0].address, arm11SysMods, ldrOffset);
    fread(section[0].address + ldrOffset, 1, ldrFileSize);
    memcpy(section[0].address + ldrOffset + ldrFileSize, arm11SysMods + ldrOffset + ldrInFirmSize, section[0].size - (ldrOffset + ldrInFirmSize));
    fclose();
    
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
        getFirmWrite(firmLocation, firmSize, &firmWriteOffset);
        memcpy((u8*)firmWriteOffset, fwPatch, 4);
    }
}

//Nand redirection
void loadEmu(void){ 
    //Read emunand code from SD
    getEmuCode(firmLocation, firmSize, &emuCodeOffset);
    memcpy((void*)emuCodeOffset, emunand, emunand_size);
    
    //Setup Emunand code
    uPtr *pos_sdmmc = (uPtr*)memsearch((void*)emuCodeOffset, "SDMC", emunand_size, 4);
    uPtr *pos_offset = (uPtr*)memsearch((void*)emuCodeOffset, "NAND", emunand_size, 4);
    uPtr *pos_header = (uPtr*)memsearch((void*)emuCodeOffset, "NCSD", emunand_size, 4);
    getSDMMC(firmLocation, firmSize, (void*)&sdmmcOffset);
    getEmuRW(firmLocation, firmSize, (void*)&emuRead, (void*)&emuWrite);
    *pos_sdmmc = sdmmcOffset;
    *pos_offset = emuOffset;
    *pos_header = emuHeader;
    
    //Add Emunand hooks
    memcpy((void*)emuRead, nandRedir, sizeof(nandRedir));
    memcpy((void*)emuWrite, nandRedir, sizeof(nandRedir));
}

//Patches arm9 things on Sys/Emu
void patchFirm(){    
    //Disable signature checks
    getSigChecks(firmLocation, firmSize, &sigPatchOffset1, &sigPatchOffset2);
    memcpy((void*)sigPatchOffset1, sigPatch1, sizeof(sigPatch1));
    memcpy((void*)sigPatchOffset2, sigPatch2, sizeof(sigPatch2));
}

void launchFirm(void){
    //Copy firm partitions to respective memory locations
    memcpy(section[1].address, firmLocation + section[1].offset, section[1].size);
    memcpy(section[2].address, firmLocation + section[2].offset, section[2].size);
    
    //Run ARM11 screen stuff
    vu32 *arm11 = (vu32*)0x1FFFFFF8;
    shutdownLCD();
    
    //Set ARM11 kernel
    *arm11 = firm->arm11Entry;
    
    //Final jump to arm9 binary
    u32 entry = PDN_MPCORE_CFG != 1 ? (u32)0x801B01C : firm->arm9Entry;
    ((void (*)())entry)();
}