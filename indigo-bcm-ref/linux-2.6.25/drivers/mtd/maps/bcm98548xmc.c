/*
 * $Id: wr_sbc82xx_flash.c,v 1.8 2005/11/07 11:14:29 gleixner Exp $
 *
 * Map for flash chips on Wind River PowerQUICC II SBC82xx board.
 *
 * Copyright (C) 2004 Red Hat, Inc.
 *
 * Author: David Woodhouse <dwmw2@infradead.org>
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#define BCM98548_NUM_FLASH_DEVS     2

static struct mtd_info *p_mtdinfo[BCM98548_NUM_FLASH_DEVS];

struct map_info bcm98548_flash_maps[BCM98548_NUM_FLASH_DEVS] = {
	{
            .name           = "Boot flash",
            .size           = 0x8000000,
            .phys           = 0xf8000000,
            .bankwidth      = 2,
        },
	{
            .name           = "PLCC boot flash",
            .size           = 0x8000000,
            .phys           = 0xf0000000,
            .bankwidth      = 2,
        }
};

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition default_partitions_flash0[] __initdata = {
        {
		.name = "Boot flash cfe",
		.size = 0x100000,
		.offset = 0x7f00000,
		.mask_flags = MTD_WRITEABLE
	}, {
		.name = "Boot flash os",
		.size = 0x7f00000,
		.offset = 0x00000
	}
};

static struct mtd_partition default_partitions_flash1[] __initdata = {
        {
		.name = "PLCC flash cfe",
		.size = 0x100000,
		.offset = 0x7f00000,
		.mask_flags = MTD_WRITEABLE
	}, {
		.name = "PLCC flash os",
		.size = 0x7f00000,
		.offset = 0x00000
	}
};

#define BCM98548_PARTS(i)                                               \
   (((i) == 0) ?                                                        \
    (sizeof(default_partitions_flash0)/sizeof(struct mtd_partition)) :  \
    (sizeof(default_partitions_flash1)/sizeof(struct mtd_partition)))

#define PTABLE_MAGIC            0x5054424C /* 'PTBL' (big-endian) */
#define PTABLE_MAGIC_LE         0x4C425450 /* 'PTBL' (little-endian) */
#define PTABLE_PNAME_MAX        16
#define PTABLE_MAX_PARTITIONS   8

typedef struct partition_s {
    char        name[PTABLE_PNAME_MAX];
    uint32_t    offset;
    uint32_t    size;
    uint32_t    type;
    uint32_t    flags;
} partition_t;

typedef struct ptable_s {
    uint32_t    magic;
    uint32_t    version;
    uint32_t    chksum;
    uint32_t    reserved;
    partition_t part[PTABLE_MAX_PARTITIONS];
} ptable_t;

static struct mtd_partition bcm98548_user_parts[BCM98548_NUM_FLASH_DEVS][PTABLE_MAX_PARTITIONS];
static ptable_t ptable;
static int num_parts;

static int bcm98548xmc_check_ptable(int fdev, ptable_t *ptbl,
                                 struct mtd_partition *part, 
                                 ssize_t max_parts, ssize_t *num_parts)
{
        uint32_t chksum, *p32;
        int i, swapped = 0;
        uint32_t    fbase, fsize, uparts = 0, top = 0;
        partition_t *up = ptbl->part;

        if (ptbl->magic == PTABLE_MAGIC_LE) {
                swapped = 1;
        }
        else if (ptbl->magic != PTABLE_MAGIC) {
                return -1;
        }
        chksum = 0;
        p32 = (uint32_t*)ptbl;
        for (i = 0; i < sizeof(ptable_t)/4; i++) {
                chksum ^= p32[i];
                if (swapped) {
                        p32[i] = swab32(p32[i]);
                }
        }
        if (chksum != 0) {
                return -1;
        }

        fbase = (fdev == 0) ? 0 : 0x8000000;
        fsize = 0x8000000;
        for (i = 0; i < PTABLE_MAX_PARTITIONS && up[i].size; i++) {
            if ((up[i].offset < fbase) || (up[i].offset >= fbase + fsize)) {
                continue;
            }

            if (top + up[i].size > fsize) break;
            /* Check for holes in partition map */
            if ((up[i].offset - fbase) > top) {
                part[uparts].size = top ? 0 : up[i].offset - top;
                part[uparts].name = "unused";
                if (++uparts >= PTABLE_MAX_PARTITIONS) break;
            }
            part[uparts].size = up[i].size;
            part[uparts].name = up[i].name;
            part[uparts].offset = up[i].offset - fbase;
            printk("Flash (%s:%d) found user partition %d (%s from 0x%08x size 0x%08x\n",
                    bcm98548_flash_maps[fdev].name,
                    fdev, uparts, part[uparts].name,
                    part[uparts].offset, part[uparts].size);

            if (++uparts >= PTABLE_MAX_PARTITIONS) break;
            top = up[i].offset - fbase + up[i].size;
        }
        *num_parts = uparts;
        return 0;
}

static int bcm98548_find_partitions(int fdev, struct mtd_info *mi, 
                                     struct mtd_partition *part, 
                                     ssize_t max_parts, ssize_t *num_parts)
{
        int retlen, offset;

        if (mi == NULL || mi->read == NULL) {
                return -1;
        }
        for (offset = 0; offset < mi->size; offset += 0x20000) {
                if (mi->read(mi, offset, sizeof(ptable), &retlen, 
                             (u_char *)&ptable) < 0) {
                        return -1;
                }
                if (retlen != sizeof(ptable)) {
                        return -1;
                }
                if (bcm98548xmc_check_ptable(fdev, &ptable, part, max_parts, 
                                          num_parts) == 0) {
                        if (*num_parts > 0) {
                                printk("Found flash partition table "
                                       "at offset 0x%08x\n", offset);
                                return 0;
                        }
                }
        }
        printk("No valid flash partition table found - using default mapping\n");
        return -1;
}

#endif
int __init init_bcm98548_flash(void)
{
	int i, ret;

	for (i=0; i<BCM98548_NUM_FLASH_DEVS; i++) {
		printk(KERN_NOTICE "MPC98548XMC %s (%ld MiB",
		       bcm98548_flash_maps[i].name,
		       (bcm98548_flash_maps[i].size >> 20));
		if (!bcm98548_flash_maps[i].phys) {
			printk("): disabled by bootloader.\n");
			continue;
		}
		printk(" at %08x)\n",  bcm98548_flash_maps[i].phys);

		bcm98548_flash_maps[i].virt = 
                            ioremap(bcm98548_flash_maps[i].phys, 
                                    bcm98548_flash_maps[i].size);

		if (!bcm98548_flash_maps[i].virt) {
			printk("Failed to ioremap\n");
			continue;
		}

		simple_map_init(&bcm98548_flash_maps[i]);

		p_mtdinfo[i] = do_map_probe("cfi_probe", &bcm98548_flash_maps[i]);

		if (!p_mtdinfo[i])
			continue;

		p_mtdinfo[i]->owner = THIS_MODULE;

#ifdef CONFIG_MTD_PARTITIONS
                if (bcm98548_find_partitions(i, 
                         p_mtdinfo[0] /* Only flash device 0 has ptable */, 
                         &bcm98548_user_parts[i][0], PTABLE_MAX_PARTITIONS, 
                                                            &num_parts) == 0) {
                    ret = add_mtd_partitions(p_mtdinfo[i], 
                                    &bcm98548_user_parts[i][0], num_parts);
                } else {
                    if (i == 0) {
                        ret = add_mtd_partitions(p_mtdinfo[i], 
                                    default_partitions_flash0, BCM98548_PARTS(i));
                    } else {
                        ret = add_mtd_partitions(p_mtdinfo[i], 
                                    default_partitions_flash1, BCM98548_PARTS(i));
                    }
                }
                if (ret) {
                    printk(KERN_ERR "pflash: add_mtd_partitions failed\n");
                    goto fail;
                }
#endif
	}
	return 0;
fail:
	for (i=0; i<BCM98548_NUM_FLASH_DEVS; i++) {
		if (!p_mtdinfo[i])
			continue;

#ifdef CONFIG_MTD_PARTITIONS
                del_mtd_partitions(p_mtdinfo[i]);
#endif
		map_destroy(p_mtdinfo[i]);
                if (bcm98548_flash_maps[i].virt) {
		    iounmap((void *)bcm98548_flash_maps[i].virt);
		    bcm98548_flash_maps[i].virt = 0;
                }
	}

}

static void __exit clean_bcm98548_flash(void)
{
	int i;

	for (i=0; i<BCM98548_NUM_FLASH_DEVS; i++) {
		if (!p_mtdinfo[i])
			continue;

#ifdef CONFIG_MTD_PARTITIONS
                del_mtd_partitions(p_mtdinfo[i]);
#endif

		map_destroy(p_mtdinfo[i]);

		iounmap((void *)bcm98548_flash_maps[i].virt);
		bcm98548_flash_maps[i].virt = 0;
	}
}

module_init(init_bcm98548_flash);
module_exit(clean_bcm98548_flash);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org>");
MODULE_DESCRIPTION("Flash map driver for BCM98548XMC");
