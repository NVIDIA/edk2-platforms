/** @file
  Structure definitions for PcdPlatformBoardIdNameTable used by BoardIdValidatePei.

  Copyright (C) 2026, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#pragma once

/// Maximum length (including NUL terminator) of a board name string.
#define MAX_BOARD_NAME_LENGTH  16

/// Maximum number of BoardId-to-BoardName entries in the table.
#define MAX_BOARD_ID_ENTRIES  64

///
/// Maps a single BoardId byte (read from EEPROM) to its board name string.
///
typedef struct {
  UINT8    BoardId;
  CHAR8    BoardName[MAX_BOARD_NAME_LENGTH];
} PLATFORM_BOARD_ID_NAME_INFO;

///
/// PCD structure holding the complete platform board ID name table.
/// NumEntry indicates how many entries in BoardIdNameTable are valid.
///
typedef struct {
  UINT8                          NumEntry;
  PLATFORM_BOARD_ID_NAME_INFO    BoardIdNameTable[MAX_BOARD_ID_ENTRIES];
} PLATFORM_BOARD_ID_NAME_TABLE;
