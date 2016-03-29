/*
*   thread.c
*       by Reisyukaku
*   Copyright (c) 2015 All Rights Reserved
*/

#include <wchar.h>
#include <stdio.h>
#include "thread.h"
#include "lib.h"
#include "FS.h"

unsigned char handle[32];

void fileReadWrite(void *buf, void *path, int size, char rw){
    unsigned int br = 0;
    memset(&handle, 0, 32);
    fopen9(&handle, path, 6);
    if(rw == 0) fread9(&handle, &br, buf, size);
    else fwrite9(&handle, &br, buf, size);
    fclose9(&handle);
}

void memdump(void* filename, void* buf, unsigned int size){
	fileReadWrite(buf, filename, size, WRITE);
	memset(VRAM+0x1E6000, 0xFF, 0x46500);
}

void thread(void){
	while(1){
        if(isPressed(BUTTON_START | BUTTON_X)){
            unsigned char buf[0x10] = {0};
            int loc = 0;
            fileReadWrite(buf, L"sdmc:/rei/RAM.txt", 0x20, READ);
            loc = atoi(buf);
            memdump(L"sdmc:/RAMdmp.bin", (void*)loc, 0x10000);
        }
	}
	__asm("SVC 0x09");
}