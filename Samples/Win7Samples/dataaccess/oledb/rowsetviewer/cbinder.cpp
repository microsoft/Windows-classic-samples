//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CBINDER.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CBinder::CBinder
//
/////////////////////////////////////////////////////////////////
CBinder::CBinder(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CBase(eCBinder, pCMainWindow, pCMDIChild)
{
	//OLE DB Interfaces
	m_pIBindResource		= NULL;		//Binder interface
	m_pICreateRow			= NULL;		//Binder interface
	m_pIDBProperties		= NULL;		//Binder interface
	m_pIDBBinderProperties	= NULL;		//Binder interface
	
	m_pIRegisterProvider	= NULL;		//Binder interface

	//Data
	m_pwszURL				= NULL;
}


/////////////////////////////////////////////////////////////////
// CBinder::~CBinder
//
/////////////////////////////////////////////////////////////////
CBinder::~CBinder()
{
	ReleaseObject(0);

	//Save Recent URL into the Registry
	if(m_pwszURL)
	{
		DelRegEntry(HKEY_ROWSETVIEWER, wszRECENTURL_KEY);
		SetRegEntry(HKEY_ROWSETVIEWER, wszRECENTURL_KEY, m_pwszURL, L"");
		SAFE_FREE(m_pwszURL);
	}
}


/////////////////////////////////////////////////////////////////
// IUnknown** CBinder::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CBinder::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IBindResource);
	HANDLE_GETINTERFACE(ICreateRow);
	HANDLE_GETINTERFACE(IDBBinderProperties);
	HANDLE_GETINTERFACE(IDBProperties);
	HANDLE_GETINTERFACE(IRegisterProvider);

	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// HRESULT CBinder::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CBinder::AutoRelease()
{
	RELEASE_INTERFACE(IBindResource);
	RELEASE_INTERFACE(ICreateRow);
	RELEASE_INTERFACE(IDBProperties);
	RELEASE_INTERFACE(IDBBinderProperties);
	RELEASE_INTERFACE(IRegisterProvider);

	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CBinder::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CBinder::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);

	//[MANDATORY] Obtain [mandatory] interfaces
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IBindResource);
		OBTAIN_INTERFACE(IDBProperties);
	}

	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		OBTAIN_INTERFACE(ICreateRow);
		OBTAIN_INTERFACE(IDBBinderProperties);
		OBTAIN_INTERFACE(IRegisterProvider);
	}

	//Get the Recent Saved URL
	if(!m_pwszURL)
		GetRegEnumValue(HKEY_ROWSETVIEWER, wszRECENTURL_KEY, 0, &m_pwszURL);
	if(!m_pwszURL)
		m_pwszURL = wcsDuplicate(L"http://");

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CBinder::GetObjectDesc
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* CBinder::GetObjectDesc()
{
	if(!m_strObjectDesc && m_guidSource != GUID_NULL)
	{
		if(m_guidSource == CLSID_RootBinder)
			m_strObjectDesc.CopyFrom(L"Root Binder");
		else
			m_strObjectDesc.CopyFrom(GetProgID(m_guidSource));
	}

	return m_strObjectDesc;
}



////////////////////////////////////////////////////////////////
// CBinder::OnDefOperation
//
/////////////////////////////////////////////////////////////////
void	CBinder::OnDefOperation() 
{ 
	//Display the Bind Dialog (by default)
	if(m_pIBindResource)
		m_pCMainWindow->DisplayDialog(IDD_BINDRESOURCE, GetFocus(), CMainWindow::BindResourceProc, this, IDM_IBINDRESOURCE_BIND);
}


/////////////////////////////////////////////////////////////////
// HRESULT CBinder::CreateBinder
//
/////////////////////////////////////////////////////////////////
HRESULT CBinder::CreateBinder(REFCLSID clsid)
{
	HRESULT	hr = S_OK;
	CComPtr<IBindResource>	spBindResource;

	//Create the Binder object (optional object)...
	XTESTC(hr = m_pCMainWindow->m_pCRootEnumerator->CreateInstance(NULL, clsid, CLSCTX_ALL, IID_IBindResource, (IUnknown**)&spBindResource));

	//Delegate
	m_guidSource = clsid;
	TESTC(hr = CreateObject(NULL, IID_IBindResource, spBindResource));

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CBinder::SetProperties
//
/////////////////////////////////////////////////////////////////
HRESULT CBinder::SetProperties(ULONG cPropSets, DBPROPSET* rgPropSets)
{
	HRESULT	hr = S_OK;

	if(m_pIDBProperties && cPropSets)
	{
		//SetProperties
		XTEST_(hr = m_pIDBProperties->SetProperties(cPropSets, rgPropSets),S_OK);
		TRACE_METHOD(hr, L"IDBProperties::SetProperties(%d, 0x%p)", cPropSets, rgPropSets);

		//Display any property errors...
		TESTC(hr = DisplayPropErrors(hr, cPropSets, rgPropSets));
	}

CLEANUP:
	return hr;
}


