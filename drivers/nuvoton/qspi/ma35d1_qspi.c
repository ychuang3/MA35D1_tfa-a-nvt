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

#include "ma35d1_qspi.h"


struct ma35d1_qspi_info {
	int page_size;
	int oob_size;
	int pages_per_block;
	long size;	/* spinand total size */
	long offset;	/* image offset in spinand */
	unsigned char   is_quad;
	unsigned char   QuadReadCmd;
	unsigned char   ReadStatusCmd;
	unsigned char   WriteStatusCmd;
	unsigned char   StatusValue;
	unsigned char   dummybyte1;     /* between command and address */
	unsigned char   dummybyte2;     /* between address and data */
	unsigned char   SuspendInterval;
};

struct ma35d1_qspi_info ma35d1_qspi;

#define SPINOR_BLOCK_SIZE	(2048)

/***************************************************************/
static void nus3500_spi_sendcmd(unsigned char *cmd, unsigned int cmdlen, unsigned char *data, unsigned int datalen)
{
	int volatile i;

	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low

	for (i=0; i<cmdlen; i++)
		mmio_write_32(REG_QSPI0_TX, cmd[i]);

	// wait tx finish
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01){}
	// clear RX buffer
	mmio_write_32(REG_QSPI0_FIFOCTL, mmio_read_32(REG_QSPI0_FIFOCTL) | 0x1);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x800000);

	// read data
	for (i=0; i<datalen; i++)
	{
		mmio_write_32(REG_QSPI0_TX, 0x00);
		while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);
		*data++ = mmio_read_32(REG_QSPI0_RX);
	}
	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
}

/***************************************************************/

static void ma35d1_spinand_reset()
{
	unsigned char cmd[1];

	cmd[0] = 0xff;
	nus3500_spi_sendcmd(cmd, 1, 0, 0);
}

static int ma35d1_spinand_getstatus(unsigned char offset)
{
	unsigned char data[2];
	unsigned char cmd[4];

	/* read status */
	cmd[0] = 0x0f;
	cmd[1] = offset;
	nus3500_spi_sendcmd(cmd, 2, data, 1);
	return data[0];
}

static void ma35d1_spinand_setstatus(unsigned char offset, unsigned char value)
{
	unsigned char cmd[4];

	/* write enable */
	cmd[0] = CMD_WRITE_ENABLE;
	nus3500_spi_sendcmd(cmd, 1, 0, 0);

	/* write status */
	cmd[0] = 0x1f;
	cmd[1] = offset;
	cmd[2] = value;
	nus3500_spi_sendcmd(cmd, 3, 0, 0);
}

static void ma35d1_spinand_quad_disable()
{
	unsigned int status;

	/* disable quad mode */
	status = ma35d1_spinand_getstatus(0xb0);
	ma35d1_spinand_setstatus(0xb0, status & 0xfe);
}

static void ma35d1_spinand_quad_enable()
{
	unsigned int status;

	/* enable quad mode */
	status = ma35d1_spinand_getstatus(0xb0);
	ma35d1_spinand_setstatus(0xb0, status | 0x01);
}

static int ma35d1_spinand_singleread(struct ma35d1_qspi_info *spinand, unsigned int addr, unsigned int *buf)
{
	unsigned char cmd[4];
	int volatile i;

	cmd[0] = 0x13;  /* page read */
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;
	nus3500_spi_sendcmd(cmd, 4, 0, 0);
	while(ma35d1_spinand_getstatus(0xc0) & 0x1);    // wait ready
	if (ma35d1_spinand_getstatus(0xc0) & 0x20)
	{
		ERROR("Err-ECC\n");
		return 1;
	}

	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low

	mmio_write_32(REG_QSPI0_TX, 0x03);
	mmio_write_32(REG_QSPI0_TX, 0);
	mmio_write_32(REG_QSPI0_TX, 0);
	mmio_write_32(REG_QSPI0_TX, 0);

	// wait tx finish
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01){}
	// clear RX buffer
	mmio_write_32(REG_QSPI0_FIFOCTL, mmio_read_32(REG_QSPI0_FIFOCTL) | 0x1);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x800000);

	// set DWIDTH to 32 bit and enable byte reorder
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL) & ~0x1F00) | (1<<19));

	// read data
	for (i=0; i<spinand->page_size/4; i++)
	{
		mmio_write_32(REG_QSPI0_TX, 0x00);
		while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);
		*buf++ = mmio_read_32(REG_QSPI0_RX);
	}
	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
	// set DWIDTH to 8 bit and disable byte reorder
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL) & ~0x80000) | (8<<8));


	return 0;
}


static int ma35d1_spinand_read_oob(struct ma35d1_qspi_info *spinand, unsigned int addr, unsigned int len, unsigned char *buf)
{
	unsigned char cmd[4];

	cmd[0] = 0x13;  /* page read */
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;
	nus3500_spi_sendcmd(cmd, 4, 0, 0);
	while(ma35d1_spinand_getstatus(0xc0) & 0x1);    // wait ready
	if (ma35d1_spinand_getstatus(0xc0) & 0x20)
	{
		ERROR("Err-ECC\n");
		return 1;
	}
	/* read data */
	cmd[0] = 0x03;  /* page read */
	cmd[1] = (spinand->page_size >> 8) & 0xff;
	cmd[2] = spinand->page_size & 0xff;
	cmd[3] = 0;
	nus3500_spi_sendcmd(cmd, 4, buf, len);
	return 0;
}


static int ma35d1_spinand_quadread(struct ma35d1_qspi_info *spinand, unsigned int addr, unsigned int *buf)
{
	int volatile i;
	unsigned char cmd[4];

	cmd[0] = 0x13;  /* page read */
	cmd[1] = (addr >> 16) & 0xFF;
	cmd[2] = (addr >> 8) & 0xFF;
	cmd[3] = addr & 0xFF;
	nus3500_spi_sendcmd(cmd, 4, 0, 0);
	while(ma35d1_spinand_getstatus(0xc0) & 0x1);    // wait ready
	if (ma35d1_spinand_getstatus(0xc0) & 0x20)
	{
		ERROR("Err-ECC\n");
		return 1;
	}
	/* read data */
	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low
	mmio_write_32(REG_QSPI0_TX, spinand->QuadReadCmd);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01){}

	if (spinand->QuadReadCmd == CMD_READ_QUAD_IO_FAST)
	{
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) & (~0x1));
		while(mmio_read_32(REG_QSPI0_STATUS) & 0x8000);
		mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL) & ~0xf00000) | 0x500000); // quad output mode
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x1);
		while((mmio_read_32(REG_QSPI0_STATUS) & 0x8000) == 0);
	}
	// dummy byte (between command and address)
	for (i=0; i<spinand->dummybyte1; i++)
		mmio_write_32(REG_QSPI0_TX, 0x00);

	// column address
	mmio_write_32(REG_QSPI0_TX, 0x00);
	mmio_write_32(REG_QSPI0_TX, 0x00);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x20000); /* For 4-level FIFO buffer SPI port */

	// dummy byte (between address and data)
	for (i=0; i<spinand->dummybyte2; i++)
		mmio_write_32(REG_QSPI0_TX, 0x00);

	// wait tx finish
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01){}

	// clear RX buffer
	mmio_write_32(REG_QSPI0_FIFOCTL, mmio_read_32(REG_QSPI0_FIFOCTL) | 0x1);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x800000);

	/* Enable Quad IO input mode */
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) & (~0x1));
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x8000);
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL)& ~0x101ff0)| 0x480000);
	if (spinand->SuspendInterval == 0xff)
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x10);
	else
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | ((spinand->SuspendInterval & 0xf) << 4));
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x1);
	while((mmio_read_32(REG_QSPI0_STATUS) & 0x8000) == 0);
	mmio_write_32(SYS_GPD_MFPL, 0x00555555);

	// read data
	for (i=0; i<spinand->page_size/4; i++)
	{
		mmio_write_32(REG_QSPI0_TX, 0x00);
		while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);
		*buf++ = mmio_read_32(REG_QSPI0_RX);
	}

	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) & (~0x1));
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x8000);
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL)& ~0x4800f0) | 0x800); // disable quad mode
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x1);
	while((mmio_read_32(REG_QSPI0_STATUS) & 0x8000) == 0);
	mmio_write_32(SYS_GPD_MFPL, 0x00005555);
	mmio_write_32(GPIOD_MODE, 0x500);
	mmio_write_32(GPIOD_DOUT, mmio_read_32(GPIOD_DOUT)|0x30);

	return 0;
}

static int ma35d1_spinand_read_pages(struct ma35d1_qspi_info *spinand, uintptr_t buf, int page_start, int page_count)
{
	while (page_count > 0)
	{
		if (spinand->is_quad) {
			if (ma35d1_spinand_quadread(spinand, page_start, (unsigned int *)buf)) {
				return -EBADMSG;
			}
		} else {
			if (ma35d1_spinand_singleread(spinand, page_start, (unsigned int *)buf)) {
				return -EBADMSG;
			}
		}
		buf += spinand->page_size;
		page_count--;
		page_start++;
	}
	return 0;
}


static int ma35d1_spinand_block_isbad(struct ma35d1_qspi_info *spinand, unsigned int block)
{
	unsigned int page = block * spinand->pages_per_block;
	unsigned char data;

	ma35d1_spinand_read_oob(spinand, page, 1, &data);
	if (data != 0xff)
		return 1;   /* bad block */

	ma35d1_spinand_read_oob(spinand, page+1, 1, &data);
	if (data != 0xff)
		return 1;   /* bad block */

	return 0;   /* good block */
}


static size_t parse_spinand_read(struct ma35d1_qspi_info *spinand, int lba, uintptr_t buf, size_t size)
{
	int pages_per_block = spinand->pages_per_block;
	int page_size = spinand->page_size;
	int pages_to_read = div_round_up(size, page_size);
	int block_count = div_round_up(pages_to_read, pages_per_block);
	int page = lba % pages_per_block;
	int block = lba / pages_per_block;
	uintptr_t p = buf;
	int page_count, ret;

	while (pages_to_read) {
		ret = ma35d1_spinand_block_isbad(spinand, block);
		if (ret) {
			block++;
			if ((--block_count) <= 0)
				goto out;
			else
				continue;
		}

		page_count = MIN(pages_per_block - page, pages_to_read);

		ret = ma35d1_spinand_read_pages(spinand, p, block * pages_per_block + page, page_count);
		if (ret)
			goto out;

		block++;
		page = 0;
		p += page_size * page_count;
		pages_to_read -= page_count;
	}

out:
	/* number of read bytes */
	return MIN(size, p - buf);
}

int ma35d1_qspi_init(void)
{
	/* enable qspi0/gpiod clock */
	mmio_write_32(CLK_SYSCLK1, mmio_read_32(CLK_SYSCLK1) | (1 << 19));
	mmio_write_32(CLK_APBCLK1, mmio_read_32(CLK_APBCLK1) | (1 << 6));

	/* set MFP */
	mmio_write_32(SYS_GPD_MFPL, 0x00005555);

	/* Set PD.1(QSPI0_CLK) high slew rate */
	mmio_write_32(GPIOD_SLEWCTL, 0x4);
	mmio_write_32(GPIOD_MODE, 0x500);
	mmio_write_32(GPIOD_DOUT, mmio_read_32(GPIOD_DOUT) | 0x30);

	/* QSPI0 clock source: PCLK0 (SYSCLK1, SYSCLK1 is SYSPLL) */
	if (mmio_read_32(SYS_PWRONOTP) & 0x2)	/* SYS_PWRONOTP[1] */
		mmio_write_32(REG_QSPI0_CLKDIV, 2);	/* Set SPI0 clock to 60 MHz => PCLK(180)/(n+1) */
	else
		mmio_write_32(REG_QSPI0_CLKDIV, 4);	/* Set SPI0 clock to 36 MHz => PCLK(180)/(n+1) */

	/* Default setting: slave selection signal is active low; disable automatic slave selection function. */
	mmio_write_32(REG_QSPI0_SSCTL, 0);
	/* Default setting: MSB first, disable unit transfer interrupt, SP_CYCLE = 0. */
	mmio_write_32(REG_QSPI0_CTL, 0x805);

	return 0;
}

static void *fdt = (void *)(uintptr_t)MA35D1_DTB_BASE;

static void ma35d1_spinand_setup(struct ma35d1_qspi_info *spinand)
{
	int node;

	ma35d1_qspi_init();

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d1-qspi");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d1-qspi` not found\n");
	}

	spinand->page_size = fdt_read_uint32_default(fdt, node, "spinand-page-size", 2048);
	spinand->pages_per_block = fdt_read_uint32_default(fdt, node, "spinand-page-count", 64);
	spinand->oob_size = fdt_read_uint32_default(fdt, node, "spinand-oob-size", 0);
	spinand->size = spinand->page_size * spinand->pages_per_block * fdt_read_uint32_default(fdt, node, "spinand-block-count", 256);
	spinand->offset = fdt_read_uint32_default(fdt, node, "spi-image-offset", 0);

	spinand->QuadReadCmd = fdt_read_uint32_default(fdt, node, "spi-quad-read-cmd", 0);
	spinand->dummybyte1 = fdt_read_uint32_default(fdt, node, "spi-dummy1", 0);
	spinand->dummybyte2 = fdt_read_uint32_default(fdt, node, "spi-dummy2", 0);
	spinand->SuspendInterval = fdt_read_uint32_default(fdt, node, "spi-suspend-interval", 0);

	INFO("SPINAND: Size %liMB, Page %i, pages per block %i, oob size %i\n", (spinand->size/1024)/1024, spinand->page_size,
		spinand->pages_per_block, spinand->oob_size);

	ma35d1_spinand_quad_disable();
	/* reset spi nand */
	ma35d1_spinand_reset();
	while(1)
	{
		if ((ma35d1_spinand_getstatus(0xc0) & 0x1) == 0)
			break;
	}

	if (spinand->is_quad) {
		ma35d1_spinand_quad_enable();
	}
}

/***************************************************************/
static int ma35d1_spinor_reset(void)
{
	/* reset SPI flash */
	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low
	mmio_write_32(REG_QSPI0_TX, 0x66);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);    // wait tx finish
	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
	udelay(50);

	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low
	mmio_write_32(REG_QSPI0_TX, 0x99);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);    // wait tx finish
	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
	udelay(50);

	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low
	mmio_write_32(REG_QSPI0_TX, 0xf0);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);    // wait tx finish
	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
	udelay(50);

	return 0;

}


int spinor_single_read(unsigned int addr, unsigned int len, unsigned int *buf)
{
	int volatile i;
	unsigned int count;

	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low

	mmio_write_32(REG_QSPI0_TX, CMD_READ_ARRAY_SLOW);
	mmio_write_32(REG_QSPI0_TX, (addr >> 16) & 0xFF);
	mmio_write_32(REG_QSPI0_TX, (addr >> 8) & 0xFF);
	mmio_write_32(REG_QSPI0_TX, addr & 0xFF);

	// wait tx finish
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01){}
	// clear RX buffer
	mmio_write_32(REG_QSPI0_FIFOCTL, mmio_read_32(REG_QSPI0_FIFOCTL) | 0x1);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x800000);

	// set DWIDTH to 32 bit and enable byte reorder
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL) & ~0x1F00) | (1<<19));

	// read data
	count = (len / 4) + 1;
	for (i=0; i<count; i++)
	{
		mmio_write_32(REG_QSPI0_TX, 0x00);
		while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);
		*buf++ = mmio_read_32(REG_QSPI0_RX);
	}
	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
	// set DWIDTH to 8 bit and disable byte reorder
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL) & ~0x80000) | (8<<8));

	return 0;
}

int ma35d1_spinor_quad_enable(struct ma35d1_qspi_info *spinor)
{
	unsigned char data[2];
	unsigned char cmd[4];

	/* write enable */
	cmd[0] = CMD_WRITE_ENABLE;
	nus3500_spi_sendcmd(cmd, 1, 0, 0);

	if (spinor->ReadStatusCmd == CMD_READ_STATUS)
	{
		/* read status */
		cmd[0] = CMD_READ_STATUS;
		nus3500_spi_sendcmd(cmd, 1, &data[0], 1);
		cmd[0] = spinor->ReadStatusCmd;
		nus3500_spi_sendcmd(cmd, 1, &data[1], 1);

		/* write status */
		cmd[0] = spinor->WriteStatusCmd;
		cmd[1] = data[0];
		cmd[2] = data[1] | spinor->StatusValue;
		nus3500_spi_sendcmd(cmd, 3, 0, 0);
		while(1)
		{
			cmd[0] = CMD_READ_STATUS;
			nus3500_spi_sendcmd(cmd, 1, &data[0], 1);
			if ((data[0] & 0x1) == 0)
				break;
		}
	}
	else if (spinor->ReadStatusCmd == CMD_READ_EVCR)  /* micron */
	{
		/* read status */
		cmd[0] = CMD_READ_EVCR;
		nus3500_spi_sendcmd(cmd, 1, &data[0], 1);
		/* write status */
		cmd[0] = spinor->WriteStatusCmd;
		cmd[1] = data[0] & (~spinor->StatusValue);
		nus3500_spi_sendcmd(cmd, 2, 0, 0);
		while(1)
		{
			cmd[0] = CMD_READ_STATUS;
			nus3500_spi_sendcmd(cmd, 1, &data[0], 1);
			if ((data[0] & 0x1) == 0)
				break;
		}
	}
	else
	{
		ERROR("Err-Unknown\n");
		return 1;
	}
	return 0;
}

int spinor_quad_read(struct ma35d1_qspi_info *spinor, unsigned int addr, unsigned int len, unsigned int *buf)
{
	int volatile i;
	int volatile count;

	ma35d1_spinor_quad_enable(spinor);

	mmio_write_32(SYS_GPD_MFPL, 0x00555555);    /* set MFP */
	mmio_write_32(REG_QSPI0_SSCTL, 0x01);   // CS0 low
	mmio_write_32(REG_QSPI0_TX, spinor->QuadReadCmd);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01){}

	if (spinor->QuadReadCmd == CMD_READ_QUAD_IO_FAST)
	{
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) & (~0x1));
		while(mmio_read_32(REG_QSPI0_STATUS) & 0x8000);
		mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL) & ~0xf00000) | 0x500000); // quad output mode
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x1);
		while((mmio_read_32(REG_QSPI0_STATUS) & 0x8000) == 0);
	}
	// dummy byte (between command and address)
	for (i=0; i<spinor->dummybyte1; i++)
		mmio_write_32(REG_QSPI0_TX, 0x00);

	// address
	mmio_write_32(REG_QSPI0_TX, (addr>>16) & 0xFF);
	mmio_write_32(REG_QSPI0_TX, (addr>>8)  & 0xFF);
	mmio_write_32(REG_QSPI0_TX,  addr      & 0xFF);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x20000); /* For 4-level FIFO buffer SPI port */

	// dummy byte (between address and data)
	for (i=0; i<spinor->dummybyte2; i++)
		mmio_write_32(REG_QSPI0_TX, 0x00);

	// wait tx finish
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x01){}

	// clear RX buffer
	mmio_write_32(REG_QSPI0_FIFOCTL, mmio_read_32(REG_QSPI0_FIFOCTL) | 0x1);
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x800000);

	/* Enable Quad IO input mode */
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) & (~0x1));
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x8000);
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL)& ~0x101ff0)| 0x480000);
	if (spinor->SuspendInterval == 0xff)
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x10);
	else
		mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | ((spinor->SuspendInterval & 0xf) << 4));
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x1);
	while((mmio_read_32(REG_QSPI0_STATUS) & 0x8000) == 0);

	count = (len / 4) + 1;
	// read data
	for (i=0; i<count; i++)
	{
		mmio_write_32(REG_QSPI0_TX, 0x00);
		while(mmio_read_32(REG_QSPI0_STATUS) & 0x01);
		*buf++ = mmio_read_32(REG_QSPI0_RX);
	}

	mmio_write_32(REG_QSPI0_SSCTL, 0x05);   // CS0 high
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) & (~0x1));
	while(mmio_read_32(REG_QSPI0_STATUS) & 0x8000);
	mmio_write_32(REG_QSPI0_CTL, (mmio_read_32(REG_QSPI0_CTL)& ~0x4800f0) | 0x800); // disable quad mode and set 8-bit
	mmio_write_32(REG_QSPI0_CTL, mmio_read_32(REG_QSPI0_CTL) | 0x1);
	while((mmio_read_32(REG_QSPI0_STATUS) & 0x8000) == 0);

	// disable quad mode
	ma35d1_spinor_reset();

	return 0;
}


static size_t parse_spinor_read(struct ma35d1_qspi_info *spinor, int lba, uintptr_t buf, size_t size)
{
	if (spinor->is_quad)
		spinor_quad_read(spinor, lba*SPINOR_BLOCK_SIZE, size, (unsigned int *)buf);
	else
		spinor_single_read(lba*SPINOR_BLOCK_SIZE, size, (unsigned int *)buf);

	/* number of read bytes */
	return size;
}


static void ma35d1_spinor_setup(struct ma35d1_qspi_info *spinor)
{
	int node;

	ma35d1_qspi_init();

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d1-qspi");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d1-qspi` not found\n");
	}

	spinor->offset = fdt_read_uint32_default(fdt, node, "spi-image-offset", 0);
	spinor->QuadReadCmd = fdt_read_uint32_default(fdt, node, "spi-quad-read-cmd", 0);
	spinor->ReadStatusCmd = fdt_read_uint32_default(fdt, node, "spi-status-read-cmd", 0);
	spinor->WriteStatusCmd = fdt_read_uint32_default(fdt, node, "spi-status-write-cmd", 0);
	spinor->StatusValue = fdt_read_uint32_default(fdt, node, "spi-status", 0);
	spinor->dummybyte1 = fdt_read_uint32_default(fdt, node, "spi-dummy1", 0);
	spinor->dummybyte2 = fdt_read_uint32_default(fdt, node, "spi-dummy2", 0);
	spinor->SuspendInterval = fdt_read_uint32_default(fdt, node, "spi-suspend-interval", 0);

	/* reset spi nor */
	ma35d1_spinor_reset();

}

/***************************************************************/
static size_t ma35d1_spinand_read(int lba, uintptr_t buf, size_t size)
{
	size_t count = 0;

	inv_dcache_range(buf, size);

	count = parse_spinand_read(&ma35d1_qspi, lba, buf, size);

	inv_dcache_range(buf, size);

	return count;
}


static struct io_block_dev_spec spinand_dev_spec = {
	.ops = {
		.read = ma35d1_spinand_read,
	},
	/* fill .block_size at run-time */
};

int ma35d1_spinand_init(struct io_block_dev_spec **block_dev_spec, long *offset, int is_quad)
{
	ma35d1_qspi.is_quad = is_quad;
	ma35d1_spinand_setup(&ma35d1_qspi);
	*offset = ma35d1_qspi.offset;

	spinand_dev_spec.block_size = ma35d1_qspi.page_size;

	*block_dev_spec = &spinand_dev_spec;

	return 0;
}

/***************************************************************/

static size_t ma35d1_spinor_read(int lba, uintptr_t buf, size_t size)
{
	size_t count = 0;

	inv_dcache_range(buf, size);

	count = parse_spinor_read(&ma35d1_qspi, lba, buf, size);
printf("lba 0x%x, buf 0x%lx, size 0x%lx, cnt 0x%lx\n", lba, buf, size, count);

	inv_dcache_range(buf, size);

	return count;
}


static struct io_block_dev_spec spinor_dev_spec = {
	.ops = {
		.read = ma35d1_spinor_read,
	},
	.block_size = SPINOR_BLOCK_SIZE,
};

int ma35d1_spinor_init(struct io_block_dev_spec **block_dev_spec, long *offset, int is_quad)
{
	ma35d1_qspi.is_quad = is_quad;
	ma35d1_spinor_setup(&ma35d1_qspi);
	*offset = ma35d1_qspi.offset;

	*block_dev_spec = &spinor_dev_spec;

	return 0;
}
