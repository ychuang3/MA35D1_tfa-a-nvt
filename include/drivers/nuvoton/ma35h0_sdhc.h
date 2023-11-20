/*
 * Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MA35H0_SDHC_H
#define MA35H0_SDHC_H

/*-----------------------------------------------------------------------------
 * SDHC Register's Definition
 *---------------------------------------------------------------------------*/
#define    SDH0_BASE        0x40180000  /*!< SD/SDIO Host Controller */
#define    SDH1_BASE        0x40190000  /*!< SD/SDIO Host Controller */

/* only define offset */
#define SDH_DMA_ADDRESS         (0x00)
#define SDH_BLOCK_SIZE          (0x04)
#define SDH_BLOCK_COUNT         (0x06)
#define SDH_ARGUMENT            (0x08)
#define SDH_XFER_MODE           (0x0C)
#define SDH_COMMAND             (0x0E)
	#define SDH_MAKE_CMD(c, f) (((c & 0xff) << 8) | (f & 0xff))
	#define SDH_GET_CMD(c) ((c>>8) & 0x3f)
#define SDH_RESPONSE            (0x10)  /* 0x10 ~ 0x1C */
#define SDH_BUFFER              (0x20)
#define SDH_PRESENT_STATE       (0x24)
#define SDH_HOST_CONTROL        (0x28)
#define SDH_POWER_CONTROL       (0x29)
#define SDH_BGAP_CONTROL        (0x2A)
#define SDH_WUP_CONTROL         (0x2B)
#define SDH_CLOCK_CONTROL       (0x2C)
#define SDH_TOUT_CONTROL        (0x2E)
#define SDH_SW_RESET            (0x2F)
#define SDH_INT_STATUS          (0x30)
#define SDH_INT_ENABLE          (0x34)
#define SDH_SIGNAL_ENABLE       (0x38)
#define SDH_ACMD12_ERR          (0x3C)
#define SDH_HOST_CONTROL2       (0x3E)
#define SDH_CAPABILITIES        (0x40)
#define SDH_CAPABILITIES_1      (0x44)
#define SDH_MAX_CURRENT         (0x48)  /* 0x48 ~ 0x4C */
#define SDH_SET_ACMD12_ERROR    (0x50)
#define SDH_SET_INT_ERROR       (0x52)
#define SDH_ADMA_ERROR          (0x54)
#define SDH_ADMA_ADDRESS        (0x58)
#define SDH_ADMA_ADDRESS_HI     (0x5c)
#define SDH_SLOT_INT_STATUS     (0xFC)
#define SDH_HOST_VERSION        (0xFE)


/*-----------------------------------------------------------------------------
 * Define some constants
 *---------------------------------------------------------------------------*/
/* SDH */
#define  SDH_POWER_ON            0x01
#define  SDH_POWER_180           0x0A
#define  SDH_POWER_300           0x0C
#define  SDH_POWER_330           0x0E

#define  SDH_RESET_ALL           0x01
#define  SDH_RESET_CMD           0x02
#define  SDH_RESET_DATA          0x04

#define  SDH_CMD_RESP_MASK       0x03
#define  SDH_CMD_CRC             0x08
#define  SDH_CMD_INDEX           0x10
#define  SDH_CMD_DATA            0x20
#define  SDH_CMD_ABORTCMD        0xC0

#define  SDH_CMD_RESP_NONE       0x00
#define  SDH_CMD_RESP_LONG       0x01
#define  SDH_CMD_RESP_SHORT      0x02
#define  SDH_CMD_RESP_SHORT_BUSY 0x03


/* MMC command */
#define MMC_CMD_GO_IDLE_STATE           0
#define MMC_CMD_SEND_OP_COND            1
#define MMC_CMD_ALL_SEND_CID            2
#define MMC_CMD_SET_RELATIVE_ADDR       3
#define MMC_CMD_SET_DSR                 4
#define MMC_CMD_SWITCH                  6
#define MMC_CMD_SELECT_CARD             7
#define MMC_CMD_SEND_EXT_CSD            8
#define MMC_CMD_SEND_CSD                9
#define MMC_CMD_SEND_CID                10
#define MMC_CMD_STOP_TRANSMISSION       12
#define MMC_CMD_SEND_STATUS             13
#define MMC_CMD_SET_BLOCKLEN            16
#define MMC_CMD_READ_SINGLE_BLOCK       17
#define MMC_CMD_READ_MULTIPLE_BLOCK     18
#define MMC_CMD_SEND_TUNING_BLOCK       19
#define MMC_CMD_SEND_TUNING_BLOCK_HS200 21
#define MMC_CMD_SET_BLOCK_COUNT         23
#define MMC_CMD_WRITE_SINGLE_BLOCK      24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK    25
#define MMC_CMD_ERASE_GROUP_START       35
#define MMC_CMD_ERASE_GROUP_END         36
#define MMC_CMD_ERASE                   38
#define MMC_CMD_APP_CMD                 55
#define MMC_CMD_SPI_READ_OCR            58
#define MMC_CMD_SPI_CRC_ON_OFF          59
#define MMC_CMD_RES_MAN                 62

#define SD_CMD_SEND_RELATIVE_ADDR       3
#define SD_CMD_SWITCH_FUNC              6
#define SD_CMD_SEND_IF_COND             8
#define SD_CMD_SWITCH_UHS18V            11

#define SD_CMD_APP_SET_BUS_WIDTH        6
#define SD_CMD_APP_SD_STATUS            13
#define SD_CMD_ERASE_WR_BLK_START       32
#define SD_CMD_ERASE_WR_BLK_END         33
#define SD_CMD_APP_SEND_OP_COND         41
#define SD_CMD_APP_SEND_SCR             51

/* MMC response */
#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136     (1 << 1)        /* 136 bit response */
#define MMC_RSP_CRC     (1 << 2)        /* expect valid crc */
#define MMC_RSP_BUSY    (1 << 3)        /* card may send busy */
#define MMC_RSP_OPCODE  (1 << 4)        /* response contains opcode */

#define MMC_RSP_NONE    (0)
#define MMC_RSP_R1  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
#define MMC_RSP_R2  (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3  (MMC_RSP_PRESENT)
#define MMC_RSP_R4  (MMC_RSP_PRESENT)
#define MMC_RSP_R5  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define DEVICE_UNKNOWN      0
#define SD_VERSION_2        1
#define SD_VERSION_1_0      2
#define MMC_VERSION         3

#define MMC_DATA_READ       1
#define MMC_DATA_WRITE      2

#endif /* MA35H0_SDHC_H */
