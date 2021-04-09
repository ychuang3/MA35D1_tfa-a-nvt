/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <libfdt.h>
#include <platform_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "ma35d1_private.h"

#define MAP_SEC_SYSRAM	MAP_REGION_FLAT(MA35D1_SRAM1_BASE, \
					MA35D1_SRAM1_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE )

#define MAP_DEVICE1	MAP_REGION_FLAT(MA35D1_REG_BASE, \
					MA35D1_REG_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_SECURE)

#define MAP_DEVICE2	MAP_REGION_FLAT(MA35D1_DRAM_BASE, \
					MA35D1_DRAM_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_NS)

#define MAP_DEVICE3	MAP_REGION_FLAT(MA35D1_DRAM_S_BASE, \
					MA35D1_DRAM_S_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE)

const mmap_region_t ma35d1_mmap[] = {
	MAP_SEC_SYSRAM,
	MAP_DEVICE1,
	MAP_DEVICE2,
	MAP_DEVICE3,
	{0}
};


void configure_mmu(void)
{
	mmap_add(ma35d1_mmap);
	init_xlat_tables();

	enable_mmu_el3(0);
}
