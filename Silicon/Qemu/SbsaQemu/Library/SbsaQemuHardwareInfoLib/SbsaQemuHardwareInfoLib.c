/** @file
*
*  Copyright (c) 2021, NUVIA Inc. All rights reserved.
*  Copyright (c) 2024, Linaro Ltd. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Library/ArmSmcLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/ResetSystemLib.h>
#include <Library/HardwareInfoLib.h>
#include <IndustryStandard/SbsaQemuSmc.h>

/**
  Get CPU count from information passed by Qemu.

**/
UINT32
GetCpuCount (
  VOID
  )
{
  UINTN          Arg0;
  UINTN          SmcResult;

  SmcResult = ArmCallSmc0 (SIP_SVC_GET_CPU_COUNT, &Arg0, NULL, NULL);
  if (SmcResult != SMC_SIP_CALL_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "%a: SIP_SVC_GET_CPU_COUNT call failed. We have no cpu information.\n", __FUNCTION__));
    ResetShutdown ();
  }

  DEBUG ((DEBUG_INFO, "%a: We have %d cpus.\n", __FUNCTION__, Arg0));

  return Arg0;
}

/**
  Get MPIDR for a given cpu from device tree passed by Qemu.

  @param [in]   CpuId    Index of cpu to retrieve MPIDR value for.

  @retval                MPIDR value of CPU at index <CpuId>
**/
UINT64
GetMpidr (
  IN UINTN  CpuId
  )
{
  UINTN  SmcResult;
  UINTN  Arg0;
  UINTN  Arg1;

  Arg0 = CpuId;

  SmcResult = ArmCallSmc0 (SIP_SVC_GET_CPU_NODE, &Arg0, &Arg1, NULL);
  if (SmcResult != SMC_SIP_CALL_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "%a: SIP_SVC_GET_CPU_NODE call failed. We have no MPIDR for CPU%d.\n", __FUNCTION__, CpuId));
    ResetShutdown ();
  }

  DEBUG ((DEBUG_INFO, "%a: MPIDR for CPU%d: = %d\n", __FUNCTION__, CpuId, Arg1));

  return Arg1;
}

/**
  Get NUMA node id for a given cpu from device tree passed by Qemu.

  @param [in]   CpuId    Index of cpu to retrieve NUMA node id for.

  @retval                NUMA node id for CPU at index <CpuId>
**/
UINT64
GetCpuNumaNode (
  IN UINTN  CpuId
  )
{
  UINTN  SmcResult;
  UINTN  Arg0;
  UINTN  Arg1;

  Arg0 = CpuId;

  SmcResult = ArmCallSmc0 (SIP_SVC_GET_CPU_NODE, &Arg0, &Arg1, NULL);
  if (SmcResult != SMC_SIP_CALL_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "%a: SIP_SVC_GET_CPU_NODE call failed. Could not find information for CPU%d.\n", __FUNCTION__, CpuId));
    return 0;
  }

  DEBUG ((DEBUG_INFO, "%a: NUMA node for CPU%d: = %d\n", __FUNCTION__, CpuId, Arg0));

  return Arg0;
}
