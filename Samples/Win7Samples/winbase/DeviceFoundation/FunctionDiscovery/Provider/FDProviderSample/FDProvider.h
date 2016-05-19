// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      This module defines the TFunctionDiscoveryProvider class that
//      implements the IFunctionDiscoveryProvider interface.

#pragma once

class TFunctionDiscoveryProvider: 
    public TList<TFunctionDiscoveryProvider>::TListEntry,
    public IFunctionDiscoveryProvider
{
public:

    //
    // IUnknown
    //
    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP QueryInterface(
        REFIID riid, 
        __deref_out_opt void** ppv);

    //
    // IFunctionDiscoveryProvider
    //

    STDMETHODIMP Initialize( 
        /* [in] */ __RPC__in_opt IFunctionDiscoveryProviderFactory *pIFunctionDiscoveryProviderFactory,
        /* [in] */ __RPC__in_opt IFunctionDiscoveryNotification *pIFunctionDiscoveryNotification,
        /* [in] */ LCID lcidUserDefault,
        /* [out] */ __RPC__out DWORD *pdwStgAccessCapabilities);
        
    STDMETHODIMP Query( 
        /* [in] */ __RPC__in_opt IFunctionDiscoveryProviderQuery *pIFunctionDiscoveryProviderQuery,
        /* [out] */ __RPC__deref_out_opt IFunctionInstanceCollection **ppIFunctionInstanceCollection);
    
    STDMETHODIMP EndQuery();
    
    STDMETHODIMP InstancePropertyStoreValidateAccess( 
        /* [in] */ __RPC__in_opt IFunctionInstance *pIFunctionInstance,
        /* [in] */ INT_PTR iProviderInstanceContext,
        /* [in] */ const DWORD dwStgAccess);
    
    STDMETHODIMP InstancePropertyStoreOpen( 
        /* [in] */ __RPC__in_opt IFunctionInstance *pIFunctionInstance,
        /* [in] */ INT_PTR iProviderInstanceContext,
        /* [in] */ const DWORD dwStgAccess,
        /* [out] */ __RPC__deref_out_opt IPropertyStore **ppPropertyStore);
    
    STDMETHODIMP InstancePropertyStoreFlush( 
        /* [in] */ __RPC__in_opt IFunctionInstance *pIFunctionInstance,
        /* [in] */ INT_PTR iProviderInstanceContext);
    
    STDMETHODIMP InstanceQueryService( 
        /* [in] */ __RPC__in_opt IFunctionInstance *pIFunctionInstance,
        /* [in] */ INT_PTR iProviderInstanceContext,
        /* [in] */ __RPC__in REFGUID guidService,
        /* [in] */ __RPC__in REFIID riid,
        /* [out] */ __RPC__deref_out_opt IUnknown **ppIUnknown);
    
    STDMETHODIMP InstanceReleased( 
        /* [in] */ __RPC__in_opt IFunctionInstance *pIFunctionInstance,
        /* [in] */ INT_PTR iProviderInstanceContext);

    //
    // Constructor / Descructor
    //
    TFunctionDiscoveryProvider(
        PFNProviderLifetimeNotificationCallback pfnProviderLifetimeNotificationCallback);

    // Logoff notification handler
    static VOID LogoffNotification(
        DWORD hSessionId);
    
    //
    // Functions to submit client notifications
    //
    HRESULT SubmitNotifyClientOnError(
        HRESULT hr);

    HRESULT SubmitNotifyClientOnEvent(
        DWORD EventId);

    HRESULT SubmitNotifyClientOnUpdate(
        QueryUpdateAction QueryUpdateAction,
        __in TFunctionInstanceInfo* pFunctionInstanceInfo);

protected:
    ~TFunctionDiscoveryProvider();  
    VOID ReleaseCachedInterfaces();
    VOID StopDiscoveryProtocolQuery();

    HRESULT TFunctionDiscoveryProvider::GetChildFunctionInstances(
        __in IFunctionInstance* pIFunctionInstance,
        __deref_out IFunctionInstanceCollection** ppIFunctionInstanceCollection);

    VOID SubmitNotifyClientWork(
        __in TClientNotificationWork* pClientNotificationWork);

    //
    //  Worker functions executed by the threadpool to dispacth client notifications
    //
    static VOID CALLBACK NotifyClientCallback(
        PTP_CALLBACK_INSTANCE Instance,
        __in PVOID pContext,
        PTP_WORK Work);
    HRESULT NotifyClientOnErrorWorker(
        HRESULT hr);
    HRESULT NotifyClientOnEventWorker(
        DWORD EventId);
    HRESULT NotifyClientOnUpdateWorker(
        QueryUpdateAction QueryUpdateAction,
        __in TFunctionInstanceInfo* pFunctionInstanceInfo);

    // Data members
protected:
    static TList<TFunctionDiscoveryProvider> sm_ProviderList;
    static TLock sm_ProviderListLock;

    LONG m_cRef;
    PFNProviderLifetimeNotificationCallback m_pfnProviderLifetimeNotificationCallback;
     TLock m_CachedInterfaceLock;
    IFunctionDiscoveryProviderFactory* m_pIFunctionDiscoveryProviderFactory;
    IFunctionDiscoveryNotification* m_pIFunctionDiscoveryNotification;
    LCID m_lcidUserDefault;
    CFDProviderHelper m_FDProviderHelper;
    TClientNotificationThreadpool* m_pThreadpool;
    bool m_fClientNotifyEnvironmentInit;
    TP_CALLBACK_ENVIRON m_ClientNotifyEnvironment;
    PTP_WORK m_hClientNotifyWork;
    TList<TClientNotificationWork> m_ClientNotificationQueue;
    TLock m_ClientNotificationQueueLock;
    HANDLE m_hDiscoveryProtocolQuery;
    TLock m_DiscoveryProtocolQueryLock;
};  // TFunctionDiscoveryProvider