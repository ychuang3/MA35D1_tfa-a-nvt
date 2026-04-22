/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <libfdt.h>

#include <plat/common/platform.h>
#include <platform_def.h>

#include <ma35h0_ddr.h>

#include <custom_ddr.h>

struct nvt_ddr_init_param ma35h0_wb_ddr2_128mb = {
	0x00000001, //(DBG1_1),     // 0
	0x00000001, //(PWRCTL_1),   // 1
	0x01040001, //(MSTR),       // 2
	0x00001010, //(MRCTRL0),    // 3
	0x00000000, //(MRCTRL1),    // 4
	0x00000000, //(PWRCTL_2),   // 5
	0x00400010, //(PWRTMG),     // 6
	0x000a0003, //(HWLPCTL),    // 7
	0x00210000, //(RFSHCTL0),   // 8
	0x003c003c, //(RFSHCTL1),   // 9
	0x00000000, //(RFSHCTL3),   // 10
	0x00100022, //(RFSHTMG),    // 11
	0x00000000, //(CRCPARCTL0), // 12
	0x40020035, //(INIT0),      // 13
	0x00010002, //(INIT1),      // 14
	0x0e730000, //(INIT3),      // 15
	0x00000080, //(INIT4),      // 16
	0x00010000, //(INIT5),      // 17
	0x00000000, //(DIMMCTL),    // 18
	0x0000031f, //(RANKCTL),    // 19
	0x0a0d110d, //(DRAMTMG0),   // 20
	0x0003040f, //(DRAMTMG1),   // 21
	0x00000407, //(DRAMTMG2),   // 22
	0x00001004, //(DRAMTMG3),   // 23
	0x04010305, //(DRAMTMG4),   // 24
	0x01010202, //(DRAMTMG5),   // 25
	0x00000502, //(DRAMTMG8),   // 26
	0x80000032, //(DRAMTMG15),  // 27
	0xc0000000, //(ZQCTL0),     // 28
	0x00074bf0, //(ZQCTL1),     // 29
	0x04020101, //(DFITMG0),    // 30
	0x00060101, //(DFITMG1),    // 31
	0x0700b030, //(DFILPCFG0),  // 32
	0x00400005, //(DFIUPD0),    // 33
	0x00170066, //(DFIUPD1),    // 34
	0x80000000, //(DFIUPD2),    // 35
	0x00000011, //(DFIMISC),    // 36
	0x00000000, //(DFIPHYMSTR), // 37
	0x0000001f, //(ADDRMAP0),   // 38
	0x00080808, //(ADDRMAP1),   // 39
	0x00000000, //(ADDRMAP2),   // 40
	0x00000000, //(ADDRMAP3),   // 41
	0x00001f1f, //(ADDRMAP4),   // 42
	0x070f0707, //(ADDRMAP5),   // 43
	0x0f0f0f07, //(ADDRMAP6),   // 44
	0x07070707, //(ADDRMAP9),   // 45
	0x07070707, //(ADDRMAP10),  // 46
	0x00000007, //(ADDRMAP11),  // 47
	0x07010708, //(ODTCFG),     // 48
	0x00000000, //(ODTMAP),     // 49
	0x00f51f00, //(SCHED),      // 50
	0x00000000, //(SCHED1),     // 51
	0x0f000001, //(PERFHPR1),   // 52
	0x0f00007f, //(PERFLPR1),   // 53
	0x0f00007f, //(PERFWR1),    // 54
	0x00000000, //(DBG0),       // 55
	0x00000000, //(DBG1_2),     // 56
	0x00000000, //(DBGCMD),     // 57
	0x00000001, //(SWCTL_1),    // 58
	0x00000000, //(SWCTLSTATIC),// 59
	0x00110011, //(POISONCFG),  // 60
	0x00000001, //(PCTRL_0),    // 61
	0x00000000, //(PCTRL_1),    // 62
	0x00000000, //(PCTRL_2),    // 63
	0x00000000, //(PCTRL_3),    // 64
	0x00000000, //(PCTRL_4),    // 65
	0x00000001, //(PCTRL_5),    // 66
	0x00000001, //(PCTRL_6),    // 67
	0x00000000, //(PCCFG),      // 68
	0x0001500f, //(PCFGR_0),    // 69
	0x0001500f, //(PCFGR_1),    // 70
	0x0001500f, //(PCFGR_2),    // 71
	0x0001500f, //(PCFGR_3),    // 72
	0x0001500f, //(PCFGR_4),    // 73
	0x0001500f, //(PCFGR_5),    // 74
	0x0001500f, //(PCFGR_6),    // 75
	0x0000500f, //(PCFGW_0),    // 76
	0x0000500f, //(PCFGW_1),    // 77
	0x0000500f, //(PCFGW_2),    // 78
	0x0000500f, //(PCFGW_3),    // 79
	0x0000500f, //(PCFGW_4),    // 80
	0x0000500f, //(PCFGW_5),    // 81
	0x0000500f, //(PCFGW_6),    // 82
	0x00000008, //(SARBASE0),   // 83
	0x00000000, //(SARSIZE0),   // 84

	// DDR PHY
	0xf004649f, //(DSGCR),      // 85
	0x0300c461, //(PGCR1),      // 86
	0x00f0027f, //(PGCR2),      // 87
	0x0c806403, //(PTR0),       // 88
	0x27100385, //(PTR1),       // 89
	0x00083def, //(PTR2),       // 90
	0x0d61a072, //(PTR3),       // 91
	0x0559a072, //(PTR4),       // 92
	0x00000e73, //(MR0),        // 93
	0x00000000, //(MR1),        // 94
	0x00000080, //(MR2),        // 95
	0x00000000, //(MR3),        // 96
	0x71997755, //(DTPR0),      // 97
	0x22822b22, //(DTPR1),      // 98
	0x1001a8c8, //(DTPR2),      // 99
	0x0000104b, //(ZQ0CR1),     // 100
	0x0000040a, //(DCR),        // 101
	0x91003587, //(DTCR),       // 102
	0x0001c000, //(PLLCR),      // 103
	0x0000f501, //(PIR),        // 104

	0x00000000, //(SWCTL_2),    // 105
	0x0000000b, //(PWRCTL_3),   // 106
	0x00000001, //(SWCTL_3),    // 107
};


static const uint32_t pid_list[] = {
	0x10A20000,
	0x10A30000,
	0x10A40000,
};

struct DDR_Setting nvt_ddr_init_setting[] = {
	DDR_CTL_REG(DBG1_1),     // 0
	DDR_CTL_REG(PWRCTL_1),   // 1
	DDR_CTL_REG(MSTR),       // 2
	DDR_CTL_REG(MRCTRL0),    // 3
	DDR_CTL_REG(MRCTRL1),    // 4
	DDR_CTL_REG(PWRCTL_2),   // 5
	DDR_CTL_REG(PWRTMG),     // 6
	DDR_CTL_REG(HWLPCTL),    // 7
	DDR_CTL_REG(RFSHCTL0),   // 8
	DDR_CTL_REG(RFSHCTL1),   // 9
	DDR_CTL_REG(RFSHCTL3),   // 10
	DDR_CTL_REG(RFSHTMG),    // 11
	DDR_CTL_REG(CRCPARCTL0), // 12
	DDR_CTL_REG(INIT0),      // 13
	DDR_CTL_REG(INIT1),      // 14
	DDR_CTL_REG(INIT3),      // 15
	DDR_CTL_REG(INIT4),      // 16
	DDR_CTL_REG(INIT5),      // 17
	DDR_CTL_REG(DIMMCTL),    // 18
	DDR_CTL_REG(RANKCTL),    // 19
	DDR_CTL_REG(DRAMTMG0),   // 20
	DDR_CTL_REG(DRAMTMG1),   // 21
	DDR_CTL_REG(DRAMTMG2),   // 22
	DDR_CTL_REG(DRAMTMG3),   // 23
	DDR_CTL_REG(DRAMTMG4),   // 24
	DDR_CTL_REG(DRAMTMG5),   // 25
	DDR_CTL_REG(DRAMTMG8),   // 26
	DDR_CTL_REG(DRAMTMG15),  // 27
	DDR_CTL_REG(ZQCTL0),     // 28
	DDR_CTL_REG(ZQCTL1),     // 29
	DDR_CTL_REG(DFITMG0),    // 30
	DDR_CTL_REG(DFITMG1),    // 31
	DDR_CTL_REG(DFILPCFG0),  // 32
	DDR_CTL_REG(DFIUPD0),    // 33
	DDR_CTL_REG(DFIUPD1),    // 34
	DDR_CTL_REG(DFIUPD2),    // 35
	DDR_CTL_REG(DFIMISC),    // 36
	DDR_CTL_REG(DFIPHYMSTR), // 37
	DDR_CTL_REG(ADDRMAP0),   // 38
	DDR_CTL_REG(ADDRMAP1),   // 39
	DDR_CTL_REG(ADDRMAP2),   // 40
	DDR_CTL_REG(ADDRMAP3),   // 41
	DDR_CTL_REG(ADDRMAP4),   // 42
	DDR_CTL_REG(ADDRMAP5),   // 43
	DDR_CTL_REG(ADDRMAP6),   // 44
	DDR_CTL_REG(ADDRMAP9),   // 45
	DDR_CTL_REG(ADDRMAP10),  // 46
	DDR_CTL_REG(ADDRMAP11),  // 47
	DDR_CTL_REG(ODTCFG),     // 48
	DDR_CTL_REG(ODTMAP),     // 49
	DDR_CTL_REG(SCHED),      // 50
	DDR_CTL_REG(SCHED1),     // 51
	DDR_CTL_REG(PERFHPR1),   // 52
	DDR_CTL_REG(PERFLPR1),   // 53
	DDR_CTL_REG(PERFWR1),    // 54
	DDR_CTL_REG(DBG0),       // 55
	DDR_CTL_REG(DBG1_2),     // 56
	DDR_CTL_REG(DBGCMD),     // 57
	DDR_CTL_REG(SWCTL_1),    // 58
	DDR_CTL_REG(SWCTLSTATIC),// 59
	DDR_CTL_REG(POISONCFG),  // 60
	DDR_CTL_REG(PCTRL_0),    // 61
	DDR_CTL_REG(PCTRL_1),    // 62
	DDR_CTL_REG(PCTRL_2),    // 63
	DDR_CTL_REG(PCTRL_3),    // 64
	DDR_CTL_REG(PCTRL_4),    // 65
	DDR_CTL_REG(PCTRL_5),    // 66
	DDR_CTL_REG(PCTRL_6),    // 67
	DDR_CTL_REG(PCCFG),      // 68
	DDR_CTL_REG(PCFGR_0),    // 69
	DDR_CTL_REG(PCFGR_1),    // 70
	DDR_CTL_REG(PCFGR_2),    // 71
	DDR_CTL_REG(PCFGR_3),    // 72
	DDR_CTL_REG(PCFGR_4),    // 73
	DDR_CTL_REG(PCFGR_5),    // 74
	DDR_CTL_REG(PCFGR_6),    // 75
	DDR_CTL_REG(PCFGW_0),    // 76
	DDR_CTL_REG(PCFGW_1),    // 77
	DDR_CTL_REG(PCFGW_2),    // 78
	DDR_CTL_REG(PCFGW_3),    // 79
	DDR_CTL_REG(PCFGW_4),    // 80
	DDR_CTL_REG(PCFGW_5),    // 81
	DDR_CTL_REG(PCFGW_6),    // 82
	DDR_CTL_REG(SARBASE0),   // 83
	DDR_CTL_REG(SARSIZE0),   // 84

	// DDR PHY
	DDR_PHY_REG(DSGCR),      // 85
	DDR_PHY_REG(PGCR1),      // 86
	DDR_PHY_REG(PGCR2),      // 87
	DDR_PHY_REG(PTR0),       // 88
	DDR_PHY_REG(PTR1),       // 89
	DDR_PHY_REG(PTR2),       // 90
	DDR_PHY_REG(PTR3),       // 91
	DDR_PHY_REG(PTR4),       // 92
	DDR_PHY_REG(MR0),        // 93
	DDR_PHY_REG(MR1),        // 94
	DDR_PHY_REG(MR2),        // 95
	DDR_PHY_REG(MR3),        // 96
	DDR_PHY_REG(DTPR0),      // 97
	DDR_PHY_REG(DTPR1),      // 98
	DDR_PHY_REG(DTPR2),      // 99
	DDR_PHY_REG(ZQ0CR1),     // 100
	DDR_PHY_REG(DCR),        // 101
	DDR_PHY_REG(DTCR),       // 102
	DDR_PHY_REG(PLLCR),      // 103
	DDR_PHY_REG(PIR),        // 104

	DDR_CTL_REG(SWCTL_2),    // 105
	DDR_CTL_REG(PWRCTL_3),   // 106
	DDR_CTL_REG(SWCTL_3),    // 107

};


static inline int pid_requires_20nm(uint32_t pid)
{
	uint32_t k;
	for (k = 0; k < (sizeof(pid_list) / sizeof(pid_list[0])); k++) {
		if (pid == pid_list[k])
			return 1;
	}
	return 0;
}

void ma35h0_ddr_setting(struct nvt_ddr_init_param ddrparam, int size)
{
	uint32_t i;
	uint64_t ddr_reg_address;
	uint32_t value;
	uint32_t u32TimeOut1 = 0, u32TimeOut2 = 0, u32TimeOut3 = 0;
	uint32_t pid, is_20nm;

	pid = mmio_read_32(SYS_BA) & 0xffff0000;
	is_20nm = pid_requires_20nm(pid);

	for(i = 0; i < size; i++)
	{
		ddr_reg_address = (uint32_t)nvt_ddr_init_setting[i].base + (uint32_t)nvt_ddr_init_setting[i].offset;
		value =  *((uint32_t *)(((uintptr_t)&ddrparam) + nvt_ddr_init_setting[i].init_flow_offset));

		if (i == 100 && is_20nm && ((value & 0xF0) != 0x50)) {
			INFO("DDR process check: 20 nm target (pid=0x%08x). "
			     "Consider verifying the register value.\n", pid);
		}

		*(volatile uint32_t *)(ddr_reg_address) = value;

		if (i == 84) // 0xf08 //DDRCTRL
		{
			//de-assert reset signals of DDR memory controller
			mmio_write_32(SYS_BA+0x20, (mmio_read_32(SYS_BA+0x20) & 0x8fffffff));
			while ((mmio_read_32(SYS_BA+0x20) & 0x20000000) != 0x00000000)
				;
		}

		if (i == 100) // 0x184 //DDRPHY
		{
			//polling PGSR0 (addr=4) to 0x0000000f
			while ((mmio_read_32(DDRPHY_BASE + 0x010) & 0x0000000f) != 0x0000000f)
			{
				u32TimeOut1++;
			}
		}

		if (i == 104) // 0x04  // DDRPHY
		{

			//polling PGSR0 (addr=4) to 0xb0000f5f
			while ((mmio_read_32(DDRPHY_BASE + 0x010) & 0xffffff5f) != 0xb0000f5f)
			{
				u32TimeOut2++;
			}

			//polling MCTL2 STAT to 0x00000001
			while ((mmio_read_32(UMCTL2_BASE + 0x004) & 0x00000003) != 0x00000001)
			{
				u32TimeOut3++;
			}
		}

	}

	/* Adjust AXI port priorities to favor DCUltra. */
	mmio_write_32(UMCTL2_BA + 0x328, 0x1);
	INFO("\n  >>>>> AXI Port Priority Finish: 0x%x, 0x%x, 0x%x, 0x%x\n",
	     mmio_read_32(UMCTL2_BA + 0x564),
	     mmio_read_32(UMCTL2_BA + 0x4b4),
	     mmio_read_32(UMCTL2_BA + 0x614),
	     mmio_read_32(UMCTL2_BA + 0x404));

	mmio_write_32(UMCTL2_BA + 0x564,
		      mmio_read_32(UMCTL2_BA + 0x564) | (0x1 << 5));
	mmio_write_32(UMCTL2_BA + 0x568,
		      mmio_read_32(UMCTL2_BA + 0x568) | (0x1 << 5));
	mmio_write_32(UMCTL2_BA + 0x4b4,
		      mmio_read_32(UMCTL2_BA + 0x4b4) | (0x8 << 5));
	mmio_write_32(UMCTL2_BA + 0x4b8,
		      mmio_read_32(UMCTL2_BA + 0x4b8) | (0x8 << 5));
	mmio_write_32(UMCTL2_BA + 0x614,
		      mmio_read_32(UMCTL2_BA + 0x614) | (0x10 << 5));
	mmio_write_32(UMCTL2_BA + 0x618,
		      mmio_read_32(UMCTL2_BA + 0x618) | (0x10 << 5));
	mmio_write_32(UMCTL2_BA + 0x404,
		      mmio_read_32(UMCTL2_BA + 0x404) | (0x1f << 5));
	mmio_write_32(UMCTL2_BA + 0x408,
		      mmio_read_32(UMCTL2_BA + 0x408) | (0x1f << 5));
	mmio_write_32(UMCTL2_BA + 0x328, 0x0);
	while ((mmio_read_32(UMCTL2_BA + 0x324) & 0x1) != 0x00000001)
		;

	INFO("\n  >>>>> AXI Port Priority Finish: 0x%x, 0x%x, 0x%x, 0x%x\n",
	     mmio_read_32(UMCTL2_BA + 0x564),
	     mmio_read_32(UMCTL2_BA + 0x4b4),
	     mmio_read_32(UMCTL2_BA + 0x614),
	     mmio_read_32(UMCTL2_BA + 0x404));

	while ((mmio_read_32(UMCTL2_BASE + 0x324) & 0x00000001) != 0x00000001)
		;

	INFO("\n DDR init Finish: 0x%x, 0x%x, 0x%x \n", u32TimeOut1, u32TimeOut2, u32TimeOut3);

	//while(1);
}

static void *fdt = (void *)(uintptr_t)MA35H0_DTB_BASE;

void ma35h0_ddr_init(void)
{
	uint32_t clk_sel0;

	clk_sel0 = mmio_read_32(CLK_BA + 0x18);

	/* Use SYS-PLL for SYS_CLK0, DCUltra and GFX during AXI setup. */
	mmio_write_32(CLK_BA + 0x18, 0xd000015);

	//Set TAHBCKEN,CM4CKEN,CA35CKEN,DDR6CKEN,GFXCKEN,VC8KCKEN,DCUCKEN,GMAC0CKEN,GMAC1CKEN,CAP0CKEN,CAP1CKEN
	mmio_write_32(CLK_BA + 0x04, (mmio_read_32(CLK_BA + 0x04) | 0x7F000037));
	mmio_write_32(CLK_BA + 0x0C, (mmio_read_32(CLK_BA + 0x0C) | 0x40000000));

	/* DDR control register clock gating disable */
	mmio_write_32(SYS_BA + 0x70, (mmio_read_32(SYS_BA + 0x70) | 0x00800000));
	/* de-assert presetn of MCTL2 */
	mmio_write_32(SYS_BA + 0x20, (mmio_read_32(SYS_BA + 0x20) & 0xafffffff));
	while ((mmio_read_32(SYS_BA + 0x20) & 0x50000000) != 0x00000000)
		;
	//set MCTLCRST to 1
	mmio_write_32(SYS_BA + 0x20, (mmio_read_32(SYS_BA + 0x20) | 0x20000000));

	/* read DTB */
	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	if (fdt_node_offset_by_compatible(fdt, -1, "wb-ddr2-128mb") >= 0) {
		INFO("Current ddr setting: internal-ddr2-128mb\n");
		ma35h0_ddr_setting(ma35h0_wb_ddr2_128mb, sizeof(ma35h0_wb_ddr2_128mb)/sizeof(uint32_t));
	} else if (fdt_node_offset_by_compatible(fdt, -1, "custom-ddr") >= 0) {
		INFO("Current ddr setting: custom-ddr\n");
		if (PID_CHECK != 0xFFFFFFFF) {
			if ((mmio_read_32(SYS_BA) & 0xffff0000) != (PID_CHECK&0xffff0000))
				WARN("The DDR parameters used may not be compatible with the IC model\n");
		}
		ma35h0_ddr_setting(custom_ddr, sizeof(custom_ddr)/sizeof(uint32_t));
	} else {
		WARN("The compatible property ddr type not found\n");
	}

	mmio_write_32(UMCTL2_BA+0x490, 0x1);
	mmio_write_32(UMCTL2_BA+0x8b0, 0x1);
	mmio_write_32(UMCTL2_BA+0x960, 0x1);

	mmio_write_32(UMCTL2_BA+0x540, 0x1);
	mmio_write_32(UMCTL2_BA+0x5f0, 0x1);
	mmio_write_32(UMCTL2_BA+0x6a0, 0x1);
	mmio_write_32(UMCTL2_BA+0x750, 0x1);
	mmio_write_32(UMCTL2_BA+0x800, 0x1);

	/* DDR control register clock gating enable */
	mmio_write_32(SYS_BA + 0x70,
		      (mmio_read_32(SYS_BA + 0x70) & ~0x00800000));
	mmio_write_32(CLK_BA + 0x04, 0x35);

	/* Restore CLK_CLKSEL0. */
	mmio_write_32(CLK_BA + 0x18, clk_sel0);
}
