//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer					   
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CENUM.H
//
//-----------------------------------------------------------------------------------

#ifndef _CENUM_H_
#define _CENUM_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "CBase.h"
#include "CDataSource.h"


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
struct ENUMINFO 
{
	WCHAR			wszName[MAX_NAME_LEN];
	WCHAR			wszParseName[MAX_NAME_LEN];
	WCHAR			wszDescription[MAX_NAME_LEN];
	USHORT			eType;
	VARIANT_BOOL	fIsParent;
};



/////////////////////////////////////////////////////////////////
// CEnumerator class
//
/////////////////////////////////////////////////////////////////
class CEnumerator : public CPropertiesBase
{
public:
	//Constructors
	CEnumerator(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CEnumerator();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Enumerator";			} 
	virtual UINT			GetObjectMenu()			{ return IDM_ENUMERATORMENU;	}
	virtual LONG			GetObjectImage()		{ return IMAGE_OBJECTS;			}
	virtual REFIID			GetDefaultInterface()	{ return IID_ISourcesRowset;	}
	virtual WCHAR*			GetObjectDesc();
	virtual	void			OnDefOperation();

	//Connection
	virtual HRESULT			CreateInstance(CAggregate* pCAggregate, REFCLSID clsid, DWORD dwCLSCTX, REFIID riid, IUnknown** ppIUnknown, WCHAR* pwszRemoteServer = NULL);
	virtual HRESULT			ParseDisplayName(WCHAR* pwszParseName, DWORD dwCLSCTX, REFIID riid, IUnknown** ppIUnknown, CBase** ppCSource, DWORD dwConnectOpts = 0, WCHAR* pwszRemoteServer = NULL);
	virtual HRESULT			GetSourcesRowset(CAggregate* pCAggregate, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown);

	virtual HRESULT			Create(REFCLSID clsid);
	virtual HRESULT 		EnumerateInfo(ULONG* pcEnumInfo, ENUMINFO** prgEnumInfo);
	virtual HRESULT			CreateEnumInfo(REFCLSID clsid = GUID_NULL, BOOL fRefresh = FALSE);

	//[MANDATORY]
	ISourcesRowset*			m_pISourcesRowset;				//Enumerator interface
	IParseDisplayName*		m_pIParseDisplayName;			//Enumerator interface

	//[OPTIONAL]

	//Enumerator ProvierInfo
	ULONG					m_cEnumInfo;
	ENUMINFO*				m_rgEnumInfo;
};


#endif	//_CENUM_H_
