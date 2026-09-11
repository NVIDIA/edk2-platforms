## @file
# AMD Platform Package DSC include file
# Provides [LibraryClasses] and [Components] sections for AmdPlatformPkg.
# Include this file from a platform DSC after base library mappings are defined.
#
# Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
##


[LibraryClasses.Common]
  TimerLib|AmdPlatformPkg/Library/CcxTscTimerLib/BaseTscTimerLib.inf

[LibraryClasses.Common.PEIM, LibraryClasses.Common.PEI_CORE]
  TimerLib|AmdPlatformPkg/Library/CcxTscTimerLib/PeiTscTimerLib.inf

[LibraryClasses.Common.PEIM]

[LibraryClasses.Common.DXE_CORE, LibraryClasses.Common.DXE_DRIVER, LibraryClasses.Common.DXE_RUNTIME_DRIVER, LibraryClasses.Common.DXE_SMM_DRIVER, LibraryClasses.Common.SMM_CORE, LibraryClasses.Common.UEFI_DRIVER, LibraryClasses.Common.UEFI_APPLICATION]
  TimerLib|AmdPlatformPkg/Library/CcxTscTimerLib/DxeTscTimerLib.inf

[LibraryClasses.Common.DXE_DRIVER]

[LibraryClasses.Common.DXE_SMM_DRIVER]

[LibraryClasses.Common.SMM_CORE]

[Components.IA32]

[Components.X64]
