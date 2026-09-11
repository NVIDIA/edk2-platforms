/** @file
  NULL instance of RedfishPlatformCredentialLib

  (C) Copyright 2020 Hewlett Packard Enterprise Development LP<BR>
  Copyright (C) 2026, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <Protocol/EdkIIRedfishCredential.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

CHAR8  *BmcUserId       = "root";
CHAR8  *BmcUserPassword = "0penBmc";

/**
  Notification of Exit Boot Service.

  @param[in]  This    Pointer to EDKII_REDFISH_CREDENTIAL_PROTOCOL.
**/
VOID
EFIAPI
LibCredentialExitBootServicesNotify (
  IN  EDKII_REDFISH_CREDENTIAL_PROTOCOL  *This
  )
{
  return;
}

/**
  Notification of End of DXe.

  @param[in]  This    Pointer to EDKII_REDFISH_CREDENTIAL_PROTOCOL.
**/
VOID
EFIAPI
LibCredentialEndOfDxeNotify (
  IN  EDKII_REDFISH_CREDENTIAL_PROTOCOL  *This
  )
{
  return;
}

/**
  Retrieve platform's Redfish authentication information.

  This functions returns the Redfish authentication method together with the user Id and
  password.
  - For AuthMethodNone, the UserId and Password could be used for HTTP header authentication
    as defined by RFC7235.
  - For AuthMethodRedfishSession, the UserId and Password could be used for Redfish
    session login as defined by  Redfish API specification (DSP0266).

  Callers are responsible for and freeing the returned string storage.

  @param[in]   This                Pointer to EDKII_REDFISH_CREDENTIAL_PROTOCOL instance.
  @param[out]  AuthMethod          Type of Redfish authentication method.
  @param[out]  UserId              The pointer to store the returned UserId string.
  @param[out]  Password            The pointer to store the returned Password string.

  @retval EFI_SUCCESS              Get the authentication information successfully.
  @retval EFI_ACCESS_DENIED        SecureBoot is disabled after EndOfDxe.
  @retval EFI_INVALID_PARAMETER    This or AuthMethod or UserId or Password is NULL.
  @retval EFI_OUT_OF_RESOURCES     There are not enough memory resources.
  @retval EFI_UNSUPPORTED          Unsupported authentication method is found.

**/
EFI_STATUS
EFIAPI
LibCredentialGetAuthInfo (
  IN  EDKII_REDFISH_CREDENTIAL_PROTOCOL  *This,
  OUT EDKII_REDFISH_AUTH_METHOD          *AuthMethod,
  OUT CHAR8                              **UserId,
  OUT CHAR8                              **Password
  )
{
  *UserId = (CHAR8 *)AllocatePool (AsciiStrSize (BmcUserId));
  if (*UserId == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Out of memory resource for Redfish user ID.\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  AsciiStrCpyS (*UserId, AsciiStrSize (BmcUserId), BmcUserId);

  *Password = (CHAR8 *)AllocatePool (AsciiStrSize (BmcUserPassword));
  if (*Password == NULL) {
    FreePool (UserId);
    DEBUG ((DEBUG_ERROR, "%a: Out of memory resource for Redfish user password.\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  AsciiStrCpyS (*Password, AsciiStrSize (BmcUserPassword), BmcUserPassword);

  *AuthMethod = AuthMethodHttpBasic;
  return EFI_SUCCESS;
}

/**
  Notify the Redfish service provide to stop provide configuration service to this platform.

  This function should be called when the platform is about to leave the safe environment.
  It will notify the Redfish service provider to abort all logged in sessions, and prohibit
  further login with original auth info. GetAuthInfo() will return EFI_UNSUPPORTED once this
  function is returned.

  @param[in]   This                Pointer to EDKII_REDFISH_CREDENTIAL_PROTOCOL instance.
  @param[in]   ServiceStopType     Reason of stopping Redfish service.

  @retval EFI_SUCCESS              Service has been stopped successfully.
  @retval EFI_INVALID_PARAMETER    This is NULL or given the wrong ServiceStopType.
  @retval EFI_UNSUPPORTED          Not support to stop Redfish service.
  @retval Others                   Some error happened.

**/
EFI_STATUS
EFIAPI
LibStopRedfishService (
  IN     EDKII_REDFISH_CREDENTIAL_PROTOCOL           *This,
  IN     EDKII_REDFISH_CREDENTIAL_STOP_SERVICE_TYPE  ServiceStopType
  )
{
  return EFI_UNSUPPORTED;
}
