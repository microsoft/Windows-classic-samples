// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "RdcSdkTestServer.h"
#include "RdcFileHandleImpl.h"
#include "globals.h"


static const size_t g_InputBufferSize = 8 * 1024;
static const size_t g_OutputBufferSize = 1024;

/*+---------------------------------------------------------------------------

  Class:      RdcGeneratorJob

  Purpose:    Hold the necessary state for generating signatures
    during signature generation.

  Notes:

----------------------------------------------------------------------------*/
class RdcGeneratorJob
{
public:
    RdcGeneratorJob()
            :
            m_Failed ( false ),
            m_Depth ( 0 ),
            m_HorizonSize1 ( MSRDC_DEFAULT_HORIZONSIZE_1 ),
            m_HorizonSizeN ( MSRDC_DEFAULT_HORIZONSIZE_N ),
            m_HashWindowSize1 ( MSRDC_DEFAULT_HASHWINDOWSIZE_1 ),
            m_HashWindowSizeN ( MSRDC_DEFAULT_HASHWINDOWSIZE_N ),
            m_SimilarityEnabled ( false )
    {
        for ( ULONG i = 0; i < MSRDC_MAXIMUM_DEPTH; ++i )
        {
            m_GeneratorParameters[i] = 0;
        }

    }
    ~RdcGeneratorJob()
    {
        for ( ULONG i = 0; i < MSRDC_MAXIMUM_DEPTH; ++i )
        {
            m_GeneratorParameters[i] = 0;
        }
    }

    DebugHresult AllocateGenerator ( ULONGLONG fileSize, ULONG requestedDepth );

    DebugHresult CreateSignatures ( RdcFileHandleImpl *rdcFileHandleImpl );

    DebugHresult GetSimilarityData ( SimilarityData * similarityData );

    ULONG GetDepth()
    {
        RDCAssert ( !m_Failed );
        return m_Depth;
    }

    ULONG GetHorizonSize1()
    {
        RDCAssert ( !m_Failed );
        return m_HorizonSize1;
    }

    DebugHresult SetHorizonSize1 ( ULONG horizonSize )
    {
        RDCAssert ( !m_Failed );
        RDCAssert ( horizonSize >= MSRDC_MINIMUM_HORIZONSIZE );
        RDCAssert ( horizonSize <= MSRDC_MAXIMUM_HORIZONSIZE );
        if ( horizonSize < MSRDC_MINIMUM_HORIZONSIZE ||
                horizonSize > MSRDC_MAXIMUM_HORIZONSIZE )
        {
            return E_INVALIDARG;
        }
        m_HorizonSize1 = horizonSize;
        return S_OK;
    }

    ULONG GetHorizonSizeN()
    {
        RDCAssert ( !m_Failed );
        return m_HorizonSizeN;
    }

    DebugHresult SetHorizonSizeN ( ULONG horizonSize )
    {
        RDCAssert ( !m_Failed );
        RDCAssert ( horizonSize >= MSRDC_MINIMUM_HORIZONSIZE );
        RDCAssert ( horizonSize <= MSRDC_MAXIMUM_HORIZONSIZE );
        if ( horizonSize < MSRDC_MINIMUM_HORIZONSIZE ||
                horizonSize > MSRDC_MAXIMUM_HORIZONSIZE )
        {
            return E_INVALIDARG;
        }
        m_HorizonSizeN = horizonSize;
        return S_OK;
    }

    ULONG GetHashWindowSize1()
    {
        RDCAssert ( !m_Failed );
        return m_HashWindowSize1;
    }

    DebugHresult SetHashWindowSize1 ( ULONG hashWindowSize )
    {
        RDCAssert ( !m_Failed );
        RDCAssert ( hashWindowSize >= MSRDC_MINIMUM_HASHWINDOWSIZE );
        RDCAssert ( hashWindowSize <= MSRDC_MAXIMUM_HASHWINDOWSIZE );
        if ( hashWindowSize < MSRDC_MINIMUM_HASHWINDOWSIZE ||
                hashWindowSize > MSRDC_MAXIMUM_HASHWINDOWSIZE )
        {
            return E_INVALIDARG;
        }
        m_HashWindowSize1 = hashWindowSize;
        return S_OK;
    }

    ULONG GetHashWindowSizeN()
    {
        RDCAssert ( !m_Failed );
        return m_HashWindowSizeN;
    }

    DebugHresult SetHashWindowSizeN ( ULONG hashWindowSize )
    {
        RDCAssert ( !m_Failed );
        RDCAssert ( hashWindowSize >= MSRDC_MINIMUM_HASHWINDOWSIZE );
        RDCAssert ( hashWindowSize <= MSRDC_MAXIMUM_HASHWINDOWSIZE );
        if ( hashWindowSize < MSRDC_MINIMUM_HASHWINDOWSIZE ||
                hashWindowSize > MSRDC_MAXIMUM_HASHWINDOWSIZE )
        {
            return E_INVALIDARG;
        }
        m_HashWindowSizeN = hashWindowSize;
        return S_OK;
    }

private:
    bool m_Failed;
    ULONG m_Depth;
    ULONG m_HorizonSize1;
    ULONG m_HorizonSizeN;
    ULONG m_HashWindowSize1;
    ULONG m_HashWindowSizeN;

    // The RDC SDK object
    CComQIPtr<IRdcLibrary>           m_RdcLibrary;

    // The parameters for generation
    CComPtr<IRdcGeneratorParameters> m_RdcGeneratorParameters[MSRDC_MAXIMUM_DEPTH];

    // The generator
    CComPtr<IRdcGenerator>             m_RdcGenerator;
    CComQIPtr<IRdcSimilarityGenerator> m_RdcSimilarityGenerator;

    // Array of pointers to generation parameters
    IRdcGeneratorParameters          *m_GeneratorParameters[MSRDC_MAXIMUM_DEPTH];

    RdcSmartArray<BYTE>              m_InputBuffer;
    RdcSmartArray<BYTE>              m_OutputBuffer[MSRDC_MAXIMUM_DEPTH];

    bool m_SimilarityEnabled;
};

/*---------------------------------------------------------------------------
Name:     RdcGeneratorJob::AllocateGenerator

Allocate the resources necessary for signature generation.
This includes connecting to the RDC SDK, allocating buffers, and parameters

Arguments:
   fileSize            - used to compute RDC recursion depth.
   requestedDepth      - the preferred recursion depth, if 0, a default is calculated

----------------------------------------------------------------------------*/
DebugHresult RdcGeneratorJob::AllocateGenerator (
    ULONGLONG fileSize,
    ULONG requestedDepth )
{
    DebugHresult hr = S_OK;

    RDCAssert ( requestedDepth <= MSRDC_MAXIMUM_DEPTH );
    RDCAssert ( !m_Failed );

    m_Depth = 0;

    BYTE *buffer = m_InputBuffer.AppendItems ( g_InputBufferSize );
    if ( !buffer )
    {
        hr = E_OUTOFMEMORY;
    }

    if ( SUCCEEDED ( hr ) )
    {
        hr = m_RdcLibrary.CoCreateInstance ( __uuidof ( RdcLibrary ) );
    }

    if ( SUCCEEDED ( hr ) )
    {
        if ( requestedDepth == 0 )
        {
            hr = m_RdcLibrary->ComputeDefaultRecursionDepth ( fileSize, &m_Depth );
        }
        else
        {
            m_Depth = requestedDepth;
        }
    }

    if ( SUCCEEDED ( hr ) )
    {
        if ( m_Depth == 0 )
        {
            m_Depth = 1;
        }
        if ( m_Depth > MSRDC_MAXIMUM_DEPTH )
        {
            m_Depth = MSRDC_MAXIMUM_DEPTH;
        }
    }

    for ( ULONG i = 0; SUCCEEDED ( hr ) && i < m_Depth; ++i )
    {
        hr = m_RdcLibrary->CreateGeneratorParameters ( RDCGENTYPE_FilterMax, i + 1, &m_RdcGeneratorParameters[i] );
        m_GeneratorParameters[i] = m_RdcGeneratorParameters[i];
        if ( SUCCEEDED ( hr ) )
        {
            IRdcGeneratorFilterMaxParameters * q = 0;
            if ( i == 0 )
            {
                hr = m_RdcGeneratorParameters[0].QueryInterface ( &q );
                if ( SUCCEEDED ( hr ) )
                {
                    hr = q->SetHashWindowSize ( m_HashWindowSize1 );
                }
                if ( SUCCEEDED ( hr ) )
                {
                    hr = q->SetHorizonSize ( m_HorizonSize1 );
                }
            }
            else
            {
                hr = m_RdcGeneratorParameters[i].QueryInterface ( &q );
                if ( SUCCEEDED ( hr ) )
                {
                    hr = q->SetHashWindowSize ( m_HashWindowSizeN );
                }
                if ( SUCCEEDED ( hr ) )
                {
                    hr = q->SetHorizonSize ( m_HorizonSizeN );
                }
            }
            if ( q )
            {
                q->Release();
            }
        }
    }

    if ( SUCCEEDED ( hr ) )
    {
        hr = m_RdcLibrary->CreateGenerator ( m_Depth, &m_GeneratorParameters[0], &m_RdcGenerator );
    }

    if ( SUCCEEDED ( hr ) )
    {
        hr = m_RdcGenerator.QueryInterface ( &m_RdcSimilarityGenerator );
    }
    if ( SUCCEEDED ( hr ) )
    {
        m_SimilarityEnabled = true;
        hr = m_RdcSimilarityGenerator->EnableSimilarity();
    }
    for ( ULONG i = 0; SUCCEEDED ( hr ) && i < m_Depth; ++i )
    {
        if ( !m_OutputBuffer[i].AppendItems ( g_OutputBufferSize ) )
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if ( FAILED ( hr ) )
    {
        m_Failed = true;
    }

    return hr;
}

/*---------------------------------------------------------------------------
Name:     RdcGeneratorJob::CreateSignatures

Call the RDC SDK to generate signatures.

Arguments:
   rdcFileHandleImpl

Returns:

----------------------------------------------------------------------------*/
DebugHresult RdcGeneratorJob::CreateSignatures ( RdcFileHandleImpl *rdcFileHandleImpl )
{
    ULONGLONG totalBytesRead = 0;
    RDCAssert ( !m_Failed );

    DebugHresult hr = S_OK;

    RdcBufferPointer inputPointer =
        {
            0,
            0,
            m_InputBuffer.Begin()
        };
    RdcBufferPointer outputPointer[MSRDC_MAXIMUM_DEPTH];
    RdcBufferPointer *outputPointers[MSRDC_MAXIMUM_DEPTH] = {0};

    if ( m_Depth > MSRDC_MAXIMUM_DEPTH )
    {
        return E_FAIL;
    }

    for ( ULONG i = 0; i < m_Depth; ++i )
    {
        outputPointer[i].m_Size = static_cast<ULONG> ( m_OutputBuffer[i].Size() );
        outputPointer[i].m_Data = m_OutputBuffer[i].Begin();
        outputPointer[i].m_Used = 0;

        outputPointers[i] = &outputPointer[i];
    }

    if ( SUCCEEDED ( hr ) )
    {
        BOOL eof = FALSE;
        BOOL eofOutput = FALSE;
        do
        {
            if ( inputPointer.m_Size == inputPointer.m_Used )
            {
                if ( eof )
                {
                    inputPointer.m_Size = 0;
                    inputPointer.m_Used = 0;
                }
                else
                {
                    // When the input buffer is completely empty
                    // refill it.
                    DWORD bytesActuallyRead = 0;
                    DWORD bytesToRead = static_cast<DWORD> ( m_InputBuffer.Size() );
                    if ( !ReadFile ( rdcFileHandleImpl->GetSourceHandle(),
                                     m_InputBuffer.Begin(),
                                     bytesToRead,
                                     &bytesActuallyRead,
                                     0 ) )
                    {
                        hr = HRESULT_FROM_WIN32 ( GetLastError() );
                    }
                    else
                    {
                        totalBytesRead += bytesActuallyRead;
                        inputPointer.m_Size = bytesActuallyRead;
                        inputPointer.m_Used = 0;

                        if ( bytesActuallyRead < bytesToRead )
                        {
                            // Tell RDC there will be no more input.
                            eof = true;
                        }
                    }
                }
            }
            else
            {
                // Input buffer is not-empty. Give RDC partial
                // buffer.
                // We could shift the remaining data to the start of the buffer
                // and fill up the end - so that we always pass RDC a full buffer,
                // but it isn't worth the trouble.
                // Typically RDC will consume all the input unless the
                // output buffers are really small.
            }

            RDC_ErrorCode rdc_ErrorCode;
            if ( SUCCEEDED ( hr ) )
            {
                hr = m_RdcGenerator->Process (
                         eof,
                         &eofOutput,
                         &inputPointer,
                         m_Depth,
                         outputPointers,
                         &rdc_ErrorCode );
            }

            if ( SUCCEEDED ( hr ) )
            {
                for ( ULONG i = 0; i < m_Depth; ++i )
                {
                    DWORD bytesActuallyWritten = 0;
                    if ( !WriteFile ( rdcFileHandleImpl->GetSignatureHandle ( i + 1 ),
                                      m_OutputBuffer[i].Begin(),
                                      outputPointers[i]->m_Used,
                                      &bytesActuallyWritten,
                                      0 ) )
                    {
                        hr = HRESULT_FROM_WIN32 ( GetLastError() );
                    }
                    if ( outputPointers[i]->m_Used != bytesActuallyWritten )
                    {
                        hr = E_FAIL;
                    }

                    outputPointers[i]->m_Used = 0;
                }
            }
        }
        while ( SUCCEEDED ( hr ) && !eofOutput );
    }

    return hr;
}

DebugHresult RdcGeneratorJob::GetSimilarityData ( SimilarityData * similarityData )
{
    DebugHresult hr = S_OK;
    if ( m_SimilarityEnabled )
    {
        hr = m_RdcSimilarityGenerator->Results ( similarityData );
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

RdcFileHandleImpl::~RdcFileHandleImpl()
{
    delete m_GeneratorJobInfo;
    CloseSignatureFiles();
}

HRESULT RdcFileHandleImpl::OpenSignatureFiles ( FileCreateMode fileCreateMode, DeleteMode deleteMode )
{
    DWORD shareMode  = FILE_SHARE_READ;
    DWORD accessMode = GENERIC_READ;
    DWORD openMode   = OPEN_EXISTING;
    if ( fileCreateMode != ReadOnly )
    {
        shareMode = FILE_SHARE_DELETE;
        accessMode = GENERIC_READ | GENERIC_WRITE;
        openMode = CREATE_ALWAYS;
    }
    HRESULT hr = S_OK;
    for ( ULONG i = 0; SUCCEEDED ( hr ) && i < ARRAYSIZE ( m_SignatureFiles ); ++i )
    {
        wchar_t  sourceFileNameSig[MAX_PATH] = {0};
        MakeSignatureFileName ( i, sourceFileNameSig );

        m_SignatureFiles[i].Set ( CreateFile (
                                      sourceFileNameSig,
                                      accessMode,
                                      shareMode,
                                      0,
                                      openMode,
                                      FILE_ATTRIBUTE_NORMAL,
                                      0 ) );

        if ( !m_SignatureFiles[i].IsValid() )
        {
            hr = HRESULT_FROM_WIN32 ( GetLastError() );
            CloseSignatureFiles();
        }
        else if ( deleteMode == DeleteOnClose )
        {
            // Delete the file when it closes
            DeleteFile ( sourceFileNameSig );
        }
    }
    return hr;
}

DebugHresult RdcFileHandleImpl::CreateSignatureFiles ( bool &existing, BOOL deleteSigs )
{
    CloseSignatureFiles();

    DebugHresult hr = S_OK;

    // NOTE: We always create new signature files and have them deleted on file
    //       close. This ensures that we never use out of date signature files.
    //       A realistic application will likely want to cache signature files and
    //       have some way to ensure that they get regenerated when the file has changed.
    existing = false;

    DeleteMode dMode;
    if ( deleteSigs )
    {
        dMode = DeleteOnClose;
    }
    else
    {
        dMode = DontDelete;
    }

#if 0 // Allow for cached signature files? If enabled, this option doesn't validate validity of the cached files!
    HRESULT hr2 = OpenSignatureFiles ( ReadOnly, dMode );
    if ( SUCCEEDED ( hr2 ) )
    {
        existing = true;
    }
    else
    {
        existing = false;
        hr = OpenSignatureFiles ( CreateNew, dMode );
    }
#else
    hr = OpenSignatureFiles ( CreateNew, dMode );
#endif
    return hr;
}

DebugHresult RdcFileHandleImpl::OpenSource (
    const wchar_t *fileName,
    RdcFileTransferInfo * fileInfo,
    BOOL deleteSigs,
    ULONG horizonSize1,
    ULONG horizonSizeN,
    ULONG hashWindowSize1,
    ULONG hashWindowSizeN )
{
    DebugHresult hr = S_OK;

    if ( m_SourceFile.IsValid() )
    {
        hr = E_FAIL;
    }

    if ( SUCCEEDED ( hr ) )
    {
        wcsncpy_s ( m_SourceFileName, ARRAYSIZE ( m_SourceFileName ), fileName, _TRUNCATE );
        m_SourceFileName[ARRAYSIZE ( m_SourceFileName ) - 1] = 0;

        m_SourceFile.Set ( CreateFile (
                               m_SourceFileName,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               0,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               0 ) );

        if ( !m_SourceFile.IsValid() )
        {
            hr = HRESULT_FROM_WIN32 ( GetLastError() );
        }
    }

    if ( SUCCEEDED ( hr ) && !GetFileInformationByHandle ( m_SourceFile.GetHandle(), &m_SourceFileInformation ) )
    {
        hr = HRESULT_FROM_WIN32 ( GetLastError() );
    }

    if ( SUCCEEDED ( hr ) )
    {
        fileInfo->m_FileSize =
            ( static_cast<ULONGLONG> ( m_SourceFileInformation.nFileSizeHigh ) << 32 ) |
            m_SourceFileInformation.nFileSizeLow;

        bool existing;
        hr = CreateSignatureFiles ( existing, deleteSigs );

        if ( !existing )
        {
            if ( SUCCEEDED ( hr ) )
            {
                m_GeneratorJobInfo = new ( std::nothrow ) RdcGeneratorJob();
                if ( !m_GeneratorJobInfo )
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            if ( SUCCEEDED ( hr ) )
            {
                hr = m_GeneratorJobInfo->SetHorizonSize1 ( horizonSize1 );
            }
            if ( SUCCEEDED ( hr ) )
            {
                hr = m_GeneratorJobInfo->SetHorizonSizeN ( horizonSizeN );
            }
            if ( SUCCEEDED ( hr ) )
            {
                hr = m_GeneratorJobInfo->SetHashWindowSize1 ( hashWindowSize1 );
            }
            if ( SUCCEEDED ( hr ) )
            {
                hr = m_GeneratorJobInfo->SetHashWindowSizeN ( hashWindowSizeN );
            }
            if ( SUCCEEDED ( hr ) )
            {
                hr = m_GeneratorJobInfo->AllocateGenerator ( fileInfo->m_FileSize,
                        fileInfo->m_SignatureDepth );
            }
            if ( SUCCEEDED ( hr ) )
            {
                fileInfo->m_SignatureDepth = m_GeneratorJobInfo->GetDepth();
            }

            if ( FAILED ( hr ) )
            {
                delete m_GeneratorJobInfo;
                m_GeneratorJobInfo = 0;
            }
        }
        else
        {
            for ( fileInfo->m_SignatureDepth = 0; fileInfo->m_SignatureDepth < MSRDC_MAXIMUM_DEPTH; ++fileInfo->m_SignatureDepth )
            {
                if ( !m_SignatureFiles[fileInfo->m_SignatureDepth].IsValid() )
                {
                    break;
                }
                ULONGLONG fileSize = 0;
                hr = GetFileSize ( fileInfo->m_SignatureDepth + 1, &fileSize );
                if ( FAILED ( hr ) )
                {
                    break;
                }
                if ( fileSize == 0 )
                {
                    break;
                }
            }
        }
    }
    if ( FAILED ( hr ) )
    {
        m_SourceFile.Close();
        CloseSignatureFiles();
    }

    return hr;
}

DebugHresult RdcFileHandleImpl::GetFileSize (
    ULONG signatureLevel,
    ULONGLONG *fileSize )
{
    DebugHresult hr = S_OK;
    HANDLE h = INVALID_HANDLE_VALUE;

    *fileSize = 0;

    hr = GetHandleForLevel ( signatureLevel, &h );

    if ( SUCCEEDED ( hr ) )
    {
        LARGE_INTEGER position = {0};
        if ( !GetFileSizeEx ( h, &position ) )
        {
            hr = HRESULT_FROM_WIN32 ( GetLastError() );
        }
        else
        {
            *fileSize = position.QuadPart;
        }
    }

    return hr;
}

DebugHresult RdcFileHandleImpl::ReadData (
    ULONG signatureLevel,
    ULONGLONG fileOffset,
    ULONG bytesToRead,
    ULONG *bytesActuallyRead,
    BYTE * data )
{
    DebugHresult hr = S_OK;
    HANDLE h = INVALID_HANDLE_VALUE;

    *bytesActuallyRead = 0;

    hr = GetHandleForLevel ( signatureLevel, &h );

    if ( SUCCEEDED ( hr ) )
    {
        LARGE_INTEGER offset;
        offset.QuadPart = fileOffset;
        if ( !SetFilePointerEx (
                    h,
                    offset,
                    0,
                    FILE_BEGIN ) )
        {
            hr = HRESULT_FROM_WIN32 ( GetLastError() );
        }
    }
    if ( SUCCEEDED ( hr ) )
    {
        if ( !ReadFile (
                    h,
                    data,
                    bytesToRead,
                    bytesActuallyRead,
                    0 ) )
        {
            hr = HRESULT_FROM_WIN32 ( GetLastError() );
            *bytesActuallyRead = 0;
        }
    }

    return hr;
}

DebugHresult RdcFileHandleImpl::GetHandleForLevel ( ULONG signatureLevel, HANDLE *h )
{
    DebugHresult hr = S_OK;

    if ( signatureLevel > 0 && m_GeneratorJobInfo )
    {
        if ( signatureLevel > MSRDC_MAXIMUM_DEPTH ||
                !m_SignatureFiles[signatureLevel - 1].IsValid() )
        {
            hr = E_INVALIDARG;
        }
        if ( SUCCEEDED ( hr ) )
        {
            // ISSUE: This could be done async...
            hr = m_GeneratorJobInfo->CreateSignatures ( this );
        }
        if ( SUCCEEDED ( hr ) )
        {
            hr = m_GeneratorJobInfo->GetSimilarityData ( &m_SimilarityData );
            if ( SUCCEEDED ( hr ) )
            {
                m_SimilarityComputed = true;
            }
        }
        delete m_GeneratorJobInfo;
        m_GeneratorJobInfo = 0;
    }

    if ( SUCCEEDED ( hr ) )
    {
        *h = ( signatureLevel == 0 ) ? m_SourceFile.GetHandle() : m_SignatureFiles[signatureLevel - 1].GetHandle();
    }

    return hr;
}

DebugHresult RdcFileHandleImpl::GetSimilarityData ( SimilarityData * similarityData )
{
    if ( m_SimilarityComputed )
    {
        memcpy ( similarityData->m_Data, m_SimilarityData.m_Data, ARRAYSIZE ( m_SimilarityData.m_Data ) );
        return S_OK;
    }
    else
    {
        HANDLE h;
        // force signature/traits calculation
        DebugHresult hr = GetHandleForLevel ( MSRDC_MINIMUM_DEPTH, &h );
        if ( SUCCEEDED ( hr ) )
        {
            return GetSimilarityData ( similarityData );
        }
    }
    return E_FAIL;
}
