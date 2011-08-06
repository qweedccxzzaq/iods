/* vim:ts=4:sw=4:expandtab
 * (No tabs, indent level is 4 spaces)
 *
 * cpuDrv_main.c
 * Device Drivers for LB6 Platform
 * Written for Linux Kernel versions in the 2.4 series.
 * Copyright (c) 2007 Quanta Computer Inc.
 *
 * This driver has the following features:
 *      - creates /dev/cpuDev device for easy access to cpu devices
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

#include "cpuDrv_main.h"

MODULE_LICENSE("GPL");
phys_addr_t get_immrbase(void);
/* cpu_drv_open
 *
 * Called when one of our device nodes is opened.
 *
 */
static int cpu_drv_open(struct inode *inode, struct file *file)
{
    return 0;
}

/* release
 *
 * Called when the last user of a device closes the device.
 *
 */
static int cpu_drv_release(struct inode *inode, struct file *file)
{
    return 0;
}

/* mmap
 *
 * Called when someone wants to memory map the packet transfer
 * pool into user space so that allocated buffers can be filled
 * with frame data
*/
int cpu_drv_mmap(struct file *file, struct vm_area_struct *vma)
{

    /* we do not want to have this area swapped out, lock it */
    vma->vm_flags |= VM_IO | VM_RESERVED;

    if (remap_pfn_range(vma, vma->vm_start,
                         0xE0000000 >> PAGE_SHIFT,
                         0x01000000,
                         PAGE_SHARED | __pgprot(_PAGE_NO_CACHE))) {
         printk("cpu_drv_mmap: remap page range failed\n");
         return -ENXIO;
    }

    return 0;
}

int cpu_drv_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0, err = 0;
    phys_addr_t immrbase;
    void *devBase;   

 
    if(_IOC_TYPE(cmd) != cpuDrv_IOC_MAGIC) return -ENOTTY;
    if(_IOC_NR(cmd) > cpuDrv_IOC_MAXNR) return -ENOTTY;

    if(_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if(_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

    if(err) return -EFAULT;

    immrbase = get_immrbase();
    switch(cmd) {
        case cpuDrv_IOCGCCSR:
            printk(KERN_DEBUG "OFFSET: 0x%x, %x\n", *((int *)arg), (phys_addr_t)(immrbase + *((int *)arg)));
            devBase = ioremap((phys_addr_t)(immrbase + *((int *)arg)), 0x4);
            printk(KERN_DEBUG "&devBase: %x, %x\n", (int *)devBase, *((int *)devBase));
            retval = __put_user(*((int *)devBase), (int __user *)arg);
            break;
    }
    
    iounmap(devBase);
    return retval;
}

static struct file_operations cpu_drv_fops =
{
    owner:      THIS_MODULE,
/*    llseek:*/
/*    read:*/
/*    write:*/
/*  readdir:    */
/*  poll:       */
    ioctl:  cpu_drv_ioctl,
    mmap:   cpu_drv_mmap,
    open:       cpu_drv_open,
/*  flush:      */
    release:    cpu_drv_release,
/*  fasync:     */
/*  lock:       */
/*  readv:      */
/*  writev:     */
};


/* cpu_drv_init_module
 *
 * called when the device driver is being loaded
 *
 */
int __init cpu_drv_init_module(void)
{
    int err = 0;

    /* Initialize variables */

    printk(KERN_INFO "\nCPU MMAP Driver (c) 2007 Quanta Computer Inc.\n");

    /* create the device */
    err = register_chrdev( CPU_MAJOR_DEVICE_ID, CPU_DRIVER_NAME, &cpu_drv_fops);
    if (err >= 0)
    {
        printk(KERN_INFO "Registered with major %d\n", CPU_MAJOR_DEVICE_ID);
    }
    else
    {
        printk(KERN_ERR PFX "module initialization failed, error=%d.\n", err);
    }

    return err;
}

/* cpu_drv_cleanup_module
 *
 * called when the device driver is being unloaded
 *
 */
void __exit cpu_drv_cleanup_module(void)
{
    unregister_chrdev(CPU_MAJOR_DEVICE_ID, CPU_DRIVER_NAME);
}


/* Module entry-points */
module_init(cpu_drv_init_module);
module_exit(cpu_drv_cleanup_module);

MODULE_AUTHOR("Raymond Huey");
MODULE_DESCRIPTION("LB4G CPU MMAP Driver");

