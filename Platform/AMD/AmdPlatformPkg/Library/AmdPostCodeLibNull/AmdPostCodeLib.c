/** @file
  NULL instance of AMD Post Code Library

 *Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <AMD.h>

/**
 * Output Postcode to I/O Port.
 *
 * @param[in] AccessWidth   Access width
 * @param[in] Value         Pointer to data
 * @return VOID
 */
VOID
LibAmdPostCode (
  IN       ACCESS_WIDTH  AccessWidth,
  IN       VOID          *Value
  )
{
}
