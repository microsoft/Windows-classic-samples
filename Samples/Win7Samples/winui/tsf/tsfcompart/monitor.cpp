/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation. All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          Monitor.cpp

   Description:   

**************************************************************************/

/**************************************************************************
	#include statements
**************************************************************************/

#include "Monitor.h"

/**************************************************************************
	global variables and definitions
**************************************************************************/

/**************************************************************************

	CCompartmentMonitor::CCompartmentMonitor()

**************************************************************************/

CCompartmentMonitor::CCompartmentMonitor(void)
{
    m_dwRef = 1;
    m_dwCookie = 0;
    m_pCompartment = NULL;
    m_guidCompartment = GUID_NULL;
    m_pCallback = NULL;
}

/**************************************************************************

	CCompartmentMonitor::~CCompartmentMonitor()

**************************************************************************/

CCompartmentMonitor::~CCompartmentMonitor()
{
    Uninitialize();
}

/**************************************************************************

	CCompartmentMonitor::Initialize()

**************************************************************************/

HRESULT CCompartmentMonitor::Initialize(    const GUID *pguidCompartment,
                                            PCOMPARTMENTMONITORPROC pCallback, 
                                            LPARAM lParam)
{
    if(!IsEqualGUID(m_guidCompartment, GUID_NULL))
    {
        //Initialize() has already been called
        return E_UNEXPECTED;
    }

    m_guidCompartment = *pguidCompartment;
    m_pCallback = pCallback;
    m_lParam = lParam;
    
    HRESULT         hr;
    ITfThreadMgr    *pThreadMgr;
    
    //create a thread manager object
    hr = CoCreateInstance(CLSID_TF_ThreadMgr, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_ITfThreadMgr, 
        (void**)&pThreadMgr);
    
    if(SUCCEEDED(hr))
    {
        ITfCompartmentMgr   *pCompMgr;

        //get the global compartment manager
        hr = pThreadMgr->GetGlobalCompartment(&pCompMgr);
        if(SUCCEEDED(hr))
        {
            //get the Speech UI compartment 
            hr = pCompMgr->GetCompartment(m_guidCompartment, 
                &m_pCompartment);
            if(SUCCEEDED(hr))
            {
                ITfSource   *pSource;
                
                //install the advise sink
                hr = m_pCompartment->QueryInterface(IID_ITfSource, 
                    (LPVOID*)&pSource);
                if(SUCCEEDED(hr))
                {
                    hr = pSource->AdviseSink(IID_ITfCompartmentEventSink, 
                        (ITfCompartmentEventSink*)this,
                        &m_dwCookie);
                }
                
                //if something went wrong, release the member interface
                if(FAILED(hr))
                {
                    m_pCompartment->Release();
                    m_pCompartment = NULL;
                }
            }
            
            //release the compartment manager
            pCompMgr->Release();
        }
        
        //release the thread manager
        pThreadMgr->Release();
    }

    return hr;
}

/**************************************************************************

	CCompartmentMonitor::Uninitialize()

**************************************************************************/

HRESULT CCompartmentMonitor::Uninitialize(void)
{
    HRESULT     hr = S_OK;

    if(m_pCompartment)
    {
        ITfSource   *pSource;

        hr = m_pCompartment->QueryInterface(IID_ITfSource, (void **)&pSource);

        if(SUCCEEDED(hr))
        {
            hr = pSource->UnadviseSink(m_dwCookie);

            pSource->Release();
        }
    
        m_pCompartment->Release();
        m_pCompartment = NULL;
    }

    m_guidCompartment = GUID_NULL;
    
    return hr;
}

//
// IUnknown
//

/**************************************************************************

	CCompartmentMonitor::QueryInterface()

**************************************************************************/

STDAPI CCompartmentMonitor::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfCompartmentEventSink))
    {
        *ppvObj = (ITfCompartmentEventSink*)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

/**************************************************************************

	CCompartmentMonitor::AddRef()

**************************************************************************/

STDAPI_(ULONG) CCompartmentMonitor::AddRef()
{
    return ++m_dwRef;
}

/**************************************************************************

	CCompartmentMonitor::Release()

**************************************************************************/

STDAPI_(ULONG) CCompartmentMonitor::Release()
{
    m_dwRef--;

    if(0 == m_dwRef)
    {
        delete this;
        return 0;
    }

    return m_dwRef;
}

//
// ITfCompartmentEventSink
//

/**************************************************************************

	CCompartmentMonitor::OnChange()

**************************************************************************/

STDAPI CCompartmentMonitor::OnChange(REFGUID rguid)
{
    HRESULT hr = S_OK;

    if(IsEqualGUID(rguid, m_guidCompartment))
    {
        GUID guid = m_guidCompartment;
        BOOL fStatus;

        GetStatus(&fStatus);

        //notify the status of this compartment has changed
        hr = (*m_pCallback)(&guid, fStatus, m_lParam);
    }

    return hr;
}

/**************************************************************************

	CCompartmentMonitor::GetStatus()

**************************************************************************/

HRESULT CCompartmentMonitor::GetStatus(BOOL *pfEnabled)
{
    if(NULL == pfEnabled)
    {
        return E_INVALIDARG;
    }

    if(!m_pCompartment)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr;
    VARIANT var;

    VariantInit(&var);
    hr = m_pCompartment->GetValue(&var);
    if(S_OK == hr)
    {
        switch(var.vt)
        {
        case VT_I4:
            *pfEnabled = var.lVal;
            break;
        }

        VariantClear(&var);
    }
    else
    {
        //this usually means the text service is not installed
        return E_UNEXPECTED;
    }
    
    
    return S_OK;
}
