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
#include "FileServiceProxy.h"

//////////////////////////////////////////////////////////////////////////////
// CFileServiceEventNotify Class
//       Represents the notification sink object for the FileServiceEvent
//////////////////////////////////////////////////////////////////////////////
class CFileServiceEventNotify : public IFileServiceEventNotify
{
private:
    LONG m_cRef;

public:
    CFileServiceEventNotify();

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
        void **ppvObject)
    {
        if (NULL == ppvObject) 
        {
            return E_POINTER;
        }

        HRESULT hr = S_OK;
        *ppvObject = NULL;

        if ((__uuidof(IFileServiceEventNotify) == riid ) ||
            ( __uuidof(IUnknown) == riid)) 
        {
            *ppvObject = this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }

        if (SUCCEEDED(hr))
        {
            ((LPUNKNOWN)*ppvObject)->AddRef();
        }

        return hr;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement((LONG *)&m_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulNewRefCount = 
            (ULONG)InterlockedDecrement((LONG *)&m_cRef);
        
        if (0 == ulNewRefCount)
        {
            delete this;
        }
        return ulNewRefCount;
    }
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
    IFileServiceProxy* m_pFileServiceProxy;
    WCHAR m_szFile[MAX_PATH];

    ~CGetFileAsyncCallback();  // Called only from Release();

public:
    CGetFileAsyncCallback();

    HRESULT Init(
        IFileServiceProxy* pFileServiceProxy,
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
        void **ppvObject)
    {
        if (NULL == ppvObject) 
        {
            return E_POINTER;
        }

        HRESULT hr = S_OK;
        *ppvObject = NULL;

        if ((__uuidof(IWSDAsyncCallback) == riid ) ||
            ( __uuidof(IUnknown) == riid)) 
        {
            *ppvObject = this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }

        if (SUCCEEDED(hr))
        {
            ((LPUNKNOWN)*ppvObject)->AddRef();
        }

        return hr;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement((LONG *)&m_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulNewRefCount = 
            (ULONG)InterlockedDecrement((LONG *)&m_cRef);
        
        if (0 == ulNewRefCount)
        {
            delete this;
        }
        return ulNewRefCount;
    }
};
