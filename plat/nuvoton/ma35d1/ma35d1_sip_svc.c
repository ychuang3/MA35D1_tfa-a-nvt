/*
 * Copyright (C) 2021 Nuvoton Technology Corp.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <tools_share/uuid.h>

#include <ma35d1_sip_svc.h>
#include <drivers/nuvoton/ma35d1_pmic.h>
#include "ma35d1_private.h"
#include "include/ma35d1_crypto.h"
#include "include/tsi_cmd.h"

#pragma weak ma35d1_plat_sip_handler

/* CPU-PLL: 1000MHz 700MHz */
static unsigned int CAPLL_MODE[7] = {
	0x000006FA,	/* 1000 MHz */
	0x00000364,	/* 800 MHz */
	0x000006AF,	/* 700 MHz */
	0x00001396,	/* 600 MHz */
	0x0000137D,	/* 500 MHz */
	0x0000237D,	/* 250 MHz */
	0x0000337D,	/* 125 MHz */
};

static uint32_t eppl_div_restore = 0xFFFFFFFF;
static uint32_t eppl_ctl1 = 0x1, vppl_ctl1 = 0x1;

uintptr_t ma35d1_plat_sip_handler(uint32_t smc_fid,
				    u_register_t x1,
				    u_register_t x2,
				    u_register_t x3,
				    u_register_t x4,
				    void *cookie,
				    void *handle,
				    u_register_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

/*
 * This function changes the CPU PLL
 */
static int32_t ma35d1_set_cpu_clock(int cpu_freq)
{
	uint64_t timeout;
	uint32_t index = 2; /* 500 MHz */

	/* CA-PLL */
	switch (cpu_freq) {
	case CPU_PLL_1G:
		index = 0;
		INFO("CA-PLL is 1000 MHz\n");
		break;
	case CPU_PLL_800: /* 1.248V */
		index = 1;
		INFO("CA-PLL is 800 MHz\n");
		break;
	case CPU_PLL_700:
		index = 2;
		INFO("CA-PLL is 700 MHz\n");
		break;
	case CPU_PLL_600:
		index = 3;
		INFO("CA-PLL is 600 MHz\n");
		break;
	case CPU_PLL_500:
		index = 4;
		INFO("CA-PLL is 500 MHz\n");
		break;
	case CPU_PLL_250:
		index = 5;
		INFO("CA-PLL is 250 MHz\n");
		break;
	case CPU_PLL_125:
		index = 6;
		INFO("CA-PLL is 125 MHz\n");
		break;
	default:
		WARN("Invalid CA-PLL !!\n");
		return 1;
	};
	/* set CA35 to DDRPLL */
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

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uintptr_t sip_smc_handler(uint32_t smc_fid,
			  u_register_t x1,
			  u_register_t x2,
			  u_register_t x3,
			  u_register_t x4,
			  void *cookie,
			  void *handle,
			  u_register_t flags)
{
	uint32_t ns;
	uint32_t volt;
	int ret;
	int cpu_clk = 0;

	/* unlock */
	mmio_write_32(SYS_RLKTZS, 0x59);
	mmio_write_32(SYS_RLKTZS, 0x16);
	mmio_write_32(SYS_RLKTZS, 0x88);


	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns)
		SMC_RET1(handle, SMC_UNK);

	switch (smc_fid) {
	case SIP_SVC_PMIC:
		volt = (uint32_t)x2;
		if (volt == 0)
			volt = ma35d1_get_pmic(x1);
		else {
			if (volt != ma35d1_get_pmic(x1))
				ma35d1_set_pmic(x1, x2);
		}
		mmio_write_32(SYS_RLKTZS, 0);
		SMC_RET1(handle, volt);

	case SIP_CPU_CLK:
		if ((uint32_t)x1 == 1000)
			cpu_clk = CPU_PLL_1G;
		else if ((uint32_t)x1 == 800)
			cpu_clk = CPU_PLL_800;
		else if ((uint32_t)x1 == 700)
			cpu_clk = CPU_PLL_700;
		else if ((uint32_t)x1 == 600)
			cpu_clk = CPU_PLL_600;
		else if ((uint32_t)x1 == 500)
			cpu_clk = CPU_PLL_500;
		else if ((uint32_t)x1 == 250)
			cpu_clk = CPU_PLL_250;
		else if ((uint32_t)x1 == 125)
			cpu_clk = CPU_PLL_125;
		else
			cpu_clk = CPU_PLL_500;

		ret = ma35d1_set_cpu_clock(cpu_clk);
		if (ret == 1) {
			WARN("Set CPU clock %ld Fail !!\n", x1);
		}
		mmio_write_32(SYS_RLKTZS, 0);
		SMC_RET1(handle, ret);

	case SIP_SET_EPLL:
		if (eppl_div_restore == 0xFFFFFFFF)
			eppl_div_restore = mmio_read_32(CLK_PLL4CTL1);
		if ((uint32_t)x1 == NVT_SIP_SVC_EPLL_DIV_BY_2)
			mmio_write_32(CLK_PLL4CTL1, eppl_div_restore + 0x10);
		else if ((uint32_t)x1 == NVT_SIP_SVC_EPLL_DIV_BY_4)
			mmio_write_32(CLK_PLL4CTL1, eppl_div_restore + 0x20);
		else if ((uint32_t)x1 == NVT_SIP_SVC_EPLL_DIV_BY_8)
			mmio_write_32(CLK_PLL4CTL1, eppl_div_restore + 0x30);
		else {
			mmio_write_32(CLK_PLL4CTL1, eppl_div_restore);
			eppl_div_restore = 0xFFFFFFFF;
		}
		mmio_write_32(SYS_RLKTZS, 0);
		SMC_RET2(handle, 0, mmio_read_32(CLK_PLL4CTL1));

	case SIP_LOW_SPEED:
		if ((uint32_t)x1 == 0) {
			ma35d1_set_cpu_clock(CPU_PLL_500);
			if ((mmio_read_32(SYS_CHIPCFG) & (1 << 8)) == 0)
				TSI_Set_Clock(0x80235A);
			mmio_write_32(CLK_PLL4CTL1, eppl_ctl1);
			mmio_write_32(CLK_PLL5CTL1, vppl_ctl1);
		} else {
			ma35d1_set_cpu_clock(CPU_PLL_125);
			if ((mmio_read_32(SYS_CHIPCFG) & (1 << 8)) == 0)
				TSI_Set_Clock(0x802312);
			eppl_ctl1 = mmio_read_32(CLK_PLL4CTL1);
			vppl_ctl1 = mmio_read_32(CLK_PLL5CTL1);
			mmio_write_32(CLK_PLL4CTL1, eppl_ctl1 | 0x70);
			mmio_write_32(CLK_PLL5CTL1, vppl_ctl1 | 0x70);
		}
		mmio_write_32(SYS_RLKTZS, 0);
		SMC_RET1(handle, 0);

	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET3(handle, 0, NVT_SIP_SVC_VERSION_MAJOR,
			NVT_SIP_SVC_VERSION_MINOR);

	default:
		mmio_write_32(SYS_RLKTZS, 0);
		return ma35d1_plat_sip_handler(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
	};

}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	ma35d1_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);
