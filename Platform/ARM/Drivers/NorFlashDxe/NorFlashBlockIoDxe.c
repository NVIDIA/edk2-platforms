/** @file  NorFlashBlockIoDxe.c

  Copyright (c) 2011-2013, ARM Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "NorFlashCommon.h"

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.Reset
//
EFI_STATUS
EFIAPI
NorFlashBlockIoReset (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  )
{
  NOR_FLASH_INSTANCE  *Instance;

  Instance = INSTANCE_FROM_BLKIO_THIS (This);

  DEBUG ((DEBUG_BLKIO, "NorFlashBlockIoReset(MediaId=0x%x)\n", This->Media->MediaId));

  return NorFlashReset (Instance);
}

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
  )
{
  NOR_FLASH_INSTANCE  *Instance;
  EFI_STATUS          Status;
  EFI_BLOCK_IO_MEDIA  *Media;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = INSTANCE_FROM_BLKIO_THIS (This);
  Media    = This->Media;

  DEBUG ((DEBUG_BLKIO, "NorFlashBlockIoReadBlocks(MediaId=0x%x, Lba=%ld, BufferSize=0x%x bytes (%d kB), BufferPtr @ 0x%08x)\n", MediaId, Lba, BufferSizeInBytes, BufferSizeInBytes, Buffer));

  if (!Media) {
    Status = EFI_INVALID_PARAMETER;
  } else if (!Media->MediaPresent) {
    Status = EFI_NO_MEDIA;
  } else if (Media->MediaId != MediaId) {
    Status = EFI_MEDIA_CHANGED;
  } else if ((Media->IoAlign > 2) && (((UINTN)Buffer & (Media->IoAlign - 1)) != 0)) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    Status = NorFlashReadBlocks (Instance, Lba, BufferSizeInBytes, Buffer);
  }

  return Status;
}

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
  )
{
  NOR_FLASH_INSTANCE  *Instance;
  EFI_STATUS          Status;

  Instance = INSTANCE_FROM_BLKIO_THIS (This);

  DEBUG ((DEBUG_BLKIO, "NorFlashBlockIoWriteBlocks(MediaId=0x%x, Lba=%ld, BufferSize=0x%x bytes, BufferPtr @ 0x%08x)\n", MediaId, Lba, BufferSizeInBytes, Buffer));

  if ( !This->Media->MediaPresent ) {
    Status = EFI_NO_MEDIA;
  } else if ( This->Media->MediaId != MediaId ) {
    Status = EFI_MEDIA_CHANGED;
  } else if ( This->Media->ReadOnly ) {
    Status = EFI_WRITE_PROTECTED;
  } else {
    Status = NorFlashWriteBlocks (Instance, Lba, BufferSizeInBytes, Buffer);
  }

  return Status;
}

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.FlushBlocks
//
EFI_STATUS
EFIAPI
NorFlashBlockIoFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  // No Flush required for the NOR Flash driver
  // because cache operations are not permitted.

  DEBUG ((DEBUG_BLKIO, "NorFlashBlockIoFlushBlocks: Function NOT IMPLEMENTED (not required).\n"));

  // Nothing to do so just return without error
  return EFI_SUCCESS;
}

/*
  Although DiskIoDxe will automatically install the DiskIO protocol whenever
  we install the BlockIO protocol, its implementation is sub-optimal as it reads
  and writes entire blocks using the BlockIO protocol. In fact we can access
  NOR flash with a finer granularity than that, so we can improve performance
  by directly producing the DiskIO protocol.
*/

/**
  Read BufferSize bytes from Offset into Buffer.

  @param  This                  Protocol instance pointer.
  @param  MediaId               Id of the media, changes every time the media is replaced.
  @param  Offset                The starting byte offset to read from
  @param  BufferSize            Size of Buffer
  @param  Buffer                Buffer containing read data

  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the read.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHANGED     The MediaId does not match the current device.
  @retval EFI_INVALID_PARAMETER The read request contains device addresses that are not
                                valid for the device.

**/
EFI_STATUS
EFIAPI
NorFlashDiskIoReadDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                DiskOffset,
  IN UINTN                 BufferSize,
  OUT VOID                 *Buffer
  )
{
  NOR_FLASH_INSTANCE  *Instance;
  UINT32              BlockSize;
  UINT32              BlockOffset;
  EFI_LBA             Lba;

  Instance = INSTANCE_FROM_DISKIO_THIS (This);

  if (MediaId != Instance->Media.MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  BlockSize = Instance->Media.BlockSize;
  Lba       = (EFI_LBA)DivU64x32Remainder (DiskOffset, BlockSize, &BlockOffset);

  return NorFlashRead (Instance, Lba, BlockOffset, BufferSize, Buffer);
}

/**
  Writes a specified number of bytes to a device.

  @param  This       Indicates a pointer to the calling context.
  @param  MediaId    ID of the medium to be written.
  @param  Offset     The starting byte offset on the logical block I/O device to write.
  @param  BufferSize The size in bytes of Buffer. The number of bytes to write to the device.
  @param  Buffer     A pointer to the buffer containing the data to be written.

  @retval EFI_SUCCESS           The data was written correctly to the device.
  @retval EFI_WRITE_PROTECTED   The device can not be written to.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the write.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHANGED     The MediaId does not match the current device.
  @retval EFI_INVALID_PARAMETER The write request contains device addresses that are not
                                 valid for the device.

**/
EFI_STATUS
EFIAPI
NorFlashDiskIoWriteDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                DiskOffset,
  IN UINTN                 BufferSize,
  IN VOID                  *Buffer
  )
{
  NOR_FLASH_INSTANCE  *Instance;
  UINT32              BlockSize;
  UINT32              BlockOffset;
  EFI_LBA             Lba;
  UINTN               RemainingBytes;
  UINTN               WriteSize;
  EFI_STATUS          Status;

  Instance = INSTANCE_FROM_DISKIO_THIS (This);

  if (MediaId != Instance->Media.MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  BlockSize = Instance->Media.BlockSize;
  Lba       = (EFI_LBA)DivU64x32Remainder (DiskOffset, BlockSize, &BlockOffset);

  RemainingBytes = BufferSize;

  // Write either all the remaining bytes, or the number of bytes that bring
  // us up to a block boundary, whichever is less.
  // (DiskOffset | (BlockSize - 1)) + 1) rounds DiskOffset up to the next
  // block boundary (even if it is already on one).
  WriteSize = MIN (RemainingBytes, ((DiskOffset | (BlockSize - 1)) + 1) - DiskOffset);

  do {
    if (WriteSize == BlockSize) {
      // Write a full block
      Status = NorFlashWriteFullBlock (Instance, Lba, Buffer, BlockSize / sizeof (UINT32));
    } else {
      // Write a partial block
      Status = NorFlashWriteSingleBlock (Instance, Lba, BlockOffset, &WriteSize, Buffer);
    }

    if (EFI_ERROR (Status)) {
      return Status;
    }

    // Now continue writing either all the remaining bytes or single blocks.
    RemainingBytes -= WriteSize;
    Buffer          = (UINT8 *)Buffer + WriteSize;
    Lba++;
    BlockOffset = 0;
    WriteSize   = MIN (RemainingBytes, BlockSize);
  } while (RemainingBytes);

  return Status;
}
