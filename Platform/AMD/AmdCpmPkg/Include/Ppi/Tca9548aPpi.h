/** @file
  TCA9548A I2C Switch PPI stub for edk2-platforms build.

  Copyright (C) 2015-2025 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

typedef EFI_STATUS (EFIAPI *TCA9548A_SET_PPI)(
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN UINTN                   BusSelect,
  IN UINTN                   SlaveAddress,
  IN UINT8                   ControlByte
  );

typedef EFI_STATUS (EFIAPI *TCA9548A_GET_PPI)(
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN UINTN                   BusSelect,
  IN UINTN                   SlaveAddress,
  OUT UINT8                  *ControlByte
  );

typedef struct {
  UINTN               Revision;
  TCA9548A_SET_PPI    Set;
  TCA9548A_GET_PPI    Get;
} EFI_PEI_TCA9548A_PPI;

#define TCA9548A_PPI_REVISION  (0x00)

extern EFI_GUID  gTca9548aPpiGuid;
