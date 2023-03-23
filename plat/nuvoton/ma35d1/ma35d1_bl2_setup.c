/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <drivers/arm/sp804_delay_timer.h>
#include <common/desc_image_load.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <common/debug.h>
#include <lib/optee_utils.h>

#include "ma35d1_private.h"
#include <ma35d1_crypto.h>
#include <tsi_cmd.h>

#if FIP_DE_AES
static __inline unsigned int Swap32(unsigned int val)
{
	return (val<<24) | ((val<<8)&0xff0000) | ((val>>8)&0xff00) | (val>>24);
}


int ma35d1_fip_verify(uintptr_t base, size_t size) {
	int j, k;
	IMAGE_INFO_T *image_info;
	unsigned int shaDigest[8], tmpbuf[8], *ptr;
	unsigned char image[256];
	unsigned int volatile u32SysCfg;
	char *param = (char *)(TSI_PARAM_BASE);

	if (size <= 0)
		return 0;

	image_info = (IMAGE_INFO_T *)image;
	memcpy(image,(unsigned char *)base+size-sizeof(IMAGE_INFO_T),sizeof(IMAGE_INFO_T));
	u32SysCfg = mmio_read_32(SYS_CHIPCFG);
	if ((u32SysCfg & 0x100) == 0x000)       /* 0: TSI; 1: crypto */
	{
		/* Enable whc0 clock */
		mmio_write_32(CLK_SYSCLK1, (mmio_read_32(CLK_SYSCLK1) | 0x10));
		/* connect with TSI */
		while (1)
		{
			if (TSI_Sync() == 0)
			{
				INFO("TSI Connected\n");
				break;
			}
		}
		if (TSI_run_sha(	1,                          /* inswap        */
					1,                          /* outswap       */
					0,                          /* mode_sel      */
					0,                          /* hmac          */
					SHA_MODE_SHA256,            /* mode          */
					0,                          /* keylen (hmac) */
					0,                          /* ks            */
					0,                          /* ks_num        */
					8,                          /* wcnt          */
					(size-sizeof(IMAGE_INFO_T)),	/* data_cnt      */
					base,				/* src_addr      */
					(long)shaDigest		/* dest_addr     */
					) != 0)
		{
				printf("\nTSI SHA command failed!!\n");
				return 1;
		}

		/* ECC authenticate */
		/* parameters */
		ptr = (unsigned int *)shaDigest;
		for (j=7, k=0; j>=0; j--, k++)
			tmpbuf[j] = Swap32(*(ptr+k));
		Reg2Hex(64, tmpbuf, param);

		ptr = (unsigned int *)image_info->signatureR;
		for (j=7, k=0; j>=0; j--, k++)
			tmpbuf[j] = Swap32(*(ptr+k));
		Reg2Hex(64, tmpbuf, (param+1728));

		ptr = (unsigned int *)image_info->signatureS;
		for (j=7, k=0; j>=0; j--, k++)
			tmpbuf[j] = Swap32(*(ptr+k));
		Reg2Hex(64, tmpbuf, (param+2304));

		inv_dcache_range((uintptr_t)param, 4096);

		if(TSI_ECC_VerifySignature(0x3, 0x1, 6, 7, ((uint32_t)(uint64_t)(TSI_PARAM_BASE)))!=0)
			return 1;
	} else { /* crypto */

		/* initial crypto engine and ks clock */
		mmio_write_32((TSI_CLK_BASE+0x04),
			    (mmio_read_32((TSI_CLK_BASE+0x04)) | 0x5000));

                /* Init KeyStore */
                /* KS INIT(KS_CTL[8]) + START(KS_CTL[0]) */
		mmio_write_32((KS_BASE+0x00), 0x101);
		while ((mmio_read_32((KS_BASE+0x08)) & 0x80) == 0)
                        ;   /* wait for INITDONE(KS_STS[7]) set */
		while (mmio_read_32((KS_BASE+0x08)) & 0x4)
                        ;      /* wait for BUSY(KS_STS[2]) cleared */

		/* enable SHA, AES, and ECC */
		/* SHA mode 256, in/out swap, key len 0 */
		SHA_Open(SHA_MODE_SHA256, SHA_IN_OUT_SWAP, 0);

		/* calculate SHA-256 HASH */
		SHA_SetDMATransfer(base, size-sizeof(IMAGE_INFO_T));
		SHA_Start(CRYPTO_DMA_ONE_SHOT);

		SHA_Read(shaDigest);
		for (j=7, k=0; j>=0; j--, k++)
			tmpbuf[j] = Swap32(shaDigest[k]);
		Reg2Hex(64, tmpbuf, param);

		ptr = (unsigned int *)image_info->signatureR;
		for (j=7, k=0; j>=0; j--, k++)
			tmpbuf[j] = Swap32(*(ptr+k));
		Reg2Hex(64, tmpbuf, (param+128));

		ptr = (unsigned int *)image_info->signatureS;
		for (j=7, k=0; j>=0; j--, k++)
			tmpbuf[j] = Swap32(*(ptr+k));
		Reg2Hex(64, tmpbuf, (param+256));

		inv_dcache_range(base, size);
		if (ECC_VerifySignature_KS(param, 0x86, 0x87, (param+128), (param+256)) < 0)
			return 1;
	}

	return 0;
}

int ma35d1_fip_deaes(uintptr_t base, size_t size) {
	int sid, j;
	volatile unsigned char *param = (volatile unsigned char *)TSI_PARAM_BASE;
	unsigned int volatile u32SysCfg;

	if (size <= 0)
		return 0;
	u32SysCfg = mmio_read_32(SYS_CHIPCFG);
	if ((u32SysCfg & 0x100) == 0x000)       /* 0: TSI; 1: crypto */
	{

		/* AES decrypt */
		if (TSI_Open_Session(C_CODE_AES, &sid) != 0)
			return 1;
		inv_dcache_range(TSI_PARAM_BASE, 32);
		for (j=0; j<32; j++)
			*(param+j) = 0;
		inv_dcache_range(TSI_PARAM_BASE, 32);
		if (TSI_AES_Set_IV(sid, (unsigned int)TSI_PARAM_BASE) != 0)
			return 1;
		if (TSI_AES_Set_Mode(sid, 0, 0, 1, 1, 0, 0, 2, AES_KEY_SIZE_256, 5, 8) != 0)
			return 1;
		if (TSI_AES_Run(sid, 1, size-sizeof(IMAGE_INFO_T), base, base) != 0)
			return 1;
		TSI_Close_Session(C_CODE_AES, sid);
		inv_dcache_range(base, size);
	} else { /* crypto */

		/* AES, channel 0, AES decode, CFB mode, key 256, in/out swap */
		mmio_write_32(AES_IV(0), 0);
		mmio_write_32(AES_IV(1), 0);
		mmio_write_32(AES_IV(2), 0);
		mmio_write_32(AES_IV(3), 0);

		/* AES decrypt */
		mmio_write_32(AES_SADDR, base);
		mmio_write_32(AES_DADDR, base);
		mmio_write_32(AES_CNT, base);

		/* 0x2<<KSCTL_RSSRC_Pos | KSCTL_RSRC_Msk | 8 */
		mmio_write_32(AES_KSCTL,  (0x2<<6) | (0x1<<5) | 8); /* from KS_OTP */

		/* ((AES_MODE_CFB << CRPT_AES_CTL_OPMODE_Pos) |
		   (AES_KEY_SIZE_256 << CRPT_AES_CTL_KEYSZ_Pos)) |
		   CRPT_AES_CTL_INSWAP_Msk | CRPT_AES_CTL_OUTSWAP_Msk |
		   CRPT_AES_CTL_DMAEN_Msk | CRPT_AES_CTL_DMALAST_Msk | CRPT_AES_CTL_START_Msk  */
		mmio_write_32(AES_CTL, ((2<<8) | (2<<2)) | (1<<23) | (1<<22) |
				(1<<7) | (1<<5) | (1<<0));
		while (1)
		{
			/* CRPT_INTSTS_AESIF_Msk|CRPT_INTSTS_AESEIF_Msk */
			if (mmio_read_32(INTSTS) & ((1<<0) | (1<<1)))
			{
				mmio_write_32(INTSTS, ((1<<0) | (1<<1)));
			}
		}
		inv_dcache_range(base, size);
	}

	return 0;
}
#endif

/*
 * bl2_platform_setup()
 * MMU on - enabled by bl2_plat_arch_setup()
 */
void bl2_platform_setup(void)
{
}


void bl2_plat_arch_setup(void)
{
	/* Setup the MMU here */
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	/* Prevent corruption of preloaded Device Tree */
	mmap_add_region(DTB_BASE, DTB_BASE,
			DTB_LIMIT - DTB_BASE,
			MT_RO_DATA | MT_SECURE);

	/* config MMC */
	configure_mmu();

	generic_delay_timer_init();

	/* TODO: system config initial, power? */

	ma35d1_arch_security_setup();

	ma35d1_io_setup();
}

/* Load the M4 image into SRAM0 and, if needed, DDR while holding reset. */
int plat_ma35d1_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	if (scp_bl2_image_info->image_size > 0x80000) {	/* 512KB */
		WARN("SCP image exceeds 512 KiB\n");
		return -1;
	}

	/* unlock */
	mmio_write_32(SYS_RLKTZS, 0x59);
	mmio_write_32(SYS_RLKTZS, 0x16);
	mmio_write_32(SYS_RLKTZS, 0x88);

	/* Stop MCU - Enable M4 Core reset */
	mmio_write_32(SYS_IPRST0, mmio_read_32(SYS_IPRST0) | 0x8);

	/* Load MCU binary into SRAM and DDR, depend on image size */
	if (scp_bl2_image_info->image_size <= 0x20000) {	/* 128KB */
		memcpy((void *)0x24000000,
		       (const void *)scp_bl2_image_info->image_base,
		       scp_bl2_image_info->image_size);
	} else {
		memcpy((void *)0x24000000,
		       (const void *)scp_bl2_image_info->image_base, 0x20000);
		memcpy((void *)0x80020000,
		       (const uint8_t *)scp_bl2_image_info->image_base + 0x20000,
		       scp_bl2_image_info->image_size - 0x20000);
	}

	flush_dcache_range(0x24000000, scp_bl2_image_info->image_size);

	/* Enable RTP clock */
	mmio_write_32(CLK_SYSCLK0, mmio_read_32(CLK_SYSCLK0) | 0x2);

	/* lock */
	mmio_write_32(SYS_RLKTZS, 0);

	INFO("Load RTP M4\n");

	return 0;
}

static uint32_t ma35d1_get_spsr_for_bl32_entry(void)
{
	return 0;
}

static uint32_t ma35d1_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	bl_mem_params_node_t *pager_mem_params;
	bl_mem_params_node_t *paged_mem_params = NULL;

	assert(bl_mem_params != NULL);

#if FIP_DE_AES
	if(ma35d1_fip_verify(bl_mem_params->image_info.image_base,bl_mem_params->image_info.image_size)!=0) {
		ERROR("ECC authenticate fail.\n");
		while(1);
	}
	ma35d1_fip_deaes(bl_mem_params->image_info.image_base,bl_mem_params->image_info.image_size);
#endif

	switch (image_id) {
	case BL32_IMAGE_ID:
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
				&pager_mem_params->image_info,
				&paged_mem_params->image_info);
		if (err != 0) {
			WARN("OPTEE header parse error.\n");
		}

		/*
		 * Keep the architecture and pageable image arguments from the
		 * OP-TEE header for the upstream OPTEED handoff protocol.
		 */
		bl_mem_params->ep_info.args.arg3 = 0;

		bl_mem_params->ep_info.spsr = ma35d1_get_spsr_for_bl32_entry();

		break;

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = ma35d1_get_spsr_for_bl33_entry();
		break;

	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific. */
		err = plat_ma35d1_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err != 0) {
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
		break;

	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}
