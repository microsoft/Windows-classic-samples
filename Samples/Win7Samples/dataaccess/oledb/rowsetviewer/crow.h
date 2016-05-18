//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CROW.H
//
//-----------------------------------------------------------------------------------

#ifndef _CROW_H_
#define _CROW_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "CRowset.h"	//CDataAccess


/////////////////////////////////////////////////////////////////
// CRow 
//
/////////////////////////////////////////////////////////////////
class CRow : public CDataAccess
{
public:
	//Constructors
	CRow(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CRow();
	
	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Row";			} 
	virtual UINT			GetObjectMenu()			{ return IDM_ROWMENU;		}
	virtual LONG			GetObjectImage()		{ return IMAGE_FORM;		}
	virtual REFIID			GetDefaultInterface()	{ return IID_IRow;			}

	virtual HRESULT			DisplayObject();
	virtual WCHAR*			GetObjectDesc();

	//Members
	virtual HRESULT			SetupColAccess(BINDCOLS eBindCols = BIND_ALLCOLS);

	virtual HRESULT			CreateCommand(CAggregate* pCAggregate, REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			GetColumns(ULONG cColAccess, DBCOLUMNACCESS* rgColAccess);
	virtual HRESULT			Open(CAggregate* pCAggregate, DBID* pColumnID, REFGUID rguidObjectType, REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			Bind(CAggregate* pCAggregate, WCHAR* pwszURL, DBBINDURLFLAG dwBindFlags, REFGUID rguidObjectType, REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			OpenRowset(CAggregate* pCAggregate, DBID* pTableID, DBID* pIndexID, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown);

	//Row
	//[MANADATORY]
	IRow*						m_pIRow;
	IGetSession*				m_pIGetSession;

	//[OPTIONAL]
	IColumnsInfo2*				m_pIColumnsInfo2;
	ICreateRow*					m_pICreateRow;
	IDBCreateCommand*			m_pIDBCreateCommand;
	IRowChange*					m_pIRowChange;
	IRowSchemaChange*			m_pIRowSchemaChange;
	IBindResource*				m_pIBindResource;
	IScopedOperations*			m_pIScopedOperations;

	//ColAccess
	ULONG						m_cColAccess;
	DBCOLUMNACCESS*				m_rgColAccess;

	//Data
	HROW						m_hSourceRow;
};




#endif //_CROW_H_