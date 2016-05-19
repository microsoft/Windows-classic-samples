//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CBINDER.H
//
//-----------------------------------------------------------------------------------

#ifndef _CBINDER_H_
#define _CBINDER_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////
// CBinder class
//
/////////////////////////////////////////////////////////////////
class CBinder : public CBase
{
public:
	//Constructors
	CBinder(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CBinder();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);
	virtual	void			OnDefOperation();

	//Pure Virtual
	virtual WCHAR*	GetObjectName()			{ return L"Binder";			} 
	virtual UINT	GetObjectMenu()			{ return IDM_BINDERMENU;	}
	virtual LONG	GetObjectImage()		{ return IMAGE_CHAPTER;		}
	virtual REFIID	GetDefaultInterface()	{ return IID_IBindResource; }
	virtual WCHAR*	GetObjectDesc();

	//Methods
	virtual HRESULT CreateBinder(REFCLSID clsidProv);
	virtual HRESULT SetProperties(ULONG cPropSets, DBPROPSET* rgPropSets);

	//OLE DB Interfaces
	//[MANDATORY]
	IBindResource*			m_pIBindResource;			//Binder interface
	ICreateRow*				m_pICreateRow;				//Binder interface
	IDBProperties*			m_pIDBProperties;			//Binder interface
	IDBBinderProperties*	m_pIDBBinderProperties;		//Binder interface

	//[OPTIONAL]
	IRegisterProvider*		m_pIRegisterProvider;		//Binder interface

	//Saved URL
	WCHAR*					m_pwszURL;
};




#endif	//_CBINDER_H_
