//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CDATALINKS.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CServiceComp::CServiceComp
//
/////////////////////////////////////////////////////////////////
CServiceComp::CServiceComp(CMainWindow* pCMainWindow) 
	: CBase(eCServiceComp, pCMainWindow, NULL)
{
	//OLE DB Interfaces
	m_pIDataInitialize			= NULL;		//DataLink interface
	
	
	//Data
	m_pwszInitString			= NULL;
	GetRegEnumValue(HKEY_ROWSETVIEWER, wszRECENTINITSTRING_KEY, 0,	&m_pwszInitString);
}

/////////////////////////////////////////////////////////////////
// CServiceComp::~CServiceComp
//
/////////////////////////////////////////////////////////////////
CServiceComp::~CServiceComp()
{
	ReleaseObject(0);

	//ServiceComponents
	DelRegEntry(HKEY_ROWSETVIEWER, wszRECENTINITSTRING_KEY);
	SetRegEntry(HKEY_ROWSETVIEWER, wszRECENTINITSTRING_KEY,	m_pwszInitString, L"");

	SAFE_FREE(m_pwszInitString);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CServiceComp::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CServiceComp::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IDataInitialize);

	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CServiceComp::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::AutoRelease()
{
	//OLE DB Interfaces
	RELEASE_INTERFACE(IDataInitialize);


	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CServiceComp::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IDataInitialize);

	}

	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// HRESULT CServiceComp::Create
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::Create(CBase* pCSource, DWORD dwCSLCTX, WCHAR* pwszRemoteServer)
{
	HRESULT		hr = S_OK;
	CComPtr<IUnknown>	spUnknown;

	//CLSID_MSDAINITIALIZE
	XTESTC(hr = m_pCMainWindow->m_pCRootEnumerator->CreateInstance(NULL, CLSID_MSDAINITIALIZE, dwCSLCTX, IID_IUnknown, &spUnknown, pwszRemoteServer));

	//Delegate
	TESTC(hr = CBase::CreateObject(pCSource, IID_IUnknown, spUnknown));

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CServiceComp::OnDefOperation
//
/////////////////////////////////////////////////////////////////
void	CServiceComp::OnDefOperation() 
{ 
	//Need to bring up the GetDataSource Dialog
	if(m_pIDataInitialize)
		m_pCMainWindow->DisplayDialog(IDD_DATAINIT_GETDATASOURCE, GetFocus(), CMainWindow::GetDataSourceProc, this);
}


/////////////////////////////////////////////////////////////////
// HRESULT CServiceComp::CreateDBInstance
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::CreateDBInstance(REFCLSID clsid, CAggregate* pCAggregate, DWORD dwCLSCTX, REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT	hr			= E_FAIL;

	//CLSID to String (just for display purposes...)
	WCHAR* pwszProgID = GetProgID(clsid);

	//Defferred Creation
	if(!m_pIUnknown)
		TESTC(hr = Create(NULL));

	if(m_pIDataInitialize)
	{
		//Now Obtain Instance of Provider (with Service Components)
		XTEST(hr = m_pIDataInitialize->CreateDBInstance(clsid, pCAggregate, dwCLSCTX, NULL, riid, ppIUnknown));
		TESTC(TRACE_METHOD(hr, L"IDataInitialize::CreateDBInstance(%s, 0x%p, 0x%08x, NULL, %s, &0x%p)", pwszProgID, pCAggregate, dwCLSCTX, GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));

		//Handle Aggregation
		if(pCAggregate)
			TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));
	}

CLEANUP:
	SAFE_FREE(pwszProgID);
	return hr;
}


/////////////////////////////////////////////////////////////////
// CServiceComp::GetDataSource
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::GetDataSource(CAggregate* pCAggregate, DWORD dwCLSCTX, WCHAR* pwszInitString, REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT	hr			= E_FAIL;

	//Defferred Creation
	if(!m_pIUnknown)
		TESTC(hr = Create(NULL));

	if(m_pIDataInitialize)
	{
		//GetDataSource based upoon the InitString
		XTEST(hr = m_pIDataInitialize->GetDataSource(pCAggregate, dwCLSCTX, pwszInitString, riid, ppIUnknown));
		TESTC(TRACE_METHOD(hr, L"IDataInitialize::GetDataSource(0x%p, 0x%08x, \"%s\", %s, &0x%p)", pCAggregate, GetOptions()->m_dwCLSCTX, pwszInitString, GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));

		//Handle Aggregation
		if(pCAggregate)
			TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));
	}

CLEANUP:
	return hr;
}

	
/////////////////////////////////////////////////////////////////
// CServiceComp::GetInitString
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::GetInitString(IUnknown* pIUnknown, boolean fIncludePassword, WCHAR** ppwszInitString)
{
	HRESULT	hr				= E_FAIL;

	//Defferred Creation
	if(!m_pIUnknown)
		TESTC(hr = Create(NULL));

	if(m_pIDataInitialize)
	{
		//GetInitializationString
		XTEST(hr = m_pIDataInitialize->GetInitializationString(pIUnknown, fIncludePassword, ppwszInitString));
		TESTC(TRACE_METHOD(hr, L"IDataInitialize::GetInitializationString(0x%p, %s, &\"%s\")", pIUnknown, fIncludePassword ? L"True" : L"False", ppwszInitString ? *ppwszInitString : NULL));
	}

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////
// CServiceComp::SaveInitString
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::SaveInitString(WCHAR* pwszFileName, WCHAR* pwszInitString, DWORD dwCreateOpts)
{
	HRESULT	hr			= E_FAIL;

	//Defferred Creation
	if(!m_pIUnknown)
		TESTC(hr = Create(NULL));

	if(m_pIDataInitialize)
	{
		//WriteStringToStorage
		XTEST(hr = m_pIDataInitialize->WriteStringToStorage(pwszFileName, pwszInitString, dwCreateOpts));
		TESTC(TRACE_METHOD(hr, L"IDataInitialize::WriteStringToStorage(\"%s\", \"%s\", 0x%08x)", pwszFileName, pwszInitString, dwCreateOpts));
	}

CLEANUP:
	return hr;
}

	
/////////////////////////////////////////////////////////////////
// CServiceComp::LoadInitString
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::LoadInitString(WCHAR* pwszFileName, WCHAR** ppwszInitString)
{
	HRESULT	hr				= E_FAIL;

	//Defferred Creation
	if(!m_pIUnknown)
		TESTC(hr = Create(NULL));

	if(m_pIDataInitialize)
	{
		//LoadStringFromStorage
		XTEST(hr = m_pIDataInitialize->LoadStringFromStorage(pwszFileName, ppwszInitString));
		TESTC(TRACE_METHOD(hr, L"IDataInitialize::LoadStringFromStorage(\"%s\", &\"%s\")", pwszFileName, ppwszInitString ? *ppwszInitString : NULL));
	}

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////
// CServiceComp::ConnectFromFile
//
/////////////////////////////////////////////////////////////////
HRESULT CServiceComp::ConnectFromFile(WCHAR* pwszSelectedFile)
{
	HRESULT hr = S_OK;
	WCHAR* pwszInitString = NULL;
	CComPtr<IUnknown> spUnknown;

	//No-op
	if(pwszSelectedFile == NULL)
		return E_FAIL;

	//Load the saved InitString from the SelectedFile
	TESTC(hr = LoadInitString(pwszSelectedFile, &pwszInitString));

	//Delegate - Now that we have the InitString
	TESTC(hr = GetDataSource(NULL, GetOptions()->m_dwCLSCTX, pwszInitString, IID_IUnknown, &spUnknown));

	//Update the Saved Files (now that successfuly connected)
	m_pCMainWindow->m_pCFullConnect->AddRecentFile(pwszSelectedFile);

	//Handle the returned object...
	//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
	if(!m_pCMainWindow->HandleObjectType(this, spUnknown, IID_IUnknown, eCDataSource, 0, NULL, CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE | GetOptions()->m_dwCreateOpts))
		TESTC(hr = E_FAIL);
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	return hr;
}





/////////////////////////////////////////////////////////////////
// CDataLinks::CDataLinks
//
/////////////////////////////////////////////////////////////////
CDataLinks::CDataLinks(CMainWindow* pCMainWindow) 
	: CBase(eCDataLinks, pCMainWindow, NULL)
{
	//OLE DB Interfaces
	m_pIDBPromptInitialize		= NULL;		//DataLink interface
	
	//Data
}

/////////////////////////////////////////////////////////////////
// CDataLinks::~CDataLinks
//
/////////////////////////////////////////////////////////////////
CDataLinks::~CDataLinks()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CDataLinks::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CDataLinks::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IDBPromptInitialize);

	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CDataLinks::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CDataLinks::AutoRelease()
{
	//OLE DB Interfaces
	RELEASE_INTERFACE(IDBPromptInitialize);

	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CDataLinks::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CDataLinks::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IDBPromptInitialize);
	}

	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// HRESULT CDataLinks::Create
//
/////////////////////////////////////////////////////////////////
HRESULT CDataLinks::Create(CBase* pCSource, DWORD dwCSLCTX, WCHAR* pwszRemoteServer)
{
	HRESULT		hr = S_OK;
	CComPtr<IUnknown> spUnknown;

	//CLSID_MSDAINITIALIZE
	XTESTC(hr = m_pCMainWindow->m_pCRootEnumerator->CreateInstance(NULL, CLSID_DataLinks, dwCSLCTX, IID_IUnknown, &spUnknown, pwszRemoteServer));

	//Delegate
	TESTC(hr = CBase::CreateObject(pCSource, IID_IUnknown, spUnknown));

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CDataLinks::OnDefOperation
//
/////////////////////////////////////////////////////////////////
void	CDataLinks::OnDefOperation() 
{ 
	//PromptDataSource dialog
	CComPtr<IDBInitialize> spDBInitialize;
			
	//Just display the PromptDataSource dialog directly (common case)
	if(SUCCEEDED(PromptDataSource(NULL, m_pCMainWindow->m_hWnd, DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDBInitialize, (IUnknown**)&spDBInitialize)))
	{
		//Handle the returned object...
		//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
		m_pCMainWindow->HandleObjectType(this, spDBInitialize, IID_IDBInitialize, eCDataSource, 0, NULL, CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE | GetOptions()->m_dwCreateOpts);
	}
}


/////////////////////////////////////////////////////////////////
// CDataLinks::PromptDataSource
//
/////////////////////////////////////////////////////////////////
HRESULT CDataLinks::PromptDataSource(CAggregate* pCAggregate, HWND hWndParent, DBPROMPTOPTIONS dwPromptOptions, ULONG cTypeFilters, DBSOURCETYPE* rgTypeFilters, WCHAR* pwszProvFilter, REFIID riid, IUnknown** ppDataSource)
{
	HRESULT	hr			= E_FAIL;

	//Defferred Creation
	if(!m_pIUnknown)
		TESTC(hr = Create(NULL));

	if(m_pIDBPromptInitialize)
	{
		//IDBPromptInitialize::PromptDataSource
		//NOTE: Expect S_OK or DB_E_CANCELED, since canceling the dialog always returns DB_E_CANCELED
		XTEST_(hr = m_pIDBPromptInitialize->PromptDataSource(pCAggregate, hWndParent, dwPromptOptions, cTypeFilters, rgTypeFilters, pwszProvFilter, riid, ppDataSource),DB_E_CANCELED);
		TESTC(TRACE_METHOD(hr, L"IDBPromptInitialize::PromptDataSource(0x%p, 0x%p, 0x%08x, %d, 0x%p, \"%s\", %s, &0x%p)", pCAggregate, hWndParent, dwPromptOptions, cTypeFilters, rgTypeFilters, pwszProvFilter, GetInterfaceName(riid), ppDataSource ? *ppDataSource : NULL));

		//Handle Aggregation
		if(pCAggregate)
			TESTC(hr = pCAggregate->HandleAggregation(riid, ppDataSource));
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// CDataLinks::PromptFileName
//
/////////////////////////////////////////////////////////////////
HRESULT CDataLinks::PromptFileName(HWND hWndParent, DBPROMPTOPTIONS dwPromptOptions, WCHAR* pwszDirectory, WCHAR* pwszFileName, WCHAR** ppwszSelectedFile)
{
	HRESULT	hr			= E_FAIL;

	//Defferred Creation
	if(!m_pIUnknown)
		TESTC(hr = Create(NULL));

	if(m_pIDBPromptInitialize)
	{
		//IDBPromptInitalize::PromptFileName
		//NOTE: Expect S_OK or DB_E_CANCELED, since canceling the dialog always returns DB_E_CANCELED
		XTEST_(hr = m_pIDBPromptInitialize->PromptFileName
					(
					hWndParent,							// hWndParent
					dwPromptOptions,					// dwPromptOptions
            		pwszDirectory,						// pwszInitialDirectory
            		pwszFileName,						// pwszInitialFile
            		ppwszSelectedFile					// pwszSelectedFile
					), DB_E_CANCELED);
		TESTC(TRACE_METHOD(hr, L"IDBPromptInitialize::PromptFileName(0x%p, 0x%08x, \"%s\", \"%s\", &\"%s\")", hWndParent, dwPromptOptions, pwszDirectory, pwszFileName, ppwszSelectedFile ? *ppwszSelectedFile : NULL));
	}

CLEANUP:
	return hr;
}




