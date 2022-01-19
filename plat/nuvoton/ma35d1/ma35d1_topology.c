/*
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
//#include <drivers/arm/ma35d1/ma35d1_pwrc.h>
#include <drivers/arm/fvp/fvp_pwrc.h>
#include <lib/cassert.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* System, one Cortex-A35 cluster and two cores. */
static const unsigned char ma35d1_power_domain_tree_desc[] = {
	1,
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CLUSTER0_CORE_COUNT,
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return ma35d1_power_domain_tree_desc;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return MA35D1_MAX_CPUS_PER_CLUSTER;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if ((mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) != 0U) {
		return -1;
	}

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		return -1;
	}

	/*
	 * Validate cpu_id by checking whether it represents a CPU in one
	 * of the two clusters present on the platform.
	 */
	if (cpu_id >= PLATFORM_CORE_COUNT) {
		return -1;
	}

	return (int)cpu_id;
}
