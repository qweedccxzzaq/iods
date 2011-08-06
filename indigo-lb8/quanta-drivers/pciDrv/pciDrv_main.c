/*******************************************************************************
**
** MODULE: PCI Driver
**
** $file: pciDrv_main.c $
**
** $Date: $
**
** $Revision: 1.00 $
**
** DESCRIPTION : This file contains PCI driver template
**
*******************************************************************************/

/*
** Include Files
*/
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <asm/pgtable.h>

#include "pciDrv.h"


/*
** Private Defines
*/
#define PCIDRV_VER "0.10"


/*
** Private Function Protoss
*/
static int pciDrv_read(struct file *file, char *buf, size_t count, loff_t *offset);
static int pciDrv_write(struct file *file, const char *buf, size_t count, loff_t *offset);
static int pciDrv_ioctl(struct inode *inode, struct file *file, unsigned int vendor, unsigned int device);
static int pciDrv_open(struct inode *inode, struct file *file);
static int pciDrv_release(struct inode *inode, struct file *file);

/*
** Local Variables
*/
static unsigned char     *pciDrvName = NULL;
static struct semaphore pciDrvSem;

/* file operation struct */
static struct file_operations pciDrv_fops =
{
    owner:      THIS_MODULE,
    read:       pciDrv_read,
    write:      pciDrv_write,
    ioctl:      pciDrv_ioctl,
    open:       pciDrv_open,
    release:    pciDrv_release,
};

#define DRV_LOCK()      down(&pciDrvSem)
#define DRV_UNLOCK()    up(&pciDrvSem)

/*
** Private Function Definitions
*/

/*******************************************************************************
**
**  FUNCTION: pciDrv_read
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Read PCI driver IO memory
**
**  INPUTS:         file            - file structure
**                  count           - read count
**                  offset          - nothing
**
**  OUTPUTS:        buf             - read buffer
**
**  RETURNS:        function result
**
*******************************************************************************/
static int pciDrv_read(struct file *file, char *buf, size_t count, loff_t *offset)
{
    struct inode   *inode = file->f_dentry->d_inode;

    if (MINOR(inode->i_rdev) != PCIDRV_MINOR)
        return  -EFAULT;

     return  0;
}


/*******************************************************************************
**
**  FUNCTION: pciDrv_write
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Write PCI driver IO memory
**
**  INPUTS:         file            - file structure
**                  buf             - write buffer
**                  count           - write count
**                  offset          - nothing
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int pciDrv_write(struct file *file, const char *buf, size_t count, loff_t *offset)
{
    struct inode   *inode = file->f_dentry->d_inode;

    if (MINOR(inode->i_rdev) != PCIDRV_MINOR)
        return  -EFAULT;


    return  0;
}


/*******************************************************************************
**
**  FUNCTION: pciDrv_ioctl
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Do PCI driver IO control
**
**  INPUTS:         inode           - inode structure
**                  file            - file structure
**                  vendor          - vendor id
**                  device          - device id
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int pciDrv_ioctl(struct inode *inode, struct file *file, unsigned int vendor, unsigned int device)
{
    struct pci_dev *pdev;

    if( MINOR(inode->i_rdev) != PCIDRV_MINOR )
        return  -EFAULT;

    pdev = pci_find_device(vendor, device, NULL);
    if(!pdev)
	 return  -EFAULT; /*Failed to find any matching devices*/
  
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: pciDrv_open
**  ___________________________________________________________________________
**
**  DESCRIPTION:    open PCI driver file descriptor
**
**  INPUTS:         inode           - inode structure
**                  file            - file structure
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int pciDrv_open(struct inode *inode, struct file *file)
{
    DRV_LOCK();

    /* return OK */
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: pciDrv_release
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Release PCI driver file descriptor
**
**  INPUTS:         inode           - inode structure
**                  file            - file structure
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int pciDrv_release(struct inode *inode, struct file *file)
{
    DRV_UNLOCK();

    /* return OK */
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: pciDrv_init
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Init. PCI driver module
**
**  INPUTS:         NONE
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int pciDrv_init(void)
{
    int         res;

    printk(KERN_INFO "\nPCI Driver v%s (c) 2007 Quanta Computer Inc.\n", PCIDRV_VER);

    if(pciDrvName == NULL)
        pciDrvName = "pciDrv";

    res = register_chrdev(PCIDRV_MAJOR, pciDrvName, &pciDrv_fops);

    if(res < 0)
    {
        printk(KERN_INFO "Cannot register PCI device with kernel\n");
        return  res;
    }
    else
    {
        printk(KERN_INFO "Registered with major %d\n", PCIDRV_MAJOR);
    }

     /* need to init. mutex/semaphore
       ... */
    init_MUTEX(&pciDrvSem);

    /* return OK */
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: pciDrv_exit
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Exit PCI driver module
**
**  INPUTS:         NONE
**
**  OUTPUTS:        NONE
**
**  RETURNS:        NONE
**
*******************************************************************************/
static void pciDrv_exit(void)
{
    /* unregister device driver */
    unregister_chrdev(PCIDRV_MAJOR, "pciDrv");
    return;
}


module_init(pciDrv_init);
module_exit(pciDrv_exit);

MODULE_LICENSE("GPL");
