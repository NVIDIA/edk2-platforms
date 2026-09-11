/** @file
  CMOS lib for restore default driver.

  Copyright (C) 2023 - 2024, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include "Cmos.h"

/**
  Reads the 8-bits of CMOS data at the location specified by Index.
  The 8-bit read value is returned.

  @param[in]  Index  The CMOS location to read.

  @retval UINT8 The value read.

**/
UINT8
EFIAPI
CmosRead8 (
  IN      UINTN  Index
  )
{
  IoWrite8 (0x70, (UINT8)Index);
  return IoRead8 (0x71);
}

/**
  Writes 8-bits of CMOS data to the location specified by Index
  with the value specified by Value and returns Value.

  @param[in]  Index  The CMOS location to write.
  @param[in]  Value  The value to write to CMOS.

  @retval Value The value written to CMOS.

**/
UINT8
EFIAPI
CmosWrite8 (
  IN      UINTN  Index,
  IN      UINT8  Value
  )
{
  IoWrite8 (0x70, (UINT8)Index);
  IoWrite8 (0x71, Value);
  return Value;
}

/**
  Check the value of CMOS offset 0x36.
  If the value = 0xaa, there is no CMOS power loss happened.
  Otherwise, a CMOS power loss event happened.

  @retval TRUE  A CMOS power loss event happened.
  @retval FALSE There is no CMOS power loss happened.

**/
BOOLEAN
EFIAPI
IsCmosPowerLossHappened (
  VOID
  )
{
  if (0xaa == CmosRead8 (CmosPowerLossIndication)) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Writes value 0xaa into CMOS offset 0x36 as an indication for CMOS power status.

  @retval EFI_SUCCESS Write the value into CMOS with specific offset.

**/
EFI_STATUS
EFIAPI
InitializeCmosPowerCheck (
  VOID
  )
{
  CmosWrite8 (CmosPowerLossIndication, 0xaa);
  return EFI_SUCCESS;
}
