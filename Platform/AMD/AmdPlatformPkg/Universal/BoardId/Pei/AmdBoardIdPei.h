/** @file

  Board Identification PEIM header

  Copyright (C) 2015-2026 Advanced Micro Devices, Inc. All rights reserved.

  SPDX-License-Identifier BSD-2-Clause-Patent
**/

#pragma once

#ifndef ACPIMMIO8
  #ifdef ACPIMMIO8_SHADOW_FN
extern UINT8 *
gAcpiMmioShadowAccess (
  UINTN  Addr
  );

#define ACPIMMIO8(x)  (*gAcpiMmioShadowAccess ((UINTN)(x)))
  #else
#define ACPIMMIO8(x)  (*(volatile UINT8*)(UINTN)(x))
  #endif
#endif
#define ACPI_MMIO_BASE   (0xFED80000ul)
#define IOMUX_BASE       (0xD00)
#define FCH_IOMUX_REG13  (0x13)
#define FCH_IOMUX_REG14  (0x14)

#define I2C_EEPROM_ADDRESS        0x53
#define I2C_EEPROM_ADDRESS_KENYA  0x51

#define I2C_BUS_NUMBER  0x2

/// Board ID info HOB
typedef struct {
  EFI_HOB_GUID_TYPE        EfiHobGuidType;        ///< GUID Hob type structure
  AMD_EEPROM_ROOT_TABLE    AmdEepromRootTable;    ///< AMD Eeprom Root Table structure
} AMD_BOARDID_INFO_HOB;
