/** @file
  Board Identification PEIM

  Copyright (C) 2016-2026 Advanced Micro Devices, Inc. All rights reserved.

  SPDX-License-Identifier BSD-2-Clause-Patent
**/

#include <AmdCpmPei.h>
#include <Library/DebugLib.h>
#include <Ppi/AmdBoardIdPpi.h>
#include <Ppi/M24Lc128Ppi.h>
#include "AmdBoardIdPei.h"
#include <AmdSoc.h>
#include <Ppi/Tca9548aPpi.h>
#include <Ppi/M24Lc256Ppi.h>

/**
  Entry point of the AMD BoardID PEIM driver

  This function registers the function to Get BoardId information.

  @param[in]     FileHandle     Pointer to the firmware file system header
  @param[in]     PeiServices    Pointer to Pei Services

  @retval        EFI_SUCCESS    Module initialized successfully
  @retval        EFI_ERROR      Initialization failed (see error for more details)
**/
EFI_STATUS
EFIAPI
AmdBoardIdPeiEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                  Status;
  EFI_PEI_M24LC128_PPI        *M24Lc128Ppi;
  AMD_EEPROM_ROOT_TABLE       *BoardIdBuffer;
  AMD_EFI_PEI_AMDBOARDID_PPI  *AmdBoardIdPpi;
  EFI_PEI_PPI_DESCRIPTOR      *PpiListAmdBoardId;
  AMD_BOARDID_INFO_HOB        *BoardIdInfoHob;
  UINT32                      RegEax;
  UINT64                      SocFamilyID;
  EFI_PEI_TCA9548A_PPI        *Tca9548aPpi;
  EFI_PEI_M24LC256_PPI        *M24Lc256Ppi;
  UINT8                       Data8;
  UINT8                       IoMux19;
  UINT8                       IoMux20;
  UINT8                       I2cMuxAddr;
  BOOLEAN                     RestoreIoMux;

  //
  // Create Board ID info HOB
  //
  Status = (*PeiServices)->CreateHob (
                             PeiServices,
                             EFI_HOB_TYPE_GUID_EXTENSION,
                             sizeof (AMD_BOARDID_INFO_HOB),
                             (VOID **)&BoardIdInfoHob
                             );

  ASSERT_EFI_ERROR (Status);

  CopyMem (&BoardIdInfoHob->EfiHobGuidType.Name, &gAmdBoardIdHobGuid, sizeof (EFI_GUID));

  BoardIdBuffer = &(BoardIdInfoHob->AmdEepromRootTable);

  RegEax = 0;
  AsmCpuid (0x80000001, &RegEax, NULL, NULL, NULL);
  SocFamilyID  = (RegEax & RAW_FAMILY_ID_MASK);
  RestoreIoMux = FALSE;
  switch (SocFamilyID) {
    case F1A_WH_RAW_ID:
    case F1A_WH_SP7C_RAW_ID:
    case F1A_WH_SP8D_RAW_ID:
    case F1A_WH_SP8C_RAW_ID:
    case F1A_WHLP_SB1_RAW_ID:
    case F1A_WHLP_SB1C_RAW_ID:

      Status = (*PeiServices)->LocatePpi (
                                 PeiServices,
                                 &gTca9548aPpiGuid,
                                 0,
                                 NULL,
                                 (VOID **)&Tca9548aPpi
                                 );
      if (EFI_ERROR (Status)) {
        break;
      }

      Status = (*PeiServices)->LocatePpi (
                                 PeiServices,
                                 &gM24Lc256PpiGuid,
                                 0,
                                 NULL,
                                 (VOID **)&M24Lc256Ppi
                                 );
      if (EFI_ERROR (Status)) {
        break;
      }

      IoMux19 = ACPIMMIO8 (ACPI_MMIO_BASE + IOMUX_BASE + FCH_IOMUX_REG13); // Save IOMUX19_GPIO
      IoMux20 = ACPIMMIO8 (ACPI_MMIO_BASE + IOMUX_BASE + FCH_IOMUX_REG14); // Save IOMUX20_GPIO

      // IOMUX19_GPIO - I2C5_SCL/BMC_SCL/SMBUS1_SCL/AGPIO19: Switch to Function 0
      ACPIMMIO8 (ACPI_MMIO_BASE + IOMUX_BASE + FCH_IOMUX_REG13) = 0;

      // IOMUX20_GPIO - I2C5_SDA/BMC_SDA/SMBUS1_SDA/AGPIO20: Switch to Function 0
      ACPIMMIO8 (ACPI_MMIO_BASE + IOMUX_BASE + FCH_IOMUX_REG14) = 0;
      RestoreIoMux = TRUE;

      // Save the Control Register of the I2C mux
      Data8      = 0;
      I2cMuxAddr = 0x70;

      Status = Tca9548aPpi->Get (PeiServices, I2C_BUS_NUMBER, I2cMuxAddr, &Data8); // I2C 7-bit Address = 0x70h
      if (EFI_ERROR (Status)) {
        Status = Tca9548aPpi->Get (PeiServices, I2C_BUS_NUMBER, 0x72, &Data8); // I2C 7-bit Address = 0x72h
        if (EFI_ERROR (Status)) {
          break;
        }

        I2cMuxAddr = 0x72;
      }

      // Steer the mux to the I2C ID EEPROM
      Status = Tca9548aPpi->Set (PeiServices, I2C_BUS_NUMBER, I2cMuxAddr, BIT0); // The ID EEPROM is on channel zero.

      // Read Board ID from the I2C ID EEPROM
      // I2C 7-bit Address = 0x50h
      Status = M24Lc256Ppi->Read (
                              PeiServices,
                              I2C_BUS_NUMBER,
                              I2C_EEPROM_ADDRESS,
                              0,
                              sizeof (AMD_EEPROM_ROOT_TABLE),
                              (UINT8 *)BoardIdBuffer
                              );

      if (EFI_ERROR (Status)) {
        // The ID EEPROM is on channel zero. For Kenya
        Status = Tca9548aPpi->Set (PeiServices, I2C_BUS_NUMBER, I2cMuxAddr, BIT0);
        Status = M24Lc256Ppi->Read (
                                PeiServices,
                                I2C_BUS_NUMBER,
                                I2C_EEPROM_ADDRESS_KENYA,
                                0,
                                sizeof (AMD_EEPROM_ROOT_TABLE),
                                (UINT8 *)BoardIdBuffer
                                );
        DEBUG ((
          DEBUG_ERROR,
          "[AmdBoardIdPei] BoardId: 0x%02x, RevisionId: 0x%02x\n",
          BoardIdBuffer->PlatformId.BoardId,
          BoardIdBuffer->PlatformId.RevisionId
          ));
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_ERROR, "[AmdBoardIdPei] Failed to read Board ID: %r\n", Status));
          break;
        }
      } else {
        DEBUG ((
          DEBUG_ERROR,
          "[AmdBoardIdPei] BoardId: 0x%02x, RevisionId: 0x%02x\n",
          BoardIdBuffer->PlatformId.BoardId,
          BoardIdBuffer->PlatformId.RevisionId
          ));
      }

      // Restore the Control Register of the I2C mux
      // I2C 7-bit Address = 0x70h or 0x72
      Status = Tca9548aPpi->Set (PeiServices, I2C_BUS_NUMBER, I2cMuxAddr, Data8);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "[AmdBoardIdPei] Failed to restore the Control Register of the I2C mux: %r\n", Status));
      }

      break;
    default:
      Status = (*PeiServices)->LocatePpi (
                                 PeiServices,
                                 &gM24Lc128PpiGuid,
                                 0,
                                 NULL,
                                 (VOID **)&M24Lc128Ppi
                                 );

      if (!EFI_ERROR (Status)) {
        Status = M24Lc128Ppi->Read (
                                PeiServices,
                                I2C_BUS_NUMBER,
                                I2C_EEPROM_ADDRESS,
                                0,
                                sizeof (AMD_EEPROM_ROOT_TABLE),
                                (UINT8 *)BoardIdBuffer
                                );
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_ERROR, "[AmdBoardIdPei] Failed to read Board ID: %r\n", Status));
        } else {
          DEBUG ((
            DEBUG_ERROR,
            "[AmdBoardIdPei] BoardId: 0x%02x, RevisionId: 0x%02x\n",
            BoardIdBuffer->PlatformId.BoardId,
            BoardIdBuffer->PlatformId.RevisionId
            ));
        }
      }

      break;
  }

  if (RestoreIoMux) {
    ACPIMMIO8 (ACPI_MMIO_BASE + IOMUX_BASE + FCH_IOMUX_REG13) = IoMux19; // Restore IOMUX19_GPIO
    ACPIMMIO8 (ACPI_MMIO_BASE + IOMUX_BASE + FCH_IOMUX_REG14) = IoMux20; // Restore IOMUX20_GPIO
  }

  //
  // Create the PPIs
  //
  Status = (*PeiServices)->AllocatePool (
                             PeiServices,
                             sizeof (AMD_EFI_PEI_AMDBOARDID_PPI),
                             (VOID **)&AmdBoardIdPpi
                             );
  ASSERT_EFI_ERROR (Status);

  AmdBoardIdPpi->Revision           = AMDBOARDID_PPI_REVISION;
  AmdBoardIdPpi->AmdEepromRootTable = BoardIdBuffer;

  //
  // Install the PPIs
  //
  Status = (*PeiServices)->AllocatePool (
                             PeiServices,
                             sizeof (EFI_PEI_PPI_DESCRIPTOR),
                             (VOID **)&PpiListAmdBoardId
                             );

  ASSERT_EFI_ERROR (Status);

  //
  // Create the PPI descriptor
  //
  PpiListAmdBoardId->Guid  = &gAmdBoardIdPpiGuid;
  PpiListAmdBoardId->Ppi   = AmdBoardIdPpi;
  PpiListAmdBoardId->Flags = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);

  //
  // Publish the PPI
  //
  Status = (*PeiServices)->InstallPpi (
                             PeiServices,
                             PpiListAmdBoardId
                             );
  return (Status);
}
