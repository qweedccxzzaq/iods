/*
 * Generic platform device IDE driver
 *
 * Copyright (C) Quanta Computer Inc.
 *
 *
 *   Copyright 2005-2006 Red Hat Inc <alan@redhat.com>, all rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <scsi/scsi_host.h>
#include <linux/ata.h>
#include <linux/libata.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>

#define DRV_NAME "qci_ide_platform"
#define DRV_VERSION "1.0"


//#include <linux/config.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/user.h>
#include <linux/a.out.h>
#include <linux/tty.h>
#include <linux/major.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/ide.h>
#include <linux/ioport.h>
#include <linux/ide.h>
#include <linux/bootmem.h>

#include <asm/mmu.h>
#include <asm/processor.h>
//#include <asm/residual.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/ide.h>
#include <asm/machdep.h>
#include <asm/irq.h>


#define IDE0_BASE_OFFSET                0xf0000000
#define IDE0_DATA_REG_OFFSET            0x00000002            /* 16 bits */ 
#define IDE0_ERROR_REG_OFFSET           0x00000007            /* 8 bits */
#define IDE0_NSECTOR_REG_OFFSET         0x0000000B            /* 8 bits */
#define IDE0_SECTOR_REG_OFFSET          0x0000000F            /* 8 bits */
#define IDE0_LCYL_REG_OFFSET            0x00000013            /* 8 bits */
#define IDE0_HCYL_REG_OFFSET            0x00000017            /* 8 bits */    
#define IDE0_SELECT_REG_OFFSET          0x0000001B            /* 8 bits */
#define IDE0_STATUS_REG_OFFSET          0x0000001F            /* 8 bits */
#define IDE0_CONTROL_REG_OFFSET         0x0001001B
#define IDE0_IRQ_REG_OFFSET             0
#define IDE0_SIZE                       0x0001FFFF
#define IDE0_INTERRUPT                  2


static void qci_lb6b_ata_input_data(ide_drive_t * drive,  struct request *rq, void *buffer, unsigned int count);
static void qci_lb6b_ata_output_data(ide_drive_t * drive,  struct request *rq, void *buffer, unsigned int count);
static void qci_lb6b_atapi_input_bytes(ide_drive_t * drive,  struct request *rq, void *buffer, unsigned int count);
static void qci_lb6b_atapi_output_bytes(ide_drive_t * drive,  struct request *rq, void *buffer, unsigned int count);

static int ioport_dsc[IDE_NR_PORTS] = {
    IDE0_DATA_REG_OFFSET,
    IDE0_ERROR_REG_OFFSET,
    IDE0_NSECTOR_REG_OFFSET,
    IDE0_SECTOR_REG_OFFSET,
    IDE0_LCYL_REG_OFFSET,
    IDE0_HCYL_REG_OFFSET,
    IDE0_SELECT_REG_OFFSET,
    IDE0_STATUS_REG_OFFSET,
    IDE0_CONTROL_REG_OFFSET,
    IDE0_IRQ_REG_OFFSET
};

#define IDE_NR_PORTS		(10)

#define IDE_DATA_OFFSET		(0)
#define IDE_ERROR_OFFSET	(1)
#define IDE_NSECTOR_OFFSET	(2)
#define IDE_SECTOR_OFFSET	(3)
#define IDE_LCYL_OFFSET		(4)
#define IDE_HCYL_OFFSET		(5)
#define IDE_SELECT_OFFSET	(6)
#define IDE_STATUS_OFFSET	(7)
#define IDE_CONTROL_OFFSET	(8)
#define IDE_IRQ_OFFSET		(9)

/* Currently only m68k, apus and m8xx need it */
#ifndef IDE_ARCH_ACK_INTR
# define ide_ack_intr(hwif) (1)
#endif




static const struct ide_tp_ops QCIide_tp_ops = {
	.exec_command		= ide_exec_command,
	.read_status		= ide_read_status,
	.read_altstatus		= ide_read_altstatus,
	.read_sff_dma_status	= ide_read_sff_dma_status,

	.set_irq		= ide_set_irq,

	.tf_load		= ide_tf_load,
	.tf_read		= ide_tf_read,

	.input_data		= qci_lb6b_ata_input_data,
	.output_data		= qci_lb6b_ata_output_data,
};

static const struct ide_port_info QCIide_port_info = {
	.tp_ops			= &QCIide_tp_ops,
	.host_flags		= IDE_HFLAG_MMIO|IDE_HFLAG_NO_DMA|IDE_HFLAG_NO_IO_32BIT,
};



static void qci_lb6b_ata_input_data(ide_drive_t * drive,  struct request *rq, void *buffer,
    unsigned int count)
{

	volatile u16 *port;
   	 u16 *buf;
   	 u16 w;
	int wcount;

	ide_hwif_t *hwif = drive->hwif;
	struct ide_io_ports *io_ports = &hwif->io_ports;
	unsigned long data_addr = io_ports->data_addr;
	

        port = (void __iomem *) data_addr;
	 buf = buffer;
            
	 // count is in 32-bit words
	 wcount = count<<1;

        while (wcount--) {
	            w = *port;
        	    *buf++ = (((w & 0x00FF) << 8) | ((w & 0xFF00) >> 8));
	  }
        //printk ("**AG** read\n");
	//ide_dump_data (buffer,32);
}


static void qci_lb6b_ata_output_data(ide_drive_t * drive,  struct request *rq, void *buffer,
    unsigned int count)
{

	volatile u16 *port;
   	 u16 *buf;
	 u16 w;
	int wcount;
	ide_hwif_t *hwif = drive->hwif;
	struct ide_io_ports *io_ports = &hwif->io_ports;
	unsigned long data_addr = io_ports->data_addr;
		
        port = (unsigned short *) data_addr;
        buf = buffer;
            
        // count is in 32-bit words
        wcount = count<<1;

        while (wcount--) {
            w = *buf++;
            *port = ((w & 0x00FF) << 8) | ((w & 0xFF00) >> 8);
        }
}



static void __init qci_ide_setup_ports (	hw_regs_t *hw,
			unsigned long base, int *offsets,
			unsigned long ctrl, unsigned long intr,
			ide_ack_intr_t *ack_intr,
/*
 *			ide_io_ops_t *iops,
 */
			int irq)
{
	int i;

	for (i = 0; i <IDE_NR_PORTS; i++)
		if (offsets[i] == -1) {
			switch(i) {
				case IDE_CONTROL_OFFSET:
					hw->io_ports_array[i] = ctrl;
					break;
				default:
					hw->io_ports_array[i] = 0;
					break;
			}
		} else {
			hw->io_ports_array[i] = base + offsets[i];
		}


	hw->io_ports.data_addr = base+offsets[IDE_DATA_OFFSET];
	hw->io_ports.ctl_addr = base+offsets[IDE_CONTROL_OFFSET];
	hw->io_ports.irq_addr = base+offsets[IDE_IRQ_OFFSET];
	hw->io_ports.device_addr = base+offsets[IDE_SELECT_OFFSET];
	hw->io_ports.status_addr =  base+offsets[IDE_STATUS_OFFSET];
	hw->io_ports.command_addr = base+offsets[IDE_STATUS_OFFSET];
	hw->irq = irq;
	//hw->dma = NO_DMA;
	hw->ack_intr = ack_intr;
	hw->chipset = ide_generic;	

/*
 *	hw->iops = iops;
 */
}

static int __init qci_lb6b_ide_dma_check(ide_drive_t *drive)
{
    /* reject dma requests */
    return 1;
}




int __devinit __qci_ide_platform_probe(struct device *dev,
				    struct resource *io_res,
				    struct resource *ctl_res,
				    struct resource *irq_res,
				    unsigned int ioport_shift)
{
	struct ide_host *host;
	hw_regs_t hw, *hws[] = { &hw, NULL, NULL, NULL };
	int rc;
	
	int index;
	int offsets[IDE_NR_PORTS];
	int i;
	unsigned long port;
	u32 ide_phy_base;
	u32 ide_phy_end;
   	unsigned int vector;
	unsigned long qci_ide_base = 0;
	unsigned long qci_csr_base = 0;
	int qci_ide_irq = 0;

      
	if (!qci_ide_base) {

		ide_phy_base =  io_res->start;
		ide_phy_end  = io_res->end;

		printk ("IDE phys mem : %08x...%08x (size %08x)\n",
			ide_phy_base, ide_phy_end,
			ide_phy_end - ide_phy_base);
		
		qci_ide_base=(unsigned long)ioremap(ide_phy_base,
			ide_phy_end-ide_phy_base);

		printk ("IDE virt base: 0x%08lx\n", qci_ide_base);
                
		qci_csr_base=(unsigned long)ioremap(ctl_res->start, ctl_res->end - ctl_res->start + 1);

		printk ("CSR virt base: 0x%08lx\n", qci_csr_base);
                
                qci_ide_irq=irq_res->start;  
	}

	for (i = 0; i < IDE_NR_PORTS; ++i) {
	 	offsets[i] = qci_ide_base + ioport_dsc[i];
		printk ("port: 0x%08x => 0x%02X\n",offsets[i],*((unsigned char *)offsets[i]));
	}

	port = 0;

	memset(&hw, 0, sizeof(hw));
	qci_ide_setup_ports(&hw, port, offsets, 0, 0, 0, qci_ide_irq);


	host = ide_host_alloc(&QCIide_port_info, hws);
	if (host == NULL) {
		rc = -ENOMEM;
		goto err;
	}

	ide_get_lock(NULL, NULL);
	rc = ide_host_register(host, &QCIide_port_info, hws);
	ide_release_lock();


	if (rc)
		goto err_free;


	return 0;

err_free:
	ide_host_free(host);
err:
	release_mem_region(0, 0x40);
	return rc;
}
EXPORT_SYMBOL_GPL(__qci_ide_platform_probe);



/**
 *	__pata_platform_remove		-	unplug a platform interface
 *	@dev: device
 *
 *	A platform bus ATA device has been unplugged. Perform the needed
 *	cleanup. Also called on module unload for any active devices.
 */
int __devexit __qci_ide_platform_remove(struct device *dev)
{
	struct ata_host *host = dev_get_drvdata(dev);

	ata_host_detach(host);

	return 0;
}
EXPORT_SYMBOL_GPL(__qci_ide_platform_remove);

static int __devinit qci_ide_platform_probe(struct platform_device *pdev)
{
	struct resource *io_res;
	struct resource *ctl_res;
	struct resource *irq_res;
	struct pata_platform_info *pp_info = pdev->dev.platform_data;

	/*
	 * Simple resource validation ..
	 */
	if ((pdev->num_resources != 3) && (pdev->num_resources != 2)) {
		dev_err(&pdev->dev, "invalid number of resources\n");
		return -EINVAL;
	}

	/*
	 * Get the I/O base first
	 */
	io_res = platform_get_resource(pdev, IORESOURCE_IO, 0);
	if (io_res == NULL) {
		io_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (unlikely(io_res == NULL))
			return -EINVAL;
	}

	/*
	 * Then the CTL base
	 */
	ctl_res = platform_get_resource(pdev, IORESOURCE_IO, 1);
	if (ctl_res == NULL) {
		ctl_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (unlikely(ctl_res == NULL))
			return -EINVAL;
	}

	/*
	 * And the IRQ
	 */
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (irq_res)
		irq_res->flags = pp_info ? pp_info->irq_flags : 0;

	return __qci_ide_platform_probe(&pdev->dev, io_res, ctl_res, irq_res,
				     pp_info ? pp_info->ioport_shift : 0);
}

static int __devexit qci_ide_platform_remove(struct platform_device *pdev)
{
	return __qci_ide_platform_remove(&pdev->dev);
}

static struct platform_driver qci_ide_platform_driver = {
	.probe		= qci_ide_platform_probe,
	.remove		= __devexit_p(qci_ide_platform_remove),
	.driver = {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
	},
};

static int __init qci_ide_platform_init(void)
{
	return platform_driver_register(&qci_ide_platform_driver);
}

static void __exit qci_ide_platform_exit(void)
{
	platform_driver_unregister(&qci_ide_platform_driver);
}
module_init(qci_ide_platform_init);
module_exit(qci_ide_platform_exit);


MODULE_AUTHOR("Aries Fang");
MODULE_DESCRIPTION("low-level driver for platform device IDE");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS("platform:" DRV_NAME);


