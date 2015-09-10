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

#define MAX_MESSAGE_NUM ( (ULONGLONG)UINT_MAX )
#define MAX_INSTANCE_ID ( (ULONGLONG)UINT_MAX )

#define TARGET_SVC_METADATA_VER 0
#define TARGET_SVC_SESSION_ID NULL

class CTargetService;

//////////////////////////////////////////////////////////////////////////////
// Factory function to create a CTargetService object
//////////////////////////////////////////////////////////////////////////////

// Creates a target service and initializes the target service
// with the EndpointReference and a list of scopes, both
// of which are optional.  The caller should call the Release() method
// on the target service when it is no longer needed.
_Success_( return == S_OK )
HRESULT CreateTargetService
(   _In_opt_ LPCWSTR id
,   _In_opt_ const WSD_URI_LIST *scopesList
,   _Outptr_ CTargetService **targetService
);

//////////////////////////////////////////////////////////////////////////////
// CTargetService Class
//       Implements the target service functionalities through the use of
//       IWSDiscoveryPublisher and IWSDiscoveryPublisherNotify
//////////////////////////////////////////////////////////////////////////////
class CTargetService 
:   public IWSDiscoveryPublisherNotify
,   public IWSDScopeMatchingRule
{
public:
    //////////////////////////////////////////////////////////////////////////
    // CTargetService public methods
    //////////////////////////////////////////////////////////////////////////

    // Gets the EndpointReference of this target service.
    // If Init has not previously been called, or the target service has
    // been terminated, this method returns E_ABORT.
    // Do not deallocate the string.
    HRESULT STDMETHODCALLTYPE GetEndpointReference
    (   _Outptr_ LPCWSTR *epr
    );

    // Sends a Hello message.  If Init has not been previously called,
    // or if Terminate has been called and Init has not been recalled,
    // this method returns E_ABORT and does nothing.
    //
    // Sending a Hello message activates the target service (i.e. bring
    // it back up from the suspended state).
    HRESULT STDMETHODCALLTYPE SendHelloMessage();

    // Sends a Bye message.  If Init has not been previously called,
    // or if Terminate has been called and Init has not been recalled,
    // this method returns E_ABORT and does nothing.
    //
    // A Bye resets the MessageNumber to 0 and increments to the
    // next InstanceId.  The behaviour of incrementing the InstanceId
    // beyond the acceptable range is undefined.
    //
    // Sending a Bye message suspends the target service (i.e. it will
    // not respond to Probe and Resolve messages).
    HRESULT STDMETHODCALLTYPE SendByeMessage();

    // Terminates the target service, which uninitializes the variables.
    // If Init has not been previously called, or if Terminate has already 
    // been called, this method returns E_ABORT and does nothing.
    //
    // The Terminate method calls SendByeMessage before proceeding with
    // termination procedures, regardless of whether a Bye message has
    // previously been sent or not.
    HRESULT STDMETHODCALLTYPE Terminate();

    // Initializes the default variables.  If CTargetService has previously
    // been initialized, this method returns E_ABORT and does nothing.
    //
    // Note that this method should only be called by the factory function
    // CreateTargetService.
    HRESULT STDMETHODCALLTYPE Init
    (   _In_opt_ LPCWSTR id
    ,   _In_opt_ const WSD_URI_LIST *scopesList
    );

public:
    //////////////////////////////////////////////////////////////////////////
    // IWSDiscoveryPublisherNotify methods for message handling
    // See MSDN Documentation.
    //////////////////////////////////////////////////////////////////////////

    HRESULT STDMETHODCALLTYPE ProbeHandler
    (   _In_ const WSD_SOAP_MESSAGE *soap
    ,   _In_ IWSDMessageParameters *messageParameters
    );
    
    HRESULT STDMETHODCALLTYPE ResolveHandler
    (   _In_ const WSD_SOAP_MESSAGE *soap
    ,   _In_ IWSDMessageParameters *messageParameters
    );

    //////////////////////////////////////////////////////////////////////////
    // IWSDScopeMatchingRule methods for custom scope matching
    // See MSDN Documentation.
    //////////////////////////////////////////////////////////////////////////

    HRESULT STDMETHODCALLTYPE GetScopeRule( _Outptr_ LPCWSTR *scopeMatchingRule );
    
    HRESULT STDMETHODCALLTYPE MatchScopes
    (   _In_ LPCWSTR scope1
    ,   _In_ LPCWSTR scope2
    ,   _Out_ BOOL *isMatch
    );

private:
    //////////////////////////////////////////////////////////////////////////
    // CTargetService private helper methods
    //////////////////////////////////////////////////////////////////////////

    // Determines the next MessageNumber and InstanceId.
    // If the message number reaches the maximum value, it resets to 0
    // and increments the InstanceId.  The behaviour of incrementing the
    // Instance ID beyond the acceptable range is undefined in the
    // WS-Discovery specifications, so we will be resetting
    // the MessageNumber and InstanceId to 0.
    HRESULT STDMETHODCALLTYPE GetNextMessageNumber
    (   _Out_ ULONGLONG *nextMessageNum
    ,   _Out_ ULONGLONG *nextInstanceId
    );

public:
    //////////////////////////////////////////////////////////////////////////
    // IUnknown (extended by IWSDiscoveryPublisherNotify)
    // methods to make this class act like a COM object
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
    // CTargetService constructor.
    //////////////////////////////////////////////////////////////////////////
    CTargetService();

private:
    //////////////////////////////////////////////////////////////////////////
    // CTargetService destructor.
    //////////////////////////////////////////////////////////////////////////

    // The destructor should only be called by the Release method
    // implemented through IUnknown.
    ~CTargetService();

private:
    //////////////////////////////////////////////////////////////////////////
    // Private member variables that are used by IWSDiscoveryPublisher
    // to identify this target service.
    //////////////////////////////////////////////////////////////////////////

    LPWSTR m_epr; // the logical or physical address of this service
    WSD_NAME_LIST *m_typesList; // the list of types associated with this service
    WSD_URI_LIST *m_scopesList; // the list of scopes associated with this service
    WSD_URI_LIST *m_xAddrsList; // the list of XAddrs associated with this service

    //////////////////////////////////////////////////////////////////////////
    // Private member variables that are used by IWSDiscoveryPublisher
    // which defines the state of the target service.  This includes
    // IWSDiscoveryPublisher itself for the purpose of organization.
    //////////////////////////////////////////////////////////////////////////

    IWSDiscoveryPublisher *m_publisher;
    ULONGLONG m_instanceId;
    ULONGLONG m_messageNum;
    BOOL volatile m_isActive;  // Determines whether the target service is active or terminated.
                               // A target service is defined as being terminated if the
                               // CTargetService::Terminate method has been called.
    BOOL volatile m_isSuspended; // Determines whether the target service is currently being
                                  // suspended or not.  A target service is defined as being
                                  // suspended if CTargetService::SendByeMessage has been called
                                  // but not CTargetService::Terminate.  A target service can be
                                  // awaken from the suspended state if the method
                                  // CTargetService::SendHelloMessage is being called.  The
                                  // m_isSuspended flag has no effect if m_isActive is FALSE.

    // Critical section key used for synchronizing
    // MessageNumber sequencing
    CRITICAL_SECTION m_msgSeqCriticalSection;

    //////////////////////////////////////////////////////////////////////////
    // Private member variables that are used by IUnknown
    //////////////////////////////////////////////////////////////////////////
    
    ULONG m_cRef; // IUnknown reference counter
};
