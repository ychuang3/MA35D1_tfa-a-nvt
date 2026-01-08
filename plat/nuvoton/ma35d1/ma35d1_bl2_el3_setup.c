/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/nuvoton/uart_ma35d1.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>
#include <tsi_cmd.h>

#include "ma35d1_private.h"

#if FIP_DE_AES && MA35D1_ENABLE_RMA
#define RMA_UART_BASE		U(0x40700000)
#define RMA_VERSION_REG		(SYS_BA + U(0x1f0))
#define RMA_PLM_STAGE		(SSPCC_BASE + U(0x704))
#define RMA_GPE_PUSEL		U(0x40040130)
#define RMA_HEX_DIGITS		64U
#define RMA_WORDS		8U
#define RMA_ENTRY_US		200000U
#define RMA_ESCAPE_US		5000000U
#define RMA_INPUT_US		30000000U
#define RMA_FIFO_US		10000U

static uint32_t rma_plain[RMA_WORDS] __aligned(CACHE_WRITEBACK_GRANULE);
static uint32_t rma_cipher[RMA_WORDS] __aligned(CACHE_WRITEBACK_GRANULE);

static bool rma_supported(void)
{
	/* Keep the source B-version, Deployed and TSI-only predicates. */
	return ((mmio_read_32(RMA_VERSION_REG) & U(0x0f000000)) ==
		U(0x01000000)) &&
	       ((mmio_read_32(RMA_PLM_STAGE) & U(0xf)) == U(0x3)) &&
	       ((mmio_read_32(SYS_CHIPCFG) & U(0x100)) == 0U);
}

static int rma_reset_rx(void)
{
	uint64_t deadline = timeout_init_us(RMA_FIFO_US);

	mmio_setbits_32(RMA_UART_BASE + UARTFCR, UARTFCR_RXCLR);
	while ((mmio_read_32(RMA_UART_BASE + UARTFCR) & UARTFCR_RXCLR) != 0U) {
		if (timeout_elapsed(deadline))
			return -ETIMEDOUT;
	}
	return 0;
}

static int rma_getc(uint64_t deadline, unsigned int *ch)
{
	uint32_t status;

	while (!timeout_elapsed(deadline)) {
		status = mmio_read_32(RMA_UART_BASE + UARTFSR);
		if ((status & (UARTFSR_BRK | UARTFSR_FERR | UARTFSR_PERR |
			       UARTFSR_OVRF)) != 0U)
			return -EIO;
		if ((status & UARTFSR_RXFIFOEMT) == 0U) {
			*ch = mmio_read_32(RMA_UART_BASE + UARTRX) & U(0xff);
			return 0;
		}
	}
	return -ETIMEDOUT;
}

static int rma_puts(const char *text)
{
	uint64_t deadline = timeout_init_us(RMA_ESCAPE_US);

	while (*text != '\0') {
		while ((mmio_read_32(RMA_UART_BASE + UARTFSR) &
			UARTFSR_TXFIFOFULL) != 0U) {
			if (timeout_elapsed(deadline))
				return -ETIMEDOUT;
		}
		mmio_write_32(RMA_UART_BASE + UARTTX, (unsigned char)*text++);
	}
	return 0;
}

static int rma_read_hex(uint32_t words[RMA_WORDS])
{
	uint64_t deadline = timeout_init_us(RMA_INPUT_US);
	unsigned int ch, count = 0U, digit;
	int ret;

	for (unsigned int i = 0U; i < RMA_WORDS; i++)
		words[i] = 0U;
	for (;;) {
		ret = rma_getc(deadline, &ch);
		if (ret != 0)
			return ret;
		if ((ch == '\n') && (count == 0U))
			continue;
		if (ch == '\r')
			return (count == RMA_HEX_DIGITS) ? 0 : -EINVAL;
		if (count >= RMA_HEX_DIGITS)
			return -E2BIG;
		if ((ch >= '0') && (ch <= '9'))
			digit = ch - '0';
		else if ((ch >= 'a') && (ch <= 'f'))
			digit = ch - 'a' + 10U;
		else if ((ch >= 'A') && (ch <= 'F'))
			digit = ch - 'A' + 10U;
		else
			return -EINVAL;
		words[count / 8U] = (words[count / 8U] << 4) | digit;
		count++;
	}
}

static void rma_clear_buffers(bool output_used)
{
	volatile uint32_t *plain = rma_plain;
	volatile uint32_t *cipher = rma_cipher;
	volatile uint32_t *output = (volatile uint32_t *)TSI_PARAM_BASE;

	for (unsigned int i = 0U; i < RMA_WORDS; i++) {
		plain[i] = 0U;
		cipher[i] = 0U;
		if (output_used)
			output[i] = 0U;
	}
	dsbsy();
}

static int enter_rma_state(void)
{
	volatile uint32_t *output = (volatile uint32_t *)TSI_PARAM_BASE;
	bool output_used = false;
	uint64_t deadline;
	unsigned int ch;
	uint32_t diff = 0U;
	int ret, close_ret, sid;

	ret = rma_puts("please press the 'Esc' first\r\n");
	if (ret != 0)
		goto out;
	ret = rma_reset_rx();
	if (ret != 0)
		goto out;
	deadline = timeout_init_us(RMA_ESCAPE_US);
	do {
		ret = rma_getc(deadline, &ch);
		if (ret != 0)
			goto out;
	} while (ch != U(0x1b));

	ret = rma_puts("please input 64 char raw data\r\n");
	if (ret != 0)
		goto out;
	ret = rma_read_hex(rma_plain);
	if (ret != 0)
		goto out;
	ret = rma_puts("done\r\nplease input 64 char encrypt data\r\n");
	if (ret != 0)
		goto out;
	ret = rma_read_hex(rma_cipher);
	if (ret != 0)
		goto out;

	mmio_setbits_32(CLK_SYSCLK1, U(0x10));
	for (unsigned int i = 0U; i < 3U; i++) {
		ret = TSI_Sync();
		if (ret == 0)
			break;
	}
	if (ret != 0)
		goto out;
	ret = TSI_Open_Session(C_CODE_AES, &sid);
	if (ret != 0)
		goto out;
	output_used = true;
	for (unsigned int i = 0U; i < RMA_WORDS; i++)
		output[i] = 0U;
	dsbsy();
	ret = TSI_AES_Set_IV(sid, TSI_PARAM_BASE);
	if (ret != 0)
		goto close_session;
	/* Preserve the source AES-256 CBC / OTP key slot 8 parameters. */
	ret = TSI_AES_Set_Mode(sid, 0, 0, 1, 1, 0, 0, 2, 2,
			       SEL_KEY_FROM_KS_OTP, 8);
	if (ret != 0)
		goto close_session;
	ret = TSI_AES_Run(sid, 1, 32, (uint32_t)(uintptr_t)rma_cipher,
			  TSI_PARAM_BASE);
close_session:
	close_ret = TSI_Close_Session(C_CODE_AES, sid);
	if (ret == 0)
		ret = close_ret;
	if (ret != 0) {
		/* Do not reuse a crypto session whose completion is uncertain. */
		if (TSI_Reset() != 0)
			panic();
		goto out;
	}
	dsbsy();
	for (unsigned int i = 0U; i < RMA_WORDS; i++)
		diff |= output[i] ^ rma_plain[i];
	if ((diff != 0U) || !rma_supported()) {
		ret = -EPERM;
		goto out;
	}

	rma_clear_buffers(output_used);
	/* A programming timeout can leave fuse state uncertain: never resume boot. */
	ret = TSI_OTP_Program(U(0x108), U(0x7));
	if (ret != 0) {
		ERROR("RMA PLM programming failed: %d\n", ret);
		panic();
	}
	ret = TSI_Reset();
	if (ret != 0)
		ERROR("RMA TSI reset failed: %d\n", ret);
	mmio_clrsetbits_32(SYS_IPRST0, ~U(0x8), U(0x1));
	dsbsy();
	/* PLM shadow state must be reloaded by reset before any further boot. */
	panic();
out:
	rma_clear_buffers(output_used);
	return ret;
}

static void ma35d1_try_enter_rma(void)
{
	uint32_t gpio_clock, gpio_pull;
	uint64_t deadline;
	int ret = 0;

	if (!rma_supported())
		return;
	/* This pre-DDR path and its TSI buffers require data cache disabled. */
	if ((read_sctlr_el3() & SCTLR_C_BIT) != 0U) {
		ERROR("RMA requires the early BL2 cache state\n");
		return;
	}
	generic_delay_timer_init();
	gpio_clock = mmio_read_32(CLK_SYSCLK1);
	mmio_write_32(CLK_SYSCLK1, gpio_clock | U(0x100000));
	gpio_pull = mmio_read_32(RMA_GPE_PUSEL);
	mmio_write_32(RMA_GPE_PUSEL,
		     (gpio_pull & U(0x3fffffff)) | U(0x40000000));
	ret = rma_reset_rx();
	if (ret == 0) {
		deadline = timeout_init_us(RMA_ENTRY_US);
		while (!timeout_elapsed(deadline)) {
			/* Preserve the source RX-count >=16 / RX-full trigger. */
			if ((mmio_read_32(RMA_UART_BASE + UARTFSR) & U(0xb000)) != 0U) {
				ret = enter_rma_state();
				break;
			}
		}
	}
	/* Aborted requests leave the existing pin/clock policy intact. */
	mmio_write_32(RMA_GPE_PUSEL, gpio_pull);
	mmio_write_32(CLK_SYSCLK1, gpio_clock);
	if (ret != 0)
		WARN("RMA request rejected: %d\n", ret);
}
#endif

void bl2_early_platform_setup2(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
	/* Initialize the platform config for future decision making */
	ma35d1_config_setup();

#if FIP_DE_AES && MA35D1_ENABLE_RMA
	ma35d1_try_enter_rma();
#endif

	ma35d1_ddr_init();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	ma35d1_interconnect_init();
	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	ma35d1_interconnect_enable();

}
