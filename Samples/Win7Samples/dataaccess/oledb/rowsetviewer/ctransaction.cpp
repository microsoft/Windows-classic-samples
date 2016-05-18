//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CTRANSCATION.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"


////////////////////////////////////////////////////////////////
// CTransaction::CTransaction
//
/////////////////////////////////////////////////////////////////
CTransaction::CTransaction(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CContainerBase(eCTransaction, pCMainWindow, pCMDIChild)
{
	//OLE DB Interfaces
	m_pITransaction				= NULL;		//Transaction interface
	
	//Extra interfaces
	m_dwCookieTransNotify		= 0;
}

/////////////////////////////////////////////////////////////////
// CTransaction::~CTransaction
//
/////////////////////////////////////////////////////////////////
CTransaction::~CTransaction()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CTransaction::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CTransaction::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(ITransaction);

	//Otherwise delegate
	return CContainerBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CTransaction::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CTransaction::AutoRelease()
{
	//DataSource interfaces
	RELEASE_INTERFACE(ITransaction);

	//Delegate
	return CContainerBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CTransaction::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CTransaction::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CContainerBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(ITransaction);
	}
	
	//Auto QI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	return S_OK;
}



	
////////////////////////////////////////////////////////////////
// CTransactionOptions::CTransactionOptions
//
/////////////////////////////////////////////////////////////////
CTransactionOptions::CTransactionOptions(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CBase(eCTransactionOptions, pCMainWindow, pCMDIChild)
{
	//OLE DB Interfaces
	m_pITransactionOptions		= NULL;		//TransactionOptions interface
}

/////////////////////////////////////////////////////////////////
// CTransactionOptions::~CTransactionOptions
//
/////////////////////////////////////////////////////////////////
CTransactionOptions::~CTransactionOptions()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CTransactionOptions::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CTransactionOptions::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(ITransactionOptions);

	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CTransactionOptions::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CTransactionOptions::AutoRelease()
{
	//Interfaces
	RELEASE_INTERFACE(ITransactionOptions);

	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CTransactionOptions::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CTransactionOptions::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(ITransactionOptions);
	}
	
	//Auto QI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	return S_OK;
}
