/** @file
  PspPreTcgEventLogPei driver.

  Retrieves PSP early-boot TPM measurements via the AmdPspDtpmPpi and deposits
  them as EDK2 TCG HOBs (gTcgEventEntryHobGuid / gTcgEvent2EntryHobGuid) for
  consumption by Tcg2Dxe's SetupEventLog(). No modification to Tcg2Dxe is
  required; it replays the HOBs automatically in Phase 3.

  Copyright (C) 2026, Advanced Micro Devices, Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include <IndustryStandard/Tpm20.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Ppi/AmdPspDtpmPpi.h>

extern EFI_GUID  gAmdPspPreTcgEventLogDonePpiGuid;

/** PPI descriptor for gAmdPspPreTcgEventLogDonePpiGuid -- installed at Exit. */
STATIC CONST EFI_PEI_PPI_DESCRIPTOR  mPspPreTcgEventLogDonePpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gAmdPspPreTcgEventLogDonePpiGuid,
  NULL
};

/** Maximum size of PSP dTPM log buffer. 4 KB matches PSP firmware limit. */
#define PSP_DTPM_LOG_BUFFER_SIZE  0x1000u

/**
  Create a gTcgEventEntryHobGuid HOB from the first (TCG 1.2) PSP log entry.

  @param[in]  EventHdr   Pointer to TCG_PCR_EVENT_HDR at the start of the PSP buffer.
  @param[in]  BufEnd     One-past-end of the valid PSP buffer region.

  @retval EFI_SUCCESS            HOB created successfully.
  @retval EFI_INVALID_PARAMETER  Entry extends beyond BufEnd.
  @retval EFI_OUT_OF_RESOURCES   HOB pool exhausted.
**/
STATIC
EFI_STATUS
CreateTcg12EntryHob (
  IN CONST TCG_PCR_EVENT_HDR  *EventHdr,
  IN CONST UINT8              *BufEnd
  )
{
  CONST UINT8  *EventData;
  UINTN        HobDataSize;
  VOID         *HobData;

  if ((CONST UINT8 *)EventHdr + sizeof (TCG_PCR_EVENT_HDR) > BufEnd) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 1.2 header truncated\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  EventData = (CONST UINT8 *)EventHdr + sizeof (TCG_PCR_EVENT_HDR);

  //
  // Guard against integer overflow before pointer arithmetic: EventHdr->EventSize is UINT32;
  // cap it before adding to EventData to prevent wrapping on 64-bit targets.
  //
  if (EventHdr->EventSize > PSP_DTPM_LOG_BUFFER_SIZE) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 1.2 EventSize 0x%x exceeds buffer\n", __func__, EventHdr->EventSize));
    return EFI_INVALID_PARAMETER;
  }

  if (EventData + EventHdr->EventSize > BufEnd) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 1.2 event data truncated\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  HobDataSize = sizeof (TCG_PCR_EVENT_HDR) + EventHdr->EventSize;
  HobData     = BuildGuidHob (&gTcgEventEntryHobGuid, HobDataSize);
  if (HobData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: BuildGuidHob TCG 1.2 failed\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (HobData, EventHdr, sizeof (TCG_PCR_EVENT_HDR));
  CopyMem ((UINT8 *)HobData + sizeof (TCG_PCR_EVENT_HDR), EventData, EventHdr->EventSize);

  DEBUG ((
    DEBUG_INFO,
    "%a: TCG 1.2 HOB PCR[%d] EventType=0x%x EventSize=%d\n",
    __func__,
    EventHdr->PCRIndex,
    EventHdr->EventType,
    EventHdr->EventSize
    ));

  return EFI_SUCCESS;
}

/**
  Create a gTcgEvent2EntryHobGuid HOB from a TCG 2.0 PSP log entry.

  Parses the compact binary format written by the PSP (per TCG PC Client
  Platform Firmware Profile: count | [hashAlg | digest]* | EventSize | EventData)
  directly from the raw buffer. Filters digest algorithms by PcdTpm2HashMask
  before writing the HOB in the same compact binary format expected by
  Tcg2Dxe's SetupEventLog().

  @param[in]   Entry      Pointer to the start of a TCG 2.0 compact binary entry.
  @param[in]   BufEnd     One-past-end of the valid PSP buffer region.
  @param[out]  NextEntry  Updated to point to the next entry in the PSP buffer.

  @retval EFI_SUCCESS            HOB created successfully.
  @retval EFI_INVALID_PARAMETER  Entry extends beyond BufEnd or contains invalid data.
  @retval EFI_OUT_OF_RESOURCES   HOB pool exhausted.
**/
STATIC
EFI_STATUS
CreateTcg20EntryHob (
  IN  CONST UINT8  *Entry,
  IN  CONST UINT8  *BufEnd,
  OUT CONST UINT8  **NextEntry
  )
{
  CONST UINT8    *Ptr;
  CONST UINT8    *DigestStart;
  CONST UINT8    *EventData;
  TCG_PCRINDEX   PcrIndex;
  TCG_EVENTTYPE  EventType;
  UINT32         DigestCount;
  UINT32         DigestIndex;
  UINT16         HashAlg;
  UINT32         DigestSize;
  UINT32         EventSize;
  UINT32         HashMask;
  UINT32         FilteredDigestListSize;
  UINT32         FilteredCount;
  UINTN          HobDataSize;
  VOID           *HobData;
  UINT8          *HobPtr;

  *NextEntry = NULL;
  Ptr        = Entry;

  if (Ptr + sizeof (TCG_PCRINDEX) > BufEnd) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 entry truncated at PCRIndex\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&PcrIndex, Ptr, sizeof (TCG_PCRINDEX));
  Ptr += sizeof (TCG_PCRINDEX);

  if (Ptr + sizeof (TCG_EVENTTYPE) > BufEnd) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 entry truncated at EventType\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&EventType, Ptr, sizeof (TCG_EVENTTYPE));
  Ptr += sizeof (TCG_EVENTTYPE);

  if (Ptr + sizeof (UINT32) > BufEnd) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 entry truncated at DigestCount\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&DigestCount, Ptr, sizeof (UINT32));
  Ptr += sizeof (UINT32);

  if (DigestCount > HASH_COUNT) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: DigestList count %d exceeds HASH_COUNT %d\n",
      __func__,
      DigestCount,
      HASH_COUNT
      ));
    return EFI_INVALID_PARAMETER;
  }

  //
  // First pass: walk compact binary digest entries to compute FilteredDigestListSize
  // and locate the EventSize field that immediately follows the digest list.
  // Using array indexing on a C struct (TPML_DIGEST_VALUES) would be incorrect here
  // because the PSP uses the compact binary format where each entry is
  // hashAlg (2 bytes) + actual digest bytes (variable), not the fixed-size TPMT_HA
  // slots that TPML_DIGEST_VALUES allocates.
  //
  HashMask               = PcdGet32 (PcdTpm2HashMask);
  FilteredDigestListSize = sizeof (UINT32);  // count field is always present
  FilteredCount          = 0;
  DigestStart            = Ptr;

  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    if (Ptr + sizeof (UINT16) > BufEnd) {
      DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 digest %d hashAlg truncated\n", __func__, DigestIndex));
      return EFI_INVALID_PARAMETER;
    }

    CopyMem (&HashAlg, Ptr, sizeof (UINT16));
    Ptr += sizeof (UINT16);

    DigestSize = GetHashSizeFromAlgo (HashAlg);
    if (DigestSize == 0) {
      DEBUG ((DEBUG_ERROR, "%a: unknown hashAlg 0x%x at digest %d\n", __func__, HashAlg, DigestIndex));
      return EFI_INVALID_PARAMETER;
    }

    if (Ptr + DigestSize > BufEnd) {
      DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 digest %d data truncated\n", __func__, DigestIndex));
      return EFI_INVALID_PARAMETER;
    }

    if ((GetHashMaskFromAlgo (HashAlg) & HashMask) != 0) {
      FilteredDigestListSize += sizeof (UINT16) + DigestSize;
      FilteredCount++;
    }

    Ptr += DigestSize;
  }

  if (Ptr + sizeof (UINT32) > BufEnd) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 EventSize field truncated\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&EventSize, Ptr, sizeof (UINT32));
  Ptr += sizeof (UINT32);

  //
  // Cap EventSize before pointer arithmetic to prevent wrapping on 64-bit targets.
  //
  if (EventSize > PSP_DTPM_LOG_BUFFER_SIZE) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 EventSize 0x%x exceeds buffer limit\n", __func__, EventSize));
    return EFI_INVALID_PARAMETER;
  }

  EventData = Ptr;
  if (EventData + EventSize > BufEnd) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 event data truncated\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  HobDataSize = sizeof (TCG_PCRINDEX)
                + sizeof (TCG_EVENTTYPE)
                + FilteredDigestListSize
                + sizeof (UINT32)
                + EventSize;

  HobData = BuildGuidHob (&gTcgEvent2EntryHobGuid, HobDataSize);
  if (HobData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: BuildGuidHob TCG 2.0 failed\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  HobPtr = (UINT8 *)HobData;

  CopyMem (HobPtr, &PcrIndex, sizeof (TCG_PCRINDEX));
  HobPtr += sizeof (TCG_PCRINDEX);

  CopyMem (HobPtr, &EventType, sizeof (TCG_EVENTTYPE));
  HobPtr += sizeof (TCG_EVENTTYPE);

  //
  // Second pass: write filtered compact digest list into HOB.
  // Layout: count (4 bytes) | [hashAlg (2 bytes) | digest (variable bytes)]*
  //
  CopyMem (HobPtr, &FilteredCount, sizeof (UINT32));
  HobPtr += sizeof (UINT32);

  Ptr = DigestStart;
  for (DigestIndex = 0; DigestIndex < DigestCount; DigestIndex++) {
    CopyMem (&HashAlg, Ptr, sizeof (UINT16));
    DigestSize = GetHashSizeFromAlgo (HashAlg);
    if ((GetHashMaskFromAlgo (HashAlg) & HashMask) != 0) {
      CopyMem (HobPtr, Ptr, sizeof (UINT16) + DigestSize);
      HobPtr += sizeof (UINT16) + DigestSize;
    }

    Ptr += sizeof (UINT16) + DigestSize;
  }

  CopyMem (HobPtr, &EventSize, sizeof (UINT32));
  HobPtr += sizeof (UINT32);

  CopyMem (HobPtr, EventData, EventSize);

  *NextEntry = EventData + EventSize;

  DEBUG ((
    DEBUG_INFO,
    "%a: TCG 2.0 HOB PCR[%d] EventType=0x%x FilteredDigestListSize=%d EventSize=%d\n",
    __func__,
    PcrIndex,
    EventType,
    FilteredDigestListSize,
    EventSize
    ));

  return EFI_SUCCESS;
}

/**
  PspPreTcgEventLogPei driver entry point.

  Queries PSP early-boot TPM measurements via the AmdPspDtpmPpi and deposits
  them as gTcgEventEntryHobGuid / gTcgEvent2EntryHobGuid HOBs. Returns
  EFI_SUCCESS unconditionally; all failures are logged but non-fatal.

  @param[in]  FileHandle   PEIM file handle.
  @param[in]  PeiServices  Pointer to the EFI_PEI_SERVICES table.

  @retval EFI_SUCCESS  Always.
**/
EFI_STATUS
EFIAPI
PspPreTcgEventLogPeiEntry (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS               Status;
  PSP_DTPM_PPI             *DtpmPpi;
  UINT32                   DesiredConfig;
  UINT32                   ConfigStatus;
  UINT32                   LogDataSize;
  VOID                     *LogDataBuffer;
  CONST UINT8              *BufEnd;
  CONST TCG_PCR_EVENT_HDR  *Tcg12Hdr;
  CONST UINT8              *NextEntry;
  CONST UINT8              *PrevEntry;

  DEBUG ((DEBUG_INFO, "%a Entry\n", __func__));

  LogDataBuffer = NULL;
  Status        = (*PeiServices)->LocatePpi (
                                    PeiServices,
                                    &gAmdPspDtpmPpiGuid,
                                    0,
                                    NULL,
                                    (VOID **)&DtpmPpi
                                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "%a: AmdPspDtpmPpi not found: %r\n", __func__, Status));
    goto Exit;
  }

  //
  // Allocate receive buffer from HOB pool to avoid a large stack frame in PEI.
  //
  LogDataBuffer = BuildGuidHob (&gEfiCallerIdGuid, PSP_DTPM_LOG_BUFFER_SIZE);
  if (LogDataBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate log buffer\n", __func__));
    goto Exit;
  }

  ZeroMem (LogDataBuffer, PSP_DTPM_LOG_BUFFER_SIZE);
  LogDataSize = PSP_DTPM_LOG_BUFFER_SIZE;

  Status = DtpmPpi->GetDtpmStatus (&DesiredConfig, &ConfigStatus, &LogDataSize, LogDataBuffer);

  DEBUG ((
    DEBUG_INFO,
    "%a: GetDtpmStatus=%r DesiredConfig=0x%x ConfigStatus=0x%x LogDataSize=0x%x\n",
    __func__,
    Status,
    DesiredConfig,
    ConfigStatus,
    LogDataSize
    ));

  //
  // EFI_WARN_BUFFER_TOO_SMALL means the PSP log was truncated to fit the buffer.
  // Process whatever was returned rather than discarding it.
  //
  if (EFI_ERROR (Status) && (Status != EFI_WARN_BUFFER_TOO_SMALL)) {
    DEBUG ((DEBUG_WARN, "%a: PSP mailbox error %r, no HOBs created\n", __func__, Status));
    goto Exit;
  }

  //
  // DesiredConfig == 0: PSP did not use dTPM (e.g. fTPM platform). Nothing to do.
  //
  if ((DesiredConfig == 0) || (LogDataSize == 0)) {
    DEBUG ((DEBUG_INFO, "%a: No dTPM log present\n", __func__));
    goto Exit;
  }

  //
  // PSP may write back LogDataSize larger than PSP_DTPM_LOG_BUFFER_SIZE when it
  // returns EFI_WARN_BUFFER_TOO_SMALL to indicate the full log would not fit.
  // Clamp to the actual allocation size so BufEnd never exceeds the buffer end.
  //
  if (LogDataSize > PSP_DTPM_LOG_BUFFER_SIZE) {
    DEBUG ((
      DEBUG_WARN,
      "%a: LogDataSize 0x%x exceeds buffer 0x%x, clamping\n",
      __func__,
      LogDataSize,
      PSP_DTPM_LOG_BUFFER_SIZE
      ));
    LogDataSize = PSP_DTPM_LOG_BUFFER_SIZE;
  }

  BufEnd = (CONST UINT8 *)LogDataBuffer + LogDataSize;

  //
  // Entry 0: TCG 1.2 format (first entry in PSP buffer)
  //
  Tcg12Hdr = (CONST TCG_PCR_EVENT_HDR *)LogDataBuffer;
  Status   = CreateTcg12EntryHob (Tcg12Hdr, BufEnd);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: TCG 1.2 HOB error: %r\n", __func__, Status));
    goto Exit;
  }

  //
  // Entries 1..N: TCG 2.0 compact binary format.
  // CreateTcg12EntryHob already validated that the first entry's event data
  // fits within the buffer, so computing NextEntry here is safe.
  //
  NextEntry  = (CONST UINT8 *)Tcg12Hdr + sizeof (TCG_PCR_EVENT_HDR);
  NextEntry += Tcg12Hdr->EventSize;

  while (NextEntry < BufEnd) {
    PrevEntry = NextEntry;
    Status    = CreateTcg20EntryHob (NextEntry, BufEnd, &NextEntry);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 HOB error: %r\n", __func__, Status));
      break;
    }

    //
    // Enforce forward progress: NextEntry must advance past PrevEntry on
    // every successful iteration to prevent an infinite loop on malformed
    // PSP data that produces a zero-length entry.
    //
    if (NextEntry <= PrevEntry) {
      DEBUG ((DEBUG_ERROR, "%a: TCG 2.0 entry made no progress, stopping\n", __func__));
      break;
    }
  }

Exit:
  //
  // Zero the scratch HOB used to receive the PSP log. PEI HOBs cannot be freed,
  // so zeroing prevents any consumer iterating HOBs by gEfiCallerIdGuid from
  // finding and misinterpreting the raw PSP log data.
  //
  if (LogDataBuffer != NULL) {
    ZeroMem (LogDataBuffer, PSP_DTPM_LOG_BUFFER_SIZE);
  }

  DEBUG ((DEBUG_INFO, "%a Exit, Status = %r\n", __func__, Status));

  //
  // Signal completion so Tcg2Pei (which depends on this PPI via DEPEX)
  // is dispatched only after all PSP measurement HOBs are in the HOB list.
  // Install unconditionally -- all exit paths (success, error, no dTPM)
  // must publish this PPI to avoid permanently blocking Tcg2Pei.
  //
  Status = (*PeiServices)->InstallPpi (PeiServices, &mPspPreTcgEventLogDonePpiList);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
