/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MA35D1_NAND_H
#define MA35D1_NAND_H

/*-----------------------------------------------------------------------------
 * NFI Register's Definition
 *---------------------------------------------------------------------------*/
#define    NAND_BASE        0x401A0000  /*!< Flash Memory Card Interface */

/* NFI Registers */
#define     REG_FMI_BUFFER      (NAND_BASE+0x000)   /*!< FMI Embedded Buffer Word */
#define     REG_FMI_DMACTL      (NAND_BASE+0x400)   /*!< FMI DMA Control Register */
#define     REG_FMI_DMASA       (NAND_BASE+0x408)   /*!< FMI DMA Transfer Starting Address Register */
#define     REG_FMI_DMABCNT     (NAND_BASE+0x40C)   /*!< FMI DMA Transfer Byte Count Register */
#define     REG_FMI_DMAINTEN    (NAND_BASE+0x410)   /*!< FMI DMA Interrupt Enable Register */
#define     REG_FMI_DMAINTSTS   (NAND_BASE+0x414)   /*!< FMI DMA Interrupt Status Register */

#define     REG_FMI_CTL         (NAND_BASE+0x800)   /*!< Global Control and Status Register */
#define     REG_FMI_INTEN       (NAND_BASE+0x804)   /*!< Global Interrupt Control Register */
#define     REG_FMI_INTSTS      (NAND_BASE+0x808)   /*!< Global Interrupt Status Register */

/* NAND-type Flash Registers */
#define     REG_NANDCTL         (NAND_BASE+0x8A0)   /*!< NAND Flash Control and Status Register */
#define     REG_NANDTMCTL       (NAND_BASE+0x8A4)   /*!< NAND Flash Timing Control Register */
#define     REG_NANDINTEN       (NAND_BASE+0x8A8)   /*!< NAND Flash Interrupt Control Register */
#define     REG_NANDINTSTS      (NAND_BASE+0x8AC)   /*!< NAND Flash Interrupt Status Register */
#define     REG_NANDCMD         (NAND_BASE+0x8B0)   /*!< NAND Flash Command Port Register */
#define     REG_NANDADDR        (NAND_BASE+0x8B4)   /*!< NAND Flash Address Port Register */
#define     REG_NANDDATA        (NAND_BASE+0x8B8)   /*!< NAND Flash Data Port Register */
#define     REG_NANDRACTL       (NAND_BASE+0x8BC)   /*!< NAND Flash Redundant Area Control Register */
#define     REG_NANDECTL        (NAND_BASE+0x8C0)   /*!< NAND Flash Extend Control Regsiter */
#define     REG_NANDECCES0      (NAND_BASE+0x8D0)   /*!< NAND Flash ECC Error Status 0 */
#define     REG_NANDECCES1      (NAND_BASE+0x8D4)   /*!< NAND Flash ECC Error Status 1 */
#define     REG_NANDECCES2      (NAND_BASE+0x8D8)   /*!< NAND Flash ECC Error Status 2 */
#define     REG_NANDECCES3      (NAND_BASE+0x8DC)   /*!< NAND Flash ECC Error Status 3 */

/* NAND-type Flash BCH Error Address Registers */
#define     REG_NANDECCEA0      (NAND_BASE+0x900)   /*!< NAND Flash ECC Error Byte Address 0 */
#define     REG_NANDECCEA1      (NAND_BASE+0x904)   /*!< NAND Flash ECC Error Byte Address 1 */
#define     REG_NANDECCEA2      (NAND_BASE+0x908)   /*!< NAND Flash ECC Error Byte Address 2 */
#define     REG_NANDECCEA3      (NAND_BASE+0x90C)   /*!< NAND Flash ECC Error Byte Address 3 */
#define     REG_NANDECCEA4      (NAND_BASE+0x910)   /*!< NAND Flash ECC Error Byte Address 4 */
#define     REG_NANDECCEA5      (NAND_BASE+0x914)   /*!< NAND Flash ECC Error Byte Address 5 */
#define     REG_NANDECCEA6      (NAND_BASE+0x918)   /*!< NAND Flash ECC Error Byte Address 6 */
#define     REG_NANDECCEA7      (NAND_BASE+0x91C)   /*!< NAND Flash ECC Error Byte Address 7 */
#define     REG_NANDECCEA8      (NAND_BASE+0x920)   /*!< NAND Flash ECC Error Byte Address 8 */
#define     REG_NANDECCEA9      (NAND_BASE+0x924)   /*!< NAND Flash ECC Error Byte Address 9 */
#define     REG_NANDECCEA10     (NAND_BASE+0x928)   /*!< NAND Flash ECC Error Byte Address 10 */
#define     REG_NANDECCEA11     (NAND_BASE+0x92C)   /*!< NAND Flash ECC Error Byte Address 11 */

/* NAND-type Flash BCH Error Data Registers */
#define     REG_NANDECCED0      (NAND_BASE+0x960)   /*!< NAND Flash ECC Error Data Register 0 */
#define     REG_NANDECCED1      (NAND_BASE+0x964)   /*!< NAND Flash ECC Error Data Register 1 */
#define     REG_NANDECCED2      (NAND_BASE+0x968)   /*!< NAND Flash ECC Error Data Register 2 */
#define     REG_NANDECCED3      (NAND_BASE+0x96C)   /*!< NAND Flash ECC Error Data Register 3 */
#define     REG_NANDECCED4      (NAND_BASE+0x970)   /*!< NAND Flash ECC Error Data Register 4 */
#define     REG_NANDECCED5      (NAND_BASE+0x974)   /*!< NAND Flash ECC Error Data Register 5 */

/* NAND-type Flash Redundant Area Registers */
#define     REG_NANDRA0         (NAND_BASE+0xA00)   /*!< NAND Flash Redundant Area Register */
#define     REG_NANDRA1         (NAND_BASE+0xA04)   /*!< NAND Flash Redundant Area Register */

/*-----------------------------------------------------------------------------
 * Define some constants
 *---------------------------------------------------------------------------*/
/* SMCR */
#define ECC_EN          0x00800000
#define PROT_3BEN       0x00000100
#define REDUN_AUTO_WEN  0x00000010
#define REDUN_REN       0x00000008

#define BCH_TSEL        0x007C0000
#define BCH_24          0x00040000
#define BCH_4           0x00080000
#define BCH_8           0x00100000
#define BCH_12          0x00200000
#define BCH_15          0x00400000

#define PSIZE           0x00030000
#define PSIZE_8K        0x00030000
#define PSIZE_4K        0x00020000
#define PSIZE_2K        0x00010000
#define PSIZE_512       0x00000000

/* SMREACTL */
#define MECC_Msk        0xFFFF0000
#define REA128_EXT      0x000001FF

/* SMADDR */
#define NAND_EOA        0x80000000

/* NAND commands */
#define NAND_CMD_READ_1ST		0x00U
#define NAND_CMD_READ_2ND		0x30U
#define NAND_CMD_STATUS			0x70U
#define NAND_CMD_RESET			0xFFU



#endif /* MA35D1_NAND_H */
