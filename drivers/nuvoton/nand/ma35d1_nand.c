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

#include "ma35d1_nand.h"


struct ma35d1_nand_info {
	int pages_per_block;
	int page_size;
	int oob_size;
	int max_bit_corr;
	long size;	/* nand total size */
	long offset;	/* image offset in nand */
};

struct ma35d1_nand_info ma35d1_nand;


/*--------------------------------------------------*/
// define the total padding bytes for 512/1024 data segment
#define BCH_PADDING_LEN_512     32
#define BCH_PADDING_LEN_1024    64
// define the BCH parity code length for 512 bytes data pattern
#define BCH_PARITY_LEN_T8       15
#define BCH_PARITY_LEN_T12      23
// define the BCH parity code length for 1024 bytes data pattern
#define BCH_PARITY_LEN_T24      45


/*********************************************************/
int ma35d1_nand_wait_ready(unsigned long delay)
{
#if 0
	uint64_t timeout;

	timeout = timeout_init_us(delay);
	while (!timeout_elapsed(timeout)) {
	while (1) {
		if ((mmio_read_32(REG_NANDINTSTS) & 0x400)) {
			mmio_write_32(REG_NANDINTSTS, 0x400);
			return 0;
		}
	}
	return -ETIMEDOUT;
#endif
	while (1) {
		if ((mmio_read_32(REG_NANDINTSTS) & 0x400)) {
			mmio_write_32(REG_NANDINTSTS, 0x400);
			return 0;
		}
	}
	return 0;
}

int ma35d1_nand_reset(void)
{
	mmio_write_32(REG_NANDINTSTS, 0x400);   /* RB0_IF */

	mmio_write_32(REG_NANDCMD, NAND_CMD_RESET);       /* RESET command */

	/* delay for NAND flash tWB time */
	//udelay(100);

	if (ma35d1_nand_wait_ready(1))
		return -ETIMEDOUT;
	else
		return 0;
}

static void ma35d1_nand_read_oob(struct ma35d1_nand_info *nand, unsigned int page)
{
	int volatile i;
	unsigned char *ptr;

	ma35d1_nand_reset();

	/* set READ command */
	mmio_write_32(REG_NANDCMD, NAND_CMD_READ_1ST);       // READ 1st cycle command
	mmio_write_32(REG_NANDADDR, nand->page_size & 0xff);
	mmio_write_32(REG_NANDADDR, (nand->page_size >> 8) & 0xff);

	mmio_write_32(REG_NANDADDR, page & 0xff);
	if (nand->size > SZ_128M) {
		mmio_write_32(REG_NANDADDR, (page >> 8) & 0xff);
		mmio_write_32(REG_NANDADDR, ((page >> 16) & 0xff) | NAND_EOA);
	} else {
		mmio_write_32(REG_NANDADDR, ((page >> 8) & 0xff) | NAND_EOA);
	}

	mmio_write_32(REG_NANDCMD, NAND_CMD_READ_2ND);       // READ 2nd cycle command

	ma35d1_nand_wait_ready(1);

	ptr = (unsigned char *)REG_NANDRA0;
	for (i=0; i<nand->oob_size; i++)
		*(unsigned char*) ptr++ = inp8(REG_NANDDATA);
}


static void ma35d1_correct_data(unsigned char ucFieidIndex, unsigned char ucErrorCnt, unsigned char* pDAddr)
{
	unsigned int uaData[24], uaAddr[24];
	unsigned int uaErrorData[6];
	unsigned char  i, j;
	unsigned int uPageSize;
	unsigned int field_len, padding_len, parity_len;
	unsigned int total_field_num;
	unsigned char  *smra_index;

	//--- assign some parameters for different BCH and page size
	field_len   = 512;
	padding_len = BCH_PADDING_LEN_512;

	switch (mmio_read_32(REG_NANDCTL) & BCH_TSEL)
	{
		case BCH_24:
			field_len   = 1024;
			padding_len = BCH_PADDING_LEN_1024;
			parity_len  = BCH_PARITY_LEN_T24;
			break;
		case BCH_12:
			parity_len  = BCH_PARITY_LEN_T12;
			break;
		case BCH_8:
			parity_len  = BCH_PARITY_LEN_T8;
			break;
		default:
			WARN("error BCH\n");
			panic();
	}

	uPageSize = mmio_read_32(REG_NANDCTL) & PSIZE;
	switch (uPageSize)
	{
		case PSIZE_8K:  total_field_num = 8192 / field_len; break;
		case PSIZE_4K:  total_field_num = 4096 / field_len; break;
		case PSIZE_2K:  total_field_num = 2048 / field_len; break;
		default:
			return;
	}

	//--- got valid BCH_ECC_DATAx and parse them to uaData[]
	// got the valid register number of BCH_ECC_DATAx since one register include 4 error bytes
	j = ucErrorCnt/4;
	j ++;
	if (j > 6)
		j = 6;     // there are 6 BCH_ECC_DATAx registers to support BCH T24

	for(i=0; i<j; i++)
	{
		uaErrorData[i] = mmio_read_32(REG_NANDECCED0 + i*4);
	}

	for(i=0; i<j; i++)
	{
		uaData[i*4+0] = uaErrorData[i] & 0xff;
		uaData[i*4+1] = (uaErrorData[i]>>8) & 0xff;
		uaData[i*4+2] = (uaErrorData[i]>>16) & 0xff;
		uaData[i*4+3] = (uaErrorData[i]>>24) & 0xff;
	}

	//--- got valid REG_BCH_ECC_ADDRx and parse them to uaAddr[]
	// got the valid register number of REG_BCH_ECC_ADDRx since one register include 2 error addresses
	j = ucErrorCnt/2;
	j ++;
	if (j > 12)
		j = 12;    // there are 12 REG_BCH_ECC_ADDRx registers to support BCH T24

	for(i=0; i<j; i++)
	{
		uaAddr[i*2+0] = mmio_read_32(REG_NANDECCEA0 + i*4) & 0x07ff;   // 11 bits for error address
		uaAddr[i*2+1] = (mmio_read_32(REG_NANDECCEA0 + i*4)>>16) & 0x07ff;
	}

	//--- pointer to begin address of field that with data error
	pDAddr += (ucFieidIndex-1) * field_len;

	//--- correct each error bytes
	for(i=0; i<ucErrorCnt; i++)
	{
		// for wrong data in field
		if (uaAddr[i] < field_len)
		{
			*(pDAddr+uaAddr[i]) ^= uaData[i];
		}
		// for wrong first-3-bytes in redundancy area
		else if (uaAddr[i] < (field_len+3))
		{
			uaAddr[i] -= field_len;
			uaAddr[i] += (parity_len*(ucFieidIndex-1));    // field offset

			*((unsigned char *)REG_NANDRA0+uaAddr[i]) ^= uaData[i];
		}
		// for wrong parity code in redundancy area
		else
		{
			// BCH_ERR_ADDRx = [data in field] + [3 bytes] + [xx] + [parity code]
			//                                   |<--     padding bytes      -->|
			// The BCH_ERR_ADDRx for last parity code always = field size + padding size.
			// So, the first parity code = field size + padding size - parity code length.
			// For example, for BCH T12, the first parity code = 512 + 32 - 23 = 521.
			// That is, error byte address offset within field is
			uaAddr[i] = uaAddr[i] - (field_len + padding_len - parity_len);

			// smra_index point to the first parity code of first field in register SMRA0~n
			smra_index = (unsigned char *)(long)(REG_NANDRA0 + (mmio_read_32(REG_NANDRACTL) & REA128_EXT) - // bottom of all parity code -
						  (parity_len * total_field_num)                            // byte count of all parity code
						 );

			// final address = first parity code of first field +
			//                 offset of fields +
			//                 offset within field
			*((unsigned char *)smra_index + (parity_len * (ucFieidIndex-1)) + uaAddr[i]) ^= uaData[i];
		}
	}   // end of for (i<ucErrorCnt)
}


static int ma35d1_nand_read_page(struct ma35d1_nand_info *nand, unsigned int page, uintptr_t buffer)
{
	unsigned int volatile uStatus, uErrorCnt;
	unsigned int volatile uF1_status;
	unsigned char volatile i, j, uLoop;

	//INFO(">%s page %i buffer %lx\n", __func__, page, buffer);

	if (nand->max_bit_corr != 0)    /* ECC_EN */
		ma35d1_nand_read_oob(nand, page);

	mmio_write_32(REG_NANDCTL, mmio_read_32(REG_NANDCTL) & ~REDUN_REN);
	/* enable DMAC */
	mmio_write_32(REG_FMI_DMACTL, 0x1); /* enable DMAC */

	/* Set DMA Transfer Starting Address */
	mmio_write_32(REG_FMI_DMASA, buffer);
	mmio_write_32(REG_NANDCMD, NAND_CMD_READ_1ST);       // READ 1st cycle command
	mmio_write_32(REG_NANDADDR, 0);
	mmio_write_32(REG_NANDADDR, 0);
	mmio_write_32(REG_NANDADDR, page & 0xff);
	if (nand->size > SZ_128M) {
		mmio_write_32(REG_NANDADDR, (page >> 8) & 0xff);
		mmio_write_32(REG_NANDADDR, ((page >> 16) & 0xff) | NAND_EOA);
	} else {
		mmio_write_32(REG_NANDADDR, ((page >> 8) & 0xff) | NAND_EOA);
	}

	mmio_write_32(REG_NANDCMD, NAND_CMD_READ_2ND);       // READ 2nd cycle command

	ma35d1_nand_wait_ready(1);

	uF1_status = 0;
	uStatus = 0;
	/* begin DMA read transfer */
	mmio_write_32(REG_NANDINTSTS, 0x5);     /* clear DMA and ECC_Field flag */
	mmio_write_32(REG_NANDCTL, mmio_read_32(REG_NANDCTL) | 0x2);    /* READ enable */

	while(1) {
		if (nand->max_bit_corr) {
			if (mmio_read_32(REG_NANDINTSTS) & 0x4) {
				switch (mmio_read_32(REG_NANDCTL) & PSIZE) {
					case PSIZE_2K:
						uLoop = 1;
						break;
					case PSIZE_4K:
						if ((mmio_read_32(REG_NANDCTL) & BCH_TSEL) == BCH_24)
							uLoop = 1;
						else
							uLoop = 2;
						break;
					case PSIZE_8K:
						if ((mmio_read_32(REG_NANDCTL) & BCH_TSEL) == BCH_24)
							uLoop = 2;
						else
							uLoop = 4;
						break;
				}

				for (j=0; j<uLoop; j++) {
					uF1_status = mmio_read_32(REG_NANDECCES0+j*4);
					if (!uF1_status)
						continue;   // no error on this register for 4 fields
					for (i=1; i<5; i++) {
						if (!(uF1_status & 0x3)) {     // no error for this field
							uF1_status >>= 8;  // next field
							continue;
						}

						if ((uF1_status & 0x3)==0x01) {
							/* correctable error in field (j*4+i) */
							uErrorCnt = (uF1_status >> 2) & 0x1F;
							ma35d1_correct_data(j*4+i, uErrorCnt, (unsigned char*)(long)buffer);
							break;
						} else if (((uF1_status & 0x3) == 0x02) || ((uF1_status & 0x3) == 0x03)) {
							/* uncorrectable error or ECC error in 1st field */
							uStatus = 1;
							break;
						}
						uStatus >>= 8;  // next field
					}
				}
				mmio_write_32(REG_NANDINTSTS, 0x4);     // clear ECC_FLD_Error
			}
		}

		if (mmio_read_32(REG_NANDINTSTS) & 0x1) {    // wait to finish DMAC transfer.
			if (nand->max_bit_corr) {
				if ( !(mmio_read_32(REG_NANDINTSTS) & 0x4) )
					break;
			} else
				break;
		}
	}

	mmio_write_32(REG_NANDINTSTS, 0x5); /* clear DMA and ECC flag */

	if (uStatus) {
		ERROR("Err-ECC 0x%x / 0x%x\n", uStatus, uF1_status);
		return 1;
	}

	return 0;
}

static int ma35d1_nand_read_pages(struct ma35d1_nand_info *nand, uintptr_t buf, int page_start, int page_count)
{
	while (page_count > 0)
	{
		if (ma35d1_nand_read_page(nand, page_start, buf)) {
			return -EBADMSG;
		}
		buf += nand->page_size;
		page_count--;
		page_start++;
	}
	return 0;
}

static int ma35d1_block_isbad(struct ma35d1_nand_info *nand, unsigned int pba)
{
	unsigned int page, data;

	/* check first 2 page and last 2 page */
	page = pba * nand->pages_per_block;
	ma35d1_nand_read_oob(nand, page);
	data = mmio_read_32(REG_NANDRA0) & 0xff;
	if (data != 0xFF)
		return 1;   /* invalid */

	ma35d1_nand_read_oob(nand, page+1);
	data = mmio_read_32(REG_NANDRA0) & 0xff;
	if (data != 0xFF)
		return 1;   /* invalid */

	ma35d1_nand_read_oob(nand, page+nand->pages_per_block-1);
	data = mmio_read_32(REG_NANDRA0) & 0xff;
	if (data != 0xFF)
		return 1;   /* invalid */

	ma35d1_nand_read_oob(nand, page+nand->pages_per_block-2);
	data = mmio_read_32(REG_NANDRA0) & 0xff;
	if (data != 0xFF)
		return 1;   /* invalid */

	ma35d1_nand_reset();

	return 0;   /* good block */
}


static size_t parse_nand_read(struct ma35d1_nand_info *nand, int lba, uintptr_t buf, size_t size)
{
	int pages_per_block = nand->pages_per_block;
	int page_size = nand->page_size;
	int pages_to_read = div_round_up(size, page_size);
	int block_count = div_round_up(pages_to_read, pages_per_block);
	int page = lba % pages_per_block;
	int block = lba / pages_per_block;
	uintptr_t p = buf;
	int page_count, ret;

	while (pages_to_read) {
		ret = ma35d1_block_isbad(nand, block);
		if (ret) {
			block++;
			if ((--block_count) <= 0)
				goto out;
			else
				continue;
		}

		page_count = MIN(pages_per_block - page, pages_to_read);

		ret = ma35d1_nand_read_pages(nand, p, block * pages_per_block + page, page_count);
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

int ma35d1_nand_ctrl_init(void)
{
	/* enable nand clock */
	mmio_write_32(CLK_SYSCLK0, mmio_read_32(CLK_SYSCLK0) | (1 << 18));

	/* set MFP */
	mmio_write_32(SYS_GPA_MFPL, 0x66666666);
	mmio_write_32(SYS_GPA_MFPH, 0x06666666);

	// reset FMI
	mmio_write_32(SYS_IPRST0, 0x4000000);
	mmio_write_32(SYS_IPRST0, 0x0000000);

	// DMAC Initial
	mmio_write_32(REG_FMI_DMACTL, 0x00000001);

	// enable SM
	mmio_write_32(REG_FMI_CTL, 0x08);

	/* enable CS0 */
	mmio_write_32(REG_NANDCTL, mmio_read_32(REG_NANDCTL) & ~0x02000000);

	return 0;
}

static void *fdt = (void *)(uintptr_t)MA35D1_DTB_BASE;

static void ma35d1_nand_setup(struct ma35d1_nand_info *nand)
{
	unsigned int reg = 0x00910090; // REG_NANDCTL default value
	int node;

	ma35d1_nand_ctrl_init();

	/* get device tree information */
	if (fdt_check_header(fdt) < 0) {
		WARN("device tree header check error.\n");
	}

	node = fdt_node_offset_by_compatible(fdt, -1, "nuvoton,ma35d1-nand");
	if (node < 0) {
		WARN("The compatible property `nuvoton,ma35d1-nand` not found\n");
	}

	nand->max_bit_corr = fdt_read_uint32_default(fdt, node, "nand-ecc-strength", 8);
	nand->page_size = fdt_read_uint32_default(fdt, node, "nand-page-size", 2048);
	nand->pages_per_block = fdt_read_uint32_default(fdt, node, "nand-page-count", 64);
	nand->oob_size = fdt_read_uint32_default(fdt, node, "nand-oob-size", 0);
	nand->size = nand->page_size * nand->pages_per_block * fdt_read_uint32_default(fdt, node, "nand-block-count", 256);
	nand->offset = fdt_read_uint32_default(fdt, node, "nand-image-offset", 0);

	INFO("NAND: Size %liMB, Page %i, pages per block %i, oob size %i, bit correct %i\n",
	     (nand->size / 1024) / 1024, nand->page_size,
	     nand->pages_per_block, nand->oob_size, nand->max_bit_corr);

	switch (nand->page_size) {
	case 2048:
		reg = (reg & (~0x30000)) | 0x10000;
		switch (nand->max_bit_corr) {
		case 0:
			reg &= ~ECC_EN;
			nand->oob_size = 8;
			break;
		case 8:
			reg = (reg & ~0x7C0000) | BCH_8;
			reg |= ECC_EN;
			break;

		case 12:
			reg = (reg & ~0x7C0000) | BCH_12;
			reg |= ECC_EN;
			break;

		case 24:
			reg = (reg & ~0x7C0000) | BCH_24;
			reg |= ECC_EN;
			break;
		default:
			WARN("error BCH in 2KB page size\n");
		}
		break;

	case 4096:
		reg = (reg & (~0x30000)) | 0x20000;
		switch (nand->max_bit_corr) {
		case 0:
			reg &= ~ECC_EN;
			nand->oob_size = 8;
			break;
		case 8:
			reg = (reg & ~0x7C0000) | BCH_8;
			reg |= ECC_EN;
			break;

		case 12:
			reg = (reg & ~0x7C0000) | BCH_12;
			reg |= ECC_EN;
			break;

		case 24:
			reg = (reg & ~0x7C0000) | BCH_24;
			reg |= ECC_EN;
			break;
		default:
			WARN("error BCH in 4KB page size\n");
		}
		break;

	case 8192:
		reg = (reg & (~0x30000)) | 0x30000;
		switch (nand->max_bit_corr) {
		case 0:
			reg &= ~ECC_EN;
			nand->oob_size = 8;
			break;
		case 8:
			reg = (reg & ~0x7C0000) | BCH_8;
			reg |= ECC_EN;
			break;

		case 12:
			reg = (reg & ~0x7C0000) | BCH_12;
			reg |= ECC_EN;
			break;

		case 24:
			reg = (reg & ~0x7C0000) | BCH_24;
			reg |= ECC_EN;
			break;
		default:
			WARN("error BCH in 8KB page size\n");
		}
		break;

	default:
		WARN("NAND Controller is not support this PAGE SIZE. (%d)\n", nand->page_size);
	}

	mmio_write_32(REG_NANDCTL, reg|PROT_3BEN|REDUN_AUTO_WEN);
	mmio_write_32(REG_NANDRACTL, mmio_read_32(REG_NANDRACTL) & ~MECC_Msk);
	mmio_write_32(REG_NANDRACTL, nand->oob_size);
}

static size_t ma35d1_nand_read(int lba, uintptr_t buf, size_t size)
{
	size_t count = 0;

	inv_dcache_range(buf, size);

	count = parse_nand_read(&ma35d1_nand, lba, buf, size);

	inv_dcache_range(buf, size);

	return count;
}


static struct io_block_dev_spec nand_dev_spec = {
	.ops = {
		.read = ma35d1_nand_read,
	},
	/* fill .block_size at run-time */
};

int ma35d1_nand_init(struct io_block_dev_spec **block_dev_spec, long *offset)
{
	ma35d1_nand_setup(&ma35d1_nand);
	*offset = ma35d1_nand.offset;

	nand_dev_spec.block_size = ma35d1_nand.page_size;

	*block_dev_spec = &nand_dev_spec;

	return 0;
}
