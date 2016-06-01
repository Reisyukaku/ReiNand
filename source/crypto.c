// From http://github.com/b1l1s/ctr

#include "crypto.h"

#include <stddef.h>
#include "memory.h"
#include "fatfs/sdmmc/sdmmc.h"
#include "fatfs/ff.h"

/****************************************************************
*                   Crypto Libs
****************************************************************/

/* original version by megazig */

#ifndef __thumb__
#define BSWAP32(x) {\
	__asm__\
	(\
		"eor r1, %1, %1, ror #16\n\t"\
		"bic r1, r1, #0xFF0000\n\t"\
		"mov %0, %1, ror #8\n\t"\
		"eor %0, %0, r1, lsr #8\n\t"\
		:"=r"(x)\
		:"0"(x)\
		:"r1"\
	);\
};

#define ADD_u128_u32(u128_0, u128_1, u128_2, u128_3, u32_0) {\
__asm__\
	(\
		"adds %0, %4\n\t"\
		"addcss %1, %1, #1\n\t"\
		"addcss %2, %2, #1\n\t"\
		"addcs %3, %3, #1\n\t"\
		: "+r"(u128_0), "+r"(u128_1), "+r"(u128_2), "+r"(u128_3)\
		: "r"(u32_0)\
		: "cc"\
	);\
}
#else
#define BSWAP32(x) {x = __builtin_bswap32(x);}

#define ADD_u128_u32(u128_0, u128_1, u128_2, u128_3, u32_0) {\
__asm__\
	(\
		"mov r4, #0\n\t"\
		"add %0, %0, %4\n\t"\
		"adc %1, %1, r4\n\t"\
		"adc %2, %2, r4\n\t"\
		"adc %3, %3, r4\n\t"\
		: "+r"(u128_0), "+r"(u128_1), "+r"(u128_2), "+r"(u128_3)\
		: "r"(u32_0)\
		: "cc", "r4"\
	);\
}
#endif /*__thumb__*/

void aes_setkey(u8 keyslot, const void* key, u32 keyType, u32 mode)
{
	if(keyslot <= 0x03) return; // Ignore TWL keys for now
	u32* key32 = (u32*)key;
	*REG_AESCNT = (*REG_AESCNT & ~(AES_CNT_INPUT_ENDIAN | AES_CNT_INPUT_ORDER)) | mode;
	*REG_AESKEYCNT = (*REG_AESKEYCNT >> 6 << 6) | keyslot | AES_KEYCNT_WRITE;

	REG_AESKEYFIFO[keyType] = key32[0];
	REG_AESKEYFIFO[keyType] = key32[1];
	REG_AESKEYFIFO[keyType] = key32[2];
	REG_AESKEYFIFO[keyType] = key32[3];
}

void aes_use_keyslot(u8 keyslot)
{
	if(keyslot > 0x3F)
		return;

	*REG_AESKEYSEL = keyslot;
	*REG_AESCNT = *REG_AESCNT | 0x04000000; /* mystery bit */
}

void aes_setiv(const void* iv, u32 mode)
{
	const u32* iv32 = (const u32*)iv;
	*REG_AESCNT = (*REG_AESCNT & ~(AES_CNT_INPUT_ENDIAN | AES_CNT_INPUT_ORDER)) | mode;

	// Word order for IV can't be changed in REG_AESCNT and always default to reversed
	if(mode & AES_INPUT_NORMAL)
	{
		REG_AESCTR[0] = iv32[3];
		REG_AESCTR[1] = iv32[2];
		REG_AESCTR[2] = iv32[1];
		REG_AESCTR[3] = iv32[0];
	}
	else
	{
		REG_AESCTR[0] = iv32[0];
		REG_AESCTR[1] = iv32[1];
		REG_AESCTR[2] = iv32[2];
		REG_AESCTR[3] = iv32[3];
	}
}

void aes_advctr(void* ctr, u32 val, u32 mode)
{
	u32* ctr32 = (u32*)ctr;
	
	int i;
	if(mode & AES_INPUT_BE)
	{
		for(i = 0; i < 4; ++i) // Endian swap
			BSWAP32(ctr32[i]);
	}
	
	if(mode & AES_INPUT_NORMAL)
	{
		ADD_u128_u32(ctr32[3], ctr32[2], ctr32[1], ctr32[0], val);
	}
	else
	{
		ADD_u128_u32(ctr32[0], ctr32[1], ctr32[2], ctr32[3], val);
	}
	
	if(mode & AES_INPUT_BE)
	{
		for(i = 0; i < 4; ++i) // Endian swap
			BSWAP32(ctr32[i]);
	}
}

void aes_change_ctrmode(void* ctr, u32 fromMode, u32 toMode)
{
	u32* ctr32 = (u32*)ctr;
	int i;
	if((fromMode ^ toMode) & AES_CNT_INPUT_ENDIAN)
	{
		for(i = 0; i < 4; ++i)
			BSWAP32(ctr32[i]);
	}

	if((fromMode ^ toMode) & AES_CNT_INPUT_ORDER)
	{
		u32 temp = ctr32[0];
		ctr32[0] = ctr32[3];
		ctr32[3] = temp;

		temp = ctr32[1];
		ctr32[1] = ctr32[2];
		ctr32[2] = temp;
	}
}

void aes_batch(void* dst, const void* src, u32 blockCount)
{
	*REG_AESBLKCNT = blockCount << 16;
	*REG_AESCNT |=	AES_CNT_START;
	
	const u32* src32	= (const u32*)src;
	u32* dst32			= (u32*)dst;
	
	u32 wbc = blockCount;
	u32 rbc = blockCount;
	
	while(rbc)
	{
		if(wbc && ((*REG_AESCNT & 0x1F) <= 0xC)) // There's space for at least 4 ints
		{
			*REG_AESWRFIFO = *src32++;
			*REG_AESWRFIFO = *src32++;
			*REG_AESWRFIFO = *src32++;
			*REG_AESWRFIFO = *src32++;
			wbc--;
		}
		
		if(rbc && ((*REG_AESCNT & (0x1F << 0x5)) >= (0x4 << 0x5))) // At least 4 ints available for read
		{
			*dst32++ = *REG_AESRDFIFO;
			*dst32++ = *REG_AESRDFIFO;
			*dst32++ = *REG_AESRDFIFO;
			*dst32++ = *REG_AESRDFIFO;
			rbc--;
		}
	}
}

void aes(void* dst, const void* src, u32 blockCount, void* iv, u32 mode, u32 ivMode)
{
	*REG_AESCNT =	mode |
					AES_CNT_INPUT_ORDER | AES_CNT_OUTPUT_ORDER |
					AES_CNT_INPUT_ENDIAN | AES_CNT_OUTPUT_ENDIAN |
					AES_CNT_FLUSH_READ | AES_CNT_FLUSH_WRITE;

	u32 blocks;
	while(blockCount != 0)
	{
		if((mode & AES_ALL_MODES) != AES_ECB_ENCRYPT_MODE
		&& (mode & AES_ALL_MODES) != AES_ECB_DECRYPT_MODE)
			aes_setiv(iv, ivMode);

		blocks = (blockCount >= 0xFFFF) ? 0xFFFF : blockCount;

		// Save the last block for the next decryption CBC batch's iv
		if((mode & AES_ALL_MODES) == AES_CBC_DECRYPT_MODE)
		{
			memcpy(iv, src + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
			aes_change_ctrmode(iv, AES_INPUT_BE | AES_INPUT_NORMAL, ivMode);
		}

		// Process the current batch
		aes_batch(dst, src, blocks);

		// Save the last block for the next encryption CBC batch's iv
		if((mode & AES_ALL_MODES) == AES_CBC_ENCRYPT_MODE)
		{
			memcpy(iv, dst + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
			aes_change_ctrmode(iv, AES_INPUT_BE | AES_INPUT_NORMAL, ivMode);
		}
		
		// Advance counter for CTR mode
		else if((mode & AES_ALL_MODES) == AES_CTR_MODE)
			aes_advctr(iv, blocks, ivMode);

		src += blocks * AES_BLOCK_SIZE;
		dst += blocks * AES_BLOCK_SIZE;
		blockCount -= blocks;
	}
}

void xor(u8 *dest, const u8 *data1, const u8 *data2, Size size){
    u32 i; for(i = 0; i < size; i++) *((u8*)dest+i) = *((u8*)data1+i) ^ *((u8*)data2+i);
}

/****************************************************************
*                   Nand/FIRM Crypto stuff
****************************************************************/

const u8 memeKey[0x10] = {
    0x52, 0x65, 0x69, 0x20, 0x69, 0x73, 0x20, 0x62, 
    0x65, 0x73, 0x74, 0x20, 0x67, 0x69, 0x72, 0x6C
};

//Sets up keys
void keyInit(void *armHdr){
    //If old3ds, skip n3ds parts
    if(PDN_MPCORE_CFG == 1) goto Legacy;
    if(UNITINFO != 0) return;
    
    //Nand key#2 (0x12C10)
    u8 key2[0x10] = {
        0x10, 0x5A, 0xE8, 0x5A, 0x4A, 0x21, 0x78, 0x53, 0x0B, 0x06, 0xFA, 0x1A, 0x5E, 0x2A, 0x5C, 0xBC
    };
    
    //Set 0x11 to key2 for the misc keys
    xor(key2, key2, memeKey, 0x10);
    aes_setkey(0x11, (u8*)key2, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x11);
    
    //Set keys 0x19..0x1F keyXs
    u8* decKey = (void*)((uPtr)armHdr+0x89824);
    memset(decKey, 0, 0x10);
    aes_use_keyslot(0x11);
    u8 slot; for(slot = 0x19; slot < 0x20; slot++) {
        aes(decKey, (void*)((uPtr)armHdr+0x89814), 1, NULL, AES_ECB_DECRYPT_MODE, 0);
        aes_setkey(slot, (u8*)decKey, AES_KEYX, AES_INPUT_BE | AES_INPUT_NORMAL);
        *(u8*)((void*)((uPtr)armHdr+0x89814+0xF)) += 1;
    }
    
    Legacy:;
    //Setup legacy keys
    u8 keyX_0x25[0x10] = {
        0x9C, 0x82, 0xB1, 0x8B, 0x59, 0xB3, 0x2D, 0xCC, 
        0xE0, 0x7D, 0x81, 0xC3, 0xE5, 0xC5, 0x28, 0x9F
    };
    xor(keyX_0x25, keyX_0x25, memeKey, 0x10);
    aes_setkey(0x25, (u8*)keyX_0x25, AES_KEYX, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x25);
}

//Decrypt firmware blob
void decryptFirm(void *firm, Size firmSize){
    u8 firmIV[0x10] = {0};
    aes_setkey(0x16, memeKey, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x16);
    aes(firm, firm, firmSize / AES_BLOCK_SIZE, firmIV, AES_CBC_DECRYPT_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);
}