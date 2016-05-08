//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CTRANSACTION.H
//
//-----------------------------------------------------------------------------------

#ifndef _CTRANSACTION_H_
#define _CTRANSACTION_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////
// CTransaction class
//
/////////////////////////////////////////////////////////////////
class CTransaction : public CContainerBase
{
public:
	//Constructors
	CTransaction(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CTransaction();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Transaction";		} 
	virtual UINT			GetObjectMenu()			{ return IDM_TRANSACTIONMENU;	}
	virtual LONG			GetObjectImage()		{ return IMAGE_FORM;			}
	virtual REFIID			GetDefaultInterface()	{ return IID_ITransaction;		}

	//Methods

	//Helpers

	//OLE DB Interfaces
	//[MANDATORY]
	ITransaction*					m_pITransaction;				//Transaction interface

	//[OPTIONAL]

	//Extra Interfaces
	DWORD							m_dwCookieTransNotify;
};



/////////////////////////////////////////////////////////////////
// CTransactionOptions class
//
/////////////////////////////////////////////////////////////////
class CTransactionOptions : public CBase
{
public:
	//Constructors
	CTransactionOptions(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CTransactionOptions();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"TransactionOptions";			} 
	virtual UINT			GetObjectMenu()			{ return IDM_TRANSACTIONOPTIONSMENU;	}
	virtual LONG			GetObjectImage()		{ return IMAGE_FORM;					}
	virtual REFIID			GetDefaultInterface()	{ return IID_ITransactionOptions;		}

	//Methods

	//Helpers

	//OLE DB Interfaces
	//[MANDATORY]
	ITransactionOptions*			m_pITransactionOptions;			//TransactionOptions interface

	//[OPTIONAL]
};


#endif	//_CTRANSACTION_H_
