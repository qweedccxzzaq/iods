/* vim:ts=4:sw=4:expandtab
 * (No tabs, indent level is 4 spaces)
 *
 * i2cDrv_main.c
 * Device Drivers for LB6 Platform
 * Written for Linux Kernel versions in the 2.4 series.
 * Copyright (c) 2007 Quanta Computer Inc.
 *
 * This driver has the following features:
 *      - creates /dev/i2cDev device for easy access to i2c devices
 *
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
#include <linux/i2c.h>

#include "i2cDrv.h"

MODULE_LICENSE("GPL");

spinlock_t i2c_device_access_lock;

/*
 * debug mode bit-mask:
 *  bit         meaning when set
 *    0         debug activities
 *
 */
int debugMode = 0;

//MODULE_PARM(debugMode, "i");
module_param(debugMode, int, 0);
MODULE_PARM_DESC(debugMode, "Debug Mode Bit-Mask");

extern struct i2c_client i2cdev_client_temp;

/* i2c_drv_open
 *
 * Called when one of our device nodes is opened.
 *
 */
static int i2c_drv_open(struct inode *inode, struct file *file)
{
    int devnum = MINOR(inode->i_rdev);
    int err = 0;
    epfcb * fcb;
    struct i2c_adapter *adap;
    struct i2c_client *client;

    spin_lock(&i2c_device_access_lock);
    fcb = kmalloc(sizeof(epfcb), GFP_KERNEL);
    if (fcb == NULL)
    {
        spin_unlock(&i2c_device_access_lock);
        return -ENODEV;
    }
    else
    {
        memset(fcb, 0, sizeof(epfcb));
        fcb->inode = inode;
        fcb->devnum = devnum;
        file->private_data = fcb;
    }

    /* Check for valid minor number */
    if (err == 0)
    {
        switch (devnum)
        {
            case I2C_MINOR_ID:
            case I2C_MINOR_ID_2:
                err = i2c_device_open(fcb, file);
                break;

            default:
                err = -ENODEV;
                break;
        }
    }
    if (err!=0)
    {
        spin_unlock(&i2c_device_access_lock);
        return -ENODEV;
    }

    adap = i2c_get_adapter(devnum);
    if (adap == NULL)
    {
        spin_unlock(&i2c_device_access_lock);
        return -ENODEV;
    }
    client = kmalloc(sizeof(*client), GFP_KERNEL);
    if (client == NULL)
    {
        i2c_put_adapter(adap);
        spin_unlock(&i2c_device_access_lock);
        return -ENOMEM;
    }
    memcpy(client, &i2cdev_client_temp, sizeof(*client));
    client->adapter = adap;
    fcb->client= client;
/*
    pr_info("%s : adap=0x%08x\n", __FUNCTION__, (int)adap->algo->master_xfer);
*/
    return err;
}


/* i2c_drv_seek
 *
 * Called when a seek command has been issued for a device.
 *
 */
static loff_t i2c_drv_seek(struct file *file, loff_t offset, int whence)
{
    int err;
    epfcb * fcb;

    fcb = (epfcb *) file->private_data;

    switch (fcb->devnum)
    {
        case I2C_MINOR_ID:
        case I2C_MINOR_ID_2:
            err = i2c_device_seek(file, offset, whence);
            break;

        default:
            err = 0;
            break;
    }

    return err;
}


/* i2c_drv_read
 *
 * Called when a read command has been issued for a device.
 *
 */
static ssize_t i2c_drv_read(struct file *file, char * buf, size_t count,
    loff_t *offsetptr)
{
    ssize_t retval;
    epfcb * fcb;

    fcb = (epfcb *) file->private_data;

    switch (fcb->devnum)
    {
        case I2C_MINOR_ID:
        case I2C_MINOR_ID_2:
            retval = i2c_device_read(file, buf, count, offsetptr);
            break;

        default:
            retval = -ENODEV;
            break;
    }

    return retval;
}


/* i2c_drv_write
 *
 * Called when a write command has been issued for a device.
 *
 */
static ssize_t i2c_drv_write(struct file *file, const char * buf, size_t count,
    loff_t * offsetptr)
{
    ssize_t retval;
    epfcb * fcb;

    fcb = (epfcb *) file->private_data;

    switch (fcb->devnum)
    {
        case I2C_MINOR_ID:
        case I2C_MINOR_ID_2:
            retval = i2c_device_write(file, buf, count, offsetptr);
            break;

        default:
            retval = -ENODEV;
            break;
    }

    return retval;
}

/* i2c_drv_ioctl()
 *
 * ioctl() handler for the eval platform devices
 */

static int i2c_drv_ioctl(struct inode *inode, struct file *file,
    unsigned int cmd, unsigned long arg)
{
    int err = 0;
    int devnum = MINOR(inode->i_rdev);

    switch (devnum)
    {
        case I2C_MINOR_ID:
        case I2C_MINOR_ID_2:
            err = i2c_device_ioctl(inode, file, cmd, arg);
            break;

        default:
            err = -ENODEV;
            break;
    }

    return err;
}


/* release
 *
 * Called when the last user of a device closes the device.
 *
 */
static int i2c_drv_release(struct inode *inode, struct file *file)
{
    int err;
    int devnum = MINOR(inode->i_rdev);
    epfcb * fcb = (epfcb *) file->private_data;
    struct i2c_client *client = (struct i2c_client *)fcb->client;

    i2c_put_adapter(client->adapter);
    kfree(client);

    switch (devnum)
    {
        case I2C_MINOR_ID:
        case I2C_MINOR_ID_2:
            err = i2c_device_release(inode, file);
            break;

        default:
            err = -ENODEV;
            break;
    }

    kfree(fcb);

    spin_unlock(&i2c_device_access_lock);

    return err;
}


static struct file_operations i2c_drv_fops =
{
    owner:      THIS_MODULE,
    llseek:     i2c_drv_seek,
    read:       i2c_drv_read,
    write:      i2c_drv_write,
/*  readdir:    */
/*  poll:       */
    ioctl:      i2c_drv_ioctl,
/*  mmap:       */
    open:       i2c_drv_open,
/*  flush:      */
    release:    i2c_drv_release,
/*  fasync:     */
/*  lock:       */
/*  readv:      */
/*  writev:     */
};


/* i2c_drv_init_module
 *
 * called when the device driver is being loaded
 *
 */
int __init i2c_drv_init_module(void)
{
    int err = 0;

    /* Initialize variables */

    printk(KERN_INFO "\nI2C Device Drivers (c) 2007 Quanta Computer Inc.\n");

    if (debugMode)
        printk(KERN_ERR PFX "Debug Mode set to 0x%x\n", debugMode);

    /* create the device */
    err = register_chrdev( I2C_MAJOR_DEVICE_ID, I2C_DRIVER_NAME, &i2c_drv_fops);
    if (err >= 0)
    {
        printk(KERN_INFO "Registered with major %d\n", I2C_MAJOR_DEVICE_ID);
        err = i2c_device_init();
        if (err<0)
        {
            printk(KERN_ERR PFX "i2c_device module initialization failed, error=%d.\n", err);
            return err;
        }

        /* initialize everything else */
        spin_lock_init(&i2c_device_access_lock);
    }
    else
    {
        printk(KERN_ERR PFX "module initialization failed, error=%d.\n", err);
    }

    return err;
}

/* i2c_drv_cleanup_module
 *
 * called when the device driver is being unloaded
 *
 */
void __exit i2c_drv_cleanup_module(void)
{
    i2c_device_cleanup();
    unregister_chrdev(I2C_MAJOR_DEVICE_ID, I2C_DRIVER_NAME);
}


/* Module entry-points */
module_init(i2c_drv_init_module);
module_exit(i2c_drv_cleanup_module);

MODULE_AUTHOR("Raymond Huey");
MODULE_DESCRIPTION("LB8 I2C Device Drivers");
