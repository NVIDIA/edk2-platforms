/** @file

  Copyright (C) 2015-2025 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

#define RAW_FAMILY_ID_MASK  (UINT32)(0x0FF00000ul | 0x000F0000ul | 0x00000F00ul)

#define F1A_BRH_A0_RAW_ID   0x00B00F00ul
#define F1A_BRH_B0_RAW_ID   0x00B00F10ul
#define F1A_BRH_B1_RAW_ID   0x00B00F11ul
#define F1A_BRHD_A0_RAW_ID  0x00B10F00ul
#define F1A_BRHD_B0_RAW_ID  0x00B10F10ul

#define F1A_WH_RAW_ID           0x00B50F00ul
#define F1A_WH_SP7_RAW_ID       0x00B50F00ul
#define F1A_WH_SP7_RAW_MAX_ID   0x00B50F7Ful
#define F1A_WH_SP7_AB_RAW_ID    0x00B50F0Bul
#define F1A_WH_SP7_A0_RAW_ID    0x00B50F00ul
#define F1A_WH_SP7_B0_RAW_ID    0x00B50F10ul
#define F1A_WHLP_SB1_RAW_ID     0x00B50F80ul
#define F1A_WH_SP7C_RAW_ID      0x00BC0F00ul
#define F1A_WH_SP7C_RAW_MAX_ID  0x00BC0F7Ful
#define F1A_WHLP_SB1C_RAW_ID    0x00BC0F80ul
#define F1A_WH_SP8D_RAW_ID      0x00B90F00ul
#define F1A_WH_SP8C_RAW_ID      0x00BA0F00ul
