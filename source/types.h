/*
*   types.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef TYPES_INC
#define TYPES_INC

#include <stdint.h>
#include <stdlib.h>

#define CFG_BOOTENV *(vu32*)0x10010000
#define HID ~*(vu32*)0x10146000
#define PDN_MPCORE_CFG *(u8*)0x10140FFC
#define PDN_SPI_CNT *(vu32*)0x101401C0
#define UNITINFO *(u8*)0x10010010

//Common data types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef volatile uint8_t vu8;
typedef volatile uint16_t vu16;
typedef volatile uint32_t vu32;
typedef volatile uint64_t vu64;
typedef uintptr_t uPtr;
typedef size_t Size;

//FIRM Header layout
typedef struct firmSectionHeader {
    u32 offset;
    u8 *address;
    u32 size;
    u32 procType;
    u8 hash[0x20];
} firmSectionHeader;

typedef struct firmHeader {
    u32 magic;
    u32 reserved1;
    u32 arm11Entry;
    u32 arm9Entry;
    u8 reserved2[0x30];
    firmSectionHeader section[4];
} firmHeader;

#endif