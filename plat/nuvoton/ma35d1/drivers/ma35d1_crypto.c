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

#include "../ma35d1_def.h"
#include "../include/ma35d1_crypto.h"
#include "../include/tsi_cmd.h"

#define ENABLE_DEBUG

#define LIB_NAME		"MA35D1 Crypto"

static int HAVE_TSI;

typedef struct e_curve_t {
	E_ECC_CURVE curve_id;
	int32_t Echar;
	char Ea[144];
	char Eb[144];
	char Px[144];
	char Py[144];
	int32_t Epl;
	char Pp[176];
	int32_t Eol;
	char Eorder[176];
	int32_t key_len;
	int32_t irreducible_k1;
	int32_t irreducible_k2;
	int32_t irreducible_k3;
	int32_t GF;
} ECC_CURVE;

static ECC_CURVE P256 = {
	/* NIST: Curve P-256 : y^2=x^3-ax+b (mod p) */
	CURVE_P_256,
	64,			/* Echar */
	"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",
	"5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
	"6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
	"4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",
	78,			/* Epl */
	"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
	78,			/* Eol */
	"FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551",
	256,			/* key_len */
	10,
	5,
	2,
	CURVE_GF_P
};

static ECC_CURVE *pCurve;

static char temp_hex_str[160];

__aligned(32) char param_block[4096];

void dump_ecc_reg(char *str, uint32_t reg_addr, uint32_t count)
{
	int i;
	uint32_t *regs = (uint32_t *) ((uint64_t) reg_addr);

	printf("%s => ", str);
	for (i = 0; i < count; i++) {
		printf("0x%08x ", regs[i]);
	}
	printf("\n");
}

static char ch2hex(char ch)
{
	if (ch <= '9') {
		ch = ch - '0';
	} else if ((ch <= 'z') && (ch >= 'a')) {
		ch = ch - 'a' + 10U;
	} else {
		ch = ch - 'A' + 10U;
	}
	return ch;
}

static int __strlen(char *str)
{
	char *p = str;
	int len = 0;

	while (*p != 0) {
		p++;
		len++;
		if (len > 1024)	/* max. 4096 bits */
			break;
	}
	return len;
}

static void Hex2Reg(char input[], uint32_t reg_addr)
{
	char hex;
	int si, ri;
	uint32_t i, val32;

	volatile uint32_t *reg = (volatile uint32_t *)((uint64_t) reg_addr);

	si = (int)__strlen(input) - 1;
	ri = 0;

	// printf("<%d>\n", si);

	while (si >= 0) {
		val32 = 0UL;
		for (i = 0UL; (i < 8UL) && (si >= 0); i++) {
			hex = ch2hex(input[si]);
			val32 |= (uint32_t) hex << (i * 4UL);
			si--;
		}
		reg[ri++] = val32;
	}
}

static const char hex_char_tbl[] = "0123456789abcdef";

/*
 * @brief  Extract specified nibble from an unsigned word in character format.
 *         For example:
 *                Suppose val32 is 0x786543210, get_Nth_nibble_char(val32, 3)
 *                will return a '3'.
 * @param[in]  val32   The input unsigned word
 * @param[in]  idx     The Nth nibble to be extracted.
 * @return  The nibble in character format.
 */
static char get_Nth_nibble_char(uint32_t val32, uint32_t idx)
{
	return hex_char_tbl[(val32 >> (idx * 4U)) & 0xfU];
}

static void Reg2Hex(int32_t count, uint32_t reg[], char output[])
{
	int32_t idx, ri;
	uint32_t i;

	output[count] = 0;
	idx = count - 1;

	for (ri = 0; idx >= 0; ri++) {
		for (i = 0UL; (i < 8UL) && (idx >= 0); i++) {
			output[idx] = get_Nth_nibble_char(reg[ri], i);
			idx--;
		}
	}
}

static void hex_to_string(unsigned char *hex, int count, char *str)
{
	for (; count > 0; hex++, count--) {
		*str++ = hex_char_tbl[(*hex >> 4) & 0xf];
		*str++ = hex_char_tbl[*hex & 0xf];
	}
	*str = 0;
}

static int ecc_strcmp(char *s1, char *s2)
{
	char c1, c2;

	while (*s1 == '0')
		s1++;
	while (*s2 == '0')
		s2++;

	for (; *s1 || *s2; s1++, s2++) {
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

static void ecc_init_curve(ECC_CURVE *curve)
{
	int i;

	pCurve = curve;

	for (i = 0; i < 18; i++) {
		outp32(ECC_A(i), 0);
		outp32(ECC_B(i), 0);
		outp32(ECC_X1(i), 0);
		outp32(ECC_Y1(i), 0);
		outp32(ECC_X2(i), 0);
		outp32(ECC_Y2(i), 0);
		outp32(ECC_N(i), 0);
	}

	Hex2Reg(pCurve->Ea, ECC_A(0));
	Hex2Reg(pCurve->Eb, ECC_B(0));
	Hex2Reg(pCurve->Px, ECC_X1(0));
	Hex2Reg(pCurve->Py, ECC_Y1(0));

	//dump_ecc_reg("CRPT_ECC_CURVE_A", ECC_A(0), 10);
	//dump_ecc_reg("CRPT_ECC_CURVE_B", ECC_B(0), 10);
	//dump_ecc_reg("CRPT_ECC_POINT_X1", ECC_X1(0), 10);
	//dump_ecc_reg("CRPT_ECC_POINT_Y1", ECC_Y1(0), 10);

	if (pCurve->GF == (int)CURVE_GF_2M) {
		outp32(ECC_N(0), 0x1);
		i = pCurve->key_len / 32;
		outp32(ECC_N(i),
		       inp32(ECC_N(i)) | (1 << (pCurve->key_len % 32)));
		i = (pCurve->irreducible_k1) / 32;
		outp32(ECC_N(i),
		       inp32(ECC_N(i)) | (1 << (pCurve->irreducible_k1 % 32)));
		i = pCurve->irreducible_k2 / 32;
		outp32(ECC_N(i),
		       inp32(ECC_N(i)) | (1 << (pCurve->irreducible_k2 % 32)));
		i = pCurve->irreducible_k3 / 32;
		outp32(ECC_N(i),
		       inp32(ECC_N(i)) | (1 << (pCurve->irreducible_k3 % 32)));
	} else {
		Hex2Reg(pCurve->Pp, ECC_N(0));
	}
	//dump_ecc_reg("CRPT_ECC_CURVE_N", ECC_N(0), 10);
}

static void run_ecc_codec(uint32_t mode, int enable_scap)
{
	uint32_t ctrl;

	if ((mode & ECC_CTL_ECCOP_MASK) == ECCOP_MODULE) {
		ctrl = ECC_CTL_FSEL;
	} else {
		if (pCurve->GF == (int)CURVE_GF_2M) {
			/* point */
			ctrl = 0;
		} else {
			/* CURVE_GF_P */
			ctrl = ECC_CTL_FSEL;
		}
	}

	if (enable_scap) {
		/* enable side-channel attack protection */
		ctrl |= ECC_CTL_SCAP;
	}

	ctrl |= ((uint32_t) pCurve->key_len << ECC_CTL_CURVEM_OFFSET) | mode;
	outp32(ECC_CTL, (ctrl | ECC_CTL_START));

	while (inp32(ECC_STS) & ECC_CTL_START)
		;
	while (inp32(ECC_STS) & ECC_STS_BUSY)
		;
}

/*
 * @brief  ECDSA digital signature verification.
 * @param[in]  ecc_curve   The pre-defined ECC curve.
 * @param[in]  message     The hash value of source context.
 * @param[in]  public_k1   The public key 1.
 * @param[in]  public_k2   The public key 2.
 * @param[in]  R           R of the (R,S) pair digital signature
 * @param[in]  S           S of the (R,S) pair digital signature
 * @return  0    Success.
 * @return  -1   "ecc_curve" value is invalid.
 * @return  -2   Verification failed.
 */
int ecc_p256_verify(char *message, char *public_k1, char *public_k2,
		    char *R, char *S)
{
	uint32_t temp_result1[18], temp_result2[18];
	uint32_t temp_x[18], temp_y[18];
	int i, ret = 0;

	/*
	 *   1. Verify that r and s are integers in the interval [1, n-1].
	 *      If not, the signature is invalid
	 *   2. Compute e = HASH (m), where HASH is the hashing algorithm in
	 *      signature generation
	 */

	/*
	 *   3. Compute w = s^-1 (mod n)
	 *      (1) Write the curve order to N registers
	 *      (2) Write 0x1 to Y1 registers
	 *      (3) Write s to X1 registers
	 *      (4) Set ECCOP(CRPT_ECC_CTL[10:9]) to 01
	 *      (5) Set MOPOP(CRPT_ECC_CTL[12:11]) to 00
	 *      (6) Set FSEL(CRPT_ECC_CTL[8]) according to used curve of prime
	 *          field or binary field
	 *      (7) Set START(CRPT_ECC_CTL[0]) to 1
	 *      (8) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
	 *      (9) Read X1 registers to get w
	 */
	ecc_init_curve(&P256);

	/*  3-(1) Write the curve order to N registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_N(i), 0);
	}
	Hex2Reg(pCurve->Eorder, ECC_N(0));

	/*  3-(2) Write 0x1 to Y1 registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_Y1(i), 0);
	}
	outp32(ECC_Y1(0), 0x1);

	/*  3-(3) Write s to X1 registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_X1(i), 0);
	}
	Hex2Reg(S, ECC_X1(0));

	run_ecc_codec(ECCOP_MODULE | MODOP_DIV, 0);

	/*  3-(9) Read X1 registers to get w */
	for (i = 0; i < 18; i++) {
		temp_result2[i] = inp32(ECC_X1(i));
	}

#ifdef ENABLE_DEBUG
	printf("e = %s\n", message);
	Reg2Hex(pCurve->Echar, temp_result2, temp_hex_str);
	printf("w = %s\n", temp_hex_str);
	printf("o = %s (order)\n", pCurve->Eorder);
#endif

	/*
	 *   4. Compute u1 = e ｝ w (mod n) and u2 = r ｝ w (mod n)
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
	for (i = 0; i < 18; i++) {
		outp32(ECC_N(i), 0);
	}
	Hex2Reg(pCurve->Eorder, ECC_N(0));

	/* 4-(2) Write e, w to X1, Y1 registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_X1(i), 0);
	}
	Hex2Reg(message, ECC_X1(0));

	for (i = 0; i < 18; i++) {
		outp32(ECC_Y1(i), temp_result2[i]);
	}

	run_ecc_codec(ECCOP_MODULE | MODOP_MUL, 0);

	/*  4-(7) Read X1 registers to get u1 */
	for (i = 0; i < 18; i++) {
		temp_result1[i] = inp32(ECC_X1(i));
	}

#ifdef ENABLE_DEBUG
	Reg2Hex(pCurve->Echar, temp_result1, temp_hex_str);
	printf("u1 = %s\n", temp_hex_str);
#endif

	/*  4-(8) Write the curve order and curve length to N ,M registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_N(i), 0);
	}
	Hex2Reg(pCurve->Eorder, ECC_N(0));

	/* 4-(9) Write r, w to X1, Y1 registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_X1(i), 0);
	}
	Hex2Reg(R, ECC_X1(0));

	for (i = 0; i < 18; i++) {
		outp32(ECC_Y1(i), temp_result2[i]);
	}

	run_ecc_codec(ECCOP_MODULE | MODOP_MUL, 0);

	/*  4-(14) Read X1 registers to get u2 */
	for (i = 0; i < 18; i++) {
		temp_result2[i] = inp32(ECC_X1(i));
	}

#ifdef ENABLE_DEBUG
	Reg2Hex(pCurve->Echar, temp_result2, temp_hex_str);
	printf("u2 = %s\n", temp_hex_str);
#endif

	/*
	 *   5. Compute X・ (x1・, y1・) = u1 * G + u2 * Q
	 *      (1) Write the curve parameter A, B, N, and curve length M to
	 *          corresponding registers
	 *      (2) Write the point G(x, y) to X1, Y1 registers
	 *      (3) Write u1 to K registers
	 *      (4) Set ECCOP(CRPT_ECC_CTL[10:9]) to 00
	 *      (5) Set START(CRPT_ECC_CTL[0]) to 1
	 *      (6) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
	 *      (7) Read X1, Y1 registers to get u1*G
	 *      (8) Write the curve parameter A, B, N, and curve length M to
	 *          corresponding registers
	 *      (9) Write the public key Q(x,y) to X1, Y1 registers
	 *      (10) Write u2 to K registers
	 *      (11) Set ECCOP(CRPT_ECC_CTL[10:9]) to 00
	 *      (12) Set START(CRPT_ECC_CTL[0]) to 1
	 *      (13) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
	 *      (14) Write the curve parameter A, B, N, and curve length M to
	 *           corresponding registers
	 *      (15) Write the result data u1*G to X2, Y2 registers
	 *      (16) Set ECCOP(CRPT_ECC_CTL[10:9]) to 10
	 *      (17) Set START(CRPT_ECC_CTL[0]) to 1
	 *      (18) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
	 *      (19) Read X1, Y1 registers to get X・(x1・, y1・)
	 *      (20) Write the curve order and curve length to N ,M registers
	 *      (21) Write x1・ to X1 registers
	 *      (22) Write 0x0 to Y1 registers
	 *      (23) Set ECCOP(CRPT_ECC_CTL[10:9]) to 01
	 *      (24) Set MOPOP(CRPT_ECC_CTL[12:11]) to 10
	 *      (25) Set START(CRPT_ECC_CTL[0]) to 1
	 *      (26) Wait for BUSY(CRPT_ECC_STS[0]) be cleared
	 *      (27) Read X1 registers to get x1・ (mod n)
	 *
	 *   6. The signature is valid if x1・ = r, otherwise it is invalid
	 */

	/*
	 *  (1) Write the curve parameter A, B, N, and curve length M to
	 *      corresponding registers.
	 *  (2) Write the point G(x, y) to X1, Y1 registers
	 */
	ecc_init_curve(&P256);

	/* (3) Write u1 to K registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_K(i), temp_result1[i]);
	}

	for (i = 0; i < 18; i++) {
		outp32(ECC_X2(i), 0);
	}
	Hex2Reg(pCurve->Eorder, ECC_X2(0));

	run_ecc_codec(ECCOP_POINT_MUL, 0);

	/* (7) Read X1, Y1 registers to get u1*G */
	for (i = 0; i < 18; i++) {
		temp_x[i] = inp32(ECC_X1(i));
		temp_y[i] = inp32(ECC_Y1(i));
	}

#ifdef ENABLE_DEBUG
	Reg2Hex(pCurve->Echar, temp_x, temp_hex_str);
	printf("5-(7) u1*G, x = %s\n", temp_hex_str);
	Reg2Hex(pCurve->Echar, temp_y, temp_hex_str);
	printf("5-(7) u1*G, y = %s\n", temp_hex_str);
#endif

	/* (8) Write the curve parameter A, B, N, and curve length M to
	 *     corresponding registers.
	 */
	ecc_init_curve(&P256);

	/* (9) Write the public key Q(x,y) to X1, Y1 registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_X1(i), 0);
		outp32(ECC_Y1(i), 0);
	}

	Hex2Reg(public_k1, ECC_X1(0));
	Hex2Reg(public_k2, ECC_Y1(0));

	/* (10) Write u2 to K registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_K(i), temp_result2[i]);
	}

	for (i = 0; i < 18; i++) {
		outp32(ECC_X2(i), 0);
	}
	Hex2Reg(pCurve->Eorder, ECC_X2(0));

	run_ecc_codec(ECCOP_POINT_MUL, 0);

	for (i = 0; i < 18; i++) {
		temp_result1[i] = inp32(ECC_X1(i));
		temp_result2[i] = inp32(ECC_Y1(i));
	}

#ifdef ENABLE_DEBUG
	Reg2Hex(pCurve->Echar, temp_result1, temp_hex_str);
	printf("5-(13) u2*Q, x = %s\n", temp_hex_str);
	Reg2Hex(pCurve->Echar, temp_result2, temp_hex_str);
	printf("5-(13) u2*Q, y = %s\n", temp_hex_str);
#endif

	/* (14) Write the curve parameter A, B, N, and curve length M to
	 * corresponding registers.
	 */
	ecc_init_curve(&P256);

	/* Write the result data u2*Q to X1, Y1 registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_X1(i), temp_result1[i]);
		outp32(ECC_Y1(i), temp_result2[i]);
	}

	/* (15) Write the result data u1*G to X2, Y2 registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_X2(i), temp_x[i]);
		outp32(ECC_Y2(i), temp_y[i]);
	}

	run_ecc_codec(ECCOP_POINT_ADD, 0);

	/* (19) Read X1, Y1 registers to get X・(x1・, y1・) */
	for (i = 0; i < 18; i++) {
		temp_x[i] = inp32(ECC_X1(i));
		temp_y[i] = inp32(ECC_Y1(i));
	}

#ifdef ENABLE_DEBUG
	Reg2Hex(pCurve->Echar, temp_x, temp_hex_str);
	printf("5-(19) x' = %s\n", temp_hex_str);
	Reg2Hex(pCurve->Echar, temp_y, temp_hex_str);
	printf("5-(19) y' = %s\n", temp_hex_str);
#endif

	/*  (20) Write the curve order and curve length to N ,M registers */
	for (i = 0; i < 18; i++) {
		outp32(ECC_N(i), 0);
	}
	Hex2Reg(pCurve->Eorder, ECC_N(0));

	/*
	 *  (21) Write x1・ to X1 registers
	 *  (22) Write 0x0 to Y1 registers
	 */
	for (i = 0; i < 18; i++) {
		outp32(ECC_X1(i), temp_x[i]);
		outp32(ECC_Y1(i), 0);
	}

#ifdef ENABLE_DEBUG
	Reg2Hex(pCurve->Echar, (uint32_t *) (ECC_X1(0)), temp_hex_str);
	printf("5-(21) x' = %s\n", temp_hex_str);
	Reg2Hex(pCurve->Echar, (uint32_t *) (ECC_Y1(0)), temp_hex_str);
	printf("5-(22) y' = %s\n", temp_hex_str);
#endif

	run_ecc_codec(ECCOP_MODULE | MODOP_ADD, 0);

	/*  (27) Read X1 registers to get x1・ (mod n) */
	Reg2Hex(pCurve->Echar, (uint32_t *) (ECC_X1(0)), temp_hex_str);
	printf("5-(27) x1' (mod n) = %s\n", temp_hex_str);

	/* 6. The signature is valid if x1・ = r, otherwise it is invalid */

	/* Compare with test pattern to check if r is correct or not */
	if (ecc_strcmp(temp_hex_str, R) != 0) {
		printf("x1' (mod n) != R Test filed!!\n");
		printf
		    ("Signature R [%s] is not matched with expected R [%s]!\n",
		     temp_hex_str, R);
		ret = -2;
	}

	return ret;
}

/*
 * Initialize the library and export the descriptor
 */
static void init(void)
{
	uint32_t ver;

	if (inp32(SYS_CHIPCFG) & (1 << 8)) {
		HAVE_TSI = 1;

		/* enable wormhole clock */
		outp32(CLK_SYSCLK1, inp32(CLK_SYSCLK1) | 0x30);

		printf("TSI sync...\n");
		while (1) {
			if (TSI_Sync() == 0) {
				if (TSI_Get_Version(&ver) == 0) {
					printf("TSI version: %x\n", ver);
					break;
				}
			}
		}
		printf("[OK]\n");
	} else {
		HAVE_TSI = 0;
		do {
			outp32(TSI_SYS_BASE + 0x100, 0x59);
			outp32(TSI_SYS_BASE + 0x100, 0x16);
			outp32(TSI_SYS_BASE + 0x100, 0x88);
		} while (inp32(TSI_SYS_BASE + 0x100) == 0UL);

		/* CLK->PLLCTL = 0x808C90;  PLL source from HIRC 144 MHz */
		outp32(TSI_CLK_BASE + 0x40, 0x808C90);

		/* wait PLL stable */
		while ((inp32(TSI_CLK_BASE + 0x50) & 0x4) == 0)
			;

		/* Select TSI HCLK from PLL */
		outp32(TSI_CLK_BASE + 0x10,
		       (inp32(TSI_CLK_BASE + 0x10) & ~0x7) | 0x2);

		/* enable crypto engine clock */
		outp32(TSI_CLK_BASE + 0x4,
		       inp32(TSI_CLK_BASE + 0x4) | (1 << 12));
	}
}

int TSI_run_sha(int inswap, int outswap, int mode_sel, int hmac,
		int mode, int keylen, int ks, int ks_num,
		int wcnt, int data_cnt, uint32_t src_addr, uint32_t dest_addr)
{
	int sid, ret;

	ret = TSI_Open_Session(C_CODE_SHA, &sid);
	if (ret != 0)
		goto err_out;

	ret =
	    TSI_SHA_Start(sid, inswap, outswap, mode_sel, hmac, mode, keylen,
			  ks, ks_num);
	if (ret != 0)
		goto err_out;

	ret = TSI_SHA_Finish(sid, wcnt, data_cnt, src_addr, dest_addr);
	if (ret != 0)
		goto err_out;

	TSI_Close_Session(C_CODE_SHA, sid);
	return 0;

err_out:
	printf("TSI ERROR!!! 0x%x\n", ret);
	TSI_Close_Session(C_CODE_SHA, sid);
	return ret;
}

/*
 * Calculate a hash
 *
 * output points to the computed hash
 */
int calc_hash(uint32_t alg, void *data_ptr,
	      uint32_t data_len, unsigned char *output)
{
	uint32_t ctrl;
	int ret;

	flush_dcache_range((uintptr_t) data_ptr, data_len);

	if (HAVE_TSI == 0) {
		ctrl = HMAC_CTL_INSWAP | HMAC_CTL_OUTSWAP |
		    SHA_OPMODE_SHA256 | HMAC_CTL_DMAEN | HMAC_CTL_DMAFIRST |
		    HMAC_CTL_DMALAST | HMAC_CTL_START;

		outp32(INTSTS, (INTSTS_HMACIF | INTSTS_HMACEIF));
		outp32(INTEN, (INTEN_HMACIEN | INTEN_HMACEIEN));

		outp32(HMAC_KEYCNT, 0);
		outp32(HMAC_DMACNT, data_len);
		outp32(HMAC_SADDR, (uint32_t) ((uint64_t) data_ptr));
		outp32(HMAC_CTL, ctrl);
		memset(output, 0, 32);

		while (inp32(HMAC_STS) & (HMAC_STS_BUSY | HMAC_STS_DMABUSY))
			;
		while ((inp32(INTSTS) & (INTSTS_HMACIF | INTSTS_HMACEIF)) == 0)
			;
		outp32(INTSTS, (INTSTS_HMACIF | INTSTS_HMACEIF));

		memcpy(output, (unsigned char *)HMAC_DGST(0), 32);
	} else {
		ret = TSI_run_sha(1,	/* inswap        */
				  1,	/* outswap       */
				  0,	/* mode_sel      */
				  0,	/* hmac          */
				  0x04,	/* mode (SHA256) */
				  0,	/* keylen (hmac) */
				  0,	/* ks            */
				  0,	/* ks_num        */
				  8,	/* wcnt          */
				  data_len,	/* data_cnt */
				  (uint32_t) ((uint64_t) data_ptr), /* src_addr  */
				  (uint32_t) ((uint64_t) output)    /* dest_addr */
		    );
		if (ret != 0) {
			printf("TSI SHA ERROR!!! 0x%x\n", ret);
			return -1;
		}
	}
	return 0;
}

/*
 * Match a hash
 *
 * Digest info is passed in DER format following the ASN.1 structure detailed
 * above.
 */
int verify_hash(void *data_ptr, uint32_t data_len,
		void *digest_info_ptr, uint32_t digest_info_len)
{
	int ret;
	unsigned char digest[64];

	ret = calc_hash(0, data_ptr, data_len, digest);
	if (ret != 0)
		return ret;

	if (memcmp(digest, digest_info_ptr, 32) == 0)
		return CRYPTO_SUCCESS;
	else
		return -1;
}

/*
 * Verify a signature.
 *
 * Parameters are passed using the DER encoding format following the ASN.1
 * structures detailed above.
 */
static int verify_signature(void *data_ptr, uint32_t data_len,
			    void *sig_ptr, uint32_t sig_len,
			    void *sig_alg, uint32_t sig_alg_len,
			    void *pk_ptr, uint32_t pk_len)
{
	int ret;
	unsigned char digest[36];
	char message[128];

	memset(digest, 0, sizeof(digest));
	ret = calc_hash(0, data_ptr, data_len, digest);
	if (ret != 0)
		return ret;

	hex_to_string(digest, 32, message);

	if (HAVE_TSI == 0) {

		ret =
		    ecc_p256_verify(message, (char *)pk_ptr,
				    (char *)pk_ptr + 64, (char *)sig_ptr,
				    (char *)sig_ptr + 64);
	} else {
		memset(param_block, 0, sizeof(param_block));
		memcpy(param_block, message, 64);
		memcpy(&param_block[576], pk_ptr, 64);
		memcpy(&param_block[1152], pk_ptr + 64, 64);
		memcpy(&param_block[1728], sig_ptr, 64);
		memcpy(&param_block[2304], sig_ptr + 64, 64);

		ret = TSI_ECC_VerifySignature(CURVE_P_256,	/* curve_id   */
					      ECC_KEY_SEL_USER,	/* psel       */
					      0,	/* x_knum     */
					      0,	/* y_knum     */
					      (uint32_t) ((uint64_t) param_block));
		if (ret != 0) {
			printf("TSI ECC signature verification failed!!\n");
			return -1;
		}
	}
	return ret;
}

int aes256_ofb_decrypt(uint32_t *key, void *src_data, void *output,
		       uint32_t data_len)
{
	int i, sid, ret;
	uint32_t ctrl;
	uint32_t aes_iv[4] = { 0, 0, 0, 0 };

	flush_dcache_range((uintptr_t) src_data, data_len);
	inv_dcache_range((uintptr_t) output, data_len);

	if (HAVE_TSI)
		goto tsi_aes;

	outp32(AES_CTL, 0);
	outp32(INTEN, (INTEN_AESIEN | INTEN_AESEIEN));
	outp32(INTSTS, (INTSTS_AESIF | INTSTS_AESEIF));

	/* program AES key */
	for (i = 0; i < 8; i++) {
		outp32(AES_KEY(i), key[i]);
	}

	/* program AES IV */
	for (i = 0; i < 4; i++)
		outp32(AES_IV(i), 0);

	ctrl = (AES_KEYSZ_SEL_256 | AES_MODE_OFB | AES_CTL_INSWAP |
		AES_CTL_OUTSWAP | AES_CTL_KINSWAP | AES_CTL_KOUTSWAP |
		AES_CTL_DMAEN);

	outp32(AES_CNT, data_len);
	outp32(AES_SADDR, (uint32_t) ((uint64_t) src_data));
	outp32(AES_DADDR, (uint32_t) ((uint64_t) output));

	/* start AES */
	outp32(AES_CTL, ctrl | AES_CTL_START);

	while ((inp32(INTSTS) & (INTSTS_AESIF | INTSTS_AESEIF)) == 0)
		;
	outp32(INTSTS, (INTSTS_AESIF | INTSTS_AESEIF));

	return 0;

tsi_aes:
	ret = TSI_Open_Session(C_CODE_AES, &sid);
	if (ret != 0)
		goto err_out;

	ret = TSI_AES_Set_IV(sid, (uint32_t) ((uint64_t) aes_iv));
	if (ret != 0)
		goto err_out;

	ret =
	    TSI_AES_Set_Key(sid, AES_KEY_SIZE_256, (uint32_t) ((uint64_t) key));
	if (ret != 0)
		goto err_out;

	ret = TSI_AES_Set_Mode(sid,	/* sid        */
			       0,	/* kinswap    */
			       0,	/* koutswap   */
			       1,	/* inswap     */
			       1,	/* outswap    */
			       0,	/* sm4en      */
			       0,	/* encrypt    */
			       0x3,	/* mode; OFB  */
			       AES_KEY_SIZE_256,	/* keysz      */
			       0,	/* ks         */
			       0	/* ks_num     */
	    );
	if (ret != 0)
		goto err_out;

	ret = TSI_AES_Run(sid, 1, data_len, (uint32_t) ((uint64_t) src_data),
			  (uint32_t) ((uint64_t) output));
	if (ret != 0)
		goto err_out;

	TSI_Close_Session(C_CODE_AES, sid);
	return 0;

err_out:
	TSI_Close_Session(C_CODE_AES, sid);
	printf("TSI ERROR!!! 0x%x\n", ret);
	return ret;
}

/*
 * Register crypto library descriptor
 */
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, NULL);
