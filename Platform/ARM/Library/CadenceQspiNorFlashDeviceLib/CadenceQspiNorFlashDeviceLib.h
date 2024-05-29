/** @file

  Copyright (c) 2024, ARM Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef CADENCE_QSPI_NOR_FLASH_DEVICE_LIB_H_
#define CADENCE_QSPI_NOR_FLASH_DEVICE_LIB_H_

#define NOR_FLASH_ERASE_RETRY  10

// QSPI Controller defines
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_OFFSET             0x90
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_EXECUTE            0x01
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_ADDR_ENABLE        0x01
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_ADDR_BIT_POS       19
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_ADDR_BYTE_BIT_POS  16
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_STATUS_BIT         0x02
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_OPCODE_BIT_POS     24
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_READ_ENABLE        0x01
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_READ_BYTE_3B       0x02
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_READEN_BIT_POS     23
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_READBYTE_BIT_POS   20
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_DUMMY_8C           0x8
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_DUMMY_BIT_POS      7
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_NUM_DATA_BYTES(x)  ((x - 1) << CDNS_QSPI_FLASH_CMD_CTRL_REG_READBYTE_BIT_POS)
#define CDNS_QSPI_FLASH_CMD_CTRL_REG_NUM_ADDR_BYTES(x)  ((x - 1) << CDNS_QSPI_FLASH_CMD_CTRL_REG_ADDR_BYTE_BIT_POS)

#define CDNS_QSPI_FLASH_CMD_READ_DATA_REG_OFFSET  0xA0

#define CDNS_QSPI_FLASH_CMD_ADDR_REG_OFFSET  0x94

#define CDNS_QSPI_FLASH_CMD_STATUS_POLL_TIMEOUT_MS  1000u // Command Status Register read timeout

#define SPINOR_SR_WIP  BIT0                               // Write in progress

#define SPINOR_OP_WREN   0x06                             // Write enable
#define SPINOR_OP_BE_4K  0x20                             // Erase 4KiB block
#define SPINOR_OP_RDID   0x9f                             // Read JEDEC ID
#define SPINOR_OP_RDSR   0x05                             // Read status register

#define SPINOR_SR_WIP_POLL_TIMEOUT_MS  1000u              // Status Register read timeout

#endif /* CADENCE_QSPI_NOR_FLASH_DEVICE_LIB_H_ */
