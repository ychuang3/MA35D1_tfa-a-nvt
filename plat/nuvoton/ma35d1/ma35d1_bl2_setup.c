/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/arm/sp804_delay_timer.h>
#include <common/desc_image_load.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <common/debug.h>
#include <lib/optee_utils.h>

#include "ma35d1_private.h"

/*
 * bl2_platform_setup()
 * MMU on - enabled by bl2_plat_arch_setup()
 */
void bl2_platform_setup(void)
{
}


void bl2_plat_arch_setup(void)
{
	/* Setup the MMU here */
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	/* Prevent corruption of preloaded Device Tree */
	mmap_add_region(DTB_BASE, DTB_BASE,
			DTB_LIMIT - DTB_BASE,
			MT_RO_DATA | MT_SECURE);

	/* config MMC */
	configure_mmu();

	generic_delay_timer_init();

	/* TODO: system config initial, power? */

	ma35d1_arch_security_setup();

	ma35d1_io_setup();
}

static uint32_t ma35d1_get_spsr_for_bl32_entry(void)
{
	return 0;
}

static uint32_t ma35d1_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	bl_mem_params_node_t *pager_mem_params;
	bl_mem_params_node_t *paged_mem_params = NULL;

	assert(bl_mem_params != NULL);

	switch (image_id) {
	case BL32_IMAGE_ID:
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
				&pager_mem_params->image_info,
				&paged_mem_params->image_info);
		if (err != 0) {
			WARN("OPTEE header parse error.\n");
		}

		/*
		 * Keep the architecture and pageable image arguments from the
		 * OP-TEE header for the upstream OPTEED handoff protocol.
		 */
		bl_mem_params->ep_info.args.arg3 = 0;

		bl_mem_params->ep_info.spsr = ma35d1_get_spsr_for_bl32_entry();

		break;

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = ma35d1_get_spsr_for_bl33_entry();
		break;

	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}
