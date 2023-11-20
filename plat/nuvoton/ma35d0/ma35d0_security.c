/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
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

/*******************************************************************************
 * Initialize the TrustZone Controller. Configure Region 0 with Secure RW access
 * and allow Non-Secure masters full access.
 ******************************************************************************/
static void init_tzc400(void)
{
#ifdef MA35D0_LOAD_BL32
	unsigned long long ddr_s_size = MA35D0_DDR_MAX_SIZE - MA35D0_DRAM_SIZE;
#endif
	unsigned int reg = mmio_read_32(0x40460204);

	/* SSMCC clock enable */
	mmio_write_32(0x40460204, mmio_read_32(0x40460204) | 0x7f7f0000);

	/* TZC2 */
	tzc400_init(MA35D0_TZC2_BASE);
	tzc400_disable_filters();
	/* Region 1 set to cover Non-Secure DRAM at 0x8000_0000 */
	tzc400_configure_region(MA35D0_FILTER_BIT_ALL, 1,
				MA35D0_DDR_BASE,
				MA35D0_DDR_BASE +
				(MA35D0_DDR_MAX_SIZE - 1U),
				TZC_REGION_S_NONE,
				PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);
	tzc400_enable_filters();

	/* TZC0 */
	tzc400_init(MA35D0_TZC0_BASE);
	tzc400_disable_filters();

	/* Region 1 set to cover Non-Secure DRAM at 0x8000_0000 */
	tzc400_configure_region(MA35D0_FILTER_BIT_ALL, 1,
				MA35D0_DDR_BASE,
				MA35D0_DDR_BASE +
				(MA35D0_DDR_MAX_SIZE - 1U),
				TZC_REGION_S_NONE,
				PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);
	tzc400_enable_filters();

#ifdef MA35D0_LOAD_BL32
	/* TZC2 */
	tzc400_init(MA35D0_TZC2_BASE);
	tzc400_disable_filters();
	/* Region 2 set to cover Secure DRAM at 0x8f80_0000 */
	tzc400_configure_region(MA35D0_FILTER_BIT_ALL, 2,
				MA35D0_DRAM_S_BASE,
				MA35D0_DRAM_S_BASE +
				(ddr_s_size - 1U),
				TZC_REGION_S_RDWR,
				0);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);
	tzc400_enable_filters();

	/* TZC0 */
	tzc400_init(MA35D0_TZC0_BASE);
	tzc400_disable_filters();

	/* Region 2 set to cover Secure DRAM at 0x8f80_0000 */
	tzc400_configure_region(MA35D0_FILTER_BIT_ALL, 2,
				MA35D0_DRAM_S_BASE,
				MA35D0_DRAM_S_BASE +
				(ddr_s_size - 1U),
				TZC_REGION_S_RDWR,
				0);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);
	tzc400_enable_filters();
#endif
	mmio_write_32(0x40460204, (mmio_read_32(0x40460204) & ~0x7f7f0000) | reg);
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
	tzc400_init(MA35D0_TZC2_BASE);
	tzc400_configure_region0(TZC_REGION_S_RDWR,
				PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	/* TZC0 */
	tzc400_init(MA35D0_TZC0_BASE);
	tzc400_configure_region0(TZC_REGION_S_RDWR,
				PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	mmio_write_32(0x40460204, (mmio_read_32(0x40460204) & ~0x7f7f0000) | reg);
}


/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void ma35d0_arch_security_setup(void)
{
	early_init_tzc400();
}

/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void ma35d0_security_setup(void)
{
	init_tzc400();
}
