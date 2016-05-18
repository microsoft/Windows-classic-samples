// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include <new>
#include "msrdc.h"

#include <string>
#include <memory>

#include <RdcSdkTestServer.h>

#include "smartFileHandle.h"
#include "sampleUnknownImpl.h"
#include "rdcSdkTestClient.h"
#include "simplefilenametable.h"

using namespace std;


// Size of the buffer used to copy data from source or seed file to the target file.
static const size_t g_InputBufferSize = 8192;
static const wchar_t g_similarityTableName[] = L"RdcSampleTraitsTable";
static const wchar_t g_filenameTableName[] = L"RdcSampleFilenameTable";
static const unsigned g_maxResults = 10;

static BOOL g_deleteSignatures = TRUE;
static BOOL g_reuse = FALSE;
static BOOL g_createtarget = TRUE;
static BOOL g_saveneeds = FALSE;
static BOOL g_similarity = FALSE;


static ULONG g_recursionDepth = 0;
static ULONG g_horizonSize1 = MSRDC_DEFAULT_HORIZONSIZE_1;
static ULONG g_horizonSizeN = MSRDC_DEFAULT_HORIZONSIZE_N;
static ULONG g_hashWindowSize1 = MSRDC_DEFAULT_HASHWINDOWSIZE_1;
static ULONG g_hashWindowSizeN = MSRDC_DEFAULT_HASHWINDOWSIZE_N;



/*+---------------------------------------------------------------------------

  Class:      LocalRdcFileReader

  Purpose:
  Provide the IRdcFileReader interface necessary for the MSRDC
  comparator object.

  This class simply forwards the call on to the native interfaces.

  Notes:

----------------------------------------------------------------------------*/

class LocalRdcFileReader : public IRdcFileReader
{
public:

    LocalRdcFileReader()
            : m_Handle ( INVALID_HANDLE_VALUE )
    {}

    virtual ~LocalRdcFileReader()
    {}

    void Set ( HANDLE h )
    {
        RDCAssert ( !IsValid() );
        m_Handle = h;
    }

    bool IsValid() const
    {
        return m_Handle != 0 && m_Handle != INVALID_HANDLE_VALUE;
    }

    STDMETHOD ( GetFileSize ) (
        ULONGLONG * fileSize )
    {
        RDCAssert ( IsValid() );

        DebugHresult hr = S_OK;

        *fileSize = 0;

        LARGE_INTEGER position = {0};
        if ( !GetFileSizeEx ( m_Handle, &position ) )
        {
            hr = HRESULT_FROM_WIN32 ( GetLastError() );
        }
        else
        {
            *fileSize = position.QuadPart;
        }

        return hr;
    }

    STDMETHOD ( Read ) (
        ULONGLONG offsetFileStart,
        ULONG bytesToRead,
        ULONG * bytesActuallyRead,
        BYTE * buffer,
        BOOL * eof )
    {
        RDCAssert ( IsValid() );

        DebugHresult hr = S_OK;

        *bytesActuallyRead = 0;
        *eof = FALSE;


        OVERLAPPED overlapped = {0};
        overlapped.Offset = static_cast<DWORD> ( offsetFileStart & 0xffffffff );
        overlapped.OffsetHigh = static_cast<DWORD> ( offsetFileStart >> 32 );

        if ( ReadFile ( m_Handle,
                        buffer,
                        bytesToRead,
                        bytesActuallyRead,
                        &overlapped ) )
        {
            if ( *bytesActuallyRead < bytesToRead )
            {
                // we must be at EOF
                *eof = TRUE;
            }
        }
        else
        {
            *bytesActuallyRead = 0;
            hr = HRESULT_FROM_WIN32 ( GetLastError() );
        }


        return hr;
    }


    STDMETHOD ( GetFilePosition ) ( ULONGLONG * offsetFromStart )
    {
        RDCAssert ( false );
        return E_NOTIMPL;
    }

private:
    HANDLE m_Handle;

};





/*+---------------------------------------------------------------------------

  Class:      MyFileTransferRdcFileReader

  Purpose:
  Provide the IRdcFileReader interface necessary for the MSRDC
  comparator object.
  This class simply forwards the call on to RdcSdkTestServer for reading
  the source and seed signatures.

  An instance of this object is created for each level of signatures
  to be read.

  Notes:

----------------------------------------------------------------------------*/
class MyFileTransferRdcFileReader : public IRdcFileReader
{
public:
    MyFileTransferRdcFileReader()
            : m_SignatureLevel ( ( ULONG ) - 1 )
    {}
    virtual ~MyFileTransferRdcFileReader()
    {}

    void Set (
        ULONG signatureLevel,
        IRdcFileTransfer *iRdcFileTransfer,
        RdcFileHandle rdcFileHandle )
    {
        // Only allow a single initialization
        RDCAssert ( m_SignatureLevel == ( ULONG ) - 1 );

        m_SignatureLevel = signatureLevel;
        m_RdcFileTransfer = iRdcFileTransfer;
        m_RdcFileHandle = rdcFileHandle;
    }

    STDMETHOD ( GetFileSize ) (
        ULONGLONG * fileSize )
    {
        // Assert that we have been initialized
        RDCAssert ( m_SignatureLevel != ( ULONG ) - 1 );
        if ( m_SignatureLevel == ( ULONG ) - 1 )
        {
            return E_FAIL;
        }

        DebugHresult hr = m_RdcFileTransfer->GetFileSize (
                              &m_RdcFileHandle,
                              m_SignatureLevel,
                              fileSize );

        return hr;
    }

    STDMETHOD ( Read ) (
        ULONGLONG offsetFileStart,
        ULONG bytesToRead,
        ULONG * bytesActuallyRead,
        BYTE * buffer,
        BOOL * eof )
    {
        // Assert that we have been initialized
        RDCAssert ( m_SignatureLevel != ( ULONG ) - 1 );
        if ( m_SignatureLevel == ( ULONG ) - 1 )
        {
            return E_FAIL;
        }
        *eof = FALSE;

        DebugHresult hr = m_RdcFileTransfer->ReadData (
                              &m_RdcFileHandle,
                              m_SignatureLevel,
                              offsetFileStart,
                              bytesToRead,
                              bytesActuallyRead,
                              buffer );

        if ( SUCCEEDED ( hr ) )
        {
            if ( *bytesActuallyRead < bytesToRead )
            {
                *eof = TRUE;
            }
        }
        return hr;
    }

    STDMETHOD ( GetFilePosition ) ( ULONGLONG * offsetFromStart )
    {
        RDCAssert ( false );
        return E_NOTIMPL;
    }
private:
    ULONG m_SignatureLevel;
    CComPtr<IRdcFileTransfer> m_RdcFileTransfer;
    RdcFileHandle m_RdcFileHandle;
};

/*+---------------------------------------------------------------------------

  Class:      SignatureFileInfo

  Purpose:
    Used to call into the RdcSdkTestServer and maintain the state
    information necessary for each open file.
    In this sample, there are 2 open files: the source and the seed.

  Notes:

----------------------------------------------------------------------------*/
class SignatureFileInfo
{
public:
    SignatureFileInfo()
    {
        Clear();
    }
    DebugHresult OpenRemoteFile (
        wchar_t const *sourceMachineName,
        wchar_t const *sourceFileName,
        ULONG recursionDepth );
    DebugHresult OpenLocalFile (
        wchar_t const *sourceFileName,
        ULONG recursionDepth )
    {
        return OpenRemoteFile ( 0, sourceFileName, recursionDepth );
    }

    DebugHresult GetSimilarityData ( SimilarityData *similarityData )
    {
        if ( m_RdcFileTransfer )
        {
            return m_RdcFileTransfer->GetSimilarityData ( similarityData );
        }
        return E_FAIL;
    }

    /*----------------------------------------------------------------------------
      Name:   CreateSignatureFileReader

        Create a MyFileTransferRdcFileReader() to provide the input to the
        MSRDC Comparator.

      Arguments:
       signatureLevel        Which signature level will be read
       reader                The resulting interface.

      Returns:

    ----------------------------------------------------------------------------*/
    DebugHresult CreateSignatureFileReader ( ULONG signatureLevel, IRdcFileReader **reader )
    {
        DebugHresult hr = S_OK;

        RDCAssert ( signatureLevel <= MSRDC_MAXIMUM_DEPTH );
        *reader = 0;

        MyFileTransferRdcFileReader *p = new ( std::nothrow ) SampleUnknownImpl<MyFileTransferRdcFileReader, IRdcFileReader>();

        if ( p )
        {
            p->Set (
                signatureLevel,
                m_RdcFileTransfer,
                m_RdcFileHandle );

            *reader = p;
            p->AddRef();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        return hr;
    }

    ULONG getRdcSignatureDepth() const
    {
        return m_RdcFileTransferInfo.m_SignatureDepth;
    }
    ULONGLONG GetFileSize() const
    {
        return m_RdcFileTransferInfo.m_FileSize;
    }
private:
    wchar_t m_Filename[ MAX_PATH ];
    RdcFileTransferInfo m_RdcFileTransferInfo;
    RdcFileHandle m_RdcFileHandle;
    CComPtr<IRdcFileTransfer> m_RdcFileTransfer;

    void Clear()
    {
        memset ( m_Filename, 0, sizeof ( m_Filename ) );
        memset ( &m_RdcFileTransferInfo, 0, sizeof ( m_RdcFileTransferInfo ) );
        memset ( &m_RdcFileHandle, 0, sizeof ( m_RdcFileHandle ) );
    }
};

/*---------------------------------------------------------------------------
Name:     SignatureFileInfo::OpenRemoteFile

Connect to RdcSdkTestServer remotely, or locally if the server name
is not provided.

Arguments:
   sourceMachineName        The remote machine name, or NULL.
   sourceFileName           The filename. Must be of the form C:\fullpath\filename
   recursionDepth           The signature depth to generate.

Returns:

----------------------------------------------------------------------------*/
DebugHresult SignatureFileInfo::OpenRemoteFile (
    wchar_t const *sourceMachineName,
    wchar_t const *sourceFileName,
    ULONG recursionDepth )
{
    DebugHresult hr = S_OK;

    if ( wcslen ( sourceFileName ) >= ARRAYSIZE ( m_Filename ) )
    {
        // Path is too long.
        return E_INVALIDARG;
    }
    wcsncpy_s ( m_Filename, ARRAYSIZE ( m_Filename ), sourceFileName, _TRUNCATE );

    m_Filename[ ARRAYSIZE ( m_Filename ) - 1 ] = 0;

    COSERVERINFO serverInfo = {
                                  0,
                                  ( LPWSTR ) sourceMachineName,
                                  0,
                                  0
                              };
    MULTI_QI mQI = {
                       &__uuidof ( IRdcFileTransfer ),
                       0,
                       S_OK
                   };
    hr = CoCreateInstanceEx (
             __uuidof ( RdcFileTransfer ),
             0,
             sourceMachineName ? CLSCTX_REMOTE_SERVER : ( CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER ),
             &serverInfo,
             1,
             &mQI
         );

    if ( SUCCEEDED ( hr ) )
    {
        m_RdcFileTransfer.Attach ( ( IRdcFileTransfer* ) mQI.pItf );
        m_RdcFileTransferInfo.m_SignatureDepth = recursionDepth;
        hr = m_RdcFileTransfer->RdcOpenFile ( m_Filename,
                                              &m_RdcFileTransferInfo,
                                              &m_RdcFileHandle,
                                              g_deleteSignatures,
                                              g_horizonSize1,
                                              g_horizonSizeN,
                                              g_hashWindowSize1,
                                              g_hashWindowSizeN );
    }
    return hr;
}

/*----------------------------------------------------------------------------
  Name:   CopyDataToTarget

Copy data from either the source or seed file to the target file.

  Arguments:
   fromFile           file reader for the source or seed file.
   fileOffset         Offset into the source or seed file.
   blockLength        Length of the block to be copied
   targetFile         Handle to the target file.

----------------------------------------------------------------------------*/
DebugHresult CopyDataToTarget (
    IRdcFileReader *fromFile,
    ULONGLONG fileOffset,
    ULONGLONG blockLength,
    HANDLE targetFile )
{
    DebugHresult hr = S_OK;

    if ( targetFile == INVALID_HANDLE_VALUE || targetFile == 0 )
    {
        return hr;
    }

    // Copy a buffer full of data at a time.
    BYTE buffer[ g_InputBufferSize ];

    BOOL eof = FALSE;
    while ( blockLength && !eof && SUCCEEDED ( hr ) )
    {
        ULONG length = static_cast<ULONG> ( min ( blockLength, sizeof ( buffer ) ) );

        ULONG bytesActuallyRead = 0;

        hr = fromFile->Read ( fileOffset,
                              length,
                              &bytesActuallyRead,
                              buffer,
                              &eof );

        if ( length != bytesActuallyRead )
        {
            printf ( "Read failed x%x, length=%d, bytesActuallyRead=%d\n", hr, length, bytesActuallyRead );
        }

        if ( SUCCEEDED ( hr ) )
        {
            ULONG bytesActuallyWritten = 0;
            if ( !WriteFile ( targetFile,
                              buffer,
                              bytesActuallyRead,
                              &bytesActuallyWritten,
                              0 ) )
            {
                hr = HRESULT_FROM_WIN32 ( GetLastError() );
            }
            if ( bytesActuallyRead != bytesActuallyWritten )
            {
                printf ( "Oops, write failed, error = x%x, bytesActuallyRead=%d, bytesActuallyWritten=%d\n", hr, bytesActuallyRead, bytesActuallyWritten );
            }

            fileOffset += bytesActuallyRead;
            blockLength -= bytesActuallyRead;

            RDCAssert ( eof || bytesActuallyRead == length );
        }
    }
    if ( eof )
    {
        printf ( "FAILED: EOF reached\n" );
    }

    return hr;
}

void CheckPosition ( ULONGLONG offset, HANDLE file )
{
    LARGE_INTEGER zero = {0};
    LARGE_INTEGER newPos;
    if ( SetFilePointerEx ( file, zero, &newPos, FILE_CURRENT ) )
    {
        if ( static_cast<ULONGLONG> ( newPos.QuadPart ) != offset )
        {
            printf ( "Target file position incorrect, is %I64d, should be %I64d, diff=%I64d\n",
                     newPos.QuadPart,
                     offset,
                     offset - newPos.QuadPart );
        }
    }
}

void sPrintTraits ( string &str, const SimilarityData & traits )
{
    static const int bufferSize = 100;
    char hexBuffer[ bufferSize ];
    str.clear();
    for ( int i = 0; i < ARRAYSIZE ( traits.m_Data ); ++i )
    {
        if ( i != 0 )
            str.append ( "-" );
        sprintf_s ( hexBuffer, ARRAYSIZE ( hexBuffer ), "%02X", traits.m_Data[ i ] );
        str.append ( hexBuffer );
    }
}

void MakeSignatureFileName ( ULONG level, wchar_t ( &sigFileName ) [ MAX_PATH ], wchar_t const *sourceFileName )
{
    RDCAssert ( level <= MSRDC_MAXIMUM_DEPTH );
    if ( level > 0 )
    {
        int n = _snwprintf_s (
                    sigFileName,
                    ARRAYSIZE ( sigFileName ),
                    _TRUNCATE,
                    L"%s_%d.sig",
                    sourceFileName,
                    level );

        RDCAssert ( n != -1 );
        UNREFERENCED_PARAMETER ( n );
    }
    else
    {
        wcsncpy_s ( sigFileName, ARRAYSIZE ( sigFileName ), sourceFileName, _TRUNCATE );

    }
    sigFileName[ ARRAYSIZE ( sigFileName ) - 1 ] = 0;
}

/*----------------------------------------------------------------------------
  Name:   CompareFiles

    Creates and uses an MSRDC Comparator object to compare the given
    signature files and constructs the target file.

    On error, the target file should be deleted by the caller.

  Arguments:
   seedSignatures   file reader for the seed signatures. (Connected to local RdcSdkTestServer)
   seedFile         file reader for the seed file. (Connected to local RdcSdkTestServer)
   sourceSignatures file reader for the source signatures. (Connected to remote RdcSdkTestServer)
   sourceFile       file reader for the source file. (Connected to remote RdcSdkTestServer)
   targetFile       NT file handle for the target.
   bytesFromSource
   bytesFromSeed

----------------------------------------------------------------------------*/
DebugHresult CompareFiles (
    IRdcFileReader *seedSignatures,
    IRdcFileReader *seedFile,
    IRdcFileReader *sourceSignatures,
    IRdcFileReader *sourceFile,
    HANDLE targetFile,
    HANDLE needsFile,
    ULONGLONG *bytesFromSource,
    ULONGLONG *bytesToTarget,
    ULONGLONG *bytesFromSeed )
{
    ULONGLONG totalSeedData = 0;
    ULONGLONG totalSourceData = 0;
    ULONGLONG totalSourceSignature = 0;
    ULONGLONG targetOffset = 0;
    ULONG needsCount = 0;
    CComQIPtr<IRdcLibrary> rdcLibrary;
    CComPtr<IRdcComparator> rdcComparator;

    // Load the MSRDC inproc
    DebugHresult hr = rdcLibrary.CoCreateInstance ( __uuidof ( RdcLibrary ), 0, CLSCTX_INPROC_SERVER );

    if ( SUCCEEDED ( hr ) )
    {
        hr = rdcLibrary->CreateComparator ( seedSignatures, 1000000 /*MSRDC_MINIMUM_COMPAREBUFFER */, &rdcComparator );
    }

    if ( SUCCEEDED ( hr ) )
    {
        RDC_ErrorCode rdc_ErrorCode = RDC_NoError;
        BOOL eof = false;
        BOOL eofOutput = false;
        DebugHresult hr = S_OK;

        // Buffer for streaming remote signatures into the comparator.
        BYTE inputBuffer[ g_InputBufferSize ];
        RdcNeed needsArray[ 256 ];

        RdcBufferPointer inputPointer = {0};
        RdcNeedPointer outputPointer = {0};

        ULONGLONG signatureFileOffset = 0;
        while ( SUCCEEDED ( hr ) && !eofOutput )
        {
            // When the inputPointer is empty, (or first time in this loop)
            // fill it from the source signature file reader.
            if ( inputPointer.m_Size == inputPointer.m_Used && !eof )
            {
                ULONG bytesActuallyRead = 0;
                hr = sourceSignatures->Read (
                         signatureFileOffset,
                         sizeof ( inputBuffer ),
                         &bytesActuallyRead,
                         inputBuffer,
                         &eof );

                // eof is set to true when Read() has read the last byte of
                // the source signatures. We pass this flag along to the
                // Comparator. The comparitor will never set eofOutput until
                // after eof has been set.
                if ( SUCCEEDED ( hr ) )
                {
                    inputPointer.m_Data = inputBuffer;
                    inputPointer.m_Size = bytesActuallyRead;
                    inputPointer.m_Used = 0;

                    signatureFileOffset += bytesActuallyRead;
                    totalSourceSignature += bytesActuallyRead;
                }
            }

            // Initialize the output needs array
            outputPointer.m_Size = ARRAYSIZE ( needsArray );
            outputPointer.m_Used = 0;
            outputPointer.m_Data = needsArray;

            if ( SUCCEEDED ( hr ) )
            {
                // Do the comparison operation.
                // This function may not produce needs output every time.
                // Also, it may not use all available input each time either.
                // You may call it with any number of input bytes
                // and output needs array entries. Obviously, it is more
                // efficient to given it reasonably sized buffers for each.
                // This sample waits until Process() consumes an entire input
                // buffer before reading more data from the source signatures file.
                // Continue calling this function until it sets "eofOutput" to true.
                hr = rdcComparator->Process (
                         eof,
                         &eofOutput,
                         &inputPointer,
                         &outputPointer,
                         &rdc_ErrorCode );
            }
            if ( SUCCEEDED ( hr ) )
            {
                // Empty the needs array completely.
                for ( ULONG i = 0; i < outputPointer.m_Used; ++i )
                {
                    switch ( needsArray[ i ].m_BlockType )
                    {
                    case RDCNEED_SOURCE:
                        totalSourceData += needsArray[ i ].m_BlockLength;
                        hr = CopyDataToTarget (
                                 sourceFile,
                                 needsArray[ i ].m_FileOffset,
                                 needsArray[ i ].m_BlockLength,
                                 targetFile );
                        break;
                    case RDCNEED_SEED:
                        totalSeedData += needsArray[ i ].m_BlockLength;
                        hr = CopyDataToTarget (
                                 seedFile,
                                 needsArray[ i ].m_FileOffset,
                                 needsArray[ i ].m_BlockLength,
                                 targetFile );
                        break;
                    default:
                        hr = E_FAIL;
                    }
                    char needsBuffer[ 256 ];

                    int n = _snprintf_s ( needsBuffer, ARRAYSIZE ( needsBuffer ), ARRAYSIZE ( needsBuffer ),
                                          "NEED: length:%12I64d"
                                          "\toffset:%12I64d"
                                          "\tsource:%12I64d"
                                          "\tblock type:%s\n",
                                          needsArray[ i ].m_BlockLength,
                                          needsArray[ i ].m_FileOffset,
                                          targetOffset,
                                          needsArray[ i ].m_BlockType == RDCNEED_SOURCE ? "source" : "seed" );

                    needsBuffer[ ARRAYSIZE ( needsBuffer ) - 1 ] = 0;

                    if ( needsFile != INVALID_HANDLE_VALUE && needsFile != 0 )
                    {
                        DWORD written;
                        WriteFile ( needsFile, needsBuffer, n, &written, 0 );
                    }
                    else
                    {
                        fputs ( needsBuffer, stdout );
                    }

                    targetOffset += needsArray[ i ].m_BlockLength;
                    CheckPosition ( targetOffset, targetFile );
                    ++needsCount;
                }
            }
        }
    }
    if ( FAILED ( hr ) )
    {
        printf ( "Comparison FAILED, error = x%x\n", hr );
    }
    *bytesFromSource = totalSourceData + totalSourceSignature;
    *bytesToTarget = totalSourceData + totalSeedData;
    *bytesFromSeed = totalSeedData;
    printf ( "Compare: Total Seed Bytes: %I64d, Total Source Bytes: %I64d\n",
             totalSeedData,
             *bytesFromSource );

    return hr;
}

void openTraitsTable ( CComQIPtr<ISimilarityTraitsTable> &similarityTraitsTable )
{
    DebugHresult hr;
    BYTE *securityDescriptor = 0;
    RdcCreatedTables tableState;
    hr = similarityTraitsTable.CoCreateInstance ( __uuidof ( SimilarityTraitsTable ) );
    similarityTraitsTable->CreateTable ( const_cast<wchar_t*> ( g_similarityTableName ), FALSE, securityDescriptor, &tableState );
    string tableStateString;
    if ( RDCTABLE_New == tableState )
    {
        tableStateString = "new";
    }
    else if ( RDCTABLE_Existing == tableState )
    {
        tableStateString = "existing";
    }
    else
    {
        tableStateString = "unknown or invalid";
        RDCAssert ( false );
    }
    printf ( "similarity traits table \"%S\" state is: %s\n", g_similarityTableName, tableStateString.c_str() );
}

void printSimilarityResults ( FindSimilarFileIndexResults *results, const DWORD &used, SimpleFilenameTable &t )
{
    printf ( "%u similarity results were found:\n\n", used );
    for ( UINT i = 0; i < used; ++i )
    {
        wstring s;
        t.lookup ( results[ i ].m_FileIndex, s );
        printf ( "\t%u traits matches\t\"%S\"\n", results[ i ].m_MatchCount, s.c_str() );
    }
    printf ( "\n", used );
}


/*----------------------------------------------------------------------------
  Name:   Transfer

Transfer a file from a remote host to the local machine.

  Arguments:
   remoteMachineName
   remoteFilename
   localFilename
   targetFilename

----------------------------------------------------------------------------*/
DebugHresult Transfer (
    wchar_t const *remoteMachineName,
    wchar_t const *remoteFilename,
    wchar_t const *localFilename,
    wchar_t const *targetFilename )
{
    DebugHresult hr = S_OK;

    SignatureFileInfo remoteFile;

    // Connect to RDCSDKTestServer on the remote host
    hr = remoteFile.OpenRemoteFile ( remoteMachineName, remoteFilename, g_recursionDepth );

    ULONG remoteDepth = 0;
    if ( SUCCEEDED ( hr ) )
    {
        remoteDepth = remoteFile.getRdcSignatureDepth();
    }

    SimilarityData sourceTraits;
    remoteFile.GetSimilarityData ( &sourceTraits );

    SignatureFileInfo localFile;
    CComQIPtr<ISimilarityTraitsTable> similarityTraitsTable;
    auto_ptr<SimpleFilenameTable> filenameTable;
    wstring localFilenameStr;
    if ( g_similarity )
    {
        openTraitsTable ( similarityTraitsTable );
        filenameTable.reset ( new SimpleFilenameTable() );
        filenameTable->deserialize ( const_cast<wchar_t*> ( g_filenameTableName ) );


        FindSimilarFileIndexResults results[ g_maxResults ] = {0};
        DWORD used = 0;
        hr = similarityTraitsTable->FindSimilarFileIndex ( &sourceTraits, MSRDC_MINIMUM_MATCHESREQUIRED, results, ARRAYSIZE ( results ), &used );
        if ( used > 0 )
        {
            printSimilarityResults ( results, used, *filenameTable );
            filenameTable->lookup ( results[ 0 ].m_FileIndex, localFilenameStr );
            printf ( "Using first entry for similarity.\n" );
            localFilename = localFilenameStr.c_str();
        }
        else
        {
            printf ( "Similarity was requested, but no matches were found.\n" );
        }
    }
    printf ( "\nseed name is \"%S\"\n", localFilename );

    if ( SUCCEEDED ( hr ) )
    {
        // Connect to RDCSDKTestServer on the local host
        hr = localFile.OpenLocalFile ( localFilename, remoteDepth );
    }

    ULONG localDepth = 0;

    if ( SUCCEEDED ( hr ) )
    {
        localDepth = localFile.getRdcSignatureDepth();
    }

    SmartFileHandle localSourceSHandle;
    SmartFileHandle targetFile;

    wchar_t sourceSigName[ MAX_PATH ] = {0};
    wchar_t needsName[ MAX_PATH + 6 ] = {0}; // +".Needs"

    CComPtr<IRdcFileReader> seedSignatures;
    CComPtr<IRdcFileReader> sourceSignatures;
    CComPtr<IRdcFileReader> seedFile;
    CComPtr<IRdcFileReader> sourceFile;


    if ( SUCCEEDED ( hr ) )
    {
        hr = remoteFile.CreateSignatureFileReader ( remoteDepth, &sourceSignatures );
    }

    ULONGLONG bytesFromSourceTotal = 0;
    ULONGLONG bytesToTargetTotal = 0;

    for ( ULONG i = remoteDepth; i > 0 && SUCCEEDED ( hr ); --i )
    {
        MakeSignatureFileName ( i - 1, sourceSigName, targetFilename );
        wcsncpy_s ( needsName, ARRAYSIZE ( needsName ), sourceSigName, _TRUNCATE );
        wcscat_s ( needsName, ARRAYSIZE ( needsName ), L".needs" );

        if ( g_createtarget || i > 1 )
        {
            // always create target if target is signature file
            targetFile.Set ( CreateFile ( sourceSigName,
                                          GENERIC_WRITE,
                                          FILE_SHARE_DELETE,
                                          0,
                                          CREATE_NEW,
                                          FILE_ATTRIBUTE_NORMAL,
                                          0 ) );

            if ( !targetFile.IsValid() )
            {
                hr = HRESULT_FROM_WIN32 ( GetLastError() );
            }
        }
        SmartFileHandle needsFile;
        if ( g_saveneeds )
        {
            // always create target if target is signature file
            needsFile.Set ( CreateFile ( needsName,
                                         GENERIC_WRITE,
                                         FILE_SHARE_DELETE,
                                         0,
                                         CREATE_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,
                                         0 ) );

            if ( !needsFile.IsValid() )
            {
                hr = HRESULT_FROM_WIN32 ( GetLastError() );
            }
        }
        if ( SUCCEEDED ( hr ) )
        {
            hr = localFile.CreateSignatureFileReader ( i, &seedSignatures );
        }
        if ( SUCCEEDED ( hr ) )
        {
            hr = localFile.CreateSignatureFileReader ( i - 1, &seedFile );
        }
        if ( SUCCEEDED ( hr ) )
        {
            hr = remoteFile.CreateSignatureFileReader ( i - 1, &sourceFile );
        }

        if ( SUCCEEDED ( hr ) )
        {
            wprintf ( L"\n\n----------\nTransferring: %s\n----------\n\n",
                      sourceSigName );

            ULONGLONG bytesFromSource = 0;
            ULONGLONG bytesToTarget = 0;
            ULONGLONG bytesFromSeed = 0;
            hr = CompareFiles (
                     seedSignatures,
                     seedFile,
                     sourceSignatures,
                     sourceFile,
                     targetFile.GetHandle(),
                     needsFile.GetHandle(),
                     &bytesFromSource,
                     &bytesToTarget,
                     &bytesFromSeed );

            bytesFromSourceTotal += bytesFromSource;
            bytesToTargetTotal += bytesToTarget;
            if ( targetFile.IsValid() )
            {
                if ( FAILED ( hr ) )
                {
                    DeleteFile ( sourceSigName );
                }
            }
        }

        targetFile.Close();
        localSourceSHandle.Close();

        if ( i > 1 )
        {
            localSourceSHandle.Set ( CreateFile ( sourceSigName,
                                                  GENERIC_READ,
                                                  FILE_SHARE_READ,
                                                  0,
                                                  OPEN_EXISTING,
                                                  FILE_ATTRIBUTE_NORMAL,
                                                  0 ) );

            if ( !localSourceSHandle.IsValid() )
            {
                hr = HRESULT_FROM_WIN32 ( GetLastError() );
            }

            if ( SUCCEEDED ( hr ) )
            {
                sourceSignatures.Release();
                LocalRdcFileReader *r = new ( std::nothrow ) SampleUnknownImpl<LocalRdcFileReader, IRdcFileReader>();
                r->Set ( localSourceSHandle.GetHandle() );
                r->AddRef();
                sourceSignatures.Attach ( r );
            }
        }
        if ( g_deleteSignatures )
        {
            MakeSignatureFileName ( i, sourceSigName, targetFilename );
            DeleteFile ( sourceSigName );
        }

        sourceFile.Release();
        seedSignatures.Release();
        seedFile.Release();
    }

    if ( SUCCEEDED ( hr ) )
    {
        ULONGLONG fileSize = remoteFile.GetFileSize();

        double savings = 0;
        if ( bytesFromSourceTotal > fileSize || fileSize == 0 )
        {
            savings = 0;
        }
        else
        {
            savings = ( double ) ( fileSize - bytesFromSourceTotal ) / fileSize * 100;
        }

        printf ( "\n\n----------------------------------------" );
        printf ( "\nTransfer : %I64d bytes from source, file size: %I64d, RDC Savings: %2.2f%%, toTargetTotal=%I64d bytes\n",
                 bytesFromSourceTotal,
                 fileSize,
                 savings,
                 bytesToTargetTotal
               );
        printf ( "\n\n----------------------------------------\n\n" );
    }

    if ( SUCCEEDED ( hr ) )
    {
        string traitsStr;
        sPrintTraits ( traitsStr, sourceTraits );
        printf ( "target trait: %s\n", traitsStr.c_str() );
        if ( !similarityTraitsTable )
        {
            openTraitsTable ( similarityTraitsTable );
        }
        SimilarityFileIndexT last;
        hr = similarityTraitsTable->GetLastIndex ( &last );
        hr = similarityTraitsTable->Append ( &sourceTraits, ++last );
        hr = similarityTraitsTable->CloseTable ( TRUE );
        if ( !filenameTable.get() )
        {
            filenameTable.reset ( new SimpleFilenameTable() );
            filenameTable->deserialize ( const_cast<wchar_t*> ( g_filenameTableName ) );
        }
        filenameTable->insert ( last, targetFilename );
        filenameTable->serialize ( const_cast<wchar_t*> ( g_filenameTableName ) );
    }
    return hr;
}

DebugHresult CheckVersion()
{
    CComQIPtr<IRdcLibrary> rdcLibrary;

    ULONG currentVersion = 0;
    ULONG minimumCompatibleAppVersion = 0;

    // Load the MSRDC inproc
    DebugHresult hr = rdcLibrary.CoCreateInstance ( __uuidof ( RdcLibrary ), 0, CLSCTX_INPROC_SERVER );

    if ( SUCCEEDED ( hr ) )
    {
        hr = rdcLibrary->GetRDCVersion (
                 &currentVersion,
                 &minimumCompatibleAppVersion );
    }

    if ( SUCCEEDED ( hr ) )
    {
        if ( currentVersion < MSRDC_MINIMUM_COMPATIBLE_APP_VERSION )
        {
            printf ( "The library is older (%d.%d) that I can accept (%d.%d)\n",
                     HIWORD ( currentVersion ),
                     LOWORD ( currentVersion ),
                     HIWORD ( MSRDC_MINIMUM_COMPATIBLE_APP_VERSION ),
                     LOWORD ( MSRDC_MINIMUM_COMPATIBLE_APP_VERSION ) );

            hr = E_FAIL;
        }
        else if ( minimumCompatibleAppVersion > MSRDC_VERSION )
        {
            printf ( "The library is newer (%d.%d) that I can accept (%d.%d)\n",
                     HIWORD ( minimumCompatibleAppVersion ),
                     LOWORD ( minimumCompatibleAppVersion ),
                     HIWORD ( MSRDC_VERSION ),
                     LOWORD ( MSRDC_VERSION ) );

            hr = E_FAIL;
        }
    }

    return hr;
}

void Usage ( int argc, wchar_t * argv[] )
{
    const wchar_t * p = wcschr ( argv[ 0 ], L'\\' );
    if ( p )
    {
        ++p;
    }
    else
    {
        p = argv[ 0 ];
    }
    char const str[] = "Usage: %ws <optional parameters> -c remoteMachine remoteFile(source) localFile(seed) targetfile\n\n"
                       "\t-k\t\tkeep signature files.\n"
                       //        "\t-reuse\t\tkeep re-use signature files.\n"
                       "\t-notarget\tdon't build the target -- compare only\n"
                       "\t-saveneeds\tsave the needs information to targetfile.needs\n"
                       "\t-similarity\tuse similarity to override seed selection\n"
                       "\t-r <depth>\trecursion depth\n"
                       "\t-hs1 <size>\t1st recursion level horizon size min=%d max=%d default=%d\n"
                       "\t-hsn <size>\t2+ recursion level horizon size min=%d max=%d default=%d\n"
                       "\t-hws1 <size>\t1st recursion level hash window size min=%d max=%d default=%d\n"
                       "\t-hwsn <size>\t2+ recursion level hash window size min=%d max=%d default=%d\n";
    printf ( str,
             p,
             MSRDC_MINIMUM_HORIZONSIZE,
             MSRDC_MAXIMUM_HORIZONSIZE,
             MSRDC_DEFAULT_HORIZONSIZE_1,
             MSRDC_MINIMUM_HORIZONSIZE,
             MSRDC_MAXIMUM_HORIZONSIZE,
             MSRDC_DEFAULT_HORIZONSIZE_N,
             MSRDC_MINIMUM_HASHWINDOWSIZE,
             MSRDC_MAXIMUM_HASHWINDOWSIZE,
             MSRDC_DEFAULT_HASHWINDOWSIZE_1,
             MSRDC_MINIMUM_HASHWINDOWSIZE,
             MSRDC_MAXIMUM_HASHWINDOWSIZE,
             MSRDC_DEFAULT_HASHWINDOWSIZE_N
           );
}

int __cdecl wmain ( int argc, wchar_t * argv[] )
{
    wchar_t const * args[ 4 ];

    CoInitWrapper coInit;

    DebugHresult hr = S_OK;

    int i;
    for ( i = 1; i < argc; ++i )
    {
        if ( argv[ i ][ 0 ] != L'-' )
        {
            Usage ( argc, argv );
            exit ( 1 );
        }
        else if ( argv[ i ][ 1 ] == L'k' )
        {
            g_deleteSignatures = FALSE;
        }
        else if ( _wcsicmp ( &argv[ i ][ 1 ], L"reuse" ) == 0 )
        {
            g_reuse = TRUE;
        }
        else if ( _wcsicmp ( &argv[ i ][ 1 ], L"notarget" ) == 0 )
        {
            g_createtarget = FALSE;
        }
        else if ( _wcsicmp ( &argv[ i ][ 1 ], L"saveneeds" ) == 0 )
        {
            g_saveneeds = TRUE;
        }
        else if ( _wcsicmp ( &argv[ i ][ 1 ], L"similarity" ) == 0 )
        {
            g_similarity = TRUE;
        }
        else if ( argv[ i ][ 1 ] == L'c' )
        {
            ++i;
            break;
        }
        else if ( argv[ i ][ 1 ] == L'r' )
        {
            wchar_t * p = argv[ ++i ];
            ULONG t = wcstoul ( p, &p, 10 );
            if ( t > MSRDC_MAXIMUM_DEPTH )
            {
                printf ( "\n\nRequested recursion depth -- \"%i\" is greater than the maximum MSRDC recursion depth (%i).\n\n", t, MSRDC_MAXIMUM_DEPTH );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else if ( t < MSRDC_MINIMUM_DEPTH )
            {
                printf ( "\nRequested recursion depth -- \"%i\" is less than the minimum MSRDC recursion depth (%i).\n\n", t, MSRDC_MINIMUM_DEPTH );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else
            {
                g_recursionDepth = t;
            }
        }
        else if ( wcscmp ( argv[ i ] + 1, L"hs1" ) == 0 )
        {
            wchar_t * p = argv[ ++i ];
            ULONG t = wcstoul ( p, &p, 10 );
            if ( t > MSRDC_MAXIMUM_HORIZONSIZE )
            {
                printf ( "\n\nRequested horizon size -- \"%i\" is greater than the maximum MSRDC horizon size (%i).\n\n", t, MSRDC_MAXIMUM_HORIZONSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else if ( t < MSRDC_MINIMUM_HORIZONSIZE )
            {
                printf ( "\n\nRequested horizon size -- \"%i\" is less than the minimum MSRDC horizon size (%i).\n\n", t, MSRDC_MINIMUM_HORIZONSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else
            {
                g_horizonSize1 = t;
            }

        }
        else if ( wcscmp ( argv[ i ] + 1, L"hsn" ) == 0 )
        {
            wchar_t * p = argv[ ++i ];
            ULONG t = wcstoul ( p, &p, 10 );
            if ( t > MSRDC_MAXIMUM_HORIZONSIZE )
            {
                printf ( "\n\nRequested horizon size -- \"%i\" is greater than the maximum MSRDC horizon size (%i).\n\n", t, MSRDC_MAXIMUM_HORIZONSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else if ( t < MSRDC_MINIMUM_HORIZONSIZE )
            {
                printf ( "\n\nRequested horizon size -- \"%i\" is less than the minimum MSRDC horizon size (%i).\n\n", t, MSRDC_MINIMUM_HORIZONSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else
            {
                g_horizonSizeN = t;
            }
        }
        else if ( wcscmp ( argv[ i ] + 1, L"hws1" ) == 0 )
        {
            wchar_t * p = argv[ ++i ];
            ULONG t = wcstoul ( p, &p, 10 );
            if ( t > MSRDC_MAXIMUM_HASHWINDOWSIZE )
            {
                printf ( "\n\nRequested hash window size -- \"%i\" is greater than the maximum MSRDC hash window size (%i).\n\n", t, MSRDC_MAXIMUM_HASHWINDOWSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else if ( t < MSRDC_MINIMUM_HASHWINDOWSIZE )
            {
                printf ( "\n\nRequested hash window size -- \"%i\" is less than the maximinimummum MSRDC hash window size (%i).\n\n", t, MSRDC_MINIMUM_HASHWINDOWSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else
            {
                g_hashWindowSize1 = t;
            }
        }
        else if ( wcscmp ( argv[ i ] + 1, L"hwsn" ) == 0 )
        {
            wchar_t * p = argv[ ++i ];
            ULONG t = wcstoul ( p, &p, 10 );
            if ( t > MSRDC_MAXIMUM_HASHWINDOWSIZE )
            {
                printf ( "\n\nRequested hash window size -- \"%i\" is greater than the maximum MSRDC hash window size (%i).\n\n", t, MSRDC_MAXIMUM_HASHWINDOWSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else if ( t < MSRDC_MINIMUM_HASHWINDOWSIZE )
            {
                printf ( "\n\nRequested hash window size -- \"%i\" is less than the minimum MSRDC hash window size (%i).\n\n", t, MSRDC_MINIMUM_HASHWINDOWSIZE );
                Usage ( argc, argv );
                exit ( 1 );
            }
            else
            {
                g_hashWindowSizeN = t;
            }
        }
        else
        {
            printf ( "\nInvalid parameter: %ws\n\n", argv[ i ] );
            Usage ( argc, argv );
            exit ( 1 );
        }
    }

    char *argPurpose [] = {"Remote machine", "Remote file (source)", "Local file (seed)", "Target file"};
    int argCount = 0;

    while ( i < argc )
    {
        args[ argCount++ ] = argv[ i++ ];
    }

    if ( argCount < ARRAYSIZE ( argPurpose ) )
    {
        printf ( "\n%s must be specified.\n\n", argPurpose[ argCount ] );
        Usage ( argc, argv );
        exit ( 1 );
    }

    hr = coInit.GetHRESULT();
    if ( SUCCEEDED ( hr ) )
    {
        hr = CheckVersion();
    }

    if ( SUCCEEDED ( hr ) )
    {
        hr = Transfer ( args[ 0 ], args[ 1 ], args[ 2 ], args[ 3 ] );
    }

    LPVOID lpMsgBuf;
    FormatMessage (
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        0,  // Default language
        ( LPTSTR ) & lpMsgBuf,
        0,
        NULL
    );
    _tprintf ( _T ( "Error code: x%08x :: %s" ), hr, lpMsgBuf );
    LocalFree ( lpMsgBuf );

    if ( FAILED ( hr ) )
    {
        exit ( 1 );
    }

    return 0;
}
