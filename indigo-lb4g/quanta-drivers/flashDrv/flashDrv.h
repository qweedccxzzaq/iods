/*****************************************************************************
**
** COMPONENT: Flash driver Public Header file
**
** $file: flashDrv.h $
**
** $Date: $
**
** $Revision: 1.00 $
**
** $Author: $
**
** DESCRIPTION: This header file contains all the protos & declarations
**              for AMD NOR-type Flash driver
**
** NOTES:       None.
**
*****************************************************************************/

#ifndef _FLASH_DRV_H
#define _FLASH_DRV_H

/*
** Include Files
*/

/*
** Constants Defines
*/
#define FLASHDRV_PATH           "/dev/flash"

/* Flash driver IOCTL command */
#define FLASHDRV_SET_ADDR_CMD   0
#define FLASHDRV_GET_INFO_CMD   1

#define FLASHDRV_MAJOR  240
#define FLASHDRV_MINOR  1


#if 1  /*LB9A LB4G project*/
#define FLASH_MEM_ADDR_BEGIN    0xFE000000
#define FLASH_MEM_ADDR_END      0xFFFFFFFF
#define FLASH_MEM_NUMBYTE       0x02000000      /* 32MB per static memory bank */
#define FLASH_MEM_BLOCK_SIZE    0x20000         /* 128KB per flash block */
#define FLASH_MEM_BLOCK_BITMSK  0x1FFFF
#else
#define FLASH_MEM_ADDR_BEGIN    0xFC000000
#define FLASH_MEM_ADDR_END      0xFFFFFFFF
#define FLASH_MEM_NUMBYTE       0x04000000      /* 64MB per static memory bank */
#define FLASH_MEM_BLOCK_SIZE    0x20000         /* 128KB per flash block */
#define FLASH_MEM_BLOCK_BITMSK  0x1FFFF
#endif

#endif  /* _FLASH_DRV_H */
