/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sdmmc/sdmmc.h"
#include "../crypto.h"
#include <string.h>

/* Definitions of physical drive number for each media */
#define SDCARD        0
#define CTRNAND       1

__attribute__((aligned(4))) static u8 nandCTR[AES_BLOCK_SIZE];
static u8 nandKeySlot;
static u32 fatStart;

void nandInit(void){
	__attribute__((aligned(4))) u8 CID[AES_BLOCK_SIZE];
	__attribute__((aligned(4))) u8 SHA_Sum[SHA_256_HASH_SIZE];

	sdmmc_get_cid(1, (u32*)CID);
	sha(SHA_Sum, CID, AES_BLOCK_SIZE, SHA_256_MODE);
	memcpy(nandCTR, SHA_Sum, sizeof(nandCTR));

	if (ISN3DS){
		fatStart = 0x5CAD7;
		nandKeySlot = 0x05;
	} else {
		fatStart = 0x5CAE5;
        nandKeySlot = 0x04;
	}
}

u32 nandRead(u32 sectorNum, u32 sectorCount, u8* dest){
	__attribute__((aligned(4))) u8 tempCTR[AES_BLOCK_SIZE];
	memcpy(tempCTR, nandCTR, AES_BLOCK_SIZE);
	aes_advctr(tempCTR, ((sectorNum + fatStart) * 0x200) / AES_BLOCK_SIZE, AES_INPUT_BE | AES_INPUT_NORMAL);

	u32 result = sdmmc_nand_readsectors(sectorNum + fatStart, sectorCount, dest);

	aes_use_keyslot(nandKeySlot);
	aes(dest, dest, (sectorCount * 0x200) / AES_BLOCK_SIZE, tempCTR, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

	return result;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	__attribute__((unused))
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
        DSTATUS ret;
        static u32 sdmmcInitResult = 4;

        if(sdmmcInitResult == 4) sdmmcInitResult = sdmmc_sdcard_init();

        if(pdrv == CTRNAND)
        {
            if(!(sdmmcInitResult & 1))
            {
                nandInit();
                ret = 0;
            }
            else ret = STA_NOINIT;
        }
        else ret = (!(sdmmcInitResult & 2)) ? 0 : STA_NOINIT;

	return ret;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
        return ((pdrv == SDCARD && !sdmmc_sdcard_readsectors(sector, count, buff)) ||
                (pdrv == CTRNAND && !nandRead(sector, count, buff))) ? RES_OK : RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,       	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	(void)pdrv;
	(void)buff;
	(void)sector;
	(void)count;

    return ((pdrv == SDCARD && !sdmmc_sdcard_writesectors(sector, count, buff))) ? RES_OK : RES_PARERR;
}
#endif



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	__attribute__((unused))
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	__attribute__((unused))
	BYTE cmd,		/* Control code */
	__attribute__((unused))
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_PARERR;
}
#endif
