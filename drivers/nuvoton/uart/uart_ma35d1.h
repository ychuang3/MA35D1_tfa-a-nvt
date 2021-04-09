/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UART_MA35D1_H
#define UART_MA35D1_H

#include <drivers/console.h>

/* UART MA35D1 Registers */
#define UARTTX			0x0
#define UARTRX			0x0
#define UARTIER			0x4
#define UARTFCR			0x8
#define UARTLCR			0xc
#define UARTMCR			0x10
#define UARTMSR			0x14
#define UARTFSR			0x18
#define UARTISR			0x1c
#define UARTTOR			0x20
#define UARTBAUD		0x24
#define UARTIRCR		0x28
#define UARTALTCSR		0x2c
#define UARTFUNSEL		0x30
#define UARTLINCTL		0x34
#define UARTLINSR		0x38

/* FIFO Control Register bits */
#define UARTFCR_RXTRIG_1	(0 << 16)
#define UARTFCR_RXTRIG_4	(1 << 16)
#define UARTFCR_RXTRIG_8	(2 << 16)
#define UARTFCR_RXTRIG_14	(3 << 16)
#define UARTFCR_DMAEN		(1 << 3)	/* Enable DMA mode */
#define UARTFCR_TXCLR		(1 << 2)	/* Clear contents of Tx FIFO */
#define UARTFCR_RXCLR		(1 << 1)	/* Clear contents of Rx FIFO */

/* FIFO Status Register bits */
#define UARTFSR_TXFIFOFULL	(1 << 23)	/* Tx Fifo Full */
#define UARTFSR_THRE		(1 << 22)	/* Tx Holding Register Empty */
#define UARTFSR_RXFIFOFULL	(1 << 15)	/* Rx Fifo Full */
#define UARTFSR_RXFIFOEMT	(1 << 14)	/* Rx Fifo Empty */
#define UARTFSR_BRK		(1 << 6)	/* Break Condition Detected */
#define UARTFSR_FERR		(1 << 5)	/* Framing Error */
#define UARTFSR_PERR		(1 << 4)	/* Parity Error */
#define UARTFSR_OVRF		(1 << 0)	/* Rx Overrun Error */

/* Line Control Register bits */
#define UARTLCR_SETB		(1 << 6)	/* Set BREAK Condition */
#define UARTLCR_SETP		(1 << 5)	/* Set Parity to LCR[4] */
#define UARTLCR_EVEN		(1 << 4)	/* Even Parity Format */
#define UARTLCR_PAR		(1 << 3)	/* Parity */
#define UARTLCR_STOP		(1 << 2)	/* Stop Bit */
#define UARTLCR_WORDSZ_5	0		/* Word Length of 5 */
#define UARTLCR_WORDSZ_6	1		/* Word Length of 6 */
#define UARTLCR_WORDSZ_7	2		/* Word Length of 7 */
#define UARTLCR_WORDSZ_8	3		/* Word Length of 8 */

/* Interrupt Status Register bits */
#define UARTISR_THRE		(1 << 1)	/* Tx Holding Register Empty */
#define UARTISR_RDR_BIT		(0)		/* Rx Data Ready Bit */
#define UARTISR_RDR		(1 << UARTISR_RDR_BIT)	/* Rx Data Ready */

#ifndef __ASSEMBLER__

#include <stdint.h>

/*
 * Initialize a new MA35D1 console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 * When |clock| has a value of 0, the UART will *not* be initialised. This
 * means the UART should already be enabled and the baudrate and clock setup
 * should have been done already, either by platform specific code or by
 * previous firmware stages. The |baud| parameter will be ignored in this
 * case as well.
 */
int console_ma35d1_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			   console_t *console);

#endif /*__ASSEMBLER__*/

#endif /* UART_MA35D1_H */
