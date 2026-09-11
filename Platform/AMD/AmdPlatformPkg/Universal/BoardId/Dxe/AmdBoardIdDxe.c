/** @file
  Board Identification DXE driver

  Copyright (C) 2016-2024 Advanced Micro Devices, Inc. All rights reserved.

  SPDX-License-Identifier BSD-2-Clause-Patent
**/

#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AmdBoardIdProtocol.h>
#include "AmdBoardIdDxe.h"

/**
  Entry point of the AMD BOARD ID DXE driver.
  Perform the configuration init, resource reservation, early post init
  and install all the supported protocol.

  @param[in]  ImageHandle   EFI Image Handle for the DXE driver
  @param[in]  SystemTable   pointer to the EFI system table

  @retval     EFI_SUCCESS   Module initialized successfully
  @retval     EFI_ERROR     Initialization failed (see error for more details)
**/
EFI_STATUS
EFIAPI
AmdBoardIdDxeInit (
  IN       EFI_HANDLE        ImageHandle,
  IN       EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                       Status;
  EFI_HANDLE                       Handle;
  AMD_EFI_DXE_AMDBOARDID_PROTOCOL  *AmdBoardIdProtocol;
  AMD_BOARDID_INFO_HOB             *AmdBoardIdInfoHob;
  AMD_EEPROM_ROOT_TABLE            *AmdEepromRootTable;

  //
  // Initialize the configuration structure and private data area
  //
  AmdBoardIdInfoHob = GetFirstGuidHob (&gAmdBoardIdHobGuid);
  if (AmdBoardIdInfoHob == NULL) {
    return EFI_NOT_FOUND;
  }

  AmdEepromRootTable = &(AmdBoardIdInfoHob->AmdEepromRootTable);

  DEBUG ((DEBUG_INFO, "Table Signature: %g\n", &AmdEepromRootTable->TableSignature));
  DEBUG ((
    DEBUG_INFO,
    "Platform Identification: Board ID 0x%02x, Revision ID 0x%02x\n",
    AmdEepromRootTable->PlatformId.BoardId,
    AmdEepromRootTable->PlatformId.RevisionId
    ));

  if (AmdEepromRootTable->MinorRevision >= ROOT_TABLE_MINOR_VERSION) {
    DEBUG ((DEBUG_INFO, "Socket Manufacturer ID: 0x%02x\n", AmdEepromRootTable->PlatformId.SocketManufacturerId));
  }

  DEBUG ((DEBUG_INFO, "APCB Instance Number: %0x\n", AmdEepromRootTable->ApcbInstance));
  DEBUG ((DEBUG_INFO, "Major Revision: %0x\n", AmdEepromRootTable->MajorRevision));
  DEBUG ((DEBUG_INFO, "Minor Revision: %0x\n", AmdEepromRootTable->MinorRevision));
  DEBUG ((DEBUG_INFO, "CRC32 Checksum: %0x\n", AmdEepromRootTable->Checksum));

  if (AmdEepromRootTable->MinorRevision >= ROOT_TABLE_MINOR_VERSION) {
    if ((AmdEepromRootTable->PlatformId.SocketManufacturerId == 0x00) ||
        (AmdEepromRootTable->PlatformId.SocketManufacturerId == 0xFF))
    {
      DEBUG ((DEBUG_WARN, "Warning!!! Invalid Socket Manufacturer ID is detected. \n"));
    }
  }

  Handle = ImageHandle;

  Status = gBS->AllocatePool (EfiBootServicesData, sizeof (AMD_EFI_DXE_AMDBOARDID_PROTOCOL), (VOID **)&AmdBoardIdProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    // clear instances content
    gBS->SetMem (AmdBoardIdProtocol, sizeof (AMD_EFI_DXE_AMDBOARDID_PROTOCOL), 0);
  }

  AmdBoardIdProtocol->Revision           = AMDBOARDID_PROTOCOL_REVISION;
  AmdBoardIdProtocol->AmdEepromRootTable = AmdEepromRootTable;

  // Publish BoardID service Protocol
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gAmdBoardIdProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  AmdBoardIdProtocol
                  );

  return (Status);
}
