/** @file

  Board Identification DXE driver header

  Copyright (C) 2015-2024 Advanced Micro Devices, Inc. All rights reserved.

  SPDX-License-Identifier BSD-2-Clause-Patent
**/

#pragma once

#define ROOT_TABLE_MINOR_VERSION  9

/// Board ID info HOB
typedef struct {
  EFI_HOB_GUID_TYPE        EfiHobGuidType;        ///< GUID Hob type structure
  AMD_EEPROM_ROOT_TABLE    AmdEepromRootTable;    ///< AMD Eeprom Root Table structure
} AMD_BOARDID_INFO_HOB;

//
// Functions Prototypes
//

EFI_STATUS
EFIAPI
AmdBoardIdDxeInit (
  IN       EFI_HANDLE        ImageHandle,
  IN       EFI_SYSTEM_TABLE  *SystemTable
  );
