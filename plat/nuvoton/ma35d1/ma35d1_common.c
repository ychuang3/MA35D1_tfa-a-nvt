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

void ma35d1_i2c0_init(unsigned int sys_clk);

/* CPU-PLL: 1000MHz 800MHz 700MHz */
static unsigned int CAPLL_MODE0[3] = {
	0x000006FA,	/* 1000 MHz */
	0x00000364,	/* 800 MHz */
	0x000006AF,	/* 700 MHz */
};

static void *fdt = (void *)(uintptr_t)MA35D1_DTB_BASE;

static void ma35d1_clock_setup(void)
{
	unsigned int pllmode[6] = { 0, 0, 0, 0, 0, 0 };
	unsigned int pllfreq[6] = { 0, 0, 0, 0, 0, 0 };
	unsigned int speed = 500000000;
	unsigned int clock, index = 2;
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
	if ((inp32((void *)CLK_PLL4CTL1) & 0x1) == 0x1) {
		outp32((void *)CLK_PLL4CTL0, 0x000060FA);
		outp32((void *)CLK_PLL4CTL1, 0x00000020);
		outp32((void *)CLK_PLL4CTL2, 0x0);
		/* check PLL stable */
		while (1) {
			if ((inp32((void *)CLK_STATUS) & 0x200) == 0x200)
				break;
		}
	}

	pmic_clk = pllfreq[1]; /* I2C0 clock for PMIC */

	/* CA-PLL */
	clock = (pllfreq[0] < speed) ? speed : pllfreq[0];
	switch (clock) {
	case 1000000000: /* 1.302V */
		/* set the voltage VDD_CPU first */
		if (ma35d1_set_pmic(VOL_CPU, VOL_1_30))
			INFO("CA-PLL is %d Hz\n", clock);
		else
			WARN("CA-PLL is %d Hz without PSCI setting.\n",
				clock);
		index = 0;
		break;
	case 800000000: /* 1.248V */
			/* set the voltage VDD_CPU first */
			if (ma35d1_set_pmic(VOL_CPU, VOL_1_25))
				INFO("CA-PLL is %d Hz\n", clock);
			else
				WARN("CA-PLL is %d Hz without PSCI setting.\n",
					clock);
			break;
	case 700000000:
			index = 2;
			INFO("CA-PLL is %d Hz\n", clock);
			break;
	default:
			INFO("CA-PLL is %d Hz\n", clock);
			return;
	};
	/* set CA35 to E-PLL */
	outp32((void *)CLK_CLKSEL0, (inp32((void *)CLK_CLKSEL0) & ~0x3) | 0x2);

	outp32((void *)CLK_PLL0CTL0, CAPLL_MODE0[index]);

	/* check PLL stable */
	while (1) {
		if ((inp32((void *)CLK_STATUS) & 0x40) == 0x40)
			break;
	}
	/* set CA35 to CA-PLL */
	outp32((void *)CLK_CLKSEL0, (inp32((void *)CLK_CLKSEL0) & ~0x3) | 0x1);

	/* check LXT */
	if (fdt_read_uint32_default(fdt, node, "lxt-enable", 0) == 1) {
		outp32((void *)CLK_PWRCTL, inp32((void *)CLK_PWRCTL) | 0x2);
	}

	if (fdt_read_uint32_default(fdt, node, "rtc-pwrctl-enable", 1) == 1)
		outp32((void *)(0x40410180),
			inp32((void *)(0x40410180)) |
			0x5aa50040);  /* power control enable */
	else	/* power control disable */
		outp32((void *)(TSI_CLK_BASE+0x40),
			(inp32((void *)(0x40410180)) & ~0xffff0040) |
			0x5aa50000);


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
	/* unlock */
	outp32((void *)SYS_RLKTZS, 0x59);
	outp32((void *)SYS_RLKTZS, 0x16);
	outp32((void *)SYS_RLKTZS, 0x88);

	/* Enable UART0 clock */
	outp32((void *)CLK_APBCLK0, inp32((void *)CLK_APBCLK0) | (1 << 12));
	outp32((void *)CLK_CLKSEL2, inp32((void *)CLK_CLKSEL2) & ~(3 << 16));
	outp32((void *)CLK_CLKDIV1, inp32((void *)CLK_CLKDIV1) & ~(0xf << 16));
	/* UART0 multi-function */
	outp32((void *)SYS_GPE_MFPH, (inp32((void *)SYS_GPE_MFPH) &
			~0xff000000) | 0x11000000);

	console_ma35d1_register(PLAT_ARM_CRASH_UART_BASE,
				PLAT_ARM_CRASH_UART_CLK_IN_HZ,
				ARM_CONSOLE_BAUDRATE,
				&ma35d1_console);

	INFO("ma35d1 config setup\n");

	/* Set the PLL */
	ma35d1_clock_setup();

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
	unsigned int cells[70 * 3];
	unsigned int reg;

	/* unlock */
	outp32((void *)SYS_RLKTZS, 0x59);
	outp32((void *)SYS_RLKTZS, 0x16);
	outp32((void *)SYS_RLKTZS, 0x88);

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	/* enable CRYPTO */
	if ((inp32((void *)SYS_CHIPCFG) & 0x100) == 0x100) {
		/* un-lock */
		do {
			outp32((void *)(TSI_SYS_BASE+0x100), 0x59);
			outp32((void *)(TSI_SYS_BASE+0x100), 0x16);
			outp32((void *)(TSI_SYS_BASE+0x100), 0x88);
		} while (inp32((void *)(TSI_SYS_BASE+0x100)) == 0UL);

		/* set TSI-PLL to HIRC */
		if ((inp32((void *)(TSI_CLK_BASE+0x10)) & 0x7) == 0x2) {
			outp32((void *)(TSI_CLK_BASE+0x10),
				inp32((void *)(TSI_CLK_BASE+0x10)) & ~0x7);
		}
		/* PLL to 200 MHz */
		outp32((void *)(TSI_CLK_BASE+0x40), 0x808cc8);
		while (1) {
			if ((inp32((void *)(TSI_CLK_BASE+0x50))
				& 0x4) == 0x4) {
				outp32((void *)(TSI_CLK_BASE+0x10),
					(inp32((void *)(TSI_CLK_BASE+0x10)) &
					~0x7) | 0x2);
				break;
			}
		}
		/* initial crypto engine and ks clock */
		outp32((void *)(TSI_CLK_BASE+0x04),
			    (inp32((void *)(TSI_CLK_BASE+0x04)) | 0x5000));
		/* initial trng clock */
		outp32((void *)(TSI_CLK_BASE+0x0c),
			    (inp32((void *)(TSI_CLK_BASE+0x0c)) |
			    0x2000000));

		/* Init KeyStore */
		/* KS INIT(KS_CTL[8]) + START(KS_CTL[0]) */
		outp32((void *)(KS_BASE+0x00), 0x101);
		while ((inp32((void *)(KS_BASE+0x08)) & 0x80) == 0)
			;   /* wait for INITDONE(KS_STS[7]) set */
		while (inp32((void *)(KS_BASE+0x08)) & 0x4)
			;      /* wait for BUSY(KS_STS[2]) cleared */
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d1-sspcc");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d1-sspcc` not found\n");
	}
	/* Enable RTP clock */
	outp32((void *)CLK_SYSCLK0, inp32((void *)CLK_SYSCLK0) | 0x2);
	/* enable SSPCC/GPIO clock */
	outp32((void *)CLK_APBCLK2, inp32((void *)CLK_APBCLK2) | 0x8);
	outp32((void *)CLK_SYSCLK1, inp32((void *)CLK_SYSCLK1) | 0x3FFF0000);
	/* set GPIO to TZNS */
	for (i = 0; i < 0x38; i += 4)
		outp32(SSPCC_BASE+0x60+i, 0x55555555);

	/* get peripheral attribution from DTB */
	if (fdt_getprop(fdt, node, "config", &value_len) != 0) {
		count = value_len / 4;
		fdt_read_uint32_array(fdt, node, "config", count, cells);

		for (i = 0; i < count; i += 3) {
			reg = inp32(SSPCC_BASE+cells[i]) &
					    ~(0x3 << cells[i+1]);
			outp32(SSPCC_BASE+cells[i], reg |
				    cells[i+2] << cells[i+1]);
		}
	}

	if (fdt_getprop(fdt, node, "gpio_s", &value_len) != 0) {
		count = value_len / 4;
		fdt_read_uint32_array(fdt, node, "gpio_s", count, cells);

		for (i = 0; i < count; i += 3) {
			reg = inp32(SSPCC_BASE+cells[i]) &
					    ~(0x3 << cells[i+1]);
			outp32(SSPCC_BASE+cells[i],  reg | cells[i+2] <<
				    cells[i+1]);
		}
	}

	/* enable WDT1/WDT2 reset */
	outp32((void *)(SYS_BA+14), 0x70000);

	/* Release Cortex-M4 core reset. */
	outp32((void *)SYS_IPRST0, inp32((void *)SYS_IPRST0) & ~0x8);

	/* lock */
	outp32((void *)SYS_RLKTZS, 0);
}
