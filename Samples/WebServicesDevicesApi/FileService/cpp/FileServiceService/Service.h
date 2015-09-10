 //////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <wsdapi.h>

// FileService.h is automatically generated from FileService.idl
#include "FileService.h"
// Include FileServiceProxy for CFileServiceEventSource class
#include "FileServiceProxy.h"

WSD_LOCALIZED_STRING thisDeviceName = {NULL, L"File Service Device"};
WSD_LOCALIZED_STRING_LIST thisDeviceNameList = {NULL, &thisDeviceName};

// thisDeviceNameList is the FriendlyName
// "0.095" is the FirmwareVersion
// "0123456789-9876543210" is the SerialNumber

const WSD_THIS_DEVICE_METADATA thisDeviceMetadata = {
    &thisDeviceNameList,
    L"0.095",
    L"0123456789-9876543210",
}; 

//////////////////////////////////////////////////////////////////////////////
// CFileServiceService Class
//       Implements the service functionality
//////////////////////////////////////////////////////////////////////////////
class CFileServiceService : public IFileService
{
private:
    ULONG m_cRef;
    WCHAR m_szFileDirectory[MAX_PATH];

public:
    CFileServiceService();

    HRESULT Init(LPCWSTR pszFileDirectory);

    HRESULT STDMETHODCALLTYPE GetFile(
        GET_FILE_REQUEST* parameters, 
        GET_FILE_RESPONSE** parametersOut);

    HRESULT STDMETHODCALLTYPE GetFileList(
        GET_FILE_LIST_RESPONSE** parametersOut);

public:
    //////////////////////////////////////////////////////////////////////////
    // Methods to make this class act like a COM object
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, 
        void** ppvObject)
    {
        if (NULL == ppvObject)
        {
            return E_POINTER;
        }

        HRESULT hr = S_OK;
        *ppvObject = NULL;

        if (( __uuidof(IFileService) == riid) ||
            ( __uuidof(IUnknown) == riid) )
        {
            *ppvObject = (IFileService *)this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }

        if (SUCCEEDED(hr))
        {
            ((LPUNKNOWN) *ppvObject)->AddRef( );
        }

        return hr;
    } 

    ULONG STDMETHODCALLTYPE AddRef()
    {
        ULONG ulNewRefCount = (ULONG)InterlockedIncrement((LONG *)&m_cRef);
        return ulNewRefCount;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulNewRefCount = (ULONG)InterlockedDecrement((LONG *)&m_cRef);

        if( 0 == ulNewRefCount )
        {
            delete this;
        }
        return ulNewRefCount;
    } 
};

//////////////////////////////////////////////////////////////////////////////
// CFileChangeNotificationThread Class
//       Performs worker thread that monitors the file system for changes
//////////////////////////////////////////////////////////////////////////////
class CFileChangeNotificationThread
{
private:
    WCHAR m_szDirectory[MAX_PATH]; 
    IWSDDeviceHost* m_pHost;         
    WCHAR m_szServiceId[MAX_PATH]; 
    // Handle for the change notification thread
    HANDLE m_hThread;
    // Handle to signal shutdown of notification thread
    HANDLE m_hWait;
    CFileServiceEventSource* m_eventSource;

public:
    CFileChangeNotificationThread();
    ~CFileChangeNotificationThread();

    HRESULT Init(
        LPCWSTR pszDirectory, 
        LPCWSTR pszServiceId, 
        IWSDDeviceHost* pHost);

    // Starts the file change notification thread 
    HRESULT Start();

    // Stops the file change notification thread 
    HRESULT Stop();

    //////////////////////////////////////////////////////////////////////////
    // WINAPI wrapper so we can call ThreadProc() in a thread callback
    //////////////////////////////////////////////////////////////////////////
    static DWORD WINAPI StaticThreadProc(
        LPVOID pParams
    )
    {
        if( NULL != pParams ) 
        {
            // Ignore result
            (void)((CFileChangeNotificationThread*)pParams)->ThreadProc();
        }

        return 0;
    }

private:
    // Performs the file change notification work
    HRESULT ThreadProc();

    // Notifies the client via the FileChangeEvent
    void NotifyClient(
        LPCWSTR pszFileName, 
        DWORD dwAction);
};

//////////////////////////////////////////////////////////////////////////////
// CSendAttachmentThread Class
//       Performs worker thread sends an attachment back to the client
//////////////////////////////////////////////////////////////////////////////
class CSendAttachmentThread
{
private:
    WCHAR m_szFilePath[MAX_PATH];
    IWSDOutboundAttachment* m_pAttachment;

public:
    // ~CSendAttachmentThread is normally called only from ThreadProc(), but
    // must be public so CSendAttachmentThread can be cleaned up if an
    // initialization error occurs.
    ~CSendAttachmentThread();

    CSendAttachmentThread();

    HRESULT Init(
        LPCWSTR pszFilePath, 
        IWSDOutboundAttachment* pAttachment);

    // Starts the send-attachment thread via thread-pool thread
    HRESULT Start();

    //////////////////////////////////////////////////////////////////////////
    // WINAPI wrapper so we can call ThreadProc() in a thread callback
    //////////////////////////////////////////////////////////////////////////
    static DWORD WINAPI StaticThreadProc(
        LPVOID pParams)
    {
        if( NULL != pParams )
        {
            // Ignore result
            (void)((CSendAttachmentThread*)pParams)->ThreadProc();
        }
        return 0;
    }

private:
    // ThreadProc - Performs the send attachment work
    HRESULT ThreadProc();
};

