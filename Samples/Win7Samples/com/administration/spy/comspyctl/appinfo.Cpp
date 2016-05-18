// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "AppInfo.h"

#include "ComSpyCtl.h"
#include "ComSpyAudit.h"
#include "CComSpy.h"
#include <algorithm>

#include "SysLCESub.h"        // Includes for all the event consumers.
#include "AppSub.h"            // Implement App Events
#include "InstanceSub.h"    // Implement Instance Events
#include "EvtStoreSub.h"    // Implement COM+ Event System Events
#include "TxSub.h"            // Implement Transaction Events
#include "ObjSub.h"            // Implement Object Events
#include "MethodSub.h"        // Implement Method Events
#include "ResourceSub.h"    // Implement Resource Events
#include "SecuritySub.h"    // Implement Security Events
#include "ThreadSub.h"        // Implement Thread Events
#include "UserSub.h"        // Implement User Defined Events
#include "ObjConstSub.h"    // Implement Object Construction Events
#include "ObjPoolSub.h"        // Implement Object Pool Events
#include "ObjPool2Sub.h"    // Implement more Object Pool Events
#include "ActivitySub.h"    // Implement Activity Events
#include "IdentitySub.h"    // Implement Identity Events
#include "QCSub.h"            // Implement Queued Components Events
#include "ExceptionSub.h"    // Implement Exception Events
#include "crmsub.h"            // Implement CRM Events
#include "lbsub.h"            // Implement COM+ Load Balancing Events  



// -----------------------------------------------------------------------------
// Method:    CAppInfo::Initialze(...)
// Params:  pSpy:    Pointer to a CComSpy object.
// Returns: (None)
// Purpose:    Initialize the class and create the Event Subscriber MAP.
// -----------------------------------------------------------------------------
void CAppInfo::Initialze(CComSpy * pSpy)
{
    m_EventMap = new SUBSCRIBERMAP;
    m_bReadyForDelete = false;
    m_pSpy = pSpy;
};



// -----------------------------------------------------------------------------
// Method:    CAppInfo::CAppInfo(...)
// Params:  pwszAppName:    A string representing 'All Applications'
//                pSpy:            Pointer to a CComSpy object.
// Returns: (None)
// Purpose:    Constructor for the special, NoFilter instance of this class.
// -----------------------------------------------------------------------------
CAppInfo::CAppInfo(LPCWSTR pwszAppName, CComSpy * pSpy)
{
    Initialze(pSpy);
    m_sAppName = pwszAppName;
    m_FilterType = NoFilter;
    m_sAppID = L"";
    m_PID = 0;
};



// -----------------------------------------------------------------------------
// Method:    CAppInfo::CAppInfo(...)
// Params:  pwszAppName:    A string representing 'All Applications'
//                pSpy:            Pointer to a CComSpy object.
//                nPID:            ProcessID for this running app.
// Returns: (None)
// Purpose:    Constructor for the Running Application version of this object.
// -----------------------------------------------------------------------------
CAppInfo::CAppInfo(LPCWSTR pwszAppName, CComSpy * pSpy, long nPID)
{
    Initialze(pSpy);
    m_sAppName = pwszAppName;
    m_FilterType = ProcessID;
    m_sAppID = L"";
    m_PID = nPID;
};



// -----------------------------------------------------------------------------
// Method:    CAppInfo::CAppInfo(...)
// Params:  pwszAppName:    String representing the AppName.
//                pSpy:            Pointer to a CComSpy object.
//                pwszAppID:        String representing AppID (a formatted GUID)
// Returns: (None)
// Purpose:    Constructor for the standard case of filtering by AppID.
//                Note that the COM+ Filtering does not want a Application Name, but
//                instead wants an Application ID as a GUID.
// -----------------------------------------------------------------------------
CAppInfo::CAppInfo(LPCWSTR pwszAppName, CComSpy * pSpy, LPCWSTR pwszAppID)
{
    Initialze(pSpy);
    m_sAppName = pwszAppName;
    m_FilterType = AppID;
    m_sAppID = pwszAppID;
    m_PID = 0;
};



// -----------------------------------------------------------------------------
// Method:    CAppInfo::~CAppInfo()
// Params:  (None)
// Returns: (None)
// Purpose:    Destructor for the CAppInfo class.  Remove All Subscriptions.
//                and delete the map.
// ToDo:        Do we need to empty this Map first?
// -----------------------------------------------------------------------------
CAppInfo::~CAppInfo()
{
    bool bResult = RemoveAllSubscriptions();
    delete m_EventMap;
    m_EventMap = NULL;
 }



// -----------------------------------------------------------------------------
// Method:    CAppInfo::RemoveAllSubscriptions()
// Params:  (None)
// Returns: True always
// Purpose:    Remove all subscriptions held by this Application object. Iterate 
//                through the event map and free each.
//    Note:        We must call Release() because these are COM objects.
// -----------------------------------------------------------------------------
bool CAppInfo::RemoveAllSubscriptions()
{
    SUBSCRIBERMAP::iterator item;
    ICOMSysLCE* pSubscriber = NULL;

    // Iterate through the list of Events to delete them all.
    item = m_EventMap->begin();
    while (item != m_EventMap->end())
    {
        pSubscriber = (*item).second;
        if (pSubscriber)
        {
            pSubscriber->Uninstall();
            pSubscriber->Release();
            pSubscriber = NULL;
        }
        item = m_EventMap->erase(item);
    };

    return true;
}



// -----------------------------------------------------------------------------
// Method:    CAppInfo::AddSubscription(...)
// Params:  e: The EventEnum that we want to subscribe to. (eg. Method)
// Returns: true if the subscription was added successfully.
// Purpose:    To create an Interface pointer to the appropriate Subscription 
//                type within COM+. These all derive from IComSysLCE, so we use
//                the polymorphism to do this. Once we have the specific interface
//                required, we then determine which filter (if any) to use and we
//                Install the filter.
// -----------------------------------------------------------------------------
bool CAppInfo::AddSubscription(EventEnum e)
{
    HRESULT hr;
    ICOMSysLCE* pSubscriber = NULL;

    if (IsSubscribed(e))
        return false;

    switch (e)
    {
        // Note: The QI will do our AddRef
        case Application:
        {                        
            CComObject <CAppSub> * pSub;
            hr = CComObject<CAppSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Thread:
        {                        
            CComObject <CThreadSub> * pSub;
            hr = CComObject<CThreadSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Instance:
        {                        
            CComObject <CInstanceSub> * pSub;
            hr = CComObject<CInstanceSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Transaction:
        {
            CComObject <CTxSub> * pSub;
            hr = CComObject<CTxSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Method:
        {                        
            CComObject <CMethodSub> * pSub;
            hr = CComObject<CMethodSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Object:
        {                        
            CComObject <CObjSub> * pSub;
            hr = CComObject<CObjSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Resource:
        {                        
            CComObject <CResourceSub> * pSub;
            hr = CComObject<CResourceSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case User:
        {                        
            CComObject <CUserSub> * pSub;
            hr = CComObject<CUserSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Security:
        {                        
            CComObject <CSecuritySub> * pSub;
            hr = CComObject<CSecuritySub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case ObjectConstruction:
        {                        
            CComObject <CObjConstSub> * pSub;
            hr = CComObject<CObjConstSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case ObjectPool:
        {                        
            CComObject <CObjPoolSub> * pSub;
            hr = CComObject<CObjPoolSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case ObjectPool2:
        {                        
            CComObject <CObjPool2Sub> * pSub;
            hr = CComObject<CObjPool2Sub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Activity:
        {                        
            CComObject <CActivitySub> * pSub;
            hr = CComObject<CActivitySub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Identity:
        {                        
            CComObject <CIdentitySub> * pSub;
            hr = CComObject<CIdentitySub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case QC:
        {                        
            CComObject <CQCSub> * pSub;
            hr = CComObject<CQCSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case Exception:
        {                        
            CComObject <CExceptionSub> * pSub;
            hr = CComObject<CExceptionSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case CRM:
        {                        
            CComObject <CCRMSub> * pSub;
            hr = CComObject<CCRMSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        //Events generated by system process  
        case EventStore:
        {                        
            CComObject <CEvtStoreSub> * pSub;
            hr = CComObject<CEvtStoreSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        case LoadBalancing:  
        {                        
            CComObject <CLBSub> * pSub;
            hr = CComObject<CLBSub>::CreateInstance(&pSub);
            if (FAILED(hr))
                return false;
            _ASSERTE(pSub);
            pSub->SetSpyObj(m_pSpy);
            pSub->QueryInterface(IID_PPV_ARGS(&pSubscriber));
            break;
        }
        default:
            ATLTRACE(L"Object type not implemented yet\n");
            _ASSERTE(0);
            break;
    }

    // We've successfully created the Subscriber interface.
    if (pSubscriber)     
    {
        CComBSTR    bstrPropertyName;
        CComVariant vPropertyValue;
        BSTR*        pbstrPropertyName = NULL;

        // Set the filter strings according to the object type.
        switch (m_FilterType)
        {
        case ProcessID:
            vPropertyValue = m_PID;
            bstrPropertyName = L"ProcessId";
            pbstrPropertyName = (BSTR*)&bstrPropertyName;
            break;
        case AppID:
            bstrPropertyName = L"AppId";
            vPropertyValue = m_sAppID;
            pbstrPropertyName = (BSTR*)&bstrPropertyName;
            break;
        case NoFilter:
            // If this is a filtertype of All Apps, then pass NULL.
            pbstrPropertyName = NULL;
            vPropertyValue = L"";
            break;
        default:
            _ASSERTE(0);
        };

        // Register the subscription.
        pSubscriber->Install(pbstrPropertyName, vPropertyValue);

        // Add the event Subscription to the MAP.
        (*m_EventMap)[e] = pSubscriber;
    };

    return true;
};



// -----------------------------------------------------------------------------
// Method:    CAppInfo::RemoveAllSubscriptions()
// Params:  e: The EventEnum to remove the subscription for.
// Returns: true if the subscrioption was removed.
// Purpose:    Remove the Event Subscription for a given EventEnum.
//    Note:        We must call Release() because these are COM objects.
// -----------------------------------------------------------------------------
bool CAppInfo::RemoveSubscription(EventEnum e)
{
    ICOMSysLCE* pSubscriber = NULL;

    if (!IsSubscribed(e))
        return false;

    // Get the Subscriber interface pointer.
    pSubscriber = (*m_EventMap)[e];
    if (pSubscriber)
    {
        pSubscriber->Uninstall();
        pSubscriber->Release();
        pSubscriber = NULL;
        m_EventMap->erase(e);
        return true;
    };    

    return false;
}



// -----------------------------------------------------------------------------
// Method:    CAppInfo::IsSubscribed(...)
// Params:  e: the EventEnum to check
// Returns: True if the given Event is currently subscribed to, otherwise False.
// Purpose:    Check whether or not this Application is subscribed to an event.
// -----------------------------------------------------------------------------
bool CAppInfo::IsSubscribed(EventEnum e)
{
    ICOMSysLCE* pSubscriber = NULL;
    pSubscriber = (*m_EventMap)[e];
    bool bResult = pSubscriber ? true : false;
    pSubscriber = NULL;
    return bResult;
};



// -----------------------------------------------------------------------------
// Method:    CAppInfo::IsSubscribedToAny()
// Params:  (None)
// Returns: True if any Event is currently subscribed to, otherwise False.
// Purpose:    Check whether or not this Application is subscribed to any events.
// -----------------------------------------------------------------------------
bool CAppInfo::IsSubscribedToAny()
{
    long nSubs = 0;
    SUBSCRIBERMAP::iterator item;
    ICOMSysLCE* pSubscriber = NULL;
    for (item = m_EventMap->begin(); item != m_EventMap->end(); ++item)
    {
        pSubscriber = (*item).second;
        if (pSubscriber)
            nSubs++;
    };
    return (nSubs > 0) ? true : false;
};
