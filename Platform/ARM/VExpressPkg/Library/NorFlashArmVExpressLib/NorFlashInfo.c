/** @file

 Copyright (c) 2011-2024, Arm Ltd. All rights reserved.<BR>

 SPDX-License-Identifier: BSD-2-Clause-Patent

 **/

#include <Base.h>
#include <Library/NorFlashPlatformLib.h>
#include <ArmPlatform.h>

NOR_FLASH_DESCRIPTION mNorFlashDevices[] = {
  { // BootMon
    ARM_VE_SMB_NOR0_BASE,
    ARM_VE_SMB_NOR0_BASE,
    SIZE_256KB * 255,
    SIZE_256KB,
  },
  { // BootMon non-volatile storage
    ARM_VE_SMB_NOR0_BASE,
    ARM_VE_SMB_NOR0_BASE + SIZE_256KB * 255,
    SIZE_64KB * 4,
    SIZE_64KB,
  },
#ifndef ENABLE_UEFI_SECURE_VARIABLE
  { // UEFI
    ARM_VE_SMB_NOR1_BASE,
    ARM_VE_SMB_NOR1_BASE,
    SIZE_256KB * 255,
    SIZE_256KB,
  },
  { // UEFI Variable Services non-volatile storage
    ARM_VE_SMB_NOR1_BASE,
    ARM_VE_SMB_NOR1_BASE + SIZE_256KB * 255,
    SIZE_64KB * 3, //FIXME: Set 3 blocks because I did not succeed to copy 4 blocks into the ARM Versatile Express NOR Flash in the last NOR Flash. It should be 4 blocks
    SIZE_64KB,
  },
#endif
};

UINT32 mNorFlashCount = ARRAY_SIZE(mNorFlashDevices);
