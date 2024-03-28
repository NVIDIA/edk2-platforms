/** @file
*
*  Copyright (c) 2024, Linaro Ltd. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#ifndef HARDWARE_INFO_LIB
#define HARDWARE_INFO_LIB

/**
  Get CPU count from information passed by Qemu.

**/
UINT32
GetCpuCount (
  VOID
  );

/**
  Get MPIDR for a given cpu from device tree passed by Qemu.

  @param [in]   CpuId    Index of cpu to retrieve MPIDR value for.

  @retval                MPIDR value of CPU at index <CpuId>
**/
UINT64
GetMpidr (
  IN UINTN  CpuId
  );

/**
  Get NUMA node id for a given cpu from device tree passed by Qemu.

  @param [in]   CpuId    Index of cpu to retrieve NUMA node id for.

  @retval                NUMA node id for CPU at index <CpuId>
**/
UINT64
GetCpuNumaNode (
  IN UINTN  CpuId
  );

#endif /* HARDWARE_INFO_LIB */
