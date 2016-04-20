/*
*   draw.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include "types.h"

#define GPU_PDN_CNT (*(vu8 *)0x10141200)

static struct fb {
    u8 *top_left;
    u8 *top_right;
    u8 *bottom;
} *fb = (struct fb *)0x23FFFE00;

u8 clearScreen(void);
void shutdownLCD(void);
void loadSplash(void);