//--------------------------------------------------------------------
// Microsoft OLE DB Sample OLEDB Simple Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation.  All Rights Reserved.
//
// module MyDataSource.cpp | MyDataSource (OLEDBSimpleProvider) object implementation
//
//

////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////
#include "Common.h"
#include "MyDataSource.h"


////////////////////////////////////////////////////////
// MyDataSource
//
////////////////////////////////////////////////////////

MyDataSource::MyDataSource()
{
	//IUnknown
	m_cRef = 0;

	//OLEDBSimpleProvider
	m_pMyOSPObject = NULL;

	//DataMembers (tables)
	m_cTables = 1;
	m_rgpwszTableName[0] = L"";
}

MyDataSource::~MyDataSource()
{
	// Remove all Listeners:
	while (!m_listListeners.IsEmpty())
		m_listListeners.RemoveHead()->Release();

	// Remove the Simple provider instance:
	SAFE_RELEASE(m_pMyOSPObject);
}

HRESULT MyDataSource::Init()
{	
	//Create a new Simple Provider instance:
	m_pMyOSPObject = new MyOSPObject;
	if (!m_pMyOSPObject)
		return E_OUTOFMEMORY;
	SAFE_ADDREF(m_pMyOSPObject);
	return S_OK;
}

HRESULT MyDataSource::QueryInterface(REFIID riid, void** ppv)
{
	if (ppv == NULL)
		return E_INVALIDARG;
	
	// IUnknown:
	if (riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
		
	// DataSource:
	else if (riid == IID_DataSource)
		*ppv = (IDataSource*)this;

	// Unsupported:
	else
	{
		*ppv = NULL;								 
		return E_NOINTERFACE;
	}

	SAFE_ADDREF((IUnknown*)*ppv);
	return S_OK;
}


////////////////////////////////////////////////////////
// IDataSource implementation
//
////////////////////////////////////////////////////////
HRESULT MyDataSource::getDataMember(BSTR bstrDM, REFIID riid, IUnknown** ppUnk)
{
	HRESULT hr = S_OK;

	if (ppUnk==NULL)
		return E_INVALIDARG;

	if (bstrDM == NULL || bstrDM[0] == L'\0')
	{
		hr = m_pMyOSPObject->Init(m_rgpwszTableName[0]);
	}
	else
	{
		hr = m_pMyOSPObject->Init(bstrDM);
	}
	if (hr != S_OK)
		return hr;

	hr = m_pMyOSPObject->QueryInterface(riid, (void**)ppUnk);
	return hr;
}

HRESULT MyDataSource::getDataMemberName(LONG lIndex, BSTR* pbstrDM)
{
	if (lIndex < 0 || lIndex >= (LONG)m_cTables || pbstrDM==NULL)
		return E_INVALIDARG;
	
	//Return indexed TableName
	*pbstrDM = SysAllocString(m_rgpwszTableName[lIndex]);
	return S_OK;
}

HRESULT MyDataSource::getDataMemberCount(LONG* plCount)
{
	if(plCount==NULL)
		return E_INVALIDARG;
	
	//Return number of tables
	*plCount = m_cTables;
	return S_OK;
}

        
HRESULT MyDataSource::addDataSourceListener(IDataSourceListener* pIDataSourceListener)
{
	if (pIDataSourceListener == NULL)
		return E_INVALIDARG;
	
	// Add the Listener:
	pIDataSourceListener->AddRef();
	m_listListeners.AddTail(pIDataSourceListener);
	return S_OK;
}

HRESULT MyDataSource::removeDataSourceListener(IDataSourceListener* pIDataSourceListener)
{
	if (pIDataSourceListener == NULL)
		return E_INVALIDARG;

	// Find the Listener in the list and release it:
	POS pos = m_listListeners.Find(pIDataSourceListener);
	if (pos == NULL)
		return E_FAIL;
	SAFE_RELEASE(pIDataSourceListener);
	m_listListeners.RemoveAt(pos);
	return S_OK;
}

