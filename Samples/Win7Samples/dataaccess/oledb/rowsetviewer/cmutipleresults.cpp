//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CMUTIPLERESULTS.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CMultipleResults::CMultipleResults
//
/////////////////////////////////////////////////////////////////
CMultipleResults::CMultipleResults(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CBase(eCMultipleResults, pCMainWindow, pCMDIChild)
{
	//MultipleResults
	m_pIMultipleResults				= NULL;//MultipleResults Interface
}


/////////////////////////////////////////////////////////////////
// CMultipleResults::~CMultipleResults
//
/////////////////////////////////////////////////////////////////
CMultipleResults::~CMultipleResults()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CMultipleResults::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CMultipleResults::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IMultipleResults);

	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CMultipleResults::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CMultipleResults::AutoRelease()
{
	//MultipleResults
	RELEASE_INTERFACE(IMultipleResults);

	//Delegate
	return CBase::AutoRelease();
}


////////////////////////////////////////////////////////////////
// CMultipleResults::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CMultipleResults::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IMultipleResults);
	}

	//Auto QI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////
// CMultipleResults::GetResult
//
/////////////////////////////////////////////////////////////////
HRESULT CMultipleResults::GetResult(CAggregate* pCAggregate, DB_LRESERVE lResultFlag, REFIID riid, DBROWCOUNT* pcRowsAffected, IUnknown** ppIUnknown)
{
	HRESULT hr = E_FAIL;

	//No-op
	if(m_pIMultipleResults == NULL)
		return E_FAIL;
	
	//IMultipleResults::GetResult...
	XTEST(hr = m_pIMultipleResults->GetResult(pCAggregate, lResultFlag, riid, pcRowsAffected, ppIUnknown));
	TESTC(TRACE_METHOD(hr, L"IMultipleResults::GetResult(0x%p, %d, %s, &%Id, &0x%p)", pCAggregate, lResultFlag, GetInterfaceName(riid), pcRowsAffected ? *pcRowsAffected : NULL, ppIUnknown ? *ppIUnknown : NULL));

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));

CLEANUP:
	return hr;
}


