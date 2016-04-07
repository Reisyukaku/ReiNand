/*
*   fs.c
*/

#include <stddef.h>
#include "fs.h"
#include "fatfs/ff.h"

static FATFS fs;

int mountSD(){
    if (f_mount(&fs, "0:", 1)) return 1;
    return 0;
}

int unmountSD(){
    if (f_mount(NULL, "0:", 1)) return 1;
    return 0;
}

int fileReadOffset(void *dest, const char *path, Size size, u32 offset){
    FIL fp;
    u32 br = 0;

    if(f_open(&fp, path, FA_READ)) goto error;
    if (!size) size = f_size(&fp);
    if (offset) {
        if (f_lseek(&fp, offset)) goto error;
    }
    if (f_read(&fp, dest, size, &br) && br != size) goto error;
    return 1;

    error:
    f_close(&fp);
    return 0;
}

int fileRead(void *dest, const char *path, Size size){
    return fileReadOffset(dest, path, size, 0);
}

int fileWrite(const void *buffer, const char *path, Size size){
    FIL fp;
    u32 br = 0;

    if(f_open(&fp, path, FA_WRITE | FA_OPEN_ALWAYS)) goto error;
    if (f_write(&fp, buffer, size, &br) && br != size) goto error;
    return 1;
    
    error:
    f_close(&fp);
    return 0;
}

int fileSize(const char* path){
    FIL fp;

    if(f_open(&fp, path, FA_READ)) goto error;
    return f_size(&fp);
    
	error:
    f_close(&fp);
	return 0;
}