/*************************
 *     Include files     *
 *************************/
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/mm.h>

#include <linux/delay.h>

#include <asm/pgtable.h>

#include "aflash_dev.h"

/*************************
 *    Local variables    *
 *************************/
#if 0
#define FlashDebug
 #endif

static FlashInfo *Flash_Info;
static FlashInfo aFlashInfo[] =
{
    {AMD_S29GL128N_DEV_ID_02, 0, AMD_S29GL128N_BLKCOUNT, AMD_S29GL128N_BLKSIZE},
    {AMD_S29GL256N_DEV_ID_02, 0, AMD_S29GL256N_BLKCOUNT, AMD_S29GL256N_BLKSIZE},
    {AMD_S29GL512N_DEV_ID_02, 0, AMD_S29GL512N_BLKCOUNT, AMD_S29GL512N_BLKSIZE},
     {0,}
};

/*=============================================================================*/
static DWORD flashGetId(FLASHDATA *aoFlashBase)
{
    FLASHDATA data1, data2, data3, data4;
    DWORD id;

    /*    Enter Autoselect Mode */
    lld_AutoselectEntryCmd(aoFlashBase);
ssleep(1);
    /*   Read Autoselect Addresses.  Expect Autoselect Data */
    data1 = (lld_ReadOp(aoFlashBase, 0x00))&0xff;
    data2 = (lld_ReadOp(aoFlashBase, 0x01))&0xff;
    data3 = (lld_ReadOp(aoFlashBase, 0x0E))&0xff;
    data4 = (lld_ReadOp(aoFlashBase, 0x0F))&0xff;
ssleep(1);
    /*    Exit Autoselect Mode */
    lld_AutoselectExitCmd(aoFlashBase);
ssleep(1);
    id = (data1<<24)+(data2<<16)+(data3<<8)+data4;
    #ifdef FlashDebug
    printf("data1 : 0x%02X\n", data1);
    printf("data2 : 0x%02X\n", data2);
    printf("data3 : 0x%02X\n", data3);
    printf("data4 : 0x%02X\n", data4);
    printf("id : 0x%08X\n", id);
    #endif

    return id;
}
/*=============================================================================*/
DEVSTATUS amd_flashblockerase(ADDRESS aoFlashBase)
{
    DEVSTATUS status;

    aoFlashBase &= 0xfffe0000;
     /*---
      * Sector Erase Operation
      * to use this function, enter base address of flash device
      * and offset within sector of interest
      ---*/
    #ifdef FlashDebug
    printf("\nFlashBlockErase : 0x%08x", aoFlashBase);
    #endif
    status = lld_SectorEraseOp((FLASHDATA *)aoFlashBase, 0);
    #ifdef FlashDebug
    if (status!=DEV_NOT_BUSY)
         printf("amdFlashBlockErase() : Error!\n");
     #endif
    return status;
}
/*=============================================================================*/
DEVSTATUS amd_flashchiperase(ADDRESS aoFlashBase)
{
    DEVSTATUS status;
     int lockKey = 0;

    aoFlashBase &= 0xfffe0000;
     /*---
      * Sector Erase Operation
      * to use this function, enter base address of flash device
      * and offset within sector of interest
      ---*/
    #ifdef FlashDebug
    printf("\nFlashCHipErase : 0x%08x", aoFlashBase);
    #endif
    status = lld_ChipEraseOp((FLASHDATA *)aoFlashBase);
    #ifdef FlashDebug
    if (status!=DEV_NOT_BUSY)
         printf("amdFlashChipErase() : Error!\n");
     #endif
    return status;
}
/*=============================================================================*/
FlashInfo *amd_flashgetinfo(ADDRESS aoFlashBase)
{
    register unsigned n = 0;
    DWORD id;
    FlashInfo   *info = aFlashInfo;

    id = flashGetId((FLASHDATA *)aoFlashBase);
    /*  Check the manufacturer code and the device code for Spansion, Numonyx and MXIC Flash */
    if ( ((((id>>24)&0xff) == AMD_MFG_ID)||(((id>>24)&0xff) == NUMONYX_MFG_ID)||(((id>>24)&0xff) == MXIC_MFG_ID))&&
		(((id>>16)&0xff) == AMD_DEV_ID_01)&&
		((id&0xff) == AMD_DEV_ID_03))
    {
        id = (id>>8)&0xff;
        for (;(n = info->uDeviceId) != 0;info++)
            if (n == id)
                break;
    }
    return n ? info : 0;
}
/*=============================================================================*/
void amd_flashwrite(ADDRESS aoFlashBase, unsigned long uOffset, void *buf, unsigned long buflen)
{
    unsigned long len;
    FLASHDATA *poSrc = buf;
    FLASHDATA data[0x10];
    FLASHDATA datatemp;
    DEVSTATUS status;
    int i;

    while (buflen > 0)
    {
        if (amd_flashblockerase(aoFlashBase)==DEV_NOT_BUSY)
        {
            uOffset = 0;
            while (uOffset < (Flash_Info->uBlockInfo>>1) && buflen > 0)
            {
                len = (buflen >= 0x20) ? 0x10 : (buflen+1)>>1;
                for (i = 0; i<len; i++)
                   data[i] = (FLASHDATA)(*(poSrc+i));
    
                #ifdef FlashDebug
                printf("\nwrite addr : 0x%08x, len : 0x%08x", aoFlashBase+uOffset, buflen);
                #endif
                /*--- Write Buffer Programming ---*/
                ReDo:
                status = lld_WriteBufferProgramOp((FLASHDATA *)aoFlashBase, uOffset, len, (FLASHDATA *)data);
                if (status!=DEV_NOT_BUSY)
                {
                    #ifdef FlashDebug
                    printf("\namdFlashWrite() : Programming Data error!");
                    #endif
                    goto ReDo;
                }
    
                /*   Verify Programmed Data */
                for (i = 0; i<len; i++)
                {
                    datatemp = lld_ReadOp((FLASHDATA *)(aoFlashBase), (uOffset+i));
                    if (data[i] != (FLASHDATA)(datatemp&0xffff))
                    {
                        #ifdef FlashDebug
                        printf("\nverify : 0x%08x", aoFlashBase+uOffset);
                        printf("\nFlashWrite() : Verifying Data error!");
                        printf( "\nreaddata = 0x%02x, data[%d]=0x%02x", datatemp, i, data[i]);
                        #endif
                        goto ReDo;
                    }
                }
                poSrc += len;
                uOffset += len;
		  if (buflen>=len*sizeof(FLASHDATA))
                    buflen -= len*sizeof(FLASHDATA);
		  else
                    buflen = 0;
            }
            aoFlashBase += Flash_Info->uBlockInfo;
        }
    }
}
/*=============================================================================*/
void amd_flashread(void *dstAddr, ADDRESS srcAddr, unsigned long length)
{
    FLASHDATA data_read;
    FLASHDATA *poSrc = dstAddr;

    while (length)
    {
        data_read = lld_ReadOp((FLASHDATA *)srcAddr, 0);
        *poSrc++ = data_read;
        srcAddr += sizeof(FLASHDATA);
        if (length>=sizeof(FLASHDATA))
            length -= sizeof(FLASHDATA);
        else
            length = 0;
    }	
}
/*=============================================================================*/
int amd_flashInit(ADDRESS aoFlashBase)
{
    /*    Reset device to place into Read Mode */
    lld_ResetCmd((FLASHDATA *)aoFlashBase);
//	ssleep(2);
    Flash_Info = amd_flashgetinfo(aoFlashBase);
    if (!Flash_Info)
        return -1;

    printk(KERN_INFO "Flash Information :\n");
    printk(KERN_INFO "  Device Id         : 0x%08x\n", Flash_Info->uDeviceId);
    printk(KERN_INFO "  Device Flags      : 0x%08x\n", Flash_Info->uDeviceFlags);
    printk(KERN_INFO "  Device Blocks     : 0x%08x\n", Flash_Info->nBlocks);
    printk(KERN_INFO "  Device Block Info : 0x%08lx\n", Flash_Info->uBlockInfo);

    return 0;
}
/*=============================================================================*/

