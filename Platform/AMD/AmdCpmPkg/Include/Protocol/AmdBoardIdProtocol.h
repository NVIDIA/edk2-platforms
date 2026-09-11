/** @file
  AMD Board ID Protocol stub for edk2-platforms build.

  Copyright (C) 2015-2025 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once
#include <AmdBoardId.h>

typedef struct S_EFI_DXE_AMDBOARDID_PROTOCOL AMD_EFI_DXE_AMDBOARDID_PROTOCOL;

struct S_EFI_DXE_AMDBOARDID_PROTOCOL {
  UINTN                    Revision;
  AMD_EEPROM_ROOT_TABLE    *AmdEepromRootTable;
};

#define AMDBOARDID_PROTOCOL_REVISION  0x00

extern EFI_GUID  gAmdBoardIdProtocolGuid;
