/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MA35D0_PRIVATE_H
#define MA35D0_PRIVATE_H

#include <plat/arm/common/plat_arm.h>

/*****************************************************************************
 * Power On Setting
 *****************************************************************************/
/* PWRON[11:10] */
#define PWRON_BOOT_MSK      (0x00000C00)
#define PWRON_BOOT_USB      (0x00000C00)
#define PWRON_BOOT_SD       (0x00000400)
#define PWRON_BOOT_NAND     (0x00000800)
#define PWRON_BOOT_SPI      (0x00000000)

/* SD boot for MFP */
#define PWRON_SD0           (0x00000000)
#define PWRON_SD1           (0x00004000)

/* SPI boot for mode select */
#define PWRON_SPI_4_NAND    (0x00004000)
#define PWRON_SPI_1_NAND    (0x00000000)
#define PWRON_SPI_4_NOR     (0x0000C000)
#define PWRON_SPI_1_NOR     (0x00008000)
#define PWRON_SPI_NOR       (0x00008000)

/* PLL Setting */
#define CPU_PLL_650         (0x00000650)
#define CPU_PLL_600         (0x00000600)
#define CPU_PLL_500         (0x00000500)
#define CPU_PLL_250         (0x00000250)
#define CPU_PLL_125         (0x00000125)

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/

void ma35d0_config_setup(void);
void plat_ma35d0_init(void);

void ma35d0_interconnect_init(void);
void ma35d0_interconnect_enable(void);
void ma35d0_interconnect_disable(void);
void ma35d0_timer_init(void);
void tsp_early_platform_setup(void);
void ma35d0_io_setup(void);

void configure_mmu(void);
void ma35d0_ddr_init(void);
void ma35d0_arch_security_setup(void);
int32_t ma35d0_change_pll(int pll);

#endif /* MA35D0_PRIVATE_H */
