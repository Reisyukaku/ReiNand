/*
*   fc.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include <stddef.h>
#include "fs.h"
#include "fatfs/ff.h"

static FATFS fs;
static FILE fp; //Had to make a static file since fatfs hated my file pointers.

u8 mountSD(void){
    if (f_mount(&fs, "0:", 1)) return 1;
    return 0;
}

u8 unmountSD(void){
    if (f_mount(NULL, "0:", 1)) return 1;
    return 0;
}

void fopen(const char *filename){
    f_open(&fp, filename, FA_READ);
}

void fclose(void){
    f_close(&fp);
}

void fseek(u32 offset){
    f_lseek(&fp, offset);
}

u8 eof(void){
    return f_eof(&fp);
}

Size fsize(void){
    return f_size(&fp);
}

Size fread(const char *buffer, Size elementSize, Size elementCnt){
    u32 br;
    if(f_read(&fp, buffer, elementSize*elementCnt, &br)) return 0;
    if (br == elementSize*elementCnt) br /= elementSize; else return 0;
    return br;
}