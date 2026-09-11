/** @file
  AMD CPM DXE Definitions stub for edk2-platforms build.

  Copyright (C) 2014-2025 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#include <AmdCpmBase.h>

#define AMD_CPM_TABLE_PROTOCOL_GUID \
  { 0x3724cf01, 0x00c2, 0x9762, { 0x11, 0xb3, 0x0e, 0xa8, 0xaa, 0x89, 0x72, 0x00 } }

extern EFI_GUID  gAmdCpmTableProtocolGuid;

typedef struct {
  UINTN                          Revision;
  AMD_CPM_MAIN_TABLE             *MainTablePtr;
  AMD_CPM_CHIP_ID                ChipId;
  AMD_CPM_COMMON_FUNCTION        CommonFunction;
  AMD_CPM_DXE_PUBLIC_FUNCTION    DxePublicFunction;
} AMD_CPM_TABLE_PROTOCOL;
