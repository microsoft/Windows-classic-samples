// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "Adminwrap.h"
#include <comadmin.h>
#include <assert.h>

#define if_failed_exit(HR)      do { if FAILED(HR) goto exit; } while (false)
#define exit_with_result(HR)    do { hr = HR; goto exit; } while (false)

//*********************************************
// Globals

CComBSTR g_bstrPublisherID(L"PublisherID");
CComBSTR g_bstrName(L"Name");
CComBSTR g_bstrValue(L"Value");
CComBSTR g_bstrID(L"ID");
CComBSTR g_bstrEventCLSID(L"EventCLSID");
CComBSTR g_bstrInterfaceID(L"InterfaceID");
CComBSTR g_bstrMethodName(L"MethodName");
CComBSTR g_bstrFilterCriteria(L"FilterCriteria");
CComBSTR g_bstrTransientSubscriptions(L"TransientSubscriptions");
CComBSTR g_bstrSubscriberInterface(L"SubscriberInterface");
CComBSTR g_bstrTransientPublisherProperties(L"TransientPublisherProperties");
CComBSTR g_bstrTransientSubscriberProperties(L"TransientSubscriberProperties");

///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : AddTransientSubscription
// Purpose      : Adds a Transient Subscription
// Parameters   :
//              : ICOMAdminCatalog* pCatalog        - COM+ admin catalog object
//              : LPCWSTR pwszSubName               - Name of the subscription
//              : LPCWSTR pwszECID                  - EventCLSID to subscribe
//              : LPCWSTR pwszPubID                 - Name of the Publisher
//              : LPCWSTR pwszIID                   - IID to subscribe
//              : IUnknown* punk                    - IUnknown* of the subscriber
//              : LPCWSTR pwszMethod                - Method name in the interface to subscribe
//              : LPCWSTR pwszCriteria              - Filter Criteria (used in parameterized filtering)
//              : ULONG cPubProps                   - Count of publisher properties
//              : SubscriptionProperty* pubProps    - Array of publisher properties
//              : ULONG cSubProps                   - Count of subscriber properties
//              : SubscriptionProperty* subProps    - Array of subscriber properties
//              : BSTR* pbstrSubscriptionID         - On return, the ID of the subscription created
// Return Value : HRESULT
////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT AddTransientSubscription(
        __in ICOMAdminCatalog* pCatalog,
        __in LPCWSTR pwszSubName,
        __in LPCWSTR pwszECID,
        __in_opt LPCWSTR pwszPubID,
        __in LPCWSTR pwszIID,
        __in IUnknown *punk,
        __in_opt LPCWSTR pwszMethod,
        __in_opt LPCWSTR pwszCriteria,
        __in ULONG cPubProps,
        __in_ecount_opt(cPubProps) SubscriptionProperty* pubProps,
        __in ULONG cSubProps,
        __in_ecount_opt(cSubProps) SubscriptionProperty* subProps,
        __deref_out BSTR* pbstrSubscriptionID)
{
    HRESULT                     hr;
    CComPtr<ICatalogObject>     pCatObjSub;
    CComPtr<ICatalogCollection> pCatCollSubs;
    CComPtr<IDispatch>          pDisp;
    LONG                        lIgnored;
    bool                        fCreate = false, fComplete = false;
    
    if (!pCatalog || !pwszSubName || !punk || !pbstrSubscriptionID)
        return E_INVALIDARG;

    if ((cPubProps > 0) && (pubProps == NULL))
        return E_INVALIDARG;

    if ((cSubProps > 0) && (subProps == NULL))
        return E_INVALIDARG;

    hr = GetCollection(pCatalog, g_bstrTransientSubscriptions, &pCatCollSubs);
    if_failed_exit(hr);

    hr = pCatCollSubs->Add(&pDisp);
    if_failed_exit(hr);

    hr = pDisp->QueryInterface(IID_PPV_ARGS(&pCatObjSub));
    if_failed_exit(hr);
    pDisp.Release();

    hr = SetStringProperty(pCatObjSub, g_bstrName, pwszSubName);
    if_failed_exit(hr);

    hr = SetStringProperty(pCatObjSub, g_bstrEventCLSID, pwszECID);
    if_failed_exit(hr);

    hr = SetStringProperty(pCatObjSub, g_bstrPublisherID, pwszPubID);
    if_failed_exit(hr);

    hr = SetStringProperty(pCatObjSub, g_bstrInterfaceID, pwszIID);
    if_failed_exit(hr);

    hr = SetIUnknownProperty(pCatObjSub, g_bstrSubscriberInterface, punk);
    if_failed_exit(hr);

    hr = SetStringProperty(pCatObjSub, g_bstrMethodName, pwszMethod);
    if_failed_exit(hr);

    hr = SetStringProperty(pCatObjSub, g_bstrFilterCriteria, pwszCriteria);
    if_failed_exit(hr);

    // The catalog automatically sets this to a new GUID value (in string form)
    hr = GetStringProperty(pCatObjSub, g_bstrID, pbstrSubscriptionID);
    if_failed_exit(hr);

    hr = pCatCollSubs->SaveChanges(&lIgnored);

    if_failed_exit(hr);

    fCreate = true;

    if (cPubProps > 0)
    {
        CComPtr<ICatalogCollection> pCatCollPubProps;
        hr = GetCollection(pCatCollSubs, pCatObjSub, g_bstrTransientPublisherProperties, &pCatCollPubProps);
        if_failed_exit(hr);

        for (ULONG i = 0; i < cPubProps; i++)
        {
            hr = pCatCollPubProps->Add(&pDisp);
            if_failed_exit(hr);

            CComPtr<ICatalogObject> pCatObjProp;
            hr = pDisp->QueryInterface(IID_PPV_ARGS(&pCatObjProp));
            if_failed_exit(hr);
            pDisp.Release();

            hr = SetStringProperty(pCatObjProp, g_bstrName, pubProps[i].pwszPropName);
            if_failed_exit(hr);

            hr = pCatObjProp->put_Value(g_bstrValue, pubProps[i].varPropVal);
            if_failed_exit(hr);
        }

        hr = pCatCollPubProps->SaveChanges(&lIgnored);
        if_failed_exit(hr);
    }

    if (cSubProps > 0)
    {
        CComPtr<ICatalogCollection> pCatCollSubProps;
        hr = GetCollection(pCatCollSubs, pCatObjSub, g_bstrTransientSubscriberProperties, &pCatCollSubProps);
        if_failed_exit(hr);

        for (ULONG i = 0; i < cSubProps; i++)
        {
            hr = pCatCollSubProps->Add(&pDisp);
            if_failed_exit(hr);

            CComPtr<ICatalogObject> pCatObjProp;
            hr = pDisp->QueryInterface(IID_PPV_ARGS(&pCatObjProp));
            if_failed_exit(hr);
            pDisp.Release();

            hr = SetStringProperty(pCatObjProp, g_bstrName, subProps[i].pwszPropName);
            if_failed_exit(hr);
        
            hr = pCatObjProp->put_Value(g_bstrValue, subProps[i].varPropVal);
            if_failed_exit(hr);
        }

        hr = pCatCollSubProps->SaveChanges(&lIgnored);
        if_failed_exit(hr);
    }

    fComplete = true;
    
exit:
    if (fCreate && !fComplete)
        RemoveTransientSubscription(pCatalog, *pbstrSubscriptionID);

    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : RemoveTransientSubscription
// Purpose      : Removes a Transient Subscription
// Parameters   :
//              : ICOMAdminCatalog* pCatalog        - COM+ admin catalog object
//              : BSTR bstrSubscriptionID           - ID of the subscription to remove
// Return Value : HRESULT
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT RemoveTransientSubscription(
        __in ICOMAdminCatalog* pCatalog,
        __in BSTR bstrSubscriptionID)
{
    HRESULT                        hr;
    CComPtr<ICatalogCollection> pCatCollSubs;
    LONG lCount, lIndex, lIgnored;

    if (!pCatalog || !bstrSubscriptionID)
        return E_INVALIDARG;

    hr = GetCollection(pCatalog, g_bstrTransientSubscriptions, &pCatCollSubs);
    if_failed_exit(hr);

    hr = pCatCollSubs->Populate();
    if_failed_exit(hr);

    hr = pCatCollSubs->get_Count(&lCount);
    if_failed_exit(hr);

    if (lCount == 0)
        exit_with_result(E_UNEXPECTED);

    // Loop through 
    for (lIndex = 0; lIndex < lCount; lIndex++)
    {
        // Get the next item
        CComPtr<IDispatch> pDisp;
        hr = pCatCollSubs->get_Item(lIndex, &pDisp);
        if_failed_exit(hr);
        
        CComPtr<ICatalogObject> pCatObjSub;
        hr = pDisp->QueryInterface(IID_PPV_ARGS(&pCatObjSub));
        if_failed_exit(hr);

        // Retrieve its key
        CComVariant var;
        hr = pCatObjSub->get_Key(&var);

        if (var.vt != VT_BSTR)
            exit_with_result(E_UNEXPECTED);

        // Check for a match
        if (wcscmp(bstrSubscriptionID, var.bstrVal) == 0)
        {
            // Found it!
            hr = pCatCollSubs->Remove(lIndex);
            if_failed_exit(hr);

            hr = pCatCollSubs->SaveChanges(&lIgnored);
            if_failed_exit(hr);

            exit_with_result(S_OK);
        }
    }
    // If we get here, the object didn't exist
    exit_with_result(E_UNEXPECTED);

exit:
    return hr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : GetCollection
// Purpose      : Gets a collection from the Catalogue
// Parameters   :
//              : ICOMAdminCatalog* pCatalog        - Admin Catalog object
//              : BSTR bstrCollName                 - Collection Name
//              : ICatalogCollection** ppCatColl    - Collection returned
// Return Value : HRESULT
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT GetCollection(
        __in ICOMAdminCatalog* pCatalog,
        __in BSTR bstrCollName,
        __deref_out ICatalogCollection** ppCatColl)
{
    HRESULT hr;
    CComPtr<IDispatch> pDisp;

    if (!pCatalog || !bstrCollName || !ppCatColl) 
        return E_INVALIDARG;
    
    hr = pCatalog->GetCollection(bstrCollName, &pDisp);
    if_failed_exit(hr);

    hr = pDisp->QueryInterface(IID_PPV_ARGS(ppCatColl));
    if_failed_exit(hr);

    hr = (*ppCatColl)->Populate();
    if_failed_exit(hr);

exit:
    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : GetCollection
// Purpose      : Gets a collection from the Catalogue
// Parameters   :
//              : ICatalogCollection* pCatCollParent- Parent Collection
//              : ICatalogObject* pCatObjParent     - Parent Object
//              : BSTR bstrCollName                 - Collection Name
//              : ICatalogCollection** ppCatColl    - Collection returned
// Return Value : HRESULT
////////////////////////////////////////////////////////////////////////////////////////////////////    
HRESULT GetCollection(
        __in ICatalogCollection* pCatCollParent,
        __in ICatalogObject* pCatObjParent,
        __in BSTR bstrCollName,
        __deref_out ICatalogCollection** ppCatColl)
{
    HRESULT hr;
    CComVariant var;
    CComPtr<IDispatch> pDisp;

    if (!pCatCollParent || !pCatObjParent || !bstrCollName || !ppCatColl)
        return E_INVALIDARG;

    hr = pCatObjParent->get_Key(&var);
    if_failed_exit(hr);

    hr = pCatCollParent->GetCollection(bstrCollName, var, &pDisp);
    if_failed_exit(hr);

    hr = pDisp->QueryInterface(IID_PPV_ARGS(ppCatColl));
    if_failed_exit(hr);

    hr = (*ppCatColl)->Populate();
    if_failed_exit(hr);

exit:
    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : SetStringProperty
// Purpose      : Sets a string property of an object
// Parameters   :
//              : ICatalogObject* pCatObj           -  Pointer to the object
//              : BSTR bstrPropName                 -  Name of the property
//              : LPCWSTR pwszVal                   -  Value of the property
// Return Value : HRESULT
////////////////////////////////////////////////////////////////////////////////////////////////////    
HRESULT SetStringProperty(
        __in ICatalogObject* pCatObj,
        __in BSTR bstrPropName,
        __in_opt LPCWSTR pwszVal)
{
    if (!pCatObj || !bstrPropName)
        return E_INVALIDARG;

    CComVariant var(pwszVal);
    if ((pwszVal != NULL) && (var.bstrVal == NULL))
        return E_OUTOFMEMORY;

    return pCatObj->put_Value(bstrPropName, var);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : SetIUnknownProperty
// Purpose      : Sets an IUnknown property of an object
// Parameters   :
//              : ICatalogObject* pCatObj           -  Pointer to the object
//              : BSTR bstrPropName                 -  Name of the property
//              : IUnknown* punk                    -  Value of the property
// Return Value : HRESULT
////////////////////////////////////////////////////////////////////////////////////////////////////    
HRESULT SetIUnknownProperty(
        __in ICatalogObject* pCatObj,
        __in BSTR bstrPropName,
        __in IUnknown* punk)
{
    if (!pCatObj || !bstrPropName || !punk)
        return E_INVALIDARG;

    CComVariant var(punk);
    return pCatObj->put_Value(bstrPropName, var);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : GetStringProperty
// Purpose      : Gets a string property of the object
// Parameters   :
//              : ICatalogObject* pCatObj           -  Pointer to the object
//              : BSTR bstrPropName                 -  Name of the property
//              : BSTR* pbstrVal                    -  Value of the property
// Return Value : HRESULT
////////////////////////////////////////////////////////////////////////////////////////////////////    
HRESULT GetStringProperty(
        __in ICatalogObject* pCatObj,
        __in BSTR bstrPropName,
        __deref_out BSTR* pbstrVal)
{
    HRESULT hr;

    if (!pCatObj || !bstrPropName || !pbstrVal)
        return E_INVALIDARG;

    CComVariant var;
    hr = pCatObj->get_Value(bstrPropName, &var);
    if_failed_exit(hr);

    if (var.vt != VT_BSTR)
        return E_UNEXPECTED;

    *pbstrVal = var.bstrVal;
    var.bstrVal = NULL; // Caller now owns the BSTR

exit:
    return hr;
}
