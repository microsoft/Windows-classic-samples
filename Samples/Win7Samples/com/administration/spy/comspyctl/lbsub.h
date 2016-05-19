// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __LBSUB_H__
#define __LBSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CLBSub
Summary: Load Balancing Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CLBSub : 
	public CSysLCESub,
	public ILBEvents
{
public:
	CLBSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CLBSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CLBSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CLBSub)
		COM_INTERFACE_ENTRY(ILBEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return LoadBalancing; }
	virtual REFCLSID EventCLSID() { return CLSID_LBEvents; }
    virtual REFIID EventIID() { return IID_ILBEvents; }

	////////////////
	STDMETHODIMP TargetUp (BSTR bstrServerName, BSTR bstrClsidEng)
	{
		LONGLONG perfCount = 0;
		::QueryPerformanceCounter( (LARGE_INTEGER *)&perfCount );
		
		m_pSpy->AddEventToList(perfCount, L"Target Up", L"COM+ Load Balancing");
		m_pSpy->AddParamValueToList(L"ServerName", bstrServerName);
		m_pSpy->AddParamValueToList(L"Engine ID", bstrClsidEng);

		IF_AUDIT_DO(TargetUp)(perfCount,bstrServerName,bstrClsidEng);	

		return S_OK;
	}


	STDMETHODIMP TargetDown (BSTR bstrServerName, BSTR bstrClsidEng)
	{
		LONGLONG perfCount = 0;
		::QueryPerformanceCounter( (LARGE_INTEGER *)&perfCount );
		
		m_pSpy->AddEventToList(perfCount, L"Target Down", L"COM+ Load Balancing");
		m_pSpy->AddParamValueToList(L"ServerName", bstrServerName);
		m_pSpy->AddParamValueToList(L"Engine ID", bstrClsidEng);

		IF_AUDIT_DO(TargetDown)(perfCount,bstrServerName,bstrClsidEng);															

		return S_OK;
	}

	STDMETHODIMP EngineDefined (BSTR bstrPropName, VARIANT *varPropValue, BSTR bstrClsidEng)
	{
		//This event has not been implemented yet and will be active in the future.
		return S_OK;
	}

};

#endif //__LBSUB_H__
