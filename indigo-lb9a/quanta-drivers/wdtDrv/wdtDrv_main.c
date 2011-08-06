/* 
 *
 *
 * wdtDrv_main.c
 * Device Drivers for LB9A Platform
 * Written for Linux Kernel versions in the 2.6 series.
 * Copyright (c) 2008 Quanta Computer Inc.
 *
 * This driver has the following features:
 *      - creates /dev/wdtDev device for easy access to wdt devices
 *
 */

//#include <linux/config.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/notifier.h>
#include <linux/watchdog.h>

#include <asm/reg_booke.h>
#include <asm/uaccess.h>
#include <asm/system.h>


MODULE_LICENSE("GPL");

/* If the kernel parameter wdt_enable=1, the watchdog will be enabled at boot.
 * Also, the wdt_period sets the watchdog timer period timeout.
 * For E500 cpus the wdt_period sets which bit changing from 0->1 will
 * trigger a watchog timeout. This watchdog timeout will occur 3 times, the
 * first time nothing will happen, the second time a watchdog exception will
 * occur, and the final time the board will reset.
 */
#define WDTDRV_MAJOR  245
#define WDTDRV_VER "0.1"
static unsigned char     *wdtDrvName = NULL;


#ifdef	CONFIG_FSL_BOOKE
#define WDT_PERIOD_DEFAULT 10	/* Ex. wdt_period=28 bus=333Mhz , reset=~40sec */
#else
#define WDT_PERIOD_DEFAULT 4	/* Refer to the PPC40x and PPC4xx manuals */
#endif				/* for timing information */

u32 wdt_drv_enabled = 0;
u32 wdt_drv_period = WDT_PERIOD_DEFAULT;

#ifdef	CONFIG_FSL_BOOKE
#define WDTP(x)		((((63-x)&0x3)<<30)|(((63-x)&0x3c)<<15))
#else
#define WDTP(x)		(TCR_WP(x))
#endif

/*
 * booke_wdt_enable:
 */
static __inline__ void wdt_drv_enable(void)
{
	u32 val;

	val = mfspr(SPRN_TCR);
	val |= (TCR_WIE|TCR_WRC(WRC_CHIP)|WDTP(wdt_drv_period));

	mtspr(SPRN_TCR, val);
}

/*
 * booke_wdt_ping:
 */
static __inline__ void wdt_drv_ping(void)
{
	mtspr(SPRN_TSR, TSR_ENW|TSR_WIS);
}

/*
 * booke_wdt_write:
 */
static ssize_t wdt_drv_write (struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	wdt_drv_ping();
	return count;
}

static struct watchdog_info ident = {
  .options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
  .firmware_version = 0,
  .identity = "PowerPC Book-E Watchdog",
};

/*
 * booke_wdt_ioctl:
 */
static int wdt_drv_ioctl (struct inode *inode, struct file *file,
			    unsigned int cmd, unsigned long arg)
{
	u32 tmp = 0;
	u32 __user *p = (u32 __user *)arg;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		if (copy_to_user ((struct watchdog_info __user *) arg, &ident,
				sizeof(struct watchdog_info)))
			return -EFAULT;
	case WDIOC_GETSTATUS:
		return put_user(ident.options, p);
	case WDIOC_GETBOOTSTATUS:
		/* XXX: something is clearing TSR */
		tmp = mfspr(SPRN_TSR) & TSR_WRS(3);
		/* returns 1 if last reset was caused by the WDT */
		return (tmp ? 1 : 0);
	case WDIOC_KEEPALIVE:
		wdt_drv_ping();
		return 0;
	case WDIOC_SETTIMEOUT:
		if (get_user(wdt_drv_period, p))
			return -EFAULT;
		mtspr(SPRN_TCR, (mfspr(SPRN_TCR)&~WDTP(0))|WDTP(wdt_drv_period));
		return 0;
	case WDIOC_GETTIMEOUT:
		return put_user(wdt_drv_period, p);
	case WDIOC_SETOPTIONS:
		if (get_user(tmp, p))
			return -EINVAL;
		if (tmp == WDIOS_ENABLECARD) {
			wdt_drv_ping();
			break;
		} else
			return -EINVAL;
		return 0;
	default:
		return -ENOIOCTLCMD;
	}

	return 0;
}
/*
 * wdt_drv_open:
 */
static int wdt_drv_open (struct inode *inode, struct file *file)
{
	if (wdt_drv_enabled == 0) {
		wdt_drv_enabled = 1;
		printk (KERN_INFO "PowerPC Book-E Watchdog Timer Enabled (wdt_period=%d)\n", wdt_drv_period);
		wdt_drv_enable();
	}

	return 0;
}

static struct file_operations wdtDrv_fops = {
  .owner = THIS_MODULE,
  .llseek = no_llseek,
  .write = wdt_drv_write,
  .ioctl = wdt_drv_ioctl,
  .open = wdt_drv_open,
};

static int wdtDrv_init(void)
{
    int         res;

    printk(KERN_INFO "\nWDT Driver v%s (c) 2008 Quanta Computer Inc.\n", WDTDRV_VER);

    
     wdtDrvName = "wdtDrv";

    res = register_chrdev(WDTDRV_MAJOR, wdtDrvName, &wdtDrv_fops);

    if(res < 0)
    {
        printk(KERN_INFO "Cannot register WDT device with kernel\n");
        return  res;
    }
    else
    {
        printk(KERN_INFO "Registered with major %d\n", WDTDRV_MAJOR);
    }

     if (wdt_drv_enabled == 1) {
		printk (KERN_INFO "PowerPC Book-E Watchdog Timer Enabled (wdt_period=%d)\n", wdt_drv_period);
		wdt_drv_enable();
	}

    /* return OK */
    return  0;
}

static void wdtDrv_exit(void)
{
    /* unregister device driver */
    unregister_chrdev(WDTDRV_MAJOR, "wdtDrv");
    return;
}

module_init(wdtDrv_init);
module_exit(wdtDrv_exit);

MODULE_AUTHOR("Vincent Hsu");
MODULE_DESCRIPTION("LB9A WDT Device Drivers");


