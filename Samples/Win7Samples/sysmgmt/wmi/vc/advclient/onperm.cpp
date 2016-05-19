// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnPerm.cpp
//
// Description:
//	This file implements the register/unregister of permenant
//		event consumers. The actual consumer is the WBEMPermEvents
//		project.
// 
// History:
//
// **************************************************************************
#include "stdafx.h"
#include "AdvClientDlg.h"
#include <tchar.h>

// my specific class GUID.
// {1E069401-087E-11d1-AD85-00AA00B8E05A}
static const GUID CLSID_AdvClientConsumer = 
{ 0x1e069401, 0x87e, 0x11d1, { 0xad, 0x85, 0x0, 0xaa, 0x0, 0xb8, 0xe0, 0x5a } };

// ========================================================
BOOL CAdvClientDlg::PermRegistered()
{
	HRESULT hRes;
	BOOL retval = FALSE;
	IWbemClassObject *pConsumerInst = NULL;
	BSTR instName = SysAllocString(L"__EventFilter.Name=\"Sample Perm Query\"");
	if (!instName)
	{		
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return FALSE;
	}

	// switch to the root\cimv2\office namespace.
	if(EnsureOfficeNamespace())
	{
		// if the EventFilter is there...
		if((hRes = m_pOfficeService->GetObject(instName,
												0L,	NULL,
												&pConsumerInst,
												NULL)) == S_OK)
		{
			pConsumerInst->Release();
			pConsumerInst = NULL;

			// assume the perm sink is registered.
			retval = TRUE;
		}
	}

	SysFreeString(instName);
	return retval;
}

// ========================================================
BOOL CAdvClientDlg::OnPermRegister() 
{
	BOOL retval = FALSE;
	IWbemClassObject *pConsumer = NULL;
	IWbemClassObject *pFilter = NULL;
	
	m_eventList.ResetContent();

	// switch to the root\cimv2\office namespace.
	if(EnsureOfficeNamespace())
	{
		if(GetConsumer(&pConsumer))
		{
			if(AddFilter(&pFilter))
			{
				if(AddBinding(pConsumer, pFilter))
				{
					retval = TRUE;
					m_eventList.AddString(_T("Permanent is ready"));
				}
				else
				{
					m_eventList.AddString(_T("Failed To Register"));
				} //endif AddBinding()
			}
			else
			{
			} //endif AddFilter()
		}
		else
		{
		} //endif AddConsumer()
	}
	return retval;
}
// ========================================================
void CAdvClientDlg::OnPermUnregister() 
{
	HRESULT hRes;
	BSTR instName;

	if(EnsureOfficeNamespace())
	{
		// delete the filter instance.
		instName = SysAllocString(L"__EventFilter.Name=\"Sample Perm Query\"");
		if (!instName)
		{		
			TRACE(_T("SysAllocString failed: not enough memory\n"));
			return;
		}
		if((hRes = m_pOfficeService->DeleteInstance(instName,
													0L,	NULL,
													NULL)) != S_OK)
		{
			TRACE(_T("Cant delete filter, %s\n"), ErrorString(hRes));
		}
		SysFreeString(instName);

		// delete the association instance.
		// NOTE: Remember the triple backslashes on the inner quotes.
		instName = SysAllocString(L"__FilterToConsumerBinding.Consumer=\"SampleViewerConsumer.Name=\\\"Sample says:\\\"\",Filter=\"__EventFilter.Name=\\\"Sample Perm Query\\\"\"");
		if (!instName)
		{		
			TRACE(_T("SysAllocString failed: not enough memory\n"));
			return;
		}
		if((hRes = m_pOfficeService->DeleteInstance(instName,
													0L,	NULL,
													NULL)) != S_OK)
		{
			TRACE(_T("Cant delete association, %s\n"), ErrorString(hRes));
		}

		SysFreeString(instName);
	}
	m_eventList.ResetContent();
	m_eventList.AddString(_T("Permanent is removed"));
}

//------------------------------------------------------------
BOOL CAdvClientDlg::GetConsumer(IWbemClassObject **ppConsumer)
{
	BOOL retval = FALSE;
	HRESULT hRes;
	BSTR curInst;
	CString str(L"SampleViewerConsumer.Name=\"Sample says:\"");
//	CString ConsumerCLSID(L"{1E069401-087E-11d1-AD85-00AA00B8E05A}");

	curInst = str.AllocSysString();
	if (! curInst)
	{
		TRACE(_T("AllocSysString failed"));

		return FALSE;	
	}

	//---------------------
	if((hRes = m_pOfficeService->GetObject(curInst, 0L, NULL,
											ppConsumer,	NULL)) == S_OK)
	{
		retval = TRUE;
	}
	else
	{
		TRACE(_T("Second GetObject() pConsumerInst failed: %s\n"), ErrorString(hRes));
	} //endif GetObject()

	SysFreeString(curInst);

	return retval;
}
// ========================================================
BOOL CAdvClientDlg::AddFilter(IWbemClassObject **ppFilter)
{
	BOOL retval = FALSE;

	HRESULT hRes;
	VARIANT v;
	VariantInit(&v);

	CString FilterName(_T("Sample Perm Query"));
	CString FilterClass(L"__EventFilter");
	CString FilterClassKey(L"Name");
	CString QueryProp(L"Query");
	CString LangProp(L"QueryLanguage");
	CString theQuery(L"select * from __InstanceCreationEvent where TargetInstance isa \"SAMPLE_OfficeEquipment\"");
	BSTR Prop = NULL;
	BSTR curInst = NULL;
	BSTR FilterClassName = NULL;

	IWbemClassObject *pFilterClass = NULL;
	IWbemClassObject *pFilterInst = NULL;

	
	FilterClassName = FilterClass.AllocSysString();
	if (!FilterClassName)
		goto OutOfMemory;


	//---------------------
	// if class exists...
	if((hRes = m_pOfficeService->GetObject(FilterClassName,
									0L,	NULL,
									&pFilterClass,
									NULL)) == S_OK)
	{
		//---------------------
		// spawn a new instance
		if((hRes = pFilterClass->SpawnInstance(0, &pFilterInst)) == S_OK)
		{
			TRACE(_T("SpawnInstance() worked\n"));

			// set the key property.
			V_VT(&v) = VT_BSTR;
			V_BSTR(&v) = FilterName.AllocSysString();
			if (!V_BSTR(&v))
				goto OutOfMemory;

			Prop = FilterClassKey.AllocSysString();
			if (!Prop)
				goto OutOfMemory;

			pFilterInst->Put(Prop, 0, &v, 0);
			VariantClear(&v);
			SysFreeString(Prop);

			V_VT(&v) = VT_BSTR;
			V_BSTR(&v) = theQuery.AllocSysString();
			if (!V_BSTR(&v))
				goto OutOfMemory;
			Prop = QueryProp.AllocSysString();
			if (!Prop)
				goto OutOfMemory;
			pFilterInst->Put(Prop, 0, &v, 0);
			VariantClear(&v);
			SysFreeString(Prop);

			V_VT(&v) = VT_BSTR;
			V_BSTR(&v) = SysAllocString(L"WQL");
			if (!V_BSTR(&v))
				goto OutOfMemory;
			Prop = LangProp.AllocSysString();
			if (!Prop)
				goto OutOfMemory;
			pFilterInst->Put(Prop, 0, &v, 0);
			VariantClear(&v);
			SysFreeString(Prop);

			//---------------------
			// putInstance
			hRes = m_pOfficeService->PutInstance(pFilterInst, 0, NULL, NULL);

			pFilterInst->Release();
			pFilterInst = NULL;

			CString str = FilterClass + _T(".") + FilterClassKey;
			str += _T("=\"");
			str += FilterName;
			str += _T("\"");

			curInst = str.AllocSysString();
			if (!curInst)
				goto OutOfMemory;

			TRACE(_T("flushing instance for %s\n"), str);

			//---------------------
			if((hRes = m_pOfficeService->GetObject(curInst, 0L, NULL,
											ppFilter, NULL)) == S_OK)
			{
				retval = TRUE;
			}
			else
			{
				TRACE(_T("Second GetObject() pFilterInst failed: %s\n"), ErrorString(hRes));
			} //endif GetObject()

			SysFreeString(curInst);
		}
		else
		{
			TRACE(_T("SpawnInstance() failed: %s\n"), ErrorString(hRes));

		} //endif SpawnInstance()

		pFilterClass->Release();  // Don't need the class any more
		pFilterClass = NULL;

	} // endif class doesnt exist.
	
	SysFreeString(FilterClassName);
	return retval;

OutOfMemory:
	SysFreeString (FilterClassName);
	SysFreeString (Prop);
	SysFreeString (curInst);

	if (pFilterClass)
		pFilterClass->Release();
	if (pFilterInst)
		pFilterInst->Release();

	VariantClear(& v);

	TRACE(_T("SysAllocString failed: not enough memory\n"));

	return FALSE;

}
// ========================================================
BOOL CAdvClientDlg::AddBinding(IWbemClassObject *pConsumer, 
								IWbemClassObject *pFilter)
{
	BOOL retval = FALSE;
	HRESULT hRes;
	VARIANT v;
	VariantInit(&v);

	IWbemClassObject *pAClass = NULL;
	IWbemClassObject *pAInst = NULL;

	BSTR AssocClassName = SysAllocString(L"__FilterToConsumerBinding");
	if (!AssocClassName)
	{
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return FALSE;
	}

	BSTR ConsumerProp = SysAllocString(L"Consumer");
	if (!ConsumerProp)
	{
		SysFreeString(AssocClassName);
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return FALSE;
	}

	BSTR FilterProp = SysAllocString(L"Filter");
	if (!FilterProp)
	{
		SysFreeString(AssocClassName);
		SysFreeString(ConsumerProp);
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return FALSE;
	}

	BSTR PathProp = SysAllocString(L"__RELPATH");
	if (!PathProp)
	{
		SysFreeString(AssocClassName);
		SysFreeString(ConsumerProp);
		SysFreeString(FilterProp);
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return FALSE;
	}

	// if association class exists...
	if((hRes = m_pOfficeService->GetObject(AssocClassName,
										0L,	NULL,
										&pAClass,
										NULL)) == S_OK)
	{
		//------------------------------
		// spawn a new instance.
		if((hRes = pAClass->SpawnInstance(0, &pAInst)) == S_OK)
		{
			TRACE(_T("Assoc SpawnInstance() worked\n"));


			//-----------------------------------
			// set consumer instance name
			if ((hRes = pConsumer->Get(PathProp, 0L, 
									&v, NULL, NULL)) == S_OK) 
			{
				hRes = pAInst->Put(ConsumerProp, 0, &v, 0);
				TRACE(_T("Got Consumer ref: %s\n"), ErrorString(hRes));
				VariantClear(&v);
			}
			else
			{
				TRACE(_T("Get() consumer ref failed: %s\n"), ErrorString(hRes));
			} //endif Get()

			//-----------------------------------
			// set Filter ref
			if ((hRes = pFilter->Get(PathProp, 0L, 
									&v, NULL, NULL)) == S_OK) 
			{
				hRes = pAInst->Put(FilterProp, 0, &v, 0);
				TRACE(_T("Got filter ref: %s\n"), ErrorString(hRes));
				VariantClear(&v);
			}
			else
			{
				TRACE(_T("Get() filter ref failed: %s\n"), ErrorString(hRes));
			} //endif Get()

			SysFreeString(PathProp);

			//-----------------------------------
			// putInstance
			if((hRes = m_pOfficeService->PutInstance(pAInst, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL)) == S_OK)
			{
				retval = TRUE;
			}
			else
			{
				TRACE(_T("PutInstance() __FilterToConsumerBinding failed: %s\n"), ErrorString(hRes));
			}
			
			pAInst->Release();
			pAInst = NULL;
		}
		else
		{
			TRACE(_T("SpawnInstance() failed: %s\n"), ErrorString(hRes));

		} //endif SpawnInstance()
	}
	else
	{
		TRACE(_T("GetObject() class failed: %s\n"), ErrorString(hRes));

	} // endif GetObject() class.
	

	if(pAClass)
	{
		pAClass->Release();
		pAClass = NULL;
	}

	SysFreeString(AssocClassName);
	SysFreeString(ConsumerProp);
	SysFreeString(FilterProp);

	return retval;
}

