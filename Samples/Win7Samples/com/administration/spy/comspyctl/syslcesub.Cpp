// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "ComSpyCtl.h"
#include "SysLCESub.h"

#include "adminwrap.h"
#include <stdio.h>


/////////////////////////////////////////////////////////////////////////////
// CSysLCESub

STDMETHODIMP CSysLCESub::Install(BSTR* pPropertyName, VARIANT PropertyValue)
{
    HRESULT hr = S_OK;

    CComBSTR bstrSubscriptionName(EventName(EventType()));
    CComBSTR bstrEventCLSID(GuidToBstr(EventCLSID()));
    CComBSTR bstrIID(GuidToBstr(EventIID()));
    
    CComPtr<ICOMAdminCatalog> spCatalog;
    if (SUCCEEDED(hr = CoCreateInstance(__uuidof(COMAdminCatalog),
                          NULL,
                          CLSCTX_SERVER,
                          IID_PPV_ARGS(&spCatalog))))
    {
        SubscriptionProperty pubProp;
        if (pPropertyName)
        {
            pubProp.pwszPropName = *pPropertyName;
            pubProp.varPropVal = PropertyValue;
        }
        hr = AddTransientSubscription(spCatalog,
                                bstrSubscriptionName,
                                bstrEventCLSID,
                                NULL,
                                bstrIID,
                                GetUnknown(),
                                NULL,
                                NULL,
                                pPropertyName ? 1 : 0,
                                pPropertyName ? &pubProp : NULL,
                                0,
                                NULL,
                                &m_bstrSubscriptionID);
    }
    return hr;
}

STDMETHODIMP CSysLCESub::Uninstall()
{
    HRESULT hr = S_OK;

    CComPtr<ICOMAdminCatalog> spCatalog;
    if (SUCCEEDED(hr = CoCreateInstance(__uuidof(COMAdminCatalog),
                          NULL,
                          CLSCTX_SERVER,
                          IID_PPV_ARGS(&spCatalog))))
    {
        hr = RemoveTransientSubscription(spCatalog, m_bstrSubscriptionID);
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Name         : CSysLCESub::EventName
// Purpose      : Translates from the EventEnum type to a text string
// Parameters   :
//              : EventEnum e
// Return Value : LPCWSTR
////////////////////////////////////////////////////////////////////////////////////////////////////
LPCWSTR CSysLCESub::EventName(EventEnum e)
{
    switch (e)
    {
        case Application:
        {                        
            return L"Application";
            break;
        }
        case Thread:
        {                        
            return L"Thread";
            break;
        }
        case Instance:
        {                        
            return L"Instance";
            break;
        }
        case Transaction:
        {
            return L"Transaction";
            break;
        }
        case Method:
        {                        
            return L"Method";
            break;
        }
        case Object:
        {                        
            return L"Object";
            break;
        }
        case Resource:
        {                        
            return L"Resource";
            break;
        }
        case User:
        {                        
            return L"User";
            break;
        }
        case Security:
        {                        
            return L"Security";
            break;
        }
        case ObjectConstruction:
        {                        
            return L"Construction";
            break;
        }
        case ObjectPool:
        {                        
            return L"Pool";
            break;
        }
        case ObjectPool2:
        {                        
            return L"Pool2";
            break;
        }
        case Activity:
        {                        
            return L"Activity";
            break;
        }
        case Identity:
        {                        
            return L"Identity";
            break;
        }
        case QC:
        {                        
            return L"Queued Components";
            break;
        }
        case Exception:
        {                        
            return L"Exception";
            break;
        }
        case CRM:
        {                        
            return L"CRM";
            break;
        }
        case EventStore:
        {                        
            return L"EventStore";
            break;
        }
        case LoadBalancing: 
        {                        
            return L"LoadBalancing";
            break;
        }
        default:
            ATLTRACE(L"Object type not implemented in SysLCESub\n");
            _ASSERTE(0);
            break;
    }
    return NULL;
}
