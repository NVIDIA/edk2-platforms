/** @file
  AMD Board ID definitions stub for edk2-platforms build.

  Copyright (C) 2015-2025 Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#pragma once

/// Platform Identification
typedef struct {
  UINT8    BoardId;               ///< Platform Identification
  UINT8    IoConfigId;            ///< Platform Identification
  UINT8    RevisionId;            ///< Motherboard revision
  UINT8    SocketManufacturerId;  ///< Socket manufacturer
} AMD_PLATFORM_ID;

/// Eeprom Root table
typedef struct {
  EFI_GUID           TableSignature;  ///< EEPROM Table Signature
  AMD_PLATFORM_ID    PlatformId;      ///< Platform Identification
  UINT32             ApcbInstance;    ///< APCB Instance Number in the BIOS Directory
  UINT16             MajorRevision;   ///< EEPROM Major Revision
  UINT16             MinorRevision;   ///< EEPROM Minor Revision
  UINT32             Checksum;        ///< Root Table Checksum
} AMD_EEPROM_ROOT_TABLE;

