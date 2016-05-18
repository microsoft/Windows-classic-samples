//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CROWPOSITION.H
//
//-----------------------------------------------------------------------------------

#ifndef _CROWPOSITION_H_
#define _CROWPOSITION_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
// CRowPosition class
//
/////////////////////////////////////////////////////////////////
class CRowPosition : public CContainerBase
{
public:
	//Constructors
	CRowPosition(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CRowPosition();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"RowPosition";		} 
	virtual UINT			GetObjectMenu()			{ return IDM_ROWPOSITIONMENU;	}
	virtual LONG			GetObjectImage()		{ return IMAGE_ARROW_DOWN;		}
	virtual REFIID			GetDefaultInterface()	{ return IID_IRowPosition;		}

	//Interface Helpers
	virtual HRESULT			Initialize(IUnknown* pIUnkRowset);
	virtual HRESULT			GetRowset(REFIID riid, IUnknown** ppIUnknown);

	//OLE DB Interfaces
	//[MANDATORY]
	IRowPosition*				m_pIRowPosition;

	//Extra interfaces
	DWORD						m_dwCookieRowPos;
};





#endif //_CROWPOSITION_H_