/** @file
  CMOS lib for restore default driver.

  Copyright (C) 2023 - 2026, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#include <PiPei.h>
#include <Library/IoLib.h>

typedef enum {
  CmosRtcRegisters        = 0x0,    ///< 0x00~0x0F: Real Time Clock Information
  CmosIbmPcRegisters      = 0x10,   ///< 0x10~0x2D: The PC Standard Information
  CmosIbmPcCheckSum       = 0x2E,   ///< 0x2E~0x2F: Check sum of the PC registers
  CmosPowerLossIndication = 0x36    ///< 0x36     : Used for CMOS power loss indication
} AMD_COMMON_CMOS_TABLE;

UINT8
EFIAPI
CmosRead8 (
  IN UINTN  Index
  );

UINT8
EFIAPI
CmosWrite8 (
  IN UINTN  Index,
  IN UINT8  Value
  );

BOOLEAN
EFIAPI
IsCmosPowerLossHappened (
  VOID
  );

EFI_STATUS
EFIAPI
InitializeCmosPowerCheck (
  VOID
  );
