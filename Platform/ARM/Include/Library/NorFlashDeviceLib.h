/** @file  NorFlashDeviceLib.h

  Copyright (c) 2011 - 2024, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef NOR_FLASH_DEVICE_LIB_H_
#define NOR_FLASH_DEVICE_LIB_H_

#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/FirmwareVolumeBlock.h>

typedef struct _NOR_FLASH_INSTANCE NOR_FLASH_INSTANCE;

#define GET_NOR_BLOCK_ADDRESS(BaseAddr, Lba, LbaSize)  ( BaseAddr + (UINTN)((Lba) * LbaSize) )

/**
  This structure describes the device path for a NOR flash device instance.
**/
#pragma pack (1)
typedef struct {
  VENDOR_DEVICE_PATH          Vendor;  ///< Vendor specific device path node.
  UINT8                       Index;   ///< Index of a NOR flash device instance.
  EFI_DEVICE_PATH_PROTOCOL    End;     ///< End node of a hardware device path.
} NOR_FLASH_DEVICE_PATH;
#pragma pack ()

/**
  NOR Flash instance structure used to identify different flash
  or regions within a flash.
**/
struct _NOR_FLASH_INSTANCE {
  UINT32                                 Signature; ///< NOR Flash instance signature.
  EFI_HANDLE                             Handle;    ///< NOR Flash instance handle.

  UINTN                                  HostControllerBaseAddress; ///< NOR Flash host controller base address.
                                                                    ///< This field is optional if no host
                                                                    ///< controller is present.
  UINTN                                  DeviceBaseAddress;         ///< NOR Flash device base address.
  UINTN                                  RegionBaseAddress;         ///< NOR Flash region base address.
  UINTN                                  Size;                      ///< NOR Flash region size.
  EFI_LBA                                StartLba;                  ///< Region start LBA.

  EFI_BLOCK_IO_PROTOCOL                  BlockIoProtocol; ///< Instance's Block IO protocol handle.
  EFI_BLOCK_IO_MEDIA                     Media;           ///< Instance's  Media information.
  EFI_DISK_IO_PROTOCOL                   DiskIoProtocol;  ///< Instance's Disk IO protocol handle.

  EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL    FvbProtocol;   ///< Instance's FVB protocol handle.
  VOID                                   *ShadowBuffer; ///< Instance's shadow buffer.

  NOR_FLASH_DEVICE_PATH                  DevicePath;  ///< Instance's device path.
};

/**
  Write a full block to a given location.

  @param[in]    Instance               NOR flash Instance.
  @param[in]    Lba                    The logical block address in NOR flash.
  @param[in]    DataBuffer             The data to write into NOR flash location.
  @param[in]    BlockSizeInWords       The number of bytes to write.

  @retval       EFI_SUCCESS            Write is complete.
  @retval       EFI_DEVICE_ERROR       The device reported an error.
  @retval       EFI_WRITE_PROTECTED    The block is write protected.
  @retval       EFI_INVALID_PARAMETER  Invalid parameters passed.
  @retval       EFI_BUFFER_TOO_SMALL   Insufficient buffer size.
  @retval       EFI_BAD_BUFFER_SIZE    Invalid buffer size.
**/
EFI_STATUS
NorFlashWriteFullBlock (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN EFI_LBA             Lba,
  IN UINT32              *DataBuffer,
  IN UINT32              BlockSizeInWords
  );

/**
  This function unlocks and erases an entire NOR Flash block.

  @param[in]     Instance             NOR flash Instance.
  @param[in]     BlockAddress         Block address to unlock and erase.

  @retval        EFI_SUCCESS          Erase and unlock successfully completed.
  @retval        EFI_DEVICE_ERROR     The device reported an error.
  @retval        EFI_WRITE_PROTECTED  The block is write protected.
**/
EFI_STATUS
NorFlashUnlockAndEraseSingleBlock (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               BlockAddress
  );

/**
  Write a full or portion of a block.

  @param[in]       Instance              NOR flash Instance.
  @param[in]       Lba                   The starting logical block index to write to.
  @param[in]       Offset                Offset into the block at which to begin writing.
  @param[in, out]  NumBytes              The total size of the buffer.
  @param[in]       Buffer                The pointer to a caller-allocated buffer that
                                         contains the source for the write.

  @retval          EFI_SUCCESS           Write is complete.
  @retval          EFI_OUT_OF_RESOURCES  Invalid Buffer passed.
  @retval          EFI_BAD_BUFFER_SIZE   Buffer size not enough.
  @retval          EFI_DEVICE_ERROR      The device reported an error.
  @retval          EFI_ACCESS_DENIED     Device is in write disabled mode.
**/
EFI_STATUS
NorFlashWriteSingleBlock (
  IN        NOR_FLASH_INSTANCE  *Instance,
  IN        EFI_LBA             Lba,
  IN        UINTN               Offset,
  IN OUT    UINTN               *NumBytes,
  IN        UINT8               *Buffer
  );

/**
  Write multiple blocks.

  @param[in]    Instance               NOR flash Instance.
  @param[in]    Lba                    The starting logical block index.
  @param[in]    BufferSizeInBytes      The number of bytes to write.
  @param[in]    Buffer                 The pointer to a caller-allocated buffer that
                                       contains the source for the write.

  @retval       EFI_SUCCESS            Write is complete.
  @retval       EFI_INVALID_PARAMETER  Invalid parameters passed.
  @retval       EFI_BAD_BUFFER_SIZE    Invalid buffer size passed.
  @retval       EFI_DEVICE_ERROR       The device reported an error.
  @retval       EFI_WRITE_PROTECTED    The block is write protected.
  @retval       EFI_BUFFER_TOO_SMALL   Insufficient buffer size.
**/
EFI_STATUS
NorFlashWriteBlocks (
  IN  NOR_FLASH_INSTANCE  *Instance,
  IN  EFI_LBA             Lba,
  IN  UINTN               BufferSizeInBytes,
  IN  VOID                *Buffer
  );

/**
  Read multiple blocks.

  @param[in]     Instance               NOR flash Instance.
  @param[in]     Lba                    The starting logical block index to read from.
  @param[in]     BufferSizeInBytes      The number of bytes to read.
  @param[out]    Buffer                 The pointer to a caller-allocated buffer that
                                        should be copied with read data.

  @retval        EFI_SUCCESS            Read is complete.
  @retval        EFI_INVALID_PARAMETER  Invalid parameters passed.
  @retval        EFI_BAD_BUFFER_SIZE    Invalid buffer size passed.
**/
EFI_STATUS
NorFlashReadBlocks (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN EFI_LBA             Lba,
  IN UINTN               BufferSizeInBytes,
  OUT VOID               *Buffer
  );

/**
  Read from NOR flash.

  @param[in]     Instance               NOR flash Instance.
  @param[in]     Lba                    The starting logical block index to read from.
  @param[in]     Offset                 Offset into the block at which to begin reading.
  @param[in]     BufferSizeInBytes      The number of bytes to read.
  @param[out]    Buffer                 The pointer to a caller-allocated buffer that
                                        should be copied with read data.

  @retval        EFI_SUCCESS            Read is complete.
  @retval        EFI_INVALID_PARAMETER  Invalid parameters passed.
**/
EFI_STATUS
NorFlashRead (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN EFI_LBA             Lba,
  IN UINTN               Offset,
  IN UINTN               BufferSizeInBytes,
  OUT VOID               *Buffer
  );

/**
  NOR Flash Reset.

  @param[in]    Instance          NOR flash instance.

  @retval       EFI_SUCCESS       Return success on successful reset.
  @retval       EFI_DEVICE_ERROR  The device reported an error.
**/
EFI_STATUS
NorFlashReset (
  IN  NOR_FLASH_INSTANCE  *Instance
  );

/**
  The following function presumes that the block has already been unlocked.

  @param[in]    Instance            NOR flash instance.
  @param[in]    BlockAddress        Block address to erase.

  @retval       EFI_SUCCESS         Request is executed successfully.
  @retval       EFI_DEVICE_ERROR    The device reported an error.
  @retval       EFI_WRITE_PROTECTED The block is write protected.
 **/
EFI_STATUS
NorFlashEraseSingleBlock (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               BlockAddress
  );

/**
  Lock all pending read/write to NOR flash device.

  @param[in]     OriginalTPL     Pointer to NOR flash device Original TPL.
**/
VOID
EFIAPI
NorFlashLock (
  IN EFI_TPL  *OriginalTPL
  );

/**
  Unlock all pending read/write to NOR flash device.

  @param[in]     OriginalTPL     NOR flash device Original TPL.
**/
VOID
EFIAPI
NorFlashUnlock (
  IN EFI_TPL  OriginalTPL
  );

/**
  Read JEDEC ID of NOR flash device.

  @param[in]     Instance               NOR flash Instance of variable store region.
  @param[out]    JedecId                JEDEC ID of NOR flash device.
                                        Maximum length of JedecId can be upto 6 bytes
  @retval        EFI_SUCCESS            The write is completed.
  @retval        EFI_UNSUPPORTED        JEDEC ID retrieval not implemented.
  @retval        EFI_DEVICE_ERROR       Failed to fetch JEDEC ID.
  @retval        EFI_INVALID_PARAMETER  Invalid parameters passed.
**/
EFI_STATUS
NorFlashReadId (
  IN  NOR_FLASH_INSTANCE  *Instance,
  OUT UINT8               *JedecId  // Maximum length of JedecId can be upto 6 bytes.
  );

#endif /* NOR_FLASH_DEVICE_LIB_H_ */
