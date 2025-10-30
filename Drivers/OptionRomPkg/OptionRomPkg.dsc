## @file
# Option Rom Package build validation file for All Architectures.
#
# This package is designed to interoperate with the EDK II open source project
# at http://www.tianocore.org, and this package is required to build PCI compliant
# Option ROM image for all CPU architectures, including EBC target.
# A single driver can support mixes of EFI 1.1, UEFI 2.0 and UEFI 2.1.
#
# Copyright (c) 2007 - 2021, Intel Corporation. All rights reserved.<BR>
# Copyright (c) 2016, Linaro Ltd. All rights reserved.<BR>
# Copyright (c) 2020, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = OptionRomPkg
  PLATFORM_GUID                  = C7B25F37-B1F4-4c46-99CB-3EA7DCF5FCDC
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/OptionRomPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64|EBC|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this
#                              Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT              # The entry: 0|DEFAULT is reserved and always required.

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses]
  DebugLib|MdePkg/Library/UefiDebugLibStdErr/UefiDebugLibStdErr.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf  
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BltLib|OptionRomPkg/Library/GopBltLib/GopBltLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
  gEfiMdePkgTokenSpaceGuid.PcdDebugClearMemoryValue|0x0
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|0x0
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|0x0
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|0x0
  gOptionRomPkgTokenSpaceGuid.PcdDriverSupportedEfiVersion|0x0002000a # EFI_2_10_SYSTEM_TABLE_REVISION

###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################

[Components]
  OptionRomPkg/Library/FrameBufferBltLib/FrameBufferBltLib.inf
  OptionRomPkg/Library/GopBltLib/GopBltLib.inf

  OptionRomPkg/AtapiPassThruDxe/AtapiPassThruDxe.inf
  OptionRomPkg/CirrusLogic5430Dxe/CirrusLogic5430Dxe.inf
  OptionRomPkg/UndiRuntimeDxe/UndiRuntimeDxe.inf
  OptionRomPkg/Bus/Usb/FtdiUsbSerialDxe/FtdiUsbSerialDxe.inf
  OptionRomPkg/RenesasFirmwarePD720202/RenesasFirmwarePD720202.inf {
    <LibraryClasses>
      DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  }

[Components.IA32, Components.X64]
  OptionRomPkg/Application/BltLibSample/BltLibSample.inf
