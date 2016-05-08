// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "ComSpyCtl.h"
#include "ComSpyAudit.h"
#include "SysLCESub.h"        // Include our base class.
#include "CComSpy.h"
#include "evtstoresub.h"

//*********************************************
// Globals

CComBSTR g_bstrPROGID_EventSubscription(PROGID_EventSubscription);
CComBSTR g_bstrPROGID_EventClass(PROGID_EventClass);


STDMETHODIMP CEvtStoreSub::ChangedSubscription
   (/* [in] */ EOC_ChangeType changeType,
    /* [in] */ BSTR bstrSubscriptionID)
{

    LONGLONG perfCount = 0;
    ::QueryPerformanceCounter( (LARGE_INTEGER *)&perfCount );
    LPWSTR lpOperation = NULL;
    switch (changeType)
    {
        case EOC_NewObject:
            lpOperation = L"New Subscription";
            break;
        case EOC_ModifiedObject:
            lpOperation = L"Modified Subscription";
            break;
        case EOC_DeletedObject:
            lpOperation = L"Deleted Subscription";
            break;
    }
    m_pSpy->AddEventToList(perfCount, lpOperation, L"COM+ Event Store");
    m_pSpy->AddParamValueToList(L"Subscription ID", bstrSubscriptionID);
    //TODO: Add Auditing code for subscription change
    AddSubscriptionInfo( m_pSpy, bstrSubscriptionID );
    return S_OK;
}

STDMETHODIMP CEvtStoreSub::ChangedEventClass
   (/* [in] */ EOC_ChangeType changeType,
    /* [in] */ BSTR bstrEventClassID)
{
    LONGLONG perfCount = 0;
    ::QueryPerformanceCounter( (LARGE_INTEGER *)&perfCount );
    LPWSTR lpOperation = NULL;
    switch (changeType)
    {
        case EOC_NewObject:
            lpOperation = L"New Event Class";
            break;
        case EOC_ModifiedObject:
            lpOperation = L"Modified Event Class";
            break;
        case EOC_DeletedObject:
            lpOperation = L"Deleted Event Class";
            break;
    }
    m_pSpy->AddEventToList(perfCount, lpOperation, L"COM+ Event Store");
    m_pSpy->AddParamValueToList(L"Event Class ID", bstrEventClassID);
    
    //TODO: Add Auditing code for event class change
    AddEventClassInfo( m_pSpy, bstrEventClassID );
    return S_OK;
}

STDMETHODIMP CEvtStoreSub::ChangedPublisher
   (/* [in] */ EOC_ChangeType changeType,
    /* [in] */ BSTR bstrPublisherID)
{
    return S_OK;
}



///////////
STDMETHODIMP CEvtStoreSub::AddSubscriptionInfo
//-----------------------------------------------------------------
// Function Description:
//
//    Given a subscription ID, query the Event Store for information.
//
// Return Value:
//
//  Arguments:
   (/* [in] */ CComSpy* pSpy,
    /* [in] */ BSTR bstrSubscriptionID)
//    
//-----------------------------------------------------------------
{
    HRESULT hr = S_OK;
    // Variables initialized to NULL here to ensure error returns are valid.
    CComPtr<IEventSystem> spIEventSystem = NULL;

    hr = CoCreateInstance(CLSID_CEventSystem,
                          NULL,
                          CLSCTX_SERVER,
                          IID_PPV_ARGS(&spIEventSystem));
    if (FAILED(hr))
    {
        ATLTRACE(L"Failed to create CEventSystem");
        return hr;
    }

    // ----------------------------------------------
    // Query for the subscription info
    // ----------------------------------------------

    int errorIndex;
    
    CComBSTR bstrCriteria;
    bstrCriteria = "SubscriptionID=";
    bstrCriteria += bstrSubscriptionID;

    CComPtr<IUnknown> pUnk = NULL;
    hr = spIEventSystem->Query(g_bstrPROGID_EventSubscription, bstrCriteria, &errorIndex, &pUnk);
    if (FAILED(hr)) 
    {
        ATLTRACE(L"Query for SubcriptionID failed");
        return hr;
    }

    // Got something - now get the info and blast it to Spy.
    CComPtr<IEventSubscription> spEventSubscription;
    hr = pUnk->QueryInterface(IID_PPV_ARGS(&spEventSubscription));
    _ASSERTE(SUCCEEDED(hr) && spEventSubscription);
    if (!spEventSubscription) 
    {
        ATLTRACE(L"QI for IEventSubscription failed");
        return hr;
    }

    CComBSTR bstrSubscriptionName;
    CComBSTR bstrEventClassID;
    CComBSTR bstrDescription;

    spEventSubscription->get_SubscriptionName( &bstrSubscriptionName );
    spEventSubscription->get_EventClassID( &bstrEventClassID );
    spEventSubscription->get_Description( &bstrDescription );

    pSpy->AddParamValueToList(L"Subscription Name", bstrSubscriptionName);
    pSpy->AddParamValueToList(L"Event Class ID", bstrEventClassID);
    pSpy->AddParamValueToList(L"Description", bstrDescription);
    //TODO: Add Auditing code for subscription change

    return hr;
    
}

STDMETHODIMP CEvtStoreSub::AddEventClassInfo
//-----------------------------------------------------------------
// Function Description:
//
//    Given an Event Class ID, query the Event Store for information.
//
// Return Value:
//
//  Arguments:
   (/* [in] */ CComSpy* pSpy,
    /* [in] */ BSTR bstrEventClassID)
//    
//-----------------------------------------------------------------
{
    HRESULT hr = S_OK;
    // Variables initialized to NULL here to ensure error returns are valid.
    CComPtr<IEventSystem> spIEventSystem = NULL;

    hr = CoCreateInstance(CLSID_CEventSystem,
                          NULL,
                          CLSCTX_SERVER,
                          IID_PPV_ARGS(&spIEventSystem));
    if (FAILED(hr))
    {
        ATLTRACE(L"Failed to create CEventSystem");
        return hr;
    }

    // ----------------------------------------------
    // Query for the EventClass info
    // ----------------------------------------------

    int errorIndex;

    CComBSTR bstrCriteria;
    bstrCriteria = "EventClassID=";
    bstrCriteria += bstrEventClassID;

    CComPtr<IUnknown> pUnk = NULL;
    hr = spIEventSystem->Query(g_bstrPROGID_EventClass, bstrCriteria, &errorIndex, &pUnk);
    if (FAILED(hr)) 
    {
        ATLTRACE(L"Query for SubcriptionID failed");
        return hr;
    }

    // Got something - now get the info and blast it to Spy.
    CComPtr<IEventClass> spEventClass;
    hr = pUnk->QueryInterface(IID_PPV_ARGS(&spEventClass));
    _ASSERTE(SUCCEEDED(hr) && pUnk);
    if (!spEventClass) 
    {
        ATLTRACE(L"QI for IEventClass failed");
        return hr;
    }

    CComBSTR bstrEventClassName;
    CComBSTR bstrFiringInterfaceID;
    CComBSTR bstrOwnerSID;
    CComBSTR bstrDescription;

    spEventClass->get_EventClassName( &bstrEventClassName );
    spEventClass->get_FiringInterfaceID( &bstrFiringInterfaceID );
    spEventClass->get_Description( &bstrDescription );
    spEventClass->get_OwnerSID( &bstrOwnerSID );

    pSpy->AddParamValueToList(L"EventClass Name", bstrEventClassName);
    pSpy->AddParamValueToList(L"Firing Interface ID", bstrFiringInterfaceID);
    pSpy->AddParamValueToList(L"Description", bstrDescription);
    pSpy->AddParamValueToList(L"Owner SID", bstrOwnerSID);
    //TODO: Add Auditing code for EventClass change

    return hr;

}

