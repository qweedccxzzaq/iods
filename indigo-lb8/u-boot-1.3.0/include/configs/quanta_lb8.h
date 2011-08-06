/*
 * Copyright 2004, 2007 Freescale Semiconductor.
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
 * mpc8548cds board configuration file
 *
 * Please refer to doc/README.mpc85xxcds for more info.
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48 */
#define CONFIG_MPC8548		1	/* MPC8548 specific */

#define CONFIG_PCI		/* enable any pci type devices */
#if 0 /*Yvonne. 4/2/2010*/
#define CONFIG_PCI1		/* PCI controller 1 */
#endif
#define CONFIG_PCIE1		/* PCIE controler 1 (slot 1) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */

#define CONFIG_TSEC_ENET		/* tsec ethernet support */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/
#define CONFIG_DDR_DLL			/* possible DLL fix needed */
#undef CONFIG_DDR_2T_TIMING		/* Sets the 2T timing bit */

#undef CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef
#undef CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */


/*
 * When initializing flash, if we cannot find the manufacturer ID,
 * assume this is the AMD flash associated with the CDS board.
 * This allows booting from a promjet.
 */
#define CONFIG_ASSUME_AMD_FLASH

#define MPC85xx_DDR_SDRAM_CLK_CNTL	/* 85xx has clock control reg */

#ifndef __ASSEMBLY__
extern unsigned long get_clock_freq(void);
#endif
#define CONFIG_SYS_CLK_FREQ	(66666000) /* sysclk for MPC85xx */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */
#define CONFIG_CLEAR_LAW0		/* Clear LAW0 in cpu_init_r */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */

#undef	CFG_DRAM_TEST			/* memory test, takes time */
#ifdef CFG_DRAM_TEST
#define MEM_TEST_START 0x00010000
#define MEM_TEST_END 0x20000000
#endif

#define CFG_MEMTEST_START	0x00200000	/* memtest works on */
#define CFG_MEMTEST_END		0x00210000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CFG_CCSRBAR_DEFAULT	0xff700000	/* CCSRBAR Default */
#define CFG_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CFG_IMMR		CFG_CCSRBAR	/* PQII uses CFG_IMMR */

#define CFG_PCI1_ADDR	(CFG_CCSRBAR+0x8000)
#define CFG_PCI2_ADDR	(CFG_CCSRBAR+0x9000)
#define CFG_PCIE1_ADDR	(CFG_CCSRBAR+0xa000)

/*
 * DDR Setup
 */
#define CFG_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_SDRAM_BASE

#define SPD_EEPROM_ADDRESS	0x53		/* DDR DIMM */

/*
 * Make sure required options are set
 */
#ifndef CONFIG_SPD_EEPROM
#error ("CONFIG_SPD_EEPROM is required by LB8")
#endif

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Local Bus Definitions
 */

/*
 * FLASH on the Local Bus
 * Two banks, 32M each, using the CFI driver.
 * Boot from BR0/OR0 bank at 0xfe00_0000
 * Alternate BR1/OR1 bank at 0xfc00_0000
 *
 * BR0, BR1:
 *    Base address 0 = 0xfe00_0000 = BR0[0:16] = 1111 1110 0000 0000 0
 *    Base address 1 = 0xfc00_0000 = BR1[0:16] = 1111 1100 0000 0000 0
 *    Port Size = 16 bits = BRx[19:20] = 10
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1110 0000 0000 0001 0000 0000 0001 = fe001001	 BR0
 * 1111 1100 0000 0000 0001 0000 0000 0001 = fc001001	 BR1
 *
 * OR0, OR1:
 *    Addr Mask = 32M = ORx[0:16] = 1111 1110 0000 0000 0
 *    Reserved ORx[17:18] = 11, confusion here?
 *    BCTLD = LBCTL not asserted = ORx[19] = 1
 *    CSNT = ORx[20] = 1
 *    ACS = half cycle delay = ORx[21:22] = 11
 *    XACS = no extra setup = ORx[23] = 0
 *    SCY = 6 = ORx[24:27] = 0110
 *    SETA = cycle terminated internaly = ORx[28] = 0
 *    TRLX = use relaxed timing = ORx[29] = 1
 *    EHTR = use relaxed timing = ORx[30] = 0
 *    EAD = use external address latch delay = OR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1110 0000 0000 0111 1110 0110 0101 = fe007e65    OR0
 * 1111 1110 0000 0000 0111 1110 0110 0101 = fe007e65    OR1
 */

#define CFG_BOOT_BLOCK		0xfe000000	/* boot TLB block */
#define CFG_FLASH_BASE		0xfc000000	/* start of FLASH 64M */

#define CFG_BR0_PRELIM		0xfe001001
#define CFG_BR1_PRELIM		0xfc001001

#define	CFG_OR0_PRELIM		0xfe007e65    
#define	CFG_OR1_PRELIM		0xfe007e65    

#define CFG_FLASH_BANKS_LIST	{0xfe000000, CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS	2		/* number of banks */
#define CFG_MAX_FLASH_SECT	 256		/* sectors per device */
#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CFG_MONITOR_BASE	TEXT_BASE	/* start of monitor */

#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_EMPTY_INFO

/*
 * Compact FLASH on the Local Bus
 * Two bank, 1M, using the IDE driver.
 *
 * BR5,BR2:
 *    Base address 5 = 0xF000_0000 = BR5[0:16] = 1111 0000 0000 0000 0
 *    Base address 2 = 0xF001_0000 = BR2[0:16] = 1111 0000 0000 0001 0
 *    Port Size = 32 bits = BRx[19:20] = 11
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0000 0001 = f0001801    BR5
 * 1111 0000 0000 0001 0001 1000 0000 0001 = f0011801    BR2
 *
 * OR5, OR2:
 *    Addr Mask = 64K = ORx[0:16] = 1111 1111 1111 1111 0
 *    Reserved ORx[17:18] = 11, confusion here?
 *    BCTLD = LBCTL not asserted = ORx[19] = 0
 *    CSNT = ORx[20] = 1
 *    ACS = half cycle delay = ORx[21:22] = 00
 *    XACS = no extra setup = ORx[23] = 0
 *    SCY = 10 = ORx[24:27] = 1010
 *    SETA = cycle terminated internaly = ORx[28] = 0
 *    TRLX = use relaxed timing = ORx[29] = 0
 *    EHTR = use relaxed timing = ORx[30] = 0
 *    EAD = use external address latch delay = OR[31] = 0
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0110 1000 1010 0000 = ffFF68A0    ORx
 * 1111 1111 1000 0000 0110 1000 1010 0000 = ffFF68A0    ORx
 */

#define CFG_CFLASH0_BASE	0xf0000000	/* start of compact Flash */

#define CFG_CFLASH1_BASE	0xf0010000	/* start of compact Flash */

#define CFG_BR5_PRELIM		0xf0001801
#define CFG_BR2_PRELIM		0xf0011801

#define	CFG_OR5_PRELIM		0xffff68A0
#define	CFG_OR2_PRELIM		0xffff68A0


/*
 * CPLD on the Local Bus
 * One bank, 1M, using the IDE driver.
 *
 * BR4:
 *    Base address = 0xF200_0000 = BRx[0:16] = 1111 0010 0000 0000 0
 *    Port Size = 32 bits = BRx[19:20] = 11
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0010 0000 0010 0001 0000 0000 0001 = F2001801    BR4
 *
 * OR4:
 *    Addr Mask = 64K = ORx[0:16] = 1111 1111 1111 1111 0
 *    Reserved ORx[17:18] = 11, confusion here?
 *    BCTLD = LBCTL not asserted = ORx[19] = 1
 *    CSNT = ORx[20] = 0
 *    ACS = half cycle delay = ORx[21:22] = 11
 *    XACS = no extra setup = ORx[23] = 0
 *    SCY = 6 = ORx[24:27] = 0110
 *    SETA = cycle terminated internaly = ORx[28] = 0
 *    TRLX = use relaxed timing = ORx[29] = 1
 *    EHTR = use relaxed timing = ORx[30] = 0
 *    EAD = use external address latch delay = OR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1111 1000 0000 0111 0110 0110 0101 = ffff7665    ORx
 */


#define CPLD_BASE_ADDR 0xf2000000
#define CFG_BR4_PRELIM   0xf2001801
#define CFG_OR4_PRELIM   0xffff7665

#define CONFIG_L1_INIT_RAM
#define CFG_INIT_RAM_ADDR	0xE8000000		/* Initial RAM address */
//#define CFG_INIT_RAM_END    	0x20000	    /* End of used area in RAM */
#define CFG_INIT_RAM_END    	0x4000	    /* End of used area in RAM */

#define CFG_INIT_L2_ADDR	0xf8f80000	/* relocate boot L2SRAM */

#define CFG_GBL_DATA_SIZE  	1024	    /* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN	    	(512 * 1024) /* Reserve 512 kB for Mon */
#define CFG_MALLOC_LEN	    	(256 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CFG_NS16550_COM1	(CFG_CCSRBAR+0x4500)
#define CFG_NS16550_COM2	(CFG_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_FLAT_TREE	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,8548@0"
#define OF_SOC			"soc8548@e0000000"
#define OF_TBCLK		(bd->bi_busfreq / 8)
#define OF_STDOUT_PATH		"/soc8548@e0000000/serial@4600"
#define OF_PCI			"pci@e0008000"

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_EEPROM_ADDR	0x54
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{0x69}	/* Don't probe these addrs */
#define CFG_I2C_OFFSET		0x3000
#define CFG_I2C2_OFFSET		0x3100  /* Yvonne add. 4/1/2010 */

/* KEN enable multi I2C */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CFG_PCI_PHYS		0x80000000	/* 1G PCI TLB */

#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCI1_IO_BASE	0xe2000000
#define CFG_PCI1_IO_PHYS	CFG_PCI1_IO_BASE
#define CFG_PCI1_IO_SIZE	0x1000000	/* 16M */


#ifdef CONFIG_PCIE1
#define CFG_PCIE1_MEM_BASE	0xa0000000
#define CFG_PCIE1_MEM_PHYS	CFG_PCIE1_MEM_BASE
#define CFG_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CFG_PCIE1_IO_BASE	0xe3000000
#define CFG_PCIE1_IO_PHYS	CFG_PCIE1_IO_BASE
#define CFG_PCIE1_IO_SIZE	0x1000000	/* 16M */
#endif



#if defined(CONFIG_PCI)

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP


    #define PCI_ENET0_IOADDR      0xe0000000
    #define PCI_ENET0_MEMADDR     0xe0000000
    #define PCI_IDSEL_NUMBER      0x0c 	/*slot0->3(IDSEL)=12->15*/


#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

/* PCI view of System Memory */
#define CFG_PCI_MEMORY_BUS	0x00000000
#define CFG_PCI_MEMORY_PHYS	0x00000000
#define CFG_PCI_MEMORY_SIZE	0x80000000

#endif	/* CONFIG_PCI */


#if defined(CONFIG_TSEC_ENET)

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

#define CONFIG_MII		1	/* MII PHY management */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#if 0 /* KEN disable service port 2 */
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC1"
#endif

#define TSEC1_PHY_ADDR		1
#define TSEC2_PHY_ADDR		2

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"eTSEC0"
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0xC0000)
#define CFG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
#define CFG_ENV_SIZE		0x2000

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	*/
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

/* Cache Configuration */
#define CFG_DCACHE_SIZE	32768
#define CFG_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/*log base 2 of the above value*/
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

/* The mac addresses for all ethernet interface */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_ETHADDR	 00:E0:0C:00:00:FD

#if 0 /*Yvonne remove. 9/9/2010*/
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR	 00:E0:0C:00:01:FD
#endif
#endif

#define CONFIG_IPADDR    192.168.2.1

#define CONFIG_HOSTNAME  LB8_X
#define CONFIG_ROOTPATH	 /nfsroot
#define CONFIG_BOOTFILE  eldk-quanta
#define CONFIG_UBOOTPATH	u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP  192.168.2.12
#define CONFIG_GATEWAYIP 192.168.2.254
#define CONFIG_NETMASK   255.255.255.0

#define CONFIG_LOADADDR  4000000   /*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 5	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs*/

#define CONFIG_BAUDRATE	115200

#if defined(CONFIG_PCIE1)
#define PCIE_ENV \
 "pciereg=md ${a}000 6; md ${a}020 4; md ${a}bf8 2; echo o;md ${a}c00 25;" \
	"echo i; md ${a}da0 15; echo e;md ${a}e00 e; echo d; md ${a}f00 c\0" \
 "pcieerr=md ${a}020 1; md ${a}e00 e; pci d.b $b.0 7 1; pci d.w $b.0 1e 1;" \
	"pci d.w $b.0 56 1; pci d $b.0 104 1; pci d $b.0 110 1;" \
	"pci d $b.0 130 1\0" \
 "pcieerrc=mw ${a}020 ffffffff; mw ${a}e00 ffffffff; pci w.b $b.0 7 ff;" \
	"pci w.w $b.0 1e ffff; pci w.w $b.0 56 ffff; pci w $b.0 104 ffffffff;"\
	"pci w $b.0 110 ffffffff; pci w $b.0 130 ffffffff\0" \
 "pciecfg=pci d $b.0 0 20; pci d $b.0 100 e; pci d $b.0 400 69\0" \
 "pcie1regs=setenv a e000a; run pciereg\0" \
 "pcie1cfg=setenv b 3; run pciecfg\0" \
 "pcie1err=setenv a e000a; setenv b 3; run pcieerr\0" \
 "pcie1errc=setenv a e000a; setenv b 3; run pcieerrc\0"
#else
#define	PCIE_ENV ""
#endif

#if defined(CONFIG_PCI1) || defined(CONFIG_PCI2)
#define PCI_ENV \
 "pcireg=md ${a}000 3; echo o;md ${a}c00 25; echo i; md ${a}da0 15;" \
	"echo e;md ${a}e00 9\0" \
 "pcierr=md ${a}e00 8; pci d.b $b.0 7 1;pci d.w $b.0 1e 1;" \
	"pci d.w $b.0 56 1\0" \
 "pcierrc=mw ${a}e00 ffffffff; mw ${a}e0c 0; pci w.b $b.0 7 ff;" \
	"pci w.w $b.0 1e ffff; pci w.w $b.0 56 ffff\0"
#else
#define	PCI_ENV ""
#endif

#if defined(CONFIG_PCI1)
#define PCI_ENV1 \
 "pci1regs=setenv a e0008; run pcireg\0" \
 "pci1err=setenv a e0008; setenv b 0; run pcierr\0" \
 "pci1errc=setenv a e0008; setenv b 0; run pcierrc\0"
#else
#define	PCI_ENV1 ""
#endif

#if defined(CONFIG_PCI2)
#define PCI_ENV2 \
 "pci2regs=setenv a e0009; run pcireg\0" \
 "pci2err=setenv a e0009; setenv b 123; run pcierr\0"	\
 "pci2errc=setenv a e0009; setenv b 123; run pcierrc\0"
#else
#define	PCI_ENV2 ""
#endif

#if defined(CONFIG_TSEC_ENET)
#define ENET_ENV \
 "enetreg1=md ${a}000 2; md ${a}010 9; md ${a}050 4; md ${a}08c 1;" \
	"md ${a}098 2\0" \
 "enetregt=echo t;md ${a}100 6; md ${a}140 2; md ${a}180 10; md ${a}200 10\0" \
 "enetregr=echo r;md ${a}300 6; md ${a}330 5; md ${a}380 10; md ${a}400 10\0" \
 "enetregm=echo mac;md ${a}500 5; md ${a}520 28;echo fifo;md ${a}a00 1;" \
	"echo mib;md ${a}680 31\0" \
 "enetreg=run enetreg1; run enetregm; run enetregt; run enetregr\0" \
 "enet1regs=setenv a e0024; run enetreg\0" \
 "enet2regs=setenv a e0025; run enetreg\0" \
 "enet3regs=setenv a e0026; run enetreg\0" \
 "enet4regs=setenv a e0027; run enetreg\0"
#else
#define ENET_ENV ""
#endif

#if 0
#define	CONFIG_EXTRA_ENV_SETTINGS				\
 "netdev=eth0\0"						\
 "uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
 "tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " MK_STR(TEXT_BASE) " +$filesize; "	\
	"erase " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize; "	\
	"protect on " MK_STR(TEXT_BASE) " +$filesize; "		\
	"cmp.b $loadaddr " MK_STR(TEXT_BASE) " $filesize\0"	\
 "consoledev=ttyS1\0"				\
 "ramdiskaddr=2000000\0"			\
 "ramdiskfile=ramdisk.uboot\0"			\
 "fdtaddr=c00000\0"				\
 "fdtfile=mpc8548cds.dtb\0"			\
 "eoi=mw e00400b0 0\0"				\
 "iack=md e00400a0 1\0"				\
 "ddrreg=md ${a}000 8; md ${a}080 8;md ${a}100 d; md ${a}140 4; md ${a}bf0 4;" \
	"md ${a}e00 3; md ${a}e20 3; md ${a}e40 7; md ${a}f00 5\0" \
 "ddrregs=setenv a e0002; run ddrreg\0"		\
 "gureg=md ${a}000 2c; md ${a}0b0 1; md ${a}0c0 1; md ${a}b20 3;" \
	"md ${a}e00 1; md ${a}e60 1; md ${a}ef0 15\0"	\
 "guregs=setenv a e00e0; run gureg\0"		\
 "ecmreg=md ${a}000 1; md ${a}010 1; md ${a}bf8 2; md ${a}e00 6\0" \
 "ecmregs=setenv a e0001; run ecmreg\0" \
 "lawregs=md e0000c08 4b\0" \
 "lbcregs=md e0005000 36\0" \
 "dma0regs=md e0021100 12\0" \
 "dma1regs=md e0021180 12\0" \
 "dma2regs=md e0021200 12\0" \
 "dma3regs=md e0021280 12\0" \
 PCIE_ENV \
 PCI_ENV \
 PCI_ENV1 \
 PCI_ENV2 \
 ENET_ENV

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr - $fdtaddr"


#define CONFIG_RAMBOOTCOMMAND \
   "setenv bootargs root=/dev/ram rw "					\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $fdtaddr $fdtfile;"						\
   "bootm $loadaddr $ramdiskaddr $fdtaddr"

#endif


#if defined(QUANTA_LB8)

#define CONFIG_FLASH_BOOTCOMMAND \
   "setenv bootargs root=/dev/ram "                                     \
   "console=ttyS0,$baudrate; "                                  \
   "bootm ffd00000 fef00000 ffee0000" /*Yvonne. 3/30/2010 original is [ffd00000 ff000000 ffee0000]*/

#define CONFIG_CFCARD_BOOTCOMMAND \
   "setenv bootargs root=/dev/ram "                                     \
   "console=ttyS0,$baudrate; "                                  \
   "ext2load ide 0:1 0x1000000 /uImage;ext2load ide 0:1 0x2000000 /uInitrd2m;ext2load ide 0:1 0x400000 /LB8.dtb;" \
   "bootm 1000000 2000000 400000"


#define CONFIG_BOOTCOMMAND                                              \
   "run flash_bootcmd "


#endif
/* #define CONFIG_BOOTCOMMAND  CONFIG_NFSBOOTCOMMAND */

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */

#define CONFIG_MAC_PARTITION    1
#define CONFIG_DOS_PARTITION    1
#define CONFIG_ISO_PARTITION	1
 
#undef	CONFIG_IDE_8xx_DIRECT		    /* no pcmcia interface required */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#define CONFIG_IDE_RESET	1	/* reset for ide supported	*/

#define CFG_IDE_MAXBUS		1		/* max. 1 IDE busses	*/
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*1) /* max. 1 drives per IDE bus */

#define CFG_ATA_BASE_ADDR	0xF0000000
#define CFG_ATA_IDE0_OFFSET	0x00000

#define CFG_ATA_DATA_OFFSET	0x00002	/* Offset for data I/O			*/
#define CFG_ATA_REG_OFFSET	0x00003	/* Offset for normal register accesses	*/
#define CFG_ATA_ALT_OFFSET	0x10000	/* Offset for alternate registers	*/
#define CFG_ATA_STRIDE      4
#define CFG_IDE_SWAPBYTES

#endif	/* __CONFIG_H */
