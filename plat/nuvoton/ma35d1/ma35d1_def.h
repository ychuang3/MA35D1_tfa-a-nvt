/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MA35D1_DEF_H
#define MA35D1_DEF_H

#include <lib/utils_def.h>

#ifndef MA35D1_CLUSTER_COUNT
#define MA35D1_CLUSTER_COUNT		1
#endif

#ifndef MA35D1_MAX_CPUS_PER_CLUSTER
#define MA35D1_MAX_CPUS_PER_CLUSTER	2
#endif

#ifndef MA35D1_MAX_PE_PER_CPU
#define MA35D1_MAX_PE_PER_CPU		1
#endif

#define MA35D1_PRIMARY_CPU		0x0

/* Defines for the Interconnect build selection */
#define MA35D1_CCI			1
#define MA35D1_CCN			2

#define MAX_IO_DEVICES			U(2)
#define MAX_IO_HANDLES			U(2)
#define MAX_IO_BLOCK_DEVICES		U(1)
//#define MAX_IO_MTD_DEVICES		U(1)

/* Define maximum page size for NAND devices */
//#define PLATFORM_MTD_MAX_PAGE_SIZE	U(0x2000)

/*******************************************************************************
 * MA35D1 memory map related constants
 ******************************************************************************/
//#define MA35D1_ROM_BASE              U(0x5FFC0000)
//#define MA35D1_ROM_SIZE              U(0x00020000)

#define MA35D1_SRAM0_BASE		U(0x24000000)
#define MA35D1_SRAM0_SIZE		U(0x00040000)

#define MA35D1_SRAM1_BASE		U(0x28000000)
#define MA35D1_SRAM1_SIZE		U(0x00040000)

/* DDR configuration */
#define MA35D1_DDR_BASE			U(0x80000000)

#define MA35D1_BL2_BASE			U(0x28000000)
#define MA35D1_BL2_SIZE			U(0x00023000)

#define MA35D1_DTB_BASE			U(0x28023000)
#define MA35D1_DTB_SIZE			U(0x00002000)

#define MA35D1_BL31_BASE		U(0x28025000)
#define MA35D1_BL31_SIZE		U(0x00015000)	/* 0x2803C000 is reserved to IBR */

#define MA35D1_BL32_SIZE		U(0x00200000)

#define MA35D1_BL33_BASE		U(0x85500000)
#define MA35D1_BL33_SIZE		U(0x00200000)

#define BL2_BASE			MA35D1_BL2_BASE
#define BL2_LIMIT			(MA35D1_BL2_BASE + MA35D1_BL2_SIZE)

#define DTB_BASE			MA35D1_DTB_BASE
#define DTB_LIMIT			(MA35D1_DTB_BASE + MA35D1_DTB_SIZE)

#define MA35D1_FIP_SIZE			0x00200000
#define MA35D1_FIP_BASE			U(0x86000000)	//(MA35D1_DDR_BASE)
#define MA35D1_FIP_LIMIT		(MA35D1_FIP_BASE + MA35D1_FIP_SIZE)

/*
 * Put BL31 at the bottom of TZC secured DRAM
 */
#define BL31_BASE			MA35D1_BL31_BASE
#define BL31_LIMIT			(MA35D1_BL31_BASE + MA35D1_BL31_SIZE)

#define BL32_BASE			MA35D1_BL32_BASE
#define BL32_LIMIT			(MA35D1_BL32_BASE + MA35D1_BL32_SIZE)

#define BL33_BASE			MA35D1_BL33_BASE
#define BL33_LIMIT			(MA35D1_BL33_BASE + MA35D1_BL33_SIZE)

/*******************************************************************************
 * ma35d1 device/io map related constants (used for MMU)
 ******************************************************************************/
#define MA35D1_REG_BASE			U(0x40000000)
#define MA35D1_REG_SIZE			U(0x20000000)

#define MA35D1_DRAM_BASE		U(0x80000000)

#define MA35D1_DRAM_S_SIZE		U(0x00800000)

/* Non-volatile counters */
//#define TRUSTED_NVCTR_BASE		UL(0x7fe70000)
//#define TFW_NVCTR_BASE		(TRUSTED_NVCTR_BASE + UL(0x0000))
//#define TFW_NVCTR_SIZE		UL(4)
//#define NTFW_CTR_BASE			(TRUSTED_NVCTR_BASE + UL(0x0004))
//#define NTFW_CTR_SIZE			UL(4)

/* Keys */
//#define SOC_KEYS_BASE			UL(0x7fe80000)
//#define TZ_PUB_KEY_HASH_BASE		(SOC_KEYS_BASE + UL(0x0000))
//#define TZ_PUB_KEY_HASH_SIZE		UL(32)
//#define HU_KEY_BASE			(SOC_KEYS_BASE + UL(0x0020))
//#define HU_KEY_SIZE			UL(16)
//#define END_KEY_BASE			(SOC_KEYS_BASE + UL(0x0044))
//#define END_KEY_SIZE			UL(32)

/* Constants to distinguish MA35D1 type */
//#define HBI_BASE_MA35D1		U(0x020)
//#define REV_BASE_MA35D1_V0		U(0x0)
//#define REV_BASE_MA35D1_REVC		U(0x2)

//#define HBI_FOUNDATION_MA35D1		U(0x010)
//#define REV_FOUNDATION_MA35D1_V2_0	U(0x0)
//#define REV_FOUNDATION_MA35D1_V2_1	U(0x1)
//#define REV_FOUNDATION_MA35D1_v9_1	U(0x2)
//#define REV_FOUNDATION_MA35D1_v9_6	U(0x3)

//#define ARCH_MODEL			U(0x1)

/* MA35D1 Power controller base address*/
#define PWRC_BASE			UL(0x1c100000)

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
/* Base MA35D1 compatible GIC memory map */
#define BASE_GICD_BASE			UL(0x50801000)
#define BASE_GICR_BASE			0	/* no GICR in GIC-400 */
#define BASE_GICC_BASE			UL(0x50802000)
#define BASE_GICH_BASE			UL(0x50804000)
#define BASE_GICV_BASE			UL(0x50806000)

#define MA35D1_IRQ_TZ_WDOG		39	/* wdt0 */
#define MA35D1_IRQ_SEC_SYS_TIMER	79	/* tmr0? */

/*******************************************************************************
 * MA35D1 TZC (TZ400)
 ******************************************************************************/
#define MA35D1_TZC0_BASE		U(0x404E1000)
#define MA35D1_TZC1_BASE		U(0x404E2000)
#define MA35D1_TZC2_BASE		U(0x404E3000)

/* NSAIDs used by devices in TZC filter 0 on MA35D1 */
#define MA35D1_NSAID_TZNS		U(0)
#define MA35D1_NSAID_SUBM		U(1)

#define MA35D1_FILTER_BIT_ALL		U(7)

/*******************************************************************************
 * Memprotect definitions
 ******************************************************************************/
/* PSCI memory protect definitions:
 * This variable is stored in a non-secure flash because some ARM reference
 * platforms do not have secure NVRAM. Real systems that provided MEM_PROTECT
 * support must use a secure NVRAM to store the PSCI MEM_PROTECT definitions.
 */
/*#define PLAT_ARM_MEM_PROT_ADDR	(V2M_FLASH0_BASE + V2M_FLASH0_SIZE \
 *					 - V2M_FLASH_BLOCK_SIZE)
 */

/*************************************/
#define UMCTL2_BA       U(0x404d0000)
#define DDRPHY_BA       U(0x404c0000)
#define SYS_BA          U(0x40460000)
#define CLK_BA          U(0x40460200)

/* sys registers */
#define SYS_PWRONOTP	U(0x40460004)
#define SYS_PWRONPIN	U(0x40460008)
#define SYS_IPRST0	U(0x40460020)
#define SYS_IPRST1	U(0x40460024)
#define SYS_CHIPCFG	U(0x404601F4)

#define SYS_GPA_MFPL	U(0x40460080)
#define SYS_GPA_MFPH	U(0x40460084)
#define SYS_GPC_MFPL	U(0x40460090)
#define SYS_GPC_MFPH	U(0x40460094)
#define SYS_GPD_MFPL	U(0x40460098)
#define SYS_GPE_MFPH	U(0x404600A4)
#define SYS_GPJ_MFPL	U(0x404600C8)
#define SYS_GPJ_MFPH	U(0x404600CC)

#define SYS_RLKTZS	U(0x404601A0)
#define SYS_RLKTZNS	U(0x404601A4)

/* clock registers */
#define CLK_PWRCTL	U(0x40460200)
#define CLK_SYSCLK0	U(0x40460204)
#define CLK_SYSCLK1	U(0x40460208)
#define CLK_APBCLK0	U(0x4046020C)
#define CLK_APBCLK1	U(0x40460210)
#define CLK_APBCLK2	U(0x40460214)
#define CLK_CLKSEL0	U(0x40460218)
#define CLK_CLKSEL1	U(0x4046021C)
#define CLK_CLKSEL2	U(0x40460220)
#define CLK_CLKDIV1	U(0x40460230)
#define CLK_STATUS	U(0x40460250)

#define CLK_PLL0CTL0	U(0x40460260)
#define CLK_PLL0CTL1	U(0x40460264)
#define CLK_PLL0CTL2	U(0x40460268)
#define CLK_PLL1CTL0	U(0x40460270)
#define CLK_PLL1CTL1	U(0x40460274)
#define CLK_PLL1CTL2	U(0x40460278)
#define CLK_PLL3CTL0	U(0x40460290)
#define CLK_PLL3CTL1	U(0x40460294)
#define CLK_PLL3CTL2	U(0x40460298)
#define CLK_PLL4CTL0	U(0x404602A0)
#define CLK_PLL4CTL1	U(0x404602A4)
#define CLK_PLL4CTL2	U(0x404602A8)

/* sspcc registers */
#define SSPCC_BASE	U(0x404F0000)

/* crypto define */
#define CRYPTO_BASE	U(0x40300000)
#define TSI_SYS_BASE	U(0x40360000)
#define TSI_CLK_BASE	U(0x40360200)
#define KS_BASE		U(0x40340000)
#define WHC0_BASE	U(0x403A0000)
#define WHC1_BASE	U(0x503B0000)

#define outp32(addr, data) (*(volatile uint32_t *)((uint64_t)(addr))) = (data)
#define inp32(addr) (*(volatile uint32_t *)((uint64_t)(addr)))
#define inp8(addr) (*(volatile uint8_t *)((uint64_t)(addr)))

#endif /* MA35D1_DEF_H */
