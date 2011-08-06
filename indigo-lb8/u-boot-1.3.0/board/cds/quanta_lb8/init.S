/*
 * Copyright 2004, 2007 Freescale Semiconductor.
 * Copyright 2002,2003, Motorola Inc.
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

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include <config.h>
#include <mpc85xx.h>

#define LAWAR_TRGT_PCI1		0x00000000
#define LAWAR_TRGT_PCI2		0x00100000
#define LAWAR_TRGT_PCIE		0x00200000
#define LAWAR_TRGT_RIO		0x00c00000
#define LAWAR_TRGT_LBC		0x00400000
#define LAWAR_TRGT_DDR		0x00f00000

/*
 * TLB0 and TLB1 Entries
 *
 * Out of reset, TLB1's Entry 0 maps the highest 4K for CCSRBAR.
 * However, CCSRBAR is then relocated to CFG_CCSRBAR right after
 * these TLB entries are established.
 *
 * The TLB entries for DDR are dynamically setup in spd_sdram()
 * and use TLB1 Entries 8 through 15 as needed according to the
 * size of DDR memory.
 *
 * MAS0: tlbsel, esel, nv
 * MAS1: valid, iprot, tid, ts, tsize
 * MAS2: epn, sharen, x0, x1, w, i, m, g, e
 * MAS3: rpn, u0-u3, ux, sx, uw, sw, ur, sr
 */

#define	entry_start \
	mflr	r1	;	\
	bl	0f	;

#define	entry_end \
0:	mflr	r0	;	\
	mtlr	r1	;	\
	blr		;


	.section	.bootpg, "ax"
	.globl	tlb1_entry
tlb1_entry:
	entry_start

	/*
	 * Number of TLB0 and TLB1 entries in the following table
	 */
	//.long (2f-1f)/16
	.long 9

1:
#if (CFG_CCSRBAR_DEFAULT != CFG_CCSRBAR)
	/*
	 * TLB0		4K	Non-cacheable, guarded
	 * 0xff700000	4K	Initial CCSRBAR mapping
	 *
	 * This ends up at a TLB0 Index==0 entry, and must not collide
	 * with other TLB0 Entries.
	 */
	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_CCSRBAR_DEFAULT), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_CCSRBAR_DEFAULT), 0,0,0,0,0,1,0,1,0,1)
#else
#error("Update the number of table entries in tlb1_entry")
#endif

	/*
	 * TLB0		128K	Cacheable, non-guarded
	 * 0xE8000000   128K L2 Cache as SRAM, Temporary Global data for
	 *				initialization
	 *
	 * Use 32 4K TLB0 entries.  These entries must be cacheable
	 * as they provide the bootstrap memory before the memory
	 * controler and real memory have been configured.
	 *
	 * These entries end up at TLB0 Indicies 0x10, 0x14, 0x18 0x1c, etc.,
	 * and must not collide with other TLB0 entries.
	 */
#define TLB0_CACHE 0
#define TLB0_GUARD 0
	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR),
			0,0,0,0,TLB0_CACHE,0,TLB0_GUARD,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_INIT_RAM_ADDR),
			0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR + 4 * 1024),
			0,0,0,0,TLB0_CACHE,0,TLB0_GUARD,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_INIT_RAM_ADDR + 4 * 1024),
			0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR + 8 * 1024),
			0,0,0,0,TLB0_CACHE,0,TLB0_GUARD,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_INIT_RAM_ADDR + 8 * 1024),
			0,0,0,0,0,1,0,1,0,1)

	.long TLB1_MAS0(0, 0, 0)
	.long TLB1_MAS1(1, 0, 0, 0, 0)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_INIT_RAM_ADDR + 12 * 1024),
			0,0,0,0,TLB0_CACHE,0,TLB0_GUARD,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_INIT_RAM_ADDR + 12 * 1024),
			0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLB 1.0:	64M	Non-cacheable, guarded
	 * 0xfe000000	64M	FLASH
	 * Out of reset this entry is only 4K.
	 */
	.long TLB1_MAS0(1, 0, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_FLASH_BASE), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_FLASH_BASE), 0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLB 1.1:	1G	Non-cacheable, guarded
	 * 0x80000000	1G	PCI1/PCIE  8,9,a,b
	 */
	.long TLB1_MAS0(1, 1, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_1G)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_PCI_PHYS), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_PCI_PHYS), 0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLB 1.5:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	16M	PCI1 IO
	 * 0xe300_0000	16M	PCIe IO
	 */
	.long TLB1_MAS0(1, 5, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_CCSRBAR), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_CCSRBAR), 0,0,0,0,0,1,0,1,0,1)

	/*
	 * TLB 1.6:	64M	Non-cacheable, guarded
	 * 0xf000_0000	64M	LBC CompactFlash and CPLD 
	 */
	.long TLB1_MAS0(1, 6, 0)
	.long TLB1_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_64M)
	.long TLB1_MAS2(E500_TLB_EPN(CFG_CFLASH0_BASE), 0,0,0,0,1,0,1,0)
	.long TLB1_MAS3(E500_TLB_RPN(CFG_CFLASH0_BASE), 0,0,0,0,0,1,0,1,0,1)


2:
	entry_end

/*
 * LAW(Local Access Window) configuration:
 *
 * 0x0000_0000     0x7fff_ffff     DDR                     2G
 * 0x8000_0000     0x9fff_ffff     PCI1 MEM                512M
 * 0xa000_0000     0xbfff_ffff     PCIe MEM                512M
 * 0xe000_0000     0xe000_ffff     CCSR                    1M (via CCSRBAR)
 * 0xe200_0000     0xe2ff_ffff     PCI1 IO                 16M
 * 0xe300_0000     0xe3ff_ffff     PCIe IO                 16M
 * 0xe800_0000     0xe801_ffff     L2 Cache as SRAM        128K (via L2SRBAR0)
 * 0xf000_0000     0xf000_ffff     CompactFlash 0          64K
 * 0xf300_0000     0xf300_ffff     CPLD                    64KB
 * 0xfc00_0000     0xfdff_ffff     FLASH (2nd bank)        32M
 * 0xfe00_0000     0xffff_ffff     FLASH (boot bank)       32M
 *
 * Notes:
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 *
 * LAW 0 is reserved for boot mapping
 */

	.section .bootpg, "ax"
	.globl	law_entry
law_entry:
	entry_start

	.long (4f-3f)/8
3:
	.long  0
	.long  (LAWAR_TRGT_DDR | (LAWAR_SIZE & LAWAR_SIZE_128M)) & ~LAWAR_EN

#ifdef CFG_PCI1_MEM_PHYS
	.long	(CFG_PCI1_MEM_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_512M)

	.long	(CFG_PCI1_IO_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_16M)
#endif


#ifdef CFG_PCIE1_MEM_PHYS
	.long	(CFG_PCIE1_MEM_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_PCIE | (LAWAR_SIZE & LAWAR_SIZE_512M)

	.long	(CFG_PCIE1_IO_PHYS>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_PCIE | (LAWAR_SIZE & LAWAR_SIZE_16M)
#endif

	/* LBC window - maps 256M 0xf0000000 -> 0xffffffff */
	.long	(CFG_CFLASH0_BASE>>12) & 0xfffff
	.long	LAWAR_EN | LAWAR_TRGT_LBC | (LAWAR_SIZE & LAWAR_SIZE_256M)

4:
	entry_end
