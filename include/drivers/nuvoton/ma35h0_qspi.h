/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MA35H0_QSPI_H
#define MA35H0_QSPI_H

/*-----------------------------------------------------------------------------
 * QSPI Register's Definition
 *---------------------------------------------------------------------------*/
#define    QSPI0_BASE       0x40680000  /*!< Serial Peripheral Interface 0 */

/* QSPI0 Registers */
#define     REG_QSPI0_CTL        (QSPI0_BASE+0x00)  /*!< SPI Control Register */
#define     REG_QSPI0_CLKDIV     (QSPI0_BASE+0x04)  /*!< SPI Clock Divider Register */
#define     REG_QSPI0_SSCTL      (QSPI0_BASE+0x08)  /*!< SPI Slave Select Control Register */
#define     REG_QSPI0_PDMACTL    (QSPI0_BASE+0x0c)  /*!< SPI PDMA Control Register */
#define     REG_QSPI0_FIFOCTL    (QSPI0_BASE+0x10)  /*!< SPI FIFO Control Register */
#define     REG_QSPI0_STATUS     (QSPI0_BASE+0x14)  /*!< SPI Status Register */
#define     REG_QSPI0_TX         (QSPI0_BASE+0x20)  /*!< SPI Data Transmit Register */
#define     REG_QSPI0_RX         (QSPI0_BASE+0x30)  /*!< SPI Data Receive Register */


/*-----------------------------------------------------------------------------
 * Define some constants
 *---------------------------------------------------------------------------*/
/* SPI */
/* Write commands */
#define CMD_WRITE_STATUS        0x01
#define CMD_WRITE_DISABLE       0x04
#define CMD_WRITE_ENABLE        0x06
#define CMD_WRITE_EVCR          0x61

/* Read commands */
#define CMD_READ_ARRAY_SLOW         0x03
#define CMD_READ_QUAD_OUTPUT_FAST   0x6b
#define CMD_READ_QUAD_IO_FAST       0xeb
#define CMD_READ_ID                 0x9f
#define CMD_READ_STATUS             0x05
#define CMD_READ_STATUS1            0x35
#define CMD_READ_CONFIG             0x35
#define CMD_READ_EVCR               0x65


#define     GPIO_BASE        0x40040000  /*!< GPIO Control */

#define     GPIOD_MODE          (GPIO_BASE+0xC0) /*!< Port A-H I/O Mode Control                       */
#define     GPIOD_DOUT          (GPIO_BASE+0xC8) /*!< Port A-H Data Output Value                      */
#define     GPIOD_SLEWCTL       (GPIO_BASE+0xE8) /*!< Port A-H High Slew Rate Control Register        */



#endif /* MA35H0_QSPI_H */
