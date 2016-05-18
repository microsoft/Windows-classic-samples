// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef _ADMINWRAP_H_
#define _ADMINWRAP_H_

#include <comadmin.h>

struct SubscriptionProperty
{
    LPCWSTR pwszPropName;
    VARIANT varPropVal;
};

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
        __deref_out BSTR* pbstrSubscriptionID);

HRESULT RemoveTransientSubscription(
        __in ICOMAdminCatalog* pCatalog,
        __in BSTR bstrSubscriptionID);

// Generic catalog access functions
HRESULT GetCollection(
        __in ICOMAdminCatalog* pCatalog,
        __in BSTR bstrCollName,
        __deref_out ICatalogCollection** ppCatColl);

HRESULT GetCollection(
        __in ICatalogCollection* pCatCollParent,
        __in ICatalogObject* pCatObjParent,
        __in BSTR bstrCollName,
        __deref_out ICatalogCollection** ppCatColl);

HRESULT SetStringProperty(
        __in ICatalogObject* pCatObj,
        __in BSTR bstrPropName,
        __in_opt LPCWSTR pwszVal);

HRESULT SetIUnknownProperty(
        __in ICatalogObject* pCatObj,
        __in BSTR bstrPropName,
        __in IUnknown* punk);

HRESULT GetStringProperty(
        __in ICatalogObject* pCatObj,
        __in BSTR bstrPropName,
        __deref_out BSTR* pbstrVal);



#endif
