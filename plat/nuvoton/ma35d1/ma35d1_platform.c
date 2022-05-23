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

/* CPU-PLL: 1000 MHz, 800 MHz, 700 MHz and 500 MHz. */
static unsigned int CAPLL_MODE[4] = {
	0x000006FA,	/* 1000 MHz */
	0x00000364,	/* 800 MHz */
	0x000006AF,	/* 700 MHz */
	0x0000137D,	/* 500 MHz */
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
	case CPU_PLL_1G:	/* 1.302V */
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, VOL_1_30)) {
			index = 0;
			INFO("CA-PLL is 1000 MHz\n");
		} else {
			index = 2;
			WARN("Set 1GHz fail, try to set 700 MHz.\n");
		}
		break;
	case CPU_PLL_800: /* 1.248V */
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, VOL_1_25)) {
			index = 1;
			INFO("CA-PLL is 800 MHz\n");
		} else {
			index = 2;
			WARN("Set 800MHz fail, try to set 700 MHz.\n");
		}
		break;
	case CPU_PLL_700:
		index = 2;
		INFO("CA-PLL is 700 MHz\n");
		break;
	case CPU_PLL_500:
		index = 3;
		INFO("CA-PLL is 500 MHz\n");
		break;
	default:
		WARN("Invalid CA-PLL !!\n");
		return 1;
	};
	/* set CA35 to E-PLL */
	outp32((void *)CLK_CLKSEL0, (inp32((void *)CLK_CLKSEL0) & ~0x3) | 0x2);

	outp32((void *)CLK_PLL0CTL0, CAPLL_MODE[index]);

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
