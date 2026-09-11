/** @file
  This driver provides example of HSTI IBV table.

  Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PlatformHstiDxe.h"

/**
  Create Hsti Ibv table.

**/
STATIC
VOID
CreateHstiIbvTable (
  VOID
  )
{
  ADAPTER_INFO_PLATFORM_SECURITY  *Hsti;
  UINTN                           HstiSize;
  UINT8                           *HstiData;
  UINT8                           FeatureBitField;

  //
  // Allocate Pool for hold ADAPTER_INFO_PLATFORM_SECURITY
  //
  HstiSize = sizeof (ADAPTER_INFO_PLATFORM_SECURITY) + 3 * HSTI_AMD_FEATUERS_SIZE_IN_BYTES + HSTI_AMD_ERROR_STRING_SIZE;
  Hsti     = AllocateZeroPool (HstiSize);
  if (Hsti == NULL) {
    DEBUG ((DEBUG_ERROR, "CreateHstiIbvTable: allocate memory failed\n"));
    return;
  }

  //
  // Initialize HSTI IBV table
  //
  Hsti->Version = PLATFORM_SECURITY_VERSION_VNEXTCS;
  Hsti->Role    = PLATFORM_SECURITY_ROLE_PLATFORM_IBV;
  StrCpyS (Hsti->ImplementationID, 256, HSTI_IBV_IMPLEMENT_ID);
  Hsti->SecurityFeaturesSize = HSTI_AMD_FEATUERS_SIZE_IN_BYTES;

  HstiData = (UINT8 *)(Hsti + 1);

  //
  // 1. SecurityFeaturesRequired
  // This field have to be set with IHV role, skip here
  //
  HstiData += HSTI_AMD_FEATUERS_SIZE_IN_BYTES;

  //
  // 2. SecurityFeaturesImplemented
  //
  HstiData[HSTI_AMD_FEATURE_BYTE_INDEX_0] = HSTI_AMD_CRYPTO_STRENGTH;

  HstiData[HSTI_AMD_FEATURE_BYTE_INDEX_1] = HSTI_AMD_FWCODE_PROTECT_PROTECT_SPI +
                                            HSTI_AMD_FWCODE_PROTECT_SIGNED_FW_CHECK;

  FeatureBitField = (UINT8)(HSTI_AMD_SECURE_FW_UPDATE_DFT_TESTKEY +
                            HSTI_AMD_SECURE_FW_UPDATE_CHECK_TESTKEY_IN_PRODUCTION +
                            HSTI_AMD_SECURE_FW_UPDATE_ROLLBACK_CHECK);

  HstiData[HSTI_AMD_FEATURE_BYTE_INDEX_2] = FeatureBitField;

  HstiData[HSTI_AMD_FEATURE_BYTE_INDEX_3] = HSTI_AMD_SECUREBOOT_BYPASS_INLINE_PROMPT_CHECK +
                                            HSTI_AMD_SECUREBOOT_BYPASS_MANUFACTURE_CHECK;

 #ifdef AMD_HSTI_CS_SYSTEM_SUPPORT
  HstiData[HSTI_AMD_FEATURE_BYTE_INDEX_5] = HSTI_AMD_CSM_DISABLE_IF_SECUREBOOT_EN +
                                            HSTI_AMD_CSM_DISABLE_ON_CS_SYSTEM;
 #else
  HstiData[HSTI_AMD_FEATURE_BYTE_INDEX_5] = HSTI_AMD_CSM_DISABLE_IF_SECUREBOOT_EN;
 #endif

  //
  // 3. SecurityFeaturesVerified
  // It will be filled in UpdateHstiIbvTable, skip here
  //
  HstiData += HSTI_AMD_FEATUERS_SIZE_IN_BYTES;

  //
  // Install HSTI table.
  //
  HstiLibSetTable ((VOID *)Hsti, HstiSize);

  return;
}

/**
  To Check Crypto Strength.

**/
STATIC
VOID
CheckCryptoStrength (
  VOID
  )
{
  //
  // Implement your check here
  //
  //  if (CheckFailed) {
  //    return;
  //  }
  //

  HstiLibSetFeaturesVerified (
    PLATFORM_SECURITY_ROLE_PLATFORM_IBV,
    HSTI_IBV_IMPLEMENT_ID,
    HSTI_AMD_FEATURE_BYTE_INDEX_0,
    HSTI_AMD_CRYPTO_STRENGTH
    );
}

/**
  To Check Csm Policy.

**/
STATIC
VOID
CheckCsmPolicy (
  VOID
  )
{
  UINT8  FeatureBitField;

  //
  // Implement your check here
  //
  //  if (CheckFailed) {
  //    return;
  //  }
  //

 #ifdef AMD_HSTI_CS_SYSTEM_SUPPORT
  FeatureBitField = HSTI_AMD_CSM_DISABLE_IF_SECUREBOOT_EN +
                    HSTI_AMD_CSM_DISABLE_ON_CS_SYSTEM;
 #else
  FeatureBitField = HSTI_AMD_CSM_DISABLE_IF_SECUREBOOT_EN;
 #endif

  HstiLibSetFeaturesVerified (
    PLATFORM_SECURITY_ROLE_PLATFORM_IBV,
    HSTI_IBV_IMPLEMENT_ID,
    HSTI_AMD_FEATURE_BYTE_INDEX_5,
    FeatureBitField
    );
}

/**
  To Check Firmware Code Protect.

**/
STATIC
VOID
CheckFirmwareCodeProtect (
  VOID
  )
{
  UINT8  FeatureBitField;

  //
  // Implement your check here
  //
  //  if (CheckFailed) {
  //    return;
  //  }
  //

  FeatureBitField = HSTI_AMD_FWCODE_PROTECT_PROTECT_SPI +
                    HSTI_AMD_FWCODE_PROTECT_SIGNED_FW_CHECK;

  HstiLibSetFeaturesVerified (
    PLATFORM_SECURITY_ROLE_PLATFORM_IBV,
    HSTI_IBV_IMPLEMENT_ID,
    HSTI_AMD_FEATURE_BYTE_INDEX_1,
    FeatureBitField
    );
}

/**
  To Check Secure Firmware Update.

**/
STATIC
VOID
CheckSecureFirmwareUpdate (
  VOID
  )
{
  UINT8  FeatureBitField;

  //
  // Implement your check here
  //
  //  if (CheckFailed) {
  //    return;
  //  }
  //

  FeatureBitField = HSTI_AMD_SECURE_FW_UPDATE_DFT_TESTKEY +
                    HSTI_AMD_SECURE_FW_UPDATE_CHECK_TESTKEY_IN_PRODUCTION +
                    HSTI_AMD_SECURE_FW_UPDATE_ROLLBACK_CHECK;

  HstiLibSetFeaturesVerified (
    PLATFORM_SECURITY_ROLE_PLATFORM_IBV,
    HSTI_IBV_IMPLEMENT_ID,
    HSTI_AMD_FEATURE_BYTE_INDEX_2,
    FeatureBitField
    );
}

/**
  To Check SecureBoot Bypass.

**/
STATIC
VOID
CheckSecureBootBypass (
  VOID
  )
{
  UINT8  FeatureBitField;

  //
  // Implement your check here
  //
  //  if (CheckFailed) {
  //    return;
  //  }
  //

  FeatureBitField = HSTI_AMD_SECUREBOOT_BYPASS_INLINE_PROMPT_CHECK +
                    HSTI_AMD_SECUREBOOT_BYPASS_MANUFACTURE_CHECK;

  HstiLibSetFeaturesVerified (
    PLATFORM_SECURITY_ROLE_PLATFORM_IBV,
    HSTI_IBV_IMPLEMENT_ID,
    HSTI_AMD_FEATURE_BYTE_INDEX_3,
    FeatureBitField
    );
}

/**
  Update Hsti Ibv Table.

**/
STATIC
VOID
UpdateHstiIbvTable (
  VOID
  )
{
  CheckCryptoStrength ();
  CheckFirmwareCodeProtect ();
  CheckSecureFirmwareUpdate ();
  CheckSecureBootBypass ();
  CheckCsmPolicy ();
}

/**
  Callback function for ready to boot event.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.

**/
STATIC
VOID
EFIAPI
PlatformHstiDxeReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  CreateHstiIbvTable ();

  UpdateHstiIbvTable ();

  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }
}

/**
  Driver entry point.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
PlatformHstiDxeEntryPoint (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             PlatformHstiDxeReadyToBoot,
             NULL,
             &Event
             );

  return Status;
}
