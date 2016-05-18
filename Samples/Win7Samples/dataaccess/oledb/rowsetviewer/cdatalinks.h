//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CDATALINKS.H
//
//-----------------------------------------------------------------------------------

#ifndef _CDATALINKS_H_
#define _CDATALINKS_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////
// CServiceComp class
//
/////////////////////////////////////////////////////////////////
class CServiceComp : public CBase
{
public:
	//Constructors
	CServiceComp(CMainWindow* pCMainWindow);
	virtual ~CServiceComp();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Service Components";	}
	virtual UINT			GetObjectMenu()			{ return IDM_SERVICECOMPMENU;	}
	virtual LONG			GetObjectImage()		{ return IMAGE_SERVICECOMP;		}
	virtual REFIID			GetDefaultInterface()	{ return IID_IDataInitialize;	}
	virtual	void			OnDefOperation();

	//Helpers
	virtual HRESULT			Create(CBase* pCSource, DWORD dwCSLCTX = CLSCTX_INPROC_SERVER, WCHAR* pwszRemoteServer = NULL);

	//IDataInitialize
	virtual	HRESULT			CreateDBInstance(REFCLSID clsid, CAggregate* pCAggregate, DWORD dwCLSCTX, REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			GetDataSource(CAggregate* pCAggregate, DWORD dwCLSCTX, WCHAR* pwszInitString, REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			GetInitString(IUnknown* pIUnknown, boolean fIncludePassword, WCHAR** ppwszInitString);
	virtual HRESULT			SaveInitString(WCHAR* pwszFileName, WCHAR* pwszInitString, DWORD dwCreateOpts = CREATE_ALWAYS);
	virtual HRESULT			LoadInitString(WCHAR* pwszFileName, WCHAR** pwszInitString = NULL);

	//Helpers
	virtual HRESULT			ConnectFromFile(WCHAR* pwszInitString);


	//Data
	WCHAR*							m_pwszInitString;

	//OLE DB Interfaces
	//[MANADATORY]
	IDataInitialize*				m_pIDataInitialize;				//DataLink interface

	//[OPTIONAL]

	//Extra interfaces
};



/////////////////////////////////////////////////////////////////
// CDataLinks class
//
/////////////////////////////////////////////////////////////////
class CDataLinks : public CBase
{
public:
	//Constructors
	CDataLinks(CMainWindow* pCMainWindow);
	virtual ~CDataLinks();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Data Links";				}
	virtual UINT			GetObjectMenu()			{ return IDM_DATALINKSMENU;			}
	virtual LONG			GetObjectImage()		{ return IMAGE_DATALINKS;			}
	virtual REFIID			GetDefaultInterface()	{ return IID_IDBPromptInitialize;	}
	virtual	void			OnDefOperation();

	//Helpers
	virtual HRESULT			Create(CBase* pCSource, DWORD dwCSLCTX = CLSCTX_INPROC_SERVER, WCHAR* pwszRemoteServer = NULL);

	//IDBPromptInitialize
	virtual HRESULT			PromptDataSource(CAggregate* pCAggregate, HWND hWndParent, DBPROMPTOPTIONS dwPromptOptions, ULONG cTypeFilters, DBSOURCETYPE* rgTypeFilters, WCHAR* pwszProvFilter, REFIID riid, IUnknown** ppDataSource);
	virtual HRESULT			PromptFileName(HWND hWndParent, DBPROMPTOPTIONS dwPromptOptions, WCHAR* pwszDirectory, WCHAR* pwszFileName, WCHAR** ppwszSelectedFile);

	//OLE DB Interfaces
	//[MANADATORY]
	IDBPromptInitialize*	m_pIDBPromptInitialize;			//DataLink interface

	//[OPTIONAL]
};


#endif	//_CDATALINKS_H_
