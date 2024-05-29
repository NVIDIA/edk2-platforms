/** @file

 Copyright (c) 2011-2024, Arm Limited. All rights reserved.<BR>

 SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef NOR_FLASH_COMMON_H_
#define NOR_FLASH_COMMON_H_

#include <Base.h>
#include <PiDxe.h>

#include <Guid/EventGroup.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/FirmwareVolumeBlock.h>

#include <Library/DebugLib.h>
#include <Library/NorFlashDeviceLib.h>
#include <Library/NorFlashPlatformLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeLib.h>

#define NOR_FLASH_SIGNATURE  SIGNATURE_32('n', 'o', 'r', '0')
#define INSTANCE_FROM_FVB_THIS(a)     CR(a, NOR_FLASH_INSTANCE, FvbProtocol, NOR_FLASH_SIGNATURE)
#define INSTANCE_FROM_BLKIO_THIS(a)   CR(a, NOR_FLASH_INSTANCE, BlockIoProtocol, NOR_FLASH_SIGNATURE)
#define INSTANCE_FROM_DISKIO_THIS(a)  CR(a, NOR_FLASH_INSTANCE, DiskIoProtocol, NOR_FLASH_SIGNATURE)

//
// NorFlashDxe.c
//
EFI_STATUS
NorFlashCreateInstance (
  IN UINTN                HostRegisterBase,
  IN UINTN                NorFlashDeviceBase,
  IN UINTN                NorFlashRegionBase,
  IN UINTN                NorFlashSize,
  IN UINT32               Index,
  IN UINT32               BlockSize,
  IN BOOLEAN              SupportFvb,
  OUT NOR_FLASH_INSTANCE  **NorFlashInstance
  );

EFI_STATUS
EFIAPI
NorFlashFvbInitialize (
  IN NOR_FLASH_INSTANCE  *Instance
  );

VOID
EFIAPI
NorFlashVirtualNotifyEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.Reset
//
EFI_STATUS
EFIAPI
NorFlashBlockIoReset (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  );

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.ReadBlocks
//
EFI_STATUS
EFIAPI
NorFlashBlockIoReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSizeInBytes,
  OUT VOID                   *Buffer
  );

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.WriteBlocks
//
EFI_STATUS
EFIAPI
NorFlashBlockIoWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSizeInBytes,
  IN  VOID                   *Buffer
  );

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.FlushBlocks
//
EFI_STATUS
EFIAPI
NorFlashBlockIoFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  );

//
// DiskIO Protocol function EFI_DISK_IO_PROTOCOL.ReadDisk
//
EFI_STATUS
EFIAPI
NorFlashDiskIoReadDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                Offset,
  IN UINTN                 BufferSize,
  OUT VOID                 *Buffer
  );

//
// DiskIO Protocol function EFI_DISK_IO_PROTOCOL.WriteDisk
//
EFI_STATUS
EFIAPI
NorFlashDiskIoWriteDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                Offset,
  IN UINTN                 BufferSize,
  IN VOID                  *Buffer
  );

//
// NorFlashFvbDxe.c
//

EFI_STATUS
EFIAPI
FvbGetAttributes (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  OUT       EFI_FVB_ATTRIBUTES_2                 *Attributes
  );

EFI_STATUS
EFIAPI
FvbSetAttributes (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN OUT    EFI_FVB_ATTRIBUTES_2                 *Attributes
  );

EFI_STATUS
EFIAPI
FvbGetPhysicalAddress (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  OUT       EFI_PHYSICAL_ADDRESS                 *Address
  );

EFI_STATUS
EFIAPI
FvbGetBlockSize (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN        EFI_LBA                              Lba,
  OUT       UINTN                                *BlockSize,
  OUT       UINTN                                *NumberOfBlocks
  );

EFI_STATUS
EFIAPI
FvbRead (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN        EFI_LBA                              Lba,
  IN        UINTN                                Offset,
  IN OUT    UINTN                                *NumBytes,
  IN OUT    UINT8                                *Buffer
  );

EFI_STATUS
EFIAPI
FvbWrite (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN        EFI_LBA                              Lba,
  IN        UINTN                                Offset,
  IN OUT    UINTN                                *NumBytes,
  IN        UINT8                                *Buffer
  );

EFI_STATUS
EFIAPI
FvbEraseBlocks (
  IN CONST  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  ...
  );

EFI_STATUS
ValidateFvHeader (
  IN  NOR_FLASH_INSTANCE  *Instance
  );

EFI_STATUS
InitializeFvAndVariableStoreHeaders (
  IN NOR_FLASH_INSTANCE  *Instance
  );

VOID
EFIAPI
FvbVirtualNotifyEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

#endif /* NOR_FLASH_COMMON_H_ */
