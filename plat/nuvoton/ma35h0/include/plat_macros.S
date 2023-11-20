/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLAT_MACROS_S
#define PLAT_MACROS_S

#include <arm_macros.S>
#include <platform_def.h>

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant GIC registers whenever an
	 * unhandled exception is taken in BL31.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro plat_crash_print_regs

	/* Assume Base Cortex mmap */
	mov_imm	x17, BASE_GICC_BASE
	mov_imm	x16, BASE_GICD_BASE
	arm_print_gic_regs
	.endm

#endif /* PLAT_MACROS_S */
