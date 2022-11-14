/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**************************************************************************//**
 * @file     crypto.h
 * @version  V1.10
 * @brief    Cryptographic Accelerator driver header file
 *
 * @copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup CRYPTO_Driver CRYPTO Driver
  @{
*/


/** @addtogroup CRYPTO_EXPORTED_CONSTANTS CRYPTO Exported Constants
  @{
*/
#define INTEN			(CRYPTO_BASE+0x000)
#define INTEN_AESIEN			(0x1 << 0)
#define INTEN_AESEIEN			(0x1 << 1)
#define INTEN_PRNGIEN			(0x1 << 16)
#define INTEN_ECCIEN			(0x1 << 22)
#define INTEN_ECCEIEN			(0x1 << 23)
#define INTEN_HMACIEN			(0x1 << 24)
#define INTEN_HMACEIEN			(0x1 << 25)
#define INTSTS			(CRYPTO_BASE+0x004)
#define INTSTS_AESIF			(0x1 << 0)
#define INTSTS_AESEIF			(0x1 << 1)
#define INTSTS_PRNGIF			(0x1 << 16)
#define INTSTS_ECCIF			(0x1 << 22)
#define INTSTS_ECCEIF			(0x1 << 23)
#define INTSTS_HMACIF			(0x1 << 24)
#define INTSTS_HMACEIF			(0x1 << 25)

#define PRNG_CTL		(CRYPTO_BASE+0x008)
#define PRNG_CTL_START			(0x1 << 0)
#define PRNG_CTL_SEEDRLD		(0x1 << 1)
#define PRNG_CTL_KEYSZ_OFFSET		(2)
#define PRNG_CTL_KEYSZ_MASK		(0xf << 2)
#define PRNG_CTL_BUSY			(0x1 << 8)
#define PRNG_CTL_SEEDSRC		(0x1 << 16)
#define PRNG_SEED		(CRYPTO_BASE+0x00C)
#define PRNG_KEY(x)		(CRYPTO_BASE+0x010 + ((x) * 0x04))

#define AES_FDBCK(x)		(CRYPTO_BASE+0x050 + ((x) * 0x04))
#define AES_GCM_IVCNT(x)	(CRYPTO_BASE+0x080 + ((x) * 0x04))
#define AES_GCM_ACNT(x)		(CRYPTO_BASE+0x088 + ((x) * 0x04))
#define AES_GCM_PCNT(x)		(CRYPTO_BASE+0x090 + ((x) * 0x04))
#define AES_FBADDR		(CRYPTO_BASE+0x0A0)
#define AES_CTL			(CRYPTO_BASE+0x100)
#define AES_CTL_START			(0x1 << 0)
#define AES_CTL_STOP			(0x1 << 1)
#define AES_CTL_KEYSZ_OFFSET		2
#define AES_CTL_KEYSZ_MASK		(0x3 << 2)
#define AES_CTL_DMALAST			(0x1 << 5)
#define AES_CTL_DMACSCAD		(0x1 << 6)
#define AES_CTL_DMAEN			(0x1 << 7)
#define AES_CTL_OPMODE_OFFSET		8
#define AES_CTL_OPMODE_MASK		(0xff << 8)
#define AES_CTL_ENCRPT			(0x1 << 16)
#define AES_CTL_SM4EN			(0x1 << 17)
#define AES_CTL_FBIN			(0x1 << 20)
#define AES_CTL_FBOUT			(0x1 << 21)
#define AES_CTL_OUTSWAP			(0x1 << 22)
#define AES_CTL_INSWAP			(0x1 << 23)
#define AES_CTL_KOUTSWAP		(0x1 << 24)
#define AES_CTL_KINSWAP			(0x1 << 25)
#define AES_STS			(CRYPTO_BASE+0x104)
#define AES_STS_BUSY			(0x1 << 0)
#define AES_STS_INBUFEMPTY		(0x1 << 8)
#define AES_STS_INBUFFULL		(0x1 << 9)
#define AES_STS_INBUFERR		(0x1 << 10)
#define AES_STS_CNTERR			(0x1 << 12)
#define AES_STS_OUTBUFEMPTY		(0x1 << 16)
#define AES_STS_OUTBUFFULL		(0x1 << 17)
#define AES_STS_OUTBUFERR		(0x1 << 18)
#define AES_STS_BUSERR			(0x1 << 20)
#define AES_STS_KSERR			(0x1 << 21)
#define AES_DATIN		(CRYPTO_BASE+0x108)
#define AES_DATOUT		(CRYPTO_BASE+0x10C)
#define AES_KEY(x)		(CRYPTO_BASE+0x110 + ((x) * 0x04))
#define AES_IV(x)		(CRYPTO_BASE+0x130 + ((x) * 0x04))
#define AES_SADDR		(CRYPTO_BASE+0x140)
#define AES_DADDR		(CRYPTO_BASE+0x144)
#define AES_CNT			(CRYPTO_BASE+0x148)

#define HMAC_CTL		(CRYPTO_BASE+0x300)
#define HMAC_CTL_START			(0x1 << 0)
#define HMAC_CTL_STOP			(0x1 << 1)
#define HMAC_CTL_DMAFIRST		(0x1 << 4)
#define HMAC_CTL_DMALAST_OFFSET		5
#define HMAC_CTL_DMALAST		(0x1 << 5)
#define HMAC_CTL_DMACSCAD		(0x1 << 6)
#define HMAC_CTL_DMAEN			(0x1 << 7)
#define HMAC_CTL_OPMODE_OFFSET		8
#define HMAC_CTL_OPMODE_MASK		(0x7 << 8)
#define HMAC_CTL_HMACEN			(0x1 << 11)
#define HMAC_CTL_SHA3EN			(0x1 << 12)
#define HMAC_CTL_SM3EN			(0x1 << 13)
#define HMAC_CTL_MD5EN			(0x1 << 14)
#define HMAC_CTL_FBIN			(0x1 << 20)
#define HMAC_CTL_FBOUT			(0x1 << 21)
#define HMAC_CTL_OUTSWAP_OFFFSET		22
#define HMAC_CTL_OUTSWAP		(0x1 << 22)
#define HMAC_CTL_INSWAP			(0x1 << 23)
#define HMAC_CTL_NEXTDGST		(0x1 << 24)
#define HMAC_CTL_FINISHDGST		(0x1 << 25)
#define HMAC_STS		(CRYPTO_BASE+0x304)
#define HMAC_STS_BUSY			(0x1 << 0)
#define HMAC_STS_DMABUSY		(0x1 << 1)
#define HMAC_STS_SHAKEBUSY		(0x1 << 2)
#define HMAC_STS_DMAERR			(0x1 << 8)
#define HMAC_STS_KSERR			(0x1 << 9)
#define HMAC_STS_DATINREQ		(0x1 << 16)
#define HMAC_DGST(x)		(CRYPTO_BASE+0x308 + ((x) * 0x04))
#define HMAC_KEYCNT		(CRYPTO_BASE+0x348)
#define HMAC_SADDR		(CRYPTO_BASE+0x34C)
#define HMAC_DMACNT		(CRYPTO_BASE+0x350)
#define HMAC_DATIN		(CRYPTO_BASE+0x354)
#define HMAC_FDBCK(x)		(CRYPTO_BASE+0x358 + ((x) * 0x04))
#define HMAC_FDBCK_WCNT			88
#define HMAC_FBADDR		(CRYPTO_BASE+0x4FC)
#define HMAC_SHAKEDGST(x)	(CRYPTO_BASE+0x500 + ((x) * 0x04))
#define HMAC_SHAKEDGST_WCNT		42

#define ECC_CTL			(CRYPTO_BASE+0x800)
#define ECC_CTL_START			(0x1 << 0)
#define ECC_CTL_STOP			(0x1 << 1)
#define ECC_CTL_ECDSAS			(0x1 << 4)
#define ECC_CTL_ECDSAR			(0x1 << 5)
#define ECC_CTL_DMAEN			(0x1 << 7)
#define ECC_CTL_FSEL			(0x1 << 8)
#define ECC_CTL_ECCOP_OFFSET		9
#define ECC_CTL_ECCOP_MASK		(0x3 << 9)
#define ECC_CTL_MODOP_OFFSET		11
#define ECC_CTL_MODOP_MASK		(0x3 << 9)
#define ECC_CTL_CSEL			(0x1 << 13)
#define ECC_CTL_SCAP			(0x1 << 14)
#define ECC_CTL_LDAP1			(0x1 << 16)
#define ECC_CTL_LDAP2			(0x1 << 17)
#define ECC_CTL_LDA			(0x1 << 18)
#define ECC_CTL_LDB			(0x1 << 19)
#define ECC_CTL_LDN			(0x1 << 20)
#define ECC_CTL_LDK			(0x1 << 21)
#define ECC_CTL_CURVEM_OFFSET		22
#define ECC_CTL_CURVEM_MASK		(0x3ff << 22)
#define ECC_STS			(CRYPTO_BASE+0x804)
#define ECC_STS_BUSY			(0x1 << 0)
#define ECC_STS_DMABUSY			(0x1 << 1)
#define ECC_STS_BUSERR			(0x1 << 16)
#define ECC_STS_KSERR			(0x1 << 17)
#define ECC_X1(x)		(CRYPTO_BASE+0x808 + ((x) * 0x04))
#define ECC_Y1(x)		(CRYPTO_BASE+0x850 + ((x) * 0x04))
#define ECC_X2(x)		(CRYPTO_BASE+0x898 + ((x) * 0x04))
#define ECC_Y2(x)		(CRYPTO_BASE+0x8E0 + ((x) * 0x04))
#define ECC_A(x)		(CRYPTO_BASE+0x928 + ((x) * 0x04))
#define ECC_B(x)		(CRYPTO_BASE+0x970 + ((x) * 0x04))
#define ECC_N(x)		(CRYPTO_BASE+0x9B8 + ((x) * 0x04))
#define ECC_K(x)		(CRYPTO_BASE+0xA00 + ((x) * 0x04))
#define ECC_KEY_WCNT			18
#define ECC_SADDR		(CRYPTO_BASE+0xA48)
#define ECC_DADDR		(CRYPTO_BASE+0xA4C)
#define ECC_STARTREG		(CRYPTO_BASE+0xA50)
#define ECC_WORDCNT		(CRYPTO_BASE+0xA54)

#define AES_KSCTL		(CRYPTO_BASE+0xF10)
#define AES_KSCTL_NUM_OFFSET		0
#define AES_KSCTL_NUM_MASK		(0x1f << 0)
#define AES_KSCTL_RSRC			(0x1 << 5)
#define AES_KSCTL_RSSRC_OFFSET		6
#define AES_KSCTL_RSSRC_MASK		(0x3 << 6)

#define ECC_KSCTL		(CRYPTO_BASE+0xF40)
#define ECC_KSCTL_NUMK_OFFSET		0
#define ECC_KSCTL_NUMK_MASK		(0x1f << 0)
#define ECC_KSCTL_RSRCK			(0x1 << 5)
#define ECC_KSCTL_RSSRCK_OFFSET		6
#define ECC_KSCTL_RSSRCK_MASK		(0x3 << 6)
#define ECC_KSCTL_TRUST			(0x1 << 16)
#define ECC_KSCTL_PRIV			(0x1 << 18)
#define ECC_KSCTL_XY			(0x1 << 20)
#define ECC_KSCTL_WDST			(0x1 << 21)
#define ECC_KSCTL_WSDST_OFFSET		22
#define ECC_KSCTL_WSDST_MASK		(0x3 << 22)
#define ECC_KSCTL_OWNER_OFFSET		24
#define ECC_KSCTL_OWNER_MASK		(0x7 << 24)
#define ECC_KSSTS		0xF44
#define ECC_KSSTS_NUM_OFFSET		0
#define ECC_KSSTS_NUM_MASK		(0x1f << 0)
#define ECC_KSXY			0xF48
#define ECC_KSXY_NUMX_OFFSET		0
#define ECC_KSXY_NUMX_MASK		(0x1f << 0)
#define ECC_KSXY_RSRCXY			(0x1 << 5)
#define ECC_KSXY_RSSRCX_OFFSET		6
#define ECC_KSXY_RSSRCX_MASK		(0x3 << 6)
#define ECC_KSXY_NUMY_OFFSET		8
#define ECC_KSXY_NUMY_MASK		(0x1f << 8)
#define ECC_KSXY_RSSRCY_OFFSET		14
#define ECC_KSXY_RSSRCY_MASK		(0x3 << 14)


#define AES_KEY_SIZE_128        0UL     /*!< AES select 128-bit key length           \hideinitializer */
#define AES_KEY_SIZE_192        1UL     /*!< AES select 192-bit key length           \hideinitializer */
#define AES_KEY_SIZE_256        2UL     /*!< AES select 256-bit key length           \hideinitializer */

#define AES_MODE_ECB            0UL     /*!< AES select ECB mode                     \hideinitializer */
#define AES_MODE_CBC            1UL     /*!< AES select CBC mode                     \hideinitializer */
#define AES_MODE_CFB            2UL     /*!< AES select CFB mode                     \hideinitializer */
#define AES_MODE_OFB            3UL     /*!< AES select OFB mode                     \hideinitializer */
#define AES_MODE_CTR            4UL     /*!< AES select CTR mode                     \hideinitializer */
#define AES_MODE_CBC_CS1        0x10UL  /*!< AES select CBC CS1 mode                 \hideinitializer */
#define AES_MODE_CBC_CS2        0x11UL  /*!< AES select CBC CS2 mode                 \hideinitializer */
#define AES_MODE_CBC_CS3        0x12UL  /*!< AES select CBC CS3 mode                 \hideinitializer */
#define AES_MODE_GCM            0x20UL  /*!< AES select GCM (Galois/Counter Mode)    \hideinitializer */
#define AES_MODE_GHASH          0x21UL  /*!< AES select GHASH (Galois Hash Function) \hideinitializer */
#define AES_MODE_CCM            0x22UL  /*!< AES select CCM (Counter with CBC-MAC Mode)   \hideinitializer */

#define AES_NO_SWAP             0UL     /*!< AES do not swap input and output data   \hideinitializer */
#define AES_OUT_SWAP            1UL     /*!< AES swap output data                    \hideinitializer */
#define AES_IN_SWAP             2UL     /*!< AES swap input data                     \hideinitializer */
#define AES_IN_OUT_SWAP         3UL     /*!< AES swap both input and output data     \hideinitializer */

#define SHA_MODE_SHA1           0UL     /*!< SHA select SHA-1 160-bit                \hideinitializer */
#define SHA_MODE_SHA224         5UL     /*!< SHA select SHA-224 224-bit              \hideinitializer */
#define SHA_MODE_SHA256         4UL     /*!< SHA select SHA-256 256-bit              \hideinitializer */
#define SHA_MODE_SHA384         7UL     /*!< SHA select SHA-384 384-bit              \hideinitializer */
#define SHA_MODE_SHA512         6UL     /*!< SHA select SHA-512 512-bit              \hideinitializer */

#define SHA_MODE_SHAKE128       0UL     /*!< SHA select SHA-3 SHAKE128               \hideinitializer */
#define SHA_MODE_SHAKE256       1UL     /*!< SHA select SHA-3 SHAKE256               \hideinitializer */


#define SHA_NO_SWAP             0UL     /*!< SHA do not swap input and output data   \hideinitializer */
#define SHA_OUT_SWAP            1UL     /*!< SHA swap output data                    \hideinitializer */
#define SHA_IN_SWAP             2UL     /*!< SHA swap input data                     \hideinitializer */
#define SHA_IN_OUT_SWAP         3UL     /*!< SHA swap both input and output data     \hideinitializer */

#define CRYPTO_DMA_FIRST        0x4UL   /*!< Do first encrypt/decrypt in DMA cascade \hideinitializer */
#define CRYPTO_DMA_ONE_SHOT     0x5UL   /*!< Do one shot encrypt/decrypt with DMA      \hideinitializer */
#define CRYPTO_DMA_CONTINUE     0x6UL   /*!< Do continuous encrypt/decrypt in DMA cascade \hideinitializer */
#define CRYPTO_DMA_LAST         0x7UL   /*!< Do last encrypt/decrypt in DMA cascade          \hideinitializer */


#define RSA_MAX_KLEN            (4096)
#define RSA_KBUF_HLEN           (RSA_MAX_KLEN/4 + 8)
#define RSA_KBUF_BLEN           (RSA_MAX_KLEN + 32)

/*@}*/ /* end of group CRYPTO_EXPORTED_CONSTANTS */


/** @addtogroup M480_CRYPTO_EXPORTED_MACROS CRYPTO Exported Macros
  @{
*/

/*----------------------------------------------------------------------------------------------*/
/*  Macros                                                                                      */
/*----------------------------------------------------------------------------------------------*/

/**
  * @brief This macro enables PRNG interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define PRNG_ENABLE_INT(crpt)       ((crpt)->INTEN |= CRPT_INTEN_PRNGIEN_Msk)

/**
  * @brief This macro disables PRNG interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define PRNG_DISABLE_INT(crpt)      ((crpt)->INTEN &= ~CRPT_INTEN_PRNGIEN_Msk)

/**
  * @brief This macro gets PRNG interrupt flag.
  * @param crpt     Specified cripto module
  * @return PRNG interrupt flag.
  * \hideinitializer
  */
#define PRNG_GET_INT_FLAG(crpt)     ((crpt)->INTSTS & CRPT_INTSTS_PRNGIF_Msk)

/**
  * @brief This macro clears PRNG interrupt flag.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define PRNG_CLR_INT_FLAG(crpt)     ((crpt)->INTSTS = CRPT_INTSTS_PRNGIF_Msk)

/**
  * @brief This macro enables AES interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define AES_ENABLE_INT(crpt)        ((crpt)->INTEN |= (CRPT_INTEN_AESIEN_Msk|CRPT_INTEN_AESEIEN_Msk))

/**
  * @brief This macro disables AES interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define AES_DISABLE_INT(crpt)       ((crpt)->INTEN &= ~(CRPT_INTEN_AESIEN_Msk|CRPT_INTEN_AESEIEN_Msk))

/**
  * @brief This macro gets AES interrupt flag.
  * @param crpt     Specified cripto module
  * @return AES interrupt flag.
  * \hideinitializer
  */
#define AES_GET_INT_FLAG(crpt)      ((crpt)->INTSTS & (CRPT_INTSTS_AESIF_Msk|CRPT_INTSTS_AESEIF_Msk))

/**
  * @brief This macro clears AES interrupt flag.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define AES_CLR_INT_FLAG(crpt)      ((crpt)->INTSTS = (CRPT_INTSTS_AESIF_Msk|CRPT_INTSTS_AESEIF_Msk))

/**
  * @brief This macro enables AES key protection.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define AES_ENABLE_KEY_PROTECT(crpt)  ((crpt)->AES_CTL |= CRPT_AES_CTL_KEYPRT_Msk)

/**
  * @brief This macro disables AES key protection.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define AES_DISABLE_KEY_PROTECT(crpt) ((crpt)->AES_CTL = ((crpt)->AES_CTL & ~CRPT_AES_CTL_KEYPRT_Msk) | (0x16UL<<CRPT_AES_CTL_KEYUNPRT_Pos)); \
									  ((crpt)->AES_CTL &= ~CRPT_AES_CTL_KEYPRT_Msk)


/**
  * @brief This macro enables SHA interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define SHA_ENABLE_INT(crpt)        ((crpt)->INTEN |= (CRPT_INTEN_HMACIEN_Msk|CRPT_INTEN_HMACEIEN_Msk))

/**
  * @brief This macro disables SHA interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define SHA_DISABLE_INT(crpt)       ((crpt)->INTEN &= ~(CRPT_INTEN_HMACIEN_Msk|CRPT_INTEN_HMACEIEN_Msk))

/**
  * @brief This macro gets SHA interrupt flag.
  * @param crpt     Specified cripto module
  * @return SHA interrupt flag.
  * \hideinitializer
  */
#define SHA_GET_INT_FLAG(crpt)      ((crpt)->INTSTS & (CRPT_INTSTS_HMACIF_Msk|CRPT_INTSTS_HMACEIF_Msk))

/**
  * @brief This macro clears SHA interrupt flag.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define SHA_CLR_INT_FLAG(crpt)      ((crpt)->INTSTS = (CRPT_INTSTS_HMACIF_Msk|CRPT_INTSTS_HMACEIF_Msk))

/**
  * @brief This macro enables ECC interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define ECC_ENABLE_INT(crpt)        ((crpt)->INTEN |= (CRPT_INTEN_ECCIEN_Msk|CRPT_INTEN_ECCEIEN_Msk))

/**
  * @brief This macro disables ECC interrupt.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define ECC_DISABLE_INT(crpt)       ((crpt)->INTEN &= ~(CRPT_INTEN_ECCIEN_Msk|CRPT_INTEN_ECCEIEN_Msk))

/**
  * @brief This macro gets ECC interrupt flag.
  * @param crpt     Specified cripto module
  * @return ECC interrupt flag.
  * \hideinitializer
  */
#define ECC_GET_INT_FLAG(crpt)      ((crpt)->INTSTS & (CRPT_INTSTS_ECCIF_Msk|CRPT_INTSTS_ECCEIF_Msk))

/**
  * @brief This macro clears ECC interrupt flag.
  * @param crpt     Specified cripto module
  * @return None
  * \hideinitializer
  */
#define ECC_CLR_INT_FLAG(crpt)      ((crpt)->INTSTS = (CRPT_INTSTS_ECCIF_Msk|CRPT_INTSTS_ECCEIF_Msk))


/*@}*/ /* end of group M480_CRYPTO_EXPORTED_MACROS */


/** @addtogroup CRYPTO_EXPORTED_FUNCTIONS CRYPTO Exported Functions
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Functions                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void Reg2Hex(int count, unsigned int volatile reg[], char output[]);
void SHA_Open(unsigned int u32OpMode, unsigned int u32SwapType, unsigned int hmac_key_len);
void SHA_Start(unsigned int u32DMAMode);
void SHA_SetDMATransfer(unsigned int u32SrcAddr, unsigned int u32TransCnt);
void SHA_Read(unsigned int u32Digest[]);
int ECC_VerifySignature_KS(char *message, int x_ksnum, int y_ksnum, char *R, char *S);
#endif /* MA35D1_CRYPTO_H */
