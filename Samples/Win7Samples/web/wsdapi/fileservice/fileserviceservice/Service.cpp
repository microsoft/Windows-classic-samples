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
#include "Service.h"
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
    __out_bcount(cbDst) LPWSTR pszDst, 
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

//////////////////////////////////////////////////////////////////////////////
// CFileServiceService Class
//////////////////////////////////////////////////////////////////////////////
CFileServiceService::CFileServiceService()
:   m_cRef(1)
{
}

HRESULT CFileServiceService::Init(LPCWSTR pszFileDirectory)
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

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceService::GetFile
//      Service method which returns the contents of the file specified 
//      from the service's files directory as an attachment back to the 
//      client.  
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CFileServiceService::GetFile(
    GET_FILE_REQUEST* pParameters, 
    GET_FILE_RESPONSE** ppResponse) 
{
    HRESULT hr = S_OK;
    WCHAR szRoot[MAX_PATH];
    WCHAR szFile[MAX_PATH];
    HANDLE hFile = NULL;
    IWSDOutboundAttachment* pAttachment = NULL;
    CSendAttachmentThread* pSendAttachmentThread = NULL;
    GET_FILE_RESPONSE* pGetFileResponse = NULL;

    if( NULL == pParameters->filePath )
    {
        return E_INVALIDARG;
    }

    if( NULL == ppResponse )
    {
        return E_POINTER;
    }

    *ppResponse = NULL;

    _cwprintf(L"Client is requesting file '%s'\r\n", pParameters->filePath);

    _cwprintf(L"    Assembling path... ");

    // for security, strip path information from the filename that was
    // sent from the client
    hr = StripCbPath( szRoot, sizeof(szRoot), pParameters->filePath );

    if( S_OK == hr )
    {
        // concatenate server file directory and root file
        hr = ::StringCbCopyW( szFile, sizeof(szFile), m_szFileDirectory );
    }

    if( S_OK == hr )
    {
        hr = ::StringCbCatW( szFile, sizeof(szFile), szRoot );
    }

    print_result( hr );

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
        } 
        print_result( hr );
    }

    if( S_OK == hr )
    {
        // File exists, discard handle since we don't need it anymore
        (void)::CloseHandle( hFile );

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

    // Start the worker thread
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

    if( NULL != pGetFileResponse )
    {
        WSDFreeLinkedMemory( pGetFileResponse );
        pGetFileResponse = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CFileServiceService::GetFileList  (a.k.a. 'dir' functionality)
//      Service method which returns the list of files that exists in the
//      directory specified on the command lined.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CFileServiceService::GetFileList(
    GET_FILE_LIST_RESPONSE** ppResponse) 
{
    HRESULT hr = S_OK;
    WCHAR szFilter[MAX_PATH];    
    GET_FILE_LIST_RESPONSE* pResponse = NULL;
    PWCHAR_LIST** ppCursor = NULL;
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = NULL;
    DWORD dwErr = 0;

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
        hr = CreateCFileServiceEventSource(
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
    DWORD dwErr = 0;

    //
    // Immediately start a new thread to send the attachment
    //
    // Use QueueUserWorkItem here because the attachment threads are created
    // often and die quickly.  Also, the threadpool cap will help keep from
    // spawning too many threads.
    //
    if( 0 == ::QueueUserWorkItem( StaticThreadProc, 
        (LPVOID*)this, WT_EXECUTELONGFUNCTION ))
    {
        dwErr = ::GetLastError();
        return HRESULT_FROM_WIN32( dwErr );
    }

    return S_OK;
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

//////////////////////////////////////////////////////////////////////////////
// Usage
//////////////////////////////////////////////////////////////////////////////
void Usage(LPCWSTR pszAdditionalInformation)
{
    _cwprintf(L"FileServiceService.exe <files-directory> [<device-address>]\r\n");

    if( NULL != pszAdditionalInformation )
    {
        _cwprintf( L"%s\r\n", pszAdditionalInformation );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Main Entry Point
//      argv[0] = executable name
//      argv[1] = files-directory - the directory from which to get the files
//      argv[2] = device address (optional) 
//////////////////////////////////////////////////////////////////////////////
int _cdecl wmain(
    int argc, 
    __in_ecount(argc) LPWSTR* argv)
{
    HRESULT hr = S_OK;
    LPCWSTR pszAdditionalInformation = NULL;
    WCHAR szFileDirectory[MAX_PATH];
    WCHAR szDeviceAddress[MAX_PATH];
    UUID uuid;
    size_t cchFileDirectoryLength = 0;
    HANDLE hDir = NULL;
    CFileChangeNotificationThread* pFileChangeThread = NULL;
    CFileServiceService* pFileService = NULL;
    IFileService* pIFileService = NULL;
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

    // copy the file-directory from first argument (i.e., argv[1])
    if( S_OK == hr )
    {
        hr = ::StringCbCopyW(szFileDirectory, sizeof(szFileDirectory),
                argv[1]);
    }
    
    // add a backslash to path if it doesn't have one
    if( S_OK == hr )
    {
        hr = ::StringCchLengthW(szFileDirectory, sizeof(szFileDirectory), 
            &cchFileDirectoryLength);
    }

    if( S_OK == hr && cchFileDirectoryLength < 1 )
    {
        pszAdditionalInformation =
            L"<files-directory> must have non-zero length";

        hr = E_INVALIDARG;
    }

    if( S_OK == hr && szFileDirectory[cchFileDirectoryLength - 1] != L'\\')
    {
        hr = ::StringCbCatW(szFileDirectory, sizeof(szFileDirectory), L"\\");
    }

    // Check if files-directory actually exists
    if( S_OK == hr )
    {
        hDir = ::CreateFileW(szFileDirectory, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (hDir == INVALID_HANDLE_VALUE)
        {
            hDir = NULL;
            pszAdditionalInformation = L"<files-directory> does not exist.";
            hr = E_INVALIDARG;
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

    if( S_OK == hr )
    {
        // If the device address is specified, copy it
        if( argc > 2 )
        {
            hr = ::StringCbCopyW( szDeviceAddress,
                    sizeof(szDeviceAddress), argv[2] );
        }
        else
        {
            // Otherwise, generate an ID for the host
            RPC_STATUS st = UuidCreate( &uuid );
            if( st != RPC_S_OK )
            {
                hr = E_FAIL;
            }

            if( S_OK == hr )
            {
                hr = StringCbPrintfW(
                    szDeviceAddress, sizeof(szDeviceAddress),
                    L"urn:uuid:%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    uuid.Data1, uuid.Data2, uuid.Data3,
                    uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
                    uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
            }
        }
    }

    if( S_OK != hr )
    {
        Usage( pszAdditionalInformation );
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////
    // Build our FileService service
    //////////////////////////////////////////////////////////////////////////
    if( S_OK == hr )
    {
        _cwprintf(L"Creating the file service... ");
        pFileService = new CFileServiceService();
        
        if( NULL == pFileService )
        {
            hr = E_OUTOFMEMORY;
        }
        print_result( hr );
    }

    if( S_OK == hr )
    {
        _cwprintf(L"Initializing the file service... ");
        hr = pFileService->Init( szFileDirectory );
        print_result( hr );
    }

    // Retrieve an IFileService pointer to pass into CreateFileServiceHost
    if( S_OK == hr )
    {
        _cwprintf(L"Querying file service for service interface... ");
        hr = pFileService->QueryInterface( __uuidof(IFileService),
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
        hr = CreateFileServiceHost( szDeviceAddress, &thisDeviceMetadata,
                pIFileService, &pHost, NULL );
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
