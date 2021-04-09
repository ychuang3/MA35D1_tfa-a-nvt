/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <lib/utils_def.h>

#include <tools_share/firmware_image_package.h>

#include "ma35d1_private.h"

//#define NVT_FIP_MMAP

int ma35d1_nand_init(struct io_block_dev_spec **block_dev_spec, long *offset);
int ma35d1_spinand_init(struct io_block_dev_spec **block_dev_spec, long *offset, int is_quad);
int ma35d1_spinor_init(struct io_block_dev_spec **block_dev_spec, long *offset, int is_quad);
int ma35d1_sdhc_init(struct io_block_dev_spec **block_dev_spec, long *offset, int sdhc);

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;

#ifndef NVT_FIP_MMAP
static const io_dev_connector_t *backend_dev_con;
static uintptr_t backend_dev_handle;

static io_block_spec_t fip_block_spec = {
	.length = MA35D1_FIP_SIZE
};
#else
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_block_spec_t fip_block_spec = {
	.offset = MA35D1_FIP_BASE,
	.length = MA35D1_FIP_SIZE
};
#endif

static const io_uuid_spec_t bl31_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl32_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t bl32_extra1_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA1,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t trusted_key_cert_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t soc_fw_key_cert_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t tos_fw_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t soc_fw_cert_uuid_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t tos_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t nt_fw_key_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t nt_fw_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	uintptr_t init_params;
};

static const struct plat_io_policy policies[] = {
#ifndef NVT_FIP_MMAP
	[FIP_IMAGE_ID] = {
		.dev_handle = &backend_dev_handle,
		.image_spec = (uintptr_t)&fip_block_spec,
	},
#else
	[FIP_IMAGE_ID] = {
		.dev_handle = &memmap_dev_handle,
		.image_spec = (uintptr_t)&fip_block_spec,
	},
#endif
	[BL31_IMAGE_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&bl31_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL32_IMAGE_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&bl32_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&bl32_extra1_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL33_IMAGE_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&bl33_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_KEY_CERT_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&trusted_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SOC_FW_KEY_CERT_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&soc_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&tos_fw_key_cert_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&nt_fw_key_cert_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&soc_fw_cert_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&tos_fw_cert_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		.dev_handle = &fip_dev_handle,
		.image_spec = (uintptr_t)&nt_fw_cert_uuid_spec,
		.init_params = FIP_IMAGE_ID,
	},
#endif /* TRUSTED_BOARD_BOOT */
};

static int ma35d1_io_fip_setup(void)
{
	int ret;

	ret = register_io_dev_fip(&fip_dev_con);
	if (ret)
		return ret;

	return io_dev_open(fip_dev_con, 0, &fip_dev_handle);
}

#ifndef NVT_FIP_MMAP
static int ma35d1_io_block_setup(struct io_block_dev_spec *block_dev_spec)
{
	int ret;

	block_dev_spec->buffer.offset = MA35D1_FIP_BASE;
	block_dev_spec->buffer.length = MA35D1_FIP_SIZE;

	ret = register_io_dev_block(&backend_dev_con);
	if (ret)
		return ret;

	return io_dev_open(backend_dev_con, (uintptr_t)block_dev_spec,
			   &backend_dev_handle);
}

static int ma35d1_io_sdhc_setup(int sdhc)
{
	struct io_block_dev_spec *block_dev_spec;
	long offset;
	int ret;

	ret = ma35d1_sdhc_init(&block_dev_spec, &offset, sdhc);
	if (ret)
		return ret;
	fip_block_spec.offset = offset;

	return ma35d1_io_block_setup(block_dev_spec);
}

static int ma35d1_io_nand_setup(void)
{
	struct io_block_dev_spec *block_dev_spec;
	long offset;
	int ret;

	ret = ma35d1_nand_init(&block_dev_spec, &offset);
	if (ret)
		return ret;
	fip_block_spec.offset = offset;

	return ma35d1_io_block_setup(block_dev_spec);
}

static int ma35d1_io_spinand_setup(int is_quad)
{
	struct io_block_dev_spec *block_dev_spec;
	long offset;
	int ret;

	ret = ma35d1_spinand_init(&block_dev_spec, &offset, is_quad);
	if (ret)
		return ret;
	fip_block_spec.offset = offset;

	return ma35d1_io_block_setup(block_dev_spec);
}

static int ma35d1_io_spinor_setup(int is_quad)
{
	struct io_block_dev_spec *block_dev_spec;
	long offset;
	int ret;

	ret = ma35d1_spinor_init(&block_dev_spec, &offset, is_quad);
	if (ret)
		return ret;
	fip_block_spec.offset = offset;

	return ma35d1_io_block_setup(block_dev_spec);
}
#endif

void ma35d1_io_setup(void)
{
	int result __unused;
#ifndef NVT_FIP_MMAP
	unsigned int por;

	/* check power-on-setting from OTP or PIN */
	por = inp32(SYS_PWRONOTP);
	if ((por & 0x1) == 0) {    /* from pin */
		por = inp32(SYS_PWRONPIN);
		por = por << 8;
	}
	printf("POR: 0x%x\n", por);

	switch (por & PWRON_BOOT_MSK) {

	case PWRON_BOOT_SPI:
		if (por & PWRON_SPI_NOR) {
			if ((por & 0xC000) == PWRON_SPI_4_NOR) {
				INFO("Boot from 4-bit SPI-NOR\n");
				ma35d1_io_spinor_setup(1);
			} else {
				INFO("Boot from 1-bit SPI-NOR\n");
				ma35d1_io_spinor_setup(0);
			}
		} else {
			if ((por & 0x4000) == PWRON_SPI_1_NAND) {
				INFO("Boot from 1-bit SPI-NAND\n");
				ma35d1_io_spinand_setup(0);
			} else {
				INFO("Boot from 4-bit SPI-NAND\n");
				ma35d1_io_spinand_setup(1);
			}
		}
		break;

	case PWRON_BOOT_NAND:
		INFO("Boot from NAND\n");
		ma35d1_io_nand_setup();
		break;

	case PWRON_BOOT_SD:
		if ((por & 0x4000) == PWRON_SD0) {
			INFO("Boot from SDHC0\n");
			ma35d1_io_sdhc_setup(0);
		} else  {
			INFO("Boot from SDHC1\n");
			ma35d1_io_sdhc_setup(1);
		}
		break;

	default:
		ERROR("Boot interface 0x%x not supported\n", por);
	}
#else
	result = register_io_dev_memmap(&memmap_dev_con);
	assert(result == 0);

	result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
			     &memmap_dev_handle);
	assert(result == 0);

#endif
	result = ma35d1_io_fip_setup();
	assert(result == 0);

}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy.
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	uintptr_t init_params;

	assert(image_id < ARRAY_SIZE(policies));

	*dev_handle = *policies[image_id].dev_handle;
	*image_spec = policies[image_id].image_spec;
	init_params = policies[image_id].init_params;

	return io_dev_init(*dev_handle, init_params);
}
