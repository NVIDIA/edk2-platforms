/** @file
*  NOR flash lib for ARM Neoverse N1 platform
*
*  Copyright (c) 2024, ARM Limited. All rights reserved.<BR>
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Library/DebugLib.h>
#include <Library/NorFlashPlatformLib.h>
#include <NeoverseN1Soc.h>
#include <PiDxe.h>

#define FW_ENV_REGION_BASE  FixedPcdGet32 (PcdFlashNvStorageVariableBase)
#define FW_ENV_REGION_SIZE  (FixedPcdGet32 (PcdFlashNvStorageVariableSize) + \
                            FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize) + \
                            FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize))

STATIC NOR_FLASH_DESCRIPTION  mNorFlashDevices[] = {
  {
    /// Environment variable region
    NEOVERSEN1SOC_SCP_QSPI_AHB_BASE,                    ///< device base
    FW_ENV_REGION_BASE,                                 ///< region base
    FW_ENV_REGION_SIZE,                                 ///< region size
    SIZE_4KB,                                           ///< block size
  },
};

/**
  Dummy implementation of NorFlashPlatformInitialization to
  comply with NorFlashPlatformLib structure.

  @retval        EFI_SUCCESS        Success.
**/
EFI_STATUS
NorFlashPlatformInitialization (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  Get NOR flash region info

  @param[out]    NorFlashDevices        NOR flash regions info.
  @param[out]    Count                  number of flash instance.

  @retval        EFI_SUCCESS            Success.
  @retval        EFI_INVALID_PARAMETER  The parameters specified are not valid.
  @retval        EFI_ACCESS_DENIED      Invalid variable region address.
**/
EFI_STATUS
NorFlashPlatformGetDevices (
  OUT NOR_FLASH_DESCRIPTION  **NorFlashDevices,
  OUT UINT32                 *Count
  )
{
  if ((NorFlashDevices == NULL) || (Count == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((NEOVERSEN1SOC_SCP_QSPI_AHB_BASE +
       NEOVERSEN1SOC_FIRMWARE_IMAGES_SZ) >=
      FW_ENV_REGION_BASE)
  {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashPlatformInitialization: Variable region overlapping with "
      "firmware region.\n"
      ));

    return EFI_ACCESS_DENIED;
  }

  *NorFlashDevices = mNorFlashDevices;
  *Count           = ARRAY_SIZE (mNorFlashDevices);
  return EFI_SUCCESS;
}
