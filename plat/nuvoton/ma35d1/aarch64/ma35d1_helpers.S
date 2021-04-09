/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/fvp/fvp_pwrc.h>
#include <platform_def.h>

	.globl	plat_my_core_pos

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary

	.globl	plat_reset_handler

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	platform_mem_init

	/* ----------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * This Function uses the plat_calc_core_pos()
	 * to get the index of the calling CPU.
	 * ----------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and 	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * TODO: Should we read the PSYS register to make sure
	 * that the request has gone through.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
#ifndef EL3_PAYLOAD_BASE
	/* ---------------------------------------------
	 * Power down this cpu.
	 * TODO: Do we need to worry about powering the
	 * cluster down as well here. That will need
	 * locks which we won't have unless an elf-
	 * loader zeroes out the zi section.
	 * ---------------------------------------------
	 */
	mrs	x0, mpidr_el1
	mov_imm	x1, PWRC_BASE
	str	w0, [x1, #PPOFFR_OFF]

	/* ---------------------------------------------
	 * There is no sane reason to come out of this
	 * wfi so panic if we do. This cpu will be pow-
	 * ered on and reset by the cpu_on pm api
	 * ---------------------------------------------
	 */
	dsb	sy
	wfi
	no_ret	plat_panic_handler
#else
	mov_imm	x0, PLAT_ARM_TRUSTED_MAILBOX_BASE

	/* Wait until the entrypoint gets populated */
poll_mailbox:
	ldr	x1, [x0]
	cbz	x1, 1f
	br	x1
1:
	wfe
	b	poll_mailbox
#endif /* EL3_PAYLOAD_BASE */
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------------------------------
	 * uintptr_t plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot. On MA35D1, this information can be queried from the power
	 * controller. The Power Control SYS Status Register (PSYSR) indicates
	 * the wake-up reason for the CPU.
	 *
	 * For a cold boot, return 0.
	 * For a warm boot, read the mailbox and return the address it contains.
	 *
	 * TODO: PSYSR is a common register and should be
	 * 	accessed using locks. Since it is not possible
	 * 	to use locks immediately after a cold reset
	 * 	we are relying on the fact that after a cold
	 * 	reset all cpus will read the same WK field
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	/* ---------------------------------------------------------------------
	 * When bit PSYSR.WK indicates either "Wake by PPONR" or "Wake by GIC
	 * WakeRequest signal" then it is a warm boot.
	 * ---------------------------------------------------------------------
	 */
#if 0
	mrs	x2, mpidr_el1
	mov_imm	x1, PWRC_BASE
	str	w2, [x1, #PSYSR_OFF]
	ldr	w2, [x1, #PSYSR_OFF]
	ubfx	w2, w2, #PSYSR_WK_SHIFT, #PSYSR_WK_WIDTH
	cmp	w2, #WKUP_PPONR
	beq	warm_reset
	cmp	w2, #WKUP_GICREQ
	beq	warm_reset
#endif
	/* Cold reset */
	mov	x0, #0
	ret

warm_reset:
	/* ---------------------------------------------------------------------
	 * A mailbox is maintained in the trusted SRAM. It is flushed out of the
	 * caches after every update using normal memory so it is safe to read
	 * it here with SO attributes.
	 * ---------------------------------------------------------------------
	 */
	mov_imm	x0, PLAT_ARM_TRUSTED_MAILBOX_BASE
	ldr	x0, [x0]
	cbz	x0, _panic_handler
	ret

	/* ---------------------------------------------------------------------
	 * The power controller indicates this is a warm reset but the mailbox
	 * is empty. This should never happen!
	 * ---------------------------------------------------------------------
	 */
_panic_handler:
	no_ret	plat_panic_handler
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #MA35D1_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary


func plat_reset_handler
	ret
endfunc plat_reset_handler

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, PLAT_ARM_CRASH_UART_BASE
	mov_imm	x1, PLAT_ARM_CRASH_UART_CLK_IN_HZ
	mov_imm	x2, ARM_CONSOLE_BAUDRATE
	b	console_ma35d1_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, PLAT_ARM_CRASH_UART_BASE
	b	console_ma35d1_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * int plat_crash_console_flush()
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, PLAT_ARM_CRASH_UART_BASE
	b	console_ma35d1_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------------------------------
	 * We don't need to carry out any memory initialization on ARM
	 * platforms. The Secure RAM is accessible straight away.
	 * ---------------------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init
