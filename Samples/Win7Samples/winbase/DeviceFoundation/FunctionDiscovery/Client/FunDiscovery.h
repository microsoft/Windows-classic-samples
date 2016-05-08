// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//////////////////////////////////////////////////////////////////////
//
// CMyFDHelper.h: interface for the CMyFDHelper class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "functiondiscovery.h"

class CMyFDHelper : public CFunctionDiscoveryNotificationWrapper
{
public:

    CMyFDHelper();
    virtual ~CMyFDHelper();

    HRESULT Initialize( );
    HRESULT ListFunctionInstances( const WCHAR* pszCategory );
    HRESULT WaitForChange( DWORD dwTimeout,
        const WCHAR* pszCategory,
        QueryUpdateAction eAction );

    // IUnknown implementation
public:
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv)
    {
        if (riid == __uuidof(IFunctionDiscoveryNotification) ||
            riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IFunctionDiscoveryNotification *>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHOD_(DWORD, AddRef)()
    {
        return ::InterlockedIncrement((LONG *)(&m_lRefCount));
    }

    STDMETHOD_(DWORD, Release)()
    {
        LONG c = ::InterlockedDecrement((LONG *)(&m_lRefCount));

        if( 0 == c )
            delete this;

        return c;
    }

    //
    // Implementation of IFunctionDiscoveryNotification
    // These are callback methods
    //

    //
    // Called when notification occurs
    //
    virtual HRESULT STDMETHODCALLTYPE OnUpdate(
        QueryUpdateAction enumQueryUpdateAction,
        __RPC__in_opt FDQUERYCONTEXT fdqcQueryContext,
        __RPC__in_opt IFunctionInstance *pIFunctionInstance);

    //
    // Called when an error occurs
    //
    virtual HRESULT STDMETHODCALLTYPE OnError(
        HRESULT hr,
        __RPC__in_opt FDQUERYCONTEXT fdqcQueryContext,
        __RPC__in const WCHAR *pszProvider);

    //
    // Called when some event occurs.  For example,
    // when some providers have finished searching the network
    // they will send an OnEvent message.
    //
    virtual HRESULT STDMETHODCALLTYPE OnEvent(
        DWORD dwEventID,
        __RPC__in_opt FDQUERYCONTEXT fdqcQueryContext,
        __RPC__in const WCHAR *pszProvider);



protected:
    HRESULT DisplayProperties( IPropertyStore * pPStore );

    // Attributes
protected:
    ULONG m_lRefCount;

    HANDLE m_hAdd;
    HANDLE m_hRemove;
    HANDLE m_hChange;

    IFunctionDiscovery * m_pFunDisc;
};

