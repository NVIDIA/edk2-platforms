## @file
#
# Copyright (c) 2020 - 2021, Ampere Computing LLC. All rights reserved.<BR>
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
  PLATFORM_NAME                  = Altra1L2T
  PLATFORM_GUID                  = edc65dea-a173-496a-8ad0-5c96e3fd2079
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x0001001E
  OUTPUT_DIRECTORY               = Build/Altra1L2T
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/ASRockRack/Altra1L2TPkg/Altra1L2T.fdf

  #
  # Defines for default states. These can be changed on the command line.
  # -D FLAG=VALUE
  #

  #  DEBUG_INIT      0x00000001       // Initialization
  #  DEBUG_WARN      0x00000002       // Warnings
  #  DEBUG_LOAD      0x00000004       // Load events
  #  DEBUG_FS        0x00000008       // EFI File system
  #  DEBUG_POOL      0x00000010       // Alloc & Free (pool)
  #  DEBUG_PAGE      0x00000020       // Alloc & Free (page)
  #  DEBUG_INFO      0x00000040       // Informational debug messages
  #  DEBUG_DISPATCH  0x00000080       // PEI/DXE/SMM Dispatchers
  #  DEBUG_VARIABLE  0x00000100       // Variable
  #  DEBUG_BM        0x00000400       // Boot Manager
  #  DEBUG_BLKIO     0x00001000       // BlkIo Driver
  #  DEBUG_NET       0x00004000       // SNP Driver
  #  DEBUG_UNDI      0x00010000       // UNDI Driver
  #  DEBUG_LOADFILE  0x00020000       // LoadFile
  #  DEBUG_EVENT     0x00080000       // Event messages
  #  DEBUG_GCD       0x00100000       // Global Coherency Database changes
  #  DEBUG_CACHE     0x00200000       // Memory range cachability changes
  #  DEBUG_VERBOSE   0x00400000       // Detailed debug messages that may
  #                                   // significantly impact boot performance
  #  DEBUG_MANAGEABILITY  0x00800000  // Detailed debug and payload manageability messages
  #                                   // related to modules such as Redfish, IPMI, MCTP etc.
  #  DEBUG_ERROR     0x80000000       // Error
!if $(TARGET) == RELEASE
  DEFINE DEBUG_PRINT_ERROR_LEVEL = 0x80000002
!else
  DEFINE DEBUG_PRINT_ERROR_LEVEL = 0x8000004F
!endif

  DEFINE FIRMWARE_VER            = 00.01.01-00
  DEFINE FIRMWARE_VER_HEX        = 0x00010100
  DEFINE CAPSULE_ENABLE          = TRUE
  DEFINE INCLUDE_TFA_FW          = TRUE
  DEFINE UEFI_SECURE_BOOT_ENABLE = TRUE
  DEFINE TPM2_ENABLE             = TRUE
  DEFINE SHELL_ENABLE            = TRUE
  DEFINE INCLUDE_TFTP_COMMAND    = TRUE
  DEFINE PLATFORM_CONFIG_UUID    = e268c8df-2676-444d-be6c-7dbd660d72c1

  #
  # Network definition
  #
  DEFINE NETWORK_ENABLE                      = TRUE
  DEFINE NETWORK_IP6_ENABLE                  = TRUE
  DEFINE NETWORK_HTTP_BOOT_ENABLE            = TRUE
  DEFINE NETWORK_ALLOW_HTTP_CONNECTIONS      = TRUE
  DEFINE NETWORK_TLS_ENABLE                  = TRUE
  DEFINE REDFISH_ENABLE                      = TRUE
  DEFINE PERFORMANCE_MEASUREMENT_ENABLE      = FALSE
  DEFINE HEAP_GUARD_ENABLE                   = FALSE

!if $(CAPSULE_ENABLE) == TRUE
  DEFINE UEFI_IMAGE              = Build/Altra1L2T/altra1l2q_uefi.bin
  DEFINE TFA_UEFI_IMAGE          = Build/Altra1L2T/altra1l2q_tfa_uefi.bin
!endif

!include MdePkg/MdeLibs.dsc.inc

# Include default Ampere Platform DSC file
!include Silicon/Ampere/AmpereAltraPkg/AmpereAltraPkg.dsc.inc

################################################################################
#
# Specific Platform Library
#
################################################################################
[LibraryClasses]

  OemMiscLib|Platform/ASRockRack/AltraBoardPkg/Library/OemMiscLib/OemMiscLib.inf
  JedecJep106Lib|MdePkg/Library/JedecJep106Lib/JedecJep106Lib.inf

  #
  # ACPI Libraries
  #
  AcpiLib|EmbeddedPkg/Library/AcpiLib/AcpiLib.inf

  #
  # EFI Redfish drivers
  #
!if $(NETWORK_ENABLE) == TRUE
!if $(REDFISH_ENABLE) == TRUE
  RedfishContentCodingLib|RedfishPkg/Library/RedfishContentCodingLibNull/RedfishContentCodingLibNull.inf
  RedfishPlatformHostInterfaceLib|RedfishPkg/Library/PlatformHostInterfaceLibNull/PlatformHostInterfaceLibNull.inf
  RedfishPlatformWantedDeviceLib|RedfishPkg/Library/RedfishPlatformWantedDeviceLibNull/RedfishPlatformWantedDeviceLibNull.inf
!endif
!endif

  #
  # Pcie Board
  #
  BoardPcieLib|Platform/ASRockRack/AltraBoardPkg/Library/BoardPcieLib/BoardPcieLib.inf

  IOExpanderLib|Platform/Ampere/JadePkg/Library/IOExpanderLib/IOExpanderLib.inf

  PlatformBmcReadyLib|Platform/Ampere/JadePkg/Library/PlatformBmcReadyLib/PlatformBmcReadyLib.inf
  LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf

  #
  # RTC Library: Common RTC
  #
  RealTimeClockLib|Platform/ASRockRack/AltraBoardPkg/Library/ISL1208RealTimeClockLib/ISL1208RealTimeClockLib.inf

  #
  # EFI Redfish drivers
  #
!if $(REDFISH_ENABLE) == TRUE
  RedfishContentCodingLib|RedfishPkg/Library/RedfishContentCodingLibNull/RedfishContentCodingLibNull.inf
  RedfishPlatformHostInterfaceLib|RedfishPkg/Library/PlatformHostInterfaceBmcUsbNicLib/PlatformHostInterfaceBmcUsbNicLib.inf
!endif

################################################################################
#
# Specific Platform Pcds
#
################################################################################
[PcdsFeatureFlag.common]
  #
  # Activate AcpiSdtProtocol
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE

  #
  # Flag to indicate option of using default or specific platform Port Map table
  #
  gAmpereTokenSpaceGuid.PcdPcieHotPlugPortMapTable.UseDefaultConfig|FALSE

  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|TRUE

[PcdsFixedAtBuild]

  gAmpereTokenSpaceGuid.PcdPcieHotPlugGpioResetMap|0x3F

  #
  # Setting Portmap table
  #
  #   * Elements of array:
  #     - 0:  Index of Portmap entry in Portmap table structure (Vport).
  #     - 1:  Socket number (Socket).
  #     - 2:  Root complex port for each Portmap entry (RcaPort).
  #     - 3:  Root complex sub-port for each Portmap entry (RcaSubPort).
  #     - 4:  Select output port of IO expander (PinPort).
  #     - 5:  I2C address of IO expander that CPLD backplane simulates (I2cAddress).
  #     - 6:  Address of I2C switch between CPU and CPLD backplane (MuxAddress).
  #     - 7:  Channel of I2C switch (MuxChannel).
  #     - 8:  It is set from PcieHotPlugSetGPIOMapCmd () function to select GPIO[16:21] (PcdPcieHotPlugGpioResetMap) or I2C for PCIe reset purpose.
  #     - 9:  Segment of root complex (Segment).
  #     - 10: SSD slot index on the front panel of backplane (DriveIndex).
  #
  #   * Caution:
  #     - The last array ({ 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF }) require if no fully structured used.
  #     - Size of Portmap table: PortMap[MAX_PORTMAP_ENTRY][sizeof(PCIE_HOTPLUG_PORTMAP_ENTRY)] <=> PortMap[96][11].
  #   * Example: Bellow configuration is the configuration for Portmap table of Mt. Jade 2U platform.
  #
  gAmpereTokenSpaceGuid.PcdPcieHotPlugPortMapTable.PortMap[0]|{ 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF }       # Require if no fully structure used

  gAmpereTokenSpaceGuid.PcdSmbusI2cBusSpeed|100000

  # We should support CoD in future, since it provides a nicer
  # upgrade experience (e.g. a progress bar).
  gEfiMdeModulePkgTokenSpaceGuid.PcdCapsuleOnDiskSupport|FALSE

!if $(UEFI_SECURE_BOOT_ENABLE) == TRUE
  gEfiSecurityPkgTokenSpaceGuid.PcdRsa2048Sha256PublicKeyBuffer|{0}
  !include Platform/ASRockRack/Altra1L2TPkg/root.cer.gEfiSecurityPkgTokenSpaceGuid.PcdPkcs7CertBuffer.inc
!endif

  gAmpereTokenSpaceGuid.PcdFirmwareVersionNumber|$(FIRMWARE_VER_HEX)

[PcdsFixedAtBuild.common]
  #
  # Platform config UUID
  #
  gAmpereTokenSpaceGuid.PcdPlatformConfigUuid|"$(PLATFORM_CONFIG_UUID)"

  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion|0x307

  # Clearing BIT0 in this PCD prevents installing a 32-bit SMBIOS entry point,
  # if the entry point version is >= 3.0. AARCH64 OSes cannot assume the
  # presence of the 32-bit entry point anyway (because many AARCH64 systems
  # don't have 32-bit addressable physical RAM), and the additional allocations
  # below 4 GB needlessly fragment the memory map. So expose the 64-bit entry
  # point only, for entry point versions >= 3.0.
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosEntryPointProvideMethod|0x2

!if $(UEFI_SECURE_BOOT_ENABLE) == TRUE
  # Override the default values from SecurityPkg to ensure images
  # from all sources are verified in secure boot
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif

  #
  # Optional feature to help prevent EFI memory map fragments
  # Turned on and off via: PcdPrePiProduceMemoryTypeInformationHob
  # Values are in EFI Pages (4K). DXE Core will make sure that
  # at least this much of each type of memory can be allocated
  # from a single memory range. This way you only end up with
  # maximum of two fragments for each type in the memory map
  # (the memory used, and the free memory that was prereserved
  # but not used).
  #
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
!if $(UEFI_SECURE_BOOT_ENABLE) == TRUE
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|600
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1500
!else
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|300
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|150
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1000
!endif
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|12000
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|20
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0

  #
  # Enable strict image permissions for all images. (This applies
  # only to images that were built with >= 4 KB section alignment.)
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdImageProtectionPolicy|0x3

  #
  # Enable NX memory protection for all non-code regions, including OEM and OS
  # reserved ones, with the exception of LoaderData regions, of which OS loaders
  # (e.g., GRUB) may assume that its contents are executable.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeNxMemoryProtectionPolicy|0xC000000000007FD5

  gEfiMdeModulePkgTokenSpaceGuid.PcdCpuStackGuard|TRUE

!if $(HEAP_GUARD_ENABLE) == TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdHeapGuardPageType|0xC000000000007B9E
  gEfiMdeModulePkgTokenSpaceGuid.PcdHeapGuardPoolType|0xC000000000007B9E
  gEfiMdeModulePkgTokenSpaceGuid.PcdHeapGuardPropertyMask|0x0F
!endif

[PcdsDynamicDefault.common.DEFAULT]
  # SMBIOS Type 0 - BIOS Information
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"ASRock Rack Inc."
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareReleaseDateString|L"MM/DD/YYYY"

[PcdsDynamicExDefault.common.DEFAULT]
  gEfiSignedCapsulePkgTokenSpaceGuid.PcdEdkiiSystemFirmwareImageDescriptor|{0x0}|VOID*|0x100
  gEfiMdeModulePkgTokenSpaceGuid.PcdSystemFmpCapsuleImageTypeIdGuid|{GUID("731cbc77-cce1-4ec2-b79a-265470b332f1")}|VOID*|0x10
  gEfiSignedCapsulePkgTokenSpaceGuid.PcdEdkiiSystemFirmwareFileGuid|{GUID("a4c7d17d-491f-4be6-a261-ed5d9d36de42")}|VOID*|0x10

  # Default Video Resolution
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|0  # 0 - Maximum
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|0    # 0 - Maximum
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|0  # 0 - Maximum
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|0     # 0 - Maximum
  # Setup Video Resolution
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|0  # 0 - Maximum
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|0    # 0 - Maximum
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|0  # 0 - Maximum
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|0     # 0 - Maximum

[PcdsPatchableInModule]
  #
  # Console Resolution (HD mode)
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768

################################################################################
#
# Specific Platform Component
#
################################################################################
[Components.common]
  #
  # ACPI
  #
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2B
  }
  Platform/Ampere/JadePkg/Drivers/AcpiPlatformDxe/AcpiPlatformDxe.inf
  Silicon/Ampere/AmpereAltraPkg/AcpiCommonTables/AcpiCommonTables.inf
  Platform/ASRockRack/AltraBoardPkg/Ac01AcpiTables/Ac01AcpiTables.inf
  Platform/ASRockRack/AltraBoardPkg/Ac02AcpiTables/Ac02AcpiTables.inf

  #
  # PCIe
  #
  Platform/Ampere/JadePkg/Drivers/PciPlatformDxe/PciPlatformDxe.inf

  !if $(NETWORK_ENABLE) == TRUE
    # Intel I210
    !if $(INTEL_UNDI_BIN) == TRUE
      IntelUndiBin/GigUndiBinRelease.inf
    !endif
    # For the Redfish USB CDC connection to the BMC
    MdeModulePkg/Bus/Usb/UsbNetwork/NetworkCommon/NetworkCommon.inf
    MdeModulePkg/Bus/Usb/UsbNetwork/UsbCdcEcm/UsbCdcEcm.inf
  !endif

  #
  # Renesas PD720202 XHCI firmware uploader
  #
!ifdef $(USB_UPD720202_ROM_FILE)
  Drivers/OptionRomPkg/RenesasFirmwarePD720202/RenesasFirmwarePD720202.inf {
    <LibraryClasses>
      DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  }
!endif

  #
  # VGA Aspeed
  #
  Drivers/ASpeed/ASpeedGopBinPkg/ASpeedAst2500GopDxe.inf

  #
  # SMBIOS
  #
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  ArmPkg/Universal/Smbios/ProcessorSubClassDxe/ProcessorSubClassDxe.inf
  ArmPkg/Universal/Smbios/SmbiosMiscDxe/SmbiosMiscDxe.inf
  Platform/ASRockRack/AltraBoardPkg/Drivers/SmbiosPlatformDxe/SmbiosPlatformDxe.inf
  ManageabilityPkg/Universal/IpmiBlobTransferDxe/IpmiBlobTransferDxe.inf
  Features/ManageabilityPkg/Universal/IpmiProtocol/Dxe/IpmiProtocolDxe.inf 

  #
  # HII
  #
  Silicon/Ampere/AmpereAltraPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
  Silicon/Ampere/AmpereAltraPkg/Drivers/MemInfoDxe/MemInfoDxe.inf
  Silicon/Ampere/AmpereAltraPkg/Drivers/CpuConfigDxe/CpuConfigDxe.inf
  Silicon/Ampere/AmpereAltraPkg/Drivers/AcpiConfigDxe/AcpiConfigDxe.inf
  Silicon/Ampere/AmpereAltraPkg/Drivers/RasConfigDxe/RasConfigDxe.inf
  Silicon/Ampere/AmpereAltraPkg/Drivers/RootComplexConfigDxe/RootComplexConfigDxe.inf
  Silicon/Ampere/AmpereSiliconPkg/Drivers/BmcConfigDxe/BmcConfigDxe.inf

  #
  # Firmware Capsule Update
  #
!if $(CAPSULE_ENABLE) == TRUE
  Platform/ASRockRack/Altra1L2TPkg/Capsule/SystemFirmwareDescriptor/SystemFirmwareDescriptor.inf
  MdeModulePkg/Universal/EsrtDxe/EsrtDxe.inf
  MdeModulePkg/Universal/EsrtFmpDxe/EsrtFmpDxe.inf
  SignedCapsulePkg/Universal/SystemFirmwareUpdate/SystemFirmwareReportDxe.inf {
    <LibraryClasses>
      FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
  }
  SignedCapsulePkg/Universal/SystemFirmwareUpdate/SystemFirmwareUpdateDxe.inf {
    <LibraryClasses>
      FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
  }
  MdeModulePkg/Application/CapsuleApp/CapsuleApp.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }

  #
  # System Firmware Update
  #
  Silicon/Ampere/AmpereAltraPkg/Drivers/SystemFirmwareUpdateDxe/SystemFirmwareUpdateDxe.inf
!endif

  # Redfish
  #
!if $(NETWORK_ENABLE) == TRUE
  SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto.inf
  !if $(REDFISH_ENABLE) == TRUE
    !include RedfishPkg/Redfish.dsc.inc
  !endif
!endif

  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf

  # Multi-Processor Support
  ArmPkg/Drivers/ArmPsciMpServicesDxe/ArmPsciMpServicesDxe.inf

!if $(PERFORMANCE_MEASUREMENT_ENABLE) == TRUE
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTablePei/FirmwarePerformancePei.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTableDxe/FirmwarePerformanceDxe.inf
!if $(SHELL_ENABLE) == TRUE
  ShellPkg/DynamicCommand/DpDynamicCommand/DpDynamicCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
!endif
!endif

  #
  # OpRom emulator
  #
!if $(X86_EMULATOR_ENABLE) == TRUE
  Emulator/X86EmulatorDxe/X86EmulatorDxe.inf
!endif
