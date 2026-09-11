/** @file
  PCI Legacy DXE Driver.

  Copyright (C) 2020 - 2024, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#include <Library/DxeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <PiDxe.h>

//
// 48-bit MMIO space (MB-aligned)
//
#define AMD_MMIO_CFG_MSR_ADDR   0xC0010058UL
#define AMD_MMIO_CFG_ADDR_MASK  0xFFFFFFF00000ULL

#pragma pack(1)
typedef union {
  struct {
    UINT32    Enable          : 1;    // [0]
    UINT32    Reserved1       : 1;    // [1]
    UINT32    BusRange        : 4;    // [5:2]
    UINT32    Reserved2       : 14;   // [19:6]
    UINT32    MmioCfgBaseAddr : 28;   // [47:20]
    UINT32    Reserved3       : 16;   // [63:48]
  } AsBits;

  UINT64    AsUint64;
} AMD_MMIO_CFG_MSR;
#pragma pack()
