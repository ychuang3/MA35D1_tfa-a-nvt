/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <libfdt.h>

#include <plat/common/platform.h>
#include <platform_def.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/delay_timer.h>
#include <drivers/io/io_block.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <drivers/nuvoton/ma35d1_pmic.h>

#define RETRY_COUNT 3
/*---------------------------------------------------------------------------*/
/* Global variables                                                          */
/*---------------------------------------------------------------------------*/
unsigned char g_u8DeviceAddr = 0xB0;
unsigned char g_uPageNum;
unsigned char g_u8RegAddr;
unsigned char g_u8WriteData;

volatile unsigned char g_u8RxData;
volatile unsigned char g_u8DataLen;
volatile unsigned char g_u8EndFlag = 0;

void I2C_MasterRx(unsigned int u32Status)
{
	if (u32Status == 0x08) {
		/* START has been transmitted and prepare SLA+W */
		mmio_write_32(REG_I2C0_DAT,
					g_u8DeviceAddr|(g_uPageNum << 1));
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | I2C_CTL_SI);
	} else if (u32Status == 0x18) {
		/* SLA+W has been transmitted and ACK has been received */
		mmio_write_32(REG_I2C0_DAT, g_u8RegAddr);
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | I2C_CTL_SI);
	} else if (u32Status == 0x20) {
		/* SLA+W has been transmitted and NACK has been received */
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | (I2C_CTL_STA |
					I2C_CTL_STO | I2C_CTL_SI));
	} else if (u32Status == 0x28) {
		/* DATA has been transmitted and ACK has been received */
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | (I2C_CTL_STA |
					I2C_CTL_SI)); // repeat start
	} else if (u32Status == 0x10) {
		/* Repeat START has been transmitted and prepare SLA+R */
		mmio_write_32(REG_I2C0_DAT,
					(g_u8DeviceAddr|(g_uPageNum << 1)) |
					0x01);
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | I2C_CTL_SI);
	} else if (u32Status == 0x40) {
		/* SLA+R has been transmitted and ACK has been received */
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | I2C_CTL_SI);
	} else if (u32Status == 0x58) {
		/* DATA has been received and NACK has been returned */
		g_u8RxData = mmio_read_32(REG_I2C0_DAT);
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | (I2C_CTL_STO | I2C_CTL_SI));
		g_u8EndFlag = 1;
	} else {
		//ERROR("Status 0x%x is NOT processed\n", u32Status);
	}
}

void I2C_MasterTx(unsigned int u32Status)
{
	if (u32Status == 0x08) {
		/* START has been transmitted */
		 /* Write SLA+W to Register I2CDAT */
		mmio_write_32(REG_I2C0_DAT,
					g_u8DeviceAddr|(g_uPageNum << 1));
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | I2C_CTL_SI);
	} else if (u32Status == 0x18) {
		/* SLA+W has been transmitted and ACK has been received */
		mmio_write_32(REG_I2C0_DAT, g_u8RegAddr);
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | I2C_CTL_SI);
	} else if (u32Status == 0x20) {
		/* SLA+W has been transmitted and NACK has been received */
		mmio_write_32(REG_I2C0_CTL,
					(mmio_read_32(REG_I2C0_CTL) &
					~0x3c) | (I2C_CTL_STA | I2C_CTL_STO |
					I2C_CTL_SI));
	} else if (u32Status == 0x28) {
		/* DATA has been transmitted and ACK has been received */
		if ((g_u8DataLen == 0) && (g_u8EndFlag == 0)) {
			mmio_write_32(REG_I2C0_DAT, g_u8WriteData);
			mmio_write_32(REG_I2C0_CTL,
						(mmio_read_32(REG_I2C0_CTL) &
						~0x3c) | I2C_CTL_SI);
			g_u8DataLen++;
		} else {
			g_u8DataLen = 0;
			mmio_write_32(REG_I2C0_CTL,
						(mmio_read_32(REG_I2C0_CTL) &
						~0x3c) |
						(I2C_CTL_STO | I2C_CTL_SI));
			g_u8EndFlag = 1;
		}
	} else {
		//ERROR("Status 0x%x is NOT processed\n", u32Status);
	}
}

unsigned int ma35d1_write_i2c_data(unsigned int u32Addr, unsigned int u32Data)
{
	unsigned int I2C_TIME_OUT_COUNT = 6000;
	unsigned int u32Status;
	unsigned int u32time_out = 0;

	g_uPageNum = 0;
	if (g_uPageNum == 0xff) {
		ERROR("\n address error !!\n");
		return 0;
	}

	g_u8RegAddr     = u32Addr;
	g_u8WriteData   = u32Data;
	g_u8EndFlag     = 0x0;

	mmio_write_32(REG_I2C0_CTL, (mmio_read_32(REG_I2C0_CTL) &
				~0x3c) | I2C_CTL_STA);

	while (1) {
		if (mmio_read_32(REG_I2C0_CTL) & I2C_CTL_SI) {
			u32Status = mmio_read_32(REG_I2C0_STATUS);
			I2C_MasterTx(u32Status);
		}

		if (g_u8EndFlag) {
			break;
		}
		u32time_out++;
		if (u32time_out > I2C_TIME_OUT_COUNT) {
			return 0; // error
		}
	}
	return 1;
}

unsigned int ma35d1_read_i2c_data(unsigned int u32Addr, unsigned int *u32Data)
{
	unsigned int I2C_TIME_OUT_COUNT = 6000;
	unsigned int u32Status;
	unsigned int u32time_out = 0;

	g_uPageNum = 0;
	if (g_uPageNum == 0xff) {
		ERROR("\n address error !!\n");
		return 0;
	}

	g_u8RegAddr     = u32Addr;
	g_u8EndFlag     = 0x0;

	mmio_write_32(REG_I2C0_CTL, (mmio_read_32(REG_I2C0_CTL) &
				 ~0x3c) | I2C_CTL_STA);

	while (1) {
		if (mmio_read_32(REG_I2C0_CTL) & I2C_CTL_SI) {
			u32Status = mmio_read_32(REG_I2C0_STATUS);
			I2C_MasterRx(u32Status);
		}

		if (g_u8EndFlag) {
			break;
		}

		u32time_out++;
		if (u32time_out > I2C_TIME_OUT_COUNT) {
			return 1; // error
		}
	}

	*u32Data = g_u8RxData;
	return 2;
}

unsigned int ma35d1_read_pmic_data(
	unsigned int u32Addr,
	unsigned int *u32Data)
{
	int j = RETRY_COUNT;

	while (j-- > 0) {
		if (ma35d1_read_i2c_data(u32Addr, u32Data) == 2) {
			break;
		}
	}

	if (j <= 0) {
		//WARN("\nREAD PMIC DATA ERROR!\n");
		return 0;
	}

	return 1;
}

unsigned int ma35d1_write_pmic_data(unsigned int u32Addr, unsigned int u32Data)
{
	int j = RETRY_COUNT;

	while (j-- > 0) {
		if (ma35d1_write_i2c_data(u32Addr, u32Data) == 1) {
			break;
		}
	}

	if (j <= 0) {
		/* WARN("\nWRITE PMIC DATA ERROR [%d]!\n", j); */
		return 0;
	}

	return 1;
}

void ma35d1_i2c0_init(unsigned int sys_clk)
{
	unsigned long clk_rate;
	unsigned int u32Div, speed;

	outp32((void *)0X40460208, inp32((void *)0X40460208) |
		    (0x3fff << 16)); // enable GPIO clock
	outp32((void *)0X40460210, inp32((void *)0X40460210) |
		    (0x1 << 0));  // I2C0 CLK

	outp32(0x40460098, ((inp32(0x40460098) & ~0x0f000000) |
		    (0x6<<24))); // PD.6 I2C0_SDA
	outp32(0x40460098, ((inp32(0x40460098) & ~0xf0000000) |
		    (0x6<<28))); // PD.7 I2C0_CLK
	outp32(0x400400F0, 0x5 << 12); // pull high

	/* i2c_clk = 100KHz */
	clk_rate = sys_clk;
	speed = 100*1000;

	/* assume speed above 1000 are Hz-specified */
	if (speed > 1000)
		speed = speed / 1000;
	if (speed > 400)
		speed = 400;

	u32Div = (unsigned int)(((clk_rate * 10U) /
			(speed * 4U) + 5U) / 10U - 1U);

	mmio_write_32(REG_I2C0_CLKDIV, u32Div);
	mmio_write_32(REG_I2C0_CTL, mmio_read_32(REG_I2C0_CTL)  |
				I2C_CTL_ENABLE); /* i2c enable */
}


int ma35d1_set_pmic(int type, int vol)
{
	unsigned int reg = 0xff;
	int ret = 0;

	if (pmicIsInit == 0) {
		ma35d1_i2c0_init(pmic_clk);
		pmicIsInit = 1;
	}

	if (type == VOL_CPU) {
		reg = 0xA4;
	} else if (type == VOL_SD) {
		reg = 0x32;
	} else {
		ERROR("Not support type!\n");
	}

	if (vol == VOL_1_10) {		// (1.1 - 0.3) / 0.01 = 80 = 0x50
		INFO("1.1V\n");
		ret = ma35d1_write_pmic_data(reg, 0x50);
	} else if (vol == VOL_1_00) {	// (1.00 - 0.3) / 0.01 = 70 = 0x46
		INFO("1.00V\n");
		ret = ma35d1_write_pmic_data(reg, 0x46);
	} else if (vol == VOL_1_15) {	// (1.15 - 0.3) / 0.01 = 85 = 0x55
		INFO("1.15V\n");
		ret = ma35d1_write_pmic_data(reg, 0x55);
	} else if (vol == VOL_1_20) {	// (1.2 - 0.3) / 0.01 = 90 = 0x5A
		INFO("1.20V\n");
		ret = ma35d1_write_pmic_data(reg, 0x5A);
	} else if (vol == VOL_1_25) {	// (1.25 - 0.3) / 0.01 = 95 = 0x5F
		INFO("1.25V\n");
		ret = ma35d1_write_pmic_data(reg, 0x5F);
	} else if (vol == VOL_1_29) {	// (1.29 - 0.3) / 0.01 = 99 = 0x63
		INFO("1.29V\n");
		ret = ma35d1_write_pmic_data(reg, 0x63);
	} else if (vol == VOL_1_30) {	// (1.3 - 0.3) / 0.01 = 100 = 0x64
		INFO("1.3V\n");
		ret = ma35d1_write_pmic_data(reg, 0x64);
	} else if (vol == VOL_1_80) {
		INFO("1.8V\n");
		ret = ma35d1_write_pmic_data(reg, 0x80);
	} else if (vol == VOL_3_30) {
		INFO("3.3V\n");
		ret = ma35d1_write_pmic_data(reg, 0x00);
	} else {
		ERROR("Not support voltage!\n");
		ret = -1;
	}

	if (ret >= 0)
		pmic_state[type] = vol;

	return ret;
}

int ma35d1_get_pmic(int type)
{
	return pmic_state[type];
}
