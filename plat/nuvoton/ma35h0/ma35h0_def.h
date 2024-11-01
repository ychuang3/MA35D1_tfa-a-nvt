/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef	MA35H0_DEF_H
#define	MA35H0_DEF_H

#include <lib/utils_def.h>

#ifndef	MA35H0_CLUSTER_COUNT
#define	MA35H0_CLUSTER_COUNT		1
#endif

#ifndef	MA35H0_MAX_CPUS_PER_CLUSTER
#define	MA35H0_MAX_CPUS_PER_CLUSTER	2
#endif

#ifndef	MA35H0_MAX_PE_PER_CPU
#define	MA35H0_MAX_PE_PER_CPU	1
#endif

#define	MA35H0_PRIMARY_CPU	0x0

/* Defines for the Interconnect	build selection	*/
#define	MA35H0_CCI		1
#define	MA35H0_CCN		2

#define	MAX_IO_DEVICES		U(2)
#define	MAX_IO_HANDLES		U(2)
#define	MAX_IO_BLOCK_DEVICES	U(1)

/*******************************************************************************
 * MA35H0 memory map related constants
 ******************************************************************************/
#define	MA35H0_SRAM1_BASE	U(0x28000000)
#define	MA35H0_SRAM1_SIZE	U(0x00040000)

/* DDR configuration */
#define	MA35H0_DDR_BASE		U(0x80000000)

#define	MA35H0_BL2_BASE		U(0x28000000)
#define	MA35H0_BL2_SIZE		U(0x00023000)

#define	MA35H0_DTB_BASE		U(0x28023000)
#define	MA35H0_DTB_SIZE		U(0x00002000)

#define	MA35H0_BL31_BASE	U(0x28025000)
#define	MA35H0_BL31_SIZE	U(0x0001A000)	/* 0x2803C000 is reserved to IBR */

#define	MA35H0_BL32_SIZE	U(0x00200000)

#define	MA35H0_BL33_BASE	U(0x85500000)
#define	MA35H0_BL33_SIZE	U(0x00200000)

#define	BL2_BASE		MA35H0_BL2_BASE
#define	BL2_LIMIT		(MA35H0_BL2_BASE + MA35H0_BL2_SIZE)

#define	DTB_BASE		MA35H0_DTB_BASE
#define	DTB_LIMIT		(MA35H0_DTB_BASE + MA35H0_DTB_SIZE)

#define	SCP_BL2_BASE			U(0x85000000)	/* temporary staging */
#define	SCP_BL2_SIZE			U(0x00080000)	/* 512 KiB */
#define	SCPBL2_BASE			MA35H0_SCPBL2_BASE

#define	MA35H0_FIP_SIZE		0x00200000
#define	MA35H0_FIP_BASE		U(0x86000000)	//(MA35H0_DDR_BASE)
#define	MA35H0_FIP_LIMIT	(MA35H0_FIP_BASE + MA35H0_FIP_SIZE)

/*
 * Put BL31 at the bottom of TZC secured DRAM
 */
#define	BL31_BASE		MA35H0_BL31_BASE
#define	BL31_LIMIT		(MA35H0_BL31_BASE + MA35H0_BL31_SIZE)

#define	BL32_BASE		MA35H0_BL32_BASE
#define	BL32_LIMIT		(MA35H0_BL32_BASE + MA35H0_BL32_SIZE)

#define	BL33_BASE		MA35H0_BL33_BASE
#define	BL33_LIMIT		(MA35H0_BL33_BASE + MA35H0_BL33_SIZE)

/*******************************************************************************
 * ma35d1 device/io map related	constants (used	for MMU)
 ******************************************************************************/
#define	MA35H0_REG_BASE		U(0x40000000)
#define	MA35H0_REG_SIZE		U(0x20000000)

#define	MA35H0_DRAM_BASE	U(0x80000000)
#define	MA35H0_DRAM_S_SIZE	U(0x00800000)

/* MA35H0 Power	controller base	address*/
#define	PWRC_BASE		UL(0x1c100000)

/*******************************************************************************
 * GIC-400 & interrupt handling	related	constants
 ******************************************************************************/
/* Base	MA35H0 compatible GIC memory map */
#define	BASE_GICD_BASE			UL(0x50801000)
#define	BASE_GICR_BASE			0	/* no GICR in GIC-400 */
#define	BASE_GICC_BASE			UL(0x50802000)
#define	BASE_GICH_BASE			UL(0x50804000)
#define	BASE_GICV_BASE			UL(0x50806000)

#define	MA35H0_IRQ_TZ_WDOG		39	/* wdt0	*/
#define	MA35H0_IRQ_SEC_SYS_TIMER	79	/* tmr0? */

/*******************************************************************************
 * MA35H0 TZC (TZ400)
 ******************************************************************************/
#define	MA35H0_TZC0_BASE		U(0x404E1000)
#define	MA35H0_TZC1_BASE		U(0x404E2000)
#define	MA35H0_TZC2_BASE		U(0x404E3000)

/* NSAIDs used by devices in TZC filter	0 on MA35H0	*/
#define	MA35H0_NSAID_TZNS	U(0)

#define	MA35H0_FILTER_BIT_ALL	U(7)

/*******************************************************************************
 * Memprotect definitions
 ******************************************************************************/
/* PSCI	memory protect definitions:
 * This	variable is stored in a non-secure flash because some ARM reference
 * platforms do	not have secure NVRAM. Real systems that provided MEM_PROTECT
 * support must	use a secure NVRAM to store the PSCI MEM_PROTECT definitions.
 */
/*#define PLAT_ARM_MEM_PROT_ADDR	(V2M_FLASH0_BASE + V2M_FLASH0_SIZE \
 *					 - V2M_FLASH_BLOCK_SIZE)
 */

/*************************************/
#define	UMCTL2_BA		U(0x404d0000)
#define	DDRPHY_BA		U(0x404c0000)
#define	SYS_BA			U(0x40460000)
#define	CLK_BA			U(0x40460200)

/* sys registers */
#define	SYS_PWRONOTP	U(0x40460004)
#define	SYS_PWRONPIN	U(0x40460008)
#define	SYS_IPRST0		U(0x40460020)
#define	SYS_IPRST1		U(0x40460024)
#define	SYS_CHIPCFG		U(0x404601F4)

#define	SYS_GPA_MFPL	U(0x40460080)
#define	SYS_GPA_MFPH	U(0x40460084)
#define	SYS_GPC_MFPL	U(0x40460090)
#define	SYS_GPC_MFPH	U(0x40460094)
#define	SYS_GPD_MFPL	U(0x40460098)
#define	SYS_GPE_MFPH	U(0x404600A4)
#define	SYS_GPJ_MFPL	U(0x404600C8)
#define	SYS_GPJ_MFPH	U(0x404600CC)
#define	SYS_GPK_MFPL	U(0x404600D0)
#define	SYS_GPN_MFPH	U(0x404600EC)

#define	SYS_RLKTZS		U(0x404601A0)
#define	SYS_RLKTZNS		U(0x404601A4)


/* clock registers */
#define	CLK_PWRCTL		U(0x40460200)
#define	CLK_SYSCLK0		U(0x40460204)
#define	CLK_SYSCLK1		U(0x40460208)
#define	CLK_APBCLK0		U(0x4046020C)
#define	CLK_APBCLK1		U(0x40460210)
#define	CLK_APBCLK2		U(0x40460214)
#define	CLK_CLKSEL0		U(0x40460218)
#define	CLK_CLKSEL1		U(0x4046021C)
#define	CLK_CLKSEL2		U(0x40460220)
#define	CLK_CLKSEL4		U(0x40460228)
#define	CLK_CLKDIV1		U(0x40460230)
#define	CLK_CLKOCTL		U(0x40460240)
#define	CLK_STATUS		U(0x40460250)

#define	CLK_PLL0CTL0	U(0x40460260)
#define CLK_PLL2CTL0	U(0x40460280)
#define CLK_PLL2CTL1	U(0x40460284)
#define CLK_PLL2CTL2	U(0x40460288)
#define	CLK_PLL3CTL0	U(0x40460290)
#define	CLK_PLL3CTL1	U(0x40460294)
#define	CLK_PLL3CTL2	U(0x40460298)
#define	CLK_PLL4CTL0	U(0x404602A0)
#define	CLK_PLL4CTL1	U(0x404602A4)
#define	CLK_PLL4CTL2	U(0x404602A8)
#define	CLK_PLL5CTL0	U(0x404602B0)
#define	CLK_PLL5CTL1	U(0x404602B4)
#define	CLK_PLL5CTL2	U(0x404602B8)

/* sspcc registers */
#define	SSPCC_BASE		U(0x404F0000)

/* crypto define */
#define	CRYPTO_BASE		U(0x40300000)
#define	KS_BASE			U(0x40340000)
#define TSI_SYS_BASE	U(0x40360000)
#define TSI_CLK_BASE	U(0x40360200)
#define TSI_CLK_AHBCLK	(TSI_CLK_BASE + 0x4)
#define TSI_CLK_APBCLK1	(TSI_CLK_BASE + 0xc)
#define TSI_CLK_CLKSEL0	(TSI_CLK_BASE + 0x10)
#define TSI_CLK_PCLKDIV	(TSI_CLK_BASE + 0x34)
#define TSI_CLK_PLLCTL	(TSI_CLK_BASE + 0x40)
#define TSI_CLK_STATUS	(TSI_CLK_BASE + 0x50)
#define TSI_PARAM_BASE	U(0x2803EE00) /* 0xA00 */

#endif /* MA35H0_DEF_H */
