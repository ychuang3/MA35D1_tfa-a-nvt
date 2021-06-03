/*
 * Copyright (C) 2021 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MA35D1_PMIC_H
#define MA35D1_PMIC_H

/*-----------------------------------------------------------------------------
 * I2C Register's Definition
 *---------------------------------------------------------------------------*/
#define    I2C0_BASE        0x40800000  /*!< I2C0 */

#define    REG_I2C0_CTL      (I2C0_BASE+0x000)   /*!< I2C Control Register */
#define    REG_I2C0_ADDR0    (I2C0_BASE+0x000)   /*!< I2C Slave Address Register0 */
#define    REG_I2C0_DAT      (I2C0_BASE+0x008)   /*!< I2C Data Register */
#define    REG_I2C0_STATUS   (I2C0_BASE+0x00C)   /*!< I2C Status Register */
#define    REG_I2C0_CLKDIV   (I2C0_BASE+0x010)   /*!< I2C Clock Divided Register */
#define    REG_I2C0_TOCTL    (I2C0_BASE+0x014)   /*!< I2C Time-out Control Register */


/*-----------------------------------------*/
/* global interface variables declarations */
/*-----------------------------------------*/
#define I2C_CTL_STA_SI     0x28UL /*!< I2C_CTL setting for I2C control bits. It would set STA and SI bits     */
#define I2C_CTL_STA_SI_AA  0x2CUL /*!< I2C_CTL setting for I2C control bits. It would set STA, SI and AA bits */
#define I2C_CTL_STO_SI     0x18UL /*!< I2C_CTL setting for I2C control bits. It would set STO and SI bits     */
#define I2C_CTL_STO_SI_AA  0x1CUL /*!< I2C_CTL setting for I2C control bits. It would set STO, SI and AA bits */
#define I2C_CTL_SI         0x08UL /*!< I2C_CTL setting for I2C control bits. It would set SI bit              */
#define I2C_CTL_SI_AA      0x0CUL /*!< I2C_CTL setting for I2C control bits. It would set SI and AA bits      */
#define I2C_CTL_STA        0x20UL /*!< I2C_CTL setting for I2C control bits. It would set STA bit             */
#define I2C_CTL_STO        0x10UL /*!< I2C_CTL setting for I2C control bits. It would set STO bit             */
#define I2C_CTL_AA         0x04UL /*!< I2C_CTL setting for I2C control bits. It would set AA bit              */
#define I2C_CTL_ALL        0x3CUL /*!< I2C_CTL setting for I2C control bits. It would set ALL bit             */

#define I2C_CTL_ENABLE     0x40UL /*!< I2C_CTL setting for I2C control bits. It would set I2C Enable bit      */

/*****************************************************************************
 * PMIC
 *****************************************************************************/
#define VOL_CPU	0x01
#define VOL_SD  0x02

enum {
	VOL_1_00 = 10,
	VOL_1_10,
	VOL_1_15,
	VOL_1_20,
	VOL_1_25,
	VOL_1_29,
	VOL_1_30,
	VOL_1_80,
	VOL_3_30
};


#endif /* MA35D1_PMIC_H */
