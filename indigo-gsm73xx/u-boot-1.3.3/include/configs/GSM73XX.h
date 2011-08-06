/*
 * Copyright 2007 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * mpc8544ds board configuration file
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_CUST_DO_RESET	1

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8544		1
#define CONFIG_GSM73XX		1

#define CONFIG_BOOTARGS		"console=ttyS0,115200 init=/etc/preinit"
#define CONFIG_BOOTCOMMAND	"bootm ffc00000 ffd00000"

#define CONFIG_BOOTDELAY        5

#define CONFIG_PCI		1	/* Enable PCI/PCIE */
#define CONFIG_PCI1		1	/* PCI controller 1 */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_FSL_PCIE_RESET	1	/* need PCIe reset errata */

#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

#define CONFIG_ENV_OVERWRITE
#undef CONFIG_DDR_DLL
#define CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */

#define CONFIG_MEM_INIT_VALUE		0xDeadBeef

#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#define MPC85xx_DDR_SDRAM_CLK_CNTL	/* 85xx has clock control reg */

#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0) /* sysclk for MPC85xx */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */
#define CONFIG_ADDR_STREAMING		/* toggle addr streaming */
#define CONFIG_CLEAR_LAW0		/* Clear LAW0 in cpu_init_r */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CFG_DRAM_TEST			/* memory test, takes time */
#define CFG_MEMTEST_START	0x00200000	/* memtest works on */
#define CFG_MEMTEST_END		0x00400000
#define CFG_ALT_MEMTEST
#define CONFIG_PANIC_HANG	/* do not reset board on panic */

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CFG_CCSRBAR_PHYS	CFG_CCSRBAR	/* physical addr of CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

#define CFG_PCI1_ADDR		(CFG_CCSRBAR+0x8000)
#define CFG_PCIE1_ADDR		(CFG_CCSRBAR+0xa000)
#define CFG_PCIE2_ADDR		(CFG_CCSRBAR+0x9000)
#define CFG_PCIE3_ADDR		(CFG_CCSRBAR+0xb000)

/*
 * DDR Setup
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE


#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Memory map
 *
 * 0x0000_0000	0x7fff_ffff	DDR			2G Cacheable
 *
 * 0x8000_0000	0xbfff_ffff	PCI Express Mem		1G non-cacheable
 *
 * 0xc000_0000	0xdfff_ffff	PCI			512M non-cacheable
 *
 * 0xe000_0000	0xe00f_ffff	CCSR			1M non-cacheable
 * 0xe100_0000	0xe3ff_ffff	PCI IO range		4M non-cacheable
 *
 * Localbus cacheable
 *
 * 0xf000_0000	0xf3ff_ffff	SDRAM			64M Cacheable
 * 0xf401_0000	0xf401_3fff	L1 for stack		4K Cacheable TLB0
 *
 * Localbus non-cacheable
 *
 * 0xf800_0000	0xf80f_ffff	NVRAM/CADMUS (*)	1M non-cacheable
 * 0xff00_0000	0xff7f_ffff	FLASH (2nd bank)	8M non-cacheable
 * 0xff80_0000	0xffff_ffff	FLASH (boot bank)	8M non-cacheable
 *
 */

/*
 * Local Bus Definitions
 */
#define CFG_BOOT_BLOCK		0xfc000000	/* boot TLB */

#define CFG_LBC_CACHE_BASE	0xf0000000	/* Localbus cacheable */

#define CFG_FLASH_BASE		0xfe000000	/* start of FLASH 8M */

#define CFG_BR0_PRELIM		0xfe001801
#define CFG_OR0_PRELIM		0xfe006ff7

#define CFG_BR3_PRELIM		0xfc000801
#define CFG_OR3_PRELIM		0xfc006ff7


#define CFG_FLASH_BANKS_LIST	{CFG_FLASH_BASE}

#define CFG_FLASH_QUIET_TEST
#define CFG_MAX_FLASH_BANKS	1		/* number of banks */
#define CFG_MAX_FLASH_SECT	256		/* sectors per device */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000		/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500		/* Flash Write Timeout (ms) */

#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#define CFG_WRITE_SWAPPED_DATA	1
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_32BIT
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO

#define CFG_LBC_NONCACHE_BASE	0xf8000000

/* define to use L1 as initial stack */
#define CONFIG_L1_INIT_RAM	1
#define CFG_INIT_L1_LOCK	1
#define CFG_INIT_L1_ADDR	0xf4010000	/* Initial L1 address */
#define CFG_INIT_L1_END		0x00004000	/* End of used area in RAM */

/* define to use L2SRAM as initial stack */
#undef CONFIG_L2_INIT_RAM
#define CFG_INIT_L2_ADDR	0xf8fc0000
#define CFG_INIT_L2_END		0x00040000	/* End of used area in RAM */

#ifdef CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_ADDR	CFG_INIT_L1_ADDR
#define CFG_INIT_RAM_END	CFG_INIT_L1_END
#else
#define CFG_INIT_RAM_ADDR	CFG_INIT_L2_ADDR
#define CFG_INIT_RAM_END	CFG_INIT_L2_END
#endif

#define CFG_GBL_DATA_SIZE	128	/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserved for malloc */

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1	(CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2	(CFG_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CFG_PCIE_PHYS		0x80000000	/* 1G PCIE TLB */
#define CFG_PCI_PHYS		0xc0000000	/* 512M PCI TLB */

#define CFG_PCI1_MEM_BASE	0xc0000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI1_IO_BASE	0x00000000
#define CFG_PCI1_IO_PHYS	0xe1000000
#define CFG_PCI1_IO_SIZE	0x00010000	/* 64k */

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS	0x00000000
#define CFG_PCI_MEMORY_PHYS	0x00000000
#define CFG_PCI_MEMORY_SIZE	0x80000000

/* controller 2, Slot 1, tgtid 1, Base address 9000 */
#define CFG_PCIE2_MEM_BASE	0x80000000
#define CFG_PCIE2_MEM_PHYS	CFG_PCIE2_MEM_BASE
#define CFG_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCIE2_IO_BASE	0x00000000
#define CFG_PCIE2_IO_PHYS	0xe1010000
#define CFG_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 1, Slot 2,tgtid 2, Base address a000 */
#define CFG_PCIE1_MEM_BASE	0xa0000000
#define CFG_PCIE1_MEM_PHYS	CFG_PCIE1_MEM_BASE
#define CFG_PCIE1_MEM_SIZE	0x10000000	/* 256M */
#define CFG_PCIE1_IO_BASE	0x00000000
#define CFG_PCIE1_IO_PHYS	0xe1020000
#define CFG_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 3, direct to uli, tgtid 3, Base address b000 */
#define CFG_PCIE3_MEM_BASE	0xb0000000
#define CFG_PCIE3_MEM_PHYS	CFG_PCIE3_MEM_BASE
#define CFG_PCIE3_MEM_SIZE	0x00100000	/* 1M */
#define CFG_PCIE3_IO_BASE	0x00000000
#define CFG_PCIE3_IO_PHYS	0xb0100000	/* reuse mem LAW */
#define CFG_PCIE3_IO_SIZE	0x00100000	/* 1M */
#define CFG_PCIE3_MEM_BASE2	0xb0200000
#define CFG_PCIE3_MEM_PHYS2	CFG_PCIE3_MEM_BASE2
#define CFG_PCIE3_MEM_SIZE2	0x00200000	/* 1M */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#ifndef CONFIG_PCI_PNP
	#define PCI_ENET0_IOADDR	CFG_PCI1_IO_BASE
	#define PCI_ENET0_MEMADDR	CFG_PCI1_IO_BASE
	#define PCI_IDSEL_NUMBER	0x11	/* IDSEL = AD11 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif	/* CONFIG_PCI */

/*
 * Environment
 */

#define CFG_ENV_SIZE		0x2000
#define CFG_ENV_SECT_SIZE	0x40000 /* 256K (one sector) */


#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + (28 * 1024 * 1024) - CFG_ENV_SECT_SIZE)

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif

#undef CONFIG_CMD_NET
#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	*/
#define CONFIG_CMDLINE_EDITING		/* Command-line editing */
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

/*
 * Environment Configuration
 */

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/
#define CONFIG_BAUDRATE	115200

#endif	/* __CONFIG_H */

