/*
*   draw.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef DRAW_INC
#define DRAW_INC

#include "types.h"

static struct fb {
    u8 *top_left;
    u8 *top_right;
    u8 *bottom;
} *fb = (struct fb *)0x23FFFE00;

u8 clearScreen(void);
void loadSplash(void);
#endif