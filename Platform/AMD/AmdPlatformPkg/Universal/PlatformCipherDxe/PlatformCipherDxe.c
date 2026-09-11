/** @file
  Platform cipher suites.

  This driver installs TLS cipher suites if it does not exist.

  Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Guid/HttpTlsCipherList.h>
#include <Protocol/Tls.h>
#include <IndustryStandard/Tls1.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_TLS_CIPHER  mTlsHttpsCipher[] = {
  TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
  TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
};

/**
  The entry point for platform redfish BIOS driver which installs the Redfish Resource
  Addendum protocol on its ImageHandle.

  @param[in]  ImageHandle        The image handle of the driver.
  @param[in]  SystemTable        The system table.

  @retval EFI_SUCCESS            Protocol install successfully.
  @retval Others                 Failed to install the protocol.

**/
EFI_STATUS
EFIAPI
PlatformCipherEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINTN       CipherListSize;
  UINTN       CipherIndex;
  CHAR16      *VariableName;

  //
  // Install TLS Cipher Suites for the platform
  //
  VariableName = (CHAR16 *)AllocateCopyPool (StrSize (EDKII_HTTP_TLS_CIPHER_LIST_VARIABLE), (CONST VOID *)EDKII_HTTP_TLS_CIPHER_LIST_VARIABLE);
  if (VariableName == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Allocate memory failed for VariableName.\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // check if HttpTlsCipherList variable exist or not.
  //
  Status = gRT->GetVariable (
                  VariableName,
                  &gEdkiiHttpTlsCipherListGuid,
                  NULL,
                  &CipherListSize,
                  NULL
                  );
  if (Status == EFI_NOT_FOUND) {
    //
    // Create TLS cipher suits.
    //
    Status = gRT->SetVariable (
                    VariableName,                        // VariableName
                    &gEdkiiHttpTlsCipherListGuid,        // VendorGuid
                    EFI_VARIABLE_RUNTIME_ACCESS,         // Attributes
                    sizeof (mTlsHttpsCipher),            // DataSize
                    (VOID *)mTlsHttpsCipher              // Data
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: failed to set %s variable: %r\n", __func__, EDKII_HTTP_TLS_CIPHER_LIST_VARIABLE, Status));
      return Status;
    }

    DEBUG ((
      DEBUG_INFO,
      "%a: %s created, total %d ciphers\n",
      __func__,
      EDKII_HTTP_TLS_CIPHER_LIST_VARIABLE,
      sizeof (mTlsHttpsCipher)/sizeof (EFI_TLS_CIPHER)
      ));
    for (CipherIndex = 0; CipherIndex < (sizeof (mTlsHttpsCipher)/sizeof (EFI_TLS_CIPHER)); CipherIndex++) {
      DEBUG ((DEBUG_INFO, "  Cipher %d: ", CipherIndex + 1));
      DEBUG ((DEBUG_INFO, "%02x%02x\n", mTlsHttpsCipher[CipherIndex].Data1, mTlsHttpsCipher[CipherIndex].Data2));
    }
  }

  if (VariableName != NULL) {
    FreePool (VariableName);
  }

  return Status;
}
