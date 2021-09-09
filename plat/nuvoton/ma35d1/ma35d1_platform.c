/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include "ma35d1_private.h"

/*
 * This function returns soc revision in below format
 *
 *   soc_revision[8:15] = major version number
 *   soc_revision[0:7]  = minor version number
 */
int32_t plat_get_soc_revision(void)
{
	return (int32_t)(mmio_read_32(SYS_BA));
}
