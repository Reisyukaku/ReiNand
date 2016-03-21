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

const firmHeader *firmLocation = (firmHeader *)0x24000000;
firmSectionHeader *section;

//Emunand vars
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
    threadCodeOffset = 0;

//Load firm into FCRAM
void loadFirm(void){
    //Read FIRM from SD card and write to FCRAM
	const char firmPath[] = "/rei/firmware.bin";
	firmSize = fileSize(firmPath);
    fileRead((u8*)firmLocation, firmPath, firmSize);
    
    //Decrypt firmware blob
    u8 firmIV[0x10] = {0};
    aes_setkey(0x16, memeKey, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x16);
    aes((u8*)firmLocation, (u8*)firmLocation, firmSize / AES_BLOCK_SIZE, firmIV, AES_CBC_DECRYPT_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);
    
    //Parse firmware
    section = (firmSectionHeader*)firmLocation->section;
    
    //Set MPU for emu/thread code region
    getMPU((u8*)firmLocation, firmSize, &mpuOffset);
    memcpy((u8*)mpuOffset, mpu, sizeof(mpu));
}

//Nand redirection
void loadEmu(void){
    
    //Check for force sysnand
    if(((~*(unsigned *)0x10146000) & 0xFFF) == (1 << 3)) return;
    
    //Read emunand code from SD
	const char path[] = "/rei/emunand/emunand.bin";
    getEmuCode((u8*)firmLocation, &emuCodeOffset, firmSize);
    u32 size = fileSize(path);
    fileRead((u8*)emuCodeOffset, path, size);
    
    //Find and patch emunand related offsets
	u32 *pos_sdmmc = memsearch((u8*)emuCodeOffset, "SDMC", size, 4);
    u32 *pos_offset = memsearch((u8*)emuCodeOffset, "NAND", size, 4);
    u32 *pos_header = memsearch((u8*)emuCodeOffset, "NCSD", size, 4);
	getSDMMC((u8*)firmLocation, &sdmmcOffset, firmSize);
    getEmunandSect(&emuOffset, &emuHeader);
    getEmuRW((u8*)firmLocation, firmSize, &emuRead, &emuWrite);
    getMPU((u8*)firmLocation, firmSize, &mpuOffset);
	*pos_sdmmc = sdmmcOffset;
	*pos_offset = emuOffset;
	*pos_header = emuHeader;
	
    //Add emunand hooks
    memcpy((u8*)emuRead, nandRedir, sizeof(nandRedir));
    memcpy((u8*)emuWrite, nandRedir, sizeof(nandRedir));
}

//Patches
void patchFirm(){
    
    //Create arm9 thread
    getThreadCode(&threadCodeOffset);
    getThreadHooks((u8*)firmLocation, firmSize, &threadOffset1, &threadOffset2);
    fileRead((u8*)threadCodeOffset, "/rei/thread/arm9.bin", 0);
    memcpy((u8*)threadOffset1, th1, sizeof(th1));
    memcpy((u8*)threadOffset2, th2, sizeof(th2));
}

void launchFirm(void){
    
    //Copy firm partitions to respective memory locations
    memcpy(section[0].address, (u8*)firmLocation + section[0].offset, section[0].size); //DSP
    memcpy(section[1].address, (u8*)firmLocation + section[1].offset, section[1].size); //K11
    memcpy(section[2].address, (u8*)firmLocation + section[2].offset, section[2].size); //K9
    
    //Run ARM11 screen stuff
    vu32 *arm11 = (vu32*)0x1FFFFFF8;
    *arm11 = (u32)shutdownLCD;
    while (*arm11);
    
    //Set ARM11 kernel
    *arm11 = (u32)firmLocation->arm11Entry;
    
    //Final jump to arm9 binary
    ((void (*)())firmLocation->arm9Entry)();
}