/*
*   thread.h
*       by Reisyukaku
*/

//ram stuff
#define VRAM (unsigned char*)0x18000000
#define FCRAM (unsigned char*)0x20000000
#define FCRAM_EXT (unsigned char*)0x28000000

//file stuff
#define READ 0
#define WRITE 1

void fileReadWrite(void *buf, void *path, int size, char rw);
void memdump(void* filename, void* buf, unsigned int size);
void patches(void);
void thread(void);