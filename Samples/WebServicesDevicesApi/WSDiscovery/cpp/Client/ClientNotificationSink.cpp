//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <windows.h>
#include "ClientNotificationSink.h"
#include "Common.h"

HRESULT CreateClientNotificationSink
(   _Outptr_ CClientNotificationSink **sink
)
{
    HRESULT hr = S_OK;
    CClientNotificationSink *tempSink = NULL;

    if ( NULL == sink )
    {
        hr = E_POINTER;
    }
    else
    {
        *sink = NULL;
    }

    if ( S_OK == hr )
    {
        tempSink = new CClientNotificationSink();

        if ( NULL == tempSink )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns the temporary reference
        *sink = tempSink;
        tempSink = NULL;
    }

    if ( NULL != tempSink )
    {
        tempSink->Release();
        tempSink = NULL;
    }

    return hr;
}

CClientNotificationSink::CClientNotificationSink()
:   m_cRef( 1 )
{
    InitializeCriticalSection( &m_printfCriticalSection );
}

CClientNotificationSink::~CClientNotificationSink()
{
    DeleteCriticalSection( &m_printfCriticalSection );
}

HRESULT STDMETHODCALLTYPE CClientNotificationSink::Add
(   _In_ IWSDiscoveredService *service
)
{
    HRESULT hr = S_OK;

    if ( NULL == service )
    {
        hr = E_INVALIDARG;
    }

    if ( S_OK == hr )
    {
        // We lock here to prevent other printing routines
        // from being called so that they do not distort the
        // output of the application.  Printing routines
        // that require this lock include the following methods:
        //
        // - Add (this method)
        // - Remove
        // - SearchFailed
        // - SearchComplete
        EnterCriticalSection( &m_printfCriticalSection );

        // In this sample, we print out target service information
        // whenever we receive it.  However, in reality, there may
        // be much more you have to do here.  An example will be the
        // scenario of a Devices Profile for Web Services (DPWS).
        // There may be instances of which the application may want
        // to create a Device Proxy against a discovered DPWS device.
        // In that case, the application shall keep a reference to
        // service and have it added to a queue so that this thread
        // is not blocked while WSDCreateDeviceProxy is executing.

        wprintf( L"Discovered the following service:\r\n" );
        
        // don't care if logging function fails
        (void)PrintDiscoveredService( service, FALSE ); // FALSE - this is not a Bye message
        wprintf( L"\r\n" );

        LeaveCriticalSection( &m_printfCriticalSection );
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CClientNotificationSink::Remove
(   _In_ IWSDiscoveredService *service
)
{
    HRESULT hr = S_OK;

    if ( NULL == service )
    {
        hr = E_INVALIDARG;
    }

    if ( S_OK == hr )
    {
        EnterCriticalSection( &m_printfCriticalSection );

        // See comments above.

        wprintf( L"The following service went offline:\r\n" );
        // don't care if logging function fails
        (void)PrintDiscoveredService( service, TRUE ); // TRUE - this is a Bye message
        wprintf( L"\r\n" );

        LeaveCriticalSection( &m_printfCriticalSection );
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CClientNotificationSink::SearchFailed
(   _In_ HRESULT hr
,   _In_opt_ LPCWSTR tag
)
{
    EnterCriticalSection( &m_printfCriticalSection );

    wprintf( L"----------------------------------------\r\n" );
    wprintf( L"The search request has failed...\r\n" );

    if ( S_FALSE == hr )
    {
        wprintf( L"Return Code: hr = S_FALSE (no matching target services found)\r\n" );
    }
    else
    {
        wprintf( L"Return Code: hr = 0x%08X\r\n", hr );
    }

    wprintf( L"Tag: %s\r\n", tag );
    wprintf( L"\r\n" );
    wprintf( L"The client shall continue to listen to\r\n" );
    wprintf( L"Hello and Bye messages.\r\n");
    wprintf( L"\r\n" );
    wprintf( L"Press any key to terminate...\r\n" ); // _getch() is in main()
    wprintf( L"----------------------------------------\r\n" );
    wprintf( L"\r\n" );

    LeaveCriticalSection( &m_printfCriticalSection );

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CClientNotificationSink::SearchComplete
(   _In_opt_ LPCWSTR tag
)
{
    EnterCriticalSection( &m_printfCriticalSection );

    wprintf( L"----------------------------------------\r\n" );
    wprintf( L"The search request has successfully completed.\r\n" );
    wprintf( L"Tag: %s\r\n", tag );
    wprintf( L"\r\n" );
    wprintf( L"The client shall continue to listen to\r\n" );
    wprintf( L"Hello and Bye messages.\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"Press any key to terminate...\r\n" ); // _getch() is in main()
    wprintf( L"----------------------------------------\r\n" );
    wprintf( L"\r\n" );

    LeaveCriticalSection( &m_printfCriticalSection );

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CClientNotificationSink::QueryInterface
(   _In_ REFIID riid
,   _Outptr_ __RPC__deref_out void __RPC_FAR *__RPC_FAR *object
)
{
    HRESULT hr = S_OK;

    if ( NULL == object )
    {
        hr = E_POINTER;
    }

    if ( S_OK == hr )
    {
        *object = NULL;

        if ( __uuidof(IWSDiscoveryProviderNotify) == riid )
        {
            *object = static_cast<IWSDiscoveryProviderNotify *>( this );
        }
        else if ( __uuidof(IUnknown) == riid )
        {
            *object = static_cast<IUnknown *>( this );
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    if ( S_OK == hr )
    {
        ((LPUNKNOWN) *object)->AddRef();
    }

    return hr;
}

ULONG STDMETHODCALLTYPE CClientNotificationSink::AddRef()
{
    ULONG newRefCount = (ULONG)InterlockedIncrement( (LONG *)&m_cRef );
    return newRefCount;
}

ULONG STDMETHODCALLTYPE CClientNotificationSink::Release()
{
    ULONG newRefCount = (ULONG)InterlockedDecrement( (LONG *)&m_cRef );

    if ( 0 == newRefCount )
    {
        delete this;
    }

    return newRefCount;
}
