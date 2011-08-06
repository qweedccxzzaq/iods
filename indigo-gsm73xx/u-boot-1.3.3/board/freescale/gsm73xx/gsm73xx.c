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

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/immap_fsl_pci.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>

#include "../common/pixis.h"

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

void sdram_init(void);

/*
 * Setup Local Access Window and TLB1 mappings for the requested
 * amount of memory.  Returns the amount of memory actually mapped
 * (usually the original request size), or 0 on error.
 *
 * NOTE : copy from spd_sdram.c
 */

static unsigned int
setup_laws_and_tlbs(unsigned int memsize)
{
	unsigned int tlb_size;
	unsigned int law_size;
	unsigned int ram_tlb_index;
	unsigned int ram_tlb_address;

	/*
	 * Determine size of each TLB1 entry.
	 */
	switch (memsize) {
	case 16:
	case 32:
		tlb_size = BOOKE_PAGESZ_16M;
		break;
	case 64:
	case 128:
		tlb_size = BOOKE_PAGESZ_64M;
		break;
	case 256:
	case 512:
		tlb_size = BOOKE_PAGESZ_256M;
		break;
	case 1024:
	case 2048:
		if (PVR_VER(get_pvr()) > PVR_VER(PVR_85xx))
			tlb_size = BOOKE_PAGESZ_1G;
		else
			tlb_size = BOOKE_PAGESZ_256M;
		break;
	default:
		puts("DDR: only 16M,32M,64M,128M,256M,512M,1G and 2G are supported.\n");

		/*
		 * The memory was not able to be mapped.
		 * Default to a small size.
		 */
		tlb_size = BOOKE_PAGESZ_64M;
		memsize=64;
		break;
	}

	/*
	 * Configure DDR TLB1 entries.
	 * Starting at TLB1 8, use no more than 8 TLB1 entries.
	 */
	ram_tlb_index = 8;
	ram_tlb_address = (unsigned int)CFG_DDR_SDRAM_BASE;
	while (ram_tlb_address < (memsize * 1024 * 1024)
	      && ram_tlb_index < 16) {
		set_tlb(1, ram_tlb_address, ram_tlb_address,
			MAS3_SX|MAS3_SW|MAS3_SR, 0,
			0, ram_tlb_index, tlb_size, 1);

		ram_tlb_address += (0x1000 << ((tlb_size - 1) * 2));
		ram_tlb_index++;
	}


	/*
	 * First supported LAW size is 16M, at LAWAR_SIZE_16M == 23.  Fnord.
	 */
	law_size = 19 + __ilog2(memsize);

	/*
	 * Set up LAWBAR for all of DDR.
	 */

#ifdef CONFIG_FSL_LAW
	set_law(1, CFG_DDR_SDRAM_BASE, law_size, LAW_TRGT_IF_DDR);
#endif

	/*
	 * Confirm that the requested amount of memory was mapped.
	 */
	return memsize;
}

#define GSM73XX_CB_OFFSET	0xfc000000
static void set_gsm73xx_cb (unsigned int off, char h)
{
	char d;
	volatile unsigned char *addr;

	addr = (volatile unsigned char *)(GSM73XX_CB_OFFSET + off);
	d = *addr;
	d |= h;
	*addr = d;
}

static char get_gsm73xx_cb (unsigned int off)
{
	volatile unsigned char *addr;

	addr = (volatile unsigned char *)(GSM73XX_CB_OFFSET + off);
	return *addr;
}



static void enable_module (int i)
{
	char type, type_reg;
	char shift = 2 * i;

	type_reg = get_gsm73xx_cb (0x06);

	type = ((type_reg & (0x03 << shift)) >> shift);
	switch (type) {
	case 0x00:
		set_gsm73xx_cb (0x08, 0x02 << i);
		break;
	case 0x01:
	case 0x10:
	default:
		break;
	}
}

int board_early_init_f (void)
{
	char mpresent;

	/* OPEN PCI interrupt */
	set_gsm73xx_cb (0x04, 0x80);

	/* OPEN POE power */
	set_gsm73xx_cb (0x10, 0x80);

	/* OPEN USB interrup */
	set_gsm73xx_cb (0x11, 0x01);

	/* check the module */
	mpresent = get_gsm73xx_cb (0x07);

	if ((mpresent & 0x01) == 0) 
		enable_module (0);

	if ((mpresent & 0x02) == 0) 
		enable_module (1);

	return 0;
}

int checkboard (void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);
	volatile ccsr_local_ecm_t *ecm = (void *)(CFG_MPC85xx_ECM_ADDR);

	if ((uint)&gur->porpllsr != 0xe00e0000) {
		printf("immap size error %x\n",&gur->porpllsr);
	}
	printf ("Board: GSM73xx\n");

	lbc->ltesr = 0xffffffff;	/* Clear LBC error interrupts */
	lbc->lteir = 0xffffffff;	/* Enable LBC error interrupts */
	ecm->eedr = 0xffffffff;		/* Clear ecm errors */
	ecm->eeer = 0xffffffff;		/* Enable ecm errors */

	return 0;
}

static long
fix_sdram (void)
{
	long dram_size = 256;

	volatile ccsr_ddr_t *ddr = (void *)(CFG_MPC85xx_DDR_ADDR);

	/* NOTE : the following value is referent to GSM73xxPSv1_Sv2 HWS_R07.pdf.
	 *        and the init seq is referent spd_sdram().
	 */

	ddr->cs0_bnds = 0x0000001f;
	ddr->cs0_config = 0x80010102;

	ddr->timing_cfg_0 = 0x00220802;
	ddr->timing_cfg_1 = 0x3733C322;
	ddr->timing_cfg_2 = 0x14904CC7;

	ddr->sdram_cfg = 0x43008000;

	ddr->sdram_mode = 0x20480432;
	ddr->sdram_mode_2 = 0x00000000;

	ddr->sdram_interval = 0x06180100;

	asm("sync;isync;msync");
	udelay(500);

	ddr->sdram_cfg_2 = 0x04400000;
	ddr->sdram_clk_cntl = 0x03800000;

	udelay(200);

	ddr->sdram_cfg = 0xc3008000;
	asm("sync;isync;msync");
	udelay(500);

	setup_laws_and_tlbs (dram_size);

	return dram_size;
}

long int
initdram(int board_type)
{
	long dram_size = 0;

	puts("Initializing\n");

#ifdef CONFIG_SPD_EEPROM
	dram_size = spd_sdram();
#else
	dram_size = fix_sdram();
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(dram_size);
#endif
	puts("    DDR: ");
	return dram_size * 1024 * 1024;
}

#if defined(CFG_DRAM_TEST)
int
testdram(void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
	uint *p;

	printf("Testing DRAM from 0x%08x to 0x%08x\n",
	       CFG_MEMTEST_START,
	       CFG_MEMTEST_END);

	printf("DRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test passed.\n");
	return 0;
}
#endif

#ifdef CONFIG_PCI1
static struct pci_config_table pci_gsm73xx_config_table[] = {
	{ 0x14e4, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	  0x16, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { 0xe1000000,
	  	0xc0000000,
		PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER
	}},
	{ 0x14e4, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	  0x17, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { 0xe1100000,
	  	0xc0010000,
		PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER
	}},

	{}
};

static struct pci_controller pci1_hose = {
	config_table: pci_gsm73xx_config_table,
};

#endif

#ifdef CONFIG_PCIE1
static struct pci_controller pcie1_hose;
#endif

#ifdef CONFIG_PCIE2
static struct pci_controller pcie2_hose;
#endif

#ifdef CONFIG_PCIE3
static struct pci_controller pcie3_hose;
#endif

int first_free_busno=0;

void
pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	uint devdisr = gur->devdisr;
	uint io_sel = (gur->pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;
	uint host_agent = (gur->porbmsr & MPC85xx_PORBMSR_HA) >> 16;

	debug ("   pci_init_board: devdisr=%x, io_sel=%x, host_agent=%x\n",
		devdisr, io_sel, host_agent);

	if (io_sel & 1) {
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII1_DIS))
			printf ("    eTSEC1 is in sgmii mode.\n");
		if (!(gur->pordevsr & MPC85xx_PORDEVSR_SGMII3_DIS))
			printf ("    eTSEC3 is in sgmii mode.\n");
	}

#ifdef CONFIG_PCIE3
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE3_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie3_hose;
	int pcie_ep = (host_agent == 1);
	int pcie_configured  = io_sel >= 1;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE3 connected to ULI as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCIE3_MEM_BASE,
			       CFG_PCIE3_MEM_PHYS,
			       CFG_PCIE3_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCIE3_IO_BASE,
			       CFG_PCIE3_IO_PHYS,
			       CFG_PCIE3_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = 3;
#ifdef CFG_PCIE3_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE3_MEM_BASE2,
			       CFG_PCIE3_MEM_PHYS2,
			       CFG_PCIE3_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno=hose->last_busno+1;
		printf ("    PCIE3 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);

		/*
		 * Activate ULI1575 legacy chip by performing a fake
		 * memory access.  Needed to make ULI RTC work.
		 */
		in_be32((u32 *)CFG_PCIE3_MEM_BASE);
	} else {
		printf ("    PCIE3: disabled\n");
	}

 }
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE3; /* disable */
#endif

#ifdef CONFIG_PCIE1
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie1_hose;
	int pcie_ep = (host_agent == 5);
	int pcie_configured  = io_sel & 6;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE1 connected to Slot2 as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCIE1_MEM_BASE,
			       CFG_PCIE1_MEM_PHYS,
			       CFG_PCIE1_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCIE1_IO_BASE,
			       CFG_PCIE1_IO_PHYS,
			       CFG_PCIE1_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = 3;
#ifdef CFG_PCIE1_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE1_MEM_BASE2,
			       CFG_PCIE1_MEM_PHYS2,
			       CFG_PCIE1_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;

		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);

		first_free_busno=hose->last_busno+1;
		printf("    PCIE1 on bus %02x - %02x\n",
		       hose->first_busno,hose->last_busno);

	} else {
		printf ("    PCIE1: disabled\n");
	}

 }
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE; /* disable */
#endif

#ifdef CONFIG_PCIE2
 {
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCIE2_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pcie2_hose;
	int pcie_ep = (host_agent == 3);
	int pcie_configured  = io_sel & 4;

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE)){
		printf ("\n    PCIE2 connected to Slot 1 as %s (base address %x)",
			pcie_ep ? "End Point" : "Root Complex",
			(uint)pci);
		if (pci->pme_msg_det) {
			pci->pme_msg_det = 0xffffffff;
			debug (" with errors.  Clearing.  Now 0x%08x",pci->pme_msg_det);
		}
		printf ("\n");

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCIE2_MEM_BASE,
			       CFG_PCIE2_MEM_PHYS,
			       CFG_PCIE2_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCIE2_IO_BASE,
			       CFG_PCIE2_IO_PHYS,
			       CFG_PCIE2_IO_SIZE,
			       PCI_REGION_IO);

		hose->region_count = 3;
#ifdef CFG_PCIE2_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE2_MEM_BASE2,
			       CFG_PCIE2_MEM_PHYS2,
			       CFG_PCIE2_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno=hose->last_busno+1;
		printf ("    PCIE2 on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);

	} else {
		printf ("    PCIE2: disabled\n");
	}

 }
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCIE2; /* disable */
#endif


#ifdef CONFIG_PCI1
{
	volatile ccsr_fsl_pci_t *pci = (ccsr_fsl_pci_t *) CFG_PCI1_ADDR;
	extern void fsl_pci_init(struct pci_controller *hose);
	struct pci_controller *hose = &pci1_hose;

	uint pci_agent = (host_agent == 6);
	uint pci_speed = 66666000; /*get_clock_freq (); PCI PSPEED in [4:5] */
	uint pci_32 = 1;
	uint pci_arb = gur->pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;	/* PORDEVSR[14] */
	uint pci_clk_sel = gur->porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;	/* PORPLLSR[16] */


	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		printf ("\n    PCI: %d bit, %s MHz, %s, %s, %s (base address %x)\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33333000) ? "33" :
			(pci_speed == 66666000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_agent ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter",
			(uint)pci
			);

		/* inbound */
		pci_set_region(hose->regions + 0,
			       CFG_PCI_MEMORY_BUS,
			       CFG_PCI_MEMORY_PHYS,
			       CFG_PCI_MEMORY_SIZE,
			       PCI_REGION_MEM | PCI_REGION_MEMORY);

		/* outbound memory */
		pci_set_region(hose->regions + 1,
			       CFG_PCI1_MEM_BASE,
			       CFG_PCI1_MEM_PHYS,
			       CFG_PCI1_MEM_SIZE,
			       PCI_REGION_MEM);

		/* outbound io */
		pci_set_region(hose->regions + 2,
			       CFG_PCI1_IO_BASE,
			       CFG_PCI1_IO_PHYS,
			       CFG_PCI1_IO_SIZE,
			       PCI_REGION_IO);
		hose->region_count = 3;
#ifdef CFG_PCIE3_MEM_BASE2
		/* outbound memory */
		pci_set_region(hose->regions + 3,
			       CFG_PCIE3_MEM_BASE2,
			       CFG_PCIE3_MEM_PHYS2,
			       CFG_PCIE3_MEM_SIZE2,
			       PCI_REGION_MEM);
		hose->region_count++;
#endif
		hose->first_busno=first_free_busno;
		pci_setup_indirect(hose, (int) &pci->cfg_addr, (int) &pci->cfg_data);

		fsl_pci_init(hose);
		first_free_busno=hose->last_busno+1;
		printf ("PCI on bus %02x - %02x\n",
			hose->first_busno,hose->last_busno);
	} else {
		printf ("    PCI: disabled\n");
	}
}
#else
	gur->devdisr |= MPC85xx_DEVDISR_PCI1; /* disable */
#endif
}


int last_stage_init(void)
{
	return 0;
}


unsigned long
get_board_sys_clk(ulong dummy)
{
	return 66666666;
}

void cust_do_reset (void)
{
	set_gsm73xx_cb (0x01, 0x01);
}


