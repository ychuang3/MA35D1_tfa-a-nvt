/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
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


/* DDR3_256MB_1066MBPS_WINBOND_INIT_BY_DDR32PHY */
void ma35d1_wb_ddr3_256mb(void)
{

	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000001);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000001);

	//polling to 0x00000000
	//while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000000);

	//set MSTR
	outp32((void *)UMCTL2_BA + 0x000,0x01040001);

	//set MRCTRL0
	outp32((void *)UMCTL2_BA + 0x010,0x0000d010);

	//set MRCTRL1
	outp32((void *)UMCTL2_BA + 0x014,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000000);  //add

	//set PWRTMG
	outp32((void *)UMCTL2_BA + 0x034,0x00400010);  //mod 20200421

	//set HWLPCTL
	outp32((void *)UMCTL2_BA + 0x038,0x000a0003);  //mod 20200121

	//set RFSHCTL0
	outp32((void *)UMCTL2_BA + 0x050,0x00210000);

	//set RFSHCTL1
	outp32((void *)UMCTL2_BA + 0x054,0x003c003c);

	//set RFSHCTL3
	outp32((void *)UMCTL2_BA + 0x060,0x00000000);  //mod 20191211

	//set RFSHTMG
	outp32((void *)UMCTL2_BA + 0x064,0x0010002b);  //mod 20210106

	//set CRCPARCTL0
	outp32((void *)UMCTL2_BA + 0x0c0,0x00000000);

	//set INIT0
	outp32((void *)UMCTL2_BA + 0x0d0,0x40020083);  //mod 20200505

	//set INIT1
	outp32((void *)UMCTL2_BA + 0x0d4,0x00350002);  //mod 20200505

	//set INIT3
	outp32((void *)UMCTL2_BA + 0x0dc,0x1b400006);  //mod 20201109

	//set INIT4
	outp32((void *)UMCTL2_BA + 0x0e0,0x00480000);  //mod 20200822

	//set INIT5
	outp32((void *)UMCTL2_BA + 0x0e4,0x00090000);

	//set DIMMCTL
	outp32((void *)UMCTL2_BA + 0x0f0,0x00000000);

	//set RANKCTL
	outp32((void *)UMCTL2_BA + 0x0f4,0x0000032f);

	//set DRAMTMG0
	outp32((void *)UMCTL2_BA + 0x100,0x090d040a);  //mod 20210108

	//set DRAMTMG1
	outp32((void *)UMCTL2_BA + 0x104,0x0003020e);  //mod 20200103

	//set DRAMTMG2
	outp32((void *)UMCTL2_BA + 0x108,0x00000408);  //mod 20200103

	//set DRAMTMG3
	outp32((void *)UMCTL2_BA + 0x10c,0x00003007);  //mod 20201208

	//set DRAMTMG4
	outp32((void *)UMCTL2_BA + 0x110,0x04020205);  //mod 20200103

	//set DRAMTMG5
	outp32((void *)UMCTL2_BA + 0x114,0x03030202);  //mod 20200103

	//set DRAMTMG8
	outp32((void *)UMCTL2_BA + 0x120,0x00000a02);  //mod 20200506

	//set DRAMTMG15
	outp32((void *)UMCTL2_BA + 0x13c,0x80000032);  //mod 20200225

	//set ZQCTL0
	outp32((void *)UMCTL2_BA + 0x180,0x00800020);

	//set ZQCTL1
	outp32((void *)UMCTL2_BA + 0x184,0x00000100);  //mod 20200505

	//set DFITMG0
	outp32((void *)UMCTL2_BA + 0x190,0x04020101);

	//set DFITMG1
	outp32((void *)UMCTL2_BA + 0x194,0x00060101);

	//set DFILPCFG0
#ifdef DFI_DDR_PHY_LP
	outp32((void *)UMCTL2_BA + 0x198,0x0700b131);  //mod 20200417
#else
	outp32((void *)UMCTL2_BA + 0x198,0x0700b030);  //mod 20200825
#endif

	//set DFIUPD0
	outp32((void *)UMCTL2_BA + 0x1a0,0x00400005);  //mod 20200303

	//set DFIUPD1
	outp32((void *)UMCTL2_BA + 0x1a4,0x00170066);  //mod 20200425

	//set DFIUPD2
	outp32((void *)UMCTL2_BA + 0x1a8,0x80000000);  //mod 20200309

	//set DFIMISC
	outp32((void *)UMCTL2_BA + 0x1b0,0x00000011);  //mod

	//set DFIPHYMSTR
	outp32((void *)UMCTL2_BA + 0x1c4,0x00000000);

	//set ADDRMAP0
	outp32((void *)UMCTL2_BA + 0x200,0x00000015);  //mod 20200506

	//set ADDRMAP1
	outp32((void *)UMCTL2_BA + 0x204,0x00080808);

	//set ADDRMAP2
	outp32((void *)UMCTL2_BA + 0x208,0x00000000);

	//set ADDRMAP3
	outp32((void *)UMCTL2_BA + 0x20c,0x00000000);

	//set ADDRMAP4
	outp32((void *)UMCTL2_BA + 0x210,0x00001f1f);

	//set ADDRMAP5
	outp32((void *)UMCTL2_BA + 0x214,0x070f0707);

	//set ADDRMAP6
	outp32((void *)UMCTL2_BA + 0x218,0x0f0f0707);  //mod 20200506

	//set ADDRMAP9
	outp32((void *)UMCTL2_BA + 0x224,0x07070707);

	//set ADDRMAP10
	outp32((void *)UMCTL2_BA + 0x228,0x07070707);

	//set ADDRMAP11
	outp32((void *)UMCTL2_BA + 0x22c,0x00000007);

	//set ODTCFG
	outp32((void *)UMCTL2_BA + 0x240,0x06000608);

	//set ODTMAP
	outp32((void *)UMCTL2_BA + 0x244,0x00000101);  //mod 20200508

	//set SCHED
	outp32((void *)UMCTL2_BA + 0x250,0x00f51f00);

	//set SCHED1
	outp32((void *)UMCTL2_BA + 0x254,0x00000000);

	//set PERFHPR1
	outp32((void *)UMCTL2_BA + 0x25c,0x0f000001);

	//set PERFLPR1
	outp32((void *)UMCTL2_BA + 0x264,0x0f00007f);

	//set PERFWR1
	outp32((void *)UMCTL2_BA + 0x26c,0x0f00007f);

	//set DBG0
	outp32((void *)UMCTL2_BA + 0x300,0x00000000);

	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000000);

	//set DBGCMD
	outp32((void *)UMCTL2_BA + 0x30c,0x00000000);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//set SWCTLSTATIC
	outp32((void *)UMCTL2_BA + 0x328,0x00000000);

	//set POISONCFG
	outp32((void *)UMCTL2_BA + 0x36c,0x00110011);

	//set PCTRL_0
	outp32((void *)UMCTL2_BA + 0x490,0x00000001);

	//set PCTRL_1
	outp32((void *)UMCTL2_BA + 0x540,0x00000000);  //mod 20200323

	//set PCTRL_2
	outp32((void *)UMCTL2_BA + 0x5f0,0x00000000);  //mod 20200323

	//set PCTRL_3
	outp32((void *)UMCTL2_BA + 0x6a0,0x00000000);  //mod 20200323

	//set PCTRL_4
	outp32((void *)UMCTL2_BA + 0x750,0x00000000);  //mod 20200323

	//set PCTRL_5
	outp32((void *)UMCTL2_BA + 0x800,0x00000000);  //mod 20200323

	//set PCTRL_6
	outp32((void *)UMCTL2_BA + 0x8b0,0x00000001);

	//set PCTRL_7
	outp32((void *)UMCTL2_BA + 0x960,0x00000001);

	//set PCCFG
	outp32((void *)UMCTL2_BA + 0x400,0x00000000);

	//set PCFGR_0
	outp32((void *)UMCTL2_BA + 0x404,0x0001500f);  //mod 20200408

	//set PCFGR_1
	outp32((void *)UMCTL2_BA + 0x4b4,0x0001500f);  //mod 20200408

	//set PCFGR_2
	outp32((void *)UMCTL2_BA + 0x564,0x0001500f);  //mod 20200408

	//set PCFGR_3
	outp32((void *)UMCTL2_BA + 0x614,0x0001500f);  //mod 20200408

	//set PCFGR_4
	outp32((void *)UMCTL2_BA + 0x6c4,0x0001500f);  //mod 20200408

	//set PCFGR_5
	outp32((void *)UMCTL2_BA + 0x774,0x0001500f);  //mod 20200408

	//set PCFGR_6
	outp32((void *)UMCTL2_BA + 0x824,0x0001500f);  //mod 20200408

	//set PCFGR_7
	outp32((void *)UMCTL2_BA + 0x8d4,0x0001500f);  //mod 20200408

	//set PCFGW_0
	outp32((void *)UMCTL2_BA + 0x408,0x0000500f);  //mod 20200408

	//set PCFGW_1
	outp32((void *)UMCTL2_BA + 0x4b8,0x0000500f);  //mod 20200408

	//set PCFGW_2
	outp32((void *)UMCTL2_BA + 0x568,0x0000500f);  //mod 20200408

	//set PCFGW_3
	outp32((void *)UMCTL2_BA + 0x618,0x0000500f);  //mod 20200408

	//set PCFGW_4
	outp32((void *)UMCTL2_BA + 0x6c8,0x0000500f);  //mod 20200408

	//set PCFGW_5
	outp32((void *)UMCTL2_BA + 0x778,0x0000500f);  //mod 20200408

	//set PCFGW_6
	outp32((void *)UMCTL2_BA + 0x828,0x0000500f);  //mod 20200408

	//set PCFGW_7
	outp32((void *)UMCTL2_BA + 0x8d8,0x0000500f);  //mod 20200408

	//set SARBASE0
	outp32((void *)UMCTL2_BA + 0xf04,0x00000008);

	//set SARSIZE0
	outp32((void *)UMCTL2_BA + 0xf08,0x00000000);

	//de-assert reset signals of DDR memory controller
	outp32((void *)SYS_BA+0x20,(inp32((void *)SYS_BA+0x20) & 0x8fffffff));
	while( (inp32((void *)SYS_BA+0x20) & 0x20000000) != 0x00000000);

	//=====================================================================
	//                  DDR PHY initialization
	//=====================================================================

	//set DSGCR,addr=16
	outp32((void *)DDRPHY_BA + 0x040,0xf004649f);

	//set PGCR1,addr=3
	outp32((void *)DDRPHY_BA + 0x00c,0x0300c461);  //mod 20200608

	//set PGCR2,addr=35
	outp32((void *)DDRPHY_BA + 0x08c,0x00f0027f);  //mod 20210106

	//set PTR0,addr=7
	outp32((void *)DDRPHY_BA + 0x01c,0x0c806403);  //mod 20200306 for pclk=100mhz

	//set PTR1,addr=8
	outp32((void *)DDRPHY_BA + 0x020,0x27100385);  //mod 20200306 for pclk=100mhz

	//set PTR2,addr=9
	outp32((void *)DDRPHY_BA + 0x024,0x00083def);

	//set PTR3,addr=10
	outp32((void *)DDRPHY_BA + 0x028,0x05b4111d);  //mod 20200506

	//set PTR4,addr=11
	outp32((void *)DDRPHY_BA + 0x02c,0x0801a072);  //mod 20200508

	//set MR0,addr=21
	outp32((void *)DDRPHY_BA + 0x054,0x00001b40);  //mod

	//set MR1,addr=22
	outp32((void *)DDRPHY_BA + 0x058,0x00000006);  //mod 20201109

	//set MR2,addr=23
	outp32((void *)DDRPHY_BA + 0x05c,0x00000048);  //mod 20200822

	//set MR3,addr=24
	outp32((void *)DDRPHY_BA + 0x060,0x00000000);

	//set DTPR0,addr=18
	outp32((void *)DDRPHY_BA + 0x048,0x71568855);

	//set DTPR1,addr=19
	outp32((void *)DDRPHY_BA + 0x04c,0x2282b32a);  //mod 20201208

	//set DTPR2,addr=20
	outp32((void *)DDRPHY_BA + 0x050,0x30023e00);

	//set ZQ0CR1,addr=97
	outp32((void *)DDRPHY_BA + 0x184,0x0000105d);  //mod 20201109

	//polling PGSR0 (addr=4) to 0x0000000f
	while((inp32((void *)DDRPHY_BA + 0x010) & 0x0000000f) != 0x0000000f);

	//set DCR,addr=17
	outp32((void *)DDRPHY_BA + 0x044,0x0000040b);

	//set DTCR,addr=26
	outp32((void *)DDRPHY_BA + 0x068,0x91003587);  //mod 20200820

	//set PIR,addr=1
	outp32((void *)DDRPHY_BA + 0x004,0x0000ff81);

	//polling PGSR0 (addr=4) to 0xb0000fff
	while((inp32((void *)DDRPHY_BA + 0x010) & 0xffffffff) != 0xb0000fff);

	//polling MCTL2 STAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000001);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x0000000b);  //mod 20191213

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//polling MCTL2 SWSTAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x324) & 0x00000001) != 0x00000001);

}

/* DDR3_512MB_1066MBPS_WINBOND_INIT_BY_DDR32PHY */
void ma35d1_wb_ddr3_512mb(void)
{
	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000001);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000001);  //mod

	//polling to 0x00000000
	//while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000000);

	//set MSTR
	outp32((void *)UMCTL2_BA + 0x000,0x01040001);  //mod 20200507

	//set MRCTRL0
	outp32((void *)UMCTL2_BA + 0x010,0x0000d010);  //mod 20200507

	//set MRCTRL1
	outp32((void *)UMCTL2_BA + 0x014,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000000);  //add

	//set PWRTMG
	outp32((void *)UMCTL2_BA + 0x034,0x00400010);  //mod 20200421

	//set HWLPCTL
	outp32((void *)UMCTL2_BA + 0x038,0x000a0003);  //mod 20200121

	//set RFSHCTL0
	outp32((void *)UMCTL2_BA + 0x050,0x00210000);

	//set RFSHCTL1
	outp32((void *)UMCTL2_BA + 0x054,0x003c003c);

	//set RFSHCTL3
	outp32((void *)UMCTL2_BA + 0x060,0x00000000);  //mod 20191211

	//set RFSHTMG
	outp32((void *)UMCTL2_BA + 0x064,0x00100046);  //mod 20210106

	//set CRCPARCTL0
	outp32((void *)UMCTL2_BA + 0x0c0,0x00000000);

	//set INIT0
	outp32((void *)UMCTL2_BA + 0x0d0,0x40020083);  //mod 20200505

	//set INIT1
	outp32((void *)UMCTL2_BA + 0x0d4,0x00350002);  //mod 20200505

	//set INIT3
	outp32((void *)UMCTL2_BA + 0x0dc,0x1b400006);  //mod 20201109

	//set INIT4
	outp32((void *)UMCTL2_BA + 0x0e0,0x00480000);  //mod 20200822

	//set INIT5
	outp32((void *)UMCTL2_BA + 0x0e4,0x00090000);

	//set DIMMCTL
	outp32((void *)UMCTL2_BA + 0x0f0,0x00000000);

	//set RANKCTL
	outp32((void *)UMCTL2_BA + 0x0f4,0x0000032f);

	//set DRAMTMG0
	outp32((void *)UMCTL2_BA + 0x100,0x090d040a);  //mod 20210108

	//set DRAMTMG1
	outp32((void *)UMCTL2_BA + 0x104,0x0003020e);  //mod 20200103

	//set DRAMTMG2
	outp32((void *)UMCTL2_BA + 0x108,0x00000408);  //mod 20200103

	//set DRAMTMG3
	outp32((void *)UMCTL2_BA + 0x10c,0x00003007);  //mod 20201208

	//set DRAMTMG4
	outp32((void *)UMCTL2_BA + 0x110,0x04020205);  //mod 20200103

	//set DRAMTMG5
	outp32((void *)UMCTL2_BA + 0x114,0x03030202);  //mod 20200103

	//set DRAMTMG8
	outp32((void *)UMCTL2_BA + 0x120,0x00000a04);

	//set DRAMTMG15
	outp32((void *)UMCTL2_BA + 0x13c,0x80000032);  //mod 20200225

	//set ZQCTL0
	outp32((void *)UMCTL2_BA + 0x180,0x00800020);

	//set ZQCTL1
	outp32((void *)UMCTL2_BA + 0x184,0x00000100);  //mod 20200505

	//set DFITMG0
	outp32((void *)UMCTL2_BA + 0x190,0x04020101);

	//set DFITMG1
	outp32((void *)UMCTL2_BA + 0x194,0x00060101);

	//set DFILPCFG0
#ifdef DFI_DDR_PHY_LP
	outp32((void *)UMCTL2_BA + 0x198,0x0700b131);  //mod 20200417
#else
	outp32((void *)UMCTL2_BA + 0x198,0x0700b030);  //mod 20200825
#endif

	//set DFIUPD0
	outp32((void *)UMCTL2_BA + 0x1a0,0x00400005);  //mod 20200303

	//set DFIUPD1
	outp32((void *)UMCTL2_BA + 0x1a4,0x00170066);  //mod 20200425

	//set DFIUPD2
	outp32((void *)UMCTL2_BA + 0x1a8,0x80000000);  //mod 20200309

	//set DFIMISC
	outp32((void *)UMCTL2_BA + 0x1b0,0x00000011);  //mod

	//set DFIPHYMSTR
	outp32((void *)UMCTL2_BA + 0x1c4,0x00000000);

	//set ADDRMAP0
	outp32((void *)UMCTL2_BA + 0x200,0x0000001f);  //mod 20200507

	//set ADDRMAP1
	outp32((void *)UMCTL2_BA + 0x204,0x00080808);

	//set ADDRMAP2
	outp32((void *)UMCTL2_BA + 0x208,0x00000000);

	//set ADDRMAP3
	outp32((void *)UMCTL2_BA + 0x20c,0x00000000);

	//set ADDRMAP4
	outp32((void *)UMCTL2_BA + 0x210,0x00001f1f);

	//set ADDRMAP5
	outp32((void *)UMCTL2_BA + 0x214,0x070f0707);

	//set ADDRMAP6
	outp32((void *)UMCTL2_BA + 0x218,0x0f070707);

	//set ADDRMAP9
	outp32((void *)UMCTL2_BA + 0x224,0x07070707);

	//set ADDRMAP10
	outp32((void *)UMCTL2_BA + 0x228,0x07070707);

	//set ADDRMAP11
	outp32((void *)UMCTL2_BA + 0x22c,0x00000007);

	//set ODTCFG
	outp32((void *)UMCTL2_BA + 0x240,0x06000608);

	//set ODTMAP
	outp32((void *)UMCTL2_BA + 0x244,0x00000101);  //mod 20200508

	//set SCHED
	outp32((void *)UMCTL2_BA + 0x250,0x00f51f00);

	//set SCHED1
	outp32((void *)UMCTL2_BA + 0x254,0x00000000);

	//set PERFHPR1
	outp32((void *)UMCTL2_BA + 0x25c,0x0f000001);

	//set PERFLPR1
	outp32((void *)UMCTL2_BA + 0x264,0x0f00007f);

	//set PERFWR1
	outp32((void *)UMCTL2_BA + 0x26c,0x0f00007f);

	//set DBG0
	outp32((void *)UMCTL2_BA + 0x300,0x00000000);

	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000000);

	//set DBGCMD
	outp32((void *)UMCTL2_BA + 0x30c,0x00000000);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//set SWCTLSTATIC
	outp32((void *)UMCTL2_BA + 0x328,0x00000000);

	//set POISONCFG
	outp32((void *)UMCTL2_BA + 0x36c,0x00110011);

	//set PCTRL_0
	outp32((void *)UMCTL2_BA + 0x490,0x00000001);

	//set PCTRL_1
	outp32((void *)UMCTL2_BA + 0x540,0x00000000);  //mod 20200323

	//set PCTRL_2
	outp32((void *)UMCTL2_BA + 0x5f0,0x00000000);  //mod 20200323

	//set PCTRL_3
	outp32((void *)UMCTL2_BA + 0x6a0,0x00000000);  //mod 20200323

	//set PCTRL_4
	outp32((void *)UMCTL2_BA + 0x750,0x00000000);  //mod 20200323

	//set PCTRL_5
	outp32((void *)UMCTL2_BA + 0x800,0x00000000);  //mod 20200323

	//set PCTRL_6
	outp32((void *)UMCTL2_BA + 0x8b0,0x00000001);

	//set PCTRL_7
	outp32((void *)UMCTL2_BA + 0x960,0x00000001);

	//set PCCFG
	outp32((void *)UMCTL2_BA + 0x400,0x00000000);

	//set PCFGR_0
	outp32((void *)UMCTL2_BA + 0x404,0x0001500f);  //mod 20200408

	//set PCFGR_1
	outp32((void *)UMCTL2_BA + 0x4b4,0x0001500f);  //mod 20200408

	//set PCFGR_2
	outp32((void *)UMCTL2_BA + 0x564,0x0001500f);  //mod 20200408

	//set PCFGR_3
	outp32((void *)UMCTL2_BA + 0x614,0x0001500f);  //mod 20200408

	//set PCFGR_4
	outp32((void *)UMCTL2_BA + 0x6c4,0x0001500f);  //mod 20200408

	//set PCFGR_5
	outp32((void *)UMCTL2_BA + 0x774,0x0001500f);  //mod 20200408

	//set PCFGR_6
	outp32((void *)UMCTL2_BA + 0x824,0x0001500f);  //mod 20200408

	//set PCFGR_7
	outp32((void *)UMCTL2_BA + 0x8d4,0x0001500f);  //mod 20200408

	//set PCFGW_0
	outp32((void *)UMCTL2_BA + 0x408,0x0000500f);  //mod 20200408

	//set PCFGW_1
	outp32((void *)UMCTL2_BA + 0x4b8,0x0000500f);  //mod 20200408

	//set PCFGW_2
	outp32((void *)UMCTL2_BA + 0x568,0x0000500f);  //mod 20200408

	//set PCFGW_3
	outp32((void *)UMCTL2_BA + 0x618,0x0000500f);  //mod 20200408

	//set PCFGW_4
	outp32((void *)UMCTL2_BA + 0x6c8,0x0000500f);  //mod 20200408

	//set PCFGW_5
	outp32((void *)UMCTL2_BA + 0x778,0x0000500f);  //mod 20200408

	//set PCFGW_6
	outp32((void *)UMCTL2_BA + 0x828,0x0000500f);  //mod 20200408

	//set PCFGW_7
	outp32((void *)UMCTL2_BA + 0x8d8,0x0000500f);  //mod 20200408

	//set SARBASE0
	outp32((void *)UMCTL2_BA + 0xf04,0x00000008);

	//set SARSIZE0
	outp32((void *)UMCTL2_BA + 0xf08,0x00000001);  //mod 20200507

	//de-assert reset signals of DDR memory controller
	outp32((void *)SYS_BA+0x20,(inp32((void *)SYS_BA+0x20) & 0x8fffffff));
	while( (inp32((void *)SYS_BA+0x20) & 0x20000000) != 0x00000000);

	//=====================================================================
	//                  DDR PHY initialization
	//=====================================================================

	//set DSGCR,addr=16
	outp32((void *)DDRPHY_BA + 0x040,0xf004649f);

	//set PGCR1,addr=3
	outp32((void *)DDRPHY_BA + 0x00c,0x0300c461);  //mod 20200608

	//set PGCR2,addr=35
	outp32((void *)DDRPHY_BA + 0x08c,0x00f0027f);  //mod 20210106

	//set PTR0,addr=7
	outp32((void *)DDRPHY_BA + 0x01c,0x0c806403);  //mod 20200306 for pclk=100mhz

	//set PTR1,addr=8
	outp32((void *)DDRPHY_BA + 0x020,0x27100385);  //mod 20200306 for pclk=100mhz

	//set PTR2,addr=9
	outp32((void *)DDRPHY_BA + 0x024,0x00083def);

	//set PTR3,addr=10
	outp32((void *)DDRPHY_BA + 0x028,0x0904111d);  //mod 20200504

	//set PTR4,addr=11
	outp32((void *)DDRPHY_BA + 0x02c,0x0801a072);  //mod 20200508

	//set MR0,addr=21
	outp32((void *)DDRPHY_BA + 0x054,0x00001b40);  //mod

	//set MR1,addr=22
	outp32((void *)DDRPHY_BA + 0x058,0x00000006);  //mod 20201109

	//set MR2,addr=23
	outp32((void *)DDRPHY_BA + 0x05c,0x00000048);  //mod 20200822

	//set MR3,addr=24
	outp32((void *)DDRPHY_BA + 0x060,0x00000000);

	//set DTPR0,addr=18
	outp32((void *)DDRPHY_BA + 0x048,0x71568855);

	//set DTPR1,addr=19
	outp32((void *)DDRPHY_BA + 0x04c,0x2284632a);  //mod 20201208

	//set DTPR2,addr=20
	outp32((void *)DDRPHY_BA + 0x050,0x30023e00);

	//set ZQ0CR1,addr=97
	outp32((void *)DDRPHY_BA + 0x184,0x0000105d);  //mod 20201109

	//polling PGSR0 (addr=4) to 0x0000000f
	while((inp32((void *)DDRPHY_BA + 0x010) & 0x0000000f) != 0x0000000f);

	//set DCR,addr=17
	outp32((void *)DDRPHY_BA + 0x044,0x0000040b);

	//set DTCR,addr=26
	outp32((void *)DDRPHY_BA + 0x068,0x91003587);  //mod 20200820

	//set PIR,addr=1
	outp32((void *)DDRPHY_BA + 0x004,0x0000ff81);

	//polling PGSR0 (addr=4) to 0xb0000fff
	while((inp32((void *)DDRPHY_BA + 0x010) & 0xffffffff) != 0xb0000fff);

	//polling MCTL2 STAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000001);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x0000000b);  //mod 20191213

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//polling MCTL2 SWSTAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x324) & 0x00000001) != 0x00000001);

}

/* #define DDR3_1GB_1066MBPS_MICRON_INIT_BY_DDR32PHY */
void ma35d1_mt_ddr3_1gb(void)
{
	INFO("MICRON DDR3 1GB\n");

	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000001);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000001);  //mod

	//polling to 0x00000000
	//while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000000);

	//set MSTR
	outp32((void *)UMCTL2_BA + 0x000,0x01040001);  //mod 20201204

	//set MRCTRL0
	outp32((void *)UMCTL2_BA + 0x010,0x0000d010);  //mod 20201204

	//set MRCTRL1
	outp32((void *)UMCTL2_BA + 0x014,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000000);  //add

	//set PWRTMG
	outp32((void *)UMCTL2_BA + 0x034,0x00400010);  //mod 20200421

	//set HWLPCTL
	outp32((void *)UMCTL2_BA + 0x038,0x000a0003);  //mod 20200121

	//set RFSHCTL0
	outp32((void *)UMCTL2_BA + 0x050,0x00210000);

	//set RFSHCTL1
	outp32((void *)UMCTL2_BA + 0x054,0x003c003c);

	//set RFSHCTL3
	outp32((void *)UMCTL2_BA + 0x060,0x00000000);  //mod 20191211

	//set RFSHTMG
	outp32((void *)UMCTL2_BA + 0x064,0x0020005e);  //mod 20210106

	//set CRCPARCTL0
	outp32((void *)UMCTL2_BA + 0x0c0,0x00000000);

	//set INIT0
	outp32((void *)UMCTL2_BA + 0x0d0,0x40020083);  //mod 20200505

	//set INIT1
	outp32((void *)UMCTL2_BA + 0x0d4,0x00350002);  //mod 20200505

	//set INIT3
	outp32((void *)UMCTL2_BA + 0x0dc,0x1b400006);  //mod 20201109

	//set INIT4
	outp32((void *)UMCTL2_BA + 0x0e0,0x00480000);  //mod 20200822

	//set INIT5
	outp32((void *)UMCTL2_BA + 0x0e4,0x00090000);

	//set DIMMCTL
	outp32((void *)UMCTL2_BA + 0x0f0,0x00000000);

	//set RANKCTL
	outp32((void *)UMCTL2_BA + 0x0f4,0x0000032f);

	//set DRAMTMG0
	outp32((void *)UMCTL2_BA + 0x100,0x090e080a);  //mod 20210108

	//set DRAMTMG1
	outp32((void *)UMCTL2_BA + 0x104,0x0003020e);

	//set DRAMTMG2
	outp32((void *)UMCTL2_BA + 0x108,0x00000407);

	//set DRAMTMG3
	outp32((void *)UMCTL2_BA + 0x10c,0x00003007);  //mod 20201208

	//set DRAMTMG4
	outp32((void *)UMCTL2_BA + 0x110,0x04020305);  //mod 20200529

	//set DRAMTMG5
	outp32((void *)UMCTL2_BA + 0x114,0x03030202);

	//set DRAMTMG8
	outp32((void *)UMCTL2_BA + 0x120,0x00000a04);

	//set DRAMTMG15
	outp32((void *)UMCTL2_BA + 0x13c,0x80000032);  //mod 20200225

	//set ZQCTL0
	outp32((void *)UMCTL2_BA + 0x180,0x00800020);

	//set ZQCTL1
	outp32((void *)UMCTL2_BA + 0x184,0x00000100);  //mod 20200505

	//set DFITMG0
	outp32((void *)UMCTL2_BA + 0x190,0x04020101);

	//set DFITMG1
	outp32((void *)UMCTL2_BA + 0x194,0x00060101);

	//set DFILPCFG0
#ifdef DFI_DDR_PHY_LP
	outp32((void *)UMCTL2_BA + 0x198,0x0700b131);  //mod 20200417
#else
	outp32((void *)UMCTL2_BA + 0x198,0x0700b030);  //mod 20200825
#endif

	//set DFIUPD0
	outp32((void *)UMCTL2_BA + 0x1a0,0x00400005);  //mod 20200303

	//set DFIUPD1
	outp32((void *)UMCTL2_BA + 0x1a4,0x00170066);  //mod 20200425

	//set DFIUPD2
	outp32((void *)UMCTL2_BA + 0x1a8,0x80000000);  //mod 20200309

	//set DFIMISC
	outp32((void *)UMCTL2_BA + 0x1b0,0x00000011);  //mod

	//set DFIPHYMSTR
	outp32((void *)UMCTL2_BA + 0x1c4,0x00000000);

	//set ADDRMAP0
	outp32((void *)UMCTL2_BA + 0x200,0x0000001f);  //mod 20201204

	//set ADDRMAP1
	outp32((void *)UMCTL2_BA + 0x204,0x00080808);

	//set ADDRMAP2
	outp32((void *)UMCTL2_BA + 0x208,0x00000000);

	//set ADDRMAP3
	outp32((void *)UMCTL2_BA + 0x20c,0x00000000);

	//set ADDRMAP4
	outp32((void *)UMCTL2_BA + 0x210,0x00001f1f);

	//set ADDRMAP5
	outp32((void *)UMCTL2_BA + 0x214,0x070f0707);

	//set ADDRMAP6
	outp32((void *)UMCTL2_BA + 0x218,0x07070707);  //mod 20200117,for 1G bytes

	//set ADDRMAP9
	outp32((void *)UMCTL2_BA + 0x224,0x07070707);

	//set ADDRMAP10
	outp32((void *)UMCTL2_BA + 0x228,0x07070707);

	//set ADDRMAP11
	outp32((void *)UMCTL2_BA + 0x22c,0x00000007);

	//set ODTCFG
	outp32((void *)UMCTL2_BA + 0x240,0x06000608);

	//set ODTMAP
	outp32((void *)UMCTL2_BA + 0x244,0x00000101);  //mod 20201204

	//set SCHED
	outp32((void *)UMCTL2_BA + 0x250,0x00f51f00);

	//set SCHED1
	outp32((void *)UMCTL2_BA + 0x254,0x00000000);

	//set PERFHPR1
	outp32((void *)UMCTL2_BA + 0x25c,0x0f000001);

	//set PERFLPR1
	outp32((void *)UMCTL2_BA + 0x264,0x0f00007f);

	//set PERFWR1
	outp32((void *)UMCTL2_BA + 0x26c,0x0f00007f);

	//set DBG0
	outp32((void *)UMCTL2_BA + 0x300,0x00000000);

	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000000);

	//set DBGCMD
	outp32((void *)UMCTL2_BA + 0x30c,0x00000000);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//set SWCTLSTATIC
	outp32((void *)UMCTL2_BA + 0x328,0x00000000);

	//set POISONCFG
	outp32((void *)UMCTL2_BA + 0x36c,0x00110011);

	//set PCTRL_0
	outp32((void *)UMCTL2_BA + 0x490,0x00000001);

	//set PCTRL_1
	outp32((void *)UMCTL2_BA + 0x540,0x00000000);  //mod 20200323

	//set PCTRL_2
	outp32((void *)UMCTL2_BA + 0x5f0,0x00000000);  //mod 20200323

	//set PCTRL_3
	outp32((void *)UMCTL2_BA + 0x6a0,0x00000000);  //mod 20200323

	//set PCTRL_4
	outp32((void *)UMCTL2_BA + 0x750,0x00000000);  //mod 20200323

	//set PCTRL_5
	outp32((void *)UMCTL2_BA + 0x800,0x00000000);  //mod 20200323

	//set PCTRL_6
	outp32((void *)UMCTL2_BA + 0x8b0,0x00000001);

	//set PCTRL_7
	outp32((void *)UMCTL2_BA + 0x960,0x00000001);

	//set PCCFG
	outp32((void *)UMCTL2_BA + 0x400,0x00000000);

	//set PCFGR_0
	outp32((void *)UMCTL2_BA + 0x404,0x0001500f);  //mod 20200408

	//set PCFGR_1
	outp32((void *)UMCTL2_BA + 0x4b4,0x0001500f);  //mod 20200408

	//set PCFGR_2
	outp32((void *)UMCTL2_BA + 0x564,0x0001500f);  //mod 20200408

	//set PCFGR_3
	outp32((void *)UMCTL2_BA + 0x614,0x0001500f);  //mod 20200408

	//set PCFGR_4
	outp32((void *)UMCTL2_BA + 0x6c4,0x0001500f);  //mod 20200408

	//set PCFGR_5
	outp32((void *)UMCTL2_BA + 0x774,0x0001500f);  //mod 20200408

	//set PCFGR_6
	outp32((void *)UMCTL2_BA + 0x824,0x0001500f);  //mod 20200408

	//set PCFGR_7
	outp32((void *)UMCTL2_BA + 0x8d4,0x0001500f);  //mod 20200408

	//set PCFGW_0
	outp32((void *)UMCTL2_BA + 0x408,0x0000500f);  //mod 20200408

	//set PCFGW_1
	outp32((void *)UMCTL2_BA + 0x4b8,0x0000500f);  //mod 20200408

	//set PCFGW_2
	outp32((void *)UMCTL2_BA + 0x568,0x0000500f);  //mod 20200408

	//set PCFGW_3
	outp32((void *)UMCTL2_BA + 0x618,0x0000500f);  //mod 20200408

	//set PCFGW_4
	outp32((void *)UMCTL2_BA + 0x6c8,0x0000500f);  //mod 20200408

	//set PCFGW_5
	outp32((void *)UMCTL2_BA + 0x778,0x0000500f);  //mod 20200408

	//set PCFGW_6
	outp32((void *)UMCTL2_BA + 0x828,0x0000500f);  //mod 20200408

	//set PCFGW_7
	outp32((void *)UMCTL2_BA + 0x8d8,0x0000500f);  //mod 20200408

	//set SARBASE0
	outp32((void *)UMCTL2_BA + 0xf04,0x00000008);

	//set SARSIZE0
	outp32((void *)UMCTL2_BA + 0xf08,0x00000003);  //mod 20201204

	//de-assert reset signals of DDR memory controller
	outp32((void *)SYS_BA+0x20,(inp32((void *)SYS_BA+0x20) & 0x8fffffff));
	while( (inp32((void *)SYS_BA+0x20) & 0x20000000) != 0x00000000);

	//=====================================================================
	//                  DDR PHY initialization
	//=====================================================================

	//set DSGCR,addr=16
	outp32((void *)DDRPHY_BA + 0x040,0xf004649f);

	//set PGCR1,addr=3
	outp32((void *)DDRPHY_BA + 0x00c,0x0300c461);  //mod 20200608

	//set PGCR2,addr=35
	outp32((void *)DDRPHY_BA + 0x08c,0x00f0068e);  //mod 20210106

	//set PTR0,addr=7
	outp32((void *)DDRPHY_BA + 0x01c,0x0c806403);  //mod 20200306 for pclk=100mhz

	//set PTR1,addr=8
	outp32((void *)DDRPHY_BA + 0x020,0x27100385);  //mod 20200306 for pclk=100mhz

	//set PTR2,addr=9
	outp32((void *)DDRPHY_BA + 0x024,0x00083def);

	//set PTR3,addr=10
	outp32((void *)DDRPHY_BA + 0x028,0x0c04111d);  //mod 20200511

	//set PTR4,addr=11
	outp32((void *)DDRPHY_BA + 0x02c,0x0801a072);  //mod 20200511

	//set MR0,addr=21
	outp32((void *)DDRPHY_BA + 0x054,0x00001b40);  //mod

	//set MR1,addr=22
	outp32((void *)DDRPHY_BA + 0x058,0x00000006);  //mod 20201109

	//set MR2,addr=23
	outp32((void *)DDRPHY_BA + 0x05c,0x00000048);  //mod 20200822

	//set MR3,addr=24
	outp32((void *)DDRPHY_BA + 0x060,0x00000000);

	//set DTPR0,addr=18
	outp32((void *)DDRPHY_BA + 0x048,0x75959955);  //mod 20200511

	//set DTPR1,addr=19
	outp32((void *)DDRPHY_BA + 0x04c,0x2285e36a);  //mod 20201208

	//set DTPR2,addr=20
	outp32((void *)DDRPHY_BA + 0x050,0x30023e00);

	//set ZQ0CR1,addr=97
	outp32((void *)DDRPHY_BA + 0x184,0x0000105d);  //mod 20201109

	//polling PGSR0 (addr=4) to 0x0000000f
	while((inp32((void *)DDRPHY_BA + 0x010) & 0x0000000f) != 0x0000000f);

	//set DCR,addr=17
	outp32((void *)DDRPHY_BA + 0x044,0x0000040b);

	//set DTCR,addr=26
	outp32((void *)DDRPHY_BA + 0x068,0x91003587);  //mod 20201204

	//set PIR,addr=1
	outp32((void *)DDRPHY_BA + 0x004,0x0000ff81);

	//polling PGSR0 (addr=4) to 0xb0000fff
	while((inp32((void *)DDRPHY_BA + 0x010) & 0xffffffff) != 0xb0000fff);

	//polling MCTL2 STAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000001);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x0000000b);  //mod 20191213

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//polling MCTL2 SWSTAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x324) & 0x00000001) != 0x00000001);

}

/* #define DDR2_128MB_1066MBPS_WINBOND_INIT_BY_DDR32PHY */
void ma35d1_wb_ddr2_128mb(void)
{

	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000001);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000001);  //mod

	//polling to 0x00000000
	//while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000000);

	//set MSTR
	outp32((void *)UMCTL2_BA + 0x000,0x01040000);

	//set MRCTRL0
	outp32((void *)UMCTL2_BA + 0x010,0x00001010);  //mod 20200511

	//set MRCTRL1
	outp32((void *)UMCTL2_BA + 0x014,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x00000000);  //add

	//set PWRTMG
	outp32((void *)UMCTL2_BA + 0x034,0x00400010);  //mod 20200421

	//set HWLPCTL
	outp32((void *)UMCTL2_BA + 0x038,0x000a0003);  //mod 20200121

	//set RFSHCTL0
	outp32((void *)UMCTL2_BA + 0x050,0x00210000);

	//set RFSHCTL1
	outp32((void *)UMCTL2_BA + 0x054,0x003c003c);

	//set RFSHCTL3
	outp32((void *)UMCTL2_BA + 0x060,0x00000000);  //mod 20191211

	//set RFSHTMG
	outp32((void *)UMCTL2_BA + 0x064,0x00100022);  //mod 20210106

	//set CRCPARCTL0
	outp32((void *)UMCTL2_BA + 0x0c0,0x00000000);

	//set INIT0
	outp32((void *)UMCTL2_BA + 0x0d0,0x40020035);  //mod 20200309

	//set INIT1
	outp32((void *)UMCTL2_BA + 0x0d4,0x00010002);

	//set INIT3
	outp32((void *)UMCTL2_BA + 0x0dc,0x0e730004);  //mod 20200918

	//set INIT4
	outp32((void *)UMCTL2_BA + 0x0e0,0x00000000);

	//set INIT5
	outp32((void *)UMCTL2_BA + 0x0e4,0x00010000);

	//set DIMMCTL
	outp32((void *)UMCTL2_BA + 0x0f0,0x00000000);

	//set RANKCTL
	outp32((void *)UMCTL2_BA + 0x0f4,0x0000031f);

	//set DRAMTMG0
	outp32((void *)UMCTL2_BA + 0x100,0x090c110c);  //mod 20200619

	//set DRAMTMG1
	outp32((void *)UMCTL2_BA + 0x104,0x0003030e);  //mod 20200309

	//set DRAMTMG2
	outp32((void *)UMCTL2_BA + 0x108,0x00000407);  //mod 20200103

	//set DRAMTMG3
	outp32((void *)UMCTL2_BA + 0x10c,0x00001004);

	//set DRAMTMG4
	outp32((void *)UMCTL2_BA + 0x110,0x03010304);  //mod 20200103

	//set DRAMTMG5
	outp32((void *)UMCTL2_BA + 0x114,0x01010202);  //mod 20200103

	//set DRAMTMG8
	outp32((void *)UMCTL2_BA + 0x120,0x00000502);

	//set DRAMTMG15
	outp32((void *)UMCTL2_BA + 0x13c,0x80000032);  //mod 20200225

	//set ZQCTL0
	outp32((void *)UMCTL2_BA + 0x180,0xc0000000);

	//set ZQCTL1
	outp32((void *)UMCTL2_BA + 0x184,0x00074bf0);

	//set DFITMG0
	outp32((void *)UMCTL2_BA + 0x190,0x04020101);

	//set DFITMG1
	outp32((void *)UMCTL2_BA + 0x194,0x00060101);

	//set DFILPCFG0
#ifdef DFI_DDR_PHY_LP
	outp32((void *)UMCTL2_BA + 0x198,0x0700b131);  //mod 20200417
#else
	outp32((void *)UMCTL2_BA + 0x198,0x0700b030);  //mod 20200825
#endif

	//set DFIUPD0
	outp32((void *)UMCTL2_BA + 0x1a0,0x00400005);  //mod 20200303

	//set DFIUPD1
	outp32((void *)UMCTL2_BA + 0x1a4,0x00170066);  //mod 20200425

	//set DFIUPD2
	outp32((void *)UMCTL2_BA + 0x1a8,0x80000000);  //mod 20200309

	//set DFIMISC
	outp32((void *)UMCTL2_BA + 0x1b0,0x00000011);  //mod

	//set DFIPHYMSTR
	outp32((void *)UMCTL2_BA + 0x1c4,0x00000000);

	//set ADDRMAP0
	outp32((void *)UMCTL2_BA + 0x200,0x0000001f);

	//set ADDRMAP1
	outp32((void *)UMCTL2_BA + 0x204,0x00080808);

	//set ADDRMAP2
	outp32((void *)UMCTL2_BA + 0x208,0x00000000);

	//set ADDRMAP3
	outp32((void *)UMCTL2_BA + 0x20c,0x00000000);

	//set ADDRMAP4
	outp32((void *)UMCTL2_BA + 0x210,0x00001f1f);

	//set ADDRMAP5
	outp32((void *)UMCTL2_BA + 0x214,0x070f0707);

	//set ADDRMAP6
	outp32((void *)UMCTL2_BA + 0x218,0x0f0f0f07);

	//set ADDRMAP9
	outp32((void *)UMCTL2_BA + 0x224,0x07070707);

	//set ADDRMAP10
	outp32((void *)UMCTL2_BA + 0x228,0x07070707);

	//set ADDRMAP11
	outp32((void *)UMCTL2_BA + 0x22c,0x00000007);

	//set ODTCFG
	outp32((void *)UMCTL2_BA + 0x240,0x07010708);

	//set ODTMAP
	outp32((void *)UMCTL2_BA + 0x244,0x00000101);  //mod 20200508

	//set SCHED
	outp32((void *)UMCTL2_BA + 0x250,0x00f51f00);

	//set SCHED1
	outp32((void *)UMCTL2_BA + 0x254,0x00000000);

	//set PERFHPR1
	outp32((void *)UMCTL2_BA + 0x25c,0x0f000001);

	//set PERFLPR1
	outp32((void *)UMCTL2_BA + 0x264,0x0f00007f);

	//set PERFWR1
	outp32((void *)UMCTL2_BA + 0x26c,0x0f00007f);

	//set DBG0
	outp32((void *)UMCTL2_BA + 0x300,0x00000000);

	//set DBG1
	outp32((void *)UMCTL2_BA + 0x304,0x00000000);

	//set DBGCMD
	outp32((void *)UMCTL2_BA + 0x30c,0x00000000);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//set SWCTLSTATIC
	outp32((void *)UMCTL2_BA + 0x328,0x00000000);

	//set POISONCFG
	outp32((void *)UMCTL2_BA + 0x36c,0x00110011);

	//set PCTRL_0
	outp32((void *)UMCTL2_BA + 0x490,0x00000001);

	//set PCTRL_1
	outp32((void *)UMCTL2_BA + 0x540,0x00000000);  //mod 20200323

	//set PCTRL_2
	outp32((void *)UMCTL2_BA + 0x5f0,0x00000000);  //mod 20200323

	//set PCTRL_3
	outp32((void *)UMCTL2_BA + 0x6a0,0x00000000);  //mod 20200323

	//set PCTRL_4
	outp32((void *)UMCTL2_BA + 0x750,0x00000000);  //mod 20200323

	//set PCTRL_5
	outp32((void *)UMCTL2_BA + 0x800,0x00000000);  //mod 20200323

	//set PCTRL_6
	outp32((void *)UMCTL2_BA + 0x8b0,0x00000001);

	//set PCTRL_7
	outp32((void *)UMCTL2_BA + 0x960,0x00000001);

	//set PCCFG
	outp32((void *)UMCTL2_BA + 0x400,0x00000000);

	//set PCFGR_0
	outp32((void *)UMCTL2_BA + 0x404,0x0001500f);  //mod 20200408

	//set PCFGR_1
	outp32((void *)UMCTL2_BA + 0x4b4,0x0001500f);  //mod 20200408

	//set PCFGR_2
	outp32((void *)UMCTL2_BA + 0x564,0x0001500f);  //mod 20200408

	//set PCFGR_3
	outp32((void *)UMCTL2_BA + 0x614,0x0001500f);  //mod 20200408

	//set PCFGR_4
	outp32((void *)UMCTL2_BA + 0x6c4,0x0001500f);  //mod 20200408

	//set PCFGR_5
	outp32((void *)UMCTL2_BA + 0x774,0x0001500f);  //mod 20200408

	//set PCFGR_6
	outp32((void *)UMCTL2_BA + 0x824,0x0001500f);  //mod 20200408

	//set PCFGR_7
	outp32((void *)UMCTL2_BA + 0x8d4,0x0001500f);  //mod 20200408

	//set PCFGW_0
	outp32((void *)UMCTL2_BA + 0x408,0x0000500f);  //mod 20200408

	//set PCFGW_1
	outp32((void *)UMCTL2_BA + 0x4b8,0x0000500f);  //mod 20200408

	//set PCFGW_2
	outp32((void *)UMCTL2_BA + 0x568,0x0000500f);  //mod 20200408

	//set PCFGW_3
	outp32((void *)UMCTL2_BA + 0x618,0x0000500f);  //mod 20200408

	//set PCFGW_4
	outp32((void *)UMCTL2_BA + 0x6c8,0x0000500f);  //mod 20200408

	//set PCFGW_5
	outp32((void *)UMCTL2_BA + 0x778,0x0000500f);  //mod 20200408

	//set PCFGW_6
	outp32((void *)UMCTL2_BA + 0x828,0x0000500f);  //mod 20200408

	//set PCFGW_7
	outp32((void *)UMCTL2_BA + 0x8d8,0x0000500f);  //mod 20200408

	//set SARBASE0
	outp32((void *)UMCTL2_BA + 0xf04,0x00000008);

	//set SARSIZE0
	outp32((void *)UMCTL2_BA + 0xf08,0x00000000);

	//de-assert reset signals of DDR memory controller
	outp32((void *)SYS_BA+0x20,(inp32((void *)SYS_BA+0x20) & 0x8fffffff));
	while( (inp32((void *)SYS_BA+0x20) & 0x20000000) != 0x00000000);

	//=====================================================================
	//                  DDR PHY initialization
	//=====================================================================

	//set DSGCR,addr=16
	outp32((void *)DDRPHY_BA + 0x040,0xf004649f);

	//set PGCR1,addr=3
	outp32((void *)DDRPHY_BA + 0x00c,0x0300c461);  //mod 20200608

	//set PGCR2,addr=35
	outp32((void *)DDRPHY_BA + 0x08c,0x00f0027f);  //mod 20210106

	//set PTR0,addr=7
	outp32((void *)DDRPHY_BA + 0x01c,0x0c806403);  //mod 20200306 for pclk=100mhz

	//set PTR1,addr=8
	outp32((void *)DDRPHY_BA + 0x020,0x27100385);  //mod 20200306 for pclk=100mhz

	//set PTR2,addr=9
	outp32((void *)DDRPHY_BA + 0x024,0x00083def);

	//set PTR3,addr=10
	outp32((void *)DDRPHY_BA + 0x028,0x0d61a072);

	//set PTR4,addr=11
	outp32((void *)DDRPHY_BA + 0x02c,0x0559a072);

	//set MR0,addr=21
	outp32((void *)DDRPHY_BA + 0x054,0x00000e73);  //mod 20200918

	//set MR1,addr=22
	outp32((void *)DDRPHY_BA + 0x058,0x00000004);

	//set MR2,addr=23
	outp32((void *)DDRPHY_BA + 0x05c,0x00000000);

	//set DTPR0,addr=18
	outp32((void *)DDRPHY_BA + 0x048,0x71987755);  //mod 20200504

	//set DTPR1,addr=19
	outp32((void *)DDRPHY_BA + 0x04c,0x22822322);

	//set DTPR2,addr=20
	outp32((void *)DDRPHY_BA + 0x050,0x0641a8c8);  //mod 20200309

	//set ZQ0CR1,addr=97
	outp32((void *)DDRPHY_BA + 0x184,0x0000104b);  //mod 20201014

	//polling PGSR0 (addr=4) to 0x0000000f
	while((inp32((void *)DDRPHY_BA + 0x010) & 0x0000000f) != 0x0000000f);

	//set DCR,addr=17
	outp32((void *)DDRPHY_BA + 0x044,0x0000040a);

	//set DTCR,addr=26
	outp32((void *)DDRPHY_BA + 0x068,0x91003587);

	//set PIR,addr=1
	outp32((void *)DDRPHY_BA + 0x004,0x0000f501);

	//polling PGSR0 (addr=4) to 0xb0000f5f
	while((inp32((void *)DDRPHY_BA + 0x010) & 0xffffffff) != 0xb0000f5f);

	//polling MCTL2 STAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000001);

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000000);

	//set PWRCTL
	outp32((void *)UMCTL2_BA + 0x030,0x0000000b);  //mod 20191213

	//set SWCTL
	outp32((void *)UMCTL2_BA + 0x320,0x00000001);

	//polling MCTL2 SWSTAT to 0x00000001
	while((inp32((void *)UMCTL2_BA + 0x324) & 0x00000001) != 0x00000001);

}

/* #define DDR3_1GB_1066MBPS_ISSI_INIT_BY_DDR32PHY */
void ma35d1_issi_ddr3_1gb(void)
{
	/* set DBG1 */
	outp32((void *)UMCTL2_BA + 0x304, 0x00000001);

	/* set PWRCTL */
	outp32((void *)UMCTL2_BA + 0x030, 0x00000001);

	/* polling to 0x00000000 */
	/* while((inp32(UMCTL2_BA + 0x004) & 0x00000003) != 0x00000000); */

	/* set MSTR */
	outp32((void *)UMCTL2_BA + 0x000, 0x01040001);

	/* set MRCTRL0 */
	outp32((void *)UMCTL2_BA + 0x010, 0x0000d010);

	/* set MRCTRL1 */
	outp32((void *)UMCTL2_BA + 0x014, 0x00000000);

	/* set PWRCTL */
	outp32((void *)UMCTL2_BA + 0x030, 0x00000000);

	/* set PWRTMG */
	outp32((void *)UMCTL2_BA + 0x034, 0x00400010);

	/* set HWLPCTL */
	outp32((void *)UMCTL2_BA + 0x038, 0x000a0003);

	/* set RFSHCTL0 */
	outp32((void *)UMCTL2_BA + 0x050, 0x00210000);

	/* set RFSHCTL1 */
	outp32((void *)UMCTL2_BA + 0x054, 0x003c003c);

	/* set RFSHCTL3 */
	outp32((void *)UMCTL2_BA + 0x060, 0x00000000);

	/* set RFSHTMG */
	outp32((void *)UMCTL2_BA + 0x064, 0x0010005e); /* mod 2021916 */

	/* set CRCPARCTL0 */
	outp32((void *)UMCTL2_BA + 0x0c0, 0x00000000);

	/* set INIT0 */
	outp32((void *)UMCTL2_BA + 0x0d0, 0x40020083);

	/* set INIT1 */
	outp32((void *)UMCTL2_BA + 0x0d4, 0x00350002);

	/* set INIT3 */
	outp32((void *)UMCTL2_BA + 0x0dc, 0x1b400006);

	/* set INIT4 */
	outp32((void *)UMCTL2_BA + 0x0e0, 0x00480000);

	/* set INIT5 */
	outp32((void *)UMCTL2_BA + 0x0e4, 0x00090000);

	/* set DIMMCTL */
	outp32((void *)UMCTL2_BA + 0x0f0, 0x00000000);

	/* set RANKCTL */
	outp32((void *)UMCTL2_BA + 0x0f4, 0x0000032f);

	/* set DRAMTMG0 */
	outp32((void *)UMCTL2_BA + 0x100, 0x090b040a); /* mod 20210916 */

	/* set DRAMTMG1 */
	outp32((void *)UMCTL2_BA + 0x104, 0x0002020e); /* mod 20210916 */

	/* set DRAMTMG2 */
	outp32((void *)UMCTL2_BA + 0x108, 0x00000407);

	/* set DRAMTMG3 */
	outp32((void *)UMCTL2_BA + 0x10c, 0x00002006); /* mod 20210916 */

	/* set DRAMTMG4 */
	outp32((void *)UMCTL2_BA + 0x110, 0x04020305);

	/* set DRAMTMG5 */
	outp32((void *)UMCTL2_BA + 0x114, 0x03030202);

	/* set DRAMTMG8 */
	outp32((void *)UMCTL2_BA + 0x120, 0x00000a04);

	/* set DRAMTMG15 */
	outp32((void *)UMCTL2_BA + 0x13c, 0x80000032);

	/* set ZQCTL0 */
	outp32((void *)UMCTL2_BA + 0x180, 0x00800020);

	/* set ZQCTL1 */
	outp32((void *)UMCTL2_BA + 0x184, 0x00000100);

	/* set DFITMG0 */
	outp32((void *)UMCTL2_BA + 0x190, 0x04020101);

	/* set DFITMG1 */
	outp32((void *)UMCTL2_BA + 0x194, 0x00060101);

	/* set DFILPCFG0 */
#ifdef DFI_DDR_PHY_LP
	outp32((void *)UMCTL2_BA + 0x198, 0x0700b131);
#else
	outp32((void *)UMCTL2_BA + 0x198, 0x0700b030);
#endif

	/* set DFIUPD0 */
	outp32((void *)UMCTL2_BA + 0x1a0, 0x00400005);

	/* set DFIUPD1 */
	outp32((void *)UMCTL2_BA + 0x1a4, 0x00170066);

	/* set DFIUPD2 */
	outp32((void *)UMCTL2_BA + 0x1a8, 0x80000000);

	/* set DFIMISC */
	outp32((void *)UMCTL2_BA + 0x1b0, 0x00000011);

	/* set DFIPHYMSTR */
	outp32((void *)UMCTL2_BA + 0x1c4, 0x00000000);

	/* set ADDRMAP0 */
	outp32((void *)UMCTL2_BA + 0x200, 0x0000001f);

	/* set ADDRMAP1 */
	outp32((void *)UMCTL2_BA + 0x204, 0x00080808);

	/* set ADDRMAP2 */
	outp32((void *)UMCTL2_BA + 0x208, 0x00000000);

	/* set ADDRMAP3 */
	outp32((void *)UMCTL2_BA + 0x20c, 0x00000000);

	/* set ADDRMAP4 */
	outp32((void *)UMCTL2_BA + 0x210, 0x00001f1f);

	/* set ADDRMAP5 */
	outp32((void *)UMCTL2_BA + 0x214, 0x070f0707);

	/* set ADDRMAP6 */
	outp32((void *)UMCTL2_BA + 0x218, 0x07070707); /* for 1G bytes */

	/* set ADDRMAP9 */
	outp32((void *)UMCTL2_BA + 0x224, 0x07070707);

	/* set ADDRMAP10 */
	outp32((void *)UMCTL2_BA + 0x228, 0x07070707);

	/* set ADDRMAP11 */
	outp32((void *)UMCTL2_BA + 0x22c, 0x00000007);

	/* set ODTCFG */
	outp32((void *)UMCTL2_BA + 0x240, 0x06000608);

	/* set ODTMAP */
	outp32((void *)UMCTL2_BA + 0x244, 0x00000101);

	/* set SCHED */
	outp32((void *)UMCTL2_BA + 0x250, 0x00f51f00);

	/* set SCHED1 */
	outp32((void *)UMCTL2_BA + 0x254, 0x00000000);

	/* set PERFHPR1 */
	outp32((void *)UMCTL2_BA + 0x25c, 0x0f000001);

	/* set PERFLPR1 */
	outp32((void *)UMCTL2_BA + 0x264, 0x0f00007f);

	/* set PERFWR1 */
	outp32((void *)UMCTL2_BA + 0x26c, 0x0f00007f);

	/* set DBG0 */
	outp32((void *)UMCTL2_BA + 0x300, 0x00000000);

	/* set DBG1 */
	outp32((void *)UMCTL2_BA + 0x304, 0x00000000);

	/* set DBGCMD */
	outp32((void *)UMCTL2_BA + 0x30c, 0x00000000);

	/* set SWCTL */
	outp32((void *)UMCTL2_BA + 0x320, 0x00000001);

	/* set SWCTLSTATIC */
	outp32((void *)UMCTL2_BA + 0x328, 0x00000000);

	/* set POISONCFG */
	outp32((void *)UMCTL2_BA + 0x36c, 0x00110011);

	/* set PCTRL_0 */
	outp32((void *)UMCTL2_BA + 0x490, 0x00000001);

	/* set PCTRL_1 */
	outp32((void *)UMCTL2_BA + 0x540, 0x00000000);

	/* set PCTRL_2 */
	outp32((void *)UMCTL2_BA + 0x5f0, 0x00000000);

	/* set PCTRL_3 */
	outp32((void *)UMCTL2_BA + 0x6a0, 0x00000000);

	/* set PCTRL_4 */
	outp32((void *)UMCTL2_BA + 0x750, 0x00000000);

	/* set PCTRL_5 */
	outp32((void *)UMCTL2_BA + 0x800, 0x00000001);

	/* set PCTRL_6 */
	outp32((void *)UMCTL2_BA + 0x8b0, 0x00000001);

	/* set PCCFG */
	outp32((void *)UMCTL2_BA + 0x400, 0x00000000);

	/* set PCFGR_0 */
	outp32((void *)UMCTL2_BA + 0x404, 0x0001500f);

	/* set PCFGR_1 */
	outp32((void *)UMCTL2_BA + 0x4b4, 0x0001500f);

	/* set PCFGR_2 */
	outp32((void *)UMCTL2_BA + 0x564, 0x0001500f);

	/* set PCFGR_3 */
	outp32((void *)UMCTL2_BA + 0x614, 0x0001500f);

	/* set PCFGR_4 */
	outp32((void *)UMCTL2_BA + 0x6c4, 0x0001500f);

	/* set PCFGR_5 */
	outp32((void *)UMCTL2_BA + 0x774, 0x0001500f);

	/* set PCFGR_6 */
	outp32((void *)UMCTL2_BA + 0x824, 0x0001500f);

	/* set PCFGW_0 */
	outp32((void *)UMCTL2_BA + 0x408, 0x0000500f);

	/* set PCFGW_1 */
	outp32((void *)UMCTL2_BA + 0x4b8, 0x0000500f);

	/* set PCFGW_2 */
	outp32((void *)UMCTL2_BA + 0x568, 0x0000500f);

	/* set PCFGW_3 */
	outp32((void *)UMCTL2_BA + 0x618, 0x0000500f);

	/* set PCFGW_4 */
	outp32((void *)UMCTL2_BA + 0x6c8, 0x0000500f);

	/* set PCFGW_5 */
	outp32((void *)UMCTL2_BA + 0x778, 0x0000500f);

	/* set PCFGW_6 */
	outp32((void *)UMCTL2_BA + 0x828, 0x0000500f);

	/* set SARBASE0 */
	outp32((void *)UMCTL2_BA + 0xf04, 0x00000008);

	/* set SARSIZE0 */
	outp32((void *)UMCTL2_BA + 0xf08, 0x00000003);

	/* de-assert reset signals of DDR memory controller */
	outp32((void *)SYS_BA + 0x20, (inp32((void *)SYS_BA + 0x20) & 0x8fffffff));
	while ((inp32((void *)SYS_BA + 0x20) & 0x20000000) != 0x00000000)
		;

	/* ===================================================================== */
	/* DDR PHY initialization */
	/* ===================================================================== */

	/* set DSGCR,addr=16 */
	outp32((void *)DDRPHY_BA + 0x040, 0xf004649f);

	/* set PGCR1,addr=3 */
	outp32((void *)DDRPHY_BA + 0x00c, 0x0300c461);

	/* set PGCR2,addr=35 */
	outp32((void *)DDRPHY_BA + 0x08c, 0x00f0027f); /* mod 20210916 */

	/* set PTR0,addr=7 */
	outp32((void *)DDRPHY_BA + 0x01c, 0x0c806403); /* for pclk=100mhz */

	/* set PTR1,addr=8 */
	outp32((void *)DDRPHY_BA + 0x020, 0x27100385); /* for pclk=100mhz */

	/* set PTR2,addr=9 */
	outp32((void *)DDRPHY_BA + 0x024, 0x00083def);

	/* set PTR3,addr=10 */
	outp32((void *)DDRPHY_BA + 0x028, 0x0c04111d);

	/* set PTR4,addr=11 */
	outp32((void *)DDRPHY_BA + 0x02c, 0x0801a072);

	/* set MR0,addr=21 */
	outp32((void *)DDRPHY_BA + 0x054, 0x00001b40);

	/* set MR1,addr=22 */
	outp32((void *)DDRPHY_BA + 0x058, 0x00000006);

	/* set MR2,addr=23 */
	outp32((void *)DDRPHY_BA + 0x05c, 0x00000048);

	/* set MR3,addr=24 */
	outp32((void *)DDRPHY_BA + 0x060, 0x00000000);

	/* set DTPR0,addr=18 */
	outp32((void *)DDRPHY_BA + 0x048, 0x6d548855); /* mod 20210916 */

	/* set DTPR1,addr=19 */
	outp32((void *)DDRPHY_BA + 0x04c, 0x1a85e2c0); /* mod 20210922 */

	/* set DTPR2,addr=20 */
	outp32((void *)DDRPHY_BA + 0x050, 0x30023e00);

	/* set ZQ0CR1,addr=97 */
	outp32((void *)DDRPHY_BA + 0x184, 0x0000105d);

	/* polling PGSR0 (addr=4) to 0x0000000f */
	while ((inp32((void *)DDRPHY_BA + 0x010) & 0x0000000f) != 0x0000000f)
		;

	/* set DCR,addr=17 */
	outp32((void *)DDRPHY_BA + 0x044, 0x0000040b);

	/* set DTCR,addr=26 */
	outp32((void *)DDRPHY_BA + 0x068, 0x91003587);

	/* set PIR,addr=1 */
	outp32((void *)DDRPHY_BA + 0x004, 0x0000ff81);

	/* polling PGSR0 (addr=4) to 0xb0000fff */
	while ((inp32((void *)DDRPHY_BA + 0x010) & 0xffffffff) != 0xb0000fff)
		;

	/* polling MCTL2 STAT to 0x00000001 */
	while ((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000001)
		;

	/* set SWCTL */
	outp32((void *)UMCTL2_BA + 0x320, 0x00000000);

	/* set PWRCTL */
	outp32((void *)UMCTL2_BA + 0x030, 0x0000000b);

	/* set SWCTL */
	outp32((void *)UMCTL2_BA + 0x320, 0x00000001);

	/* polling MCTL2 SWSTAT to 0x00000001 */
	while ((inp32((void *)UMCTL2_BA + 0x324) & 0x00000001) != 0x00000001)
		;

}

/* #define DDR3_1GB_1066MBPS_ZENTEL_INIT_BY_DDR32PHY */
void ma35d1_zentel_ddr3_1gb(void)
{
	/* set DBG1 */
	outp32((void *)UMCTL2_BA + 0x304, 0x00000001);

	/* set PWRCTL */
	outp32((void *)UMCTL2_BA + 0x030, 0x00000001); /* mod */

	/* polling to 0x00000000 */
	/* while((inp32(UMCTL2_BA + 0x004) & 0x00000003) != 0x00000000); */

	/* set MSTR */
	outp32((void *)UMCTL2_BA + 0x000, 0x01040001); /* mod 20201204 */

	/* set MRCTRL0 */
	outp32((void *)UMCTL2_BA + 0x010, 0x0000d010); /* mod 20201204 */

	/* set MRCTRL1 */
	outp32((void *)UMCTL2_BA + 0x014, 0x00000000);

	/* set PWRCTL */
	outp32((void *)UMCTL2_BA + 0x030, 0x00000000); /* add */

	/* set PWRTMG */
	outp32((void *)UMCTL2_BA + 0x034, 0x00400010); /* mod 20200421 */

	/* set HWLPCTL */
	outp32((void *)UMCTL2_BA + 0x038, 0x000a0003); /* mod 20200121 */

	/* set RFSHCTL0 */
	outp32((void *)UMCTL2_BA + 0x050, 0x00210000);

	/* set RFSHCTL1 */
	outp32((void *)UMCTL2_BA + 0x054, 0x003c003c);

	/* set RFSHCTL3 */
	outp32((void *)UMCTL2_BA + 0x060, 0x00000000); /* mod 20191211 */

	/* set RFSHTMG */
	outp32((void *)UMCTL2_BA + 0x064, 0x0020005e); /* mod 20210106 */

	/* set CRCPARCTL0 */
	outp32((void *)UMCTL2_BA + 0x0c0, 0x00000000);

	/* set INIT0 */
	outp32((void *)UMCTL2_BA + 0x0d0, 0x40020083); /* mod 20200505 */

	/* set INIT1 */
	outp32((void *)UMCTL2_BA + 0x0d4, 0x00350002); /* mod 20200505 */

	/* set INIT3 */
	outp32((void *)UMCTL2_BA + 0x0dc, 0x1b400006); /* mod 20201109 */

	/* set INIT4 */
	outp32((void *)UMCTL2_BA + 0x0e0, 0x00480000); /* mod 20200822 */

	/* set INIT5 */
	outp32((void *)UMCTL2_BA + 0x0e4, 0x00090000);

	/* set DIMMCTL */
	outp32((void *)UMCTL2_BA + 0x0f0, 0x00000000);

	/* set RANKCTL */
	outp32((void *)UMCTL2_BA + 0x0f4, 0x0000032f);

	/* set DRAMTMG0 */
	outp32((void *)UMCTL2_BA + 0x100, 0x090b080a); /* mod 20211222 */

	/* set DRAMTMG1 */
	outp32((void *)UMCTL2_BA + 0x104, 0x0003020e);

	/* set DRAMTMG2 */
	outp32((void *)UMCTL2_BA + 0x108, 0x00000407);

	/* set DRAMTMG3 */
	outp32((void *)UMCTL2_BA + 0x10c, 0x00002006); /* mod 20211222 */

	/* set DRAMTMG4 */
	outp32((void *)UMCTL2_BA + 0x110, 0x04020205); /* mod 20211222 */

	/* set DRAMTMG5 */
	outp32((void *)UMCTL2_BA + 0x114, 0x03030202);

	/* set DRAMTMG8 */
	outp32((void *)UMCTL2_BA + 0x120, 0x00000a04);

	/* set DRAMTMG15 */
	outp32((void *)UMCTL2_BA + 0x13c, 0x80000032); /* mod 20200225 */

	/* set ZQCTL0 */
	outp32((void *)UMCTL2_BA + 0x180, 0x00800020);

	/* set ZQCTL1 */
	outp32((void *)UMCTL2_BA + 0x184, 0x00000100); /* mod 20200505 */

	/* set DFITMG0 */
	outp32((void *)UMCTL2_BA + 0x190, 0x04020101);

	/* set DFITMG1 */
	outp32((void *)UMCTL2_BA + 0x194, 0x00060101);

	/* set DFILPCFG0 */
#ifdef DFI_DDR_PHY_LP
	outp32((void *)UMCTL2_BA + 0x198, 0x0700b131); /* mod 20200417 */
#else
	outp32((void *)UMCTL2_BA + 0x198, 0x0700b030); /* mod 20200825 */
#endif

	/* set DFIUPD0 */
	outp32((void *)UMCTL2_BA + 0x1a0, 0x00400005); /* mod 20200303 */

	/* set DFIUPD1 */
	outp32((void *)UMCTL2_BA + 0x1a4, 0x00170066); /* mod 20200425 */

	/* set DFIUPD2 */
	outp32((void *)UMCTL2_BA + 0x1a8, 0x80000000); /* mod 20200309 */

	/* set DFIMISC */
	outp32((void *)UMCTL2_BA + 0x1b0, 0x00000011); /* mod */

	/* set DFIPHYMSTR */
	outp32((void *)UMCTL2_BA + 0x1c4, 0x00000000);

	/* set ADDRMAP0 */
	outp32((void *)UMCTL2_BA + 0x200, 0x0000001f); /* mod 20201204 */

	/* set ADDRMAP1 */
	outp32((void *)UMCTL2_BA + 0x204, 0x00080808);

	/* set ADDRMAP2 */
	outp32((void *)UMCTL2_BA + 0x208, 0x00000000);

	/* set ADDRMAP3 */
	outp32((void *)UMCTL2_BA + 0x20c, 0x00000000);

	/* set ADDRMAP4 */
	outp32((void *)UMCTL2_BA + 0x210, 0x00001f1f);

	/* set ADDRMAP5 */
	outp32((void *)UMCTL2_BA + 0x214, 0x070f0707);

	/* set ADDRMAP6 */
	outp32((void *)UMCTL2_BA + 0x218, 0x07070707); /* mod 20200117, for 1G bytes */

	/* set ADDRMAP9 */
	outp32((void *)UMCTL2_BA + 0x224, 0x07070707);

	/* set ADDRMAP10 */
	outp32((void *)UMCTL2_BA + 0x228, 0x07070707);

	/* set ADDRMAP11 */
	outp32((void *)UMCTL2_BA + 0x22c, 0x00000007);

	/* set ODTCFG */
	outp32((void *)UMCTL2_BA + 0x240, 0x06000608);

	/* set ODTMAP */
	outp32((void *)UMCTL2_BA + 0x244, 0x00000101); /* mod 20201204 */

	/* set SCHED */
	outp32((void *)UMCTL2_BA + 0x250, 0x00f51f00);

	/* set SCHED1 */
	outp32((void *)UMCTL2_BA + 0x254, 0x00000000);

	/* set PERFHPR1 */
	outp32((void *)UMCTL2_BA + 0x25c, 0x0f000001);

	/* set PERFLPR1 */
	outp32((void *)UMCTL2_BA + 0x264, 0x0f00007f);

	/* set PERFWR1 */
	outp32((void *)UMCTL2_BA + 0x26c, 0x0f00007f);

	/* set DBG0 */
	outp32((void *)UMCTL2_BA + 0x300, 0x00000000);

	/* set DBG1 */
	outp32((void *)UMCTL2_BA + 0x304, 0x00000000);

	/* set DBGCMD */
	outp32((void *)UMCTL2_BA + 0x30c, 0x00000000);

	/* set SWCTL */
	outp32((void *)UMCTL2_BA + 0x320, 0x00000001);

	/* set SWCTLSTATIC */
	outp32((void *)UMCTL2_BA + 0x328, 0x00000000);

	/* set POISONCFG */
	outp32((void *)UMCTL2_BA + 0x36c, 0x00110011);

	/* set PCTRL_0 */
	outp32((void *)UMCTL2_BA + 0x490, 0x00000001);

	/* set PCTRL_1 */
	outp32((void *)UMCTL2_BA + 0x540, 0x00000000); /* mod 20200323 */

	/* set PCTRL_2 */
	outp32((void *)UMCTL2_BA + 0x5f0, 0x00000000); /* mod 20200323 */

	/* set PCTRL_3 */
	outp32((void *)UMCTL2_BA + 0x6a0, 0x00000000); /* mod 20200323 */

	/* set PCTRL_4 */
	outp32((void *)UMCTL2_BA + 0x750, 0x00000000); /* mod 20200323 */

	/* set PCTRL_5 */
	outp32((void *)UMCTL2_BA + 0x800, 0x00000001); /* mod 20201204 */

	/* set PCTRL_6 */
	outp32((void *)UMCTL2_BA + 0x8b0, 0x00000001);

	/* set PCCFG */
	outp32((void *)UMCTL2_BA + 0x400, 0x00000000);

	/* set PCFGR_0 */
	outp32((void *)UMCTL2_BA + 0x404, 0x0001500f); /* mod 20200408 */

	/* set PCFGR_1 */
	outp32((void *)UMCTL2_BA + 0x4b4, 0x0001500f); /* mod 20200408 */

	/* set PCFGR_2 */
	outp32((void *)UMCTL2_BA + 0x564, 0x0001500f); /* mod 20200408 */

	/* set PCFGR_3 */
	outp32((void *)UMCTL2_BA + 0x614, 0x0001500f); /* mod 20200408 */

	/* set PCFGR_4 */
	outp32((void *)UMCTL2_BA + 0x6c4, 0x0001500f); /* mod 20200408 */

	/* set PCFGR_5 */
	outp32((void *)UMCTL2_BA + 0x774, 0x0001500f); /* mod 20200408 */

	/* set PCFGR_6 */
	outp32((void *)UMCTL2_BA + 0x824, 0x0001500f); /* mod 20200408 */

	/* set PCFGW_0 */
	outp32((void *)UMCTL2_BA + 0x408, 0x0000500f); /* mod 20200408 */

	/* set PCFGW_1 */
	outp32((void *)UMCTL2_BA + 0x4b8, 0x0000500f); /* mod 20200408 */

	/* set PCFGW_2 */
	outp32((void *)UMCTL2_BA + 0x568, 0x0000500f); /* mod 20200408 */

	/* set PCFGW_3 */
	outp32((void *)UMCTL2_BA + 0x618, 0x0000500f); /* mod 20200408 */

	/* set PCFGW_4 */
	outp32((void *)UMCTL2_BA + 0x6c8, 0x0000500f); /* mod 20200408 */

	/* set PCFGW_5 */
	outp32((void *)UMCTL2_BA + 0x778, 0x0000500f); /* mod 20200408 */

	/* set PCFGW_6 */
	outp32((void *)UMCTL2_BA + 0x828, 0x0000500f); /* mod 20200408 */

	/* set SARBASE0 */
	outp32((void *)UMCTL2_BA + 0xf04, 0x00000008);

	/* set SARSIZE0 */
	outp32((void *)UMCTL2_BA + 0xf08, 0x00000003); /* mod 20201204 */

	/* de-assert reset signals of DDR memory controller */
	outp32((void *)SYS_BA + 0x20, (inp32((void *)SYS_BA + 0x20) & 0x8fffffff));
	while ((inp32((void *)SYS_BA + 0x20) & 0x20000000) != 0x00000000)
		;

	/* ===================================================================== */
	/* DDR PHY initialization */
	/* ===================================================================== */

	/* set DSGCR,addr=16 */
	outp32((void *)DDRPHY_BA + 0x040, 0xf004649f);

	/* set PGCR1,addr=3 */
	outp32((void *)DDRPHY_BA + 0x00c, 0x0300c461); /* mod 20200608 */

	/* set PGCR2,addr=35 */
	outp32((void *)DDRPHY_BA + 0x08c, 0x00f0068e); /* mod 20210106 */

	/* set PTR0,addr=7 */
	outp32((void *)DDRPHY_BA + 0x01c, 0x0c806403); /* mod 20200306 for pclk=100mhz */

	/* set PTR1,addr=8 */
	outp32((void *)DDRPHY_BA + 0x020, 0x27100385); /* mod 20200306 for pclk=100mhz */

	/* set PTR2,addr=9 */
	outp32((void *)DDRPHY_BA + 0x024, 0x00083def);

	/* set PTR3,addr=10 */
	outp32((void *)DDRPHY_BA + 0x028, 0x0c04111d); /* mod 20200511 */

	/* set PTR4,addr=11 */
	outp32((void *)DDRPHY_BA + 0x02c, 0x0801a072); /* mod 20200511 */

	/* set MR0,addr=21 */
	outp32((void *)DDRPHY_BA + 0x054, 0x00001b40); /* mod */

	/* set MR1,addr=22 */
	outp32((void *)DDRPHY_BA + 0x058, 0x00000006); /* mod 20201109 */

	/* set MR2,addr=23 */
	outp32((void *)DDRPHY_BA + 0x05c, 0x00000048); /* mod 20200822 */

	/* set MR3,addr=24 */
	outp32((void *)DDRPHY_BA + 0x060, 0x00000000);

	/* set DTPR0,addr=18 */
	outp32((void *)DDRPHY_BA + 0x048, 0x6d558855); /* mod 20211222 */

	/* set DTPR1,addr=19 */
	outp32((void *)DDRPHY_BA + 0x04c, 0x2285e2ca); /* mod 20211222 */

	/* set DTPR2,addr=20 */
	outp32((void *)DDRPHY_BA + 0x050, 0x30023e00);

	/* set ZQ0CR1,addr=97 */
	outp32((void *)DDRPHY_BA + 0x184, 0x0000105d); /* mod 20201109 */

	/* polling PGSR0 (addr=4) to 0x0000000f */
	while ((inp32((void *)DDRPHY_BA + 0x010) & 0x0000000f) != 0x0000000f)
		;

	/* set DCR,addr=17 */
	outp32((void *)DDRPHY_BA + 0x044, 0x0000040b);

	/* set DTCR,addr=26 */
	outp32((void *)DDRPHY_BA + 0x068, 0x91003587); /* mod 20201204 */

	/* set PIR,addr=1 */
	outp32((void *)DDRPHY_BA + 0x004, 0x0000ff81);

	/* polling PGSR0 (addr=4) to 0xb0000fff */
	while ((inp32((void *)DDRPHY_BA + 0x010) & 0xffffffff) != 0xb0000fff)
		;

	/* polling MCTL2 STAT to 0x00000001 */
	while ((inp32((void *)UMCTL2_BA + 0x004) & 0x00000003) != 0x00000001)
		;

	/* set SWCTL */
	outp32((void *)UMCTL2_BA + 0x320, 0x00000000);

	/* set PWRCTL */
	outp32((void *)UMCTL2_BA + 0x030, 0x0000000b); /* mod 20191213 */

	/* set SWCTL */
	outp32((void *)UMCTL2_BA + 0x320, 0x00000001);

	/* polling MCTL2 SWSTAT to 0x00000001 */
	while ((inp32((void *)UMCTL2_BA + 0x324) & 0x00000001) != 0x00000001)
		;

}


static void *fdt = (void *)(uintptr_t)MA35D1_DTB_BASE;

void ma35d1_ddr_init(void)
{
	//Set TAHBCKEN,CM4CKEN,CA35CKEN,DDR6CKEN,GFXCKEN,VC8KCKEN,DCUCKEN,GMAC0CKEN,GMAC1CKEN,CAP0CKEN,CAP1CKEN
	outp32(CLK_BA + 0x04, (inp32(CLK_BA + 0x04) | 0x7F000037));
	outp32(CLK_BA + 0x0C, (inp32(CLK_BA + 0x0C) | 0x40000000));

	/* DDR control register clock gating disable */
	outp32(SYS_BA + 0x70, (inp32(SYS_BA + 0x70) | 0x00800000));
	/* de-assert presetn of MCTL2 */
	outp32(SYS_BA + 0x20, (inp32(SYS_BA + 0x20) & 0xafffffff));
	while((inp32(SYS_BA + 0x20) & 0x50000000) != 0x00000000);
	//set MCTLCRST to 1
	outp32(SYS_BA + 0x20, (inp32(SYS_BA + 0x20) | 0x20000000));

	/* read DTB */
	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	if (fdt_node_offset_by_compatible(fdt, -1, "wb-ddr3-256mb") >= 0) {
		ma35d1_wb_ddr3_256mb();
	} else if (fdt_node_offset_by_compatible(fdt, -1, "wb-ddr3-512mb") >= 0) {
		ma35d1_wb_ddr3_512mb();
	} else if (fdt_node_offset_by_compatible(fdt, -1, "mt-ddr3-1gb") >= 0) {
		ma35d1_mt_ddr3_1gb();
	} else if (fdt_node_offset_by_compatible(fdt, -1, "wb-ddr2-128mb") >= 0) {
		ma35d1_wb_ddr2_128mb();
	} else if (fdt_node_offset_by_compatible(fdt, -1, "issi-ddr3-1gb") >= 0) {
		ma35d1_issi_ddr3_1gb();
	} else if (fdt_node_offset_by_compatible(fdt, -1, "zentel-ddr3-1gb") >= 0) {
		ma35d1_zentel_ddr3_1gb();
	} else {
		WARN("The compatible property ddr type not found\n");
	}

	outp32((void *)UMCTL2_BA+0x490, 0x1);
	outp32((void *)UMCTL2_BA+0x8b0, 0x1);
	outp32((void *)UMCTL2_BA+0x960, 0x1);

	outp32((void *)UMCTL2_BA+0x540, 0x1);
	outp32((void *)UMCTL2_BA+0x5f0, 0x1);
	outp32((void *)UMCTL2_BA+0x6a0, 0x1);
	outp32((void *)UMCTL2_BA+0x750, 0x1);
	outp32((void *)UMCTL2_BA+0x800, 0x1);

	outp32(SYS_BA + 0x70,(inp32(SYS_BA + 0x70) & ~0x00800000));	/* DDR control register clock gating enable */
	outp32(CLK_BA + 0x04, 0x35);
}
