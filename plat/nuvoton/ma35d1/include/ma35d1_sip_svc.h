/*
 * Copyright (C) 2021 Nuvoton Technology Corp.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MA35D1_SIP_SVC_H
#define MA35D1_SIP_SVC_H

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_PMIC		0xC2000008
#define SIP_SVC_VERSION		0xC200000F

/* MA35D1 SiP Service Calls version numbers */
#define NVT_SIP_SVC_VERSION_MAJOR	0x0
#define NVT_SIP_SVC_VERSION_MINOR	0x1

/* Number of MA35D1 SiP Calls implemented */
#define NVT_COMMON_SIP_NUM_CALLS	0x3

enum {
	RK_SIP_E_SUCCESS = 0,
	RK_SIP_E_INVALID_PARAM = -1
};

#endif /* MA35D1_SIP_SVC_H */
