/** @file

  Copyright (c) 2015, ARM Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "FdtPlatform.h"

#include <Library/FdtLib.h>

#define ALIGN(x, a)     (((x) + ((a) - 1)) & ~((a) - 1))
#define PALIGN(p, a)    ((void *)(ALIGN ((unsigned long)(p), (a))))
#define GET_CELL(p)     (p += 4, *((const UINT32 *)(p-4)))

#define FDT_BEGIN_NODE  0x1             /* Start node: full name */
#define FDT_END_NODE    0x2             /* End node */
#define FDT_PROP        0x3             /* Property: name off,
                                           size, content */
#define FDT_NOP         0x4             /* nop */
#define FDT_END         0x9

STATIC
UINTN
IsPrintableString (
  IN CONST VOID* data,
  IN UINTN len
  )
{
  CONST CHAR8 *s = data;
  CONST CHAR8 *ss;

  // Zero length is not
  if (len == 0) {
    return 0;
  }

  // Must terminate with zero
  if (s[len - 1] != '\0') {
    return 0;
  }

  ss = s;
  while (*s/* && isprint (*s)*/) {
    s++;
  }

  // Not zero, or not done yet
  if (*s != '\0' || (s + 1 - ss) < len) {
    return 0;
  }

  return 1;
}

STATIC
VOID
PrintData (
  IN CONST CHAR8* data,
  IN UINTN len
  )
{
  UINTN i;
  CONST CHAR8 *p = data;

  // No data, don't print
  if (len == 0)
    return;

  if (IsPrintableString (data, len)) {
    Print (L" = \"%a\"", (const char *)data);
  } else if ((len % 4) == 0) {
    Print (L" = <");
    for (i = 0; i < len; i += 4) {
      Print (L"0x%08x%a", Fdt32ToCpu (GET_CELL (p)), i < (len - 4) ? " " : "");
    }
    Print (L">");
  } else {
    Print (L" = [");
    for (i = 0; i < len; i++)
      Print (L"%02x%a", *p++, i < len - 1 ? " " : "");
    Print (L"]");
  }
}

STATIC
VOID
DumpFdt (
  IN VOID*                FdtBlob
  )
{
  FDT_HEADER *bph;
  UINT32 off_dt;
  UINT32 off_str;
  CONST CHAR8* p_struct;
  CONST CHAR8* p_strings;
  CONST CHAR8* p;
  CONST CHAR8* s;
  CONST CHAR8* t;
  UINT32 tag;
  UINTN sz;
  UINTN depth;
  UINTN shift;
  UINT32 version;

  {
    // Can 'memreserve' be printed by below code?
    INTN num = FdtGetNumberOfReserveMapEntries (FdtBlob);
    INTN i, err;
    UINT64 addr = 0, size = 0;

    for (i = 0; i < num; i++) {
      err = FdtGetReserveMapEntry (FdtBlob, i, &addr, &size);
      if (err) {
        DEBUG ((DEBUG_ERROR, "Error (%d) : Cannot get memreserve section (%d)\n", err, i));
      }
      else {
        Print (L"/memreserve/ \t0x%lx \t0x%lx;\n", addr, size);
      }
    }
  }

  depth = 0;
  shift = 4;

  bph = FdtBlob;
  off_dt = Fdt32ToCpu (bph->OffsetDtStruct);
  off_str = Fdt32ToCpu (bph->OffsetDtStrings);
  p_struct = (CONST CHAR8*)FdtBlob + off_dt;
  p_strings = (CONST CHAR8*)FdtBlob + off_str;
  version = Fdt32ToCpu (bph->Version);

  p = p_struct;
  while ((tag = Fdt32ToCpu (GET_CELL (p))) != FDT_END) {
    if (tag == FDT_BEGIN_NODE) {
      s = p;
      p = PALIGN (p + AsciiStrLen (s) + 1, 4);

      if (*s == '\0')
              s = "/";

      Print (L"%*s%a {\n", depth * shift, L" ", s);

      depth++;
      continue;
    }

    if (tag == FDT_END_NODE) {
      depth--;

      Print (L"%*s};\n", depth * shift, L" ");
      continue;
    }

    if (tag == FDT_NOP) {
      Print (L"%*s// [NOP]\n", depth * shift, L" ");
      continue;
    }

    if (tag != FDT_PROP) {
      Print (L"%*s ** Unknown tag 0x%08x\n", depth * shift, L" ", tag);
      break;
    }
    sz = Fdt32ToCpu (GET_CELL (p));
    s = p_strings + Fdt32ToCpu (GET_CELL (p));
    if (version < 16 && sz >= 8)
            p = PALIGN (p, 8);
    t = p;

    p = PALIGN (p + sz, 4);

    Print (L"%*s%a", depth * shift, L" ", s);
    PrintData (t, sz);
    Print (L";\n");
  }
}

/**
  This is the shell command "dumpfdt" handler function. This function handles
  the command when it is invoked in the shell.

  @param[in]  This             The instance of the
                               EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL.
  @param[in]  SystemTable      The pointer to the UEFI system table.
  @param[in]  ShellParameters  The parameters associated with the command.
  @param[in]  Shell            The instance of the shell protocol used in the
                               context of processing this command.

  @return  SHELL_SUCCESS            The operation was successful.
  @return  SHELL_ABORTED            Operation aborted due to internal error.
  @return  SHELL_NOT_FOUND          Failed to locate the Device Tree into the EFI Configuration Table
  @return  SHELL_OUT_OF_RESOURCES   A memory allocation failed.

**/
SHELL_STATUS
EFIAPI
ShellDynCmdDumpFdtHandler (
  IN EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL  *This,
  IN EFI_SYSTEM_TABLE                    *SystemTable,
  IN EFI_SHELL_PARAMETERS_PROTOCOL       *ShellParameters,
  IN EFI_SHELL_PROTOCOL                  *Shell
  )
{
  SHELL_STATUS  ShellStatus;
  EFI_STATUS    Status;
  VOID          *FdtBlob;

  ShellStatus  = SHELL_SUCCESS;

  //
  // Install the Shell and Shell Parameters Protocols on the driver
  // image. This is necessary for the initialisation of the Shell
  // Library to succeed in the next step.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gImageHandle,
                  &gEfiShellProtocolGuid, Shell,
                  &gEfiShellParametersProtocolGuid, ShellParameters,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return SHELL_ABORTED;
  }

  //
  // Initialise the Shell Library as we are going to use it.
  // Assert that the return code is EFI_SUCCESS as it should.
  // To anticipate any change is the codes returned by
  // ShellInitialize(), leave in case of error.
  //
  Status = ShellInitialize ();
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return SHELL_ABORTED;
  }

  Status = EfiGetSystemConfigurationTable (&gFdtTableGuid, &FdtBlob);
  if (EFI_ERROR (Status)) {
    Print (L"ERROR: Did not find the Fdt Blob.\n");
    return EfiCodeToShellCode (Status);
  }

  DumpFdt (FdtBlob);

  gBS->UninstallMultipleProtocolInterfaces (
         gImageHandle,
         &gEfiShellProtocolGuid, Shell,
         &gEfiShellParametersProtocolGuid, ShellParameters,
         NULL
         );

  return ShellStatus;
}

/**
  This is the shell command "dumpfdt" help handler function. This
  function returns the formatted help for the "dumpfdt" command.
  The format matchs that in Appendix B of the revision 2.1 of the
  UEFI Shell Specification.

  @param[in]  This      The instance of the EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL.
  @param[in]  Language  The pointer to the language string to use.

  @return  CHAR16*  Pool allocated help string, must be freed by caller.
**/
CHAR16*
EFIAPI
ShellDynCmdDumpFdtGetHelp (
  IN EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL  *This,
  IN CONST CHAR8                         *Language
  )
{
  //
  // This allocates memory. The caller has to free the allocated memory.
  //
  return HiiGetString (
                mFdtPlatformDxeHiiHandle,
                STRING_TOKEN (STR_GET_HELP_DUMPFDT),
                Language
                );
}
