//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>
#include <string.h>
#include <wsdapi.h>
#include <time.h>
#include "TargetService.h"
#include "Common.h"

// Generates an EndpointReference in the form of a urn:uuid.
// The caller should call delete[] on the EndpointReference.
_Success_( return == S_OK )
static HRESULT _GenerateEndpointReference
(   _Outptr_ LPWSTR *epr
);

// Generates an XAddrs list that contains an XAddrs as
// described below.  The caller shall call WSDFreeLinkedMemory
// on the returned WSD_URI_LIST.
//
// For the purpose of this demo, a mechanism for providing an
// XAddrs was not provided by WSDiscoveryService.exe, and hence
// one will be generated on your behalf.  The XAddrs that is
// being generated will be dependent on what EndpointReference
// was provided.  By default, if no EndpointReference was
// provided at the time when WSDiscoveryService.exe is created,
// a logical address will be automatically generated and used
// for this target service.
//
// A logical address is any address that begins with urn:uuid:
// or uuid:, followed by a 36-character UUID.  
// A physical address is any addresses that begins with http://,
// https:// or uri:.  Anything else is treated as invalid.
//
// If a logical address is provided (or generated), then the
// XAddrs list will contain a generated XAddrs of the following
// format:
// http://hostname.sdk/(UUID of the logical address)
// E.g.: http://prn-example.sdk/f452f1ae-fbb4-11de-a6bb-00cc30bfc300
//
// If a physical address is provided, then the physical address
// is also used as an XAddrs and will be included in the XAddrs list.
//
// Note: This generated XAddrs is for demo purposes only.
// The developer should ensure that the XAddrs are valid
// network resolvable addresses.  An example of such an
// address may look like http://prn-example/PRN42/b42-1668-a
// (Section 1.3 of the WS-Discovery spec).  The developer
// should further ensure that the clients understand
// the XAddrs and know how to contact the target service.
_Success_(return == S_OK)
static HRESULT _GenerateXAddrsList
(   _In_ LPWSTR epr
,   _Outptr_ WSD_URI_LIST **xAddrsList
);

_Success_( return == S_OK )
HRESULT CreateTargetService
(   _In_opt_ LPCWSTR id
,   _In_opt_ const WSD_URI_LIST *scopesList
,   _Outptr_ CTargetService **targetService
)
{
    HRESULT hr = S_OK;
    CTargetService *tempService = NULL;

    if ( NULL == targetService )
    {
        hr = E_POINTER;
    }
    else
    {
        *targetService = NULL;
    }

    if ( S_OK == hr )
    {
        tempService = new CTargetService();

        if ( NULL == tempService )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( S_OK == hr )
    {
        hr = tempService->Init( id, scopesList );
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns the temp service
        *targetService = tempService;
        tempService = NULL;
    }

    if ( NULL != tempService )
    {
        tempService->Release();
        tempService = NULL;
    }

    return hr;
}

CTargetService::CTargetService()
:   m_epr( NULL )
,   m_typesList( NULL ) // <-- always NULL for the purpose of this sample
,   m_scopesList( NULL )
,   m_xAddrsList( NULL )
,   m_publisher( NULL )
,   m_instanceId( 0 )
,   m_messageNum( 0 )
,   m_isActive( FALSE )
,   m_isSuspended( TRUE ) // suspend by default - will be waken up when sending Hello
,   m_cRef( 1 )
{
    InitializeCriticalSection( &m_msgSeqCriticalSection );
}

CTargetService::~CTargetService()
{
    // Terminate the service if it is still running.
    // The Terminate method releases the notification sinks
    // if they are still being registered.
    //
    // No need to check whether it is active or not
    // since the Terminate method will check this.
    (void)Terminate();

    if ( NULL != m_publisher )
    {
        m_publisher->Release();
        m_publisher = NULL;
    }

    if ( NULL != m_epr )
    {
        delete[] m_epr;
        m_epr = NULL;
    }

    if ( NULL != m_typesList )
    {
        WSDFreeLinkedMemory( m_typesList );
        m_typesList = NULL;
    }

    if ( NULL != m_scopesList )
    {
        WSDFreeLinkedMemory( m_scopesList );
        m_scopesList = NULL;
    }

    if ( NULL != m_xAddrsList )
    {
        WSDFreeLinkedMemory( m_xAddrsList );
        m_xAddrsList = NULL;
    }

    m_cRef = 0;
    m_instanceId = 0;
    m_messageNum = 0;
    m_isActive = FALSE;
    m_isSuspended = TRUE;

    DeleteCriticalSection( &m_msgSeqCriticalSection );
}

HRESULT STDMETHODCALLTYPE CTargetService::Init
(   _In_opt_ LPCWSTR id
,   _In_opt_ const WSD_URI_LIST *scopesList
)
{
    HRESULT hr = S_OK;

    IWSDiscoveryPublisher *tempPublisher = NULL;
    LPWSTR tempEpr = NULL;
    WSD_URI_LIST *tempUriList = NULL;
    WSD_URI_LIST *tempXAddrsList = NULL;

    if ( m_isActive || NULL != m_publisher )
    {
        // The target service has previously been initialized,
        // or the previous initialization has failed.
        hr = E_ABORT;
    }

    if ( S_OK == hr )
    {
        // Create the discovery publisher with no context.
        // The discovery publisher is responsible for handling
        // all WS-Discovery related messages, including sending
        // and receiving operations.
        //
        // Note: You may supply your own XML context for this
        // target service by replacing NULL with your XML context.
        hr = WSDCreateDiscoveryPublisher( NULL, &tempPublisher );
    }

    if ( S_OK == hr )
    {
        if ( NULL == id )
        {
            // no EndpointReference specified
            // generate one
            hr = _GenerateEndpointReference( &tempEpr );
        }
        else
        {
            // an EndpointReference has been specified
            // deep copy it
            hr = DeepCopyString( id, &tempEpr );
        }
    }

    if ( S_OK == hr )
    {
        // Generate XAddrsList
        // See above for comment on the usages of XAddrs.
        hr = _GenerateXAddrsList( tempEpr, &tempXAddrsList );
    }

    if ( S_OK == hr && NULL != scopesList )
    {
        // scopes list has been specified - deep copy it
        hr = DeepCopyWsdUriList( scopesList, &tempUriList );
    }

    if ( S_OK == hr )
    {
        // Register a custom scope matching rule.  All custom
        // scope matching rules should implement IWSDScopeMatchingRule
        // and have them registered using this method.  If you have no
        // custom rules to use, this step is optional.
        hr = tempPublisher->RegisterScopeMatchingRule(
            static_cast<IWSDScopeMatchingRule *>( this )
        );
    }

    if ( S_OK == hr )
    {
        // Use the current time as the InstanceId.
        // This returns the time in seconds.
        // Note that the maximum value allowed for
        // the InstanceId is UINT_MAX, which is
        // 0xFFFF FFFF, 4 294 967 295.
        // Using the current time, this gives you
        // a valid InstanceId for about 136 years
        // from 1970-01-01 - ample amount of valid
        // InstanceId's to use.
        //
        // We are using the time function here to simplify
        // the logic around InstanceId's in this sample.
        // Although Appendix I of the WS-Discovery
        // Specifications states that using this approach
        // is one way of dealing with InstanceId's, it is
        // not a good practice in general.  If more than
        // one target service is created within a second,
        // there is a strong chance that the InstanceId's
        // of these target services will be the same, and
        // hence violate the application sequencing
        // requirement of WS-Discovery.  A better approach
        // in doing this is to have the InstanceId's stored
        // in a persistence storage mechanism such as in a
        // file or in the registry.  This will be up to the
        // developer of the target service to implement.
        m_instanceId = (ULONGLONG)time( NULL );

        // reset message number to 0
        m_messageNum = 0;

        // let target service own the "temporary variables"
        m_publisher = tempPublisher;
        m_epr = tempEpr;
        m_scopesList = tempUriList;
        m_xAddrsList = tempXAddrsList;

        tempPublisher = NULL;
        tempEpr = NULL;
        tempUriList = NULL;
        tempXAddrsList = NULL;

        // Note that, by default, m_typesList is always NULL  in this sample.
        // You can have them implemented in this method as appropriate.
        //
        // We let the target service own the temporary variables now because
        // the notification sink, which we are about to register, uses these
        // variables when it receives callbacks.
        //
        // Registering the notification sink starts the publisher
        // (it will begin to listen to Probe and Resolve messages).
        hr = m_publisher->RegisterNotificationSink(
            static_cast<IWSDiscoveryPublisherNotify *>( this )
        );
    }

    if ( S_OK == hr )
    {
        // If the notification sink registers successfully, then
        // this target service officially becomes active.  If it fails,
        // then this target service is not usable and it must be released.
        // A new target service must be created instead.  Reinitializing
        // a previously failed attempt is not allowed since m_publisher
        // will not be NULL.
        m_isActive = TRUE;

        // Keep the target service in suspended state.  It will be awaken
        // when a Hello message is sent.
        m_isSuspended = TRUE;
    }

    if ( NULL != tempPublisher )
    {
        tempPublisher->Release();
        tempPublisher = NULL;
    }

    if ( NULL != tempEpr )
    {
        delete[] tempEpr;
        tempEpr = NULL;
    }

    if ( NULL != tempUriList )
    {
        WSDFreeLinkedMemory( tempUriList );
        tempUriList = NULL;
    }

    if ( NULL != tempXAddrsList )
    {
        WSDFreeLinkedMemory( tempXAddrsList );
        tempXAddrsList = NULL;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::GetEndpointReference
(   _Outptr_ LPCWSTR *epr
)
{
    HRESULT hr = S_OK;

    if ( NULL == epr )
    {
        hr = E_POINTER;
    }

    if ( S_OK == hr )
    {
        // if Init has not been successfully called,
        // (or if the target service has been terminated)
        // this will result in NULL (which is OK)
        //
        // do not return S_FALSE as outside sources may
        // only be checking for S_OK
        *epr = m_epr;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::SendHelloMessage()
{
    HRESULT hr = S_OK;
    ULONGLONG instanceId = 0;
    ULONGLONG messageNum = 0;

    if ( !m_isActive )
    {
        // target service has not been initialized yet
        hr = E_ABORT;
    }

    if ( S_OK == hr )
    {
        // Get the MessageNumber and InstanceId for
        // the next message.  This is needed so that the
        // target service can conform to the WS-Discovery
        // sequencing specifications.
        hr = this->GetNextMessageNumber( &messageNum, &instanceId );
    }

    if ( S_OK == hr )
    {
        // The Publish method sends the Hello message.
        //
        // To publish the target service with extended information in the ANY section,
        // use PublishEx instead.
        //
        // Note that a target service may send more than one Hello message to update
        // its metadata, type, scope and XAddr information.
        hr = m_publisher->Publish(
            m_epr, TARGET_SVC_METADATA_VER, instanceId,
            messageNum, TARGET_SVC_SESSION_ID, m_typesList, m_scopesList, m_xAddrsList );
    }

    if ( S_OK == hr )
    {
        // Once the Hello message is sent, wake up the target service.
        m_isSuspended = FALSE;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::SendByeMessage()
{
    HRESULT hr = S_OK;
    ULONGLONG instanceId = 0;
    ULONGLONG messageNum = 0;

    if ( !m_isActive )
    {
        // target service has not been initialized yet
        hr = E_ABORT;
    }

    if ( S_OK == hr )
    {
        // Get the MessageNumber and InstanceId for
        // the next message.  This is needed so that the
        // target service can conform to the WS-Discovery
        // sequencing specifications.
        hr = this->GetNextMessageNumber( &messageNum, &instanceId );
    }

    if ( S_OK == hr )
    {
        // The UnPublish method sends the Bye message.
        // If there are extended information needed in the
        // ANY section, replace the last argument with that information.
        hr = m_publisher->UnPublish(
            m_epr, instanceId, messageNum,
            TARGET_SVC_SESSION_ID, NULL );
    }

    if ( S_OK == hr )
    {
        // Once the target service has successfully send a Bye message,
        // suspend the target service.
        m_isSuspended = TRUE;
    }

    if ( S_OK == hr )
    {
        // reset MessageNumber to 0 and increment InstanceId
        EnterCriticalSection( &m_msgSeqCriticalSection );

        m_messageNum = 0;

        // We cap InstanceId between 0 and MAX_INSTANCE_ID.  See
        // GetNextMessageNumber for details on this.
        m_instanceId = ( m_instanceId + 1 ) % ( MAX_INSTANCE_ID + 1 );

        LeaveCriticalSection( &m_msgSeqCriticalSection );
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::Terminate()
{
    HRESULT hr = S_OK;

    if ( !m_isActive )
    {
        // the target service has previously been terminated
        hr = E_ABORT;
    }

    if ( S_OK == hr )
    {
        // We do not care about the return code.  Once terminated,
        // it cannot be reinitialized.  If the terminate fails,
        // the remaining clean up will be done when the target service
        // is released.

        // Send one final Bye message before we shut down.
        // (don't care if this fails)
        (void)SendByeMessage();

        // unregister notification sink
        (void)m_publisher->UnRegisterNotificationSink(
            static_cast<IWSDiscoveryPublisherNotify *>( this )
        );

        // unregister scope matching rule
        (void)m_publisher->UnRegisterScopeMatchingRule(
            static_cast<IWSDScopeMatchingRule *>( this )
        );
    }

    // Terminate does not clean up internal variables.
    // That is done in the destructor when Release is called.

    // The terminate method can only be called once.
    // Hence, whether it is successful or not, the
    // target service will now be deactivated.  (If it
    // has never been activated before, setting
    // m_isActive = FALSE will have no effect.)
    //
    // Setting m_isActive = FALSE does not mean this target
    // service can be reactivated by calling Init.  Init
    // will check to make sure that m_publisher is NULL
    // and m_isActive is FALSE before an initialization
    // can be done.
    m_isActive = FALSE;

    // Also suspend the target service.
    m_isSuspended = TRUE;

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::ProbeHandler
(   _In_ const WSD_SOAP_MESSAGE *soap
,   _In_ IWSDMessageParameters *messageParameters
)
{
    HRESULT hr = S_OK;
    ULONGLONG instanceId = 0;
    ULONGLONG messageNum = 0;

    if ( !m_isActive )
    {
        // target service has not been initialized yet
        hr = E_ABORT;
    }

    if ( S_OK == hr && !m_isSuspended )
    {
        // Only handle this Probe message if a Bye message has not previously
        // been sent (i.e. the target service is not suspended).  Otherwise,
        // ignore this Probe.

        if ( S_OK == hr )
        {
            // Get the MessageNumber and InstanceId for
            // the next message.  This is needed so that the
            // target service can conform to the WS-Discovery
            // sequencing specifications.
            hr = GetNextMessageNumber( &messageNum, &instanceId );
        }

        if ( S_OK == hr )
        {
            // The MatchProbe method performs all probe matching
            // routines for the target service.  These routines
            // include scopes matching, types matching, etc.
            // It also sends a ProbeMatches message to the client
            // if one should be sent.  Custom scopes matching
            // is also done by this method since it calls MatchScopes
            // method of the target service.  The MatchScopes method
            // will only be called by the MatchProbe method if there
            // is at least one scope attached to the target service,
            // i.e. m_scopesList contains at least one scope.
            //
            // If there are any extended information needed to
            // be populated in the ANY section, use MatchProbeEx
            // instead.
            //
            // Note that calling the MatchProbe method is OPTIONAL.
            // If you already knew that the Probe does not match,
            // you do not need to call MatchProbe for
            // additional processing.  However, you may always call
            // MatchProbe regardless of whether you know the Probe
            // will match or not.  The MatchProbe method will send
            // a ProbeMatches message ONLY if the Probe matches the
            // target service.
            hr = m_publisher->MatchProbe(
                soap, messageParameters, m_epr,
                TARGET_SVC_METADATA_VER, instanceId, messageNum,
                TARGET_SVC_SESSION_ID, m_typesList, m_scopesList,
                m_xAddrsList );
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::ResolveHandler
(   _In_ const WSD_SOAP_MESSAGE *soap
,   _In_ IWSDMessageParameters *messageParameters
)
{
    HRESULT hr = S_OK;
    ULONGLONG instanceId = 0;
    ULONGLONG messageNum = 0;

    if ( !m_isActive )
    {
        // target service has not been initialized yet
        hr = E_ABORT;
    }

    if ( S_OK == hr && !m_isSuspended )
    {
        // Only handle this Resolve message if a Bye message has not previously
        // been sent (i.e. the target service is not suspended).  Otherwise,
        // ignore this Probe.

        if ( S_OK == hr )
        {
            // Get the MessageNumber and InstanceId for
            // the next message.  This is needed so that the
            // target service can conform to the WS-Discovery
            // sequencing specifications.
            hr = this->GetNextMessageNumber( &messageNum, &instanceId );
        }

        if ( S_OK == hr )
        {
            // This method functions almost the same as MatchProbe,
            // except this method handles Resolve matching routines
            // rather than Probe matching.  See ProbeHandler method
            // (above) for additional comments.
            hr = m_publisher->MatchResolve(
                soap, messageParameters, m_epr,
                TARGET_SVC_METADATA_VER, instanceId, messageNum,
                TARGET_SVC_SESSION_ID, m_typesList, m_scopesList,
                m_xAddrsList );
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::GetScopeRule
(   _Outptr_ LPCWSTR *scopeMatchingRule
)
{
    HRESULT hr = S_OK;

    if ( NULL == scopeMatchingRule )
    {
        hr = E_POINTER;
    }
    else
    {
        *scopeMatchingRule = NULL;
    }

    if ( S_OK == hr )
    {
        hr = DeepCopyStringLinked( MATCHBY_CUSTOM, NULL, const_cast<LPWSTR *>( scopeMatchingRule ) );
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::MatchScopes
(   _In_ LPCWSTR scope1 // client
,   _In_ LPCWSTR scope2 // target service
,   _Out_ BOOL *isMatch
)
{
    HRESULT hr = S_OK;
    size_t length = 0;

    if ( NULL == scope1 || NULL == scope2 )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == isMatch )
    {
        hr = E_POINTER;
    }
    else
    {
        *isMatch = FALSE;
    }

    if ( S_OK == hr )
    {
        // Note that the client scope is scope1,
        // target service is scope2.  The MatchScopes
        // method is called at least once per client scope.
        // If a client scope does not match a given target
        // sevice scope, it will continue matching with more
        // target service scope until one matches the client.
        // If none matches the client, then the target service
        // doe not match the client's scopes.
        //
        // Example: (assuming they must be identical to match)
        // client: uri:123, uri:456, uri:789
        // service: uri:846, uri:123, uri:790
        //
        // order    scope1       scope2      isMatch
        // 1        uri:123      uri:846     FALSE
        // 2        uri:123      uri:123     TRUE (match with uri:123)
        // 3        uri:456      uri:846     FALSE
        // 4        uri:456      uri:123     FALSE
        // 5        uri:456      uri:790     FALSE (does not match uri:456)
        //
        // Therefore, the target service does not match the scopes specified
        // by the client.
        //
        // The target service much have all scopes matching the scopes specified
        // by the client in the probe message in order for a target service to be
        // considered a match.
        //
        // Also note that this target service must have at least one scope
        // attached to it in order for the MatchScopes method to be called.

        // For the purpose of this sample, we have an
        // usual custom scope matching rule here.  Normally,
        // a scope matching rule checks for scopes from
        // both client and target service.  However,
        // we only check the scope passed by the client here.
        // If the scope of the client ends with one of
        // L'1', L'3', L'5', L'7' or L'9', it is a match.
        // Otherwise, it is not a match.  Again, the
        // target service must have at least one scope
        // attached to it, regardless of whether we are
        // checking against the target service scopes or not.
        length = wcslen( scope1 ); // scope 1 = client

        if (1 <= length)
        {
            *isMatch = ( L'1' == scope1[length - 1] ||
                         L'3' == scope1[length - 1] ||
                         L'5' == scope1[length - 1] ||
                         L'7' == scope1[length - 1] ||
                         L'9' == scope1[length - 1] );
        }
        else
        {
            // empty string don't check
            *isMatch = FALSE;
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::GetNextMessageNumber
(   _Out_ ULONGLONG *nextMessageNum
,   _Out_ ULONGLONG *nextInstanceId
)
{
    HRESULT hr = S_OK;

    EnterCriticalSection( &m_msgSeqCriticalSection );

    if ( !m_isActive )
    {
        // target service has not been initialized yet
        hr = E_ABORT;
    }
    else if ( NULL == nextMessageNum || 
              NULL == nextInstanceId )
    {
        hr = E_POINTER;
    }

    if ( S_OK == hr )
    {
        // The m_instanceId and m_messageNum specifies
        // the Instance ID and the message number of the next message.
        *nextMessageNum = m_messageNum;
        *nextInstanceId = m_instanceId;

        // Increment the MessageNumber (and InstanceId).
        
        if ( MAX_MESSAGE_NUM <= m_messageNum )
        {
            // We have reached the maximum MessageNumber
            // and we need to increment the InstanceId.

            m_messageNum = 0;

            // We cap InstanceId between 0 and MAX_INSTANCE_ID.  Note that it
            // is very difficult for the Instance ID to ever get to the
            // maximum value.  It practically never happens on properly functioning
            // device.  You will only get to this kind of situation if your device is
            // sending A LOT OF MESSAGES or a lot of BYEs to be able to get here.
            // However, you should still properly handle this case as the WS-Discovery
            // specifications clearly indicated a maximum value for this.
            m_instanceId = ( m_instanceId + 1 ) % ( MAX_INSTANCE_ID + 1 );
        }
        else
        {
            // InstanceId does not need to be incremented
            m_messageNum++;
        }
    }

    LeaveCriticalSection( &m_msgSeqCriticalSection );

    return hr;
}

HRESULT STDMETHODCALLTYPE CTargetService::QueryInterface
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

        if ( __uuidof(IWSDiscoveryPublisherNotify) == riid )
        {
            *object = static_cast<IWSDiscoveryPublisherNotify *>( this );
        }
        else if ( __uuidof(IWSDScopeMatchingRule) == riid )
        {
            *object = static_cast<IWSDScopeMatchingRule *>( this );
        }
        else if ( __uuidof(IUnknown) == riid )
        {
            *object = static_cast<IUnknown*>(
                static_cast<IWSDiscoveryPublisherNotify*>(this) );
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

ULONG STDMETHODCALLTYPE CTargetService::AddRef()
{
    ULONG newRefCount = (ULONG)InterlockedIncrement( (LONG *)&m_cRef );
    return newRefCount;
}

ULONG STDMETHODCALLTYPE CTargetService::Release()
{
    ULONG newRefCount = (ULONG)InterlockedDecrement( (LONG *)&m_cRef );

    if ( 0 == newRefCount )
    {
        delete this;
    }

    return newRefCount;
}

_Success_( return == S_OK )
HRESULT _GenerateEndpointReference
(   _Outptr_ LPWSTR *epr
)
{
    HRESULT hr = S_OK;
    UUID uuidGenerated = {0};
    size_t guidStringLength = 0;
    size_t eprStringLength = 0;
    LPWSTR guidString = NULL;
    LPWSTR tempEpr = NULL;

    if ( NULL == epr )
    {
        hr = E_POINTER;
    }
    else
    {
        *epr = NULL;
    }

    if ( S_OK == hr )
    {
        // generate a UUID for use with this EndpointReference
        hr = UuidCreate(&uuidGenerated);

        if ( (HRESULT)RPC_S_OK == hr || (HRESULT)RPC_S_UUID_LOCAL_ONLY == hr )
        {
            // the UUID has been generated successfully
            hr = S_OK;
        }
        else
        {
            // failed to generate a UUID
            hr = E_FAIL;
        }
    }

    if ( S_OK == hr )
    {
        // convert the UUID (which is a GUID) as a string
        hr = GetGuidString( uuidGenerated, &guidString );
    }

    if ( S_OK == hr )
    {
        // length of the GUID string
        guidStringLength = wcslen( guidString );

        // 123456789 + GUID string len
        // urn:uuid:<GUID string>
        eprStringLength = 9 + guidStringLength;

        // allocate string for the EndpointReference
        tempEpr = new WCHAR[eprStringLength + 1]; // + 1 for null char

        if ( NULL == tempEpr )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( S_OK == hr )
    {
        // formulate the EndpointReference string
        hr = StringCchPrintfW( tempEpr, eprStringLength + 1,
            L"urn:uuid:%s", guidString);
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns the string
        *epr = tempEpr;
        tempEpr = NULL;
    }

    if ( NULL != tempEpr )
    {
        delete[] tempEpr;
        tempEpr = NULL;
    }

    if ( NULL != guidString )
    {
        delete[] guidString;
        guidString = NULL;
    }

    return hr;
}

_Success_(return == S_OK)
HRESULT _GenerateXAddrsList
(   _In_ LPWSTR epr
,   _Outptr_ WSD_URI_LIST **xAddrsList
)
{
    HRESULT hr = S_OK;
    LPWSTR hostName = NULL;
    LPWSTR tempXAddrs = NULL;
    LPWSTR eprSubstring = NULL; // (soft copy) used to determine if this is a logical or physical address
    LPWSTR tempEprUuid = NULL; // (soft copy) used to retrieve the UUID of a logical address
    WSD_URI_LIST *tempXAddrsList = NULL;
    size_t length = 0;

    if ( NULL == epr )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == xAddrsList )
    {
        hr = E_POINTER;
    }
    else
    {
        *xAddrsList = NULL;
    }

    if ( S_OK == hr )
    {
        // create the XAddrs list
        tempXAddrsList = (WSD_URI_LIST *)
            WSDAllocateLinkedMemory( NULL, sizeof( WSD_URI_LIST ) );

        if ( NULL == tempXAddrsList )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            tempXAddrsList->Element = NULL;
            tempXAddrsList->Next = NULL;
        }
    }
    
    if ( S_OK == hr )
    {
        // Determine whether this EndpointReference begins with http://
        eprSubstring = wcsstr( epr, L"http://" );
        
        if ( NULL == eprSubstring )
        {
            // The EndpointReference does not begin with http:.
            // See if it begins with https://.
            eprSubstring = wcsstr( epr, L"https://" );
        }
        
        if ( NULL == eprSubstring )
        {
            // The EndpointReference does not begin with https://.
            // See if it begins with uri:.
            eprSubstring = wcsstr( epr, L"uri:" );
        }
        
        if ( NULL != eprSubstring )
        {
            // This EndpointReference begins with one of http://, https:// or uri:// and we will
            // treat this as a physical address, hence XAddrs = EndpointReference.
            
            // Duplicate the EndpointReference and link it to the XAddrs list.
            hr = DeepCopyStringLinked( epr, tempXAddrsList, &tempXAddrs );
        }
    }
    
    if ( S_OK == hr && NULL == eprSubstring )
    {
        // The EndpointReference is not a physical address.
        // Determine whether this string begins with urn:uuid:
        eprSubstring = wcsstr( epr, L"urn:uuid:" );
        
        if ( NULL != eprSubstring )
        {
            // The EndpointReference begins with urn:uuid:.
            
            //          1         2        3          4
            // 123456789012345678901234567890123456789012345
            // urn:uuid:f452f1ae-fbb4-11de-a6bb-00cc30bfc300
            
            // Do a sanity check to make sure that the string is exactly
            // 45 characters long.
            if ( wcslen( eprSubstring ) != 45 )
            {
                hr = E_INVALIDARG;
            }
            
            if ( S_OK == hr )
            {
                // Soft copy the UUID portion of the address.
                // 0123456789...
                // urn:uuid:f452f1ae-fbb4-11de-a6bb-00cc30bfc300
                tempEprUuid = &(eprSubstring[9]);
            }
        }
    }
    
    if ( S_OK == hr && NULL == eprSubstring )
    {
        // The EndpointReference does not begin with urn:uuid:.
        // See if it begins with uuid: instead
        eprSubstring = wcsstr( epr, L"uuid:" );
    
        if ( NULL != eprSubstring )
        {
            // The string begins with uuid:.
            
            //          1         2        3          4
            // 12345678901234567890123456789012345678901
            // uuid:f452f1ae-fbb4-11de-a6bb-00cc30bfc300
            
            // Do a sanity check to make sure that the string is exactly
            // 41 characters long.
            if ( wcslen( eprSubstring ) != 41 )
            {
                hr = E_INVALIDARG;
            }
            
            if ( S_OK == hr )
            {
                // Soft copy the UUID portion of the address.
                // 012345...
                // uuid:f452f1ae-fbb4-11de-a6bb-00cc30bfc300
                tempEprUuid =  &(eprSubstring[5]);
            }
        }
    }
    
    if ( S_OK == hr && NULL == eprSubstring )
    {
        // The string is not a logical nor a physical address.
        hr = E_INVALIDARG;
    }
    
    if ( S_OK == hr && NULL != tempEprUuid )
    {
        // The EndpointReference begins with either urn:uuid or uuid:,
        // in which we know what the UUID of the EndpointReference is.
        // We proceed to generate an XAddrs using the format
        // http://hostname.sdk/(UUID of the logical address).
        
        if ( S_OK == hr )
        {
            // get the host name
            hr = GetWideStringHostName( &hostName );
        }

        if ( S_OK == hr )
        {
            // allocate memory big enough for the string
            //
            // 1234567 + wcslen(hostName) + 12345 + wcslen(tempEprUuid)        +1 for NULL char
            // http://   hostName           .sdk/   (UUID of the logical address)
            
            length = sizeof( WCHAR ) * ( 
                7 + // http://
                wcslen( hostName ) + 
                5 + // .sdk/
                wcslen( tempEprUuid ) + 
                1 // NULL char
            ); 

            tempXAddrs = (LPWSTR)WSDAllocateLinkedMemory( tempXAddrsList, length );

            if ( NULL == tempXAddrs )
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if ( S_OK == hr )
        {
            // create the XAddrs string
            hr = StringCchPrintfW( tempXAddrs, length, L"http://%s.sdk/%s",
                hostName, tempEprUuid );
        }
    }

    if ( S_OK == hr )
    {
        // the xaddrs list now owns the xaddrs string
        tempXAddrsList->Element = tempXAddrs;
        tempXAddrs = NULL;

        // outside pointer now owns the list
        *xAddrsList = tempXAddrsList;
        tempXAddrsList = NULL;
    }
    
    // soft copy strings - do not delete
    eprSubstring = NULL;
    tempEprUuid = NULL;

    if ( NULL != tempXAddrs )
    {
        WSDFreeLinkedMemory( tempXAddrs );
        tempXAddrs = NULL;
    }

    if ( NULL != tempXAddrsList )
    {
        WSDFreeLinkedMemory( tempXAddrsList );
        tempXAddrsList = NULL;
    }

    if ( NULL != hostName )
    {
        delete[] hostName;
        hostName = NULL;
    }

    return hr;
}
