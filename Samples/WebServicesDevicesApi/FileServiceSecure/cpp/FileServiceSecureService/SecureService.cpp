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
#include "SecureService.h"
#include <strsafe.h>

//////////////////////////////////////////////////////////////////////////////
// print_result - Displays the HRESULT
//////////////////////////////////////////////////////////////////////////////
void print_result( HRESULT hr )
{
    if( hr == S_OK )
    {
        _cwprintf(L"[S_OK]\r\n");
    }
    else 
    {
        _cwprintf(L"[ERROR: %x]\r\n", hr);
    }
}

//////////////////////////////////////////////////////////////////////////////
// StripCbPath - Strips path info from a filename
//////////////////////////////////////////////////////////////////////////////
HRESULT StripCbPath(
    _Out_writes_bytes_(cbDst) LPWSTR pszDst, 
    size_t cbDst, 
    LPCWSTR pszSrc)
{
    LPCWSTR pszBackSlash = NULL;
    LPCWSTR pszSlash = NULL;
    LPCWSTR psz = NULL;

    if( NULL == pszSrc )
    {
        return E_INVALIDARG;
    }

    pszBackSlash = wcsrchr(pszSrc, '\\');
    if( NULL != pszBackSlash )
    {
        pszBackSlash++;
    }

    pszSlash = wcsrchr(pszSrc, '/');
    if( NULL != pszSlash )
    {
        pszSlash++;
    }

    psz = (pszBackSlash > pszSlash ? pszBackSlash : pszSlash);
    if( NULL == psz )
    {
        psz = pszSrc;
    }

    return ::StringCbCopyW(pszDst, cbDst, psz);
}

//////////////////////////////////////////////////////////////////////////////
// CloneString - Allocates memory (via WSD Linked Memory) and copies a string
//////////////////////////////////////////////////////////////////////////////
HRESULT CloneString(
    LPCWSTR pszSrc,
    LPCWSTR *ppszDst)
{
    HRESULT hr = S_OK;
    LPWSTR pszDst = NULL;
    size_t len = 0;
    size_t cchDst = 0;

    if( NULL == pszSrc )
    {
        return E_INVALIDARG;
    }

    if( NULL == ppszDst )
    {
        return E_POINTER;
    }

    *ppszDst = NULL;

    hr = ::StringCchLengthW( pszSrc, MAX_PATH, &len );

    if( S_OK == hr )
    {
        cchDst = len + 1;
        pszDst = (LPWSTR)WSDAllocateLinkedMemory( NULL,
                cchDst * sizeof(WCHAR) );

        if( NULL == pszDst )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( S_OK == hr )
    {
        hr = ::StringCchCopyW( pszDst, cchDst, pszSrc );
    }

    // cleanup
    if( S_OK != hr )
    {
        if( NULL != pszDst )
        {
            WSDFreeLinkedMemory( pszDst );
            pszDst = NULL;
        }
    }

    *ppszDst = pszDst;

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CreateStringList - Allocates a string list object for a string
//      The resulting list object will eventually be used in a linked list,
//      but this routine does not chain the objects together.
//
//      The outparam ppList will contain the following members:
//          Element (cloned from pszItem, and linked to ppList)
//          Next    (NULL, but will be modified by caller to point to next
//                   PWCHAR_LIST in the linked list)
//
//////////////////////////////////////////////////////////////////////////////
HRESULT CreateStringList(
    LPCWSTR pszItem,
    PWCHAR_LIST **ppList)
{
    PWCHAR_LIST* pList = NULL;
    HRESULT hr = S_OK;
   
    if( NULL == pszItem )
    {
        return E_INVALIDARG;
    }

    if( NULL == ppList )
    {
        return E_POINTER;
    }

    *ppList = NULL;

    // Allocate and zero the linked-list container structure first
    if( S_OK == hr )
    {
        pList = (PWCHAR_LIST*)WSDAllocateLinkedMemory( NULL, sizeof(PWCHAR_LIST) );

        if( NULL == pList )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // NULL the Next field and clone a string into the Element field
    if( S_OK == hr )
    {
        pList->Next = NULL;
        hr = CloneString( pszItem, &pList->Element );
    }

    // Attach the Element item to pList
    if( S_OK == hr )
    {
        WSDAttachLinkedMemory( (void*)pList, (void*)pList->Element );
    }

    // If all was successful, set outparam and clear local variable
    if( S_OK == hr )
    {
        *ppList = pList;
        pList = NULL;
    }

    // cleanup
    if( NULL != pList )
    {
        WSDFreeLinkedMemory( pList );
        pList = NULL;
    }

    return hr;
}

typedef enum _UserTokenMode
{
    CertificateBased,
    HttpBased
} UserTokenMode;

//////////////////////////////////////////////////////////////////////////////
// GetUserToken - Given a WSD_EVENT, retrieve the user token present in the
//      event.  The function retrieves a certificate-based user token if
//      tokenMode is CertificateBased.  Otherwise, the function retrieves
//      an Negotiate HTTP Authenticated user token.
//
//      Returns one of the following:
//      - S_OK along with the user token if the retrieval is
//        successful.
//      - S_FALSE if no user token is present in the event.
//      - E_INVALIDARG if wsdEvent is NULL.
//      - E_POINTER if phUserToken is NULL.
//      - E_FAIL or other appropriate code if a failure occured while
//        attempting to retrieve the user token.
//////////////////////////////////////////////////////////////////////////////
HRESULT GetUserToken(
    WSD_EVENT* wsdEvent,
    UserTokenMode tokenMode,
    HANDLE* phUserToken)
{
    HRESULT hr = S_OK;
    IWSDMessageParameters* pMessageParams = NULL; // shallow copy
    IWSDMessageParameters* pLowerParams = NULL;
    IWSDHttpMessageParameters* pHttpParams = NULL;
    IWSDSSLClientCertificate* pClientCert = NULL;
    IWSDHttpAuthParameters* pHttpAuthParams = NULL;
    HANDLE hUserToken = NULL;
    
    if ( NULL == wsdEvent )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == phUserToken )
    {
        hr = E_POINTER;
    }
    else
    {
        *phUserToken = NULL;
    }
    
    if ( S_OK == hr )
    {
        pMessageParams = wsdEvent->MessageParameters;

        if ( NULL == pMessageParams )
        {
            hr = E_FAIL;
        }
    }
    
    if ( S_OK == hr )
    {
        hr = pMessageParams->GetLowerParameters( &pLowerParams );
    }
    
    if ( S_OK == hr )
    {
        // The user token is going to be available in the HTTP message
        // parameters.  Since the traffic arrived via HTTP or HTTPS, we
        // expect such parameters to be available.
        hr = pLowerParams->QueryInterface(
                __uuidof(IWSDHttpMessageParameters),
                (void**)&pHttpParams);
    }
    
    if ( S_OK == hr )
    {
        if ( CertificateBased == tokenMode )
        {
            // Certificate-based user token will be available under
            // IWSDSSLClientCertificate.  If this interface is not
            // available, then no such user token is present.
        
            hr = pHttpParams->QueryInterface(
                    __uuidof(IWSDSSLClientCertificate),
                    (void**)&pClientCert);
            
            if ( S_OK == hr )
            {
                hr = pClientCert->GetMappedAccessToken( &hUserToken );
            }
            else if ( E_NOINTERFACE == hr )
            {
                hr = S_FALSE;
            }
        }
        else // if ( HttpBased == tokenMode )
        {
            // HTTP-based user token will be available under
            // IWSDHttpAuthParameters.  If this interface is not
            // available, then no such user token is present.
            
            hr = pHttpParams->QueryInterface(
                    __uuidof(IWSDHttpAuthParameters),
                    (void**)&pHttpAuthParams);
            
            if ( S_OK == hr )
            {
                hr = pHttpAuthParams->GetClientAccessToken( &hUserToken );
            }
            else if ( E_NOINTERFACE == hr )
            {
                hr = S_FALSE;
            }
        }
    }
    
    if ( S_OK == hr )
    {
        // Outside pointer now owns the token.
        *phUserToken = hUserToken;
        hUserToken = NULL;
    }
    
    if ( NULL != hUserToken )
    {
        CloseHandle( hUserToken );
        hUserToken = NULL;
    }
    
    if ( NULL != pHttpAuthParams )
    {
        pHttpAuthParams->Release();
        pHttpAuthParams = NULL;
    }
    
    if ( NULL != pClientCert )
    {
        pClientCert->Release();
        pClientCert = NULL;
    }
    
    if ( NULL != pHttpParams )
    {
        pHttpParams->Release();
        pHttpParams = NULL;
    }
    
    if ( NULL != pLowerParams )
    {
        pLowerParams->Release();
        pLowerParams = NULL;
    }
    
    pMessageParams = NULL; // shallow copy
    
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceSecureService Class
//////////////////////////////////////////////////////////////////////////////
CFileServiceSecureService::CFileServiceSecureService()
:   m_cRef(1), m_bIsAcceptCertAuth(FALSE), m_bIsAcceptHttpAuth(FALSE)
{
}

HRESULT CFileServiceSecureService::Init(
    LPCWSTR pszFileDirectory,
    BOOL bIsAcceptCertAuth,
    BOOL bIsAcceptHttpAuth)
{
    HRESULT hr = S_OK;

    // validate pszFileDirectory
    if( NULL == pszFileDirectory )
    {
        return E_INVALIDARG;
    }

    if( 0 == wcscmp( pszFileDirectory, L"" ) )
    {
        return E_INVALIDARG;
    }

    hr = ::StringCbCopyW( m_szFileDirectory, sizeof(m_szFileDirectory),
            pszFileDirectory );

    if ( S_OK == hr )
    {
        m_bIsAcceptCertAuth = bIsAcceptCertAuth;
        m_bIsAcceptHttpAuth = bIsAcceptHttpAuth;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceSecureService::QueryInterface
//      Implementation of IUnknown-related methods.
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CFileServiceSecureService::QueryInterface(
    REFIID riid, 
    void** ppvObject)
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
        if ( __uuidof(IFileServiceSecureService) == riid )
        {
            *ppvObject = (IFileServiceSecureService *)this;
        }
        else if ( __uuidof(IUnknown) == riid )
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
        ((LPUNKNOWN) *ppvObject)->AddRef();
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceSecureService::AddRef
//      Implementation of IUnknown-related methods.
//////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CFileServiceSecureService::AddRef()
{
    ULONG ulNewRefCount = (ULONG)InterlockedIncrement( (LONG *)&m_cRef );
    return ulNewRefCount;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceSecureService::Release
//      Implementation of IUnknown-related methods.
//////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CFileServiceSecureService::Release()
{
    ULONG ulNewRefCount = (ULONG)InterlockedDecrement( (LONG *)&m_cRef );

    if( 0 == ulNewRefCount )
    {
        delete this;
    }
    
    return ulNewRefCount;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceSecureService::GetFile
//      Service method which returns the contents of the file specified 
//      from the service's files directory as an attachment back to the 
//      client.  
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CFileServiceSecureService::GetFile(
    WSD_EVENT* wsdEvent,
    GET_FILE_REQUEST* pParameters, 
    GET_FILE_RESPONSE** ppResponse) 
{
    HRESULT hr = S_OK;
    HRESULT localHr = S_OK;
    BOOL bIsImpersonated = TRUE;
    WCHAR szRoot[MAX_PATH];
    WCHAR szFile[MAX_PATH];
    HANDLE hUserToken = NULL;
    HANDLE hFile = NULL;
    IWSDOutboundAttachment* pAttachment = NULL;
    CSendAttachmentThread* pSendAttachmentThread = NULL;
    GET_FILE_RESPONSE* pGetFileResponse = NULL;

    if( NULL == pParameters->filePath || NULL == wsdEvent )
    {
        hr = E_INVALIDARG;
    }
    else if( NULL == ppResponse )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppResponse = NULL;
    }

    if ( S_OK == hr )
    {
        _cwprintf(L"Client is requesting file '%s'\r\n", pParameters->filePath);
        _cwprintf(L"    Assembling path... ");

        // for security, strip path information from the filename that was
        // sent from the client
        hr = StripCbPath( szRoot, sizeof(szRoot), pParameters->filePath );
        
        if ( S_OK != hr )
        {
            print_result( hr );
        }
    }

    if( S_OK == hr )
    {
        // concatenate server file directory and root file
        hr = ::StringCbCopyW( szFile, sizeof(szFile), m_szFileDirectory );
        
        if ( S_OK != hr )
        {
            print_result( hr );
        }
    }

    if( S_OK == hr )
    {
        hr = ::StringCbCatW( szFile, sizeof(szFile), szRoot );
        print_result( hr );
    }
    
    if ( S_OK == hr && ( m_bIsAcceptCertAuth || m_bIsAcceptHttpAuth ) )
    {
        // Certificate or HTTP authentication is enabled for this service.
        // Attempt to retrieve the requested credentials beginning with the
        // certificate authentication.

        if ( m_bIsAcceptCertAuth )
        {
            _cwprintf(L"    Retrieving certificate-based user token... ");

            localHr = GetUserToken( wsdEvent, CertificateBased, &hUserToken );
            print_result( localHr );
        }
        
        if ( NULL == hUserToken && m_bIsAcceptHttpAuth )
        {
            _cwprintf(L"    Retrieving HTTP-based user token... ");
            
            localHr = GetUserToken( wsdEvent, HttpBased, &hUserToken );
            print_result( localHr );
        }
        
        if ( NULL == hUserToken )
        {
            // No user tokens available.  Access denied.
            _cwprintf(L"    Failed to retrieve a usable user token.  "
                    L"Denying access to this request.\r\n");
            
            hr = E_ACCESSDENIED;
        }
    }
    
    if ( S_OK == hr && NULL != hUserToken )
    {
        // Attempt to impersonate the user retrieved from the event.  Any
        // file operations that happen below will be executed through the
        // impersonated user.
        
        _cwprintf(L"    Attempting to impersonate this user... ");
        bIsImpersonated = ImpersonateLoggedOnUser( hUserToken );
        
        if ( !bIsImpersonated )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        
        print_result( hr );
    }

    // Check to see if file exists
    if( S_OK == hr )
    {
        _cwprintf(L"    Checking if file exists... ");
        hFile = ::CreateFileW( szFile, FILE_READ_DATA, 0, NULL, 
            OPEN_EXISTING, 0, NULL );

        if( INVALID_HANDLE_VALUE == hFile )
        {
            // File doesn't exist
            hr = E_FAIL;
            hFile = NULL;
        } 
        print_result( hr );
    }

    if( S_OK == hr )
    {
        // File exists, discard handle since we don't need it anymore
        CloseHandle( hFile );
        hFile = NULL;
    }

    //
    // Generate response structure to pass back as an outparam.  The
    // attachment will be linked inside, and WSDAPI will read from
    // this attachment as data is written to it inside the
    // CSendAttachmentThread.
    //
    if( S_OK == hr )
    {
        pGetFileResponse = (GET_FILE_RESPONSE*)
            WSDAllocateLinkedMemory(NULL, sizeof(GET_FILE_RESPONSE));

        if( NULL == pGetFileResponse )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Create an outbound attachment object for the file
    if( S_OK == hr )
    {
        _cwprintf(L"    Creating outbound attachment object... ");
        hr = WSDCreateOutboundAttachment( &pAttachment );
        print_result( hr );
    }

    // Create worker thread to send the attachment
    if( S_OK == hr )
    {
        pSendAttachmentThread = new CSendAttachmentThread();

        if( NULL == pSendAttachmentThread )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( S_OK == hr )
    {
        hr = pSendAttachmentThread->Init( szFile, pAttachment );
    }

    // Start the worker thread.  The worker thread will run under the
    // impersonated user context.
    if( S_OK == hr )
    {
        _cwprintf(L"    Starting thread to send %s as attachment... ",
                szFile);
        hr = pSendAttachmentThread->Start();

        //
        // Once Start has been called, the thread may delete itself of its
        // own volition.  Set our reference to NULL so we don't access
        // it after it has deleted itself.
        //
        pSendAttachmentThread = NULL;

        print_result( hr );
    }

    if( S_OK == hr )
    {
        // Hand our reference off to the response
        pGetFileResponse->Attachment = pAttachment;
        pAttachment = NULL;

        *ppResponse = pGetFileResponse; 
        pGetFileResponse = NULL;
    }
    
    // cleanup
    if( NULL != pGetFileResponse )
    {
        WSDFreeLinkedMemory( pGetFileResponse );
        pGetFileResponse = NULL;
    }
    
    if( NULL != pSendAttachmentThread )
    {
        delete pSendAttachmentThread;
        pSendAttachmentThread = NULL;
    }
    
    if( NULL != pAttachment )
    {
        pAttachment->Release();
        pAttachment = NULL;
    }
    
    if ( NULL != hFile )
    {
        CloseHandle( hFile );
        hFile = NULL;
    }
    
    // If we have previously impersonated the user, we need to revert back
    // to the original user before proceeding.
    
    if ( bIsImpersonated )
    {
        if ( !RevertToSelf() )
        {
            _cwprintf(L"Failed to revert to self: 0x%x\r\n", GetLastError());
        }
    }
    
    if ( NULL != hUserToken )
    {
        CloseHandle( hUserToken );
        hUserToken = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceSecureService::GetFileList  (a.k.a. 'dir' functionality)
//      Service method which returns the list of files that exists in the
//      directory specified on the command lined.
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CFileServiceSecureService::GetFileList(
    WSD_EVENT* wsdEvent,
    GET_FILE_LIST_RESPONSE** ppResponse) 
{
    HRESULT hr = S_OK;
    HRESULT localHr = S_OK;
    BOOL bIsImpersonated = TRUE;
    WCHAR szFilter[MAX_PATH];    
    GET_FILE_LIST_RESPONSE* pResponse = NULL;
    PWCHAR_LIST** ppCursor = NULL;
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = NULL;
    DWORD dwErr = 0;
    HANDLE hUserToken = NULL;

    if( NULL == ppResponse )
    {
        return E_POINTER;
    }

    *ppResponse = NULL;

    _cwprintf(L"Client is requesting file list:\r\n");

    _cwprintf(L"    Building directory filter... ");
    // Build wildcard directory filter
    hr = ::StringCbCopyW( szFilter, sizeof(szFilter), m_szFileDirectory );

    if( S_OK == hr )
    {
        hr = ::StringCbCatW( szFilter, sizeof(szFilter), (LPCWSTR)L"*.*" );
    }

    print_result( hr );
    
    // Allocate and zero response structure
    if( S_OK == hr )
    {
        //
        // pResponse will get returned as an outparameter, and our
        // configuration states that the Deallocator for outparams is
        // WSDFreeLinkedMemory.
        //
        pResponse = (GET_FILE_LIST_RESPONSE*)
            WSDAllocateLinkedMemory(NULL, sizeof(GET_FILE_LIST_RESPONSE));

        if (NULL == pResponse)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( S_OK == hr )
    {
        ::ZeroMemory(pResponse, sizeof(GET_FILE_LIST_RESPONSE));
        ppCursor = &pResponse->FileList;
    }
    
    if ( S_OK == hr && ( m_bIsAcceptCertAuth || m_bIsAcceptHttpAuth ) )
    {
        // Certificate or HTTP authentication is enabled for this service.
        // Attempt to retrieve the requested credentials beginning with the
        // certificate authentication.

        if ( m_bIsAcceptCertAuth )
        {
            _cwprintf(L"    Retrieving certificate-based user token... ");

            localHr = GetUserToken( wsdEvent, CertificateBased, &hUserToken );
            print_result( localHr );
        }
        
        if ( NULL == hUserToken && m_bIsAcceptHttpAuth )
        {
            _cwprintf(L"    Retrieving HTTP-based user token... ");
            
            localHr = GetUserToken( wsdEvent, HttpBased, &hUserToken );
            print_result( localHr );
        }
        
        if ( NULL == hUserToken )
        {
            // No user tokens available.  Access denied.
            _cwprintf(L"    Failed to retrieve a usable user token.  "
                    L"Denying access to this request.\r\n");
            
            hr = E_ACCESSDENIED;
        }
    }
    
    if ( S_OK == hr && NULL != hUserToken )
    {
        // Attempt to impersonate the user retrieved from the event.  Any
        // file operations that happen below will be executed through the
        // impersonated user.
        
        _cwprintf(L"    Attempting to impersonate this user... ");
        bIsImpersonated = ImpersonateLoggedOnUser( hUserToken );
        
        if ( !bIsImpersonated )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        
        print_result( hr );
    }

    // Seek to first file in directory list
    if( S_OK == hr )
    {
        hFind = ::FindFirstFileW( szFilter, &findFileData );

        if( INVALID_HANDLE_VALUE == hFind )
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
            hFind = NULL;
        }
    }

    // Iterate through the files in the directory
    while( S_OK == hr )
    {
        bool bAddFile = true;

        // Skip '.' and '..'
        if( 0 == wcscmp( findFileData.cFileName, L"." ) ||
            0 == wcscmp( findFileData.cFileName, L".." ) )
        {
            bAddFile = false;
        }

        if( bAddFile )
        {
            _cwprintf(L"    Adding file '%s' to list... ",
                    findFileData.cFileName );
            // Build list entry out of current file
            hr = CreateStringList( findFileData.cFileName, ppCursor );

            if( S_OK == hr )
            {
                // Add new list entry to end of list
                WSDAttachLinkedMemory( pResponse, *ppCursor );

                // Advance cursor
                ppCursor = &((*ppCursor)->Next);
            }

            print_result( hr );
        }

        // If we hit the end of the directory, bail out
        if( S_OK == hr && 0 == ::FindNextFileW( hFind, &findFileData ) )
        {
            break;
        }
    }

    if( S_OK == hr )
    {
        *ppResponse = pResponse;
        pResponse = NULL;

        _cwprintf(L"    List has been prepared.\r\n");
    }

    _cwprintf(L"    Service method exit code: ");
    print_result( hr );

    // cleanup
    if( NULL != hFind )
    {
        ::FindClose( hFind );
        hFind = NULL;
    }

    if( NULL != pResponse )
    {
        WSDFreeLinkedMemory( pResponse );
        pResponse = NULL;
    }
    
    // If we have previously impersonated the user, we need to revert back
    // to the original user before proceeding.
    
    if ( bIsImpersonated )
    {
        if ( !RevertToSelf() )
        {
            _cwprintf(L"Failed to revert to self: 0x%x\r\n", GetLastError());
        }
    }
    
    if ( NULL != hUserToken )
    {
        CloseHandle( hUserToken );
        hUserToken = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CFileChangeNotificationThread Class
//       Performs worker thread that monitors the file system for changes
//////////////////////////////////////////////////////////////////////////////
CFileChangeNotificationThread::CFileChangeNotificationThread()
:   m_pHost(NULL)
,   m_hThread(NULL)
,   m_hWait(NULL)
,   m_eventSource(NULL)
{
}

CFileChangeNotificationThread::~CFileChangeNotificationThread()
{
    // m_hWait and m_hThread are cleaned up in Stop
    if( NULL != m_eventSource )
    {
        m_eventSource->Release();
        m_eventSource = NULL;
    }

    if( NULL != m_pHost )
    {
        m_pHost->Release();
        m_pHost = NULL;
    }
}

HRESULT CFileChangeNotificationThread::Init(
    LPCWSTR pszDirectory, 
    LPCWSTR pszServiceId, 
    IWSDDeviceHost* pHost)
{
    HRESULT hr = S_OK;

    if( NULL == pszDirectory )
    {
        return E_INVALIDARG;
    }

    if( NULL == pszServiceId )
    {
        return E_INVALIDARG;
    }

    if( NULL == pHost )
    {
        return E_INVALIDARG;
    }

    m_pHost = pHost;
    m_pHost->AddRef();

    // Copy directory and serviceId parameters
    hr = ::StringCbCopyW( m_szDirectory, sizeof(m_szDirectory),
            pszDirectory);

    if( S_OK == hr )
    {
        hr = ::StringCbCopyW( m_szServiceId, sizeof(m_szServiceId),
                pszServiceId );
    }

    // Allocate and initialize event source object so we can send events to
    // clients when we are notified of file system changes
    if( S_OK == hr )
    {
        hr = CreateCFileServiceSecureEventSource(
                m_pHost,
                m_szServiceId,
                &m_eventSource );
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// Start - Starts the file change notification thread 
//////////////////////////////////////////////////////////////////////////////
HRESULT CFileChangeNotificationThread::Start()
{
    HRESULT hr = S_OK;
    DWORD dwErr = 0;

    //
    // Set up handle to shut down the notification thread
    // Parameters to CreateEvent:
    //     lpEventAttributes (NULL)     Ignored, must be NULL
    //     bManualReset (TRUE)          Event must be manually reset
    //     bInitialState (FALSE)        Event starts in unsignalled state
    //     lpName (NULL)                Event is unnamed
    //
    m_hWait = ::CreateEvent( NULL, TRUE, FALSE, NULL );

    if( NULL == m_hWait )
    {
        dwErr = ::GetLastError();
        hr = HRESULT_FROM_WIN32( dwErr );
    }

    //
    // Create thread to listen for file change notifications
    //
    // Use CreateThread here because this thread should be persistent
    // for the lifetime of the executable.
    //
    if( S_OK == hr )
    {
        m_hThread = ::CreateThread( 0, 0, StaticThreadProc, this, 0, 0 );

        // Failure code for CreateThread is NULL
        if( NULL == m_hThread )
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// Stop - Stops the file change notification thread 
//////////////////////////////////////////////////////////////////////////////
HRESULT CFileChangeNotificationThread::Stop()
{
    HRESULT hr = S_OK;
    DWORD dwErr = 0;

    // Signal thread to exit
    ::SetEvent( m_hWait );

    // Wait for thread to exit
    if( S_OK == hr )
    {
        if( WAIT_FAILED == ::WaitForSingleObject( m_hThread, INFINITE ) )
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }
    }

    // Clean up handles
    if( NULL != m_hThread )
    {
        ::CloseHandle( m_hThread );
        m_hThread = NULL;
    }

    if( NULL != m_hWait )
    {
        ::CloseHandle( m_hWait );
        m_hWait = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// ThreadProc - Performs the file change notification work
//////////////////////////////////////////////////////////////////////////////
HRESULT CFileChangeNotificationThread::ThreadProc()
{
    DWORD dwInfoSize = 0;
    FILE_NOTIFY_INFORMATION *pInfo = NULL;
    HRESULT hr = S_OK;
    WCHAR szFileName[MAX_PATH];
    HANDLE hDir = NULL;
    OVERLAPPED overlapped = { 0 };
    HANDLE waitHandles[2];
    DWORD dwErr = 0;

    const DWORD dwShareMode = 
        FILE_SHARE_READ | 
        FILE_SHARE_WRITE | 
        FILE_SHARE_DELETE;

    const DWORD dwFlags =
        FILE_FLAG_BACKUP_SEMANTICS |
        FILE_FLAG_OVERLAPPED;

    const DWORD dwNotifyFilter = 
        FILE_NOTIFY_CHANGE_FILE_NAME | 
        FILE_NOTIFY_CHANGE_LAST_WRITE | 
        FILE_NOTIFY_CHANGE_SIZE | 
        FILE_NOTIFY_CHANGE_ATTRIBUTES;

    // allocate a FILE_NOTIFY_INFORMATION object to receive notifications 
    dwInfoSize = sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH;

    pInfo = (FILE_NOTIFY_INFORMATION*)malloc(dwInfoSize);

    if( NULL == pInfo )
    {
        hr = E_OUTOFMEMORY;
    }

    // Open directory handle
    if( S_OK == hr )
    {
        hDir = ::CreateFileW( m_szDirectory, GENERIC_READ, 
            dwShareMode, NULL, OPEN_EXISTING, dwFlags, NULL );

        if( INVALID_HANDLE_VALUE == hDir )
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }
    }

    // Set up the OVERLAPPED structure for receiving async notifications
    if( S_OK == hr )
    {
        ::ZeroMemory( &overlapped, sizeof(OVERLAPPED) );

        overlapped.hEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );

        if( NULL == overlapped.hEvent )
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }
    }

    // Loop for change notifications
    while( S_OK == hr )
    {
        DWORD dwBytesReturned = 0;
        ZeroMemory( pInfo, dwInfoSize );

        //
        // Get information that describes the most recent file change
        // This call will return immediately and will set
        // overlapped.hEvent when it has completed
        //
        if( 0 == ::ReadDirectoryChangesW(hDir, pInfo, dwInfoSize, FALSE,
            dwNotifyFilter, &dwBytesReturned, &overlapped, NULL))
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }

        //
        // Set up a wait on the overlapped handle and on the handle used
        // to shut down this thread
        //
        if( S_OK == hr )
        {
            DWORD waitResult = 0;

            //
            // WaitForMultipleObjects will wait on both event handles in
            // waitHandles.  If either one of these events is signalled,
            // WaitForMultipleObjects will return and its return code
            // can be used to tell which event was set.
            //
            waitHandles[0] = m_hWait;
            waitHandles[1] = overlapped.hEvent;

            waitResult = ::WaitForMultipleObjects( 2, waitHandles, FALSE,
                    INFINITE );

            if( WAIT_FAILED == waitResult )
            {
                // Hit an error--bail out from this thread
                dwErr = ::GetLastError();
                hr = HRESULT_FROM_WIN32( dwErr );
            }
            else if( WAIT_OBJECT_0 == waitResult )
            {
                // Stop was called, and this thread should be shut down
                hr = S_FALSE;
                _cwprintf(L"Change notification thread shutting down.\r\n");
            }
            // Otherwise, a change notification occured, and we should
            // continue through the loop
        }

        // Retrieve result from overlapped structure
        if( S_OK == hr )
        {
            DWORD dwBytesTransferred = 0;

            if( 0 == ::GetOverlappedResult( hDir, &overlapped,
                        &dwBytesTransferred, TRUE ) )
            {
                dwErr = ::GetLastError();
                hr = HRESULT_FROM_WIN32( dwErr );
            }
        }
        
        if( S_OK == hr )
        {
            // Copy the file name into a message structure
            hr = ::StringCchCopyNW( szFileName, MAX_PATH,
                    pInfo->FileName, pInfo->FileNameLength );
        }

        if( S_OK == hr ) 
        {
            // Notify all clients subscribed for FileChange events
            NotifyClient( szFileName, pInfo->Action );
        }
    }

    // cleanup
    if( NULL != pInfo )
    {
        free( pInfo );
        pInfo = NULL;
    }

    if( NULL != overlapped.hEvent )
    {
        ::CloseHandle( overlapped.hEvent );
        overlapped.hEvent = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// NotifyClient - Notifies the client via the FileChangeEvent
//////////////////////////////////////////////////////////////////////////////
void CFileChangeNotificationThread::NotifyClient(
    LPCWSTR pszFileName, 
    DWORD dwAction)
{   
    LPCWSTR pszEventType = NULL;
    FILE_CHANGE_EVENT changeEvent;
    HRESULT hr = S_OK;

    //
    // First, select which file change type we want to send to the clients.
    // This will be packaged as a parameter for the message.
    //
    switch (dwAction)
    {
        case FILE_ACTION_ADDED:
            pszEventType = FileChangeEventType_Added;
            break;
        case FILE_ACTION_REMOVED:
            pszEventType = FileChangeEventType_Removed;
            break;
        case FILE_ACTION_MODIFIED:
            pszEventType = FileChangeEventType_Modified;
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            pszEventType = FileChangeEventType_RenamedOldName;
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            pszEventType = FileChangeEventType_RenamedNewName;
            break;
    }

    _cwprintf(L"Noticed file change: %s %s\r\n",
            pszEventType, pszFileName);

    //
    // Second, build the parameter structure and send the message to all
    // subscribed clients
    //
    _cwprintf(L"Sending event to clients... ");

    changeEvent.EventType = pszEventType;
    changeEvent.FileName = pszFileName;

    hr = m_eventSource->FileChangeEvent(&changeEvent);

    print_result( hr );

    // No return code--signalling events is best-effort
}

DWORD WINAPI CFileChangeNotificationThread::StaticThreadProc(LPVOID pParams)
{
    if( NULL != pParams ) 
    {
        // Ignore result
        (void)((CFileChangeNotificationThread*)pParams)->ThreadProc();
    }

    return 0;
}

CSendAttachmentThread::CSendAttachmentThread()
:   m_pAttachment(NULL)
{
}

CSendAttachmentThread::~CSendAttachmentThread()
{
    if( NULL != m_pAttachment )
    {
        m_pAttachment->Release();
        m_pAttachment = NULL;
    }
}

HRESULT CSendAttachmentThread::Init(
    LPCWSTR pszFilePath, 
    IWSDOutboundAttachment* pAttachment)
{
    HRESULT hr = S_OK;

    if( NULL == pAttachment )
    {
        return E_INVALIDARG;
    }

    m_pAttachment = pAttachment;
    m_pAttachment->AddRef();

    hr = ::StringCbCopyW( m_szFilePath, sizeof(m_szFilePath), pszFilePath );

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// Start - Starts the send-attachment thread via thread-pool thread
//////////////////////////////////////////////////////////////////////////////
HRESULT CSendAttachmentThread::Start()
{
    HRESULT hr = S_OK;
    BOOL bIsSuccess = TRUE;

    //
    // Immediately start a new thread to send the attachment
    //
    // Use QueueUserWorkItem here because the attachment threads are created
    // often and die quickly.  Also, the threadpool cap will help keep from
    // spawning too many threads.
    //
    bIsSuccess = QueueUserWorkItem(
            StaticThreadProc,
            (LPVOID*)this,
            WT_EXECUTELONGFUNCTION | WT_TRANSFER_IMPERSONATION );

    if ( !bIsSuccess )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// ThreadProc - Performs the send attachment work
//////////////////////////////////////////////////////////////////////////////
HRESULT CSendAttachmentThread::ThreadProc()
{
    HRESULT hr = S_OK;
    HANDLE hFile = NULL;
    BYTE buffer[8192];
    DWORD dwBytesRead = 0;
    DWORD dwBytesLeft = 0;
    DWORD dwErr = 0;

    _cwprintf(L"Sending file '%s' as attachment...\r\n", m_szFilePath);

    _cwprintf(L"    Opening file...");
    // Open the file to send
    hFile = ::CreateFileW(m_szFilePath, FILE_READ_DATA, 0, 
        NULL, OPEN_EXISTING, 0, NULL);

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hFile = NULL;
        dwErr = ::GetLastError();
        hr = HRESULT_FROM_WIN32( dwErr );
    }

    print_result( hr );

    // Loop through data in file until finished
    while( S_OK == hr )
    {
        _cwprintf(L"    Reading from file... ");
        // Read a block from the file
        if( 0 == ::ReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead, 0) )
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }

        dwBytesLeft = dwBytesRead;

        if( 0 == dwBytesLeft )
        {
            // No data left--time to bail out
            _cwprintf(L" done.\r\n");
            break;
        }

        print_result( hr );

        while( S_OK == hr && 0 < dwBytesLeft )
        {
            DWORD dwBytesWritten = 0;

            _cwprintf(L"    Writing to attachment... ");
            // Write multiple times until this block has been consumed
            hr = m_pAttachment->Write(
                    buffer + (dwBytesRead - dwBytesLeft),
                    dwBytesLeft,
                    &dwBytesWritten);

            print_result( hr );

            dwBytesLeft -= dwBytesWritten;
        }
    }

    // cleanup
    if( NULL != hFile )
    {
        ::CloseHandle( hFile );
        hFile = NULL;
    }

    if( S_OK == hr )
    {
        _cwprintf(L"    Finished reading file.  Closing attachment... ");
        hr = m_pAttachment->Close();
        print_result( hr );
    }
    else
    {
        _cwprintf(L"    Error reading file.  Aborting attachment... ");
        m_pAttachment->Abort();
        print_result( hr );
    }

    _cwprintf(L"    Attachment thread exiting.\r\n");

    delete this;

    return hr;
}

DWORD WINAPI CSendAttachmentThread::StaticThreadProc(LPVOID pParams)
{
    if( NULL != pParams )
    {
        // Ignore result
        (void)((CSendAttachmentThread*)pParams)->ThreadProc();
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Argument parsing function
//////////////////////////////////////////////////////////////////////////////
HRESULT ParseArgs(
    int argc,
    _In_reads_(argc) LPWSTR* argv,
    BOOL* pbIsCertAuth,
    BOOL* pbIsHttpAuth,
    BOOL* pbIsCertOrHttpAuth,
    _In_reads_(cchFileDirectoryBufferSize) LPWSTR pszFileDirectory,
    size_t cchFileDirectoryBufferSize,
    _In_reads_(cchDeviceAddressBufferSize) LPWSTR pszDeviceAddress,
    size_t cchDeviceAddressBufferSize)
{
    HRESULT hr = S_OK;
    
    BOOL bIsHttpAuth = FALSE;
    BOOL bIsCertAuth = FALSE;
    BOOL bIsCertOrHttpAuth = FALSE;
    LPWSTR pszTempFileDirectory = NULL; // shallow copy
    LPWSTR pszTempDeviceAddress = NULL; // shallow copy
    
    size_t cchFileDirectoryLength = 0;
    UUID uuid = {0};
    
    if ( argc < 2 || argc > 6 || NULL == argv )
    {
        // argv[0] = executable name
        //
        // one additional argument required at the minimum for the
        // files directory
        //
        // maximum is (1 exe name) + (all 5 args) = 6
        hr = E_INVALIDARG;
    }
    else if ( NULL == pbIsCertAuth || NULL == pbIsHttpAuth
            || NULL == pbIsCertOrHttpAuth )
    {
        hr = E_POINTER;
    }
    else if ( NULL == pszFileDirectory || cchFileDirectoryBufferSize < 1
            || NULL == pszDeviceAddress || cchDeviceAddressBufferSize < 1 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *pbIsCertAuth = FALSE;
        *pbIsHttpAuth = FALSE;
        *pbIsCertOrHttpAuth = FALSE;
        
        ::ZeroMemory( pszFileDirectory, 
                cchFileDirectoryBufferSize * sizeof(WCHAR) );
        
        ::ZeroMemory( pszDeviceAddress, 
                cchDeviceAddressBufferSize * sizeof(WCHAR) );
    }
    
    for ( int i = 1; i < argc && S_OK == hr; i++ ) // skip argv[0] for exe name
    {
        if ( 0 == _wcsicmp( argv[i], L"-CertAuth" ) )
        {
            bIsCertAuth= TRUE;
        }
        else if ( 0 == _wcsicmp( argv[i], L"-HttpAuth" ) )
        {
            bIsHttpAuth = TRUE;
        }
        else if ( 0 == _wcsicmp( argv[i], L"-CertOrHttpAuth" ) )
        {
            bIsCertOrHttpAuth = TRUE;
        }
        else if ( NULL == pszTempFileDirectory )
        {
            pszTempFileDirectory = argv[i];
        }
        else if ( NULL == pszTempDeviceAddress )
        {
            pszTempDeviceAddress = argv[i];
        }
        else
        {
            // Unknown argument
            hr = E_INVALIDARG;
        }
    }
    
    if ( S_OK == hr && NULL == pszTempFileDirectory )
    {
        // File Directory is required.
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
    
    if ( S_OK == hr )
    {
        // Determine if there is enough buffer space to hold the
        // directory string, including the ending backslash if one is not
        // already present.
        if ( '\\' != pszTempFileDirectory[cchFileDirectoryLength - 1] )
        {
            // Backslash required at the end.
            
            if ( cchFileDirectoryBufferSize < (cchFileDirectoryLength + 2) )
            {
                // + 1 for slash, + 1 for NULL char
                hr = E_INVALIDARG;
            }
        }
        else
        {
            // Backslash is already present.
            if ( cchFileDirectoryBufferSize < (cchFileDirectoryLength + 1) )
            {
                // + 1 for NULL char
                hr = E_INVALIDARG;
            }
        }
    }
    
    // Return the variables to the caller.
    
    if ( S_OK == hr )
    {
        if ( NULL == pszTempDeviceAddress )
        {
            // Device address not specified.
            // Generate a UUID and return that as the device address.
            
            RPC_STATUS st = UuidCreate( &uuid );
            
            if( RPC_S_OK != st )
            {
                hr = E_FAIL;
            }
            else
            {
                hr = StringCbPrintfW(
                        pszDeviceAddress, 
                        cchDeviceAddressBufferSize * sizeof(WCHAR),
                        L"urn:uuid:%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        uuid.Data1, uuid.Data2, uuid.Data3,
                        uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], 
                        uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], 
                        uuid.Data4[6], uuid.Data4[7]);
            }
        }
        else
        {
            // Device address specified.  Copy the string into the buffer.
            hr = StringCbPrintfW(
                    pszDeviceAddress,
                    cchDeviceAddressBufferSize * sizeof(WCHAR),
                    L"%s", pszTempDeviceAddress );
                    
        }
    }
    
    if ( S_OK == hr )
    {
        if ( '\\' != pszTempFileDirectory[cchFileDirectoryLength - 1] )
        {
            // Backslash required at the end.
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
        *pbIsCertAuth = bIsCertAuth;
        *pbIsHttpAuth = bIsHttpAuth;
        *pbIsCertOrHttpAuth = bIsCertOrHttpAuth;
    }
    
    pszTempFileDirectory = NULL; // shallow copy
    pszTempDeviceAddress = NULL; // shallow copy
    
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// Usage
//////////////////////////////////////////////////////////////////////////////
void Usage()
{
    _cwprintf(L"FileServiceSecureService.exe [-CertAuth] [-HttpAuth] "
            L"[-CertOrHttpAuth] <files-directory> [<device-address>]\r\n");
    _cwprintf(L"\r\n");
    _cwprintf(L"See the ReadMe file for a detailed usage description.\r\n");
}

//////////////////////////////////////////////////////////////////////////////
// Main Entry Point
//      argv[0]  = executable name
//      argv[1+] = application arguments
//////////////////////////////////////////////////////////////////////////////
int _cdecl wmain(
    int argc, 
    _In_reads_(argc) LPWSTR* argv)
{
    HRESULT hr = S_OK;
    WCHAR szFileDirectory[MAX_PATH] = {0};
    WCHAR szDeviceAddress[MAX_PATH] = {0};
    BOOL bIsCertAuth = FALSE;
    BOOL bIsHttpAuth = FALSE;
    BOOL bIsCertOrHttpAuth = FALSE;
    HANDLE hDir = NULL;
    CFileChangeNotificationThread* pFileChangeThread = NULL;
    CFileServiceSecureService* pFileService = NULL;
    IFileServiceSecureService* pIFileService = NULL;
    IWSDDeviceHost* pHost = NULL;
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

    if ( S_OK == hr )
    {
        hr = ParseArgs( argc, argv,
                &bIsCertAuth, &bIsHttpAuth, &bIsCertOrHttpAuth,
                szFileDirectory, MAX_PATH,
                szDeviceAddress, MAX_PATH );
    }
    
    if( S_OK != hr )
    {
        Usage();
        return -1;
    }

    // Check if files-directory actually exists
    if( S_OK == hr )
    {
        hDir = ::CreateFileW(szFileDirectory, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (hDir == INVALID_HANDLE_VALUE)
        {
            hDir = NULL;
            hr = E_INVALIDARG;
            
            _cwprintf(L"The files directory %s does not exist.\r\n",
                    szFileDirectory);
        }
    }

    if( S_OK == hr )
    {
        // If the handle could be opened, we have no more use for it
        if( 0 == ::CloseHandle( hDir ))
        {
            dwErr = ::GetLastError();
            hr = HRESULT_FROM_WIN32( dwErr );
        }

        hDir = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    // Build our FileService service
    //////////////////////////////////////////////////////////////////////////
    if( S_OK == hr )
    {
        _cwprintf(L"Creating the file service... ");
        pFileService = new CFileServiceSecureService();
        
        if( NULL == pFileService )
        {
            hr = E_OUTOFMEMORY;
        }
        print_result( hr );
    }

    if( S_OK == hr )
    {
        _cwprintf(L"Initializing the file service... ");

        hr = pFileService->Init( 
                szFileDirectory,
                bIsCertAuth || bIsCertOrHttpAuth,
                bIsHttpAuth || bIsCertOrHttpAuth );

        print_result( hr );
    }

    // Retrieve an IFileService pointer to pass into CreateFileServiceHost
    if( S_OK == hr )
    {
        _cwprintf(L"Querying file service for service interface... ");
        hr = pFileService->QueryInterface( __uuidof(IFileServiceSecureService),
                (void**)&pIFileService );
        print_result( hr );
    }

    //////////////////////////////////////////////////////////////////////////
    // Build the host
    //////////////////////////////////////////////////////////////////////////

    // Create host
    if( S_OK == hr )
    {
        _cwprintf(L"Creating file service host with ID %s... ",
                szDeviceAddress );

        hr = CreateFileServiceSecureHost( 
                bIsCertAuth,
                bIsHttpAuth,
                bIsCertOrHttpAuth,
                szDeviceAddress, 
                &thisDeviceMetadata,
                pIFileService, 
                &pHost, 
                NULL );

        print_result( hr );
    }

    // Start host
    if( S_OK == hr )
    {
        _cwprintf(L"Starting Host... ");
        hr = pHost->Start( 0, NULL, NULL );
        print_result( hr );
    }

    // Launch thread to listen for file change notifications and send events
    if( S_OK == hr )
    {
        _cwprintf(L"Creating file change notification thread... ");
        pFileChangeThread = new CFileChangeNotificationThread();

        if( NULL == pFileChangeThread )
        {
            hr = E_OUTOFMEMORY;
        }
        print_result( hr );
    }

    if( S_OK == hr )
    {
        // Use first hosted metadata service ID as serviceID
        _cwprintf(L"Initializing file change notification thread... ");
        hr = pFileChangeThread->Init( szFileDirectory,
                hostMetadata.Hosted->Element->ServiceId, pHost );
        print_result( hr );
    }

    if( S_OK == hr )
    {
        _cwprintf(L"Starting file change notification thread... ");
        hr = pFileChangeThread->Start();
        print_result( hr );
    }

    // Let service run
    if( S_OK == hr )
    {
        _cwprintf(L">>> Service running.  Press any key to stop service. <<<\r\n");
        // Ignore character returned from prompt
        (void)::_getch();
    }

    // Stop event notification thread first
    if( S_OK == hr )
    {
        _cwprintf(L"Stopping file change notification thread... ");
        hr = pFileChangeThread->Stop();
        print_result( hr );
    }

    // Stop host
    if( S_OK == hr )
    {
        _cwprintf(L"Stopping host... ");
        hr = pHost->Stop();
        print_result(hr);
    }

    _cwprintf(L"Cleaning up resources... ");

    // cleanup
    if( NULL != pIFileService )
    {
        pIFileService->Release();
        pIFileService = NULL;
    }

    if( NULL != pFileService )
    {
        pFileService->Release();
        pFileService = NULL;
    }

    if( NULL != pFileChangeThread )
    {
        // pFileChangeThread is not a refcounted object
        delete pFileChangeThread;
        pFileChangeThread = NULL;
    }

    if( NULL != pHost )
    {
        // Terminating the host is nonoptional, so it's done in the cleanup
        // block
        hr = pHost->Terminate();
        pHost->Release();
        pHost = NULL;
    }

    _cwprintf(L"finished.\r\n");

    if( SUCCEEDED(hr) )
    {
        return 0;
    }

    return -1;
}
