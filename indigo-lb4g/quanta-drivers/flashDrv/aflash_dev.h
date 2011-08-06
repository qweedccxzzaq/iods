#ifndef AFLASH_ROM_DEFINES
#define AFLASH_ROM_DEFINES

#include "lld.h"

/* Manufacture and Device ID */
#define AMD_MFG_ID      	         	   	0x01
#define NUMONYX_MFG_ID      	         	0x89
#define MXIC_MFG_ID      	         		0xC2

#define AMD_DEV_ID_01                 	0x7E
#define AMD_S29GL128N_DEV_ID_02   	0x21
#define AMD_S29GL256N_DEV_ID_02   	0x22
#define AMD_S29GL512N_DEV_ID_02   	0x23
#define AMD_DEV_ID_03                 	0x01

/* Block count */
#define AMD_S29GL128N_BLKCOUNT                	128
#define AMD_S29GL256N_BLKCOUNT                	256
#define AMD_S29GL512N_BLKCOUNT                	512

/* Block Size */
#define AMD_S29GL128N_BLKSIZE                 	0x20000L
#define AMD_S29GL256N_BLKSIZE                 	0x20000L
#define AMD_S29GL512N_BLKSIZE                 	0x20000L

/* Flash ROM size */
#define AMD_S29GL128N_ROMSIZE                 (AMD_S29GL128N_BLKSIZE * AMD_S29GL128N_BLKCOUNT) /* 128 MBit */
#define AMD_S29GL256N_ROMSIZE                 (AMD_S29GL256N_BLKSIZE * AMD_S29GL256N_BLKCOUNT) /* 256 MBit */
#define AMD_S29GL512N_ROMSIZE                 (AMD_S29GL512N_BLKSIZE * AMD_S29GL512N_BLKCOUNT) /* 512 MBit */

#define H_EBUF_SIZE     (256)  /* Error Buffer Size */

typedef struct tagFlashInfo {
  unsigned uDeviceId;
  unsigned uDeviceFlags;
  unsigned nBlocks;
  unsigned long uBlockInfo;
} FlashInfo;

extern int amd_flashInit(ADDRESS aoFlashBase);
extern DEVSTATUS amd_flashblockerase(ADDRESS aoFlashBase);
extern DEVSTATUS amd_flashchiperase(ADDRESS aoFlashBase);
extern FlashInfo *amd_flashgetinfo(ADDRESS aoFlashBase);
extern void amd_flashwrite(ADDRESS aoFlashBase, unsigned long uOffset, void *buf, unsigned long buflen);
extern void amd_flashread(void *dstAddr, ADDRESS srcAddr, unsigned long length);
#endif /* IFLASH_ROM_DEFINES */
