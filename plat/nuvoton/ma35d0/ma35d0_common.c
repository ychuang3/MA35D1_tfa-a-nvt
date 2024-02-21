/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
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
#include <drivers/nuvoton/ma35d0_pmic.h>

#include "ma35d0_private.h"

/* Defines for GIC Driver build time selection */
#define MA35D0_GICV2		1
#define MA35D0_GICV3		2

/*******************************************************************************
 * arm_config holds the characteristics of the differences between the
 * three MA35D0 platforms (Base, A53_A57 & Foundation). It will be
 * populated during cold boot at each boot stage by the primary before
 * enabling the MMU (to allow interconnect configuration) & used thereafter.
 * Each BL will have its own copy to allow independent operation.
 ******************************************************************************/
arm_config_t arm_config;

#if MA35D0_INTERCONNECT_DRIVER != MA35D0_CCN

static unsigned int get_interconnect_master(void)
{
	unsigned int master;
	u_register_t mpidr;

	mpidr = read_mpidr_el1();
	master = ((arm_config.flags & ARM_CONFIG_FVP_SHIFTED_AFF) != 0U) ?
		MPIDR_AFFLVL2_VAL(mpidr) : MPIDR_AFFLVL1_VAL(mpidr);

	assert(master < MA35D0_CLUSTER_COUNT);
	return master;
}
#endif

int console_ma35d0_register(uintptr_t baseaddr, uint32_t clock,
					uint32_t baud, console_t *console);
int console_ma35d0_putc(int character, console_t *console);
int console_ma35d0_getc(console_t *console);
void console_ma35d0_flush(console_t *console);
static console_t ma35d0_console;

static console_t ma35d0_console = {
	.flags = CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME |
			CONSOLE_FLAG_CRASH | CONSOLE_FLAG_TRANSLATE_CRLF,
	.putc = console_ma35d0_putc,
	.getc = console_ma35d0_getc,
	.flush = console_ma35d0_flush,
};

static void *fdt = (void *)(uintptr_t)MA35D0_DTB_BASE;

static void ma35d0_clkopin_setup(void)
{
	int node;
	unsigned int ClkoFreqDiv, ClkoSrc;

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d0-clk");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d0-clk` not found\n");
	}

	/* CLKO multi-function */
	if (fdt_read_uint32_default(fdt, node, "set-clko-pin", 1) == 1)
		mmio_write_32(SYS_GPN_MFPH, (mmio_read_32(SYS_GPN_MFPH) &
				~0xf0000000) | 0xa0000000);/* Set PN15 */
	else
		mmio_write_32(SYS_GPK_MFPL, (mmio_read_32(SYS_GPK_MFPL) &
			~0xf0000000) | 0x90000000);/* Set PK7 */

	/* set CLKO clock source */
	ClkoSrc = fdt_read_uint32_default(fdt, node, "clko-src", 1);
	mmio_write_32(CLK_CLKSEL4, (mmio_read_32(CLK_CLKSEL4)& ~(0xF << 24)) | (ClkoSrc << 24));

	/* Set CLKOCTL */
	if (fdt_read_uint32_default(fdt, node, "clko-div1-en", 1) == 1) {
		mmio_write_32((CLK_CLKOCTL),
			(mmio_read_32((CLK_CLKOCTL)) & ~0x00000020) |
			0x00000020);  /* Enable CLKO Div1 */
	}
	else {
		ClkoFreqDiv = fdt_read_uint32_default(fdt, node, "clko-freqdiv", 1);
		mmio_write_32((CLK_CLKOCTL),
			(mmio_read_32((CLK_CLKOCTL)) & ~0x0000000F) | ClkoFreqDiv);/* Enable FREQSEL */
	}
}

static void ma35d0_clock_setup(void)
{
	unsigned int pllmode[6] = { 0, 0, 0, 0, 0, 0 };
	unsigned int pllfreq[6] = { 0, 0, 0, 0, 0, 0 };
	int node;

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d0-clk");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d0-clk` not found\n");
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

	/* DDR-PLL */
	/* Set DDR-PLL control register0 */
	mmio_write_32(CLK_BA + 0x80,0x0F04102C);  //for DDRPLL is 266Mhz
	/* Set DDR-PLL control register1 */
	mmio_write_32(CLK_BA + 0x84,0x6B851E40);
	/* Set DDR-PLL control register2 */
	mmio_write_32(CLK_BA + 0x88,0x000048A3);
	/* polling DDR-PLL stable */
	while((mmio_read_32(CLK_BA + 0x50) & 0x00000100) != 0x00000100);

	/* set CA35 to E-PLL */
	mmio_write_32(CLK_CLKSEL0, (mmio_read_32(CLK_CLKSEL0) & ~0x3) | 0x2);

	/* CA-PLL */
	INFO("CA-PLL is 650MHz\n");
	mmio_write_32(CLK_PLL0CTL0, 0x000006a2); /* 648 MHz */

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
	if (fdt_read_uint32_default(fdt, node, "rtc-pwrctl-enable", 1) == 1)
		mmio_write_32((0x40410180),
			mmio_read_32((0x40410180)) |
			0x5aa50040);  /* power control enable */
	else	/* power control disable */
		mmio_write_32((0x40410180),
			(mmio_read_32((0x40410180)) & ~0xffff0040) |
			0x5aa50000);

	/* Set PH8/PH9 */
	if (fdt_read_uint32_default(fdt, node, "set-ph8-ph9-high", 1) == 1) {
		mmio_write_32((0x40410100),
			(mmio_read_32((0x40410100)) & ~0x00000100) |
			0x00000100);  /* Enable IOCTLSET */

		mmio_write_32((0x40410104),
			(mmio_read_32((0x40410104)) & ~0x00000707) |
			0x00000505); /* Set PH8/PH9 output high */
	}
	else {
		mmio_write_32((0x40410100),
			(mmio_read_32((0x40410100)) & ~0x00000100));  /* Disable IOCTLSET */
	}

	/* Set CLKO pin function */
	if (fdt_read_uint32_default(fdt, node, "clko-en", 1) == 1) {
		/* set CKO enable */
		mmio_write_32(CLK_SYSCLK1, mmio_read_32(CLK_SYSCLK1) | (0x1 << 13));
		mmio_write_32((CLK_CLKOCTL),
			(mmio_read_32((CLK_CLKOCTL)) & ~0x00000010) |
			0x00000010);  /* Enable CLKO pin Enable */
		INFO("ma35d0 CLKO enable. 0x%08x\n", mmio_read_32(CLK_CLKOCTL));
		ma35d0_clkopin_setup();
	}
}

/*******************************************************************************
 * A single boot loader stack is expected to work on both the Foundation MA35D0
 * models and the two flavours of the Base MA35D0 models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 ******************************************************************************/
void __init ma35d0_config_setup(void)
{
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

	console_ma35d0_register(PLAT_ARM_CRASH_UART_BASE,
				PLAT_ARM_CRASH_UART_CLK_IN_HZ,
				ARM_CONSOLE_BAUDRATE,
				&ma35d0_console);

	INFO("ma35d0 config setup\n");

	/* Set the PLL */
	ma35d0_clock_setup();

}


void __init ma35d0_interconnect_init(void)
{
}

void ma35d0_interconnect_enable(void)
{
	unsigned int master;

	if ((arm_config.flags & (ARM_CONFIG_FVP_HAS_CCI400 |
				 ARM_CONFIG_FVP_HAS_CCI5XX)) != 0U) {
		master = get_interconnect_master();
		cci_enable_snoop_dvm_reqs(master);
	}
}

void ma35d0_interconnect_disable(void)
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

void ma35d0_timer_init(void)
{
	generic_delay_timer_init();
}

void plat_ma35d0_init(void)
{
	int value_len = 0, i, count = 0;
	int node;
	unsigned int cells[100 * 3];
	unsigned int reg;

	/* unlock */
	mmio_write_32(SYS_RLKTZS, 0x59);
	mmio_write_32(SYS_RLKTZS, 0x16);
	mmio_write_32(SYS_RLKTZS, 0x88);

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	/* enable CRYPTO */
	/* initial crypto engine and ks clock */
	mmio_write_32(TSI_CLK_AHBCLK, mmio_read_32(TSI_CLK_AHBCLK) | 0x5000);

	/* initial trng clock */
	mmio_write_32(TSI_CLK_APBCLK1, mmio_read_32(TSI_CLK_APBCLK1) | 0x2000000);

	/* Init KeyStore */
	mmio_write_32((KS_BASE+0x00), 0x101);

	while ((mmio_read_32((KS_BASE+0x08)) & 0x80) == 0)
		;   /* wait for INITDONE(KS_STS[7]) set */

	while (mmio_read_32((KS_BASE+0x08)) & 0x4)
		;      /* wait for BUSY(KS_STS[2]) cleared */

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d0-sspcc");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d0-sspcc` not found\n");
	}

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

	/* enable WDT1 reset */
	mmio_write_32((SYS_BA+0x14), 0x50000);

	/* lock */
	mmio_write_32(SYS_RLKTZS, 0);
}
