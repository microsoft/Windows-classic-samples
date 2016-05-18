// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//  Abstract:
//      This module implements the TFunctionDiscoveryProvider class that 
//      implements the IFunctionDiscoveryProvider interface

#include <stdafx.h>


// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
//
// Every profider must declare its own category name here.
// This name much match with the entry that the provider's
// installer will create in the registry to register the provider.
// 
// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
const WCHAR g_szProviderCategory[] = L"Provider\\Sample.FDProvider";  

// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
// 
// Generate a new UUID to use here.
//
// SID_SampleProviderService is just used for demonstration purposes here.
// a real provider will provide its own services IDs GUIS for services 
// that it supports through QueryService
//
// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
const GUID SID_SampleProviderService = { 0x3e6fe3cf, 0x98ab, 0x44c7, { 0xbf, 0xbc, 0xd1, 0x70, 0x12, 0x49, 0x34, 0xd1 } };  

// The maximum length that a notification queue to a client can grow to.
// After this notifications for the client will be dropped.
const LONG g_MaxClientNotificationQueueLength = 1000;


#ifndef FD_EVENTID_PRIVATE
#define FD_EVENTID_PRIVATE 100 // Defined by post Vista SDK
#endif


// Static member initialization
TList<TFunctionDiscoveryProvider> TFunctionDiscoveryProvider::sm_ProviderList;
TLock TFunctionDiscoveryProvider::sm_ProviderListLock;

//---------------------------------------------------------------------------
// Begin Constructor / Destructor
//---------------------------------------------------------------------------

TFunctionDiscoveryProvider::TFunctionDiscoveryProvider(
    PFNProviderLifetimeNotificationCallback pfnProviderLifetimeNotificationCallback):
    m_cRef(1),
    m_pfnProviderLifetimeNotificationCallback(pfnProviderLifetimeNotificationCallback),
    m_pIFunctionDiscoveryProviderFactory(NULL),
    m_pIFunctionDiscoveryNotification(NULL),
    m_lcidUserDefault(0),
    m_pThreadpool(NULL),
    m_fClientNotifyEnvironmentInit(false),
    m_hClientNotifyWork(NULL),
    m_hDiscoveryProtocolQuery(NULL)
{
    ZeroMemory(&m_ClientNotifyEnvironment, sizeof(m_ClientNotifyEnvironment));
    
    IncModuleCount();

    // Notify the exe host that a new provider was created
    if (m_pfnProviderLifetimeNotificationCallback)
    {
        m_pfnProviderLifetimeNotificationCallback(false);
    }

}  // TFunctionDiscoveryProvider::TFunctionDiscoveryProvider

TFunctionDiscoveryProvider::~TFunctionDiscoveryProvider()
{
    // Call EndQuery because it may not have been called if the client was 
    // unexpectedly terminated.
    EndQuery();

    // Free the objects that were cached from Initialize
    ReleaseCachedInterfaces();

    // Remove the provider from the list of active providers
    sm_ProviderListLock.AcquireExclusive();
    if (m_pNextListEntry) // m_pNextListEntry (and m_pPrevListEntry) will be NULL if the provider Is not in the list.
    {
        sm_ProviderList.RemoveEntry(this);
    }
    sm_ProviderListLock.ReleaseExclusive();

    // Notify the exe host that a provider was destructed
    if (m_pfnProviderLifetimeNotificationCallback)
    {
        m_pfnProviderLifetimeNotificationCallback(true);
    }

    DecModuleCount();
}  // TFunctionDiscoveryProvider::~TFunctionDiscoveryProvider

//---------------------------------------------------------------------------
// End Constructor / Destructor
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin IUnknown
//---------------------------------------------------------------------------

STDMETHODIMP_(ULONG) TFunctionDiscoveryProvider::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}  // TFunctionDiscoveryProvider::AddRef

STDMETHODIMP_(ULONG) TFunctionDiscoveryProvider::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}  // TFunctionDiscoveryProvider::Release

STDMETHODIMP TFunctionDiscoveryProvider::QueryInterface(
    REFIID riid, 
    __deref_out_opt void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv)
    {
        *ppv = NULL;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        if (__uuidof(IUnknown) == riid )
        {
            AddRef();
            *ppv = (IFunctionDiscoveryProvider*) this;
        }
        else if (__uuidof(IFunctionDiscoveryProvider) == riid)
        {
            AddRef();
            *ppv = (IFunctionDiscoveryProvider*) this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}  // TFunctionDiscoveryProvider::QueryInterface

//---------------------------------------------------------------------------
// End IUnknown
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Begin IFunctionDiscoveryProvider
//---------------------------------------------------------------------------

STDMETHODIMP TFunctionDiscoveryProvider::Initialize( 
    /* [in] */ __RPC__in_opt IFunctionDiscoveryProviderFactory* pIFunctionDiscoveryProviderFactory,
    /* [in] */ __RPC__in_opt IFunctionDiscoveryNotification* pIFunctionDiscoveryNotification,
    /* [in] */ LCID lcidUserDefault,
    /* [out] */ __RPC__out DWORD *pdwStgAccessCapabilities)
{
    HRESULT hr = S_OK;
    BOOL fClientIsAlice = FALSE;

    // Initialize m_FDProviderHelper based on either of the interfaces that are passed in.
    if (pIFunctionDiscoveryProviderFactory)
    {
        hr = m_FDProviderHelper.Initialize(pIFunctionDiscoveryProviderFactory);
    }
    else if (pIFunctionDiscoveryNotification)
    {
        hr = m_FDProviderHelper.Initialize(pIFunctionDiscoveryNotification);
    }

    // Verify the Interface pointers and set the security blanket on the interfaces.
    // Even if one of the interfaces are NULL, we'll still set the blanket on the other.
    if (S_OK == hr)
    {
        if (pIFunctionDiscoveryProviderFactory)
        {
            // Set the security blanket for pIFunctionDiscoveryProviderFactory
            hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionDiscoveryProviderFactory);
        }
        else
        {
            hr =  E_INVALIDARG;
        }

        if (pIFunctionDiscoveryNotification)
        {
            // Set the security blanket for pIFunctionDiscoveryNotification
            HRESULT temphr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionDiscoveryNotification);
            if (S_OK == hr)
            {
                hr = temphr;
            }
        }
        else
        {
            if (S_OK == hr)
            {
                hr =  E_INVALIDARG;
            }
        }
    }

    if (S_OK == hr)
    {
        if (!pdwStgAccessCapabilities)
        {
            hr =  E_INVALIDARG;
        }
    }
    
    // Save the user's language
    m_lcidUserDefault = lcidUserDefault;

    // Cache pIFunctionDiscoveryProviderFactory and pIFunctionDiscoveryProviderFactory 
    if (S_OK == hr)
    {
        m_CachedInterfaceLock.AcquireExclusive();
    
        pIFunctionDiscoveryProviderFactory->AddRef();
        m_pIFunctionDiscoveryProviderFactory = pIFunctionDiscoveryProviderFactory;
        pIFunctionDiscoveryNotification->AddRef();
        m_pIFunctionDiscoveryNotification = pIFunctionDiscoveryNotification;

        m_CachedInterfaceLock.ReleaseExclusive();
    }
   
    // Initialize the storage capabilities
    if (S_OK == hr)
    {
        *pdwStgAccessCapabilities = STGM_READ;
    }

    // Add the provider to the list of providers
    sm_ProviderListLock.AcquireExclusive();
    sm_ProviderList.InsertTail(this);
    sm_ProviderListLock.ReleaseExclusive();

    // Since it is possible that a loggoff notification arrived after
    // the provider was created, but before it got added to the list of
    // active providers, check to see if the client is still alive.
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryNotification->OnEvent(
            FD_EVENTID_PRIVATE,  // A private event, ignored by the client
            0,  // FunDisc will supply the context to the client
            g_szProviderCategory);
    }

    if (S_OK != hr)
    {
        // If an error occured release the cached interfaces and client token
        ReleaseCachedInterfaces();
        m_FDProviderHelper.ReleaseToken();
    }

    return hr;
}  // TFunctionDiscoveryProvider::Initialize
    
STDMETHODIMP TFunctionDiscoveryProvider::Query( 
    /* [in] */ __RPC__in_opt IFunctionDiscoveryProviderQuery* pIFunctionDiscoveryProviderQuery,
    /* [out] */ __RPC__deref_out_opt IFunctionInstanceCollection** ppIFunctionInstanceCollection)
{
    HRESULT hr = S_OK;
    PWSTR pszDeviceCategory = NULL;
    PWSTR pszInstanceQueryId = NULL;
    PWSTR pszSubCategory = NULL;
    BOOL isInstanceQuery = FALSE;
    BOOL isSubcategoryQuery = FALSE;
    IProviderQueryConstraintCollection* pIProviderQueryConstraintCollection = NULL;

    if (pIFunctionDiscoveryProviderQuery)
    {
        // Set proxy security blanket for pIFunctionDiscoveryProviderQuery to ensure that the provider can call the client interface
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionDiscoveryProviderQuery);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (ppIFunctionInstanceCollection)
    {
        *ppIFunctionInstanceCollection = NULL;
    }
    else
    {
        if (S_OK == hr)
        {
            hr = E_INVALIDARG;
        }
    }

    // Check if a subcatecory was specified.
    // This provider does not support sub categories.
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryProviderQuery->IsSubcategoryQuery(
            &isSubcategoryQuery, 
            &pszSubCategory);
    }
    if (S_OK == hr)
    {
        if (isSubcategoryQuery)
        {
            hr = E_INVALIDARG;
        }
    }

    /// 
    /// Retieve the query constraints the the client specified
    ///
    
    // Check if this is an instance query.
    // If it is, we'll save the Instance Id that we are querying for.
    // All providers should support querying for a specific instance
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryProviderQuery->IsInstanceQuery(
            &isInstanceQuery, 
            &pszInstanceQueryId);
    }

    // Retrieve the constraint collection to check the rest of the constraints, if any
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryProviderQuery->GetQueryConstraints(&pIProviderQueryConstraintCollection);
    }
    // Set proxy security blanket for pIProviderQueryConstraintCollection to ensure that the provider can call the client interface
    if (S_OK == hr)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIProviderQueryConstraintCollection);
    }

    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
    //
    // The sample retrieves a constraint called DeviceCategory here.
    // Implementers should retrieve their provider's constraints here and 
    // pass them to the discovery implementation.
    // 
    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO    
    if (S_OK == hr)
    {
        // "DeviceCategory" is a custom constraint just for this provider
        hr = pIProviderQueryConstraintCollection->Get(
            L"DeviceCategory", 
            &pszDeviceCategory);
        if (S_FALSE == hr)
        {
            // Get will return S_FALSE if the constraint is not found.  
            // If that's the case just continue.
            hr = S_OK;
        }
    }

    ///
    /// End constrains retrieval
    ///

    //
    // Set up the threadpool environment for queuing callbacks to the client.
    //
    
    // Initialize m_ClientNotifyEnvironment
    if (S_OK == hr)
    {
        InitializeThreadpoolEnvironment(&m_ClientNotifyEnvironment);
        m_fClientNotifyEnvironmentInit = true;
    }

    // Get a pointer to the client notification threadpool
    if (S_OK == hr)
    {
        m_pThreadpool = TClientNotificationThreadpool::GetThreadpool();
        
        if (!m_pThreadpool)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Associate the Threadpool with the environment block
    if (S_OK == hr)
    {
        SetThreadpoolCallbackPool(
            &m_ClientNotifyEnvironment,
            m_pThreadpool->GetPTP_POOL());
    }
    
    if (S_OK == hr)
    {
        m_hClientNotifyWork = CreateThreadpoolWork(
            &NotifyClientCallback,
            this,  // Pass the provider as the context
            &m_ClientNotifyEnvironment);
        
        if (!m_hClientNotifyWork)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (S_OK == hr)
    {
        m_DiscoveryProtocolQueryLock.AcquireExclusive();

        hr = StartDiscoveryQuery(
            this,
            pszDeviceCategory,
            pszInstanceQueryId,
            &m_hDiscoveryProtocolQuery);

        m_DiscoveryProtocolQueryLock.ReleaseExclusive();
    }

    if (S_OK == hr)
    {
        // Since the is an Async provider, it has to return E_PENDING on success
        hr = E_PENDING;
    }
    else
    {
        // If creating the query fails, call EndQuery to release the resources.
        EndQuery();
    }

    // Cleanup
    if (pszSubCategory)
    {
        CoTaskMemFree(pszSubCategory);
    }
    if (pszDeviceCategory)
    {
        CoTaskMemFree(pszDeviceCategory);
    }
    if (pszInstanceQueryId)
    {
        CoTaskMemFree(pszInstanceQueryId);
    }
    if (pIProviderQueryConstraintCollection)
    {
        pIProviderQueryConstraintCollection->Release();
    }

    return hr;
} // TFunctionDiscoveryProvider::Query

STDMETHODIMP TFunctionDiscoveryProvider::EndQuery()
{

    // NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE
    //
    // It is important that EndQuery completes quickly without waiting for
    // lengthy timeouts.  Any delay in EndQuery will be reflected in 
    // calling applications such as Network Explorer when a user 
    // refresh the UI.
    // 
    // NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE

    HRESULT hr = S_OK;

    StopDiscoveryProtocolQuery();

    // Delete all remaining work in the queue, but leave the first entry
    // because the work callback function may be using it.
    m_ClientNotificationQueueLock.AcquireExclusive();
    
    while (m_ClientNotificationQueue.GetCount() > 1)
    {
        delete m_ClientNotificationQueue.RemoveTail();
    }

    m_ClientNotificationQueueLock.ReleaseExclusive();

    // Wait for outstanding work to complete and 
    // close the work.
    if (m_hClientNotifyWork)
    {
        WaitForThreadpoolWorkCallbacks(
            m_hClientNotifyWork,
            FALSE);
        CloseThreadpoolWork(m_hClientNotifyWork);
        m_hClientNotifyWork = NULL;
    }

    if (m_fClientNotifyEnvironmentInit)
    {
        DestroyThreadpoolEnvironment(&m_ClientNotifyEnvironment);
        ZeroMemory(&m_ClientNotifyEnvironment, sizeof(m_ClientNotifyEnvironment));
        m_fClientNotifyEnvironmentInit = false;
    }

    if (m_pThreadpool)
    {
        m_pThreadpool->Release();
        m_pThreadpool = NULL;
    }

    assert(m_ClientNotificationQueue.IsEmpty());
 
    return hr;
}  // TFunctionDiscoveryProvider::EndQuery

STDMETHODIMP TFunctionDiscoveryProvider::InstancePropertyStoreValidateAccess( 
    /* [in] */ __RPC__in_opt IFunctionInstance* pIFunctionInstance,
    /* [in] */ INT_PTR iProviderInstanceContext,
    /* [in] */ const DWORD dwStgAccess)
{
    HRESULT hr = S_OK;

    if (pIFunctionInstance)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionInstance);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        // Storage access returned in Initialize pdwStgAccessCapabilities
        // The sample Provider does not validate storage access on a per property basis.
        hr = E_NOTIMPL;
    }

    return hr; 
}  // TFunctionDiscoveryProvider::InstancePropertyStoreValidateAccess

STDMETHODIMP TFunctionDiscoveryProvider::InstancePropertyStoreOpen( 
    /* [in] */ __RPC__in_opt IFunctionInstance* pIFunctionInstance,
    /* [in] */ INT_PTR iProviderInstanceContext,
    /* [in] */ const DWORD dwStgAccess,
    /* [out] */ __RPC__deref_out_opt IPropertyStore **ppPropertyStore)
{
    HRESULT hr = S_OK;

    if (pIFunctionInstance)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionInstance);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        // The Sample Provider does not manage it's own Property store
        hr = E_NOTIMPL;
    }

    return hr; 
}  // TFunctionDiscoveryProvider::InstancePropertyStoreOpen

STDMETHODIMP TFunctionDiscoveryProvider::InstancePropertyStoreFlush( 
    /* [in] */ __RPC__in_opt IFunctionInstance* pIFunctionInstance,
    /* [in] */ INT_PTR iProviderInstanceContext)
{
    HRESULT hr = S_OK;

    if (pIFunctionInstance)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionInstance);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        // The Sample Provider does not manage it's own Property store
        hr = E_NOTIMPL;
    }

    return hr; 
}  // TFunctionDiscoveryProvider::InstancePropertyStoreFlush

STDMETHODIMP TFunctionDiscoveryProvider::InstanceQueryService( 
    /* [in] */ __RPC__in_opt IFunctionInstance* pIFunctionInstance,
    /* [in] */ INT_PTR iProviderInstanceContext,
    /* [in] */ __RPC__in REFGUID guidService,
    /* [in] */ __RPC__in REFIID riid,
    /* [out] */ __RPC__deref_out_opt IUnknown **ppIUnknown)
{
    HRESULT hr = S_OK;

    if (pIFunctionInstance)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionInstance);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (ppIUnknown)
    {
        *ppIUnknown = NULL;
    }
    else
    {
        if (S_OK == hr)
        {
            hr = E_INVALIDARG;
        }
    }

    if (S_OK == hr)
    {
        if (SID_PNPXServiceCollection == guidService)
        {
            if (__uuidof(IFunctionInstanceCollection) == riid)
            {
                IFunctionInstanceCollection* pIFunctionInstanceCollection = NULL;

                // The client is querying the provider for child services.
                // return a IFunctionInstanceCollection representing the child services.
                hr = GetChildFunctionInstances(
                    pIFunctionInstance,
                    &pIFunctionInstanceCollection);

                if (S_OK == hr)
                {
                    *ppIUnknown = pIFunctionInstanceCollection;
                }
            }
            else
            {
                // InstanceQueryService must return E_NOINTERFACE if the 
                // requested interface is not supported.
                hr = E_NOINTERFACE;
            }
        }
        else if (SID_SampleProviderService == guidService) 
        {
            // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
            // 
            // From demonstration purposes, the sample provider support creating 
            // a IStream interface for the SID_SampleProviderService service.
            // A real provider would support its own services and Interfaces
            //
            // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO

            if (__uuidof(IStream) == riid)
            {
                // Simply create an in memory IStream object to return.
                // A real provider would read properties from pIFunctionInstance
                // and create and return a COM object implementing the requested interface.

                LPSTREAM pStream = NULL;

                hr = CreateStreamOnHGlobal(
                    NULL,
                    TRUE,
                    &pStream);

                if (S_OK == hr)
                {
                    *ppIUnknown = (IUnknown*) pStream;
                }
            }
            else
            {
                // InstanceQueryService must return E_NOINTERFACE if the 
                // requested interface is not supported.
                hr = E_NOINTERFACE;
            }
        }
        else
        {
            // InstanceQueryService must return E_NOTIMPL if the 
            // requested service is not supported.
            hr = E_NOTIMPL;
        }
    }

    return hr;
}  // TFunctionDiscoveryProvider::InstanceQueryService

STDMETHODIMP TFunctionDiscoveryProvider::InstanceReleased( 
    /* [in] */ __RPC__in_opt IFunctionInstance* pIFunctionInstance,
    /* [in] */ INT_PTR iProviderInstanceContext)
{
    HRESULT hr = S_OK;
    
    if (pIFunctionInstance)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionInstance);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    // Instance release is called by Function Discovery whenever 
    // a function instance provided by the profider is released.
    // This allows the provider to do any cleanup related to cacheing 
    // that may be taking place in the provider.

    // The sample provider does not do caching.
    return hr;
}  // TFunctionDiscoveryProvider::InstanceReleased

//---------------------------------------------------------------------------
// End IFunctionDiscoveryProvider
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin Public methods 
//---------------------------------------------------------------------------

VOID TFunctionDiscoveryProvider::LogoffNotification(
        DWORD hSessionId)
{
    BOOL fSessionIsForThisProvider = FALSE;

    sm_ProviderListLock.AcquireShared();

    for (
         TList<TFunctionDiscoveryProvider>::TIterator pFunctionDiscoveryProvider = sm_ProviderList.Begin();
         pFunctionDiscoveryProvider != sm_ProviderList.End();
         pFunctionDiscoveryProvider = ++pFunctionDiscoveryProvider)
    {
        // Call m_FDProviderHelper.ReleaseToken for the session that has been logged off.
        // ReleaseToken will close the user token, if the token is for this session.
        // This is required when a session is logged off because the session will not end
        // until all tokens for the user has been closed.
        fSessionIsForThisProvider = pFunctionDiscoveryProvider->m_FDProviderHelper.ReleaseToken(hSessionId);
        
        if (fSessionIsForThisProvider)
        {
            // If the session was for this provider then call ReleaseCachedInterfaces
            // because CoSetSecurityBlanket associated the user token on the interfaces
            pFunctionDiscoveryProvider->ReleaseCachedInterfaces();
        }
    }

    sm_ProviderListLock.ReleaseShared();
}  // TFunctionDiscoveryProvider::LogoffNotification

HRESULT TFunctionDiscoveryProvider::SubmitNotifyClientOnError(
    HRESULT hrVal)
{
    HRESULT hr = S_OK;
    TClientNotificationWork* pClientNotificationWork = TClientNotificationWork::CreateClientOnErrorWork(hrVal);

    if (pClientNotificationWork)
    {
        SubmitNotifyClientWork(pClientNotificationWork);
    }
    else
    {
        // Attempt to call the client syncronously
        hr = NotifyClientOnErrorWorker(hrVal);
    }

    return hr;
}  // TFunctionDiscoveryProvider::SubmitNotifyClientOnError

HRESULT TFunctionDiscoveryProvider::SubmitNotifyClientOnEvent(
    DWORD EventId)
{
    HRESULT hr = S_OK;
    TClientNotificationWork* pClientNotificationWork = TClientNotificationWork::CreateClientOnEventWork(EventId);

    if (pClientNotificationWork)
    {
        SubmitNotifyClientWork(pClientNotificationWork);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}  // TFunctionDiscoveryProvider::SubmitNotifyClientOnEvent

HRESULT TFunctionDiscoveryProvider::SubmitNotifyClientOnUpdate(
    QueryUpdateAction QueryUpdateAction,
    __in TFunctionInstanceInfo* pFunctionInstanceInfo)
{
    HRESULT hr = S_OK;
    TClientNotificationWork* pClientNotificationWork = TClientNotificationWork::CreateClientOnUpdateWork(
        QueryUpdateAction,
        pFunctionInstanceInfo);

    if (pClientNotificationWork)
    {
        SubmitNotifyClientWork(pClientNotificationWork);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}  // TFunctionDiscoveryProvider::SubmitNotifyClientOnUpdate

//---------------------------------------------------------------------------
// End Public methods 
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin private methods
//---------------------------------------------------------------------------

VOID TFunctionDiscoveryProvider::ReleaseCachedInterfaces()
{
    m_CachedInterfaceLock.AcquireExclusive();
    
    if (m_pIFunctionDiscoveryProviderFactory)
    {
        m_pIFunctionDiscoveryProviderFactory->Release();
        m_pIFunctionDiscoveryProviderFactory = NULL;
    }
    if (m_pIFunctionDiscoveryNotification)
    {
        m_pIFunctionDiscoveryNotification->Release();
        m_pIFunctionDiscoveryNotification = NULL;
    }

    m_CachedInterfaceLock.ReleaseExclusive();
}  // TFunctionDiscoveryProvider::ReleaseCachedInterfaces()

VOID TFunctionDiscoveryProvider::StopDiscoveryProtocolQuery()
{
    m_DiscoveryProtocolQueryLock.AcquireExclusive();

    if (m_hDiscoveryProtocolQuery)
    {
        CloseDiscoveryQuery(m_hDiscoveryProtocolQuery);
        m_hDiscoveryProtocolQuery = NULL;
    }

    m_DiscoveryProtocolQueryLock.ReleaseExclusive();
}  // TFunctionDiscoveryProvider::StopDiscoveryProtocolQuery()

HRESULT TFunctionDiscoveryProvider::GetChildFunctionInstances(
        __in IFunctionInstance* pIFunctionInstance,
        __deref_out IFunctionInstanceCollection** ppIFunctionInstanceCollection)
{
    HRESULT hr = S_OK;
    IFunctionDiscoveryProviderFactory* pIFunctionDiscoveryProviderFactory = NULL;
    IFunctionInstanceCollection* pIFunctionInstanceCollection = NULL;

    *ppIFunctionInstanceCollection = NULL;

    // Get local references to the cached interfaces used by this function
    m_CachedInterfaceLock.AcquireShared();

    if (m_pIFunctionDiscoveryProviderFactory)
    {
        m_pIFunctionDiscoveryProviderFactory->AddRef();
        pIFunctionDiscoveryProviderFactory = m_pIFunctionDiscoveryProviderFactory;
    }
    else
    {
        // Interfaces has been released due to logoff
        hr = E_FAIL;
    }

    m_CachedInterfaceLock.ReleaseShared();

    // Create a function instance collection that will contain
    // function instances representing child devices / services
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryProviderFactory->CreateFunctionInstanceCollection(&pIFunctionInstanceCollection);
    }
    if (S_OK == hr)
    {
        // Set the security blanket on pIFunctionInstanceCollection
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionInstanceCollection);
    }

    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
    //
    // If devices have child devices or services, create Function Insances
    // that represent them and add them to pFunctionInstanceCollection.
    // 
    // TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO




    if (S_OK == hr)
    {
        *ppIFunctionInstanceCollection = pIFunctionInstanceCollection;
    }
    else
    {
        if (pIFunctionInstanceCollection)
        {
            pIFunctionInstanceCollection->Release();
        }
    }

// Cleanup
    if (pIFunctionDiscoveryProviderFactory)
    {
        pIFunctionDiscoveryProviderFactory->Release();
    }
    
    return hr;
}  // TFunctionDiscoveryProvider::GetChildFunctionInstances

VOID TFunctionDiscoveryProvider::SubmitNotifyClientWork(
    __in TClientNotificationWork* pClientNotificationWork)
{
    bool fDropNotification = false;

    // Add the work to the queue for the provider
    m_ClientNotificationQueueLock.AcquireExclusive();

    // If the client notification queue is not fulldd the notification to the queue 
    // otherwise drop the notification.
    // This check prevents the queue from growing unbounded if the rate of notifications 
    // exceeds the client's ability to process them.
    if (m_ClientNotificationQueue.GetCount() < g_MaxClientNotificationQueueLength)
    {
        m_ClientNotificationQueue.InsertTail(pClientNotificationWork);

        // If the queue was empty, add work to the threadpool
        // If the queue was not empty the threadpool worker will continue to service the queue.
        if (1 == m_ClientNotificationQueue.GetCount())
        {
            SubmitThreadpoolWork(m_hClientNotifyWork);
        }
    }
    else
    {
        fDropNotification = true;
    }

    m_ClientNotificationQueueLock.ReleaseExclusive();

    if (fDropNotification)
    {
        // Didn't queue the work so it needs to be deleted.
        delete pClientNotificationWork;
    }
    
}  // TFunctionDiscoveryProvider::SubmitNotifyClientWork

HRESULT TFunctionDiscoveryProvider::NotifyClientOnErrorWorker(
    HRESULT hrVal)
{
    HRESULT hr = S_OK;
    IFunctionDiscoveryNotification* pIFunctionDiscoveryNotification = NULL;

    // Get local references to the cached interfaces used by this function
    m_CachedInterfaceLock.AcquireShared();

    if (m_pIFunctionDiscoveryNotification)
    {
        m_pIFunctionDiscoveryNotification->AddRef();
        pIFunctionDiscoveryNotification = m_pIFunctionDiscoveryNotification;
    }
    else
    {
        // Interfaces has been released due to logoff
        hr = E_FAIL;
    }

    m_CachedInterfaceLock.ReleaseShared();

    // Notify the client about the error
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryNotification->OnError(
            hrVal,
            0,  // FunDisc will supply the context to the client
            g_szProviderCategory);
    }

    // Cleanup
    if (pIFunctionDiscoveryNotification)
    {
        pIFunctionDiscoveryNotification->Release();
    }

    return hr;
}  // TFunctionDiscoveryProvider::NotifyClientOnErrorWorker

HRESULT TFunctionDiscoveryProvider::NotifyClientOnEventWorker(
    DWORD EventId)
{
    HRESULT hr = S_OK;
    IFunctionDiscoveryNotification* pIFunctionDiscoveryNotification = NULL;

    // Get local references to the cached interfaces used by this function
    m_CachedInterfaceLock.AcquireShared();

    if (m_pIFunctionDiscoveryNotification)
    {
        m_pIFunctionDiscoveryNotification->AddRef();
        pIFunctionDiscoveryNotification = m_pIFunctionDiscoveryNotification;
    }
    else
    {
        // Interfaces has been released due to logoff
        hr = E_FAIL;
    }

    m_CachedInterfaceLock.ReleaseShared();

    // Notify the client about the event
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryNotification->OnEvent(
            EventId,
            0,  // FunDisc will supply the context to the client
            g_szProviderCategory);
    }


    // Cleanup
    if (pIFunctionDiscoveryNotification)
    {
        pIFunctionDiscoveryNotification->Release();
    }

    return hr;
}  // TFunctionDiscoveryProvider::NotifyClientOnEventWorker

HRESULT TFunctionDiscoveryProvider::NotifyClientOnUpdateWorker(
    QueryUpdateAction eQueryUpdateAction,
    __in TFunctionInstanceInfo* pFunctionInstanceInfo)
{
    HRESULT hr = S_OK;
    IFunctionDiscoveryProviderFactory* pIFunctionDiscoveryProviderFactory = NULL;
    IFunctionDiscoveryNotification* pIFunctionDiscoveryNotification = NULL;
    IFunctionInstance* pIFunctionInstance = NULL;
    IPropertyStore* pClientPropertyStore = NULL;

    // Get local references to the cached interfaces used by this function
    m_CachedInterfaceLock.AcquireShared();

    if (   m_pIFunctionDiscoveryProviderFactory
        && m_pIFunctionDiscoveryNotification)
    {
        m_pIFunctionDiscoveryProviderFactory->AddRef();
        pIFunctionDiscoveryProviderFactory = m_pIFunctionDiscoveryProviderFactory;
        m_pIFunctionDiscoveryNotification->AddRef();
        pIFunctionDiscoveryNotification = m_pIFunctionDiscoveryNotification;
    }
    else
    {
        // Interfaces has been released due to logoff
        hr = E_FAIL;
    }

    m_CachedInterfaceLock.ReleaseShared();

    // Create a property store in the client process
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryProviderFactory->CreatePropertyStore(&pClientPropertyStore);
    }
    // Set proxy security blanket for pClientPropertyStore to ensure that the provider can call the client interface
    if (S_OK == hr)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pClientPropertyStore);
    }

    // Populate the client's Propery store
    if (S_OK == hr)
    {
        hr = pFunctionInstanceInfo->PopulatePropertyStore(pClientPropertyStore);
    }

    // Create a function instance in the client process,
    // give it an identity and assign the propery store
    // to the function instance
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryProviderFactory->CreateInstance(
            NULL,  // No SubCategory
            pFunctionInstanceInfo->GetFunctionInstanceId(),
            NULL,
            pClientPropertyStore,
            this,
            &pIFunctionInstance);
    }
    // Set proxy security blanket for pIFunctionInstance to ensure that the provider can call the client interface
    if (S_OK == hr)
    {
        hr = m_FDProviderHelper.CoSetProxyBlanketWithThreadToken(pIFunctionInstance);
    }

    // Notify the client about update
    if (S_OK == hr)
    {
        hr = pIFunctionDiscoveryNotification->OnUpdate(
            eQueryUpdateAction,
            0,  // FunDisc will supply the context to the client
            pIFunctionInstance);
    }

    // Cleanup
    if (pClientPropertyStore)
    {
        pClientPropertyStore->Release();
    }
    if (pIFunctionInstance)
    {
        pIFunctionInstance->Release();
    }
    if (pIFunctionDiscoveryProviderFactory)
    {
        pIFunctionDiscoveryProviderFactory->Release();
    }
    if (pIFunctionDiscoveryNotification)
    {
        pIFunctionDiscoveryNotification ->Release();
    }

    return hr;
} // TFunctionDiscoveryProvider::NotifyClientOnUpdateWorker

VOID CALLBACK TFunctionDiscoveryProvider::NotifyClientCallback(
    PTP_CALLBACK_INSTANCE Instance,
    __in PVOID pContext,
    PTP_WORK Work)
{
    TFunctionDiscoveryProvider* pFunctionDiscoveryProvider = (TFunctionDiscoveryProvider*) pContext;
    TClientNotificationWork* pClientNotificationWork = NULL;
    bool fSubmitWork = false;

    // Initialize COM on the thread pool thread
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (S_OK == hr)
    {
        // Get the next work item from the queue, but leave it in the queue
        // to prevent SubmitNotifyClientWork from posting new work before 
        // NotifyClientOnUpdateWorker has completed the notification call
        pFunctionDiscoveryProvider->m_ClientNotificationQueueLock.AcquireExclusive();

        pClientNotificationWork = pFunctionDiscoveryProvider->m_ClientNotificationQueue.GetHead();

        pFunctionDiscoveryProvider->m_ClientNotificationQueueLock.ReleaseExclusive();

        // Dispatch the work
        switch (pClientNotificationWork->WorkType)
        {
        case OnError:
            hr = pFunctionDiscoveryProvider->NotifyClientOnErrorWorker(pClientNotificationWork->WorkData.OnErrorWork.hr);
            break;
        case OnEvent:
            hr = pFunctionDiscoveryProvider->NotifyClientOnEventWorker(pClientNotificationWork->WorkData.OnEventWork.EventId);
            break;
        case OnUpdate:
            hr = pFunctionDiscoveryProvider->NotifyClientOnUpdateWorker(
                pClientNotificationWork->WorkData.OnUpdateWork.QueryUpdateAction,
                pClientNotificationWork->WorkData.OnUpdateWork.pFunctionInstanceInfo);

            break;
        }

        if (   (HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) != hr)
            && (HRESULT_FROM_WIN32(RPC_E_DISCONNECTED) != hr))
        {
            // The notification was delivered.
            
            pFunctionDiscoveryProvider->m_ClientNotificationQueueLock.AcquireExclusive();

            // Remove it from the queue.
            pFunctionDiscoveryProvider->m_ClientNotificationQueue.RemoveEntry(pClientNotificationWork);

            // If the queue is not empty, new work must be submitted to the threadpool to service the rest of the queue.
            fSubmitWork = !pFunctionDiscoveryProvider->m_ClientNotificationQueue.IsEmpty();

            pFunctionDiscoveryProvider->m_ClientNotificationQueueLock.ReleaseExclusive();

            // delete the work data that was removed from the queue
            delete pClientNotificationWork;
            pClientNotificationWork = NULL;

            if (S_OK != hr)
            {
                // An error has occured.  Inform the client.
                pFunctionDiscoveryProvider->NotifyClientOnErrorWorker(hr);
            }

            // if more work must be submitted, do it here.
            if (fSubmitWork)
            {
                SubmitThreadpoolWork(Work);
            }
        }
        else
        {
            // Calls to the client are no longer possible.
            // Clean up resources associated with this provider that 
            // are not threadpool related.

            pFunctionDiscoveryProvider->ReleaseCachedInterfaces();

            pFunctionDiscoveryProvider->StopDiscoveryProtocolQuery();

            // Clean up all outstanding notifications.
            pFunctionDiscoveryProvider->m_ClientNotificationQueueLock.AcquireExclusive();
            
            while (!pFunctionDiscoveryProvider->m_ClientNotificationQueue.IsEmpty())
            {
                delete pFunctionDiscoveryProvider->m_ClientNotificationQueue.RemoveHead();
            }

            pFunctionDiscoveryProvider->m_ClientNotificationQueueLock.ReleaseExclusive();
        }

        // Thread pool threads must restore the thread state 
        CoUninitialize();
    }
    // else 
    // Failure of COM to initialize on the thread should not happen
    // It means that someone else failed to clean up COM on the threadpool thread.
    // There is no way to recover if this should happen.

}  // TFunctionDiscoveryProvider::NotifyClientCallback

//---------------------------------------------------------------------------
// End private methods
//---------------------------------------------------------------------------
