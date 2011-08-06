/*
 * Copyright 2004 Freescale Semiconductor.
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <ioports.h>
#include <spd.h>

#include "../common/cadmus.h"
#include "../common/eeprom.h"
#include "../common/via.h"

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

extern long int spd_sdram(void);

void local_bus_init(void);
void sdram_init(void);


#if !defined(QUANTA_LB9A)
#error "Platform define error!"
#endif

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 *
 * ppar=1 => specialized function
 * ppar=0 => general purpose I/O
 * pdir=1 => output
 * pdir=0 => input
 * psor=0 => specialized function #1
 * psor=1 => specialized function #2
 * podr=0 => normal output
 * podr=1 => open-drain
 */
#if defined(QUANTA_LB9A)
const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {   /*         conf ppar psor pdir podr pdat */
	/* PA31 */ {   0,   0,   0,   0,   0,   0   }, /*pin doesn't exist */
	/* PA30 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist*/
	/* PA29 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist*/
	/* PA28 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA27 */ {   1,   0,   0,   0,   0,   0   }, /* AD_PWR_GD */
	/* PA26 */ {   1,   0,   0,   0,   0,   0   }, /* P1V2_PG */
	/* PA25 */ {   1,   0,   0,   1,   0,   1   }, /* SOFT_RST_N (Software reset, active low)*/
	/* PA24 */ {   1,   0,   0,   1,   0,   1   }, /* HARD_RST_N (Hardware reset, active low)*/
	/* PA23 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA22 */ {   1,   0,   0,   1,   0,   0   }, /* STATUS_LED (The LED indicate system status, active low)*/	
	/* PA21 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA20 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA19 */ {   1,   0,   0,   1,   0,   1   }, /* CF_BUS_EN_N (Bus enable for Compact Flash card, active low)*/	
	/* PA18 */ {   1,   0,   0,   1,   0,   1   }, /* CF_PWR_EN_N (Power enable for Compact Flash card, active low) */	
	/* PA17 */ {   1,   0,   0,   1,   0,   0   }, /* CF_RST_N (Reset Compact Flash card, active low)*/	
	/* PA16 */ {   1,   0,   0,   1,   0,   0   }, /* DMA_RST (Reset CPLD, active low)*/
	/* PA15 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA14 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA13 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA12 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA11 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA10 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA9  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA8  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA7  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA6  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA5  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA4  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PA0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port B configuration */
    {   /*         conf ppar psor pdir podr pdat */
	/* PB31 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB30 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB29 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB28 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB27 */ {   1,   0,   0,   0,   0,   0   }, /* MODULE_INT_N */
	/* PB26 */ {   1,   0,   0,   1,   0,   1   }, /* PHY_ISO_N */
	/* PB25 */ {   1,   0,   0,   1,   0,   1   }, /* MOD_RST_N */
	/* PB24 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB23 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB22 */ {   1,   0,   0,   1,   0,   1   }, /* F_RST_N */	
	/* PB21 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB20 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB19  */ {   1,   0,   0,   0,   0,   0   }, /* MOD_ID_SEL1 */
	/* PB18  */ {   1,   0,   0,   0,   0,   0   }, /* MOD_ID_SEL0 */	
	/* PB17 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB16 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB15 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB14 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB13 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB12 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB11 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB10 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB9  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB8  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB7  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB6  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB5  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB4  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*         conf ppar psor pdir podr pdat */
	/* PC31 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC30 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC29 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC28 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC27 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC26 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC25 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC24 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC23 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC22 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC21 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC20 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC19 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC18 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC17 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC16 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC15 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC14 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC13 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC12 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC11 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC10 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC9  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC8  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC7  */ {   1,   0,   0,   0,   0,   0   }, /* ADT_IRQ_N */
	/* PC6  */ {   1,   0,   0,   0,   0,   0   }, /* CF_OC_N (Compact Flash card over-current detect active low)*/
	/* PC5  */ {   1,   0,   0,   0,   0,   0   }, /* CF_DET1 (Compact Flash card present 1, active low)*/
	/* PC4  */ {   1,   0,   0,   0,   0,   0   }, /* CF_DET0 (Compact Flash card present 0, active low)*/	
	/* PC3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PC0  */ {   1,   0,   0,   0,   0,   0   }, /* RTC_IRQ_N */
     },

    /* Port D */
    {   /*         conf ppar psor pdir podr pdat */
	/* PD31 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD30 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD29 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD28 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD27 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD26 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD25 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD24 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD23 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD22 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD21 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD20 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD19 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD18 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD17 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD16 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD15 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD14 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD13 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD12 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD11 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD10 */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD9  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD8  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD7  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD6  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD5  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD4  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};
#else
#error "Platform define error!"
#endif

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	volatile immap_t *immap = (immap_t *) CFG_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
    volatile ccsr_cpm_iop_t *pio;
	/*
	 * Initialize local bus.
	 */
	local_bus_init ();
	return 0;
}

long int
initdram(int board_type)
{
	long dram_size = 0;
	volatile immap_t *immap = (immap_t *)CFG_IMMR;

	puts("Initializing\n");

#if defined(CONFIG_DDR_DLL)
	{
		/*
		 * Work around to stabilize DDR DLL MSYNC_IN.
		 * Errata DDR9 seems to have been fixed.
		 * This is now the workaround for Errata DDR11:
		 *    Override DLL = 1, Course Adj = 1, Tap Select = 0
		 */

		volatile ccsr_gur_t *gur= &immap->im_gur;

		gur->ddrdllcr = 0x81000000;
		asm("sync;isync;msync");
		udelay(200);
	}
#endif
	dram_size = spd_sdram();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(dram_size);
#endif
	/*
	 * SDRAM Initialization
	 */
#if !defined(QUANTA_LB9A)
	sdram_init();
#endif

	puts("    DDR: ");
	return dram_size;
}

/*
 * Initialize Local Bus
 */
void
local_bus_init(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	volatile ccsr_lbc_t *lbc = &immap->im_lbc;

	uint clkdiv;
	uint lbc_hz;
	sys_info_t sysinfo;
	uint temp_lbcdll;

	/*
	 * Errata LBC11.
	 * Fix Local Bus clock glitch when DLL is enabled.
	 *
	 * If localbus freq is < 66Mhz, DLL bypass mode must be used.
	 * If localbus freq is > 133Mhz, DLL can be safely enabled.
	 * Between 66 and 133, the DLL is enabled with an override workaround.
	 */

	get_sys_info(&sysinfo);
	clkdiv = lbc->lcrr & 0x0f;
	lbc_hz = sysinfo.freqSystemBus / 1000000 / clkdiv;
#if 1
	if (lbc_hz < 66) {
		lbc->lcrr |= 0x80000000;	/* DLL Bypass */

	} else if (lbc_hz >= 133) {
		lbc->lcrr &= (~0x80000000);		/* DLL Enabled */

	} else {
		lbc->lcrr &= (~0x8000000);	/* DLL Enabled */
		udelay(200);

		/*
		 * Sample LBC DLL ctrl reg, upshift it to set the
		 * override bits.
		 */
		temp_lbcdll = gur->lbcdllcr;
		gur->lbcdllcr = (((temp_lbcdll & 0xff) << 16) | 0x80000000);
		asm("sync;isync;msync");
	}
#else
		lbc->lcrr = 0x00030008;
#endif
}

#if !defined(QUANTA_LB9A)
/*
 * Initialize SDRAM memory on the Local Bus.
 */
void
sdram_init(void)
{
#if defined(CFG_OR2_PRELIM) && defined(CFG_BR2_PRELIM)

	uint idx;
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_lbc_t *lbc = &immap->im_lbc;
	uint *sdram_addr = (uint *)CFG_LBC_SDRAM_BASE;
	uint cpu_board_rev;
	uint lsdmr_common;

	puts("    SDRAM: ");

	print_size (CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers
	 */
	lbc->or2 = CFG_OR2_PRELIM;
	asm("msync");

	lbc->br2 = CFG_BR2_PRELIM;
	asm("msync");

	lbc->lbcr = CFG_LBC_LBCR;
	asm("msync");


	lbc->lsrt = CFG_LBC_LSRT;
	lbc->mrtpr = CFG_LBC_MRTPR;
	asm("msync");

	/*
	 * Determine which address lines to use baed on CPU board rev.
	 */
	cpu_board_rev = get_cpu_board_revision();
	lsdmr_common = CFG_LBC_LSDMR_COMMON;
	if (cpu_board_rev == MPC85XX_CPU_BOARD_REV_1_0) {
		lsdmr_common |= CFG_LBC_LSDMR_BSMA1617;
	} else if (cpu_board_rev == MPC85XX_CPU_BOARD_REV_1_1) {
		lsdmr_common |= CFG_LBC_LSDMR_BSMA1516;
	} else {
		/*
		 * Assume something unable to identify itself is
		 * really old, and likely has lines 16/17 mapped.
		 */
		lsdmr_common |= CFG_LBC_LSDMR_BSMA1617;
	}

	/*
	 * Issue PRECHARGE ALL command.
	 */
	lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_PCHALL;
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(100);

	/*
	 * Issue 8 AUTO REFRESH commands.
	 */
	for (idx = 0; idx < 8; idx++) {
		lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_ARFRSH;
		asm("sync;msync");
		*sdram_addr = 0xff;
		ppcDcbf((unsigned long) sdram_addr);
		udelay(100);
	}

	/*
	 * Issue 8 MODE-set command.
	 */
	lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_MRW;
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(100);

	/*
	 * Issue NORMAL OP command.
	 */
	lbc->lsdmr = lsdmr_common | CFG_LBC_LSDMR_OP_NORMAL;
	asm("sync;msync");
	*sdram_addr = 0xff;
	ppcDcbf((unsigned long) sdram_addr);
	udelay(200);    /* Overkill. Must wait > 200 bus cycles */

#endif	/* enable SDRAM init */
}
#endif


#if defined(CFG_DRAM_TEST)

int
testdram(void)
{
        unsigned int  MemoryTestStart, MemoryTestEnd;
        uint i;
        int do_test;


		
        printf("\nPress any key for RAM test, otherwise, just wait 2s!\n");
        do_test = 0;
        for (i=0; i<2000; ++i) {
            udelay (1000);
            if (tstc()) {
                (void) getc();
                do_test = 1;
                break;
            }
	}
        if ( do_test == 0 ) return 0;

        /* Check if RAM test desired */                

    MemoryTestStart = MEM_TEST_START;
    MemoryTestEnd = MEM_TEST_END;

    printf("\n\nSDRAM Test ... \n\n");
    printf("Adress From 0x%x to 0x%x\n\n",MemoryTestStart,MemoryTestEnd);

    /* DATA LINE TEST */
    printf("Start Data Line Test\n");
   DataLineTest(MemoryTestStart, MemoryTestEnd);
    printf("\n");

    /* ADDRESS LINE TEST */
    printf("Start Address Line Test\n");
    AddressLineTest(MemoryTestStart, MemoryTestEnd);
    printf("\n");

    /* Read/Write Test */
    printf("Start Read/Write Test\n");
    MemoryWriteReadTest(MemoryTestStart, MemoryTestEnd);
    printf("\n");

    /* Byte-Word-Long Test */
    printf("Start Byte-WORD-Long Test\n");
    BWL_Test(MemoryTestStart, MemoryTestEnd);
    printf("\n");

    /* Power of 2 Test */
    printf("Start Power of 2 Test\n");
    PO2_Test(MemoryTestStart, MemoryTestEnd);
    printf("\n");

    /* Walking 1 and 0 Test */
    printf("Start Walking 1 and 0 Test\n");
    WALKING10_Test(MemoryTestStart, MemoryTestEnd);
    printf("\n");

    /* XOR Test */
    printf("Start XOR Test\n");
    XOR_Test(MemoryTestStart, MemoryTestEnd);
    printf("\n");

    /* Test Over */
    printf("Test Over...\n\n");
    
    
        return 0;
}
#endif


#if defined(CONFIG_PCI)
/* For some reason the Tundra PCI bridge shows up on itself as a
 * different device.  Work around that by refusing to configure it.
 */
void dummy_func(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab) { }

#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc85xxcds_config_table[] = {
    { PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
      PCI_IDSEL_NUMBER, PCI_ANY_ID,
      pci_cfgfunc_config_device, { PCI_ENET0_IOADDR,
				   PCI_ENET0_MEMADDR,
				   PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER
      } },
    { }
};
#endif

static struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_mpc85xxcds_config_table,
#endif
};

#endif	/* CONFIG_PCI */

void
pci_init_board(void)
{
#ifdef CONFIG_PCI
	extern void pci_mpc85xx_init(struct pci_controller *hose);

	pci_mpc85xx_init(&hose);
#endif
}
ide_set_reset (int reset)
{
//**AG** TO BE COMPLETED
}

