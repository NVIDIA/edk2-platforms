/** @file
  M24LC128 EEPROM PPI stub for edk2-platforms build.

  Copyright (C) 2015-2025 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

typedef EFI_STATUS (EFIAPI *M24LC128_WRITE_PPI)(
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN UINTN                   BusSelect,
  IN UINTN                   SlaveAddress,
  IN UINT16                  Offset,
  IN UINT32                  Length,
  IN UINT8                   *Data
  );

typedef EFI_STATUS (EFIAPI *M24LC128_READ_PPI)(
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN UINTN                   BusSelect,
  IN UINTN                   SlaveAddress,
  IN UINT16                  Offset,
  IN UINT32                  Length,
  OUT UINT8                  *Data
  );

typedef struct {
  UINTN                 Revision;
  M24LC128_WRITE_PPI    Write;
  M24LC128_READ_PPI     Read;
} EFI_PEI_M24LC128_PPI;

#define M24LC128_PPI_REVISION  0x00

extern EFI_GUID  gM24Lc128PpiGuid;
