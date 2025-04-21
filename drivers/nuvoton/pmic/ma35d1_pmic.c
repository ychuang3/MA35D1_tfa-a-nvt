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

#define RETRY_COUNT	3

#define PMIC_NO		0
#define PMIC_DIALOG		1
#define PMIC_IP6103		2
#define PMIC_APW7704F	3

#if (MA35D1_PMIC == PMIC_IP6103)
#define DEVICE_ADDR 0x60
#endif

#if (MA35D1_PMIC == PMIC_DIALOG)
#define DEVICE_ADDR 0xB0
#endif

#if (MA35D1_PMIC == PMIC_APW7704F)
#define DEVICE_ADDR (0x25 << 1)
#endif

#if (MA35D1_PMIC == PMIC_NO)
#define DEVICE_ADDR 0x0
#endif



/*---------------------------------------------------------------------------*/
/* Global variables                                                          */
/*---------------------------------------------------------------------------*/
unsigned char g_u8DeviceAddr = DEVICE_ADDR;
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
	unsigned int I2C_TIME_OUT_COUNT = 0x200000;
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
	unsigned int I2C_TIME_OUT_COUNT = 0x200000;
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

	mmio_write_32(0X40460208, mmio_read_32(0X40460208) |
		    (0x3fff << 16)); // enable GPIO clock
	mmio_write_32(0X40460210, mmio_read_32(0X40460210) |
		    (0x1 << 0));  // I2C0 CLK

	mmio_write_32(0x40460098, ((mmio_read_32(0x40460098) & ~0x0f000000) |
		    (0x6<<24))); // PD.6 I2C0_SDA
	mmio_write_32(0x40460098, ((mmio_read_32(0x40460098) & ~0xf0000000) |
		    (0x6<<28))); // PD.7 I2C0_CLK
	mmio_write_32(0x400400F0, 0x5 << 12); // pull high

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


#if MA35D1_PMIC == PMIC_IP6103
#define VOL_LDO4 0x03
#define VOL_LDO5 0x04
#define VOL_LDO6 0x05

int ma35d1_set_pmic(int type, int vol)
{
	unsigned int reg0 = 0xff;
	unsigned int reg1 = 0x41;
	unsigned int temp = 0;
	int ret = 0;

	if (pmicIsInit == 0) {
		ma35d1_i2c0_init(pmic_clk);
		pmicIsInit = 1;
		ma35d1_write_pmic_data(0x27, 0xF0);
		ma35d1_write_pmic_data(0x2e, 0xF0);
		ma35d1_write_pmic_data(0x35, 0xF0);
		ma35d1_write_pmic_data(0x3c, 0xF0);
	}

	if (type == VOL_CPU) {
		reg0 = 0x21;	/* cpu */
		switch (vol) {
		case VOL_1_00:
			INFO("IP6103 1.00V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x24);
			break;
		case VOL_1_10:
			INFO("IP6103 1.10V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x28);
			break;
		case VOL_1_15:
			INFO("IP6103 1.15V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x2c);
			break;
		case VOL_1_18:
			INFO("IP6103 1.175V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x2e);
			break;
		case VOL_1_19:
			INFO("IP6103 1.1875V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x2f);
			break;
		case VOL_1_20:
			INFO("IP6103 1.20V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x30);
			break;
		case VOL_1_21:
			INFO("IP6103 1.2125V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x31);
			break;
		case VOL_1_22:
			INFO("IP6103 1.225V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x32);
			break;
		case VOL_1_23:
		case VOL_1_24:
			INFO("IP6103 1.2375V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x33);
			break;
		case VOL_1_25:
			INFO("IP6103 1.25V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x34);
			break;
		case VOL_1_26:
			INFO("IP6103 1.2625V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x35);
			break;
		case VOL_1_27:
			INFO("IP6103 1.275V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x36);
			break;
		case VOL_1_28:
		case VOL_1_29:
			INFO("IP6103 1.29V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x37);
			break;
		case VOL_1_30:
			INFO("IP6103 1.30V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x38);
			break;
		case VOL_1_32:
			INFO("IP6103 1.32V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x3a);
			break;
		case VOL_1_34:
			INFO("IP6103 1.34V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x3c);
			break;
		case VOL_1_36:
			INFO("IP6103 1.36V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x3e);
			break;
		default:
			ERROR("Not support voltage!\n");
			ret = -1;
			break;
		}
	} else if (type == VOL_SD) {
		reg0 = 0x42;
		switch (vol) {
		case VOL_1_80:
			INFO("IP6103 SD 1.8V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x2c);
			break;
		case VOL_3_30:
			INFO("IP6103 SD 3.3V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x68);
			break;
		default:
			ERROR("Not support voltage!\n");
			ret = -1;
			break;
		}
	} else if (type == VOL_LDO5) {
		reg0 = 0x4C;
		ma35d1_read_pmic_data(reg1, &temp);
		switch (vol) {
		case VOL_1_00:
			INFO("IP6103 LDO5 1.00V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x0C);
			ma35d1_write_pmic_data(reg1, temp|0x20);
			break;
		case VOL_1_20:
			INFO("IP6103 LDO5 1.20V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x14);
			ma35d1_write_pmic_data(reg1, temp|0x20);
			break;
		case VOL_1_80:
			INFO("IP6103 LDO5 1.80V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x2c);
			ma35d1_write_pmic_data(reg1, temp|0x20);
			break;
		case VOL_2_50:
			INFO("IP6103 LDO5 2.50V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x48);
			ma35d1_write_pmic_data(reg1, temp|0x20);
			break;
		case VOL_3_30:
			INFO("IP6103 LDO5 3.30V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x68);
			ma35d1_write_pmic_data(reg1, temp|0x20);
			break;
		default:
			ma35d1_write_pmic_data(reg1, temp&~0x20);
			ERROR("Not support voltage!\n");
			ret = -1;
			break;
		}
	} else if (type == VOL_LDO6) {
		reg0 = 0x4E;
		ma35d1_read_pmic_data(reg1, &temp);
		switch (vol) {
		case VOL_1_00:
			INFO("IP6103 LDO6 1.00V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x0C);
			ma35d1_write_pmic_data(reg1, temp|0x40);
			break;
		case VOL_1_20:
			INFO("IP6103 LDO6 1.20V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x14);
			ma35d1_write_pmic_data(reg1, temp|0x40);
			break;
		case VOL_1_80:
			INFO("IP6103 LDO6 1.80V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x2c);
			ma35d1_write_pmic_data(reg1, temp|0x40);
			break;
		case VOL_2_50:
			INFO("IP6103 LDO6 2.50V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x48);
			ma35d1_write_pmic_data(reg1, temp|0x40);
			break;
		case VOL_3_30:
			INFO("IP6103 LDO6 3.30V\n");
			ret = ma35d1_write_pmic_data(reg0, 0x68);
			ma35d1_write_pmic_data(reg1, temp|0x40);
			break;
		default:
			ma35d1_write_pmic_data(reg1, temp&~0x40);
			ERROR("Not support voltage!\n");
			ret = -1;
			break;
		}
	} else {
		ERROR("Not support type!\n");
	}

	if (ret >= 0)
		pmic_state[type] = vol;

	return ret;
}
#elif (MA35D1_PMIC == PMIC_DIALOG)
int ma35d1_set_pmic(int type, int vol)
{
	unsigned int reg = 0xff;
	char str[32] = { 0 };
	int ret = 0;

	if (pmicIsInit == 0) {
		ma35d1_i2c0_init(pmic_clk);
		pmicIsInit = 1;
	}

	if (type == VOL_CPU || type == VOL_CORE) {
		if (type == VOL_CPU) {
			reg = 0xA4;
			memcpy(str, "CPU Power:", 10);
		} else {
			reg = 0xA3;
			memcpy(str, "CORE Power:", 11);
		}

		if (vol == VOL_1_10) {		/* (1.1 - 0.3) / 0.01 = 80 = 0x50 */
			INFO("%s 1.1V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x50);
		} else if (vol == VOL_1_00) {	/* (1.00 - 0.3) / 0.01 = 70 = 0x46 */
			INFO("%s 1.00V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x46);
		} else if (vol == VOL_1_15) {	/* (1.15 - 0.3) / 0.01 = 85 = 0x55 */
			INFO("%s 1.15V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x55);
		} else if (vol == VOL_1_18) {	/* (1.18 - 0.3) / 0.01 = 88 = 0x58 */
			INFO("%s 1.18V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x58);
		} else if (vol == VOL_1_19) {	/* (1.19 - 0.3) / 0.01 = 89 = 0x59 */
			INFO("%s 1.19V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x59);
		} else if (vol == VOL_1_20) {	/* (1.2 - 0.3) / 0.01 = 90 = 0x5A */
			INFO("%s 1.20V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x5A);
		} else if (vol == VOL_1_21) {	/* (1.21 - 0.3) / 0.01 = 91 = 0x5B */
			INFO("%s 1.21V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x5B);
		} else if (vol == VOL_1_22) {	/* (1.22 - 0.3) / 0.01 = 92 = 0x5C */
			INFO("%s 1.22V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x5C);
		} else if (vol == VOL_1_23) {	/* (1.23 - 0.3) / 0.01 = 93 = 0x5D */
			INFO("%s 1.23V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x5D);
		} else if (vol == VOL_1_24) {	/* (1.24 - 0.3) / 0.01 = 94 = 0x5E */
			INFO("%s 1.24V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x5E);
		} else if (vol == VOL_1_25) {	/* (1.25 - 0.3) / 0.01 = 95 = 0x5F */
			INFO("%s 1.25V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x5F);
		} else if (vol == VOL_1_26) {	/* (1.26 - 0.3) / 0.01 = 96 = 0x60 */
			INFO("%s 1.26V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x60);
		} else if (vol == VOL_1_27) {	/* (1.27 - 0.3) / 0.01 = 97 = 0x61 */
			INFO("%s 1.27V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x61);
		} else if (vol == VOL_1_28) {	/* (1.28 - 0.3) / 0.01 = 98 = 0x62 */
			INFO("%s 1.28V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x62);
		} else if (vol == VOL_1_29) {	/* (1.29 - 0.3) / 0.01 = 99 = 0x63 */
			INFO("%s 1.29V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x63);
		} else if (vol == VOL_1_30) {	/* (1.30 - 0.3) / 0.01 = 100 = 0x64 */
			INFO("%s 1.30V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x64);
		} else if (vol == VOL_1_31) {	/* (1.31 - 0.3) / 0.01 = 101 = 0x65 */
			INFO("%s 1.31V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x65);
		} else if (vol == VOL_1_32) {	/* (1.32 - 0.3) / 0.01 = 102 = 0x66 */
			INFO("%s 1.32V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x66);
		} else if (vol == VOL_1_33) {	/* (1.33 - 0.3) / 0.01 = 103 = 0x67 */
			INFO("%s 1.33V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x67);
		} else if (vol == VOL_1_34) {   /* (1.34 - 0.3) / 0.01 = 104 = 0x68 */
			INFO("%s 1.34V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x68);
		} else if (vol == VOL_1_35) {   /* (1.35 - 0.3) / 0.01 = 105 = 0x69 */
			INFO("%s 1.35V\n", str);
			ret = ma35d1_write_pmic_data(reg, 0x69);
		} else {
			ERROR("Not support voltage!\n");
			ret = -1;
		}
	} else if (type == VOL_SD) {
		reg = 0x32;
		if (vol == VOL_1_80) {
			INFO("SD 1.8V\n");
			ret = ma35d1_write_pmic_data(reg, 0x80);
		} else if (vol == VOL_3_30) {
			INFO("SD 3.3V\n");
			ret = ma35d1_write_pmic_data(reg, 0x00);
		} else {
			ERROR("SD - Not support voltage!\n");
			ret = -1;
		}
	} else {
		ERROR("SD - Not support type!\n");
	}

	if (ret >= 0)
		pmic_state[type] = vol;

	return ret;
}
#elif (MA35D1_PMIC == PMIC_APW7704F)
int ma35d1_set_pmic(int type, int vol)
{
	unsigned int reg = 0x07;
	int ret = 0;

	if (pmicIsInit == 0) {
		ma35d1_i2c0_init(pmic_clk);
		pmicIsInit = 1;
	}

	switch (vol)
	{
	case VOL_1_10:
		INFO("APW7704F 1.11V\n");
		ret = ma35d1_write_pmic_data(reg, 0x32);
		break;
	case VOL_1_18:
		INFO("APW7704F 1.17V\n");
		ret = ma35d1_write_pmic_data(reg, 0x32);
		break;
	case VOL_1_19:
	case VOL_1_20:
	case VOL_1_21:
	case VOL_1_22:
		INFO("APW7704F 1.20V\n");
		ret = ma35d1_write_pmic_data(reg, 0x33);
		break;
	case VOL_1_23:
	case VOL_1_24:
		INFO("APW7704F 1.23V\n");
		ret = ma35d1_write_pmic_data(reg, 0x34);
		break;
	case VOL_1_25:
	case VOL_1_26:
	case VOL_1_27:
		INFO("APW7704F 1.26V\n");
		ret = ma35d1_write_pmic_data(reg, 0x35);
		break;
	case VOL_1_28:
	case VOL_1_29:
		INFO("APW7704F 1.29V\n");
		ret = ma35d1_write_pmic_data(reg, 0x36);
		break;

	case VOL_1_34:
		INFO("APW7704F 1.34V\n");
		/* The same value 0x36 as VOL_1_28, but different board circuit. */
		ret = ma35d1_write_pmic_data(reg, 0x36);
		break;

	default:
		ERROR("Not support voltage!\n");
		ret=-1;
		break;
	}

	if (ret >= 0)
		pmic_state[type] = vol;

	return ret;
}
#else
int ma35d1_set_pmic(int type, int vol) {
	return 1;
}
#endif

int ma35d1_get_pmic(int type)
{
	return pmic_state[type];
}
