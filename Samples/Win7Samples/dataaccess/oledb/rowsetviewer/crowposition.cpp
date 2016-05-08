//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CROWPOSITION.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CRowPosition::CRowPosition
//
/////////////////////////////////////////////////////////////////
CRowPosition::CRowPosition(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CContainerBase(eCRowPosition, pCMainWindow, pCMDIChild)
{
	//RowPosition
	m_pIRowPosition					= NULL;//RowPosition Interface

	//Extra Interfaces
	m_dwCookieRowPos				= 0;
}


/////////////////////////////////////////////////////////////////
// CRowPosition::~CRowPosition
//
/////////////////////////////////////////////////////////////////
CRowPosition::~CRowPosition()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CRowPosition::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CRowPosition::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IRowPosition);

	//Otherwise delegate
	return CContainerBase::GetInterfaceAddress(riid);
}

/////////////////////////////////////////////////////////////////
// CRowPosition::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CRowPosition::AutoRelease()
{
	//RowPosition
	RELEASE_INTERFACE(IRowPosition);

	//Don't Unadvise the Listeners until the Rowset goes away.
	//We want to receive and Display the ROWSET_RELEASE notifications
	UnadviseListener(IID_IRowPositionChange, &m_dwCookieRowPos);

	//Delegate
	return CContainerBase::AutoRelease();
}


////////////////////////////////////////////////////////////////
// CRowPosition::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CRowPosition::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have IConnectionPointContainer
	CContainerBase::AutoQI(dwCreateOpts);

	//[MANDATORY] Obtain [mandatory] interfaces
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IRowPosition);
	}

	//AutoQI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	//Advise Listeners
	AdviseListener(IID_IRowPositionChange, &m_dwCookieRowPos);
	return S_OK;
}


////////////////////////////////////////////////////////////////
// CRowPosition::Initialize
//
/////////////////////////////////////////////////////////////////
HRESULT CRowPosition::Initialize(IUnknown* pIUnkRowset)
{
	HRESULT hr = S_OK;

	//Initialize
	XTEST(hr = m_pIRowPosition->Initialize(pIUnkRowset));
	TESTC(TRACE_METHOD(hr, L"IRowPosition::Initialize(0x%p)", pIUnkRowset));
	
CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CRowPosition::GetRowset
//
/////////////////////////////////////////////////////////////////
HRESULT CRowPosition::GetRowset(REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;
	
	if(m_pIRowPosition)
	{
		//IRowPosition::GetRowset
		XTEST(hr = m_pIRowPosition->GetRowset(riid, ppIUnknown));
		TESTC(TRACE_METHOD(hr, L"IRowPosition::GetRowset(%s, &0x%p)", GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));
	}

CLEANUP:
	return hr;
}


