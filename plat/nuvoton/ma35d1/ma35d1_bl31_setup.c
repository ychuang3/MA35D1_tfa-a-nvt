/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/arm/gicv2.h>

#include <lib/debugfs.h>

#include "ma35d1_private.h"

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;


void bl31_early_platform_setup2(u_register_t arg0,
		u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	/* Initialize the platform config for future decision making */
	ma35d1_config_setup();

	/*************************************************************/
	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params != NULL) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0U)
		panic();

	/*************************************************************/

	/*
	 * Initialize the correct interconnect for this cluster during cold
	 * boot. No need for locks as no other CPU is active.
	 */
	ma35d1_interconnect_init();

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * MA35D1 PSCI code will enable coherency for other clusters.
	 */
	ma35d1_interconnect_enable();

	/* Initialize System level generic or SP804 timer */
	ma35d1_timer_init();

}

void bl31_plat_arch_setup(void)
{
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	/* config MMC */
	configure_mmu();
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}


/*******************************************************************************
 * GICv2 driver setup information
 ******************************************************************************/
static const interrupt_prop_t nvt_interrupt_props[] = {
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

gicv2_driver_data_t arm_gic_data = {
	.gicd_base = BASE_GICD_BASE,
	.gicc_base = BASE_GICC_BASE,
	.interrupt_props = nvt_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(nvt_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};


/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	gicv2_driver_init(&arm_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	plat_ma35d1_init();
}

void bl31_plat_runtime_setup(void)
{
}
