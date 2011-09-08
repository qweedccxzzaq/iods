/*
 * MPC85xx setup and early boot code plus other random bits.
 *
 * Maintained by Kumar Gala (see MAINTAINERS for contact information)
 *
 * Copyright 2005 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/reboot.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/initrd.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fsl_devices.h>

#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/atomic.h>
#include <asm/time.h>
#include <asm/io.h>
#include <asm/machdep.h>
#include <asm/ipic.h>
#include <asm/pci-bridge.h>
#include <asm/irq.h>
#include <mm/mmu_decl.h>
#include <asm/prom.h>
#include <asm/udbg.h>
#include <asm/mpic.h>
#include <asm/i8259.h>

#include <sysdev/fsl_soc.h>
#include <sysdev/fsl_pci.h>

/* CADMUS info */
/* xxx - galak, move into device tree */
#define CADMUS_BASE (0xf8004000)
#define CADMUS_SIZE (256)
#define CM_VER	(0)
#define CM_CSR	(1)
#define CM_RST	(2)


#ifdef CONFIG_PCI

static int bcm98548xmc_exclude_device(struct pci_controller *hose,
				  u_char bus, u_char devfn)
{
    return PCIBIOS_SUCCESSFUL;
}

static __be32 __iomem *rstcr;
static __be32 __iomem *rstdr;
 
extern void abort(void);

static int __init bcm98548xmc_rstcr(void)
{
       /* map reset control register */
       rstdr = ioremap(get_immrbase() + 0xE0040, 0xff);
       rstcr = ioremap(get_immrbase() + 0xE0030, 0xff);
       return 0;
} 

arch_initcall(bcm98548xmc_rstcr);

static void bcm98548xmc_restart(char *cmd)
{
       local_irq_disable();
       if (rstcr) {
               /* set reset control register */
               out_be32(rstdr, ~0x0);  /* HRESET_REQ */
               out_be32(rstcr, 0x200); /* HRESET_REQ */
               out_be32(rstdr, ~0x1);  /* HRESET_REQ */
               out_be32(rstdr, ~0x3);  /* HRESET_REQ */
       } else
               printk (KERN_EMERG "Error: reset control register not mapped, spinning!\n");
       abort();
}

static void __init bcm98548xmc_pci_irq_fixup(struct pci_dev *dev)
{
}

static void __devinit skip_fake_bridge(struct pci_dev *dev)
{
	/* Make it an error to skip the fake bridge
	 * in pci_setup_device() in probe.c */
	dev->hdr_type = 0x7f;
}
DECLARE_PCI_FIXUP_EARLY(0x1957, 0x3fff, skip_fake_bridge);
DECLARE_PCI_FIXUP_EARLY(0x3fff, 0x1957, skip_fake_bridge);
DECLARE_PCI_FIXUP_EARLY(0xff3f, 0x5719, skip_fake_bridge);

#endif /* CONFIG_PCI */

static void __init bcm98548xmc_pic_init(void)
{
	struct mpic *mpic;
	struct resource r;
	struct device_node *np = NULL;

	np = of_find_node_by_type(np, "open-pic");

	if (np == NULL) {
		printk(KERN_ERR "Could not find open-pic node\n");
		return;
	}

	if (of_address_to_resource(np, 0, &r)) {
		printk(KERN_ERR "Failed to map mpic register space\n");
		of_node_put(np);
		return;
	}

	mpic = mpic_alloc(np, r.start,
			MPIC_PRIMARY | MPIC_WANTS_RESET | MPIC_BIG_ENDIAN,
			4, 0, " OpenPIC  ");
	BUG_ON(mpic == NULL);

	/* Return the mpic node */
	of_node_put(np);

        mpic_assign_isu(mpic, 0, r.start + 0x10200);
        mpic_assign_isu(mpic, 1, r.start + 0x10280);
        mpic_assign_isu(mpic, 2, r.start + 0x10300);
        mpic_assign_isu(mpic, 3, r.start + 0x10380);
        mpic_assign_isu(mpic, 4, r.start + 0x10400);
        mpic_assign_isu(mpic, 5, r.start + 0x10480);
        mpic_assign_isu(mpic, 6, r.start + 0x10500);
        mpic_assign_isu(mpic, 7, r.start + 0x10580);

        /* Used only for 8548 so far, but no harm in
         * allocating them for everyone */
        mpic_assign_isu(mpic, 8, r.start + 0x10600);
        mpic_assign_isu(mpic, 9, r.start + 0x10680);
        mpic_assign_isu(mpic, 10, r.start + 0x10700);
        mpic_assign_isu(mpic, 11, r.start + 0x10780);

        /* External Interrupts */
        mpic_assign_isu(mpic, 12, r.start + 0x10000);
        mpic_assign_isu(mpic, 13, r.start + 0x10080);
        mpic_assign_isu(mpic, 14, r.start + 0x10100);

	mpic_init(mpic);
}

static void __init
bcm98548xmc_map_io(void)
{
        if (ppc_md.progress) ppc_md.progress("bcm98245_map_io", 0);

        /* __ioremap_at(0xf0000000, 0xf0000000, 0x10000000, _PAGE_IO);*/
}

/*
 * Hook up RTC
 */
#if defined(CONFIG_SENSORS_M41T00)
#include <linux/m41t00.h>
#include <linux/rtc.h>
extern ulong	m41t00_get_rtc_time(void);
extern int	m41t00_set_rtc_time(ulong);

static struct m41t00_platform_data m41t00_pdata = {
	.type = M41T00_TYPE_M41T81,
	.i2c_addr = 0x68,
	.sqw_freq = 0x0,
};

static struct platform_device *m41t00_dev = NULL;

static int __init bcm98548_device_init(void)
{
	int ret;

	m41t00_dev = platform_device_alloc(M41T00_DRV_NAME, -1);
	if (!m41t00_dev) {
		printk(KERN_WARNING "Kernel can't allocate memory for M41T00\n");
		return -1;
	}

	m41t00_dev->dev.platform_data = &m41t00_pdata;	

	ret = platform_device_add(m41t00_dev);
	if(ret) {
		printk(KERN_WARNING "Kernel add device into platform failure!\n");
		platform_device_put(m41t00_dev);
		return -1;
	}
	return 0;
}
 
device_initcall(bcm98548_device_init);

extern ulong	m41t00_get_rtc_time(void);
extern int	m41t00_set_rtc_time(ulong);

static void _rtc_drv_wrapper_get_time(struct rtc_time * rtime)
{
    ulong sec;
    /* kernel.h system_state */

    if (unlikely(system_state == SYSTEM_RUNNING)) {
        return;
    }

    sec = m41t00_get_rtc_time();
    to_tm(sec, rtime);
    return;
}

static int _rtc_drv_wrapper_set_time(struct rtc_time * rtime)
{
    ulong sec;

    if (unlikely(system_state == SYSTEM_RUNNING)) {
        return -1;
    }

    sec = mktime(rtime->tm_year, rtime->tm_mon, rtime->tm_mday,
                 rtime->tm_hour, rtime->tm_min, rtime->tm_sec);
    m41t00_set_rtc_time(sec);
    return 0;
}

static int __init bcm98548_rtc_hookup(void)
{
	struct timespec	tv;

	ppc_md.get_rtc_time = _rtc_drv_wrapper_get_time;
	ppc_md.set_rtc_time = _rtc_drv_wrapper_set_time;

	tv.tv_nsec = 0;
	tv.tv_sec = m41t00_get_rtc_time();
	do_settimeofday(&tv);

	return 0;
}
late_initcall(bcm98548_rtc_hookup);
#endif /* CONFIG_SENSORS_M41T00 */

/*
 * Setup the architecture
 */
static void __init bcm98548xmc_setup_arch(void)
{
#ifdef CONFIG_PCI
	struct device_node *np;
#endif

	if (ppc_md.progress)
		ppc_md.progress("bcm98548xmc_setup_arch()", 0);

        ppc_md.setup_io_mappings = bcm98548xmc_map_io;

#ifdef CONFIG_PCI
	for_each_node_by_type(np, "pci") {
		if (of_device_is_compatible(np, "85xx") ||
                    of_device_is_compatible(np, "fsl,mpc8540-pci") ||
		    of_device_is_compatible(np, "fsl,mpc8548-pcie")) {
			struct resource rsrc;
			of_address_to_resource(np, 0, &rsrc);
			if ((rsrc.start & 0xfffff) == 0x8000)
				fsl_add_bridge(np, 1);
			else
				fsl_add_bridge(np, 0);
		}
	}

	ppc_md.pci_irq_fixup = bcm98548xmc_pci_irq_fixup;
	ppc_md.pci_exclude_device = bcm98548xmc_exclude_device;
#endif
}

static void bcm98548xmc_show_cpuinfo(struct seq_file *m)
{
	uint pvid, svid, phid1;
	uint memsize = total_memory;

	pvid = mfspr(SPRN_PVR);
	svid = mfspr(SPRN_SVR);

	seq_printf(m, "Vendor\t\t: Freescale Semiconductor\n");
	seq_printf(m, "PVR\t\t: 0x%x\n", pvid);
	seq_printf(m, "SVR\t\t: 0x%x\n", svid);

	/* Display cpu Pll setting */
	phid1 = mfspr(SPRN_HID1);
	seq_printf(m, "PLL setting\t: 0x%x\n", ((phid1 >> 24) & 0x3f));

	/* Display the amount of memory */
	seq_printf(m, "Memory\t\t: %d MB\n", memsize / (1024 * 1024));
}


/*
 * Called very early, device-tree isn't unflattened
 */
static int __init bcm98548xmc_probe(void)
{
        return 1;
}

define_machine(bcm98548xmc) {
	.name		= "BCM98548XMC",
	.probe		= bcm98548xmc_probe,
	.setup_arch	= bcm98548xmc_setup_arch,
	.init_IRQ	= bcm98548xmc_pic_init,
	.show_cpuinfo	= bcm98548xmc_show_cpuinfo,
	.get_irq	= mpic_get_irq,
#ifdef CONFIG_PCI
	.restart	= bcm98548xmc_restart,
#else
	.restart	= fsl_rstcr_restart,
#endif
	.calibrate_decr = generic_calibrate_decr,
	.progress	= udbg_progress,
};
