/*****************************************************************************
**
** COMPONENT: PCI driver Public Header file
**
** $file: pciDrv.h $
**
** $Date: $
**
** $Revision: 1.00 $
**
** $Author: $
**
** DESCRIPTION: This header file contains all the protos & declarations
**              for PCI driver
**
** NOTES:       None.
**
*****************************************************************************/

#ifndef _PCI_DRV_H
#define _PCI_DRV_H

/*
** Include Files
*/

/*
** Constants Defines
*/
#define PCIDRV_PATH           "/dev/pci"

/* PCI driver IOCTL command */
#define PCIDRV_SET_ADDR_CMD   0
#define PCIDRV_GET_INFO_CMD   1

#define PCIDRV_MAJOR  244
#define PCIDRV_MINOR  0

#endif  /* _PCI_DRV_H */
