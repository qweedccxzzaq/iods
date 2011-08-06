/*
 * OF-platform IDE  driver
 *
 * Copyright (c) 2009  Quanta Computer, Inc.
 *                     Aries Fang <Aries.Fang@quantatw.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/ata_platform.h>



extern int __devinit __qci_ide_platform_probe(struct device *dev,
					   struct resource *io_res,
					   struct resource *ctl_res,
					   struct resource *irq_res,
					   unsigned int ioport_shift  );

extern int __devexit __qci_ide_platform_remove(struct device *dev);




static int __devinit ide_of_platform_probe(struct of_device *ofdev,
					    const struct of_device_id *match)
{
	int ret;
	struct device_node *dn = ofdev->node;
	struct resource io_res;
	struct resource ctl_res;
	struct resource irq_res;
	unsigned int reg_shift = 0;
	const u32 *prop;

	ret = of_address_to_resource(dn, 0, &io_res);

	if (ret) {
		dev_err(&ofdev->dev, "can't get IO address from "
			"device tree\n");
		return -EINVAL;
	}

	if (of_device_is_compatible(dn, "electra-ide")) {
		/* Altstatus is really at offset 0x3f6 from the primary window
		 * on electra-ide. Adjust ctl_res and io_res accordingly.
		 */
		ctl_res = io_res;
		ctl_res.start = ctl_res.start+0x3f6;
		io_res.end = ctl_res.start-1;
	} else {
		ret = of_address_to_resource(dn, 1, &ctl_res);
		if (ret) {
			dev_err(&ofdev->dev, "can't get CTL address from "
				"device tree\n");
			return -EINVAL;
		}
	}

	ret = of_irq_to_resource(dn, 0, &irq_res);
	if (ret == NO_IRQ)
		irq_res.start = irq_res.end = -1;
	else
		irq_res.flags = 0;

	prop = of_get_property(dn, "reg-shift", NULL);
	if (prop)
		reg_shift = *prop;


	return __qci_ide_platform_probe(&ofdev->dev, &io_res, &ctl_res, &irq_res,
				     reg_shift);
}

static int __devexit ide_of_platform_remove(struct of_device *ofdev)
{
	return __qci_ide_platform_remove(&ofdev->dev);
}

static struct of_device_id ide_of_platform_match[] = {
	{ .compatible = "qci-ide", },
	{ .compatible = "electra-ide", },
	{},
};
MODULE_DEVICE_TABLE(of, ide_of_platform_match);

static struct of_platform_driver ide_of_platform_driver = {
	.name		= "ide_of_platform",
	.match_table	= ide_of_platform_match,
	.probe		= ide_of_platform_probe,
	.remove		= __devexit_p(ide_of_platform_remove),
};

static int __init ide_of_platform_init(void)
{
	return of_register_platform_driver(&ide_of_platform_driver);
}
module_init(ide_of_platform_init);

static void __exit ide_of_platform_exit(void)
{
	of_unregister_platform_driver(&ide_of_platform_driver);
}
module_exit(ide_of_platform_exit);

MODULE_DESCRIPTION("OF-platform IDE driver");
MODULE_AUTHOR("Aries Fang <Aries.Fang@Quantatw.com>");
MODULE_LICENSE("GPL");
