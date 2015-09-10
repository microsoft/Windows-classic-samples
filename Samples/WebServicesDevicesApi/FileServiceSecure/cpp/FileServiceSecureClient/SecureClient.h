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

// FileServiceSecure.h is automatically generated from FileServiceSecure.idl
#include "FileServiceSecure.h"
#include "FileServiceSecureProxy.h"

//////////////////////////////////////////////////////////////////////////////
// CFileServiceSecureEventNotify Class
//       Represents the notification sink object for the FileServiceSecureEvent
//////////////////////////////////////////////////////////////////////////////
class CFileServiceSecureEventNotify : public IFileServiceSecureEventNotify
{
private:
    LONG m_cRef;

public:
    CFileServiceSecureEventNotify();

    //////////////////////////////////////////////////////////////////////////
    // FileChangeEvent - Invoked when the service sends this event
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE FileChangeEvent(
        FILE_CHANGE_EVENT* pFileChangeEvent);

public:
    //////////////////////////////////////////////////////////////////////////
    // Methods to make this class act like a COM object
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, 
        void **ppvObject);

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();
};

//////////////////////////////////////////////////////////////////////////////
// CGetFileAsyncCallback
//      Used to receive and process notification callbacks from
//      the GetFile asyncronous operation
//////////////////////////////////////////////////////////////////////////////
class CGetFileAsyncCallback : public IWSDAsyncCallback
{
private:
    LONG m_cRef;
    IFileServiceSecureProxy* m_pFileServiceSecureProxy;
    WCHAR m_szFile[MAX_PATH];

    ~CGetFileAsyncCallback();  // Called only from Release();

public:
    CGetFileAsyncCallback();

    HRESULT Init(
        IFileServiceSecureProxy* pFileServiceSecureProxy,
        LPCWSTR pszFileName,
        LPCWSTR pszReceiveDirectory);

    // Callback when a GetFile operation completes
    HRESULT STDMETHODCALLTYPE AsyncOperationComplete(
        IWSDAsyncResult* pAsyncResult,
        IUnknown* pAsyncState);

private:
    // Called from AsyncOperationComplete--saves an attachment as a local file
    HRESULT STDMETHODCALLTYPE ReceiveBinary(
        IWSDAttachment* pAttachment,
        LPCWSTR pszLocalFileName);

public:
    //////////////////////////////////////////////////////////////////////////
    // Methods to make this class act like a COM object
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, 
        void **ppvObject);

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();
};
