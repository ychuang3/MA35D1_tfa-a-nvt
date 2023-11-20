/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <drivers/nuvoton/ma35h0_pmic.h>
#include "ma35h0_private.h"

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

/* CPU-PLL */
static unsigned int CAPLL_MODE[2] = {
	0x000006a2,	/* 648 MHz */
	0x0000137D,	/* 500 MHz */
};

/*
 * This function changes the CPU PLL
 */
int32_t ma35h0_change_pll(int pll)
{
	uint64_t timeout;
	uint32_t index = 1; /* 500 MHz */

	/* CA-PLL */
	switch (pll) {
	case CPU_PLL_650:
		index = 0;
		INFO("CA-PLL is 650 MHz\n");
		break;
	case CPU_PLL_500:
		index = 1;
		INFO("CA-PLL is 500 MHz\n");
		break;
	default:
		WARN("Invaild CA-PLL !!\n");
		return 1;
	};
	/* set CA35 to E-PLL */
	mmio_write_32(CLK_CLKSEL0, (mmio_read_32(CLK_CLKSEL0) & ~0x3) | 0x2);

	mmio_write_32(CLK_PLL0CTL0, CAPLL_MODE[index]);

	timeout = timeout_init_us(12000);	/* 1ms */
	/* check PLL stable */
	while (1) {
		if ((mmio_read_32(CLK_STATUS) & 0x40) == 0x40)
			break;

		if (timeout_elapsed(timeout)) {
			ERROR("PLL wait stable timeout!\n");
			return 1;
		}
	}
	/* set CA35 to CA-PLL */
	mmio_write_32(CLK_CLKSEL0, (mmio_read_32(CLK_CLKSEL0) & ~0x3) | 0x1);

	return 0;
}
