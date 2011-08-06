/*******************************************************************************
**
** MODULE: Flash Driver
**
** $file: flash.c $
**
** $Date: $
**
** $Revision: 1.00 $
**
** DESCRIPTION : This file contains AMD NOR-type Flash driver template
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

#include <linux/delay.h>

#include <asm/pgtable.h>

#include "flashDrv.h"
#include "aflash_dev.h"


/*
** Private Defines
*/
#define FLASHDRV_VER "0.10"


/*
** Private Function Protoss
*/
static int  flashDrv_read(struct file *file, char *buf, size_t count, loff_t *offset);
static int  flashDrv_write(struct file *file, const char *buf, size_t count, loff_t *offset);
static int flashDrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int flashDrv_open(struct inode *inode, struct file *file);
static int flashDrv_release(struct inode *inode, struct file *file);

/*
** Local Variables
*/
static unsigned int     flashPhyAddr;
static unsigned int     flashPhyCmd;
static unsigned char     *flashDrvName = NULL;
static struct semaphore flashDrvSem;

/* file operation struct */
static struct file_operations flashDrv_fops =
{
    owner:      THIS_MODULE,
    read:       flashDrv_read,
    write:      flashDrv_write,
    ioctl:      flashDrv_ioctl,
    open:       flashDrv_open,
    release:    flashDrv_release,
};

#define DRV_LOCK()      down(&flashDrvSem)
#define DRV_UNLOCK()    up(&flashDrvSem)

/*
** Private Function Definitions
*/
static unsigned long flash_remap_addr;

/*******************************************************************************
**
**  FUNCTION: flashDrv_read
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Read Flash driver IO memory
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
static int flashDrv_read(struct file *file, char *buf, size_t count, loff_t *offset)
{
    struct inode   *inode = file->f_dentry->d_inode;
    unsigned long srcAddr;

    if (MINOR(inode->i_rdev) != FLASHDRV_MINOR)
        return  -EFAULT;

    if (flashPhyAddr < FLASH_MEM_ADDR_BEGIN ||flashPhyAddr > FLASH_MEM_ADDR_END)
        return  -EFAULT;

    srcAddr = flash_remap_addr + (flashPhyAddr - FLASH_MEM_ADDR_BEGIN);
    if (flashPhyCmd == FLASHDRV_SET_ADDR_CMD)
    {
        amd_flashread(buf, srcAddr, (unsigned long)count);
    }
    else if (flashPhyCmd == FLASHDRV_GET_INFO_CMD)
    {
        FlashInfo   *pFlashInfo;

        pFlashInfo = amd_flashgetinfo(srcAddr);
        memcpy(buf, (char *)pFlashInfo, sizeof(FlashInfo));
    }
    else
        return  -EFAULT;

    flashPhyAddr = 0;

    return  0;
}


/*******************************************************************************
**
**  FUNCTION: flashDrv_write
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Write Flash driver IO memory
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
static int flashDrv_write(struct file *file, const char *buf, size_t count, loff_t *offset)
{
    struct inode   *inode = file->f_dentry->d_inode;
    unsigned long aoFlashBase;

    if (MINOR(inode->i_rdev) != FLASHDRV_MINOR)
        return  -EFAULT;

    if (flashPhyAddr < FLASH_MEM_ADDR_BEGIN ||flashPhyAddr > FLASH_MEM_ADDR_END)
        return  -EFAULT;

    aoFlashBase = flash_remap_addr + (flashPhyAddr - FLASH_MEM_ADDR_BEGIN);
    amd_flashwrite(aoFlashBase, 0, (void *)buf, (unsigned long)count);
    flashPhyAddr = 0;

    return  0;
}


/*******************************************************************************
**
**  FUNCTION: flashDrv_ioctl
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Do Flash driver IO control
**
**  INPUTS:         inode           - inode structure
**                  file            - file structure
**                  cmd             - IO control command
**                  arg             - command argument
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int flashDrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{

    if( MINOR(inode->i_rdev) != FLASHDRV_MINOR )
        return  -EFAULT;

    /* need to apply mutex/semaphore
       ... */

    switch(cmd)
    {
        case FLASHDRV_SET_ADDR_CMD:

            if (arg < FLASH_MEM_ADDR_BEGIN ||
                arg > FLASH_MEM_ADDR_END ||
                flashPhyAddr )
                return  -EFAULT;

            /* set the Flash physical memory addr */
            flashPhyAddr = (unsigned int)arg;
	     flashPhyCmd = FLASHDRV_SET_ADDR_CMD;

        break;

        case FLASHDRV_GET_INFO_CMD:
            if (arg < FLASH_MEM_ADDR_BEGIN ||
                arg > FLASH_MEM_ADDR_END ||
                flashPhyAddr )
                return  -EFAULT;

            /* set the Flash physical memory addr */
            flashPhyAddr = (unsigned int)arg;
	     flashPhyCmd = FLASHDRV_GET_INFO_CMD;
        break;

        default:
            return  -ENOIOCTLCMD;
    }

    /* return OK */
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: flashDrv_open
**  ___________________________________________________________________________
**
**  DESCRIPTION:    open Flash driver file descriptor
**
**  INPUTS:         inode           - inode structure
**                  file            - file structure
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int flashDrv_open(struct inode *inode, struct file *file)
{
    DRV_LOCK();

    /* return OK */
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: flashDrv_release
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Release Flash driver file descriptor
**
**  INPUTS:         inode           - inode structure
**                  file            - file structure
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int flashDrv_release(struct inode *inode, struct file *file)
{
    DRV_UNLOCK();

    /* return OK */
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: flashDrv_init
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Init. Flash driver module
**
**  INPUTS:         NONE
**
**  OUTPUTS:        NONE
**
**  RETURNS:        function result
**
*******************************************************************************/
static int flashDrv_init(void)
{
    int         res;

    printk(KERN_INFO "\nFlash Driver v%s (c) 2007 Quanta Computer Inc.\n", FLASHDRV_VER);

    if(flashDrvName == NULL)
        flashDrvName = "flashDrv";

    res = register_chrdev(FLASHDRV_MAJOR, flashDrvName, &flashDrv_fops);

    if(res < 0)
    {
        printk(KERN_INFO "Cannot register Flash device with kernel\n");
        return  res;
    }
    else
    {
        printk(KERN_INFO "Registered with major %d\n", FLASHDRV_MAJOR);
    }

    flash_remap_addr = (unsigned long)ioremap(FLASH_MEM_ADDR_BEGIN, FLASH_MEM_NUMBYTE);

#if 1  /*LB9A LB4G project*/
    amd_flashInit(flash_remap_addr);
#else
    printk(KERN_INFO "\nFLASH 1 (0xFC000000~0xFDFFFFFF)\n");
    amd_flashInit(flash_remap_addr);
    printk(KERN_INFO "\nFLASH 2 (0xFE000000~0xFFFFFFFF)\n");
    amd_flashInit(flash_remap_addr+0x2000000);
#endif

    /* init. Flash physical address */
    flashPhyAddr  = 0;

    /* need to init. mutex/semaphore
       ... */
    init_MUTEX(&flashDrvSem);

    /* return OK */
    return  0;
}


/*******************************************************************************
**
**  FUNCTION: flashDrv_exit
**  ___________________________________________________________________________
**
**  DESCRIPTION:    Exit Flash driver module
**
**  INPUTS:         NONE
**
**  OUTPUTS:        NONE
**
**  RETURNS:        NONE
**
*******************************************************************************/
static void flashDrv_exit(void)
{
    /* unregister device driver */
    unregister_chrdev(FLASHDRV_MAJOR, "flashDrv");
    iounmap((void *)flash_remap_addr);
    return;
}


module_init(flashDrv_init);
module_exit(flashDrv_exit);

MODULE_LICENSE("GPL");
