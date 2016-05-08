//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CMAINWINDOW.H
//
//-----------------------------------------------------------------------------------

#ifndef _CMAINWINDOW_H_
#define _CMAINWINDOW_H_


//////////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////////
enum IMAGEICONS
{
	IMAGE_NORMAL		= 0,
	IMAGE_CHECKED		= 1,
	IMAGE_UNCHECKED		= 2,
	IMAGE_LOCK			= 3,
	IMAGE_QUESTION		= 4,
	IMAGE_ERROR			= 5,
	IMAGE_FORM			= 6,
	IMAGE_ARROW_UP		= 7,
	IMAGE_ARROW_DOWN	= 8,
	IMAGE_DELETE		= 9,
	IMAGE_CHANGE		= 10,
	IMAGE_INSERT		= 11,
	IMAGE_CHAPTER		= 12,
	IMAGE_MULTIPLE		= 13,
	IMAGE_OBJECTS		= 14,
	IMAGE_DATASOURCE	= 15,
	IMAGE_SESSION		= 16,
	IMAGE_COMMAND		= 17,
	IMAGE_TABLE			= 18,
	IMAGE_SERVICECOMP	= 19,
	IMAGE_DATALINKS		= 19,
	IMAGE_REMOTING		= 20,
	IMAGE_DRAWNPOOL		= 21,
	IMAGE_POOLING		= 22,
	IMAGE_INTERFACE		= 23,
	IMAGE_STREAM		= 24,
	IMAGE_CUBE			= 25
};


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "Property.h"
#include "CDialog.h"
#include "CMDIChild.h"
#include "CListener.h"
#include "CTrace.h"
#include "CObjTree.h"
#include "CBinder.h"
#include "CRowset.h"
#include "COptions.h"
		

//////////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////////
enum STATEICONS
{
	STATE_NORMAL		= 0,
	STATE_CHECKED		= 1,
	STATE_UNCHECKED		= 2
};

enum COLUMN_DESC
{
	COLUMN_DBCID,
	COLUMN_TYPENAME,
	COLUMN_DATATYPE,
	COLUMN_SIZE,
	COLUMN_PREC,
	COLUMN_SCALE
};

enum INDEX_COLUMN_DESC
{
	COLUMN_ID,
	COLUMN_ORDER
};

enum COLUMN_ISCO
{
	COLUMN_ISCO_SRCURL,
	COLUMN_ISCO_DESTURL
};


/////////////////////////////////////////////////////////////////////
// CMainWindow
//
/////////////////////////////////////////////////////////////////////
class CMainWindow : public CMDIFrameLite
{
public:
	//constructors
	CMainWindow();
	virtual ~CMainWindow();

	//Messages
	virtual BOOL OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnDestroy();
	virtual BOOL OnInitialUpdate();

	virtual BOOL OnSize(WPARAM nType, REFPOINTS pts);
	virtual BOOL OnMenuSelect(UINT uID);
	virtual BOOL OnInitMenuPopup(HMENU hMenu, UINT uPos, BOOL fSysMenu);

	virtual BOOL OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);
	virtual BOOL OnDropFiles(HDROP hDrop);

	//IUnknown Menu
	virtual void OnAddRef(CBase* pCBase);
	virtual void OnRelease(CBase* pCBase);
	virtual void OnQueryInterface(CBase* pCBase);
	virtual void OnAutoQI(CBase* pCBase);
	virtual void OnReleaseAll(CBase* pCBase, BOOL fChildren = FALSE);
	virtual void OnSupportErrorInfo(CBase* pCBase);
	
	//ConnectionPoints
	virtual void OnFindConnectionPoint(CContainerBase* pCCPointBase, REFIID riid);
	virtual void OnGetConnectionInterface(CConnectionPoint* pCConnectionPoint);
	virtual void OnAdvise(CConnectionPoint* pCConnectionPoint);
	virtual void OnUnadvise(CConnectionPoint* pCConnectionPoint);

	//Asynchronous
	virtual void OnAbort(CAsynchBase* pCAsynchBase);
	virtual void OnGetStatus(CAsynchBase* pCAsynchBase);
	virtual void OnInitialize(CAsynchBase* pCAsynchBase);
	virtual void OnUninitialize(CAsynchBase* pCAsynchBase);
	
	//File Menu
	virtual void OnConnect();
	virtual void OnLoadStringFromStorage(WCHAR* pwszFileName = NULL);
	virtual void OnGetDataSource();
	virtual void OnPromptDataSource(BOOL fAdvanced = FALSE);
	virtual void OnPromptFileName(BOOL fAdvanced = FALSE);
	virtual void OnCreateFileMoniker();

	virtual void OnRootBinder();
	virtual void OnRootEnumerator();
	virtual void OnServiceComponents();
	virtual void OnDataLinks();

	virtual void OnLoadRecentConfig(UINT iID);
	virtual void OnLoadRecentFile(UINT iID);

	//Error Menu
	
	//Tools Menu
	virtual void OnOptions();
	virtual void OnTraceWindow();
	virtual void OnObjectsWindow();

	//Windows Menu
	virtual void OnDisconnect();
	virtual void OnTile(BOOL fHorizontal);
	virtual void OnCascade();
	virtual void OnArrangeIcons();
	virtual void OnCloseAll();
	virtual void OnNextWindow(BOOL fNextWindow);
	
	//Help Menu
	virtual void OnAbout();

	//Helpers
	virtual BOOL UpdateControls();
	virtual BOOL LoadSettings();
	virtual BOOL SaveSettings();

	//MDI Child Windows
	virtual	BOOL			RemoveAllChildren();
	virtual INT_PTR			DisplayDialog(UINT uID, HWND hWndParent, DLGPROC lpDialogFunc, CBase* pCObject, UINT idSource = 0);

	virtual CBase*			HandleObjectType(CBase* pCSource, IUnknown* pIUnknown, REFIID riid, SOURCE eSource, ULONG cPropSets, DBPROPSET* rgPropSets, DWORD dwFlags);
	
	virtual COptionsSheet*	GetOptions()	{ return m_pCOptionsSheet;			} 
	virtual CBase**			GetObjectAddress(SOURCE eSource = eCUnknown);
	virtual CBase*			GetObject(SOURCE eSource = eCUnknown, BOOL fAlways = FALSE);

	//Binder Procs
	static INT_PTR WINAPI	BindResourceProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI	RegisterProviderProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//IDataInitialize
	static INT_PTR WINAPI	GetDataSourceProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI	EnumPropertiesProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//IDBPromptInitialize
	static INT_PTR WINAPI	PromptDataSourceProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI	PromptFileNameProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Enum Procs
	static INT_PTR WINAPI	GetSourcesRowsetProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Error Procs
	static INT_PTR WINAPI	ErrorRecordsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Stream Procs
	static INT_PTR WINAPI	StreamViewerProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


	//Objects
	CFullConnect*			m_pCFullConnect;
 	COptionsSheet*			m_pCOptionsSheet;
	CBinder*				m_pCRootBinder;		
	CEnumerator*			m_pCRootEnumerator;
	CServiceComp*			m_pCServiceComp;
	CDataLinks*				m_pCDataLinks;
	CListener*				m_pCListener;

	//Controls
	CStatusBarLite			m_CStatusBar;
	CToolBarLite			m_CToolBar;
	
	//MDI Windows
	CMDITrace*				m_pCMDITrace;
	CMDIObjects*			m_pCMDIObjects;

	//Other
	HIMAGELIST				m_hImageList;
	HIMAGELIST				m_hStateList;

	//Handles
	HCURSOR					m_hCurSizeNS;
	HMODULE					m_hLibRichEdit20;
	HMODULE					m_hLibRichEdit10;

	//list of MTS interfaces
	CList<ITransaction*, ITransaction*>	m_listTransactions;

	//Source
	CBase*					m_pCSource;
	ULONG					m_idSource;
};




/////////////////////////////////////////////////////////////////////////////
// CAboutDlg
//
/////////////////////////////////////////////////////////////////////////////
class CAboutDlg : public CDialogLite
{
public:
	CAboutDlg();

	//Messages
	virtual BOOL OnInitDialog();

protected:
	//data
};



#endif //_CMAINWINDOW_H_
