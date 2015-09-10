//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <wsdapi.h>

class CClientNotificationSink;

//////////////////////////////////////////////////////////////////////////////
// Factory function to create a CClientNotificationSink object
//////////////////////////////////////////////////////////////////////////////

// Creates a client notification sink.
// The caller should call the Release() method
// of the sink if it is no longer needed.
HRESULT CreateClientNotificationSink
(   _Outptr_ CClientNotificationSink **sink
);

//////////////////////////////////////////////////////////////////////////////
// CClientNotificationSink Class
//       Implements the client-side callback functionality
//////////////////////////////////////////////////////////////////////////////
class CClientNotificationSink : public IWSDiscoveryProviderNotify
{
public:
    //////////////////////////////////////////////////////////////////////////
    // IWSDiscoveryProviderNotify methods for the client-side callback
    // routines.  
    //////////////////////////////////////////////////////////////////////////

    // Add is called when a Hello, ResolveMatches, or ProbeMatches message is processed.
    HRESULT STDMETHODCALLTYPE Add
    (   _In_ IWSDiscoveredService *service
    );
    
    // Remove is called when a Bye is processed.
    HRESULT STDMETHODCALLTYPE Remove
    (   _In_ IWSDiscoveredService *service
    );
    
    // SearchFailed is called when the search request initiated by
    // IWSDiscoveryProvider failed, which includes the case when
    // no target services matches the Probe or Resolve messages.
    // This method is typically called after 10 seconds of the initial
    // query.  See MSDN documentation for details.
    HRESULT STDMETHODCALLTYPE SearchFailed
    (   _In_ HRESULT hr
    ,   _In_opt_ LPCWSTR tag
    );
    
    // SearchComplete is called when the search request initiated by
    // IWSDiscoveryProvider has successfully been processed.
    // This method is typically called after 10 seconds of the initial
    // query.  See MSDN documentation for details.
    HRESULT STDMETHODCALLTYPE SearchComplete
    (   _In_opt_ LPCWSTR tag
    );

public:
    //////////////////////////////////////////////////////////////////////////
    // IUnknown (extended by IWSDiscoveryProviderNotify)
    // methods to make this class act like a COM object.
    //
    // See MSDN documentation.
    //////////////////////////////////////////////////////////////////////////

    HRESULT STDMETHODCALLTYPE QueryInterface
    (   _In_ REFIID riid
    ,   _Outptr_ __RPC__deref_out void __RPC_FAR *__RPC_FAR *object
    );

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();

public:
    //////////////////////////////////////////////////////////////////////////
    // CClientNotificationSink constructor.
    //////////////////////////////////////////////////////////////////////////
    CClientNotificationSink();

private:
    //////////////////////////////////////////////////////////////////////////
    // CClientNotificationSink destructor.
    //////////////////////////////////////////////////////////////////////////

    // The destructor should only be called by the Release method
    // implemented through IUnknown.
    ~CClientNotificationSink();

private:
    //////////////////////////////////////////////////////////////////////////
    // Private member variables used by CClientNotificationSink.
    //////////////////////////////////////////////////////////////////////////

    // Critical section key used for synchronizing
    // printf calls
    CRITICAL_SECTION m_printfCriticalSection;

    //////////////////////////////////////////////////////////////////////////
    // Private member variables used by IUnknown methods.
    //////////////////////////////////////////////////////////////////////////

    ULONG m_cRef; // IUnknown reference counter
};
