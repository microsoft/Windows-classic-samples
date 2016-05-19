// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "globals.h"
#include "smartFileHandle.h"
/*

    Internal fileHandle implementation.

    Handles all the details of generating signatures and
    reading the signatures and file data.

    NOT THREAD SAFE: Do not allow multiple thread access to the same file handle.

    There should be a lock held for files beign transferred.
    Currently, when creating signatures the signature files are held
    open after generating. Ideally, the signatures files would be closed
    and re-opened with FILE_SHARE_READ.

    A lock would be necessary to avoid race conditions. The lock
    would have to be cross-process -- perhaps byte-range locking would
    provide the necessary functionality.

 */
class RdcGeneratorJob;
class RdcFileHandleImpl
{
public:
    RdcFileHandleImpl() :
            m_SimilarityComputed ( false )
    {
        m_SourceFileName[0] = 0;
        m_GeneratorJobInfo = 0;
    }
    ~RdcFileHandleImpl();

    DebugHresult OpenSource (
        const wchar_t *fileName,
        RdcFileTransferInfo * fileInfo,
        BOOL deleteSigs,
        ULONG horizonSize1,
        ULONG horizonSizeN,
        ULONG hashWindowSize1,
        ULONG hashWindowSizeN );

    DebugHresult GetSimilarityData (
        SimilarityData * similarityData );

    DebugHresult CreateSignatureFiles ( bool &existing, BOOL deleteSigs );

    void CloseSignatureFiles()
    {
        for ( ULONG i = 0; i < ARRAYSIZE ( m_SignatureFiles ); ++i )
        {
            m_SignatureFiles[i].Close();
        }
    }

    HANDLE GetSourceHandle() const
    {
        return m_SourceFile.GetHandle();
    }
    HANDLE GetSignatureHandle ( ULONG depth )
    {
        RDCAssert ( depth > 0 );
        RDCAssert ( depth <= MSRDC_MAXIMUM_DEPTH );
        return m_SignatureFiles[depth - 1].GetHandle();
    }

    DebugHresult GetFileSize (
        ULONG signatureLevel,
        ULONGLONG *fileSize );

    DebugHresult ReadData (
        ULONG signatureLevel,
        ULONGLONG fileOffset,
        ULONG bytesToRead,
        ULONG *bytesActuallyRead,
        BYTE * data );
private:
    wchar_t         m_SourceFileName[MAX_PATH];
    RdcGeneratorJob *m_GeneratorJobInfo;

    SmartFileHandle m_SourceFile;
    SmartFileHandle m_SignatureFiles[MSRDC_MAXIMUM_DEPTH];
    BY_HANDLE_FILE_INFORMATION m_SourceFileInformation;

    void MakeSignatureFileName ( ULONG level, wchar_t ( &sigFileName ) [MAX_PATH] )
    {
        RDCAssert ( level <= MSRDC_MAXIMUM_DEPTH );
        int n = _snwprintf_s (
                    sigFileName,
                    ARRAYSIZE ( sigFileName ),
                    _TRUNCATE,
                    L"%s_%d.sig",
                    m_SourceFileName,
                    level + 1 );
        RDCAssert ( n != -1 );
        UNREFERENCED_PARAMETER ( n );
        sigFileName[ARRAYSIZE ( sigFileName ) - 1] = 0;
    }
    DebugHresult GetHandleForLevel ( ULONG signatureLevel, HANDLE *h );

    enum FileCreateMode
    {
        ReadOnly, CreateNew
    };
    enum DeleteMode
    {
        DontDelete, DeleteOnClose
    };
    HRESULT OpenSignatureFiles ( FileCreateMode fileCreateMode, DeleteMode deleteMode );
    bool m_SimilarityComputed;
    SimilarityData m_SimilarityData;
};
