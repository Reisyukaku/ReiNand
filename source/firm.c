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
     ldrOffset = 0,
     rebootOffset = 0, 
     fOpenOffset = 0;

//Load firm into FCRAM
void loadFirm(void){
    //Read FIRM from SD card and write to FCRAM
    fopen("/rei/firmware.bin", "rb");
    firmSize = fsize()/2;
    if(PDN_MPCORE_CFG == 1) fseek(firmSize); //If O3DS, load 2nd firm
    fread(firmLocation, 1, firmSize);
    fclose();
    decryptFirm(firmLocation, firmSize);
    
    //Initial setup
    firm = firmLocation;
    k9loader(firmLocation + firm->section[2].offset);
    
    //Set MPU for emu code region
    getMPU(firmLocation, firmSize, &mpuOffset);
    *(u32*)mpuOffset = PERMS(RW_RW, N_N, SIZE_256MB);        //Area4:0x10100000
    *(u32*)(mpuOffset+0x18) = PERMS(RW_RW, R_R, SIZE_128KB); //Area6:0x8000000
    *(u32*)(mpuOffset+0x24) = PERMS(RW_RW, R_R, SIZE_32KB);  //Area7:0x8020000
    
    //Inject custom loader if exists
    if(fopen("/rei/loader.cxi", "rb")){
        u8 *arm11SysMods = (u8*)firm + firm->section[0].offset;
        Size ldrInFirmSize;
        Size ldrFileSize = fsize();
        getLoader(arm11SysMods, &ldrInFirmSize, &ldrOffset);
        memcpy(firm->section[0].address, arm11SysMods, ldrOffset);
        fread(firm->section[0].address + ldrOffset, 1, ldrFileSize);
        memcpy(firm->section[0].address + ldrOffset + ldrFileSize, arm11SysMods + ldrOffset + ldrInFirmSize, firm->section[0].size - (ldrOffset + ldrInFirmSize));
        fclose();
    }else{
        memcpy(firm->section[0].address, firmLocation + firm->section[0].offset, firm->section[0].size);
    }
    
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
        *(u32*)firmWriteOffset = 0x46C02000;
    }
}

//Nand redirection
void loadEmu(void){
    //Read emunand code from SD
    getEmuCode(firmLocation, firmSize, &emuCodeOffset);
    memcpy((void*)emuCodeOffset, emunand, emunand_size);
    u32 branchAddr = (u32)((uPtr)emuCodeOffset - (uPtr)firmLocation - firm->section[2].offset + firm->section[2].address);
    
    //Setup Emunand code
    uPtr *pos_sdmmc = (uPtr*)memsearch((void*)emuCodeOffset, "SDMC", emunand_size, 4);
    uPtr *pos_offset = (uPtr*)memsearch((void*)emuCodeOffset, "NAND", emunand_size, 4);
    uPtr *pos_header = (uPtr*)memsearch((void*)emuCodeOffset, "NCSD", emunand_size, 4);
    getSDMMC(firmLocation, firmSize, (void*)&sdmmcOffset);
    getEmuRW(firmLocation, firmSize, (void*)&emuRead, (void*)&emuWrite);
    *pos_sdmmc = sdmmcOffset;
    *pos_offset = emuOffset;
    *pos_header = emuHeader;
    *(u32*)emuRead = *(u32*)emuWrite = 0x47A04C00;
    
    //Add Emunand hooks
    *(u32*)(emuRead+4) = *(u32*)(emuWrite+4) = branchAddr;
}

//Patches arm9 things on Sys/Emu
void patchFirm(){    
    //Disable signature checks
    getSigChecks(firmLocation, firmSize, &sigPatchOffset1, &sigPatchOffset2);
    *(u16*)sigPatchOffset1 = 0x2000;
    *(u32*)sigPatchOffset2 = 0x47702000;

    //Apply Reboot Patch on Firm. Add kernel check 
    getReboot(firmLocation, firmSize, &rebootOffset);
    memcpy((void*)rebootOffset, reboot, reboot_size);
    uPtr *FOpenPos = (uPtr*)memsearch(rebootOffset, "OPEN", reboot_size, 4);
    getFOpen(firmLocation, firmSize, &fOpenOffset);
    *FOpenPos = fOpenOffset;
}

void launchFirm(void){
    //Copy firm partitions to respective memory locations
    memcpy(firm->section[1].address, firmLocation + firm->section[1].offset, firm->section[1].size);
    memcpy(firm->section[2].address, firmLocation + firm->section[2].offset, firm->section[2].size);
    
    //Run ARM11 screen stuff
    vu32 *arm11 = (vu32*)0x1FFFFFF8;
    shutdownLCD();
    
    //Set ARM11 kernel
    *arm11 = firm->arm11Entry;
    
    //Final jump to arm9 binary
    u32 entry = PDN_MPCORE_CFG != 1 ? (u32)0x801B01C : firm->arm9Entry;
    ((void (*)())entry)();
}