//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <conio.h>
#include "Client.h"
#include <strsafe.h>

//////////////////////////////////////////////////////////////////////////////
// print_result - Display HRESULTs
//////////////////////////////////////////////////////////////////////////////
void print_result(HRESULT hr)
{
    if( S_OK == hr )
    {
        _cwprintf(L"[S_OK]\r\n");
    }
    else if( S_FALSE == hr )
    {
        _cwprintf(L"[S_FALSE]\r\n");
    }
    else 
    {
        _cwprintf(L"[ERROR: %x]\r\n", hr);
    }
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceEventNotify methods
//////////////////////////////////////////////////////////////////////////////
CFileServiceEventNotify::CFileServiceEventNotify()
    : m_cRef(1)
{
}

//////////////////////////////////////////////////////////////////////////////
// FileChangeEvent - Invoked when the service sends this event
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CFileServiceEventNotify::FileChangeEvent(
    FILE_CHANGE_EVENT* pFileChangeEvent)
{
    if( NULL == pFileChangeEvent )
    {
        return E_INVALIDARG;
    }

    // Simply print the event and return
    _cwprintf(L"\r\nReceived event: [%s: %s]\r\n", 
        pFileChangeEvent->EventType, 
        pFileChangeEvent->FileName);

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CGetFileAsyncCallback methods
//////////////////////////////////////////////////////////////////////////////
CGetFileAsyncCallback::CGetFileAsyncCallback()
:   m_cRef(1)
,   m_pFileServiceProxy(NULL)
{
}

CGetFileAsyncCallback::~CGetFileAsyncCallback()
{
    if( NULL != m_pFileServiceProxy )
    {
        m_pFileServiceProxy->Release();
        m_pFileServiceProxy = NULL;
    }
}

HRESULT CGetFileAsyncCallback::Init(
        IFileServiceProxy* pFileServiceProxy, 
        LPCWSTR pszFileName,
        LPCWSTR pszReceiveDirectory)
{
    HRESULT hr = S_OK;

    // Validate parameters
    if( NULL == pFileServiceProxy )
    {
        return E_INVALIDARG;
    }

    if( NULL == pszFileName )
    {
        return E_INVALIDARG;
    }

    if( NULL == pszReceiveDirectory )
    {
        return E_INVALIDARG;
    }

    // AddRef or copy parameters
    m_pFileServiceProxy = pFileServiceProxy;
    m_pFileServiceProxy->AddRef();

    hr = ::StringCbCopyW(m_szFile, sizeof(m_szFile), pszReceiveDirectory);

    if( S_OK == hr )
    {
        hr = ::StringCbCatW(m_szFile, sizeof(m_szFile), pszFileName);
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// ReceiveBinary - Creates a local file in the files-directory
//                 from the inbound attachment
//////////////////////////////////////////////////////////////////////////////
HRESULT CGetFileAsyncCallback::ReceiveBinary(
    IWSDAttachment* pAttachment, 
    LPCWSTR pszLocalFileName)
{
    HRESULT hr = S_OK;
    IWSDInboundAttachment* pStream = NULL;
    BYTE buffer[8192];
    ULONG cbBytesRead = 0;
    ULONG cbBytesLeft = 0;
    DWORD cbBytesWritten = 0;
    ULONG cbBytesTotal = 0;
    HANDLE hFile = NULL;
    DWORD dwErr = 0;

    // Validate parameters
    if( NULL == pAttachment )
    {
        return E_INVALIDARG;
    }

    if( NULL == pszLocalFileName )
    {
        return E_INVALIDARG;
    }

    if( 0 == wcscmp( pszLocalFileName, L"" ) )
    {
        return E_INVALIDARG;
    }

    // Make sure this attachment is an inbound attachment, and get that
    // interface
    hr = pAttachment->QueryInterface(
        __uuidof(IWSDInboundAttachment), (void**)&pStream);

    // Create a file to write attachment data into
    if( S_OK == hr )
    {
        _cwprintf(L"    Creating local file %s... ", pszLocalFileName);
        hFile = ::CreateFileW( pszLocalFileName, FILE_WRITE_DATA, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

        if ( INVALID_HANDLE_VALUE == hFile )
        {
            hFile = NULL;

            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }

        print_result( hr );
    }

    // Read from the inbound attachment and write to file until finished
    while( S_OK == hr )
    {
        _cwprintf(L"    Reading attachment data... ");
        hr = pStream->Read( buffer, sizeof(buffer), &cbBytesRead );

        cbBytesLeft = cbBytesRead;

        // pStream->Read will return S_FALSE on the last read if the buffer
        // was not completely filled.  Hang onto the S_FALSE until the
        // end of this loop
        while( (S_OK == hr || S_FALSE == hr) && 0 < cbBytesLeft )
        {
            if( 0 == WriteFile( hFile,
                        buffer + (cbBytesRead - cbBytesLeft),
                        cbBytesLeft,
                        &cbBytesWritten, NULL ) )
            {
                dwErr = ::GetLastError();
                hr = HRESULT_FROM_WIN32( dwErr );
            }

            if( S_OK == hr || S_FALSE == hr )
            {
                cbBytesLeft -= cbBytesWritten;
            }
        }
        print_result( hr );

        cbBytesTotal += (cbBytesRead - cbBytesLeft);

        // If that was the last read, reset hr and bail out
        if( S_FALSE == hr )
        {
            hr = S_OK;
            break;
        }
    }

    // Print success message if we got the file without any problems
    if( S_OK == hr )
    {
        _cwprintf( L"%s recieved (%d bytes).\r\n", pszLocalFileName,
                cbBytesTotal );
    }
    else
    {
        _cwprintf( L"Error receiving %s.  Only recieved (%d bytes).\r\n",
                pszLocalFileName, cbBytesTotal );
    }

    // cleanup
    if( NULL != hFile )
    {
        ::CloseHandle( hFile );
        hFile = NULL;
    }

    if( NULL != pStream )
    {
        pStream->Release();
        pStream = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// AsyncOperationComplete - Called when the GetFile operation is complete
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CGetFileAsyncCallback::AsyncOperationComplete(
    IWSDAsyncResult* pAsyncResult,
    IUnknown* pAsyncState)
{
    UNREFERENCED_PARAMETER(pAsyncState);

    GET_FILE_RESPONSE* pResponse = NULL;
    HRESULT hr = S_OK;

    //
    // When the GetFile operation completes, we enter this callback.  It's
    // then our responsibility to call EndGetFile to actually retrieve
    // the results.
    //
    _cwprintf(L"Asynchronous GetFile operation completed.\r\n");
    hr = m_pFileServiceProxy->EndGetFile( pAsyncResult, &pResponse );

    // Call into our helper method to save the attachment to disk
    if( S_OK == hr && NULL != pResponse )
    {
        hr = ReceiveBinary( pResponse->Attachment, m_szFile );
    }
    else
    {
        _cwprintf(L"    GetFile operation failed or returned NULL response: ");
        print_result( hr );
    }

    // cleanup
    if( NULL != pResponse )
    {
        WSDFreeLinkedMemory( pResponse );
        pResponse = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// GetFileList - Invoke the GetFileList (i.e., "dir") service method
//////////////////////////////////////////////////////////////////////////////
HRESULT GetFileList(
    IFileServiceProxy* pFileServiceProxy)
{
    GET_FILE_LIST_RESPONSE* pResponse = NULL;
    HRESULT hr = S_OK;

    if( NULL == pFileServiceProxy )
    {
        return E_INVALIDARG;
    }

    // Invoke GetFileList method on service
    _cwprintf(L"Invoking GetFileList method on service... ");
    hr = pFileServiceProxy->GetFileList( &pResponse );
    print_result( hr );

    // Print results
    if( S_OK == hr && NULL != pResponse )
    {
        PWCHAR_LIST *pList = pResponse->FileList;

        while( pList )
        {
            _cwprintf(L"%s\r\n", (NULL == pList->Element ? L"(null)" : pList->Element));
            pList = pList->Next;
        }
    }

    // cleanup
    if( NULL != pResponse )
    {
        WSDFreeLinkedMemory( pResponse );
        pResponse = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// GetFile - Invoke the GetFile service method
//////////////////////////////////////////////////////////////////////////////
HRESULT GetFile(
    IFileServiceProxy* pFileServiceProxy, 
    LPCWSTR pszFileName, 
    LPCWSTR pszReceiveDirectory)
{
    GET_FILE_REQUEST params;
    HRESULT hr = S_OK;
    CGetFileAsyncCallback* pGetFileCallback = NULL;
    IWSDAsyncResult* pAsyncResult = NULL;

    if( NULL == pszFileName )
    {
        return E_INVALIDARG;
    }

    _cwprintf(L"Invoking GetFile method on service (file=%s)...\r\n",
            pszFileName);

    // Prepare parameters for service method
    params.filePath = pszFileName;
        
    //
    // Set up the async callback function.
    // pGetFileCallback->AsyncOperationComplete will get called when the stream
    // has finished downloading, and will save the results into a local file.
    //
    pGetFileCallback = new CGetFileAsyncCallback();

    if( NULL == pGetFileCallback )
    {
        hr = E_OUTOFMEMORY;
    }

    if( S_OK == hr )
    {
        _cwprintf(L"    Initializing callback structure... ");
        hr = pGetFileCallback->Init( pFileServiceProxy,
                pszFileName, pszReceiveDirectory );
        print_result( hr );
    }

    // Invoke GetFile method on service
    if( S_OK == hr )
    {
        _cwprintf(L"    Starting GetFile operation... ");
        hr = pFileServiceProxy->BeginGetFile( &params, NULL,
                pGetFileCallback, &pAsyncResult );
        print_result( hr );
    }

    // cleanup
    if( NULL != pGetFileCallback )
    {
        // Release pGetFileCallback now--if it was successful,
        // BeginGetFile has added its own reference
        pGetFileCallback->Release();
        pGetFileCallback = NULL;
    }

    if( NULL != pAsyncResult )
    {
        // pGetFileCallback will hold a reference to our async result
        // object, so we can release our reference now.
        pAsyncResult->Release();
        pAsyncResult = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// Help - Displays list of commands
//////////////////////////////////////////////////////////////////////////////
void Help()
{
    _cwprintf(L"COMMANDS:\r\n");
    _cwprintf(L"   dir     Get the list of files on server\r\n");
    _cwprintf(L"   get     Retrieve a file from the server\r\n");
    _cwprintf(L"   quit    Exits program\r\n");
}

//////////////////////////////////////////////////////////////////////////////
// Usage - Displays command-line options
//////////////////////////////////////////////////////////////////////////////
void Usage(LPCWSTR pszAdditionalInformation)
{
    _cwprintf(L"FileServiceClient.exe <files-directory> [<device-address>]\r\n");
    _cwprintf(L"Ex: FileServiceClient.exe c:\\temp urn:uuid:2BA7FF9A-CC7D-4781-B744-9347C5AC5A28\r\n");

    if( NULL != pszAdditionalInformation )
    {
        _cwprintf( L"%s\r\n", pszAdditionalInformation );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Main Entry Point
//      argv[0] = executable name
//      argv[1] = files-directory - the local directory to copy files to
//      argv[2] = device address
//////////////////////////////////////////////////////////////////////////////
int _cdecl wmain(
    int argc, 
    __in_ecount(argc) LPWSTR* argv)
{
    HRESULT hr = S_OK;
    LPCWSTR pszAdditionalInformation = NULL;
    WCHAR szReceiveDirectory[MAX_PATH];
    // szLocalAddress 46 bytes to hold UUID plus "urn:uuid:" and trailing 0
    WCHAR szLocalAddress[46];
    LPCWSTR pszDeviceAddress = NULL;
    size_t cchFileDirectoryLength = 0;            
    HANDLE hDir = NULL;
    UUID uuid = { 0 };
    CFileServiceProxy* pFileServiceProxy = NULL;
    CFileServiceEventNotify* pFileServiceEventNotify = NULL;
    bool bSubscribedToEvents = false;
    DWORD dwErr = 0;

    //////////////////////////////////////////////////////////////////////////
    // Validate command-line parameters
    //////////////////////////////////////////////////////////////////////////
    if( argc <= 2 )
    {
        hr = E_INVALIDARG;
    }

    // copy the files-directory from first argument (argv[1])
    if( S_OK == hr )
    {
        hr = ::StringCbCopyW( szReceiveDirectory, sizeof(szReceiveDirectory),
                argv[1] );
    }

    // add a backslash to files-directory if it doesn't have one
    if( S_OK == hr )
    {
        hr = ::StringCchLengthW( szReceiveDirectory, sizeof(szReceiveDirectory),
                &cchFileDirectoryLength );
    }

    if( S_OK == hr && cchFileDirectoryLength < 1 )
    {
        pszAdditionalInformation =
            L"<files-directory> must have non-zero length";

        hr = E_INVALIDARG;
    }

    if( S_OK == hr && szReceiveDirectory[cchFileDirectoryLength - 1] != L'\\')
    {
        hr = ::StringCbCatW( szReceiveDirectory, sizeof(szReceiveDirectory),
                L"\\");
    }

    // Check if files-directory actually exists
    if( S_OK == hr )
    {
        hDir = ::CreateFileW( szReceiveDirectory, GENERIC_READ, 
            FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,
            NULL );

        if( hDir == INVALID_HANDLE_VALUE )
        {
            hDir = NULL;
            pszAdditionalInformation = L"<files-directory> does not exist";
            hr = E_INVALIDARG;
        }
    }

    if( S_OK == hr )
    {
        // If the handle could be opened, the directory exists.  However,
        // we have no more use for the handle, so close it now.
        if( 0 == ::CloseHandle( hDir ))
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }

        hDir = NULL;
    }

    // Get device address from second argument (argv[2])
    if( S_OK == hr )
    {
        pszDeviceAddress = argv[2];
    }

    //////////////////////////////////////////////////////////////////////////
    // Done with command-line validation--if we've hit an error, print
    // Usage now and return -1
    //////////////////////////////////////////////////////////////////////////
    if( S_OK != hr )
    {
        Usage( pszAdditionalInformation );
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////
    // Build the proxy
    //////////////////////////////////////////////////////////////////////////

    // Generate local ID for our proxy
    if( S_OK == hr )
    {
        RPC_STATUS st = UuidCreate(&uuid);

        if( RPC_S_OK != st )
        {
            hr = E_FAIL;
        }
    }

    if( S_OK == hr )
    {
        hr = ::StringCbPrintfW(
            szLocalAddress, sizeof(szLocalAddress), 
            L"urn:uuid:%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid.Data1, uuid.Data2, uuid.Data3,
            uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], 
            uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]); 
    }

    // Build the proxy
    if( S_OK == hr )
    {
        _cwprintf(L"Creating a proxy for device %s... ", pszDeviceAddress );
        hr = CreateCFileServiceProxy( pszDeviceAddress, szLocalAddress,
                &pFileServiceProxy, NULL );
        print_result( hr );
    }

    // Build event sink for FileChange event
    if( S_OK == hr )
    {
        _cwprintf(L"Creating an event sink... ");
        pFileServiceEventNotify = new CFileServiceEventNotify();

        if( NULL == pFileServiceEventNotify )
        {
            hr = E_OUTOFMEMORY;
        }
        print_result( hr );
    }
   
    // Subscribe to FileChange event
    if( S_OK == hr )
    {
        _cwprintf(L"Subscribing to Events... ");
        hr = pFileServiceProxy->SubscribeToFileChangeEvent( pFileServiceEventNotify );

        if( S_OK == hr )
        {
            bSubscribedToEvents = true;
        }
        print_result( hr );
    }

    if( S_OK == hr )
    {
        _cwprintf(L"Proxy creation finished.\r\n");  
        Help();
    }

    //////////////////////////////////////////////////////////////////////////
    // Present interactive prompt
    //////////////////////////////////////////////////////////////////////////
    while( S_OK == hr )
    {
        WCHAR szCommand[MAX_PATH];
        size_t cbSizeRead = 0;
        LPWSTR pszArg = NULL;

        _cwprintf(L"\r\n>");       

        // Ignore result of _cgetws_s
        (void)_cgetws_s( szCommand,
                         sizeof(szCommand) / sizeof(WCHAR), &cbSizeRead );

        // Look for arguments in command
        pszArg = wcschr(szCommand, L' ');
        if( pszArg )
        {
            *pszArg = 0;
            pszArg++;
        }

        // Ignore result of towlower
        (void)towlower( *szCommand );
       
        // Match first character and perform appropriate operation
        if( 0 == wcscmp( szCommand, L"get" ) )
        {
            hr = GetFile( pFileServiceProxy, pszArg, szReceiveDirectory );
        }
        else if( 0 == wcscmp( szCommand, L"dir" ) )
        {
            hr = GetFileList( pFileServiceProxy );
        }
        else if( 0 == wcscmp( szCommand, L"quit" ) )
        {
            // Use S_FALSE to break out of the while loop and exit
            hr = S_FALSE;
        }
        else 
        {
            // Encountered an unknown command--print help
            Help();
        }
    }

    _cwprintf(L"Cleaning up resources... ");

    // cleanup
    if( NULL != pFileServiceEventNotify )
    {
        pFileServiceEventNotify->Release();
        pFileServiceEventNotify = NULL;
    }

    if( NULL != pFileServiceProxy )
    {
        if( bSubscribedToEvents )
        {
            pFileServiceProxy->UnsubscribeToFileChangeEvent();
        }

        pFileServiceProxy->Release();
        pFileServiceProxy = NULL;
    }

    _cwprintf(L"finished.\r\n");

    if( SUCCEEDED(hr) )
    {
        return 0;
    }

    return -1;
}
