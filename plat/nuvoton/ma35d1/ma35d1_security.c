/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <common/debug.h>
#include <lib/mmio.h>

/*
 * We assume that all security programming is done by the primary core.
 */
void plat_arm_security_setup(void)
{
}

/* Configure the carveout while preserving the early region 0 policy. */
static void init_tzc400(void)
{
#ifdef MA35D1_LOAD_BL32
	unsigned long long ddr_s_size =
		MA35D1_DDR_MAX_SIZE - MA35D1_DRAM_SIZE - 0x100000;
	unsigned int reg = mmio_read_32(0x40460204);

	/* Enable SSMCC clocks while configuring the filters. */
	mmio_write_32(0x40460204, mmio_read_32(0x40460204) | 0x7f7f0000);

	/* Secure region 1 leaves the final 1 MiB available for shared memory. */
	tzc400_init(MA35D1_TZC2_BASE);
	tzc400_disable_filters();
	tzc400_configure_region(MA35D1_TZC2_FILTER, 1,
				MA35D1_DRAM_S_BASE,
				MA35D1_DRAM_S_BASE + (ddr_s_size - 1U),
				TZC_REGION_S_RDWR, 0);
	tzc400_set_action(TZC_ACTION_ERR);
	tzc400_enable_filters();

	tzc400_init(MA35D1_TZC0_BASE);
	tzc400_disable_filters();
	tzc400_configure_region(MA35D1_TZC0_FILTER, 1,
				MA35D1_DRAM_S_BASE,
				MA35D1_DRAM_S_BASE + (ddr_s_size - 1U),
				TZC_REGION_S_RDWR, 0);
	tzc400_set_action(TZC_ACTION_ERR);
	tzc400_enable_filters();

	mmio_write_32(0x40460204, (mmio_read_32(0x40460204) &
			~0x7f7f0000) | reg);

	/* Enable Cortex-M4 DDR access. */
	mmio_write_32(0x40460070, mmio_read_32(0x40460070) | 0x4);
#endif
}


/*******************************************************************************
 * Initialize the TrustZone Controller.
 * Early initialization create only one region with full access to secure.
 * This setting is used before and during DDR initialization.
 ******************************************************************************/
static void early_init_tzc400(void)
{
	unsigned int reg = mmio_read_32(0x40460204);

	/* SSMCC clock enable */
	mmio_write_32(0x40460214, mmio_read_32(0x40460214) | (1 << 2));
	mmio_write_32(0x40460218, mmio_read_32(0x40460218) | (1 << 2));
	mmio_write_32(0x40460204, mmio_read_32(0x40460204) | 0x7f7f0000);

	/* TZC2 */
	tzc400_init(MA35D1_TZC2_BASE);
	tzc400_configure_region0(TZC_REGION_S_RDWR,
				PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	/* TZC0 */
	tzc400_init(MA35D1_TZC0_BASE);
	tzc400_configure_region0(TZC_REGION_S_RDWR,
				PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	mmio_write_32(0x40460204, (mmio_read_32(0x40460204) &
			~0x7f7f0000) | reg);

	/* Enable Cortex-M4 DDR access. */
	mmio_write_32(0x40460070, mmio_read_32(0x40460070) | 0x4);
}


/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void ma35d1_arch_security_setup(void)
{
	early_init_tzc400();
}

/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void ma35d1_security_setup(void)
{
	init_tzc400();
}
