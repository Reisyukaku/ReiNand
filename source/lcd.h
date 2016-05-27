/*
*   lcd.h
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#ifndef LCD_INC
#define LCD_INC

#include "types.h"

#define GPU_PDN_CNT (*(vu8*)0x10141200)

void shutdownLCD(void);
#endif