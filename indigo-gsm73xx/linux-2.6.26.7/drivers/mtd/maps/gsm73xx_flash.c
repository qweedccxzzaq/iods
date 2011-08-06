/*
 * drivers/mtd/maps/tqm82xx.c
 *
 * MTD mapping driver for TQM82xx boards
 *
 * Copyright 2006 Heiko Schocher, DENX Software Engineering, <hs@denx.de>.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/io.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

/* trivial struct to describe partition information */
struct mtd_part_def
{
	int nums;
	unsigned char *type;
	struct mtd_partition* mtd_part;
};

struct flash_bank {
    unsigned int    start_addr;
    unsigned int    size;
    unsigned int    bankwidth;
#ifdef CONFIG_MTD_PARTITIONS
    struct mtd_partition    *partition_info;
    unsigned int             partition_size;
#endif

    unsigned long   start_scan_addr;
};

static unsigned long num_banks;

#define GSM73XX_FLASH_START_ADDR        0xfe000000
#define GSM73XX_FLASH_SZ                0x02000000


#ifdef CONFIG_MTD_PARTITIONS
/*
 * The following defines the partition layout of TQM82xx boards.
 *
 * See include/linux/mtd/partitions.h for definition of the
 * mtd_partition structure.
 *
 * Assume minimal initial size of 4 MiB per bank, will be updated
 * later in init_tqm82xx_mtd() routine.
 */

#define KB		(1024)
#define MB		(1024 * KB)

#ifdef CONFIG_MTD_GSM73XX_LOADER
/* Partition definition for the first flash bank which is always present. */
static struct mtd_partition gsm73xx_partitions_bank[] = {
	{
		.name = "runtime",
		.offset = 0,
		.size = 28*MB - 256*KB,
	},
	{
		.name = "loader_env",
		.offset = 28*MB - 256*KB,
		.size = 256*KB,
	},
	{
		.name = "loader",
		.offset = 28*MB,
		.size = 4*MB,
	},
	
};
#endif

#endif	/* CONFIG_MTD_PARTITIONS */

static struct flash_bank my_bank[] = {
    {
        .start_addr = GSM73XX_FLASH_START_ADDR,
        .size       = GSM73XX_FLASH_SZ,
        .bankwidth  = 4,
        .partition_info = gsm73xx_partitions_bank,
        .partition_size = ARRAY_SIZE(gsm73xx_partitions_bank),
    },
    
};

#define FLASH_BANK_MAX  ARRAY_SIZE(my_bank)

static struct mtd_info* mtd_banks[FLASH_BANK_MAX];
static struct map_info* map_banks[FLASH_BANK_MAX];
static struct mtd_part_def part_banks[FLASH_BANK_MAX];


static int __init init_gsm73xx_mtd(void)
{
	int idx = 0, ret = 0;
	unsigned long flash_addr, flash_size, mtd_size = 0;
	int n = 0;
	unsigned long start_scan_addr;



	for(idx = 0 ; idx < FLASH_BANK_MAX ; idx++) {

		flash_addr = my_bank[idx].start_addr;
		flash_size = my_bank[idx].size;

		/* request maximum flash size address space */
		start_scan_addr = (unsigned long)ioremap(flash_addr, flash_size);
		if (!start_scan_addr) {
			printk(KERN_NOTICE "%s: Failed to ioremap address: 0x%lx\n",
				__FUNCTION__, flash_addr);
			return -EIO;
		}
        
		if (mtd_size >= flash_size)
			break;

		pr_debug("%s: chip probing count %d\n", __FUNCTION__, idx);

		map_banks[idx] = (struct map_info *)kmalloc(sizeof(struct map_info),
							    GFP_KERNEL);
		if (map_banks[idx] == NULL) {
			ret = -ENOMEM;
			goto error_mem;
		}
		memset((void *)map_banks[idx], 0, sizeof(struct map_info));
		map_banks[idx]->name = (char *)kmalloc(16, GFP_KERNEL);
		if (map_banks[idx]->name == NULL) {
			ret = -ENOMEM;
			goto error_mem;
		}
		memset((void *)map_banks[idx]->name, 0, 16);

		sprintf(map_banks[idx]->name, "Gsm73xx-%d", idx);
		map_banks[idx]->size = flash_size;
		map_banks[idx]->bankwidth = my_bank[idx].bankwidth;

		simple_map_init(map_banks[idx]);

		map_banks[idx]->virt = (void __iomem *)start_scan_addr;
		map_banks[idx]->phys = flash_addr;

		/* start to probe flash chips */
		mtd_banks[idx] = do_map_probe("cfi_probe", map_banks[idx]);
		if (mtd_banks[idx]) {
			mtd_banks[idx]->owner = THIS_MODULE;
			mtd_size += mtd_banks[idx]->size;
			num_banks++;
			pr_debug("%s: bank %ld, name: %s, size: %d bytes \n",
				 __FUNCTION__, num_banks,
				 mtd_banks[idx]->name, mtd_banks[idx]->size);
		}

		my_bank[idx].start_scan_addr = start_scan_addr;
	}

	/* no supported flash chips found */
	if (!num_banks) {
		printk(KERN_NOTICE "Gsm73xx: No supported flash chips found!\n");
		ret = -ENXIO;
		goto error_mem;
	}

#ifdef CONFIG_MTD_PARTITIONS
	/*
	 * Select static partition definitions
	 */
	for(idx = 0 ; idx < FLASH_BANK_MAX ; idx++) {
		n = my_bank[idx].partition_size;
		part_banks[idx].mtd_part= my_bank[idx].partition_info;
		part_banks[idx].type	= "static image bank";
		part_banks[idx].nums	= n;
	}

	for(idx = 0; idx < num_banks ; idx++) {

		if (part_banks[idx].nums == 0) {
			printk(KERN_NOTICE
			       "Gsm73xx flash bank %d: no partition info "
			       "available, registering whole device\n", idx);
			add_mtd_device(mtd_banks[idx]);
		} else {
			printk(KERN_NOTICE
			       "Gsm73xx flash bank %d: Using %s partition "
			       "definition\n", idx, part_banks[idx].type);
			add_mtd_partitions(mtd_banks[idx],
					   part_banks[idx].mtd_part,
					   part_banks[idx].nums);
		}
	}
#else	/* ! CONFIG_MTD_PARTITIONS */
	printk(KERN_NOTICE "Gsm73xx flash: registering %d flash banks "
	       "at once\n", num_banks);

	for(idx = 0 ; idx < num_banks ; idx++)
		add_mtd_device(mtd_banks[idx]);

#endif	/* CONFIG_MTD_PARTITIONS */

	return 0;
error_mem:
	for (idx = 0 ; idx < FLASH_BANK_MAX ; idx++) {
		if (map_banks[idx] != NULL) {
			if (map_banks[idx]->name != NULL) {
				kfree(map_banks[idx]->name);
				map_banks[idx]->name = NULL;
			}
			kfree(map_banks[idx]);
			map_banks[idx] = NULL;

			if (my_bank[idx].start_scan_addr) {
				iounmap((void *)my_bank[idx].start_scan_addr);
				my_bank[idx].start_scan_addr = 0;
			}
		}
	}

	return ret;
}

static void __exit cleanup_gsm73xx_mtd(void)
{
	unsigned int idx = 0;
	for(idx = 0 ; idx < num_banks ; idx++) {
		/* destroy mtd_info previously allocated */
		if (mtd_banks[idx]) {
			del_mtd_partitions(mtd_banks[idx]);
			map_destroy(mtd_banks[idx]);
		}

		/* release map_info not used anymore */
		kfree(map_banks[idx]->name);
		kfree(map_banks[idx]);

		if (my_bank[idx].start_scan_addr) {
			iounmap((void *)my_bank[idx].start_scan_addr);
			my_bank[idx].start_scan_addr = 0;
		}
	}
}

module_init(init_gsm73xx_mtd);
module_exit(cleanup_gsm73xx_mtd);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dave Hu <dave.hu@deltaww.com.cn>");
MODULE_DESCRIPTION("MTD map driver for Gsm73xx boards");
