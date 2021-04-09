/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/arm/common/plat_arm.h>

#include "ma35d1_private.h"

void bl2_early_platform_setup2(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
	/* Initialize the platform config for future decision making */
	ma35d1_config_setup();

	ma35d1_ddr_init();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	ma35d1_interconnect_init();
	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	ma35d1_interconnect_enable();

}
