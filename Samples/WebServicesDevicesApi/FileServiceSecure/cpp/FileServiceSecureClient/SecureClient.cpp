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
#include "SecureClient.h"
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
// CFileServiceSecureEventNotify methods
//////////////////////////////////////////////////////////////////////////////
CFileServiceSecureEventNotify::CFileServiceSecureEventNotify()
    : m_cRef(1)
{
}

HRESULT STDMETHODCALLTYPE CFileServiceSecureEventNotify::QueryInterface(
    REFIID riid, 
    void **ppvObject)
{
    HRESULT hr = S_OK;
    
    if ( NULL == ppvObject )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppvObject = NULL;
    }
    
    if ( S_OK == hr )
    {
        if ( riid == __uuidof(IFileServiceSecureEventNotify) )
        {
            *ppvObject = (IFileServiceSecureEventNotify *)this;
        }
        else if ( riid == __uuidof(IUnknown) )
        {
            *ppvObject = (IUnknown *)this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    
    if ( S_OK == hr )
    {
        ((LPUNKNOWN)*ppvObject)->AddRef();
    }

    return hr;
}

ULONG STDMETHODCALLTYPE CFileServiceSecureEventNotify::AddRef()
{
    return InterlockedIncrement( (LONG *)&m_cRef );
}

ULONG STDMETHODCALLTYPE CFileServiceSecureEventNotify::Release()
{
    ULONG ulNewRefCount = (ULONG)InterlockedDecrement( (LONG *)&m_cRef );
    
    if ( 0 == ulNewRefCount )
    {
        delete this;
    }
    return ulNewRefCount;
}

//////////////////////////////////////////////////////////////////////////////
// FileChangeEvent - Invoked when the service sends this event
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CFileServiceSecureEventNotify::FileChangeEvent(
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
,   m_pFileServiceSecureProxy(NULL)
{
}

CGetFileAsyncCallback::~CGetFileAsyncCallback()
{
    if( NULL != m_pFileServiceSecureProxy )
    {
        m_pFileServiceSecureProxy->Release();
        m_pFileServiceSecureProxy = NULL;
    }
}

HRESULT CGetFileAsyncCallback::Init(
        IFileServiceSecureProxy* pFileServiceSecureProxy, 
        LPCWSTR pszFileName,
        LPCWSTR pszReceiveDirectory)
{
    HRESULT hr = S_OK;

    // Validate parameters
    if( NULL == pFileServiceSecureProxy )
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
    m_pFileServiceSecureProxy = pFileServiceSecureProxy;
    m_pFileServiceSecureProxy->AddRef();

    hr = ::StringCbCopyW(m_szFile, sizeof(m_szFile), pszReceiveDirectory);

    if( S_OK == hr )
    {
        hr = ::StringCbCatW(m_szFile, sizeof(m_szFile), pszFileName);
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CGetFileAsyncCallback::QueryInterface(
    REFIID riid, 
    void **ppvObject)
{
    HRESULT hr = S_OK;
    
    if ( NULL == ppvObject )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppvObject = NULL;
    }
    
    if ( S_OK == hr )
    {
        if ( riid == __uuidof(IWSDAsyncCallback) )
        {
            *ppvObject = (IWSDAsyncCallback *)this;
        }
        else if ( riid == __uuidof(IUnknown) )
        {
            *ppvObject = (IUnknown *)this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    
    if ( S_OK == hr )
    {
        ((LPUNKNOWN)*ppvObject)->AddRef();
    }

    return hr;
}

ULONG STDMETHODCALLTYPE CGetFileAsyncCallback::AddRef()
{
    return InterlockedIncrement( (LONG *)&m_cRef );
}

ULONG STDMETHODCALLTYPE CGetFileAsyncCallback::Release()
{
    ULONG ulNewRefCount = (ULONG)InterlockedDecrement( (LONG *)&m_cRef );
    
    if ( 0 == ulNewRefCount )
    {
        delete this;
    }
    return ulNewRefCount;
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
    hr = m_pFileServiceSecureProxy->EndGetFile( pAsyncResult, &pResponse );

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
    IFileServiceSecureProxy* pFileServiceSecureProxy)
{
    GET_FILE_LIST_RESPONSE* pResponse = NULL;
    HRESULT hr = S_OK;

    if( NULL == pFileServiceSecureProxy )
    {
        return E_INVALIDARG;
    }

    // Invoke GetFileList method on service
    _cwprintf(L"Invoking GetFileList method on service... ");
    hr = pFileServiceSecureProxy->GetFileList( &pResponse );
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
    IFileServiceSecureProxy* pFileServiceSecureProxy, 
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
        hr = pGetFileCallback->Init( pFileServiceSecureProxy,
                pszFileName, pszReceiveDirectory );
        print_result( hr );
    }

    // Invoke GetFile method on service
    if( S_OK == hr )
    {
        _cwprintf(L"    Starting GetFile operation... ");
        hr = pFileServiceSecureProxy->BeginGetFile( &params, NULL,
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
// ParseArgs - Parses the command lind arguments
//////////////////////////////////////////////////////////////////////////////
_Success_( return == S_OK )
HRESULT ParseArgs(
    int argc, 
    _In_reads_(argc) LPWSTR* argv,
    BOOL* pbIsServerCertHash,
    _Outptr_result_maybenull_ LPWSTR* ppszServerCertHashAlg,
    _Outptr_result_maybenull_ LPWSTR* ppszServerCertHashThumbprint,
    BOOL* pbIsCertAuth,
    _Outptr_result_maybenull_ LPWSTR* ppszCertAuthThumbprint,
    BOOL* pbIsHttpAuth,
    DWORD* pdwHttpAuthScheme,
    _In_reads_(cchFileDirectoryBufferSize) LPWSTR pszFileDirectory,
    size_t cchFileDirectoryBufferSize,
    _Outptr_ LPWSTR* ppszDeviceAddress)
{
    HRESULT hr = S_OK;
    DWORD dwStatus = 0;
    int iParamIndex = 1; // ignore argv[0] - start with argv[1]

    // All LPWSTRs are shallow copied.  Do not delete.

    BOOL bIsServerCertHash = FALSE;
    LPWSTR pszServerCertHashAlg = NULL;
    LPWSTR pszServerCertHashThumbprint = NULL;
    BOOL bIsCertAuth = FALSE;
    LPWSTR pszCertAuthThumbprint = NULL;
    BOOL bIsHttpAuth = FALSE;
    DWORD dwHttpAuthScheme = 0;
    LPWSTR pszTempFileDirectory = NULL;
    LPWSTR pszTempDeviceAddress = NULL;

    size_t cchFileDirectoryLength = 0;

    if ( argc < 3 || argc > 10 || NULL == argv
            || 0 == cchFileDirectoryBufferSize || NULL == pszFileDirectory )
    {
        //
        // argv[0] = L"FileServiceSecureClient.exe"
        //
        // Minimum: argc = 3
        // FileServiceSecureClient.exe
        //          <files-directory> <device-address>
        //
        // Maximum: argc = 10
        // FileServiceSecureClient.exe
        //          -ServerCertHash <alg> <thumbprint>
        //          -CertAuth <thumbprint>
        //          -HttpAuth <scheme>
        //          <files-directory> <device-address>
        //
        hr = E_INVALIDARG;
    }
    else if ( NULL == pbIsServerCertHash || NULL == ppszServerCertHashAlg
            || NULL == ppszServerCertHashThumbprint || NULL == pbIsCertAuth
            || NULL == ppszCertAuthThumbprint || NULL == pbIsHttpAuth 
            || NULL == pdwHttpAuthScheme || NULL == ppszDeviceAddress )
    {
        hr = E_POINTER;
    }
    else
    {
        *pbIsServerCertHash = FALSE;
        *ppszServerCertHashAlg = NULL;
        *ppszServerCertHashThumbprint = NULL;

        *pbIsCertAuth = FALSE;
        *ppszCertAuthThumbprint = NULL;

        *pbIsHttpAuth = FALSE;
        *pdwHttpAuthScheme = 0;

        ::ZeroMemory( pszFileDirectory, cchFileDirectoryBufferSize );

        *ppszDeviceAddress = NULL;
    }

    //
    // -ServerCertHash <Algorithm> <Thumbprint>
    //
    if ( S_OK == hr && iParamIndex < argc 
            && 0 == _wcsicmp( L"-ServerCertHash", argv[iParamIndex] ) )
    {
        if ( (iParamIndex + 2) >= argc )
        {
            hr = E_INVALIDARG;
        }
        else
        {
            bIsServerCertHash = TRUE;
            pszServerCertHashAlg = argv[iParamIndex + 1];
            pszServerCertHashThumbprint = argv[iParamIndex + 2];

            iParamIndex += 3;
        }
    }

    //
    // -CertAuth NULL
    // -CertAuth <Thumbprint>
    //
    if ( S_OK == hr && iParamIndex < argc 
            && 0 == _wcsicmp( L"-CertAuth", argv[iParamIndex] ) )
    {
        if ( (iParamIndex + 1) >= argc )
        {
            hr = E_INVALIDARG;
        }
        else if ( 0 == _wcsicmp( L"NULL", argv[iParamIndex + 1] ) )
        {
            bIsCertAuth = TRUE;
            pszCertAuthThumbprint = NULL;

            iParamIndex += 2;
        }
        else
        {
            bIsCertAuth = TRUE;
            pszCertAuthThumbprint = argv[iParamIndex + 1];

            iParamIndex += 2;
        }
    }

    //
    // -HttpAuth <Scheme>
    //
    if ( S_OK == hr && iParamIndex < argc 
            && 0 == _wcsicmp( L"-HttpAuth", argv[iParamIndex] ) )
    {
        if ( (iParamIndex + 1) >= argc )
        {
            hr = E_INVALIDARG;
        }
        else
        {
            dwStatus = swscanf_s( 
                    argv[iParamIndex + 1], L"%d", &dwHttpAuthScheme );

            if ( 1 != dwStatus )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                bIsHttpAuth = TRUE;
                iParamIndex += 2;
            }
        }
    }

    //
    // <files-directory> <device-address>
    //
    if ( S_OK == hr )
    {
        if ( (iParamIndex + 1) >= argc )
        {
            hr = E_INVALIDARG;
        }
        else
        {
#pragma prefast( push )
#pragma prefast( disable: 6385, "String count was checked previously." )
            pszTempFileDirectory = argv[iParamIndex];
            pszTempDeviceAddress = argv[iParamIndex + 1];
#pragma prefast( pop )

            iParamIndex += 2;
        }
    }

    if ( S_OK == hr && iParamIndex != argc )
    {
        // By now, all the parameters must have been used up.  If not,
        // don't allow to proceed.
        hr = E_INVALIDARG;
    }

    if ( S_OK == hr )
    {
        // Check to see that the file directory is not an empty string
        hr = StringCchLengthW(
                pszTempFileDirectory, 
                cchFileDirectoryBufferSize, 
                &cchFileDirectoryLength );
        
        if ( S_OK == hr && cchFileDirectoryLength < 1 )
        {
            hr = E_INVALIDARG;
        }
    }

    // Return the variables to the caller.

    if ( S_OK == hr )
    {
        if ( '\\' != pszTempFileDirectory[cchFileDirectoryLength - 1] )
        {
            // Backslash required at the end but one is not present.
            // Add one to it as the string is being copied to the buffer.
            hr = StringCbPrintfW(
                    pszFileDirectory,
                    cchFileDirectoryBufferSize * sizeof(WCHAR),
                    L"%s\\", pszTempFileDirectory );
        }
        else
        {
            // Backslash is already present.
            hr = StringCbPrintfW(
                    pszFileDirectory,
                    cchFileDirectoryBufferSize * sizeof(WCHAR),
                    L"%s", pszTempFileDirectory );
        }
    }

    if ( S_OK == hr )
    {
        *pbIsServerCertHash = bIsServerCertHash;
        *ppszServerCertHashAlg = pszServerCertHashAlg;
        *ppszServerCertHashThumbprint = pszServerCertHashThumbprint;

        *pbIsCertAuth = bIsCertAuth;
        *ppszCertAuthThumbprint = pszCertAuthThumbprint;

        *pbIsHttpAuth = bIsHttpAuth;
        *pdwHttpAuthScheme = dwHttpAuthScheme;
        *ppszDeviceAddress = pszTempDeviceAddress;
    }

    // shallow copied
    pszServerCertHashAlg = NULL;
    pszServerCertHashThumbprint = NULL;
    pszCertAuthThumbprint = NULL;
    pszTempFileDirectory = NULL;
    pszTempDeviceAddress = NULL;

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
void Usage()
{
    _cwprintf(L"FileServiceSecureClient.exe\r\n");
    _cwprintf(L"    [-ServerCertHash <Algorithm> <Thumbprint>]\r\n");
    _cwprintf(L"    [    [-CertAuth NULL]\r\n");
    _cwprintf(L"         | [-CertAuth <Thumbprint>]]\r\n");
    _cwprintf(L"    [-HttpAuth <Scheme>]\r\n");
    _cwprintf(L"    <files-directory> <device-address>\r\n");
    _cwprintf(L"\r\n");
    _cwprintf(L"See the ReadMe file for a detailed usage description.\r\n");
}

//////////////////////////////////////////////////////////////////////////////
// Main Entry Point
//      argv[0] = executable name
//      argv[1] = files-directory - the local directory to copy files to
//      argv[2] = device address
//////////////////////////////////////////////////////////////////////////////
int _cdecl wmain(
    int argc, 
    _In_reads_(argc) LPWSTR* argv)
{
    HRESULT hr = S_OK;
    WCHAR szReceiveDirectory[MAX_PATH] = { 0 };
    // szLocalAddress 46 bytes to hold UUID plus "urn:uuid:" and trailing 0
    WCHAR szLocalAddress[46] = { 0 };
    LPWSTR pszDeviceAddress = NULL;
    BOOL bIsServerCertHash = FALSE;
    LPWSTR pszServerCertHashAlg = NULL;
    LPWSTR pszServerCertHashThumbprint = NULL;
    BOOL bIsCertAuth = FALSE;
    LPWSTR pszCertAuthThumbprint = NULL;
    BOOL bIsHttpAuth = FALSE;
    DWORD dwHttpAuthScheme = 0;
    HANDLE hDir = NULL;
    UUID uuid = { 0 };
    CFileServiceSecureProxy* pFileServiceSecureProxy = NULL;
    CFileServiceSecureEventNotify* pFileServiceSecureEventNotify = NULL;
    bool bSubscribedToEvents = false;
    DWORD dwErr = 0;

    //////////////////////////////////////////////////////////////////////////
    // Validate command-line parameters
    //////////////////////////////////////////////////////////////////////////
    if( argc <= 1 )
    {
        hr = E_INVALIDARG;
    }

    // check first argument for /? or -?
    if( S_OK == hr )
    {
        if( 0 == wcscmp( argv[1], L"/?") || 0 == wcscmp( argv[1], L"-?" ) )
        {
            hr = E_INVALIDARG;
        }
    }

    // Parse the arguments.
    if ( S_OK == hr )
    {
        hr = ParseArgs( argc, argv,
                &bIsServerCertHash, &pszServerCertHashAlg, &pszServerCertHashThumbprint,
                &bIsCertAuth, &pszCertAuthThumbprint,
                &bIsHttpAuth, &dwHttpAuthScheme,
                szReceiveDirectory, ARRAYSIZE(szReceiveDirectory), &pszDeviceAddress );
    }

    //////////////////////////////////////////////////////////////////////////
    // Done with command-line validation--if we've hit an error, print
    // Usage now and return -1
    //////////////////////////////////////////////////////////////////////////
    if( S_OK != hr )
    {
        Usage();
        return -1;
    }

    // Check if files-directory actually exists
    if( S_OK == hr )
    {
        hDir = CreateFileW( szReceiveDirectory, GENERIC_READ, 
                FILE_SHARE_READ, NULL, OPEN_EXISTING, 
                FILE_FLAG_BACKUP_SEMANTICS, NULL );

        if( INVALID_HANDLE_VALUE == hDir )
        {
            hDir = NULL;
            hr = E_INVALIDARG;

            _cwprintf(L"The files directory %s does not exist.\r\n",
                    szReceiveDirectory);
        }
    }

    if( S_OK == hr )
    {
        // If the handle could be opened, the directory exists.  However,
        // we have no more use for the handle, so close it now.
        if( 0 == CloseHandle( hDir ))
        {
            dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }

        hDir = NULL;
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
        hr = CreateCFileServiceSecureProxy( 
                bIsServerCertHash, pszServerCertHashAlg, pszServerCertHashThumbprint,
                bIsCertAuth, pszCertAuthThumbprint,
                bIsHttpAuth, dwHttpAuthScheme,
                pszDeviceAddress, szLocalAddress,
                &pFileServiceSecureProxy, NULL );
        print_result( hr );
    }

    // Build event sink for FileChange event
    if( S_OK == hr )
    {
        _cwprintf(L"Creating an event sink... ");
        pFileServiceSecureEventNotify = new CFileServiceSecureEventNotify();

        if( NULL == pFileServiceSecureEventNotify )
        {
            hr = E_OUTOFMEMORY;
        }
        print_result( hr );
    }
   
    // Subscribe to FileChange event
    if( S_OK == hr )
    {
        _cwprintf(L"Subscribing to Events... ");
        hr = pFileServiceSecureProxy->SubscribeToFileChangeEvent( pFileServiceSecureEventNotify );

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

        ZeroMemory(szCommand, MAX_PATH);

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
            hr = GetFile( pFileServiceSecureProxy, pszArg, szReceiveDirectory );
        }
        else if( 0 == wcscmp( szCommand, L"dir" ) )
        {
            hr = GetFileList( pFileServiceSecureProxy );
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

        if ( S_FALSE != hr && S_OK != hr )
        {
            _cwprintf(L"Command failed with [0x%08x]\r\n", hr);

            // Allow to continue looping
            hr = S_OK;
        }
    }

    _cwprintf(L"Cleaning up resources... ");

    // cleanup
    if( NULL != pFileServiceSecureEventNotify )
    {
        pFileServiceSecureEventNotify->Release();
        pFileServiceSecureEventNotify = NULL;
    }

    if( NULL != pFileServiceSecureProxy )
    {
        if( bSubscribedToEvents )
        {
            pFileServiceSecureProxy->UnsubscribeToFileChangeEvent();
        }

        pFileServiceSecureProxy->Release();
        pFileServiceSecureProxy = NULL;
    }

    _cwprintf(L"finished.\r\n");

    if( SUCCEEDED(hr) )
    {
        return 0;
    }

    return -1;
}
