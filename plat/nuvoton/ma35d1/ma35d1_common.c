/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/ccn.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <libfdt.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/spm_mm_partition.h>
#include <common/fdt_wrappers.h>
#include <drivers/nuvoton/ma35d1_pmic.h>

#include "ma35d1_private.h"

/* Defines for GIC Driver build time selection */
#define MA35D1_GICV2		1
#define MA35D1_GICV3		2

/*******************************************************************************
 * arm_config holds the characteristics of the differences between the
 * three MA35D1 platforms (Base, A53_A57 & Foundation). It will be
 * populated during cold boot at each boot stage by the primary before
 * enabling the MMU (to allow interconnect configuration) & used thereafter.
 * Each BL will have its own copy to allow independent operation.
 ******************************************************************************/
arm_config_t arm_config;

#if MA35D1_INTERCONNECT_DRIVER != MA35D1_CCN

static unsigned int get_interconnect_master(void)
{
	unsigned int master;
	u_register_t mpidr;

	mpidr = read_mpidr_el1();
	master = ((arm_config.flags & ARM_CONFIG_FVP_SHIFTED_AFF) != 0U) ?
		MPIDR_AFFLVL2_VAL(mpidr) : MPIDR_AFFLVL1_VAL(mpidr);

	assert(master < MA35D1_CLUSTER_COUNT);
	return master;
}
#endif

int console_ma35d1_register(uintptr_t baseaddr, uint32_t clock,
					uint32_t baud, console_t *console);
int console_ma35d1_putc(int character, console_t *console);
int console_ma35d1_getc(console_t *console);
void console_ma35d1_flush(console_t *console);
static console_t ma35d1_console;

static console_t ma35d1_console = {
	.flags = CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME |
			CONSOLE_FLAG_CRASH | CONSOLE_FLAG_TRANSLATE_CRLF,
	.putc = console_ma35d1_putc,
	.getc = console_ma35d1_getc,
	.flush = console_ma35d1_flush,
};

static int ma35d1_pll_find_closest(unsigned long rate, unsigned long parent_rate,
				   uint32_t pll_mode, uint32_t *reg_ctl,
				   uint32_t *freq);

static unsigned int CAPLL_MODE0[5] = {
	0x00000364,	/* 800 MHz */
	0x000006AF,	/* 700 MHz */
	0x000006A2,	/* 650 MHz */
	0x00001396,	/* 600 MHz */
	0x0000137D,	/* 500 MHz */
};

static void *fdt = (void *)(uintptr_t)MA35D1_DTB_BASE;

static void ma35d1_clkopin_setup(void)
{
	int node;
	unsigned int ClkoFreqDiv, ClkoSrc;

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d1-clk");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d1-clk` not found\n");
	}

	/* CLKO multi-function */
	if (fdt_read_uint32_default(fdt, node, "set-clko-pin", 1) == 1)
		mmio_write_32(SYS_GPN_MFPH, (mmio_read_32(SYS_GPN_MFPH) &
				~0xf0000000) | 0xa0000000); /* Set PN15 */
	else
		mmio_write_32(SYS_GPK_MFPL, (mmio_read_32(SYS_GPK_MFPL) &
			~0xf0000000) | 0x90000000); /* Set PK7 */

	/* set CLKO clock source */
	ClkoSrc = fdt_read_uint32_default(fdt, node, "clko-src", 1);
	mmio_write_32(CLK_CLKSEL4,
		      (mmio_read_32(CLK_CLKSEL4) & ~(0xF << 24)) |
		      (ClkoSrc << 24));

	/* Set CLKOCTL */
	if (fdt_read_uint32_default(fdt, node, "clko-div1-en", 1) == 1) {
		mmio_write_32((CLK_CLKOCTL),
			(mmio_read_32((CLK_CLKOCTL)) & ~0x00000020) |
			0x00000020);  /* Enable CLKO Div1 */
	} else {
		ClkoFreqDiv = fdt_read_uint32_default(fdt, node, "clko-freqdiv", 1);
		mmio_write_32((CLK_CLKOCTL),
			(mmio_read_32((CLK_CLKOCTL)) & ~0x0000000F) |
			ClkoFreqDiv); /* Enable FREQSEL */
	}
}

static void ma35d1_clock_setup(void)
{
	unsigned int pllmode[6] = { 0, 0, 0, 0, 0, 0 };
	unsigned int pllfreq[6] = { 0, 0, 0, 0, 0, 0 };
	unsigned int reg, clock, index;
	uint32_t reg_ctl[3];
	uint32_t freq;
	int node;

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d1-clk");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d1-clk` not found\n");
	}

	fdt_read_uint32_array(fdt, node, "clock-pll-mode", 6, pllmode);
	fdt_read_uint32_array(fdt, node, "assigned-clock-rates", 6, pllfreq);

	/* E-PLL: 500MHz */
	if ((mmio_read_32(CLK_PLL4CTL1) & 0x1) == 0x1) {
		mmio_write_32(CLK_PLL4CTL0, 0x000060FA);
		mmio_write_32(CLK_PLL4CTL1, 0x00000020);
		mmio_write_32(CLK_PLL4CTL2, 0x0);
		/* check PLL stable */
		while (1) {
			if ((mmio_read_32(CLK_STATUS) & 0x200) == 0x200)
				break;
		}
	}

	pmic_clk = pllfreq[1]; /* I2C0 clock for PMIC */

	reg = (mmio_read_32(SYS_BA) >> 16) & 0xff;
	if (reg == 0xa1 || reg == 0x81 || reg == 0x82)
		clock = 650000000;
	else
		clock = pllfreq[0];

	switch (clock) {
	case 800000000:
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, MA35D1_CPU_CORE))
			INFO("CA-PLL is 800 MHz\n");
		else
			WARN("CA-PLL is 800 MHz without PSCI setting.\n");
		index = 0;
		break;

	case 700000000:
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, MA35D1_CPU_CORE))
			INFO("CA-PLL is 700 MHz\n");
		else
			WARN("CA-PLL is 700 MHz without PSCI setting.\n");
		index = 1;
		break;

	case 650000000:
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, MA35D1_CPU_CORE))
			INFO("CA-PLL is 650 MHz\n");
		else
			WARN("CA-PLL is 650 MHz without PSCI setting.\n");
		index = 2;
		break;

	case 600000000:
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, MA35D1_CPU_CORE))
			INFO("CA-PLL is 600 MHz\n");
		else
			WARN("CA-PLL is 600 MHz without PSCI setting.\n");
		index = 3;
		break;

	case 500000000:
		index = 4;
		INFO("CA-PLL is 500 MHz\n");
		break;

	default:
		WARN("CA-PLL is %d Hz is not supported, set as 500 MHz\n",
		     clock);
		index = 4;
		break;
	};
	/* DDR-PLL */
	INFO("DDR-PLL is 266000000 Hz.\n");
	mmio_write_32(CLK_PLL2CTL0, 0x0F04102C); /* 266 MHz */
	mmio_write_32(CLK_PLL2CTL1, 0x6B851E40);
	mmio_write_32(CLK_PLL2CTL2, 0x000048A3);
	while ((mmio_read_32(CLK_STATUS) & 0x00000100) != 0x00000100)
		;

	reg = (mmio_read_32(SYS_BA) >> 16) & 0xff;
	if (reg == 0xa1 || reg == 0x81 || reg == 0x82)
		index = 2;

	/* set CA35 to E-PLL */
	mmio_write_32(CLK_CLKSEL0, (mmio_read_32(CLK_CLKSEL0) & ~0x3) | 0x2);

	mmio_write_32(CLK_PLL0CTL0, CAPLL_MODE0[index]);

	/* check PLL stable */
	while (1) {
		if ((mmio_read_32(CLK_STATUS) & 0x40) == 0x40)
			break;
	}
	/* set CA35 to CA-PLL */
	mmio_write_32(CLK_CLKSEL0, (mmio_read_32(CLK_CLKSEL0) & ~0x3) | 0x1);

	/* check LXT */
	if (fdt_read_uint32_default(fdt, node, "lxt-enable", 0) == 1) {
		mmio_write_32(CLK_PWRCTL, mmio_read_32(CLK_PWRCTL) | 0x2);
	}

	/* Enable RTC clock */
	mmio_write_32(CLK_APBCLK0, mmio_read_32(CLK_APBCLK0) | (0x1 << 29));
	if (fdt_read_uint32_default(fdt, node, "rtc-pwrctl-enable", 1) == 1) {
		uint32_t pwr_off_t;
		int ret;

		ret = fdt_read_uint32(fdt, node, "rtc-power-off-time", &pwr_off_t);
		if (ret == 0 && pwr_off_t >= 4 && pwr_off_t <= 7) {
			mmio_write_32(0x40410180,
				      (mmio_read_32(0x40410180) & ~(0x3 << 12)) |
				      ((pwr_off_t - 4) << 12) |
				      0x5aa50044);
		} else {
			mmio_write_32(0x40410180,
				      mmio_read_32(0x40410180) | 0x5aa50040);
		}
	} else {
		mmio_write_32(0x40410180,
			      (mmio_read_32(0x40410180) & ~0xffff0040) | 0x5aa50000);
	}

	/* Set PH8/PH9 */
	if (fdt_read_uint32_default(fdt, node, "set-ph8-ph9-high", 1) == 1) {
		mmio_write_32((0x40410100),
			(mmio_read_32((0x40410100)) & ~0x00000100) |
			0x00000100);  /* Enable IOCTLSEL */

		mmio_write_32((0x40410104),
			(mmio_read_32((0x40410104)) & ~0x00000707) |
			0x00000505); /* Set PH8/PH9 output high */
	} else {
		mmio_write_32((0x40410100),
			(mmio_read_32((0x40410100)) & ~0x00000100));  /* Disable IOCTLSEL */
	}

	/* Set CLKO pin function */
	if (fdt_read_uint32_default(fdt, node, "clko-en", 1) == 1) {
		/* set CKO enable */
		mmio_write_32(CLK_SYSCLK1,
			      mmio_read_32(CLK_SYSCLK1) | (0x1 << 13));
		mmio_write_32((CLK_CLKOCTL),
			(mmio_read_32((CLK_CLKOCTL)) & ~0x00000010) |
			0x00000010);  /* Enable CLKO pin Enable */
		INFO("ma35d1 CLKO enable. 0x%08x\n", mmio_read_32(CLK_CLKOCTL));
		ma35d1_clkopin_setup();
	}

	/*
	 *  Configure APLL
	 */
	if ((pllmode[3] == 0) || (pllmode[3] == 1)) {
		if (ma35d1_pll_find_closest(pllfreq[3], 24000000,
					   pllmode[3], reg_ctl, &freq) == 0) {
			mmio_write_32(CLK_PLL3CTL0, reg_ctl[0]);
			mmio_write_32(CLK_PLL3CTL1, reg_ctl[1]);
		} else {
			INFO("Failed to set APLL %d Hz!!\n", pllfreq[3]);
		}
	}

	/*
	 *  Configure VPLL
	 */
	if ((pllmode[5] == 0) || (pllmode[5] == 1)) {
		if (ma35d1_pll_find_closest(pllfreq[5], 24000000,
					   pllmode[5], reg_ctl, &freq) == 0) {
			mmio_write_32(CLK_PLL5CTL0, reg_ctl[0]);
			mmio_write_32(CLK_PLL5CTL1, reg_ctl[1]);
		} else {
			INFO("Failed to set VPLL %d Hz!!\n", pllfreq[5]);
		}
	}
}

/*******************************************************************************
 * A single boot loader stack is expected to work on both the Foundation MA35D1
 * models and the two flavours of the Base MA35D1 models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 ******************************************************************************/
void __init ma35d1_config_setup(void)
{
	unsigned int reg;

	/* unlock */
	mmio_write_32(SYS_RLKTZS, 0x59);
	mmio_write_32(SYS_RLKTZS, 0x16);
	mmio_write_32(SYS_RLKTZS, 0x88);

	/* Enable UART0 clock */
	mmio_write_32(CLK_APBCLK0, mmio_read_32(CLK_APBCLK0) | (1 << 12));
	mmio_write_32(CLK_CLKSEL2, mmio_read_32(CLK_CLKSEL2) & ~(3 << 16));
	mmio_write_32(CLK_CLKDIV1, mmio_read_32(CLK_CLKDIV1) & ~(0xf << 16));
	/* UART0 multi-function */
	mmio_write_32(SYS_GPE_MFPH, (mmio_read_32(SYS_GPE_MFPH) &
			~0xff000000) | 0x11000000);

	console_ma35d1_register(PLAT_ARM_CRASH_UART_BASE,
				PLAT_ARM_CRASH_UART_CLK_IN_HZ,
				ARM_CONSOLE_BAUDRATE,
				&ma35d1_console);

	reg = (mmio_read_32(SYS_BA) >> 16) & 0xff;
	if (reg == 0xa1 || reg == 0x81 || reg == 0x82)
		mmio_write_32(SYS_GPE_MFPH,
			      mmio_read_32(SYS_GPE_MFPH) & ~0xff000000);

	INFO("ma35d1 config setup\n");

	/* Set the PLL */
	ma35d1_clock_setup();

	/* Get the BL2 metadata word. */
	{
		unsigned int *ptr, *p;

		p = (unsigned int *)0x2803FFF0;
		ptr = (unsigned int *)(MA35D1_BL2_BASE + 0xC800);
		while ((unsigned long)ptr <
		       (MA35D1_BL2_BASE + MA35D1_BL2_SIZE)) {
			if (*ptr == 0x4E565420) {
				*p = *(ptr + 1);
				break;
			}
			ptr += 1;
		}
	}
}


void __init ma35d1_interconnect_init(void)
{
}

void ma35d1_interconnect_enable(void)
{
	unsigned int master;

	if ((arm_config.flags & (ARM_CONFIG_FVP_HAS_CCI400 |
				 ARM_CONFIG_FVP_HAS_CCI5XX)) != 0U) {
		master = get_interconnect_master();
		cci_enable_snoop_dvm_reqs(master);
	}
}

void ma35d1_interconnect_disable(void)
{
	unsigned int master;

	if ((arm_config.flags & (ARM_CONFIG_FVP_HAS_CCI400 |
				 ARM_CONFIG_FVP_HAS_CCI5XX)) != 0U) {
		master = get_interconnect_master();
		cci_disable_snoop_dvm_reqs(master);
	}
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

void ma35d1_timer_init(void)
{
	generic_delay_timer_init();
}

void plat_ma35d1_init(void)
{
	int value_len = 0, i, count = 0;
	int node;
	unsigned int cells[100 * 3];
	unsigned int reg;
	volatile int loop_delay;

	/* unlock */
	mmio_write_32(SYS_RLKTZS, 0x59);
	mmio_write_32(SYS_RLKTZS, 0x16);
	mmio_write_32(SYS_RLKTZS, 0x88);

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	/* enable CRYPTO */
	if ((mmio_read_32(SYS_CHIPCFG) & 0x100) == 0x100) {
		/* un-lock */
		do {
			mmio_write_32((TSI_SYS_BASE + 0x100), 0x59);
			mmio_write_32((TSI_SYS_BASE + 0x100), 0x16);
			mmio_write_32((TSI_SYS_BASE + 0x100), 0x88);
		} while (mmio_read_32((TSI_SYS_BASE + 0x100)) == 0UL);

		/* set HCLK from HIRC */
		mmio_write_32(TSI_CLK_CLKSEL0,
			      mmio_read_32(TSI_CLK_CLKSEL0) | 0x7);

		for (loop_delay = 0; loop_delay < 1000; loop_delay++)
			;

		/* PLL to 180 MHz */
		mmio_write_32(TSI_CLK_PLLCTL, 0x80235a);

		for (loop_delay = 0; loop_delay < 5000; loop_delay++)
			;

		/* Select HCLK from PLL */
		mmio_write_32(TSI_CLK_CLKSEL0,
			      (mmio_read_32(TSI_CLK_CLKSEL0) & ~0x7) | 0x2);

		/* Set PCLK0 to HCLK/2. */
		mmio_write_32(TSI_CLK_PCLKDIV,
			      (mmio_read_32(TSI_CLK_PCLKDIV) & ~0x7) | 0x1);

		/* initial crypto engine and ks clock */
		mmio_write_32(TSI_CLK_AHBCLK,
			      mmio_read_32(TSI_CLK_AHBCLK) | 0x5000);

		/* initial trng clock */
		mmio_write_32(TSI_CLK_APBCLK1,
			      mmio_read_32(TSI_CLK_APBCLK1) | 0x2000000);

		/* Init KeyStore */
		mmio_write_32((KS_BASE+0x00), 0x101);

		while ((mmio_read_32((KS_BASE+0x08)) & 0x80) == 0)
			;   /* wait for INITDONE(KS_STS[7]) set */

		while (mmio_read_32((KS_BASE+0x08)) & 0x4)
			;      /* wait for BUSY(KS_STS[2]) cleared */
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d1-sspcc");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d1-sspcc` not found\n");
	}
	/* Enable RTP clock */
	mmio_write_32(CLK_SYSCLK0, mmio_read_32(CLK_SYSCLK0) | 0x2);
	/* enable SSPCC/GPIO clock */
	mmio_write_32(CLK_APBCLK2, mmio_read_32(CLK_APBCLK2) | 0x8);
	mmio_write_32(CLK_SYSCLK1, mmio_read_32(CLK_SYSCLK1) | 0x3FFF0000);
	/* set GPIO to TZNS */
	for (i = 0; i < 0x38; i += 4)
		mmio_write_32(SSPCC_BASE+0x60+i, 0x55555555);

	/* get peripheral attribution from DTB */
	if (fdt_getprop(fdt, node, "config", &value_len) != 0) {
		count = value_len / 4;
		fdt_read_uint32_array(fdt, node, "config", count, cells);

		for (i = 0; i < count; i += 3) {
			reg = mmio_read_32(SSPCC_BASE+cells[i]) &
					    ~(0x3 << cells[i+1]);
			mmio_write_32(SSPCC_BASE+cells[i], reg |
				    cells[i+2] << cells[i+1]);
		}
	}

	if (fdt_getprop(fdt, node, "gpio_s", &value_len) != 0) {
		count = value_len / 4;
		fdt_read_uint32_array(fdt, node, "gpio_s", count, cells);

		for (i = 0; i < count; i += 3) {
			reg = mmio_read_32(SSPCC_BASE+cells[i]) &
					    ~(0x3 << cells[i+1]);
			mmio_write_32(SSPCC_BASE+cells[i],  reg | cells[i+2] <<
				    cells[i+1]);
		}
	}

	/* enable WDT1/WDT2 reset */
	mmio_write_32((SYS_BA+0x14), 0x70000);

	/* Start M4 after configuring peripheral attribution. */
	mmio_write_32(SYS_IPRST0, mmio_read_32(SYS_IPRST0) & ~0x8);

	/* Release core 1 when its SCP image address is ready. */
	if (mmio_read_32(SYS_BA + 0x48) == SCPBL2_BASE)
		sev();

	/* lock */
	mmio_write_32(SYS_RLKTZS, 0);
}

/* PLL frequency limits */
#define HZ_PER_MHZ		U(1000000)
#define PLL_FREF_MAX_FREQ	(200 * HZ_PER_MHZ)
#define PLL_FREF_MIN_FREQ	(1 * HZ_PER_MHZ)
#define PLL_FREF_M_MAX_FREQ	(40 * HZ_PER_MHZ)
#define PLL_FREF_M_MIN_FREQ	(10 * HZ_PER_MHZ)
#define PLL_FCLK_MAX_FREQ	(2400 * HZ_PER_MHZ)
#define PLL_FCLK_MIN_FREQ	(600 * HZ_PER_MHZ)
#define PLL_FCLKO_MAX_FREQ	(2400 * HZ_PER_MHZ)
#define PLL_FCLKO_MIN_FREQ	(85700000)
#define PLL_SS_RATE		0x77
#define PLL_SLOPE		0x58CFA

/* bit fields for REG_CLK_PLLxCTL0 ~ REG_CLK_PLLxCTL2, where x = 2 ~ 5 */
#define PLL_CTL0_FBDIV_POS	0
#define PLL_CTL0_INDIV_POS	12
#define PLL_CTL0_MODE		GENMASK(19, 18)
#define PLL_CTL0_SSRATE		GENMASK(30, 20)
#define PLL_CTL1_PD		BIT(0)
#define PLL_CTL1_BP		BIT(1)
#define PLL_CTL1_OUTDIV_POS	4
#define PLL_CTL1_FRAC		GENMASK(31, 24)
#define PLL_CTL2_SLOPE		GENMASK(23, 0)

#define INDIV_MIN		1
#define INDIV_MAX		63
#define FBDIV_MIN		16
#define FBDIV_MAX		2047
#define FBDIV_FRAC_MIN		1600
#define FBDIV_FRAC_MAX		204700
#define OUTDIV_MIN		1
#define OUTDIV_MAX		7

#define PLL_MODE_INT		0
#define PLL_MODE_FRAC		1
#define PLL_MODE_SS		2

static int ma35d1_pll_find_closest(unsigned long rate, unsigned long parent_rate,
				   uint32_t pll_mode, uint32_t *reg_ctl,
				   uint32_t *freq)
{
	unsigned long min_diff = U(0xffffffff);
	int fbdiv_min, fbdiv_max;
	int p, m, n;

	*freq = 0;
	if (rate < PLL_FCLKO_MIN_FREQ || rate > PLL_FCLKO_MAX_FREQ)
		return -1;

	if (pll_mode == PLL_MODE_INT) {
		fbdiv_min = FBDIV_MIN;
		fbdiv_max = FBDIV_MAX;
	} else {
		fbdiv_min = FBDIV_FRAC_MIN;
		fbdiv_max = FBDIV_FRAC_MAX;
	}

	for (m = INDIV_MIN; m <= INDIV_MAX; m++) {
		for (n = fbdiv_min; n <= fbdiv_max; n++) {
			for (p = OUTDIV_MIN; p <= OUTDIV_MAX; p++) {
				unsigned long tmp, fout, fclk, diff;

				tmp = parent_rate / m;
				if (tmp < PLL_FREF_M_MIN_FREQ ||
				    tmp > PLL_FREF_M_MAX_FREQ)
					continue; /* constrain */

				fclk = parent_rate * n / m;
				/* for 2 decimal places */
				if (pll_mode != PLL_MODE_INT)
					fclk = fclk / 100;

				if (fclk < PLL_FCLK_MIN_FREQ ||
				    fclk > PLL_FCLK_MAX_FREQ)
					continue; /* constrain */

				fout = fclk / p;
				if (fout < PLL_FCLKO_MIN_FREQ ||
				    fout > PLL_FCLKO_MAX_FREQ)
					continue; /* constrain */

				diff = (rate > fout) ? (rate - fout) : (fout - rate);
				if (diff < min_diff) {
					reg_ctl[0] = (m << PLL_CTL0_INDIV_POS) |
					     (n << PLL_CTL0_FBDIV_POS);
					reg_ctl[1] = p << PLL_CTL1_OUTDIV_POS;
					*freq = fout;
					min_diff = diff;
					if (min_diff == 0)
						goto out;
				}
			}
		}
	}
out:
	if (*freq == 0)
		return -1; /* cannot find even one valid setting */
	return 0;
}
