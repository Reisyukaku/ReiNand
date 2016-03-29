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
#include "crypto.h"
#include "draw.h"

const void *firmLocation = (void*)0x24000000;
firmHeader *firm = NULL;
firmSectionHeader *section = NULL;
u32 emuOffset = 0,
    emuHeader = 0,
    emuRead = 0,
    emuWrite = 0,
    sdmmcOffset = 0, 
    firmSize = 0,
    mpuOffset = 0,
    emuCodeOffset = 0;
//Patch vars
u32 sigPatchOffset1 = 0,
    sigPatchOffset2 = 0,
    threadOffset1 = 0,
    threadOffset2 = 0,
    threadCodeOffset = 0,
    exeOffset = 0;

//Load firm into FCRAM
void loadFirm(void){
    //Read FIRM from SD card and write to FCRAM
	const char firmPath[] = "/rei/firmware.bin";
	firmSize = fileSize(firmPath);
    fileRead(firmLocation, firmPath, firmSize);
    
    //Decrypt firmware blob
    u8 firmIV[0x10] = {0};
    aes_setkey(0x16, memeKey, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x16);
    aes(firmLocation, firmLocation, firmSize / AES_BLOCK_SIZE, firmIV, AES_CBC_DECRYPT_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);
    
    //Parse firmware
    firm = firmLocation;
    section = firm->section;
    arm9loader(firmLocation + section[2].offset);
    
    //Set MPU for emu/thread code region
    getMPU(firmLocation, firmSize, &mpuOffset);
    memcpy((u8*)mpuOffset, mpu, sizeof(mpu));
}

//Nand redirection
void loadEmu(void){
    
    //Check for Emunand
    if(((~*(unsigned *)0x10146000) & 0xFFF) == (1 << 3)) return; //Press start to override emunand
    getEmunandSect(&emuOffset, &emuHeader);
    if(!(emuOffset | emuHeader)) return;
    
    //Read emunand code from SD
    const char path[] = "/rei/emunand/emunand.bin";
    u32 size = fileSize(path);
    getEmuCode(firmLocation, &emuCodeOffset, firmSize);
    fileRead(emuCodeOffset, path, size);
    
    //Setup Emunand code
    u32 *pos_sdmmc = memsearch(emuCodeOffset, "SDMC", size, 4);
    u32 *pos_offset = memsearch(emuCodeOffset, "NAND", size, 4);
    u32 *pos_header = memsearch(emuCodeOffset, "NCSD", size, 4);
	getSDMMC(firmLocation, &sdmmcOffset, firmSize);
    getEmuRW(firmLocation, firmSize, &emuRead, &emuWrite);
    *pos_sdmmc = sdmmcOffset;
    *pos_offset = emuOffset;
    *pos_header = emuHeader;
    
    //Add Emunand hooks
    memcpy((u8*)emuRead, nandRedir, sizeof(nandRedir));
    memcpy((u8*)emuWrite, nandRedir, sizeof(nandRedir));
}

//Patches
void patchFirm(){
    
    //Disable signature checks
    getSigChecks(firmLocation, firmSize, &sigPatchOffset1, &sigPatchOffset2);
    memcpy((u8*)sigPatchOffset1, sigPatch1, sizeof(sigPatch1));
    memcpy((u8*)sigPatchOffset2, sigPatch2, sizeof(sigPatch2));
    
    //Disable firm partition update
    getExe(firmLocation, firmSize, &exeOffset);
    memcpy((u8*)exeOffset, "kek", 3);
    
    //Create arm9 thread
    const char thPath[] = "/rei/thread/arm9.bin";
    u32 thSize = fileSize(thPath);
    getThreadCode(&threadCodeOffset);
    getThreadHooks(firmLocation, firmSize, &threadOffset1, &threadOffset2);
    fileRead(threadCodeOffset, thPath, thSize);
    memcpy((u8*)threadOffset1, threadHook1, sizeof(threadHook1));
    memcpy((u8*)threadOffset2, threadHook2, sizeof(threadHook2));
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
    ((void (*)())0x801B01C)();
}