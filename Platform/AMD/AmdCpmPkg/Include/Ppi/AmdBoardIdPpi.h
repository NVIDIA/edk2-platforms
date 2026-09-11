/** @file
  AMD Board ID PPI stub for edk2-platforms build.

  Copyright (C) 2015-2025 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#include <AmdBoardId.h>

typedef struct {
  UINTN                    Revision;
  AMD_EEPROM_ROOT_TABLE    *AmdEepromRootTable;
} AMD_EFI_PEI_AMDBOARDID_PPI;

#define AMDBOARDID_PPI_REVISION  0x00

extern EFI_GUID  gAmdBoardIdPpiGuid;
