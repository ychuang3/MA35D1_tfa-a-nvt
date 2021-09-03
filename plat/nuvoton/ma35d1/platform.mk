#
# Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default cluster count for MA35D1
MA35D1_CLUSTER_COUNT	:= 1
# Default number of CPUs per cluster on MA35D1
MA35D1_MAX_CPUS_PER_CLUSTER	:= 2
# Default number of threads per CPU on MA35D1
MA35D1_MAX_PE_PER_CPU	:= 1

RESET_TO_BL2		:=	1

# Preserve the original console input support.
ENABLE_CONSOLE_GETC	:=	1
USE_COHERENT_MEM	:=	0
#ENABLE_PIE		:=	1

# Flags to build TF with Trusted Boot support
TRUSTED_BOARD_BOOT	:= 0

# Pass MA35D1_CLUSTER_COUNT to the build system.
$(eval $(call add_define,MA35D1_CLUSTER_COUNT))
# Pass MA35D1_MAX_CPUS_PER_CLUSTER to the build system.
$(eval $(call add_define,MA35D1_MAX_CPUS_PER_CLUSTER))
# Pass MA35D1_MAX_PE_PER_CPU to the build system.
$(eval $(call add_define,MA35D1_MAX_PE_PER_CPU))
$(eval $(call add_define,MA35D1_INTERCONNECT_DRIVER))

MA35D1_BL32_BASE ?= 0x8f800000
$(eval $(call add_define,MA35D1_BL32_BASE))

MA35D1_DRAM_SIZE ?= 0x0f800000
$(eval $(call add_define,MA35D1_DRAM_SIZE))

MA35D1_DDR_MAX_SIZE ?= 0x10000000
$(eval $(call add_define,MA35D1_DDR_MAX_SIZE))

MA35D1_DRAM_S_BASE ?= 0x8f800000
$(eval $(call add_define,MA35D1_DRAM_S_BASE))

# dump the state on crash console
CRASH_REPORTING		:=	1
$(eval $(call add_define,CRASH_REPORTING))

# enable assert() for release/debug builds
ENABLE_ASSERTIONS	:=	1
PLAT_LOG_LEVEL_ASSERT	:=	50
$(eval $(call add_define,PLAT_LOG_LEVEL_ASSERT))

# Keep debugfs opt-in via the common USE_DEBUGFS build option.

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,MA35D1_LOAD_BL32))
endif

# Enable dynamic memory mapping
PLAT_XLAT_TABLES_DYNAMIC :=	1
$(eval $(call assert_boolean,PLAT_XLAT_TABLES_DYNAMIC))
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))


PLAT_INCLUDES		:=	-Iplat/nuvoton/ma35d1/include		\
				-Iinclude/plat/arm/common/aarch64

# Device tree
DTB_FILE_NAME		?=	ma35d1xx8.dtb ma35d1xx7.dtb ma35d1xx0.dtb
FDT_SOURCES		:=	$(addprefix fdts/, $(patsubst %.dtb,%.dts,$(DTB_FILE_NAME)))
DTC_FLAGS		+=	-Wno-unit_address_vs_reg

# Add `libfdt` and Arm common helpers required for Dynamic Config
include lib/libfdt/libfdt.mk
# Firmware Configuration Framework sources
include lib/fconf/fconf.mk

include lib/debugfs/debugfs.mk

# Include GICv2 driver files
MA35D1_GIC_SOURCES :=		drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\

MA35D1_SECURITY_SOURCES :=	plat/nuvoton/ma35d1/ma35d1_security.c		\

PLAT_BL_COMMON_SOURCES	:=	common/fdt_wrappers.c				\
				drivers/nuvoton/uart/ma35d1_console.S	\
				drivers/nuvoton/ddr/ma35d1_ddr.c	\
				plat/arm/common/arm_common.c			\
				plat/nuvoton/ma35d1/ma35d1_common.c		\
				plat/nuvoton/ma35d1/ma35d1_private.c		\
				drivers/nuvoton/pmic/ma35d1_pmic.c		\
#				plat/nuvoton/ma35d1/drivers/ma35d1_crypto.c	\
#				plat/nuvoton/ma35d1/drivers/tsi_cmd.c	        \

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch64/cortex_a35.S			\
				lib/cpus/${ARCH}/aem_generic.S			\
				drivers/arm/cci/cci.c				\

PLAT_BL_COMMON_SOURCES	+=	drivers/arm/tzc/tzc400.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				plat/nuvoton/ma35d1/aarch64/ma35d1_helpers.S	\

BL2_SOURCES		+=	common/desc_image_load.c			\
				drivers/io/io_block.c				\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_dyn_cfg.c			\
				plat/arm/common/arm_dyn_cfg_helpers.c		\
				plat/nuvoton/ma35d1/ma35d1_bl2_setup.c	\
				plat/nuvoton/ma35d1/ma35d1_io_storage.c	\
				drivers/nuvoton/nand/ma35d1_nand.c		\
				drivers/nuvoton/qspi/ma35d1_qspi.c		\
				drivers/nuvoton/sdhc/ma35d1_sdhc.c		\
				plat/nuvoton/ma35d1/ma35d1_bl2_el3_setup.c	\
				plat/nuvoton/ma35d1/ma35d1_bl2_mem_params_desc.c	\
				plat/nuvoton/ma35d1/ma35d1_image_load.c		\
				drivers/io/io_semihosting.c			\
				lib/optee/optee_utils.c				\
				lib/utils/mem_region.c				\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				${MA35D1_SECURITY_SOURCES}




BL31_SOURCES		+=	drivers/arm/smmu/smmu_v3.c			\
				drivers/cfi/v2m/v2m_flash.c			\
				lib/utils/mem_region.c				\
				plat/nuvoton/ma35d1/ma35d1_bl31_setup.c	\
				plat/nuvoton/ma35d1/ma35d1_pm.c		\
				plat/nuvoton/ma35d1/ma35d1_topology.c		\
				plat/nuvoton/ma35d1/ma35d1_sip_svc.c		\
				${MA35D1_GIC_SOURCES}				\
				${MA35D1_SECURITY_SOURCES}			\

BL31_SOURCES		+=	plat/common/plat_psci_common.c

# Preserve NEED_BL32=yes compatibility; SPD=opteed is the standard interface.
ifeq ($(NEED_BL32),yes)
ifeq ($(SPD),none)
SPD := opteed
include services/spd/opteed/opteed.mk
else ifneq ($(SPD),opteed)
$(error MA35D1 currently supports only SPD=opteed)
endif
endif

override BL1_SOURCES =

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif

ifeq (${TRUSTED_BOARD_BOOT},1)

USE_TBBR_DEFS		:= 1
GENERATE_COT		:= 1

NVT_USE_RSA		:= 0
NVT_USE_ECDSA		:= 1

MBEDTLS_SHA256_SMALLER	:= 0

# Set the default algorithm for the generation of Trusted Board Boot keys
ifeq (${NVT_USE_RSA},1)
	KEY_ALG		:= rsa
endif
ifeq (${NVT_USE_ECDSA},1)
	KEY_ALG		:= ecdsa
	KEY_SIZE	:= 256
endif

MBEDTLS_DIR ?= contrib/mbed-tls

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

BL2_SOURCES		+=	drivers/auth/auth_mod.c			\
				drivers/auth/auth_util.c			\
				drivers/auth/crypto_mod.c		\
				drivers/auth/img_parser_mod.c		\
				drivers/auth/tbbr/tbbr_cot_common.c	\
				drivers/auth/tbbr/tbbr_cot_bl2.c	\
				plat/common/tbbr/plat_tbbr.c		\
				plat/nuvoton/ma35d1/ma35d1_rotpk.S	\
				plat/nuvoton/ma35d1/ma35d1_trusted_boot.c

ROT_KEY			= $(BUILD_PLAT)/rot_key.pem
ROTPK_HASH		= $(BUILD_PLAT)/rotpk_sha256.bin

$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))
#$(eval $(call MAKE_LIB_DIRS))

$(BUILD_PLAT)/bl2/ma35d1_rotpk.o: $(ROTPK_HASH)

certificates: $(ROT_KEY)

$(ROT_KEY):
	@mkdir -p $(@D)
	@echo "  OPENSSL $@"
ifeq (${NVT_USE_RSA},1)
	@if [ ! -f $(ROT_KEY) ]; then \
		openssl genrsa 2048 > $@ 2>/dev/null; \
	fi
endif

ifeq (${NVT_USE_ECDSA},1)
	@if [ ! -f $(ROT_KEY) ]; then \
		openssl ecparam -name secp256r1 -genkey -noout > $@ 2>/dev/null; \
	fi
endif

$(ROTPK_HASH): $(ROT_KEY)
	@echo "  OPENSSL $@"
ifeq (${NVT_USE_RSA},1)
	$(Q)openssl rsa -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null
endif

ifeq (${NVT_USE_ECDSA},1)
	$(Q)openssl ec -in $< -pubout -outform DER 2>/dev/null |\
	openssl dgst -sha256 -binary > $@ 2>/dev/null
endif

endif
