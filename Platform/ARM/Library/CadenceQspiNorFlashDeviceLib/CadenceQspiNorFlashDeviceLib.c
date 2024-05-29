/** @file

  Copyright (c) 2024 ARM Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/NorFlashDeviceLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>

#include "CadenceQspiNorFlashDeviceLib.h"

/**
  Converts milliseconds into number of ticks of the performance counter.

  @param[in] Milliseconds  Milliseconds to convert into ticks.

  @retval Milliseconds expressed as number of ticks.

**/
STATIC
UINT64
MilliSecondsToTicks (
  IN UINTN  Milliseconds
  )
{
  UINT64  NanoSecondsPerTick;
  UINT64  NanoSeconds;

  NanoSecondsPerTick = GetTimeInNanoSecond (1);
  NanoSeconds        = MultU64x32 (Milliseconds, 1000000);

  return DivU64x64Remainder (NanoSeconds, NanoSecondsPerTick, NULL);
}

/**
  Execute Flash cmd ctrl and Read Status.

  @param[in]      Instance         NOR flash Instance.
  @param[in]      Val              Value to be written to Flash cmd ctrl Register.

  @retval         EFI_SUCCESS      Request is executed successfully.
  @retval         EFI_TIMEOUT      Command execution timed out.

**/
STATIC
EFI_STATUS
CdnsQspiExecuteCommand (
  IN  NOR_FLASH_INSTANCE  *Instance,
  IN  UINT32              Val
  )
{
  // Set the command
  MmioWrite32 (
    Instance->HostControllerBaseAddress + CDNS_QSPI_FLASH_CMD_CTRL_REG_OFFSET,
    Val
    );
  // Execute the command
  MmioWrite32 (
    Instance->HostControllerBaseAddress + CDNS_QSPI_FLASH_CMD_CTRL_REG_OFFSET,
    Val | CDNS_QSPI_FLASH_CMD_CTRL_REG_EXECUTE
    );

  CONST UINT64  TickOut =
    GetPerformanceCounter () +
    MilliSecondsToTicks (CDNS_QSPI_FLASH_CMD_STATUS_POLL_TIMEOUT_MS);

  // Wait until command has been executed
  do {
    if (GetPerformanceCounter () > TickOut) {
      DEBUG ((
        DEBUG_ERROR,
        "CdnsQspiExecuteCommand: Timeout waiting for command execution.\n"
        ));
      return EFI_TIMEOUT;
    }
  } while ((MmioRead32 (
              Instance->HostControllerBaseAddress +
              CDNS_QSPI_FLASH_CMD_CTRL_REG_OFFSET
              ) &
            CDNS_QSPI_FLASH_CMD_CTRL_REG_STATUS_BIT) ==
           CDNS_QSPI_FLASH_CMD_CTRL_REG_STATUS_BIT
           );

  return EFI_SUCCESS;
}

/**
  Poll Status register for NOR flash erase/write completion.

  @param[in]      Instance           NOR flash Instance.

  @retval         EFI_SUCCESS        Request is executed successfully.
  @retval         EFI_TIMEOUT        Operation timed out.
  @retval         EFI_DEVICE_ERROR   Controller operation failed.

**/
STATIC
EFI_STATUS
NorFlashPollStatusRegister (
  IN NOR_FLASH_INSTANCE  *Instance
  )
{
  BOOLEAN  SRegDone;
  UINT32   val;

  val = (SPINOR_OP_RDSR << CDNS_QSPI_FLASH_CMD_CTRL_REG_OPCODE_BIT_POS) |
        (CDNS_QSPI_FLASH_CMD_CTRL_REG_READ_ENABLE <<
         CDNS_QSPI_FLASH_CMD_CTRL_REG_READEN_BIT_POS) |
        CDNS_QSPI_FLASH_CMD_CTRL_REG_NUM_DATA_BYTES (1) |
        (CDNS_QSPI_FLASH_CMD_CTRL_REG_DUMMY_8C <<
         CDNS_QSPI_FLASH_CMD_CTRL_REG_DUMMY_BIT_POS);

  CONST UINT64  TickOut =
    GetPerformanceCounter () +
    MilliSecondsToTicks (SPINOR_SR_WIP_POLL_TIMEOUT_MS);

  do {
    if (GetPerformanceCounter () > TickOut) {
      DEBUG ((
        DEBUG_ERROR,
        "NorFlashPollStatusRegister: Timeout waiting for erase/write.\n"
        ));
      return EFI_TIMEOUT;
    }

    if (EFI_ERROR (CdnsQspiExecuteCommand (Instance, val))) {
      return EFI_DEVICE_ERROR;
    }

    SRegDone =
      (MmioRead8 (
         Instance->HostControllerBaseAddress +
         CDNS_QSPI_FLASH_CMD_READ_DATA_REG_OFFSET
         )
       & SPINOR_SR_WIP
      ) == 0;
  } while (!SRegDone);

  return EFI_SUCCESS;
}

/**
  Check whether NOR flash operations are Locked.

  @param[in]     Instance         NOR flash Instance.
  @param[in]     BlockAddress     BlockAddress in NOR flash device.

  @retval        FALSE            If NOR flash is not locked.
**/
STATIC
BOOLEAN
NorFlashBlockIsLocked (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               BlockAddress
  )
{
  return FALSE;
}

/**
  Unlock NOR flash operations on given block.

  @param[in]      Instance         NOR flash instance.
  @param[in]      BlockAddress     BlockAddress in NOR flash device.

  @retval         EFI_SUCCESS      NOR flash operations unlocked.
**/
EFI_STATUS
NorFlashUnlockSingleBlock (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               BlockAddress
  )
{
  return EFI_SUCCESS;
}

/**
  Unlock NOR flash operations if it is necessary.

  @param[in]      Instance         NOR flash instance.
  @param[in]      BlockAddress     BlockAddress in NOR flash device.

  @retval         EFI_SUCCESS      Request is executed successfully.
**/
EFI_STATUS
NorFlashUnlockSingleBlockIfNecessary (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               BlockAddress
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  if (!NorFlashBlockIsLocked (Instance, BlockAddress)) {
    Status = NorFlashUnlockSingleBlock (Instance, BlockAddress);
  }

  return Status;
}

/**
  Enable write to NOR flash device.

  @param[in]      Instance         NOR flash instance.

  @retval         EFI_SUCCESS      Request is executed successfully.
  @retval         EFI_DEVICE_ERROR The device reported an error.
**/
STATIC
EFI_STATUS
NorFlashEnableWrite (
  IN  NOR_FLASH_INSTANCE  *Instance
  )
{
  UINT32  val;

  DEBUG ((DEBUG_INFO, "NorFlashEnableWrite()\n"));
  val = (SPINOR_OP_WREN << CDNS_QSPI_FLASH_CMD_CTRL_REG_OPCODE_BIT_POS);
  if (EFI_ERROR (CdnsQspiExecuteCommand (Instance, val))) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  The following function presumes that the block has already been unlocked.

  @param[in]      Instance           NOR flash instance.
  @param[in]      BlockAddress       Block address within the variable region.

  @retval       EFI_SUCCESS          Request is executed successfully.
  @retval       EFI_DEVICE_ERROR     The device reported an error.
 **/
EFI_STATUS
NorFlashEraseSingleBlock (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               BlockAddress
  )
{
  UINT32  DevConfigVal;
  UINT32  EraseOffset;

  EraseOffset = 0x0;

  DEBUG ((
    DEBUG_INFO,
    "NorFlashEraseSingleBlock(BlockAddress=0x%08x)\n",
    BlockAddress
    ));

  if (EFI_ERROR (NorFlashEnableWrite (Instance))) {
    return EFI_DEVICE_ERROR;
  }

  EraseOffset = BlockAddress - Instance->DeviceBaseAddress;

  MmioWrite32 (
    Instance->HostControllerBaseAddress + CDNS_QSPI_FLASH_CMD_ADDR_REG_OFFSET,
    EraseOffset
    );

  DevConfigVal = (SPINOR_OP_BE_4K << CDNS_QSPI_FLASH_CMD_CTRL_REG_OPCODE_BIT_POS) |
                 (CDNS_QSPI_FLASH_CMD_CTRL_REG_ADDR_ENABLE <<
                  CDNS_QSPI_FLASH_CMD_CTRL_REG_ADDR_BIT_POS) |
                 CDNS_QSPI_FLASH_CMD_CTRL_REG_NUM_ADDR_BYTES (3);

  if (EFI_ERROR (CdnsQspiExecuteCommand (Instance, DevConfigVal))) {
    return EFI_DEVICE_ERROR;
  }

  if (EFI_ERROR (NorFlashPollStatusRegister (Instance))) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  This function unlocks and erases an entire NOR Flash block.

  @param[in]     Instance       NOR flash Instance of variable store region.
  @param[in]     BlockAddress   Block address within the variable store region.

  @retval        EFI_SUCCESS          Erase and unlock successfully completed.
  @retval        EFI_DEVICE_ERROR     The device reported an error.
**/
EFI_STATUS
NorFlashUnlockAndEraseSingleBlock (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               BlockAddress
  )
{
  EFI_STATUS  Status;
  EFI_TPL     OriginalTPL;

  NorFlashLock (&OriginalTPL);

  // Unlock the block if we have to
  Status = NorFlashUnlockSingleBlockIfNecessary (Instance, BlockAddress);
  if (!EFI_ERROR (Status)) {
    Status = NorFlashEraseSingleBlock (Instance, BlockAddress);
    ASSERT_EFI_ERROR (Status);
  }

  NorFlashUnlock (OriginalTPL);

  return Status;
}

/**
  Write a single word to given location.

  @param[in]    Instance          NOR flash Instance of variable store region.
  @param[in]    WordAddress       The address in NOR flash to write given word.
  @param[in]    WriteData         The data to write into NOR flash location.

  @retval       EFI_SUCCESS       Write is complete.
  @retval       EFI_DEVICE_ERROR  The device reported an error.
**/
EFI_STATUS
NorFlashWriteSingleWord (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN UINTN               WordAddress,
  IN UINT32              WriteData
  )
{
  DEBUG ((
    DEBUG_INFO,
    "NorFlashWriteSingleWord(WordAddress=0x%08x, WriteData=0x%08x)\n",
    WordAddress,
    WriteData
    ));

  if (EFI_ERROR (NorFlashEnableWrite (Instance))) {
    return EFI_DEVICE_ERROR;
  }

  MmioWrite32 (WordAddress, WriteData);
  if (EFI_ERROR (NorFlashPollStatusRegister (Instance))) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Write a full block to given location.

  @param[in]    Instance           NOR flash Instance of variable store region.
  @param[in]    Lba                The logical block address in NOR flash.
  @param[in]    DataBuffer         The data to write into NOR flash location.
  @param[in]    BlockSizeInWords   The number of bytes to write.

  @retval       EFI_SUCCESS            Write is complete.
  @retval       EFI_DEVICE_ERROR       The device reported an error.
**/
EFI_STATUS
NorFlashWriteFullBlock (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN EFI_LBA             Lba,
  IN UINT32              *DataBuffer,
  IN UINT32              BlockSizeInWords
  )
{
  EFI_STATUS  Status;
  UINTN       WordAddress;
  UINT32      WordIndex;
  UINTN       BlockAddress;
  EFI_TPL     OriginalTPL;

  Status = EFI_SUCCESS;

  // Get the physical address of the block
  BlockAddress = GET_NOR_BLOCK_ADDRESS (
                   Instance->RegionBaseAddress,
                   Lba,
                   BlockSizeInWords * 4
                   );

  // Start writing from the first address at the start of the block
  WordAddress = BlockAddress;

  NorFlashLock (&OriginalTPL);

  Status = NorFlashUnlockAndEraseSingleBlock (Instance, BlockAddress);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "WriteSingleBlock: ERROR - Failed to Unlock and Erase the single "
      "block at 0x%X\n",
      BlockAddress
      ));
    goto exit_handler;
  }

  for (WordIndex = 0;
       WordIndex < BlockSizeInWords;
       WordIndex++, DataBuffer++, WordAddress += 4)
  {
    Status = NorFlashWriteSingleWord (Instance, WordAddress, *DataBuffer);
    if (EFI_ERROR (Status)) {
      goto exit_handler;
    }
  }

exit_handler:
  NorFlashUnlock (OriginalTPL);

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "NOR FLASH Programming [WriteSingleBlock] failed at address 0x%08x. "
      "Exit Status = %r.\n",
      WordAddress,
      Status
      ));
  }

  return Status;
}

/**
  Write multiple blocks.

  @param[in]    Instance               NOR flash Instance of variable store region.
  @param[in]    Lba                    The starting logical block index.
  @param[in]    BufferSizeInBytes      The number of bytes to write.
  @param[in]    Buffer                 The pointer to a caller-allocated buffer that
                                       contains the source for the write.

  @retval       EFI_SUCCESS            Write is complete.
  @retval       EFI_INVALID_PARAMETER  Invalid parameters passed.
  @retval       EFI_BAD_BUFFER_SIZE    Invalid buffer size passed.
**/
EFI_STATUS
NorFlashWriteBlocks (
  IN NOR_FLASH_INSTANCE  *Instance,
  IN EFI_LBA             Lba,
  IN UINTN               BufferSizeInBytes,
  IN VOID                *Buffer
  )
{
  UINT32      *WriteBuffer;
  EFI_STATUS  Status;
  EFI_LBA     CurrentBlock;
  UINT32      BlockSizeInWords;
  UINT32      NumBlocks;
  UINT32      BlockCount;

  Status = EFI_SUCCESS;
  // The buffer must be valid
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // We must have some bytes to read
  DEBUG ((
    DEBUG_INFO,
    "NorFlashWriteBlocks: BufferSizeInBytes=0x%x\n",
    BufferSizeInBytes
    ));
  if (BufferSizeInBytes == 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  // The size of the buffer must be a multiple of the block size
  DEBUG ((
    DEBUG_INFO,
    "NorFlashWriteBlocks: BlockSize in bytes =0x%x\n",
    Instance->Media.BlockSize
    ));
  if ((BufferSizeInBytes % Instance->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  // All blocks must be within the device
  NumBlocks = ((UINT32)BufferSizeInBytes) / Instance->Media.BlockSize;

  DEBUG ((
    DEBUG_INFO,
    "NorFlashWriteBlocks: NumBlocks=%d, LastBlock=%ld, Lba=%ld.\n",
    NumBlocks,
    Instance->Media.LastBlock,
    Lba
    ));

  if ((Lba + NumBlocks) > (Instance->Media.LastBlock + 1)) {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashWriteBlocks: ERROR - Write will exceed last block.\n"
      ));
    return EFI_INVALID_PARAMETER;
  }

  BlockSizeInWords = Instance->Media.BlockSize / 4;

  // Because the target *Buffer is a pointer to VOID, we must put
  // all the data into a pointer to a proper data type, so use *WriteBuffer
  WriteBuffer = (UINT32 *)Buffer;

  CurrentBlock = Lba;
  for (BlockCount = 0;
       BlockCount < NumBlocks;
       BlockCount++, CurrentBlock++, WriteBuffer += BlockSizeInWords)
  {
    DEBUG ((
      DEBUG_INFO,
      "NorFlashWriteBlocks: Writing block #%d\n",
      (UINTN)CurrentBlock
      ));

    Status = NorFlashWriteFullBlock (
               Instance,
               CurrentBlock,
               WriteBuffer,
               BlockSizeInWords
               );

    if (EFI_ERROR (Status)) {
      break;
    }
  }

  DEBUG ((DEBUG_INFO, "NorFlashWriteBlocks: Exit Status = %r.\n", Status));
  return Status;
}

/**
  Read multiple blocks.

  @param[in]     Instance               NOR flash Instance of variable store region.
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
  )
{
  UINT32  NumBlocks;
  UINTN   StartAddress;

  DEBUG ((
    DEBUG_INFO,
    "NorFlashReadBlocks: BufferSize=0x%xB BlockSize=0x%xB LastBlock=%ld, "
    "Lba=%ld.\n",
    BufferSizeInBytes,
    Instance->Media.BlockSize,
    Instance->Media.LastBlock,
    Lba
    ));

  // The buffer must be valid
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Return if we do not have any byte to read
  if (BufferSizeInBytes == 0) {
    return EFI_SUCCESS;
  }

  // The size of the buffer must be a multiple of the block size
  if ((BufferSizeInBytes % Instance->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  NumBlocks = ((UINT32)BufferSizeInBytes) / Instance->Media.BlockSize;

  if ((Lba + NumBlocks) > (Instance->Media.LastBlock + 1)) {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashReadBlocks: ERROR - Read will exceed last block\n"
      ));
    return EFI_INVALID_PARAMETER;
  }

  // Get the address to start reading from
  StartAddress = GET_NOR_BLOCK_ADDRESS (
                   Instance->RegionBaseAddress,
                   Lba,
                   Instance->Media.BlockSize
                   );

  // Readout the data
  CopyMem (Buffer, (UINTN *)StartAddress, BufferSizeInBytes);

  return EFI_SUCCESS;
}

/**
  Read from NOR flash.

  @param[in]     Instance               NOR flash Instance of variable store region.
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
  )
{
  UINTN  StartAddress;

  // The buffer must be valid
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Return if we do not have any byte to read
  if (BufferSizeInBytes == 0) {
    return EFI_SUCCESS;
  }

  if (((Lba * Instance->Media.BlockSize) + Offset + BufferSizeInBytes) >
      Instance->Size)
  {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashRead: ERROR - Read will exceed device size.\n"
      ));
    return EFI_INVALID_PARAMETER;
  }

  // Get the address to start reading from
  StartAddress = GET_NOR_BLOCK_ADDRESS (
                   Instance->RegionBaseAddress,
                   Lba,
                   Instance->Media.BlockSize
                   );

  // Readout the data
  CopyMem (Buffer, (UINTN *)(StartAddress + Offset), BufferSizeInBytes);

  return EFI_SUCCESS;
}

/**
  Write a full or portion of a block.

  @param[in]       Instance               NOR flash Instance of variable store region.
  @param[in]       Lba                    The starting logical block index to write to.
  @param[in]       Offset                 Offset into the block at which to begin writing.
  @param[in, out]  NumBytes               The total size of the buffer.
  @param[in]       Buffer                 The pointer to a caller-allocated buffer that
                                          contains the source for the write.

  @retval          EFI_SUCCESS            Write is complete.
  @retval          EFI_INVALID_PARAMETER  Invalid parameters passed.
  @retval          EFI_BAD_BUFFER_SIZE    Buffer size not enough.
  @retval          EFI_DEVICE_ERROR       The device reported an error.
**/
EFI_STATUS
NorFlashWriteSingleBlock (
  IN        NOR_FLASH_INSTANCE  *Instance,
  IN        EFI_LBA             Lba,
  IN        UINTN               Offset,
  IN OUT    UINTN               *NumBytes,
  IN        UINT8               *Buffer
  )
{
  EFI_STATUS  Status;
  UINT32      Tmp;
  UINT32      TmpBuf;
  UINT32      WordToWrite;
  UINT32      Mask;
  BOOLEAN     DoErase;
  UINTN       BytesToWrite;
  UINTN       CurOffset;
  UINTN       WordAddr;
  UINTN       BlockSize;
  UINTN       BlockAddress;
  UINTN       PrevBlockAddress;

  if (Buffer == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashWriteSingleBlock: ERROR - Buffer is invalid\n"
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (NumBytes == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashWriteSingleBlock: ERROR - NumBytes is invalid\n"
      ));
    return EFI_INVALID_PARAMETER;
  }

  PrevBlockAddress = 0;

  DEBUG ((
    DEBUG_INFO,
    "NorFlashWriteSingleBlock(Parameters: Lba=%ld, Offset=0x%x, "
    "*NumBytes=0x%x, Buffer @ 0x%08x)\n",
    Lba,
    Offset,
    *NumBytes,
    Buffer
    ));

  // Locate the block size to avoid de-referencing pointers all the time
  BlockSize = Instance->Media.BlockSize;

  // The write must not span block boundaries.
  // We need to check each variable individually because adding two large
  // values together overflows.
  if ((Offset               >= BlockSize) ||
      (*NumBytes            >  BlockSize) ||
      ((Offset + *NumBytes) >  BlockSize))
  {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashWriteSingleBlock: ERROR - EFI_BAD_BUFFER_SIZE: "
      "(Offset=0x%x + NumBytes=0x%x) > BlockSize=0x%x\n",
      Offset,
      *NumBytes,
      BlockSize
      ));
    return EFI_BAD_BUFFER_SIZE;
  }

  // We must have some bytes to write
  if (*NumBytes == 0) {
    DEBUG ((
      DEBUG_ERROR,
      "NorFlashWriteSingleBlock: ERROR - EFI_BAD_BUFFER_SIZE: "
      "(Offset=0x%x + NumBytes=0x%x) > BlockSize=0x%x\n",
      Offset,
      *NumBytes,
      BlockSize
      ));
    return EFI_BAD_BUFFER_SIZE;
  }

  // Pick 128bytes as a good start for word operations as opposed to erasing the
  // block and writing the data regardless if an erase is really needed.
  // It looks like most individual NV variable writes are smaller than 128bytes.
  if (*NumBytes <= 128) {
    // Check to see if we need to erase before programming the data into NOR.
    // If the destination bits are only changing from 1s to 0s we can just write.
    // After a block is erased all bits in the block is set to 1.
    // If any byte requires us to erase we just give up and rewrite all of it.
    DoErase      = FALSE;
    BytesToWrite = *NumBytes;
    CurOffset    = Offset;

    while (BytesToWrite > 0) {
      // Read full word from NOR, splice as required. A word is the smallest
      // unit we can write.
      Status = NorFlashRead (
                 Instance,
                 Lba,
                 CurOffset & ~(0x3),
                 sizeof (Tmp),
                 &Tmp
                 );
      if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
      }

      // Physical address of word in NOR to write.
      WordAddr = (CurOffset & ~(0x3)) +
                 GET_NOR_BLOCK_ADDRESS (
                   Instance->RegionBaseAddress,
                   Lba,
                   BlockSize
                   );

      // The word of data that is to be written.
      TmpBuf = ReadUnaligned32 (
                 (UINT32 *)(Buffer + (*NumBytes - BytesToWrite))
                 );

      // First do word aligned chunks.
      if ((CurOffset & 0x3) == 0) {
        if (BytesToWrite >= 4) {
          // Is the destination still in 'erased' state?
          if (~Tmp != 0) {
            // Check to see if we are only changing bits to zero.
            if ((Tmp ^ TmpBuf) & TmpBuf) {
              DoErase = TRUE;
              break;
            }
          }

          // Write this word to NOR
          WordToWrite   = TmpBuf;
          CurOffset    += sizeof (TmpBuf);
          BytesToWrite -= sizeof (TmpBuf);
        } else {
          // BytesToWrite < 4. Do small writes and left-overs
          Mask = ~((~0) << (BytesToWrite * 8));
          // Mask out the bytes we want.
          TmpBuf &= Mask;
          // Is the destination still in 'erased' state?
          if ((Tmp & Mask) != Mask) {
            // Check to see if we are only changing bits to zero.
            if ((Tmp ^ TmpBuf) & TmpBuf) {
              DoErase = TRUE;
              break;
            }
          }

          // Merge old and new data. Write merged word to NOR
          WordToWrite  = (Tmp & ~Mask) | TmpBuf;
          CurOffset   += BytesToWrite;
          BytesToWrite = 0;
        }
      } else {
        // Do multiple words, but starting unaligned.
        if (BytesToWrite > (4 - (CurOffset & 0x3))) {
          Mask = ((~0) << ((CurOffset & 0x3) * 8));
          // Mask out the bytes we want.
          TmpBuf &= Mask;
          // Is the destination still in 'erased' state?
          if ((Tmp & Mask) != Mask) {
            // Check to see if we are only changing bits to zero.
            if ((Tmp ^ TmpBuf) & TmpBuf) {
              DoErase = TRUE;
              break;
            }
          }

          // Merge old and new data. Write merged word to NOR
          WordToWrite   = (Tmp & ~Mask) | TmpBuf;
          BytesToWrite -= (4 - (CurOffset & 0x3));
          CurOffset    += (4 - (CurOffset & 0x3));
        } else {
          // Unaligned and fits in one word.
          Mask = (~((~0) << (BytesToWrite * 8))) << ((CurOffset & 0x3) * 8);
          // Mask out the bytes we want.
          TmpBuf = (TmpBuf << ((CurOffset & 0x3) * 8)) & Mask;
          // Is the destination still in 'erased' state?
          if ((Tmp & Mask) != Mask) {
            // Check to see if we are only changing bits to zero.
            if ((Tmp ^ TmpBuf) & TmpBuf) {
              DoErase = TRUE;
              break;
            }
          }

          // Merge old and new data. Write merged word to NOR
          WordToWrite  = (Tmp & ~Mask) | TmpBuf;
          CurOffset   += BytesToWrite;
          BytesToWrite = 0;
        }
      }

      BlockAddress = GET_NOR_BLOCK_ADDRESS (
                       Instance->RegionBaseAddress,
                       Lba,
                       BlockSize
                       );
      if (BlockAddress != PrevBlockAddress) {
        Status = NorFlashUnlockSingleBlockIfNecessary (Instance, BlockAddress);
        if (EFI_ERROR (Status)) {
          return EFI_DEVICE_ERROR;
        }

        PrevBlockAddress = BlockAddress;
      }

      Status = NorFlashWriteSingleWord (Instance, WordAddr, WordToWrite);
      if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
      }
    }

    // Exit if we got here and could write all the data. Otherwise do the
    // Erase-Write cycle.
    if (!DoErase) {
      return EFI_SUCCESS;
    }
  }

  // Check we did get some memory. Buffer is BlockSize.
  if (Instance->ShadowBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "FvbWrite: ERROR - Buffer not ready\n"));
    return EFI_DEVICE_ERROR;
  }

  // Read NOR Flash data into shadow buffer
  Status = NorFlashReadBlocks (
             Instance,
             Lba,
             BlockSize,
             Instance->ShadowBuffer
             );
  if (EFI_ERROR (Status)) {
    // Return one of the pre-approved error statuses
    return EFI_DEVICE_ERROR;
  }

  // Put the data at the appropriate location inside the buffer area
  CopyMem (
    (VOID *)((UINTN)Instance->ShadowBuffer + Offset),
    Buffer,
    *NumBytes
    );

  // Write the modified buffer back to the NorFlash
  Status = NorFlashWriteBlocks (
             Instance,
             Lba,
             BlockSize,
             Instance->ShadowBuffer
             );
  if (EFI_ERROR (Status)) {
    // Return one of the pre-approved error statuses
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Read JEDEC ID of NOR flash device.

  @param[in]     Instance               NOR flash Instance of variable store region.
  @param[out]    JedecId                JEDEC ID of NOR flash device.
                                        Maximum length of JedecId can be upto 6 bytes

  @retval        EFI_SUCCESS            The write is completed.
  @retval        EFI_DEVICE_ERROR       Failed to fetch JEDEC ID.
  @retval        EFI_INVALID_PARAMETER  Invalid parameters passed.
**/
EFI_STATUS
NorFlashReadId (
  IN  NOR_FLASH_INSTANCE  *Instance,
  OUT UINT8               *JedecId
  )
{
  UINT32  val;

  if ((Instance == NULL) || (JedecId == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  val = (SPINOR_OP_RDID <<
         CDNS_QSPI_FLASH_CMD_CTRL_REG_OPCODE_BIT_POS) |
        (CDNS_QSPI_FLASH_CMD_CTRL_REG_READ_ENABLE <<
         CDNS_QSPI_FLASH_CMD_CTRL_REG_READEN_BIT_POS) |
        CDNS_QSPI_FLASH_CMD_CTRL_REG_NUM_DATA_BYTES (3);

  if (EFI_ERROR (CdnsQspiExecuteCommand (Instance, val))) {
    return EFI_DEVICE_ERROR;
  }

  val = MmioRead32 (
          Instance->HostControllerBaseAddress + CDNS_QSPI_FLASH_CMD_READ_DATA_REG_OFFSET
          );

  // Manufacturer ID field
  JedecId[0] = (UINT8)val;
  // Type field
  JedecId[1] = (UINT8)(val >> 8);
  // Capacity field
  JedecId[2] = (UINT8)(val >> 16);

  DEBUG ((
    DEBUG_INFO,
    "Nor flash detected, Jedec ID, Manufacturer Id=%x Type=%x Capacity=%x \n",
    JedecId[0],
    JedecId[1],
    JedecId[2]
    ));

  return EFI_SUCCESS;
}

/**
  NOR Flash Reset

  @param[in]         Instance     NOR flash instance.

  @retval            EFI_SUCCESS  Return success on every call.
**/
EFI_STATUS
NorFlashReset (
  IN  NOR_FLASH_INSTANCE  *Instance
  )
{
  return EFI_SUCCESS;
}
