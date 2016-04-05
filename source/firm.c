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
    exeOffset = 0;

//Load firm into FCRAM
void loadFirm(void){
    //Read FIRM from SD card and write to FCRAM
	const char firmPath[] = "/rei/firmware.bin";
	firmSize = fileSize(firmPath);
    fileRead(firmLocation, firmPath, firmSize);
    decryptFirm(firmLocation, firmSize);
    
    //Inject custom loader
    const char ldrPath[] = "/rei/loader.cxi";
	u32 ldrSize = fileSize(ldrPath);
    fileRead(firmLocation + 0x26600, ldrPath, ldrSize);
    
    //Parse firmware
    firm = firmLocation;
    section = firm->section;
    if(PDN_MPCORE_CFG != 1) arm9loader(firmLocation + section[2].offset);
    
    //Set MPU for emu/thread code region
    getMPU(firmLocation, firmSize, &mpuOffset);
    memcpy((u8*)mpuOffset, mpu, sizeof(mpu));
}

//Nand redirection
void loadEmu(void){
    
    //Check for Emunand
    if((HID & 0xFFF) == (1 << 3) || CFG_BOOTENV == 0x7) return; //Override emunand
    getEmunandSect(&emuOffset, &emuHeader);
    if(!(emuOffset | emuHeader)) return;
    
    //Read emunand code from SD
    const char path[] = "/rei/emunand/emunand.bin";
    Size emuSize = fileSize(path);
    getEmuCode(firmLocation, firmSize, &emuCodeOffset);
    fileRead(emuCodeOffset, path, emuSize);
    
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
    Size thSize = fileSize(thPath);
    getThreadCode(&threadCodeOffset);
    getThreadHooks(firmLocation, firmSize, &threadOffset1, &threadOffset2);
    fileRead(threadCodeOffset, thPath, thSize);
    uPtr *pos_THD1 = memsearch(threadCodeOffset, "THD1", thSize, 4);
    uPtr *pos_THD2 = memsearch(threadCodeOffset, "THD2", thSize, 4);
    *pos_THD1 = 0x080E3408;
    *pos_THD2 = 0x0808519C;
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
    u32 entry = PDN_MPCORE_CFG != 1 ? 0x801B01C : firm->arm9Entry;
    ((void (*)())entry)();
}