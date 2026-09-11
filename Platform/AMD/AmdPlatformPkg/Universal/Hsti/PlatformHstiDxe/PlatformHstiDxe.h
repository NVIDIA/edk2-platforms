/** @file
  This driver provides example of HSTI IBV table.

  Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#include <Library/UefiLib.h>
#include <IndustryStandard/Hsti.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/HstiLib.h>
#include <Protocol/AdapterInformation.h>

#include <Library/AmdBaseLib.h>
#include <Include/AmdHsti.h>

#define HSTI_IBV_IMPLEMENT_ID  L"AMD EDKII sample code"

#ifndef HSTI_AMD_SECUREBOOT_BYPASS_INLINE_PROMPT_CHECK
#define HSTI_AMD_SECUREBOOT_BYPASS_INLINE_PROMPT_CHECK  BIT0
#endif
#ifndef HSTI_AMD_SECUREBOOT_BYPASS_MANUFACTURE_CHECK
#define HSTI_AMD_SECUREBOOT_BYPASS_MANUFACTURE_CHECK  BIT1
#endif
