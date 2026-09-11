/** @file
  Define PostCode Interface for AMD Platform.

  Copyright (C) 2023 - 2024, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#include <AMD.h>

/**
  @brief Output Postcode to IO Port PcdIdsDebugPort specified, output to NBCFG_SCRATCH_0 in emulation case

  @param[in] AccessWidth   Access width
  @param[in] Value         Pointer to data

  @retval VOID

**/
VOID
LibAmdPostCode (
  IN       ACCESS_WIDTH  AccessWidth,
  IN       VOID          *Value
  );
