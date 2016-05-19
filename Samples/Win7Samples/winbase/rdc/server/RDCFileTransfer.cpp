// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// RdcFileTransfer.cpp : Implementation of CRdcFileTransfer

#include "stdafx.h"
#include <ctype.h>
#include "RdcSdkTestServer.h"
#include "RdcFileTransfer.h"
#include "RdcFileHandleImpl.h"
#include "msrdc.h"


static DebugHresult ValidateFilename ( LPCWSTR fileName )
{
    // Require the filename be of the form DriverLetter:\path
    if ( wcslen ( fileName ) < 4 )
    {
        return E_INVALIDARG;
    }

    if ( !iswalpha ( fileName[ 0 ] ) )
    {
        return E_INVALIDARG;
    }

    if ( fileName[ 1 ] != L':' || fileName[ 2 ] != L'\\' )
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

DebugHresult CRdcFileTransfer::FinalConstruct()
{
    return S_OK;
}

/*---------------------------------------------------------------------------
Name:     CRdcFileTransfer::FinalRelease
 
Close all open file handles.
 
----------------------------------------------------------------------------*/
void CRdcFileTransfer::FinalRelease()
{
    for ( ULONG i = 0; i < m_RdcFileHandles.Size(); ++i )
    {
        delete m_RdcFileHandles[ i ];
        m_RdcFileHandles[ i ] = 0;
    }
}

/*---------------------------------------------------------------------------
Name:     CRdcFileTransfer::RdcOpenFile
 
Open the file.
Open existing signature files, or create new ones.
Create internal FileHandleImpl to track all the allocated resources, and
add it to our list of handles.
 
Arguments:
   fileName          The path of the file to open.
   fileInfo          Information about the file to return to the client.
   fileHandle        The resulting filehandle.
   deleteSigs        Delete temporary signature when files if TRUE.
                     Note: When an error is encountered, signature files
                           are always deleted.
   horizonSize1      The horizon size for the first recursion level.
   horizonSizeN      The horizon size for the second and higher recursion levels.
   hashWindowSize1   The hash window size for the first recursion level.
   hashWindowSizeN   The hash window size for the second and higher recursion levels.
----------------------------------------------------------------------------*/
STDMETHODIMP CRdcFileTransfer::RdcOpenFile (
    LPCWSTR fileName,
    RdcFileTransferInfo * fileInfo,
    RdcFileHandle * fileHandle,
    BOOL deleteSigs,
    ULONG horizonSize1,
    ULONG horizonSizeN,
    ULONG hashWindowSize1,
    ULONG hashWindowSizeN )
{
    DebugHresult hr = S_OK;
    RdcFileHandleImpl *fileHandleImpl = 0;

    hr = ValidateFilename ( fileName );

    if ( SUCCEEDED ( hr ) )
    {
        fileHandleImpl = new ( std::nothrow ) RdcFileHandleImpl();

        if ( !fileHandleImpl )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( SUCCEEDED ( hr ) )
    {

        hr = fileHandleImpl->OpenSource ( fileName, fileInfo, deleteSigs, horizonSize1, horizonSizeN, hashWindowSize1, hashWindowSizeN );
    }

    if ( SUCCEEDED ( hr ) )
    {
        CComCritSecLock<CComAutoCriticalSection>lock ( m_HandlesLock );

        for ( size_t i = 0; i < m_RdcFileHandles.Size(); ++i )
        {
            if ( !m_RdcFileHandles[ i ] )
            {
                m_RdcFileHandles[ i ] = fileHandleImpl;
                fileHandle->m_HandleValue = ( ULONG ) ( i + 1 );
                fileHandleImpl = 0;
                break;
            }
        }
        if ( !fileHandle->m_HandleValue )
        {
            if ( !m_RdcFileHandles.Append ( fileHandleImpl ) )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                fileHandle->m_HandleValue = static_cast<ULONG> ( ( m_RdcFileHandles.Size() - 1 ) + 1 );
                // It is in the list now, loose the local reference.
                fileHandleImpl = 0;
            }
        }
    }
    if ( FAILED ( hr ) )
    {
        delete fileHandleImpl;
    }

    return hr;
}

/*---------------------------------------------------------------------------
Name:     CRdcFileTransfer::ReadData
 
Read file or signature data from the given file handle.
 
Signature generation is done on-demand -- so the first call
to ReadData() can take a long time.
 
FUTURE: Put signature generation into a seperate thread, and
have it started by RdcOpenFile().
 
Arguments:
   fileHandle               The filehandle, returned by RdcOpenFile.
   signatureLevel           0==The file data, 1 == first level of signatures, etc...
   fileOffset               The offset into the file to begin reading from.
   bytesToRead              The number of bytes to read
   bytesActuallyRead        The number of bytes actually read
   data                     The buffer to store the data in.
 
Returns:
 
----------------------------------------------------------------------------*/
STDMETHODIMP CRdcFileTransfer::ReadData (
    RdcFileHandle * fileHandle,
    ULONG signatureLevel,
    ULONGLONG fileOffset,
    ULONG bytesToRead,
    ULONG *bytesActuallyRead,
    BYTE * data )
{
    DebugHresult hr = S_OK;

    *bytesActuallyRead = 0;

    // Validate and lookup the filehandle in our array
    // of filehandles.
    RdcFileHandleImpl *fileHandleImpl = 0;
    if ( fileHandle->m_HandleValue )
    {
        CComCritSecLock<CComAutoCriticalSection> lock ( m_HandlesLock );
        if ( m_RdcFileHandles.Size() >= fileHandle->m_HandleValue )
        {
            fileHandleImpl = m_RdcFileHandles[ fileHandle->m_HandleValue - 1 ];
        }
    }

    if ( !fileHandleImpl ||
            signatureLevel > MSRDC_MAXIMUM_DEPTH )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED ( hr ) )
    {
        // Delegate the actual reading to the internal fileHandle implementation
        hr = fileHandleImpl->ReadData (
                 signatureLevel,
                 fileOffset,
                 bytesToRead,
                 bytesActuallyRead,
                 data );
    }

    return hr;
}


STDMETHODIMP CRdcFileTransfer::GetSimilarityData ( SimilarityData * similarityData )
{
    if ( m_RdcFileHandles[ 0 ] )
    {
        return m_RdcFileHandles[ 0 ] ->GetSimilarityData ( similarityData );
    }

    return E_FAIL;
}


STDMETHODIMP CRdcFileTransfer::GetFileSize (
    RdcFileHandle * fileHandle,
    ULONG signatureLevel,
    ULONGLONG * fileSize )
{
    DebugHresult hr = S_OK;

    *fileSize = 0;

    // Validate and lookup the filehandle in our array
    // of filehandles.
    RdcFileHandleImpl *fileHandleImpl = 0;
    if ( fileHandle->m_HandleValue )
    {
        CComCritSecLock<CComAutoCriticalSection> lock ( m_HandlesLock );
        if ( m_RdcFileHandles.Size() >= fileHandle->m_HandleValue )
        {
            fileHandleImpl = m_RdcFileHandles[ fileHandle->m_HandleValue - 1 ];
        }
    }

    if ( !fileHandleImpl ||
            signatureLevel > MSRDC_MAXIMUM_DEPTH )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED ( hr ) )
    {
        // Delegate the actual reading to the internal fileHandle implementation
        hr = fileHandleImpl->GetFileSize ( signatureLevel, fileSize );
    }

    return hr;
}

STDMETHODIMP CRdcFileTransfer::Close (
    RdcFileHandle * fileHandle )
{
    DebugHresult hr = S_OK;

    RdcFileHandleImpl *fileHandleImpl = 0;
    // Validate and lookup the filehandle in our array
    // of filehandles.
    if ( fileHandle->m_HandleValue )
    {
        CComCritSecLock<CComAutoCriticalSection> lock ( m_HandlesLock )
            ;
        if ( m_RdcFileHandles.Size() >= fileHandle->m_HandleValue )
        {
            fileHandleImpl = m_RdcFileHandles[ fileHandle->m_HandleValue - 1 ];
            m_RdcFileHandles[ fileHandle->m_HandleValue - 1 ] = 0;
        }
    }

    if ( fileHandleImpl )
    {
        // Release all the filehandle resources.
        delete fileHandleImpl;
        fileHandle->m_HandleValue = 0;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}
