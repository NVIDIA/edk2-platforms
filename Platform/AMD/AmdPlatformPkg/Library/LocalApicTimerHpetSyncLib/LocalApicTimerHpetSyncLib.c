/** @file
  LocalApicTimerHpetSyncLib library implementation for AMD platforms.

  Copyright (C) 2025 Advanced Micro Devices, Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseMemoryLib.h>

// FILE_GUID of LocalApicTimerDxe.inf
#define LOCAL_APIC_TIMER_DXE_GUID \
  {0x52fe8196, 0xf9de, 0x4d07, {0xb2, 0x2f, 0x51, 0xf7, 0x7a, 0x0e, 0x7c, 0x41}}

// FILE_GUID of HpetTimerDxe.inf
#define HPET_TIMER_DXE_GUID \
  {0x6ce6b0de, 0x781c, 0x4f6c, {0xb4, 0x2d, 0x98, 0x34, 0x6c, 0x61, 0x4b, 0xec}}

EFI_GUID  mLocalApicTimerDxeGuid = LOCAL_APIC_TIMER_DXE_GUID;
EFI_GUID  mHpetTimerDxeGuid      = HPET_TIMER_DXE_GUID;

/**
  The constructor function of the LocalApicTimerHpetSyncLib.

  The constructor function of the LocalApicTimerHpetSyncLib. It checks if HPET is enabled or not.
  If HPET is enabled, then it will not load the local apic timer.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The constructor always returns EFI_SUCCESS.
  @retval EFI_DEVICE_ERROR  HPET is enabled, hence not loading the local apic timer.
**/
EFI_STATUS
EFIAPI
LocalApicTimerHpetSyncLibConstructor (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  BOOLEAN  HpetEnabled;
  UINT64   HpetCapabilities;

  HpetCapabilities = MmioRead64 (PcdGet32 (PcdHpetBaseAddress));
  HpetEnabled      = (HpetCapabilities != 0) && (HpetCapabilities != MAX_UINT64);

  if (HpetEnabled && CompareGuid (&gEfiCallerIdGuid, &mLocalApicTimerDxeGuid)) {
    DEBUG ((DEBUG_INFO, "HPET is enabled, Not Loading the driver. %g\n", gEfiCallerIdGuid));
    ExitDriver (EFI_DEVICE_ERROR);
  } else if (!HpetEnabled && CompareGuid (&gEfiCallerIdGuid, &mHpetTimerDxeGuid)) {
    DEBUG ((DEBUG_INFO, "HPET is not enabled, Not Loading the driver. %g\n", gEfiCallerIdGuid));
    ExitDriver (EFI_DEVICE_ERROR);
  }

  return EFI_SUCCESS;
}

/**
  The destructor function of the LocalApicTimerHpetSyncLib.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The destructor always returns EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
LocalApicTimerHpetSyncLibDestructor (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
