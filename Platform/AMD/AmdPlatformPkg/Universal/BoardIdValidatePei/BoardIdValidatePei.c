/** @file
  Board ID validation PEIM.

  Verifies that the BIOS ROM was built for the physical board detected via the
  EEPROM Board ID.  The BoardId-to-board-name mapping is supplied by each
  platform through PcdPlatformBoardIdNameTable so this module is shared across
  all AMD server platform families.

  Copyright (C) 2026, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Ppi/AmdBoardIdPpi.h>
#include <Pcd/BoardIdValidatePcd.h>

/**
  Entry point of the BoardIdValidatePei PEIM driver.
  Checks that the BIOS ROM platform name matches the board detected at runtime.

  @param[in] FileHandle   Pointer to the firmware file system header.
  @param[in] PeiServices  Pointer to Pei Services.

  @retval EFI_SUCCESS        Check passed or mismatch was logged (non-fatal).
  @retval EFI_NOT_FOUND      AmdBoardIdPpi could not be located.
**/
EFI_STATUS
EFIAPI
BoardIdValidateEntry (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                    Status;
  AMD_EFI_PEI_AMDBOARDID_PPI    *AmdBoardIdPpi;
  AMD_EEPROM_ROOT_TABLE         *AmdEepromRootTable;
  UINT8                         BoardIdInEeprom;
  PLATFORM_BOARD_ID_NAME_TABLE  *Table;
  UINT8                         Index;
  CONST CHAR8                   *PlatformNameInFw;
  CONST CHAR8                   *DetectedBoardName;
  BOOLEAN                       BoardNameMatchesFw;

  DEBUG ((DEBUG_VERBOSE, "%a - ENTRY\n", __func__));

  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gAmdBoardIdPpiGuid,
                             0,
                             NULL,
                             (VOID **)&AmdBoardIdPpi
                             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a - Failed to locate AmdBoardIdPpi: %r\n", __func__, Status));
    return Status;
  }

  AmdEepromRootTable = AmdBoardIdPpi->AmdEepromRootTable;
  BoardIdInEeprom    = AmdEepromRootTable->PlatformId.BoardId;
  PlatformNameInFw   = (CONST CHAR8 *)PcdGetPtr (PcdPlatformBoardName);

  DEBUG ((DEBUG_INFO, "BIOS version : %a\n", (CONST CHAR8 *)PcdGetPtr (PcdBiosVersionString)));

  Table              = (PLATFORM_BOARD_ID_NAME_TABLE *)PcdGetPtr (PcdPlatformBoardIdNameTable);
  BoardNameMatchesFw = FALSE;
  DetectedBoardName  = "Unknown";

  for (Index = 0; Index < Table->NumEntry; Index++) {
    if (Table->BoardIdNameTable[Index].BoardId == BoardIdInEeprom) {
      DetectedBoardName = Table->BoardIdNameTable[Index].BoardName;
      if (AsciiStrnCmp (DetectedBoardName, PlatformNameInFw, MAX_BOARD_NAME_LENGTH - 1) == 0) {
        BoardNameMatchesFw = TRUE;
      }

      break;
    }
  }

  DEBUG ((DEBUG_INFO, "Platform name: %a\n", DetectedBoardName));

  if (!BoardNameMatchesFw) {
    if ((UINT32)BoardIdInEeprom != AmdEepromRootTable->ApcbInstance) {
      DEBUG ((DEBUG_ERROR, "%a - Board ID EEPROM is corrupt!\n", __func__));
    } else {
      DEBUG ((
        DEBUG_WARN,
        "%a - !!! Warning !!! BIOS platform name (%a) does not match board ID 0x%02x (%a).\n",
        __func__,
        PlatformNameInFw,
        BoardIdInEeprom,
        DetectedBoardName
        ));
    }
  }

  DEBUG ((DEBUG_VERBOSE, "%a - EXIT (Status = %r)\n", __func__, Status));
  return Status;
}
