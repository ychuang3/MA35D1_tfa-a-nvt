/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <plat/common/platform.h>
#include <plat/common/common_def.h>
#include <platform_def.h>

#include <ma35d1_crypto.h>
#include <tsi_cmd.h>
//#include "../ma35d1_def.h"
//#include "../include/ma35d1_crypto.h"
//#include "../include/tsi_cmd.h"

/*-----------------------------------------------------------------------------------------------*/
/*                                                                                               */
/*    ECC                                                                                        */
/*                                                                                               */
/*-----------------------------------------------------------------------------------------------*/

#define ECCOP_POINT_MUL     (0x0UL << ECC_CTL_ECCOP_OFFSET)
#define ECCOP_MODULE        (0x1UL << ECC_CTL_ECCOP_OFFSET)
#define ECCOP_POINT_ADD     (0x2UL << ECC_CTL_ECCOP_OFFSET)
#define ECCOP_POINT_DOUBLE  (0x0UL << ECC_CTL_ECCOP_OFFSET)

#define MODOP_DIV           (0x0UL << ECC_CTL_MODOP_OFFSET)
#define MODOP_MUL           (0x1UL << ECC_CTL_MODOP_OFFSET)
#define MODOP_ADD           (0x2UL << ECC_CTL_MODOP_OFFSET)
#define MODOP_SUB           (0x3UL << ECC_CTL_MODOP_OFFSET)
/*-----------------------------------------------------*/
/*  Define elliptic curve (EC):                        */
/*-----------------------------------------------------*/
enum
{
	CURVE_GF_P,
	CURVE_GF_2M,
};

typedef struct e_curve_t
{
	int   Echar;
	char  Ea[144];
	char  Eb[144];
	char  Px[144];
	char  Py[144];
	int   Epl;
	char  Pp[176];
	int   Eol;
	char  Eorder[176];
	int   key_len;
	int   irreducible_k1;
	int   irreducible_k2;
	int   irreducible_k3;
	int   GF;
}  ECC_CURVE;


static ECC_CURVE  *pCurve;
const ECC_CURVE  Curve_P256 =
{
	/* NIST: Curve P-256 : y^2=x^3-ax+b (mod p) */
	64,     /* Echar */
	"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",  /* "0000000000000000000000000000000000000000000000000000000000000003" */
	"5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
	"6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
	"4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",
	78,     /* Epl */
	"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",  /* "115792089210356248762697446949407573530086143415290314195533631308867097853951" */
	78,     /* Eol */
	"FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551",  /* "115792089210356248762697446949407573529996955224135760342422259061068512044369" */
	256,    /* key_len */
	10,
	5,
	2,
	CURVE_GF_P
};

const char  hex_char_tbl[] = "0123456789abcdef";

static char  ch2hex(char ch)
{
	if (ch <= '9')
	{
		ch = ch - '0';
	}
	else if ((ch <= 'z') && (ch >= 'a'))
	{
		ch = ch - 'a' + 10U;
	}
	else
	{
		ch = ch - 'A' + 10U;
	}
	return ch;
}

void Hex2Reg(char input[], unsigned int volatile *reg)
{
	char      hex;
	int       si, ri;
	unsigned int  i, val32;

	si = (int)strlen(input) - 1;
	ri = 0;

	while (si >= 0)
	{
		val32 = 0UL;
		for (i = 0UL; (i < 8UL) && (si >= 0); i++)
		{
			hex = ch2hex(input[si]);
			val32 |= (unsigned int)hex << (i * 4UL);
			si--;
		}
		reg[ri++] = val32;
	}
}

void hex_to_string(unsigned char *hex, int count, char *str)
{
	for (; count > 0; hex++, count--) {
		*str++ = hex_char_tbl[(*hex >> 4) & 0xf];
		*str++ = hex_char_tbl[*hex & 0xf];
	}
	*str = 0;
}

static int ecc_init_curve(void)
{
	int  i, ret = 0;

	pCurve = (ECC_CURVE  *)&Curve_P256;
	if (pCurve == 0)
	{
		ret = -1;
	}

	if (ret == 0)
	{
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_A(i), 0UL);
			outp32(ECC_B(i), 0UL);
			outp32(ECC_X1(i), 0UL);
			outp32(ECC_Y1(i), 0UL);
			outp32(ECC_X2(i), 0UL);
			outp32(ECC_Y2(i), 0UL);
			outp32(ECC_N(i), 0UL);
		}

		Hex2Reg(pCurve->Ea, (void *)ECC_A(0));
		Hex2Reg(pCurve->Eb, (void *)ECC_B(0));
		Hex2Reg(pCurve->Px, (void *)ECC_X1(0));
		Hex2Reg(pCurve->Py, (void *)ECC_Y1(0));

		if (pCurve->GF == (int)CURVE_GF_2M)
		{
			outp32(ECC_N(0), 0x1UL);
			outp32(ECC_N((pCurve->key_len) / 32), \
				inp32(ECC_N((pCurve->key_len) / 32)| (1UL << ((pCurve->key_len) % 32))));
			outp32(ECC_N((pCurve->irreducible_k1) / 32), \
				inp32(ECC_N((pCurve->irreducible_k1) / 32))|(1UL << ((pCurve->irreducible_k1) % 32)));
			outp32(ECC_N((pCurve->irreducible_k2) / 32), \
				inp32(ECC_N((pCurve->irreducible_k2) / 32))|(1UL << ((pCurve->irreducible_k2) % 32)));
			outp32(ECC_N((pCurve->irreducible_k3) / 32),
				inp32(ECC_N((pCurve->irreducible_k3) / 32))|(1UL << ((pCurve->irreducible_k3) % 32)));
		}
		else
		{
			Hex2Reg(pCurve->Pp, (void *)ECC_N(0));
		}
	}
	return ret;
}

static int ecc_strcmp(char *s1, char *s2)
{
	char  c1, c2;

	while (*s1 == '0') s1++;
	while (*s2 == '0') s2++;

	for ( ; *s1 || *s2; s1++, s2++)
	{
		if ((*s1 >= 'A') && (*s1 <= 'Z'))
			c1 = *s1 + 32;
		else
			c1 = *s1;

		if ((*s2 >= 'A') && (*s2 <= 'Z'))
			c2 = *s2 + 32;
		else
			c2 = *s2;

		if (c1 != c2)
			return 1;
	}
	return 0;
}

int wait_ECC_complete()
{
	while (1)
	{
		if (inp32(INTSTS) & INTSTS_ECCEIF)
		{
			outp32(INTSTS, INTSTS_ECCEIF);
			return 1;
		}

		if (inp32(INTSTS) & INTSTS_ECCIF)
		{
			outp32(INTSTS, INTSTS_ECCIF);
			return 1;
		}
	}
}

static void run_ecc_codec(unsigned int mode)
{

	if ((mode & ECC_CTL_ECCOP_MASK) == ECCOP_MODULE)
	{
		outp32(ECC_CTL ,ECC_CTL_FSEL);
	}
	else
	{
		if (pCurve->GF == (int)CURVE_GF_2M)
		{
			/* point */
			outp32(ECC_CTL, 0UL);
		}
		else
		{
			/* CURVE_GF_P */
			outp32(ECC_CTL, ECC_CTL_FSEL);
		}
	}

	outp32(ECC_CTL, inp32(ECC_CTL)|((unsigned int)pCurve->key_len << ECC_CTL_CURVEM_OFFSET) | mode | ECC_CTL_START);
	wait_ECC_complete();
}

/**
  * @brief  Extract specified nibble from an unsigned word in character format.
  *         For example:
  *                Suppose val32 is 0x786543210, get_Nth_nibble_char(val32, 3) will return a '3'.
  * @param[in]  val32   The input unsigned word
  * @param[in]  idx     The Nth nibble to be extracted.
  * @return  The nibble in character format.
  */
static char get_Nth_nibble_char(unsigned int val32, unsigned int idx)
{
	return hex_char_tbl[ (val32 >> (idx * 4U)) & 0xfU ];
}


void Reg2Hex(int count, unsigned int *reg, char output[])
{
	int idx, ri;
	unsigned int i;

	output[count] = 0U;
	idx = count - 1;

	for (ri = 0; idx >= 0; ri++)
	{
		for (i = 0UL; (i < 8UL) && (idx >= 0); i++)
		{
			output[idx] = get_Nth_nibble_char(reg[ri], i);
			idx--;
		}
	}
}

/**
  * @brief  Set AES initial vectors
  * @param[in]  CRPT        Reference to Crypto module.
  * @param[in]  u32Channel  AES channel. Must be 0~3.
  * @param[in]  au32IV      A four entry word array contains AES initial vectors.
  * @return None
  */
void AES_SetInitVect(void)
{
	outp32((void *)AES_IV(0), 0);
	outp32((void *)AES_IV(1), 0);
	outp32((void *)AES_IV(2), 0);
	outp32((void *)AES_IV(3), 0);
}

/**
  * @brief  Open SHA encrypt function.
  * @param[in]  CRPT        Reference to Crypto module.
  * @param[in]  u32OpMode   SHA operation mode, including:
  *         - \ref SHA_MODE_SHA1
  *         - \ref SHA_MODE_SHA224
  *         - \ref SHA_MODE_SHA256
  *         - \ref SHA_MODE_SHA384
  *         - \ref SHA_MODE_SHA512
  * @param[in]  u32SwapType is SHA input/output data swap control, including:
  *         - \ref SHA_NO_SWAP
  *         - \ref SHA_OUT_SWAP
  *         - \ref SHA_IN_SWAP
  *         - \ref SHA_IN_OUT_SWAP
  * @param[in]  hmac_key_len   HMAC key byte count
  * @return None
  */
void SHA_Open(unsigned int u32OpMode, unsigned int u32SwapType, unsigned int hmac_key_len)
{
	outp32((void *)HMAC_CTL, (u32OpMode << HMAC_CTL_OPMODE_OFFSET) |
		(u32SwapType << HMAC_CTL_OUTSWAP_OFFFSET));

	if (hmac_key_len != 0UL)
	{
		outp32(HMAC_KEYCNT, hmac_key_len);
		outp32(HMAC_CTL, inp32(HMAC_CTL)|HMAC_CTL_HMACEN);
	}
}

/**
  * @brief  Start SHA encrypt
  * @param[in]  CRPT        Reference to Crypto module.
  * @param[in]  u32DMAMode  TDES DMA control, including:
  *         - \ref CRYPTO_DMA_ONE_SHOT   One shop SHA encrypt.
  *         - \ref CRYPTO_DMA_CONTINUE   Continuous SHA encrypt.
  *         - \ref CRYPTO_DMA_LAST       Last SHA encrypt of a series of SHA_Start.
  * @return None
  */
void SHA_Start(unsigned int u32DMAMode)
{
	outp32(HMAC_CTL, inp32(HMAC_CTL)&~(0x7UL << HMAC_CTL_DMALAST_OFFSET));
	outp32(HMAC_CTL, inp32(HMAC_CTL)|HMAC_CTL_START | (u32DMAMode << HMAC_CTL_DMALAST_OFFSET));
}

/**
  * @brief  Set SHA DMA transfer
  * @param[in]  CRPT         Reference to Crypto module.
  * @param[in]  u32SrcAddr   SHA DMA source address
  * @param[in]  u32TransCnt  SHA DMA transfer byte count
  * @return None
  */
void SHA_SetDMATransfer(unsigned int u32SrcAddr, unsigned int u32TransCnt)
{
	outp32(HMAC_SADDR, u32SrcAddr);
	outp32(HMAC_DMACNT, u32TransCnt);
}

/**
  * @brief  Read the SHA digest.
  * @param[in]  CRPT        Reference to Crypto module.
  * @param[out]  u32Digest  The SHA encrypt output digest.
  * @return None
  */
void SHA_Read(unsigned int u32Digest[])
{
	unsigned int  i, wcnt;
	void *reg_addr;

	i = (inp32(HMAC_CTL) & HMAC_CTL_OPMODE_MASK ) >> HMAC_CTL_OPMODE_OFFSET;
	//i = (CRPT->HMAC_CTL & CRPT_HMAC_CTL_OPMODE_Msk) >> CRPT_HMAC_CTL_OPMODE_Pos;
	wcnt = 8UL;

	reg_addr = (void *)((unsigned long)&inp32(HMAC_DGST(0)));
	/* reg_addr = (void *)((unsigned long)&(CRPT->HMAC_DGST[0]));*/
	for (i = 0UL; i < wcnt; i++)
	{
		u32Digest[i] = inp32(reg_addr);
		reg_addr += 4UL;
	}
}

/**
  * @brief  ECDSA dogotal signature verification.
  * @param[in]  crpt        Reference to Crypto module.
  * @param[in]  ecc_curve   The pre-defined ECC curve.
  * @param[in]  message     The hash value of source context.
  * @param[in]  x_ksnum     -1:  do not use Key Store, instead, use <public_x> as input public key X
  *                         x_ksnum >= 0x80: Use Key Store OTP key number "x_ksnum - 0x80" as input public key X
  *                         x_ksnum >= 0:    Use Key Store SRAM key number x_ksnum as input public key X
  * @param[in]  y_ksnum     -1:  do not use Key Store, instead, use <public_y> as input public key Y
  *                         y_ksnum >= 0x80: Use Key Store OTP key number "y_ksnum - 0x80" as input public key Y
  *                         y_ksnum >= 0:    Use Key Store SRAM key number y_ksnum as input public key Y
  * @param[in]  R           R of the (R,S) pair digital signature
  * @param[in]  S           S of the (R,S) pair digital signature
  * @return  0    Success.
  * @return  -1   "ecc_curve" value is invalid.
  * @return  -2   Verification failed.
  * @return  -3   KS error
  */
int ECC_VerifySignature_KS(char *message, int x_ksnum, int y_ksnum, char *R, char *S)
{
	char temp_hex_str[64];
	unsigned int temp_result1[18], temp_result2[18];
	unsigned int temp_x[18], temp_y[18];
	unsigned int ksxy;
	int i, ret = 0;

	/*
	 *   1. Verify that r and s are integers in the interval [1, n-1]. If not, the signature is invalid
	 *   2. Compute e = HASH (m), where HASH is the hashing algorithm in signature generation
	 *      (1) Use SHA to calculate e
	 */

	/*
	 *   3. Compute w = s^-1 (mod n)
	 *      (1) Write the curve order to N registers
	 *      (2) Write 0x1 to Y1 registers
	 *      (3) Write s to X1 registers
	 *      (4) Set ECCOP(CRPT_ECC_CTL[10:9]) to 01
	 *      (5) Set MOPOP(CRPT_ECC_CTL[12:11]) to 00
	 *      (6) Set FSEL(CRPT_ECC_CTL[8]) according to used curve of prime field or binary field
	 *      (7) Set START(CRPT_ECC_CTL[0]) to 1
	 *      (8) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
	 *      (9) Read X1 registers to get w
	 */

	if (ecc_init_curve() != 0)
	{
		ret = -1;
	}

	if (ret == 0)
	{
		/*  3-(1) Write the curve order to N registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_N(i), 0UL);
		}
		Hex2Reg(pCurve->Eorder, (unsigned int *)ECC_N(0));

		/*  3-(2) Write 0x1 to Y1 registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_Y1(i), 0UL);
		}
		outp32(ECC_Y1(0), 0x1UL);

		/*  3-(3) Write s to X1 registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_X1(i), 0UL);
		}
		Hex2Reg(S, (unsigned int *)ECC_X1(0));

		run_ecc_codec(ECCOP_MODULE | MODOP_DIV);

		/*  3-(9) Read X1 registers to get w */
		for (i = 0; i < 18; i++)
		{
			temp_result2[i] = inp32(ECC_X1(i));
		}

		/*
		 *   4. Compute u1 = e \A1\D1 w (mod n) and u2 = r \A1\D1 w (mod n)
		 *      (1) Write the curve order and curve length to N ,M registers
		 *      (2) Write e, w to X1, Y1 registers
		 *      (3) Set ECCOP(CRPT_ECC_CTL[10:9]) to 01
		 *      (4) Set MOPOP(CRPT_ECC_CTL[12:11]) to 01
		 *      (5) Set START(CRPT_ECC_CTL[0]) to 1
		 *      (6) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
		 *      (7) Read X1 registers to get u1
		 *      (8) Write the curve order and curve length to N ,M registers
		 *      (9) Write r, w to X1, Y1 registers
		 *      (10) Set ECCOP(CRPT_ECC_CTL[10:9]) to 01
		 *      (11) Set MOPOP(CRPT_ECC_CTL[12:11]) to 01
		 *      (12) Set START(CRPT_ECC_CTL[0]) to 1
		 *      (13) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
		 *      (14) Read X1 registers to get u2
		 */

		/*  4-(1) Write the curve order and curve length to N ,M registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_N(i), 0UL);
		}
		Hex2Reg(pCurve->Eorder, (unsigned int *)ECC_N(0));

		/* 4-(2) Write e, w to X1, Y1 registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_X1(i), 0UL);
		}
		Hex2Reg(message, (unsigned int *)ECC_X1(0));

		for (i = 0; i < 18; i++)
		{
			outp32(ECC_Y1(i), temp_result2[i]);
		}

		run_ecc_codec(ECCOP_MODULE | MODOP_MUL);

		/*  4-(7) Read X1 registers to get u1 */
		for (i = 0; i < 18; i++)
		{
			temp_result1[i] = inp32(ECC_X1(i));
		}

		/*  4-(8) Write the curve order and curve length to N ,M registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_N(i), 0UL);
		}
		Hex2Reg(pCurve->Eorder, (unsigned int *)ECC_N(0));

		/* 4-(9) Write r, w to X1, Y1 registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_X1(i), 0UL);
		}
		Hex2Reg(R, (unsigned int *)ECC_X1(0));

		for (i = 0; i < 18; i++)
		{
			outp32(ECC_Y1(i), temp_result2[i]);
		}

		run_ecc_codec(ECCOP_MODULE | MODOP_MUL);

		/*  4-(14) Read X1 registers to get u2 */
		for (i = 0; i < 18; i++)
		{
			temp_result2[i] = inp32(ECC_X1(i));
		}

		/*
		 *   5. Compute X\A1\A6 (x1\A1\A6, y1\A1\A6) = u1 * G + u2 * Q
		 *      (1) Write the curve parameter A, B, N, and curve length M to corresponding registers
		 *      (2) Write the point G(x, y) to X1, Y1 registers
		 *      (3) Write u1 to K registers
		 *      (4) Set ECCOP(CRPT_ECC_CTL[10:9]) to 00
		 *      (5) Set START(CRPT_ECC_CTL[0]) to 1
		 *      (6) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
		 *      (7) Read X1, Y1 registers to get u1*G
		 *      (8) Write the curve parameter A, B, N, and curve length M to corresponding registers
		 *      (9) Write the public key Q(x,y) to X1, Y1 registers
		 *      (10) Write u2 to K registers
		 *      (11) Set ECCOP(CRPT_ECC_CTL[10:9]) to 00
		 *      (12) Set START(CRPT_ECC_CTL[0]) to 1
		 *      (13) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
		 *      (14) Write the curve parameter A, B, N, and curve length M to corresponding registers
		 *      (15) Write the result data u1*G to X2, Y2 registers
		 *      (16) Set ECCOP(CRPT_ECC_CTL[10:9]) to 10
		 *      (17) Set START(CRPT_ECC_CTL[0]) to 1
		 *      (18) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
		 *      (19) Read X1, Y1 registers to get X\A1\A6(x1\A1\A6, y1\A1\A6)
		 *      (20) Write the curve order and curve length to N ,M registers
		 *      (21) Write x1\A1\A6 to X1 registers
		 *      (22) Write 0x0 to Y1 registers
		 *      (23) Set ECCOP(CRPT_ECC_CTL[10:9]) to 01
		 *      (24) Set MOPOP(CRPT_ECC_CTL[12:11]) to 10
		 *      (25) Set START(CRPT_ECC_CTL[0]) to 1
		 *      (26) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
		 *      (27) Read X1 registers to get x1\A1\A6 (mod n)
		 *
		 *   6. The signature is valid if x1\A1\A6 = r, otherwise it is invalid
		 */

		/*
		 *  (1) Write the curve parameter A, B, N, and curve length M to corresponding registers
		 *  (2) Write the point G(x, y) to X1, Y1 registers
		 */
		ecc_init_curve();

		/* (3) Write u1 to K registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_K(i), temp_result1[i]);
		}

		run_ecc_codec(ECCOP_POINT_MUL);

		/* (7) Read X1, Y1 registers to get u1*G */
		for (i = 0; i < 18; i++)
		{
			temp_x[i] = inp32(ECC_X1(i));
			temp_y[i] = inp32(ECC_Y1(i));
		}

		/* (8) Write the curve parameter A, B, N, and curve length M to corresponding registers */
		ecc_init_curve();

		/* (9) Write the public key Q(x,y) to X1, Y1 registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_X1(i), 0UL);
			outp32(ECC_Y1(i), 0UL);
		}

		//Hex2Reg(public_k1, CRPT->ECC_X1);
		//Hex2Reg(public_k2, CRPT->ECC_Y1);

		ksxy = ECC_KSXY_RSRCXY;

		if (x_ksnum >= 0x80)
		{
			ksxy |= (2<<ECC_KSXY_RSSRCX_OFFSET) | (x_ksnum - 0x80);
		}
		else if (x_ksnum >= 0)
		{
			ksxy |= (0<<ECC_KSXY_RSSRCX_OFFSET) | (x_ksnum);
		}
		else
		{
			printf("--> 1\n");
			return -3;
		}

		if (y_ksnum >= 0x80)
		{
			ksxy |= (2<<ECC_KSXY_RSSRCX_OFFSET) | ((y_ksnum - 0x80) << 8);
		}
		else if (y_ksnum >= 0)
		{
			ksxy |= (0<<ECC_KSXY_RSSRCX_OFFSET) | (y_ksnum << 8);
		}
		else
		{
			printf("--> 2\n");
			return -3;
		}
		outp32(ECC_KSXY, ksxy);

		/* (10) Write u2 to K registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_K(i), temp_result2[i]);
		}

		run_ecc_codec(ECCOP_POINT_MUL);
		outp32(ECC_KSXY, 0);

		for (i = 0; i < 18; i++)
		{
			temp_result1[i] = inp32(ECC_X1(i));
			temp_result2[i] = inp32(ECC_Y1(i));
		}

		/* (14) Write the curve parameter A, B, N, and curve length M to corresponding registers */
		ecc_init_curve();

		/* Write the result data u2*Q to X1, Y1 registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_X1(i),temp_result1[i]);
			outp32(ECC_Y1(i),temp_result2[i]);
		}

		/* (15) Write the result data u1*G to X2, Y2 registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_X2(i),temp_x[i]);
			outp32(ECC_Y2(i),temp_y[i]);
		}

		run_ecc_codec(ECCOP_POINT_ADD);

		/* (19) Read X1, Y1 registers to get X\A1\A6(x1\A1\A6, y1\A1\A6) */
		for (i = 0; i < 18; i++)
		{
			temp_x[i] = inp32(ECC_X1(i));
			temp_y[i] = inp32(ECC_Y1(i));
		}

		/*  (20) Write the curve order and curve length to N ,M registers */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_N(i),0UL);
		}
		Hex2Reg(pCurve->Eorder, (unsigned int *)ECC_N(0));

		/*
		 *  (21) Write x1\A1\A6 to X1 registers
		 *  (22) Write 0x0 to Y1 registers
		 */
		for (i = 0; i < 18; i++)
		{
			outp32(ECC_X1(i),temp_x[i]);
			outp32(ECC_Y1(i),0UL);
		}

		run_ecc_codec(ECCOP_MODULE | MODOP_ADD);

		/*  (27) Read X1 registers to get x1\A1\A6 (mod n) */
		Reg2Hex(pCurve->Echar, (unsigned int *)ECC_X1(0), temp_hex_str);

		/* 6. The signature is valid if x1\A1\A6 = r, otherwise it is invalid */

		/* Compare with test pattern to check if r is correct or not */
		if (ecc_strcmp(temp_hex_str, R) != 0)
		{
			printf("--> 3\n");
			ret = -2;
		}
	}  /* ret == 0 */

	return ret;
}
