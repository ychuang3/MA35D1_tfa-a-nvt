/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <drivers/nuvoton/ma35d1_pmic.h>
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

/* CPU-PLL: 1000 MHz, 700 MHz and 500 MHz. */
static unsigned int CAPLL_MODE[3][3] = {
	{ 0x0000307D, 0x00000010, 0x00000000 },	/* 1000 MHz */
	{ 0x000060AF, 0x00000010, 0x00000000 },	/* 700 MHz */
	{ 0x0000307D, 0x00000020, 0x00000000 },	/* 500 MHz */
};

/*
 * This function changes the CPU PLL
 */
int32_t ma35d1_change_pll(int pll)
{
	uint64_t timeout;
	uint32_t index = 2; /* 500 MHz */

	/* CA-PLL */
	switch (pll) {
	case CPU_PLL_1G:
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, VOL_1_29)) {
			index = 0;
			INFO("CA-PLL is 1000 MHz\n");
		} else {
			index = 1;
			WARN("Set 1GHz fail, try to set 700 MHz.\n");
		}
		break;
	case CPU_PLL_700:
		index = 1;
		INFO("CA-PLL is 700 MHz\n");
		break;
	case CPU_PLL_500:
		index = 2;
		INFO("CA-PLL is 500 MHz\n");
		break;
	default:
		WARN("Invalid CA-PLL !!\n");
		return 1;
	};
	/* set CA35 to E-PLL */
	outp32((void *)CLK_CLKSEL0, (inp32((void *)CLK_CLKSEL0) & ~0x3) | 0x2);

	outp32((void *)CLK_PLL0CTL0, CAPLL_MODE[index][0]);
	outp32((void *)CLK_PLL0CTL1, CAPLL_MODE[index][1]);
	outp32((void *)CLK_PLL0CTL2, CAPLL_MODE[index][2]);

	timeout = timeout_init_us(12000);	/* 12 ms */
	/* check PLL stable */
	while (1) {
		if ((inp32((void *)CLK_STATUS) & 0x40) == 0x40)
			break;

		if (timeout_elapsed(timeout)) {
			ERROR("PLL wait stable timeout!\n");
			return 1;
		}
	}
	/* set CA35 to CA-PLL */
	outp32((void *)CLK_CLKSEL0, (inp32((void *)CLK_CLKSEL0) & ~0x3) | 0x1);

	return 0;
}
