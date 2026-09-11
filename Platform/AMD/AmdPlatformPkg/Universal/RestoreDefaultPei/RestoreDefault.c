/** @file
  Driver to restore default status in PEI phase for NV.

  Copyright (C) 2023 - 2024, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include "Cmos.h"

/**
  Switch NV Store PCDs to point to backup default image in SPI.

  @retval EFI_SUCCESS       Successfully changed NV Store PCDs to Default Store.
  @retval EFI_DEVICE_ERROR  Failed to change NV Store PCDs to Default Store.

**/
EFI_STATUS
EFIAPI
NvStoreSwitchToDefault (
  VOID
  )
{
  DEBUG ((DEBUG_ERROR, "%a: Switching to default variable store.\n", __func__));
  if ((PcdGet32 (PcdFlashNvStorageVariablePrimaryOffset) == 0) ||
      (PcdGet32 (PcdFlashNvStorageVariableDefaultOffset) == 0))
  {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Not supported (PrimaryOffset=0x%X, DefaultOffset=0x%X).\n",
      __func__,
      PcdGet32 (PcdFlashNvStorageVariablePrimaryOffset),
      PcdGet32 (PcdFlashNvStorageVariableDefaultOffset)
      ));
    return EFI_SUCCESS;
  }

  DEBUG ((
    DEBUG_ERROR,
    "%a: Before: VarBase=0x%X, VarBase64=0x%lX\n",
    __func__,
    PcdGet32 (PcdFlashNvStorageVariableBase),
    PcdGet64 (PcdFlashNvStorageVariableBase64)
    ));

  PcdSet32S (PcdFlashNvStorageVariableBase, PcdGet32 (PcdFlashNvStorageVariableDefaultBase));

  DEBUG ((
    DEBUG_ERROR,
    "%a: After:  VarBase=0x%X, VarBase64=0x%lX, DefaultBase=0x%X\n",
    __func__,
    PcdGet32 (PcdFlashNvStorageVariableBase),
    PcdGet64 (PcdFlashNvStorageVariableBase64),
    PcdGet32 (PcdFlashNvStorageVariableDefaultBase)
    ));
  return EFI_SUCCESS;
}

/**
  Initiate restore of system defaults.
  Check CMOS (and possibly reset).
  Select NV Store default backup.

  @param[in] FileHandle     Handle of the file being invoked.
  @param[in] PeiServices    Describes the list of possible PEI Services.

  @retval EFI_SUCCESS       Successfully initiated system default restore.
  @retval EFI_DEVICE_ERROR  Failed to initiate default restore.

**/
EFI_STATUS
EFIAPI
RestoreDefault (
  IN       EFI_PEI_FILE_HANDLE     FileHandle,
  IN       CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS  Status;
  BOOLEAN     CmosLoss;

  CmosLoss = IsCmosPowerLossHappened ();
  DEBUG ((DEBUG_ERROR, "%a: ENTRY -- IsCmosPowerLossHappened()=%d\n", __func__, CmosLoss));
  DEBUG ((
    DEBUG_INFO,
    "%a: PcdFlashNvStorageVariableBase    = 0x%X\n",
    __func__,
    PcdGet32 (PcdFlashNvStorageVariableBase)
    ));

  if (!CmosLoss) {
    DEBUG ((DEBUG_ERROR, "%a: No CMOS power loss -- using primary store.\n", __func__));
    return EFI_SUCCESS;
  }

  InitializeCmosPowerCheck ();

  DEBUG ((DEBUG_INFO, "%a: CMOS power loss detected -- initiating restore defaults!\n", __func__));

  Status = NvStoreSwitchToDefault ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: NvStoreSwitchToDefault failed Status=%r.\n", __func__, Status));
    ASSERT_EFI_ERROR (Status);
    return EFI_DEVICE_ERROR;
  }

  DEBUG ((
    DEBUG_INFO,
    "%a: EXIT -- VarBase=0x%X\n",
    __func__,
    PcdGet32 (PcdFlashNvStorageVariableBase)
    ));
  return EFI_SUCCESS;
}
