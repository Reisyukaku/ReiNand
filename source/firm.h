/*
*   firm.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/
#ifndef FIRM_INC
#define FIRM_INC

#include "types.h"

#define CFG_BOOTENV *(volatile uint32_t *)0x10010000
#define HID ~*(volatile uint32_t *)0x10146000
#define PDN_MPCORE_CFG *(u8*)0x10140FFC

void loadSplash(void);
void loadFirm(void);
void loadEmu(void);
void patchFirm(void);
void launchFirm(void);

#endif