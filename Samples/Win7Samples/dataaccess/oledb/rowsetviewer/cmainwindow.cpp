//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CMAINWINDOW.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//												   
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"
#include "version.h"	//ABOUT Box


/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////
enum 
{
	STATUSBAR_TEXT	= 0,
	STATUSBAR_DESC	= 1
} STATUSBAR;


////////////////////////////////////////////////////////////////
// CMainWindow::CMainWindow
//
/////////////////////////////////////////////////////////////////
CMainWindow::CMainWindow()
{
	//Objects
	m_pCRootBinder		= NULL;
	m_pCFullConnect		= NULL;
	m_pCOptionsSheet	= NULL;
	m_pCRootEnumerator	= NULL;
	m_pCServiceComp		= NULL;
	m_pCDataLinks		= NULL;
	m_pCListener		= NULL;

	//Controls
	m_pCMDITrace		= NULL;
	m_pCMDIObjects		= NULL;

	//Cursors
	m_hCurSizeNS		= NULL;
	m_hLibRichEdit20	= NULL;
	m_hLibRichEdit10	= NULL;
	
	//Bitmaps
	m_hImageList		= ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR);	
	m_hStateList		= ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_STATE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR);

	//make sure NULL is an option to pass into JoinTransaction as the transaction pointer
	m_listTransactions.AddTail(NULL);
}


////////////////////////////////////////////////////////////////
// CMainWindow::~CMainWindow
//
/////////////////////////////////////////////////////////////////
CMainWindow::~CMainWindow()
{
	ITransaction	*pITransaction	= NULL;

	while(!m_listTransactions.IsEmpty())
	{
		//get first in list
		pITransaction = m_listTransactions.GetHead();
		//Remove from list
		m_listTransactions.RemoveHead();

		//make sure this isn't the NULL option
		if (pITransaction)
		{
			//make sure the txn is stopped
			pITransaction->Abort(NULL,FALSE,FALSE);
		}
		//Free pITransaction
		SAFE_RELEASE(pITransaction);
	}

	SAFE_RELEASE(m_pCRootEnumerator);
	SAFE_RELEASE(m_pCRootBinder);
	SAFE_DELETE(m_pCFullConnect);
	SAFE_RELEASE(m_pCServiceComp);
	SAFE_RELEASE(m_pCDataLinks);
	SAFE_RELEASE(m_pCListener);

	if(m_hLibRichEdit20)
		FreeLibrary(m_hLibRichEdit20);
	if(m_hLibRichEdit10)
		FreeLibrary(m_hLibRichEdit10);

	SAFE_DELETE(m_pCMDIObjects);
	SAFE_DELETE(m_pCMDITrace);
	
	//Options
	if(m_pCOptionsSheet)
		m_pCOptionsSheet->SaveOptions();
	SAFE_DELETE(m_pCOptionsSheet);
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnCreate
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//Create the client area first...
	CMDIFrameLite::OnCreate(lpCreateStruct);
	
	//Load Saved Settings
	LoadSettings();

	//ToolBar
	const static TBBUTTON rgButtons[] = 
	{
		{ 0, IDM_FULLCONNECT,								TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 1, IDM_FULLDISCONNECT,							TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 0, 0,												TBSTATE_ENABLED,	TBSTYLE_SEP,	0L, 0},
		{ 2, IDM_IDBCREATESESSION_CREATESESSION,			TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 0, 0,												TBSTATE_ENABLED,	TBSTYLE_SEP,	0L, 0},
		{ 3, IDM_OPENROWSET,								TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 4, IDM_EXECUTE,									TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 0, 0,												TBSTATE_ENABLED,	TBSTYLE_SEP,	0L, 0},
		{ 5, IDM_RESTARTPOSITION,							TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 6, IDM_REFRESH,									TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 7, IDM_GETSCHEMAROWSET,							TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 0, 0,												TBSTATE_ENABLED,	TBSTYLE_SEP,	0L, 0},
		{ 8, IDM_SETDATA,									TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 9, IDM_INSERTROW,									TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{10, IDM_DELETEROWS,								TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 0, 0,												TBSTATE_ENABLED,	TBSTYLE_SEP,	0L, 0},
		{11, IDM_UPDATE,									TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{12, IDM_UNDO,										TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
		{ 0, 0,												TBSTATE_ENABLED,	TBSTYLE_SEP,	0L, 0},
	};
	m_CToolBar.Create(m_hWnd, ID_TOOLBAR, IDB_TOOLBAR, NUMELE(rgButtons), (TBBUTTON*)rgButtons);

	//Status Bar...
	m_CStatusBar.Create(m_hWnd, ID_STATUSBAR);
	m_CStatusBar.SetText(L"", STATUSBAR_TEXT, 0/*SBT_NOBORDERS*/);
	m_CStatusBar.SetText(L"", STATUSBAR_DESC, 0);

	//Controls
	m_pCListener		= new CListener();
	m_pCMDITrace		= new CMDITrace(this);
	m_pCMDIObjects		= new CMDIObjects(this);

	//See if we can load the new version RichEdit Controls
	//NOTE:  We always need to load the older one, since the Dialog Resource uses the old "RICHEDIT".
	//The newer one, we will try and use for the tracing window...
	m_hLibRichEdit20 = LoadLibrary("RICHED20.DLL");
	
	//All systems must have the RichEdit controls for RowsetViewer to work.
	//The resource, (built with the VC resource editor, hard codes to the 1.0 version = richedt32)
	//So dialogs will not appear if for some reason this dialog or its dependencies are missing...
	GETLASTERROR(m_hLibRichEdit10 = LoadLibrary("RICHED32.DLL"));
	
	//Objects
	m_pCFullConnect		=	new CFullConnect(this);
	m_pCOptionsSheet	=	new COptionsSheet;
	m_pCRootBinder		=	new CBinder(this);
	m_pCRootEnumerator	=	new CEnumerator(this);
	m_pCServiceComp		=	new CServiceComp(this);

	//Deferred Objects
	m_pCDataLinks		=	NULL;

	//Load Cursors
	m_hCurSizeNS = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));

	//Register this window for Drag and Drop capabilities...
	DragAcceptFiles(m_hWnd, TRUE/*fAccept*/);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnInitialUpdate
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnInitialUpdate()
{
	//Make sure the MDIClient area is sized correctly before displaying the 
	//MDIChild windows...
	SIZE size = GetClientSize(m_hWnd);
	POINTS pts = { (SHORT)size.cx, (SHORT)size.cy };
	OnSize(0, pts);

	//Display the Objects Window
	//NOTE: We display the objects window first so we can use the 
	//font from the tree with our output window...
	OnObjectsWindow();

	//Display the Trace Window
	OnTraceWindow();

	//Update the Controls
	UpdateControls();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnDestroy() 
{
	//Save Settings
	SaveSettings();

	//Remove all MDIChildren....
	RemoveAllChildren();

	//Now remove the MDI Objects Tree window
	MDIDestroy(m_pCMDIObjects->m_hWnd);
	//Now remove the MDI Trace window
	MDIDestroy(m_pCMDITrace->m_hWnd);

	//Unregister this window for Drag and Drop capabilities...
	DragAcceptFiles(m_hWnd, FALSE/*fAccept*/);

	//Delegate
	return CMDIFrameLite::OnDestroy();
}


////////////////////////////////////////////////////////////////
// CMainWindow::DisplayDialog
//
/////////////////////////////////////////////////////////////////
INT_PTR		CMainWindow::DisplayDialog(UINT uID, HWND hWndParent, DLGPROC lpDialogFunc, CBase* pCObject, UINT idSource)
{
	m_idSource = idSource;
	m_pCSource = pCObject;
	return ::DisplayDialog(uID, hWndParent, lpDialogFunc, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnDropFiles
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnDropFiles(HDROP hDrop)
{	
	//Obtain the number of files...
	UINT cFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0); 
		
	//For every file...
	for(ULONG iFile=0; iFile<cFiles; iFile++)
	{
		WCHAR wszFileName[_MAX_PATH]; 
		WCHAR wszExt[_MAX_EXT];
		
		if(IsUnicodeOS())
		{
			DragQueryFileW(hDrop, iFile, wszFileName, NUMELE(wszFileName)); 
		}
		else
		{
			CHAR szFileName[_MAX_PATH]={0}; 
			DragQueryFileA(hDrop, iFile, szFileName, NUMELE(szFileName)); 
			ConvertToWCHAR(szFileName, wszFileName, _MAX_PATH);
		}
		
		_wsplitpath_s(wszFileName, NULL,0, NULL, 0, NULL, 0, wszExt, NUMELE(wszExt));
		{
			//Load the UDL file
			OnLoadStringFromStorage(wszFileName);
		}
	}

	return TRUE;
}

		
/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	//Determine the source object for this menu
	SOURCE eSource = (SOURCE)GetMenuData(hMenu, 0, TRUE/*fByPosition*/); 

	//Is there an active object, make sure its the same type as the selected object
	CBase*	pCObject	= m_pCMDIObjects->m_pCObjTree->GetSelectedObject();
	if(pCObject && pCObject->GetObjectType() != eSource)
		pCObject = NULL;

	//Reset Resent Configurations
	if(GetMenuItemID(hMenu, 0)==IDM_RECENTCONFIG1)
	{
		WCHAR wszBuffer[MAX_NAME_LEN+10];
		CHAR  szBuffer[MAX_NAME_LEN+10];
		CList<WCHAR*,WCHAR*>* pCList = &m_pCFullConnect->m_listConfigs;
		
		ASSERT(pCList);
		LONG cRecentConfigs = pCList->GetCount();

		//Remove all menu items
		if(cRecentConfigs)
		{
			INT iMenuItems = GetMenuItemCount(hMenu);
			for(INT i=0; i<iMenuItems; i++)
				DeleteMenu(hMenu, 0, MF_BYPOSITION);
		}

		//Display all Recent Configs
		for(LONG i=0; i<cRecentConfigs; i++)
		{
			StringFormat(wszBuffer, NUMELE(wszBuffer), L"&%d - %s", i+1, pCList->GetAt(pCList->FindIndex(i)));
			ConvertToMBCS(wszBuffer, szBuffer, NUMELE(szBuffer));
			AppendMenu(hMenu, MF_STRING, IDM_RECENTCONFIG1 + i, szBuffer);
		}
	}
			
	//Reset Resent Files
	if(GetMenuItemID(hMenu, 0)==IDM_RECENTFILE1)
	{
		CHAR szBuffer[MAX_NAME_LEN+10];
		WCHAR wszBuffer[MAX_NAME_LEN+10];
		CList<WCHAR*,WCHAR*>* pCList = &m_pCFullConnect->m_listFiles;
		
		ASSERT(pCList);
		LONG cRecentFiles = pCList->GetCount();

		//Remove all menu items
		if(cRecentFiles)
		{
			INT iMenuItems = GetMenuItemCount(hMenu);
			for(INT i=0; i<iMenuItems; i++)
				DeleteMenu(hMenu, 0, MF_BYPOSITION);
		}

		//Display all Recent Files
		for(LONG i=0; i<cRecentFiles; i++)
		{
			StringFormat(wszBuffer, NUMELE(wszBuffer), L"&%d - %s", i+1, pCList->GetAt(pCList->FindIndex(i)));
			ConvertToMBCS(wszBuffer, szBuffer, NUMELE(szBuffer));
			AppendMenu(hMenu, MF_STRING, IDM_RECENTFILE1 + i, szBuffer);
		}
	}

	switch(nID)
	{
		///////////////////////////////////////////////////////////////////
		// File Menu
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_FULLCONNECT,									TRUE) 
		ON_COMMAND_UI_ENABLED(IDM_FULLDISCONNECT,								GetActiveWindow(L"MDICHILD") ? TRUE : FALSE)
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_CREATE_ROOTBINDER,							TRUE) 
		ON_COMMAND_UI_ENABLED(IDM_CREATE_ROOTENUMERATOR,						TRUE) 
		ON_COMMAND_UI_ENABLED(IDM_CREATE_SERVICECOMPONENTS,						TRUE) 
		ON_COMMAND_UI_ENABLED(IDM_CREATE_DATALINKS,								TRUE) 
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG1,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG2,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG3,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG4,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG5,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG6,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG7,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG8,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG9,								m_pCFullConnect->m_listConfigs.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTCONFIG10,								m_pCFullConnect->m_listConfigs.GetCount())
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE1,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE2,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE3,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE4,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE5,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE6,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE7,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE8,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE9,									m_pCFullConnect->m_listFiles.GetCount())
		ON_COMMAND_UI_ENABLED(IDM_RECENTFILE10,									m_pCFullConnect->m_listFiles.GetCount())
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_EXIT,											TRUE)


		///////////////////////////////////////////////////////////////////
		// Unknown
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_UNKNOWN_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		ON_COMMAND_UI_ENABLED(IDM_IUNKNOWN_RELEASE,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		ON_COMMAND_UI_ENABLED(IDM_IUNKNOWN_QUERYINTERFACE,						SOURCE_GETINTERFACE(pCObject, IUnknown))
		ON_COMMAND_UI_ENABLED(IDM_IUNKNOWN_AUTOQI,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		ON_COMMAND_UI_ENABLED(IDM_IUNKNOWN_RELEASEALL,							SOURCE_GETINTERFACE(pCObject, IUnknown))
		ON_COMMAND_UI_ENABLED(IDM_IUNKNOWN_RELEASECHILDREN,						SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ISUPPORTERRORINFO,							SOURCE_GETINTERFACE(pCObject, ISupportErrorInfo))
		

		
		///////////////////////////////////////////////////////////////////
		// DataSource
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_DATASOURCE_ADDREF,							SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IDBINITIALIZE_INITIALIZE,						SOURCE_GETINTERFACE(pCObject, IDBInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDBINITIALIZE_UNINITIALIZE,					SOURCE_GETINTERFACE(pCObject, IDBInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDBCREATESESSION_CREATESESSION,				SOURCE_GETINTERFACE(pCObject, IDBCreateSession))
		ON_COMMAND_UI_ENABLED(IDM_IDBPROPERTIES_GETPROPERTYINFO,				SOURCE_GETINTERFACE(pCObject, IDBProperties))
		ON_COMMAND_UI_ENABLED(IDM_IDBPROPERTIES_GETPROPERTIES,					SOURCE_GETINTERFACE(pCObject, IDBProperties))
		ON_COMMAND_UI_ENABLED(IDM_IDBPROPERTIES_SETPROPERTIES,					SOURCE_GETINTERFACE(pCObject, IDBProperties))
		ON_COMMAND_UI_ENABLED(IDM_IPERSIST_GETCLASSID,							SOURCE_GETINTERFACE(pCObject, IPersist))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ICPC_FINDCONNECTIONPOINT,						SOURCE_GETINTERFACE(pCObject, IConnectionPointContainer))
		ON_COMMAND_UI_ENABLED(IDM_ICPC_ENUMCONNECTIONPOINTS,					FALSE)	
		ON_COMMAND_UI_ENABLED(IDM_IDBASYNCHSTATUS_ABORT,						SOURCE_GETINTERFACE(pCObject, IDBAsynchStatus))
		ON_COMMAND_UI_ENABLED(IDM_IDBASYNCHSTATUS_GETSTATUS,					SOURCE_GETINTERFACE(pCObject, IDBAsynchStatus))
		ON_COMMAND_UI_ENABLED(IDM_IDATASOURCEADMIN_CREATEDATASOURCE,			SOURCE_GETINTERFACE(pCObject, IDBDataSourceAdmin))
		ON_COMMAND_UI_ENABLED(IDM_IDATASOURCEADMIN_DESTROYDATASOURCE,			SOURCE_GETINTERFACE(pCObject, IDBDataSourceAdmin))
		ON_COMMAND_UI_ENABLED(IDM_IDATASOURCEADMIN_GETCREATIONPROPERTIES,		SOURCE_GETINTERFACE(pCObject, IDBDataSourceAdmin))
		ON_COMMAND_UI_ENABLED(IDM_IDATASOURCEADMIN_MODIFYDATASOURCE,			SOURCE_GETINTERFACE(pCObject, IDBDataSourceAdmin))
		ON_COMMAND_UI_ENABLED(IDM_IDBINFO_GETKEYWORDS,							SOURCE_GETINTERFACE(pCObject, IDBInfo))
		ON_COMMAND_UI_ENABLED(IDM_IDBINFO_GETLITERALINFO,						SOURCE_GETINTERFACE(pCObject, IDBInfo))
		ON_COMMAND_UI_ENABLED(IDM_IPERSISTFILE_LOAD,							SOURCE_GETINTERFACE(pCObject, IPersistFile))
		ON_COMMAND_UI_ENABLED(IDM_IPERSISTFILE_SAVE,							SOURCE_GETINTERFACE(pCObject, IPersistFile))
		ON_COMMAND_UI_ENABLED(IDM_IPERSISTFILE_SAVECOMPLETED,					SOURCE_GETINTERFACE(pCObject, IPersistFile))
		ON_COMMAND_UI_ENABLED(IDM_IPERSISTFILE_GETCURFILE,						SOURCE_GETINTERFACE(pCObject, IPersistFile))
		ON_COMMAND_UI_ENABLED(IDM_IPERSISTFILE_ISDIRTY,							SOURCE_GETINTERFACE(pCObject, IPersistFile))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IGETPOOL_GETPOOL,								FALSE)
		ON_COMMAND_UI_ENABLED(IDM_IGETPOOL_GETPOOLSTATE,						FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ISERVICEPROVIDER,								SOURCE_GETINTERFACE(pCObject, IServiceProvider))
		//----------------------------
		
		
		
		///////////////////////////////////////////////////////////////////
		// Session
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_SESSION_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_GETDATASOURCE,								SOURCE_GETINTERFACE(pCObject, IGetDataSource))
		ON_COMMAND_UI_ENABLED(IDM_OPENROWSET,									SOURCE_GETINTERFACE(pCObject, IOpenRowset))
		ON_COMMAND_UI_ENABLED(IDM_ISESSIONPROPERTIES_GETPROPERTIES,				SOURCE_GETINTERFACE(pCObject, ISessionProperties))
		ON_COMMAND_UI_ENABLED(IDM_ISESSIONPROPERTIES_SETPROPERTIES,				SOURCE_GETINTERFACE(pCObject, ISessionProperties))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IDBCREATECOMMAND_CREATECOMMAND,				SOURCE_GETINTERFACE(pCObject, IDBCreateCommand))
		ON_COMMAND_UI_ENABLED(IDM_GETSCHEMAS,									SOURCE_GETINTERFACE(pCObject, IDBSchemaRowset))
		ON_COMMAND_UI_ENABLED(IDM_GETSCHEMAROWSET,								SOURCE_GETINTERFACE(pCObject, IDBSchemaRowset))
		ON_COMMAND_UI_ENABLED(IDM_INDEX_CREATEINDEX,							SOURCE_GETINTERFACE(pCObject, IIndexDefinition))
		ON_COMMAND_UI_ENABLED(IDM_INDEX_DROPINDEX,								SOURCE_GETINTERFACE(pCObject, IIndexDefinition))
		ON_COMMAND_UI_ENABLED(IDM_INDEX_ALTERINDEX,								SOURCE_GETINTERFACE(pCObject, IAlterIndex))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_CREATETABLE,							SOURCE_GETINTERFACE(pCObject, ITableDefinition))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_ADDCOLUMN,								SOURCE_GETINTERFACE(pCObject, ITableDefinition))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_DROPCOLUMN,								SOURCE_GETINTERFACE(pCObject, ITableDefinition))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_DROPTABLE,								SOURCE_GETINTERFACE(pCObject, ITableDefinition))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_CREATETABLEWITHCONSTRAINTS,				NULL); //SOURCE_GETINTERFACE(pCObject, ITableDefinitionWithConstraints))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_ADDCONSTRAINT,							SOURCE_GETINTERFACE(pCObject, ITableDefinitionWithConstraints))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_DROPCONSTRAINT,							SOURCE_GETINTERFACE(pCObject, ITableDefinitionWithConstraints))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_ALTERTABLE,								SOURCE_GETINTERFACE(pCObject, IAlterTable))
		ON_COMMAND_UI_ENABLED(IDM_TABLE_ALTERCOLUMN,							SOURCE_GETINTERFACE(pCObject, IAlterTable))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONLOCAL_GETOPTIONSOBJECT,			SOURCE_GETINTERFACE(pCObject, ITransactionLocal))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONLOCAL_STARTTRANSACTION,			SOURCE_GETINTERFACE(pCObject, ITransactionLocal))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONOBJECT_GETTRANSACTIONOBJECT,		SOURCE_GETINTERFACE(pCObject, ITransactionObject))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONJOIN_GETOPTIONSOBJECT,			SOURCE_GETINTERFACE(pCObject, ITransactionJoin))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONJOIN_JOINTRANSACTION,				SOURCE_GETINTERFACE(pCObject, ITransactionJoin))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONJOIN_RELEASETRANSACTION,			SOURCE_GETINTERFACE(pCObject, ITransactionJoin))
#ifdef MTSTXN       
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONDISPENSOR_BEGINTRANSACTION,		SOURCE_GETINTERFACE(pCObject, ITransactionJoin))
#else  //MTSTXN
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONDISPENSOR_BEGINTRANSACTION,		FALSE)
#endif //MTSTXN
		//----------------------------
		//----------------------------


		////////////////////////////////////////////////////////////////
		// Command
		//
		////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_COMMAND_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IACCESSOR_GETBINDINGS,						SOURCE_GETINTERFACE(pCObject, IAccessor))
		ON_COMMAND_UI_ENABLED(IDM_IACCESSOR_ADDREFACCESSOR,						SOURCE_GETINTERFACE(pCObject, IAccessor))
		ON_COMMAND_UI_ENABLED(IDM_IACCESSOR_RELEASEACCESSOR,					SOURCE_GETINTERFACE(pCObject, IAccessor))
		ON_COMMAND_UI_ENABLED(IDM_IACCESSOR_CREATEACCESSOR,						SOURCE_GETINTERFACE(pCObject, IAccessor))
		ON_COMMAND_UI_ENABLED(IDM_ICOLUMNSINFO_GETCOLUMNINFO,					SOURCE_GETINTERFACE(pCObject, IColumnsInfo))
		ON_COMMAND_UI_ENABLED(IDM_ICOLUMNSINFO_MAPCOLUMNIDS,					FALSE)
		ON_COMMAND_UI_ENABLED(IDM_EXECUTE,										SOURCE_GETINTERFACE(pCObject, ICommand))
		ON_COMMAND_UI_ENABLED(IDM_COMMAND_CANCEL,								SOURCE_GETINTERFACE(pCObject, ICommand))
		ON_COMMAND_UI_ENABLED(IDM_COMMAND_GETDBSESSION,							SOURCE_GETINTERFACE(pCObject, ICommand))
		ON_COMMAND_UI_ENABLED(IDM_COMMAND_GETPROPERTIES,						SOURCE_GETINTERFACE(pCObject, ICommandProperties))
		ON_COMMAND_UI_ENABLED(IDM_COMMAND_SETPROPERTIES,						SOURCE_GETINTERFACE(pCObject, ICommandProperties))
		ON_COMMAND_UI_ENABLED(IDM_SETCOMMANDTEXT,								SOURCE_GETINTERFACE(pCObject, ICommandText))
		ON_COMMAND_UI_ENABLED(IDM_GETCOMMANDTEXT,								SOURCE_GETINTERFACE(pCObject, ICommandText))
		ON_COMMAND_UI_ENABLED(IDM_SETCOMMANDSTREAM,								SOURCE_GETINTERFACE(pCObject, ICommandStream))
		ON_COMMAND_UI_ENABLED(IDM_GETCOMMANDSTREAM,								SOURCE_GETINTERFACE(pCObject, ICommandStream))
		ON_COMMAND_UI_ENABLED(IDM_COMMANDTEXT_EXECUTE,							SOURCE_GETINTERFACE(pCObject, ICommandText))
		ON_COMMAND_UI_ENABLED(IDM_ICONVERTTYPE_CANCONVERT,						SOURCE_GETINTERFACE(pCObject, IConvertType))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ICOLUMNSROWSET_GETCOLUMNSROWSET,				SOURCE_GETINTERFACE(pCObject, IColumnsRowset))
		ON_COMMAND_UI_ENABLED(IDM_ICOLUMNSROWSET_GETAVAILABLECOLUMNS,			FALSE)
		ON_COMMAND_UI_ENABLED(IDM_COMMANDPREPARE,								SOURCE_GETINTERFACE(pCObject, ICommandPrepare))
		ON_COMMAND_UI_ENABLED(IDM_COMMANDUNPREPARE,								SOURCE_GETINTERFACE(pCObject, ICommandPrepare))
		ON_COMMAND_UI_ENABLED(IDM_COMMANDPERSIST_DELETECOMMAND,					SOURCE_GETINTERFACE(pCObject, ICommandPersist))
		ON_COMMAND_UI_ENABLED(IDM_COMMANDPERSIST_GETCURRENTCOMMAND,				SOURCE_GETINTERFACE(pCObject, ICommandPersist))
		ON_COMMAND_UI_ENABLED(IDM_COMMANDPERSIST_LOADCOMMAND,					SOURCE_GETINTERFACE(pCObject, ICommandPersist))
		ON_COMMAND_UI_ENABLED(IDM_COMMANDPERSIST_SAVECOMMAND,					SOURCE_GETINTERFACE(pCObject, ICommandPersist))
		ON_COMMAND_UI_ENABLED(IDM_GETPARAMETERINFO,								SOURCE_GETINTERFACE(pCObject, ICommandWithParameters))
		ON_COMMAND_UI_ENABLED(IDM_SETPARAMETERINFO,								SOURCE_GETINTERFACE(pCObject, ICommandWithParameters))
		ON_COMMAND_UI_ENABLED(IDM_MAPPARAMETERNAMES,							FALSE)
		//----------------------------
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_COMMAND_CLEARCOMMAND,							SOURCE_GETINTERFACE(pCObject, ICommand))
			 
		 
		///////////////////////////////////////////////////////////////////
		// Rowset
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_ROWSET_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_GETDATA,										SOURCE_GETINTERFACE(pCObject, IRowset))
		ON_COMMAND_UI_ENABLED(IDM_GETNEXTROWS,									SOURCE_GETINTERFACE(pCObject, IRowset))
		ON_COMMAND_UI_ENABLED(IDM_RELEASEROWS,									SOURCE_GETINTERFACE(pCObject, IRowset))
		ON_COMMAND_UI_ENABLED(IDM_ADDREFROWS,									SOURCE_GETINTERFACE(pCObject, IRowset))
		ON_COMMAND_UI_ENABLED(IDM_REFRESH,										SOURCE_GETINTERFACE(pCObject, IRowset))
		ON_COMMAND_UI_ENABLED(IDM_RESTARTPOSITION,								SOURCE_GETINTERFACE(pCObject, IRowset))
		ON_COMMAND_UI_ENABLED(IDM_RELEASEALLROWS,								SOURCE_GETINTERFACE(pCObject, IRowset))
			 
		ON_COMMAND_UI_ENABLED(IDM_IROWSET_GETPROPERTIES,						SOURCE_GETINTERFACE(pCObject, IRowsetInfo))
		ON_COMMAND_UI_ENABLED(IDM_GETSPECIFICATION,								SOURCE_GETINTERFACE(pCObject, IRowsetInfo))
		ON_COMMAND_UI_ENABLED(IDM_GETREFERENCEDROWSET,							SOURCE_GETINTERFACE(pCObject, IRowsetInfo))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_INSERTROW,									SOURCE_GETINTERFACE(pCObject, IRowsetChange))			 
		ON_COMMAND_UI_ENABLED(IDM_DELETEROWS,									SOURCE_GETINTERFACE(pCObject, IRowsetChange))			 
		ON_COMMAND_UI_ENABLED(IDM_SETDATA,										SOURCE_GETINTERFACE(pCObject, IRowsetChange))			 
		ON_COMMAND_UI_ENABLED(IDM_IROWSETFIND_FINDNEXTROW,						SOURCE_GETINTERFACE(pCObject, IRowsetFind))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETINDENTITY_ISSAMEROW,					SOURCE_GETINTERFACE(pCObject, IRowsetIdentity))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETLOCATE_GETDATA,						SOURCE_GETINTERFACE(pCObject, IRowsetLocate))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETLOCATE_COMPARE,						SOURCE_GETINTERFACE(pCObject, IRowsetLocate))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETLOCATE_GETROWSAT,						SOURCE_GETINTERFACE(pCObject, IRowsetLocate))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETLOCATE_GETROWSBYBOOKMARK,				SOURCE_GETINTERFACE(pCObject, IRowsetLocate))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETLOCATE_HASH,							SOURCE_GETINTERFACE(pCObject, IRowsetLocate))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETBOOKMARK_POSITIONONBOOKMARK,			SOURCE_GETINTERFACE(pCObject, IRowsetBookmark))
		ON_COMMAND_UI_ENABLED(IDM_GETVISIBLEDATA,								SOURCE_GETINTERFACE(pCObject, IRowsetResynch))
		ON_COMMAND_UI_ENABLED(IDM_RESYNCHROWS,									SOURCE_GETINTERFACE(pCObject, IRowsetResynch))
		ON_COMMAND_UI_ENABLED(IDM_GETLASTVISIBLEDATA,							SOURCE_GETINTERFACE(pCObject, IRowsetRefresh))
		ON_COMMAND_UI_ENABLED(IDM_REFRESHVISIBLEDATA,							SOURCE_GETINTERFACE(pCObject, IRowsetRefresh))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETSCROLL_GETDATA,						SOURCE_GETINTERFACE(pCObject, IRowsetScroll))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETSCROLL_GETAPPROXIMATEPOSITION,			SOURCE_GETINTERFACE(pCObject, IRowsetScroll))
		ON_COMMAND_UI_ENABLED(IDM_IROWSETSCROLL_GETROWSATRATIO,					FALSE)
		ON_COMMAND_UI_ENABLED(IDM_UNDO,											SOURCE_GETINTERFACE(pCObject, IRowsetUpdate))
		ON_COMMAND_UI_ENABLED(IDM_UPDATE,										SOURCE_GETINTERFACE(pCObject, IRowsetUpdate))
		ON_COMMAND_UI_ENABLED(IDM_GETPENDINGROWS,								SOURCE_GETINTERFACE(pCObject, IRowsetUpdate))
		ON_COMMAND_UI_ENABLED(IDM_GETROWSTATUS,									SOURCE_GETINTERFACE(pCObject, IRowsetUpdate))
		ON_COMMAND_UI_ENABLED(IDM_GETORIGINALDATA,								SOURCE_GETINTERFACE(pCObject, IRowsetUpdate))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_GETROWFROMHROW,								SOURCE_GETINTERFACE(pCObject, IGetRow))
		ON_COMMAND_UI_ENABLED(IDM_GETURLFROMHROW,								SOURCE_GETINTERFACE(pCObject, IGetRow))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_CREATE_ROWPOSITION,							TRUE)
		ON_COMMAND_UI_ENABLED(IDM_IROWSET_CURSORENGINE,							FALSE)
		ON_COMMAND_UI_ENABLED(IDM_MSPERSIST_ROWSET_SAVE,						FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_MSPERSIST_ROWSET_LOAD,						FALSE) 
			 
			 
		
		///////////////////////////////////////////////////////////////////
		// Row
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_ROW_ADDREF,									SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IROW_GETCOLUMNS,								SOURCE_GETINTERFACE(pCObject, IRow))
		ON_COMMAND_UI_ENABLED(IDM_IROW_GETSOURCEROWSET,							SOURCE_GETINTERFACE(pCObject, IRow))
		ON_COMMAND_UI_ENABLED(IDM_IROW_OPEN,									SOURCE_GETINTERFACE(pCObject, IRow))
		ON_COMMAND_UI_ENABLED(IDM_IGETSESSION_GETSESSION,						SOURCE_GETINTERFACE(pCObject, IGetSession))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ICOLUMNSINFO2_GETRESTRICTEDCOLUMNINFO,		FALSE)
		ON_COMMAND_UI_ENABLED(IDM_IROW_SETCOLUMNS,								SOURCE_GETINTERFACE(pCObject, IRowChange))
		ON_COMMAND_UI_ENABLED(IDM_IROW_ADDCOLUMNS,								FALSE)
		ON_COMMAND_UI_ENABLED(IDM_IROW_DELETECOLUMNS,							SOURCE_GETINTERFACE(pCObject, IRowSchemaChange))
		ON_COMMAND_UI_ENABLED(IDM_ISCOPEDOPERATIONS_OPENROWSET,					SOURCE_GETINTERFACE(pCObject, IScopedOperations))
		ON_COMMAND_UI_ENABLED(IDM_ISCOPEDOPERATIONS_DELETE,						SOURCE_GETINTERFACE(pCObject, IScopedOperations))
		ON_COMMAND_UI_ENABLED(IDM_ISCOPEDOPERATIONS_COPY,						SOURCE_GETINTERFACE(pCObject, IScopedOperations))
		ON_COMMAND_UI_ENABLED(IDM_ISCOPEDOPERATIONS_MOVE,						SOURCE_GETINTERFACE(pCObject, IScopedOperations))
		//----------------------------
		//----------------------------

		
		///////////////////////////////////////////////////////////////////
		// Error Object
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_ERROR_ADDREF,									SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ERRORINFO_GETDESCRIPTION,						SOURCE_GETINTERFACE(pCObject, IErrorInfo))
		ON_COMMAND_UI_ENABLED(IDM_ERRORINFO_GETGUID,							SOURCE_GETINTERFACE(pCObject, IErrorInfo))
		ON_COMMAND_UI_ENABLED(IDM_ERRORINFO_GETHELPCONTEXT,						SOURCE_GETINTERFACE(pCObject, IErrorInfo))
		ON_COMMAND_UI_ENABLED(IDM_ERRORINFO_GETHELPFILE,						SOURCE_GETINTERFACE(pCObject, IErrorInfo))
		ON_COMMAND_UI_ENABLED(IDM_ERRORINFO_GETSOURCE,							SOURCE_GETINTERFACE(pCObject, IErrorInfo))
		ON_COMMAND_UI_ENABLED(IDM_ERRORRECORDS_GETRECORDCOUNT,					SOURCE_GETINTERFACE(pCObject, IErrorRecords))
		ON_COMMAND_UI_ENABLED(IDM_ERRORRECORDS_GETCUSTOMERROROBJECT,			SOURCE_GETINTERFACE(pCObject, IErrorRecords))
		ON_COMMAND_UI_ENABLED(IDM_ERRORRECORDS_GETERRORINFO,					SOURCE_GETINTERFACE(pCObject, IErrorRecords))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_GETERRORINFO,									TRUE)
		ON_COMMAND_UI_ENABLED(IDM_SETERRORINFO,									TRUE)
		ON_COMMAND_UI_ENABLED(IDM_DISPLAYERRORINFO,								TRUE)
		//----------------------------


		///////////////////////////////////////////////////////////////////
		// Custom Error Object
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_CUSTOMERROR_ADDREF,							SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_SQLERRORINFO_GETSQLINFO,						SOURCE_GETINTERFACE(pCObject, ISQLErrorInfo))
		//----------------------------

		
		///////////////////////////////////////////////////////////////////
		// Binder
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_BINDER_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IBINDRESOURCE_BIND,							SOURCE_GETINTERFACE(pCObject, IBindResource))
		ON_COMMAND_UI_ENABLED(IDM_ICREATEROW_CREATEROW,							SOURCE_GETINTERFACE(pCObject, ICreateRow))
		ON_COMMAND_UI_ENABLED(IDM_IDBBINDERPROPERTIES_RESET,					SOURCE_GETINTERFACE(pCObject, IDBBinderProperties))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IREGISTERPROVIDER_SETURLMAPPING,				SOURCE_GETINTERFACE(pCObject, IRegisterProvider))
		ON_COMMAND_UI_ENABLED(IDM_IREGISTERPROVIDER_GETURLMAPPING,				SOURCE_GETINTERFACE(pCObject, IRegisterProvider))
		ON_COMMAND_UI_ENABLED(IDM_IREGISTERPROVIDER_UNREGISTERPROVIDER,			SOURCE_GETINTERFACE(pCObject, IRegisterProvider))
		//----------------------------

		
		///////////////////////////////////////////////////////////////////
		// ConnectionPoint
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_CONNECTIONPOINT_ADDREF,						SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ICONNECTIONPOINT_GETCONNECTIONINTERFACE,		SOURCE_GETINTERFACE(pCObject, IConnectionPoint))
		ON_COMMAND_UI_ENABLED(IDM_ICONNECTIONPOINT_ADVISE,						SOURCE_GETINTERFACE(pCObject, IConnectionPoint))
		ON_COMMAND_UI_ENABLED(IDM_ICONNECTIONPOINT_UNADVISE,					SOURCE_GETINTERFACE(pCObject, IConnectionPoint))
		ON_COMMAND_UI_ENABLED(IDM_ICONNECTIONPOINT_ENUMCONNECTIONS,				FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ICONNECTIONPOINT_GETCONNECTIONPOINTCONTAINER,	FALSE)
		//----------------------------
		//----------------------------


		///////////////////////////////////////////////////////////////////
		// Service Components
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_SERVICECOMP_ADDREF,									SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IDATAINITIALIZE_CREATEDBINSTANCE,						SOURCE_GETINTERFACE(pCObject, IDataInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDATAINITIALIZE_CREATEDBINSTANCEEX,					SOURCE_GETINTERFACE(pCObject, IDataInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDATAINITIALIZE_LOADSTRINGFROMSTORAGE,				SOURCE_GETINTERFACE(pCObject, IDataInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDATAINITIALIZE_GETDATASOURCE,						SOURCE_GETINTERFACE(pCObject, IDataInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDATAINITIALIZE_GETINITIALIZATIONSTRING,				SOURCE_GETINTERFACE(pCObject, IDataInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDATAINITIALIZE_GETINITIALIZATIONSTRING_NOPASSWORD,	SOURCE_GETINTERFACE(pCObject, IDataInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDATAINITIALIZE_WRITESTRINGTOSTORAGE,					SOURCE_GETINTERFACE(pCObject, IDataInitialize))
		ON_COMMAND_UI_ENABLED(IDM_CREATEFILEMONIKER,									TRUE) 
		//----------------------------											
		ON_COMMAND_UI_ENABLED(IDM_POOLMNGR_ENUMPOOLS,									FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_POOLMNGR_REFRESH,										FALSE) 
		//----------------------------



		///////////////////////////////////////////////////////////////////
		// Data Links
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_DATALINKS_ADDREF,										SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IDBPROMPTINITIALIZE_PROMPTDATASOURCE,					SOURCE_GETINTERFACE(pCObject, IDBPromptInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDBPROMPTINITIALIZE_PROMPTDATASOURCE_ADVANCED,		SOURCE_GETINTERFACE(pCObject, IDBPromptInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDBPROMPTINITIALIZE_PROMPTFILENAME,					SOURCE_GETINTERFACE(pCObject, IDBPromptInitialize))
		ON_COMMAND_UI_ENABLED(IDM_IDBPROMPTINITIALIZE_PROMPTFILENAME_ADVANCED,			SOURCE_GETINTERFACE(pCObject, IDBPromptInitialize))
		//----------------------------											
		//----------------------------

		
		////////////////////////////////////////////////////////////////
		// Dataset
		//
		////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_DATASET_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_DATASET_GETAXISINFO,							SOURCE_GETINTERFACE(pCObject, IMDDataset))
  		ON_COMMAND_UI_ENABLED(IDM_DATASET_GETAXISROWSET,						SOURCE_GETINTERFACE(pCObject, IMDDataset))
  		ON_COMMAND_UI_ENABLED(IDM_DATASET_GETCELLDATA,							SOURCE_GETINTERFACE(pCObject, IMDDataset))
  		ON_COMMAND_UI_ENABLED(IDM_DATASET_FREEAXISINFO,							SOURCE_GETINTERFACE(pCObject, IMDDataset))
  		ON_COMMAND_UI_ENABLED(IDM_DATASET_GETSPECIFICATION,						SOURCE_GETINTERFACE(pCObject, IMDDataset))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IMDFIND_FINDCELL,								FALSE && SOURCE_GETINTERFACE(pCObject, IMDFind))
  		ON_COMMAND_UI_ENABLED(IDM_IMDFIND_FINDTUPEL,							FALSE && SOURCE_GETINTERFACE(pCObject, IMDDataset))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IMDRANGEROWSET_GETRANGEROWSET,				FALSE && SOURCE_GETINTERFACE(pCObject, IMDRangeRowset))
		//----------------------------
		//----------------------------

		
		////////////////////////////////////////////////////////////////
		// Enumerator
		//
		////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_ENUMERATOR_ADDREF,							SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ISOURCESROWSET_GETSOURCESROWSET,				SOURCE_GETINTERFACE(pCObject, ISourcesRowset))
		//----------------------------
		//----------------------------

		
		////////////////////////////////////////////////////////////////
		// MultipleResults
		//
		////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_MULTIPLERESULTS_ADDREF,						SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IMULTIPLERESULTS_GETRESULT,					SOURCE_GETINTERFACE(pCObject, IMultipleResults))
		//----------------------------
		//----------------------------

		
		////////////////////////////////////////////////////////////////
		// Stream
		//
		////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_STREAM_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ISEQSTREAM_READ,								SOURCE_GETINTERFACE(pCObject, ISequentialStream))
		ON_COMMAND_UI_ENABLED(IDM_ISEQSTREAM_WRITE,								SOURCE_GETINTERFACE(pCObject, ISequentialStream))
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_READ,									SOURCE_GETINTERFACE(pCObject, IStream))
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_WRITE,								SOURCE_GETINTERFACE(pCObject, IStream))
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_SEEK,									SOURCE_GETINTERFACE(pCObject, IStream))
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_CLONE,								SOURCE_GETINTERFACE(pCObject, IStream))
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_SETSIZE,								SOURCE_GETINTERFACE(pCObject, IStream) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_COPYTO,								SOURCE_GETINTERFACE(pCObject, IStream) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_COMMIT,								SOURCE_GETINTERFACE(pCObject, IStream) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_REVERT,								SOURCE_GETINTERFACE(pCObject, IStream) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_LOCKREGION,							SOURCE_GETINTERFACE(pCObject, IStream) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_UNLOCKREGION,							SOURCE_GETINTERFACE(pCObject, IStream) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ISTREAM_STAT,									SOURCE_GETINTERFACE(pCObject, IStream) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ILOCKBYTES_READAT,							SOURCE_GETINTERFACE(pCObject, ILockBytes))
		ON_COMMAND_UI_ENABLED(IDM_ILOCKBYTES_WRITE,								SOURCE_GETINTERFACE(pCObject, ILockBytes))
		ON_COMMAND_UI_ENABLED(IDM_ILOCKBYTES_STAT,								SOURCE_GETINTERFACE(pCObject, ILockBytes) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ILOCKBYTES_FLUSH,								SOURCE_GETINTERFACE(pCObject, ILockBytes) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ILOCKBYTES_SETSIZE,							SOURCE_GETINTERFACE(pCObject, ILockBytes) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ILOCKBYTES_LOCKREGION,						SOURCE_GETINTERFACE(pCObject, ILockBytes) && FALSE)
		ON_COMMAND_UI_ENABLED(IDM_ILOCKBYTES_UNLOCKREGION,						SOURCE_GETINTERFACE(pCObject, ILockBytes) && FALSE)
		//----------------------------
		//----------------------------
		

		////////////////////////////////////////////////////////////////
		// PoolInfo
		//
		////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_POOLINFO_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_REFRESH,							FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_GETCOUNT,							FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_GETPOOLID,							FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_GETPROTOTYPE,						FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_SETPOOLTIMEOUT,						FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_GETPOOLTIMEOUT,						FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_SETRETRYTIMEOUT,					FALSE) 
		ON_COMMAND_UI_ENABLED(IDM_IPOOLINFO_GETRETRYTIMEOUT,					FALSE) 
		//----------------------------
		//----------------------------

		
		////////////////////////////////////////////////////////////////
		// RowPosition
		//
		////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_ROWPOS_ADDREF,								SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_IROWPOS_INITIALIZE,							SOURCE_GETINTERFACE(pCObject, IRowPosition))
		ON_COMMAND_UI_ENABLED(IDM_IROWPOS_GETROWSET,							SOURCE_GETINTERFACE(pCObject, IRowPosition))
		ON_COMMAND_UI_ENABLED(IDM_IROWPOS_CLEARROWPOSITION,						SOURCE_GETINTERFACE(pCObject, IRowPosition))
		ON_COMMAND_UI_ENABLED(IDM_IROWPOS_SETROWPOSITION,						SOURCE_GETINTERFACE(pCObject, IRowPosition))
		ON_COMMAND_UI_ENABLED(IDM_IROWPOS_GETROWPOSITION,						SOURCE_GETINTERFACE(pCObject, IRowPosition))
		//----------------------------
		//----------------------------

		
		///////////////////////////////////////////////////////////////////
		// Transaction
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_TRANSACTION_ADDREF,							SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTION_ABORT,							SOURCE_GETINTERFACE(pCObject, ITransaction))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTION_COMMIT,							SOURCE_GETINTERFACE(pCObject, ITransaction))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTION_GETTRANSACTIONINFO,				SOURCE_GETINTERFACE(pCObject, ITransaction))
		//----------------------------
		//----------------------------


		///////////////////////////////////////////////////////////////////
		// TransactionOptions
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_TRANSACTIONOPTIONS_ADDREF,					SOURCE_GETINTERFACE(pCObject, IUnknown))
		//----------------------------
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONOPTIONS_GETOPTIONS,				SOURCE_GETINTERFACE(pCObject, ITransactionOptions))
		ON_COMMAND_UI_ENABLED(IDM_ITRANSACTIONOPTIONS_SETOPTIONS,				SOURCE_GETINTERFACE(pCObject, ITransactionOptions))
		//----------------------------
		//----------------------------

		

		///////////////////////////////////////////////////////////////////
		// Tools Menu
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_OPTIONS,										TRUE)
		ON_COMMAND_UI_ENABLED(IDM_TRACEWINDOW,									TRUE)
		ON_COMMAND_UI_ENABLED(IDM_OBJECTSWINDOW,								TRUE)

		///////////////////////////////////////////////////////////////////
		// Window Menu
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND_UI_ENABLED(IDM_CLOSEWINDOW,									GetActiveWindow())
									
		ON_COMMAND_UI_ENABLED(IDM_TILEHORIZONTAL,								GetActiveWindow())
		ON_COMMAND_UI_ENABLED(IDM_TILEVERTICAL,									GetActiveWindow())
		ON_COMMAND_UI_ENABLED(IDM_CASCADE,										GetActiveWindow())
		ON_COMMAND_UI_ENABLED(IDM_ARRANGEICONS,									GetActiveWindow())
		ON_COMMAND_UI_ENABLED(IDM_AUTOPOSITION,									GetActiveWindow())
		ON_COMMAND_UI_ENABLED(IDM_CLOSEALL,										GetActiveWindow())
		ON_COMMAND_UI_ENABLED(IDM_NEXTWINDOW,									GetActiveWindow())
		ON_COMMAND_UI_ENABLED(IDM_PREVWINDOW,									GetActiveWindow())

		//Help Menu
		ON_COMMAND_UI_ENABLED(IDM_ABOUT,										TRUE)
	};

	return CMDIFrameLite::OnUpdateCommand(hMenu, nID, pdwFlags);
}

	
/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnCommand(UINT iID, HWND hWndCtrl) 
{
	HRESULT	hr = S_OK;
	DWORD dwValue = 0;

	//Is there an active object...
	CBase*				pCObject				= m_pCMDIObjects->m_pCObjTree->GetSelectedObject();
	CContainerBase*		pCContainerBase			= SOURCE_GETOBJECT(pCObject, CContainerBase);
	CAsynchBase*		pCAsynchBase			= SOURCE_GETOBJECT(pCObject, CAsynchBase);
	CConnectionPoint*	pCConnectionPoint		= SOURCE_GETOBJECT(pCObject, CConnectionPoint);
	CRowPosition*		pCRowPosition			= SOURCE_GETOBJECT(pCObject, CRowPosition);
	CError*				pCError					= SOURCE_GETOBJECT(pCObject, CError);
	CStream*			pCStream				= SOURCE_GETOBJECT(pCObject, CStream);


	//Setup the Source info
	m_idSource = iID;
	m_pCSource = pCObject;

	switch(iID)
	{
		//File Menu
		ON_COMMAND(IDM_EXIT,									OnClose())
//		ON_COMMAND(IDM_SAVE,									OnSave())
//		ON_COMMAND(IDM_SAVEAS,									OnSave(TRUE))

		ON_COMMAND(IDM_FULLCONNECT,								OnConnect()) 
		ON_COMMAND(IDM_FULLDISCONNECT,							OnDisconnect())


		///////////////////////////////////////////////////////////////////
		// IUnknown
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND(IDM_UNKNOWN_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_DATASOURCE_ADDREF,								OnAddRef(pCObject)) 
		ON_COMMAND(IDM_SESSION_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_COMMAND_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_MULTIPLERESULTS_ADDREF,							OnAddRef(pCObject)) 
		ON_COMMAND(IDM_ROWSET_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_ROW_ADDREF,										OnAddRef(pCObject)) 
		ON_COMMAND(IDM_STREAM_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_ENUMERATOR_ADDREF,								OnAddRef(pCObject)) 
		ON_COMMAND(IDM_BINDER_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_SERVICECOMP_ADDREF,								OnAddRef(pCObject)) 
		ON_COMMAND(IDM_DATALINKS_ADDREF,								OnAddRef(pCObject)) 
		ON_COMMAND(IDM_DATASET_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_TRANSACTION_ADDREF,								OnAddRef(pCObject)) 
		ON_COMMAND(IDM_TRANSACTIONOPTIONS_ADDREF,						OnAddRef(pCObject)) 
		ON_COMMAND(IDM_ERROR_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_CUSTOMERROR_ADDREF,								OnAddRef(pCObject)) 
		ON_COMMAND(IDM_ROWPOS_ADDREF,									OnAddRef(pCObject)) 
		ON_COMMAND(IDM_CONNECTIONPOINT_ADDREF,							OnAddRef(pCObject)) 
		ON_COMMAND(IDM_POOLINFO_ADDREF,									OnAddRef(pCObject)) 

		ON_COMMAND(IDM_IUNKNOWN_RELEASE,								OnRelease(pCObject)) 
		ON_COMMAND(IDM_IUNKNOWN_QUERYINTERFACE,							OnQueryInterface(pCObject)) 
		ON_COMMAND(IDM_IUNKNOWN_AUTOQI,									OnAutoQI(pCObject))
		ON_COMMAND(IDM_IUNKNOWN_RELEASEALL,								OnReleaseAll(pCObject)) 
		ON_COMMAND(IDM_IUNKNOWN_RELEASECHILDREN,						OnReleaseAll(pCObject, TRUE/*fChildren*/)) 

		
		///////////////////////////////////////////////////////////////////
		// ISupportErrorInfo
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND(IDM_ISUPPORTERRORINFO,								OnSupportErrorInfo(pCObject)) 
		
		///////////////////////////////////////////////////////////////////
		// ISupportErrorInfo
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND(IDM_ICPC_FINDCONNECTIONPOINT,						OnFindConnectionPoint(pCContainerBase, IID_IDBAsynchNotify))
		ON_COMMAND(IDM_ICPC_ENUMCONNECTIONPOINTS,						FALSE)


		///////////////////////////////////////////////////////////////////
		// IConnectionPoint
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND(IDM_ICONNECTIONPOINT_GETCONNECTIONINTERFACE,			OnGetConnectionInterface(pCConnectionPoint))
		ON_COMMAND(IDM_ICONNECTIONPOINT_ADVISE,							OnAdvise(pCConnectionPoint))
		ON_COMMAND(IDM_ICONNECTIONPOINT_UNADVISE,						OnUnadvise(pCConnectionPoint))
		ON_COMMAND(IDM_ICONNECTIONPOINT_ENUMCONNECTIONS,				FALSE)
		ON_COMMAND(IDM_ICONNECTIONPOINT_GETCONNECTIONPOINTCONTAINER,	FALSE)

		
		///////////////////////////////////////////////////////////////////
		// IDBAsynchStatus
		//
		///////////////////////////////////////////////////////////////////
		ON_COMMAND(IDM_IDBASYNCHSTATUS_ABORT,							OnAbort(pCAsynchBase))
		ON_COMMAND(IDM_IDBASYNCHSTATUS_GETSTATUS,						OnGetStatus(pCAsynchBase))
		ON_COMMAND(IDM_IDBINITIALIZE_INITIALIZE,						OnInitialize(pCAsynchBase))
		ON_COMMAND(IDM_IDBINITIALIZE_UNINITIALIZE,						OnUninitialize(pCAsynchBase))

		
		///////////////////////////////////////////////////////////////////
		// IDBProperties
		//
		///////////////////////////////////////////////////////////////////
		case IDM_IDBPROPERTIES_SETPROPERTIES:
		{	
			IDBProperties* pIDBProperties = SOURCE_GETINTERFACE(pCObject, IDBProperties);
			if(pIDBProperties)
			{
				CDataSource* pCDataSource = SOURCE_GETOBJECT(pCObject, CDataSource);
				
				CPropertiesDlg sCPropertiesDlg(this);
				sCPropertiesDlg.SetProperties(m_hWnd, pCDataSource ? (pCDataSource->IsInitialized() ? &DBPROPSET_DATASOURCEALL : &DBPROPSET_DBINITALL) : NULL, IID_IDBProperties, pIDBProperties, pIDBProperties);
			}
			return TRUE;
		}

		case IDM_IDBPROPERTIES_GETPROPERTIES:
		{	
			IDBProperties* pIDBProperties = SOURCE_GETINTERFACE(pCObject, IDBProperties);
			if(pIDBProperties)
			{
				CPropertiesDlg sCPropertiesDlg(this);
				sCPropertiesDlg.GetProperties(m_hWnd, &DBPROPSET_DATASOURCEALL, IID_IDBProperties, pIDBProperties, pIDBProperties);
			}
			return TRUE;
		}

		case IDM_IDBPROPERTIES_GETPROPERTYINFO:
		{	
			IDBProperties* pIDBProperties = SOURCE_GETINTERFACE(pCObject, IDBProperties);
			if(pIDBProperties)
			{
				CPropertiesDlg sCPropertiesDlg(this);
				sCPropertiesDlg.GetPropertyInfo(m_hWnd, &DBPROPSET_DATASOURCEALL, IID_IDBProperties, pIDBProperties);
			}
			return TRUE;
		}

			
		///////////////////////////////////////////////////////////////////
		// IBindResource
		//
		///////////////////////////////////////////////////////////////////
		case IDM_IBINDRESOURCE_BIND:
			if(SOURCE_GETINTERFACE(pCObject, IBindResource))
				DisplayDialog(IDD_BINDRESOURCE, m_hWnd, CMainWindow::BindResourceProc, pCObject, iID);
			return TRUE;

		case IDM_ICREATEROW_CREATEROW:
			if(SOURCE_GETINTERFACE(pCObject, ICreateRow))
				DisplayDialog(IDD_BINDRESOURCE, m_hWnd, CMainWindow::BindResourceProc, pCObject, iID);
			return TRUE;
		

		case IDM_IREGISTERPROVIDER_SETURLMAPPING:
			if(SOURCE_GETINTERFACE(pCObject, IRegisterProvider))
				DisplayDialog(IDD_URLMAPPING, m_hWnd, CMainWindow::RegisterProviderProc, pCObject, iID);
			return TRUE;

		case IDM_IREGISTERPROVIDER_GETURLMAPPING:
			if(SOURCE_GETINTERFACE(pCObject, IRegisterProvider))
				DisplayDialog(IDD_URLMAPPING, m_hWnd, CMainWindow::RegisterProviderProc, pCObject, iID);
			return TRUE;

		case IDM_IREGISTERPROVIDER_UNREGISTERPROVIDER:
			if(SOURCE_GETINTERFACE(pCObject, IRegisterProvider))
				DisplayDialog(IDD_URLMAPPING, m_hWnd, CMainWindow::RegisterProviderProc, pCObject, iID);
			return TRUE;

		case IDM_IDBBINDERPROPERTIES_RESET:
		{
			IDBBinderProperties* pIDBBinderProperties = SOURCE_GETINTERFACE(pCObject, IDBBinderProperties);
			if(pIDBBinderProperties)
			{
				CWaitCursor waitCursor;

				//IDBBinderProperties::Reset
				XTEST(hr = pIDBBinderProperties->Reset());
				TRACE_METHOD(hr, L"IDBBinderProperties::Reset()");
			}
			return TRUE;
		}

		
		///////////////////////////////////////////////////////////////////
		// Stream
		//
		///////////////////////////////////////////////////////////////////
		case IDM_ISTREAM_READ:
		case IDM_ISTREAM_WRITE:
		case IDM_ISEQSTREAM_READ:
		case IDM_ISEQSTREAM_WRITE:
			if(pCStream)
				DisplayDialog(IDD_STREAM_VIEWER, m_hWnd, CMainWindow::StreamViewerProc, pCObject, iID);
			return TRUE;

		case IDM_ISTREAM_CLONE:
			if(pCStream)
			{
				IStream* pIStream = NULL;
				
				//IStream::Clone
				if(SUCCEEDED(pCStream->Clone(&pIStream)))
				{
					//Display the new object
					HandleObjectType(pCStream, pIStream, IID_IStream, eCStream, 0, NULL, CREATE_DETERMINE_TYPE);
				}

				SAFE_RELEASE(pIStream);
			}
			return TRUE;

		case IDM_ISTREAM_SEEK:
			if(pCStream)
			{
				LARGE_INTEGER lgOffset;
				lgOffset.QuadPart = 0;
				
				//IStream::Seek
				XTEST(pCStream->Seek(lgOffset, STREAM_SEEK_SET, NULL));
			}
			return TRUE;

		case IDM_ISTREAM_STAT:
			if(pCStream)
			{
				STATSTG statstg;
				pCStream->Stat(&statstg);
			}
			return TRUE;


		///////////////////////////////////////////////////////////////////
		// Enumerator
		//
		///////////////////////////////////////////////////////////////////
		case IDM_ISOURCESROWSET_GETSOURCESROWSET:
			if(SOURCE_GETINTERFACE(pCObject, ISourcesRowset))
				DisplayDialog(IDD_GETSOURCESROWSET, m_hWnd, CMainWindow::GetSourcesRowsetProc, pCObject, iID);
			return TRUE;


		//Service Components
		ON_COMMAND(IDM_IDATAINITIALIZE_CREATEDBINSTANCE,				OnConnect()) 
		ON_COMMAND(IDM_IDATAINITIALIZE_CREATEDBINSTANCEEX,				OnConnect()) 
		ON_COMMAND(IDM_IDATAINITIALIZE_LOADSTRINGFROMSTORAGE,			OnLoadStringFromStorage()) 
		ON_COMMAND(IDM_IDATAINITIALIZE_GETDATASOURCE,					OnGetDataSource()) 
		ON_COMMAND(IDM_IDBPROMPTINITIALIZE_PROMPTDATASOURCE,			OnPromptDataSource()) 
		ON_COMMAND(IDM_IDBPROMPTINITIALIZE_PROMPTDATASOURCE_ADVANCED,	OnPromptDataSource(TRUE/*fAdvanced*/)) 
		ON_COMMAND(IDM_IDBPROMPTINITIALIZE_PROMPTFILENAME,				OnPromptFileName()) 
		ON_COMMAND(IDM_IDBPROMPTINITIALIZE_PROMPTFILENAME_ADVANCED,		OnPromptFileName(TRUE/*fAdvanced*/)) 


		//IDataInitialize
		case IDM_IDATAINITIALIZE_GETINITIALIZATIONSTRING:
		case IDM_IDATAINITIALIZE_GETINITIALIZATIONSTRING_NOPASSWORD:
			if(m_pCServiceComp)
			{
				CWaitCursor waitCursor;

				//Password
				WCHAR* pwszInitString = NULL;
				boolean fIncludePassword = (iID == IDM_IDATAINITIALIZE_GETINITIALIZATIONSTRING);
				
				//Obtain the Active DataSource
				IUnknown* pIUnknown = NULL;
				CMDIChild* pCMDIChild = (CMDIChild*)FindWindow(L"MDICHILD");
				if(pCMDIChild)
					pIUnknown = SOURCE_GETINTERFACE(pCMDIChild->GetObject(eCDataSource), IUnknown);

				//GetInitializationString
				m_pCServiceComp->GetInitString(pIUnknown, fIncludePassword, &pwszInitString);
				SAFE_FREE(pwszInitString);
			}
			return TRUE;
		
		//IDataInitialize
		case IDM_IDATAINITIALIZE_WRITESTRINGTOSTORAGE:
			if(m_pCServiceComp)
			{
				CWaitCursor waitCursor;

				WCHAR* pwszInitString = NULL;
				boolean fIncludePassword = TRUE;

				//Use active object...
				CServiceComp* pCServiceComp	= m_pCServiceComp;
				
				//Obtain the Active DataSource
				IUnknown* pIUnknown = NULL;
				CMDIChild* pCMDIChild = (CMDIChild*)FindWindow(L"MDICHILD");
				if(pCMDIChild)
					pIUnknown = SOURCE_GETINTERFACE(pCMDIChild->GetObject(eCDataSource), IUnknown);

				//Get the InitString for this DataSource
				hr = pCServiceComp->GetInitString(pIUnknown, fIncludePassword, &pwszInitString);
				if(SUCCEEDED(hr))
				{
					//Find the FileName to Save as...
					WCHAR wszFileName[MAX_QUERY_LEN] = {0};
					
					//Display Common Dialog to obtain File To Save...
					hr = BrowseSaveFileName(GetAppLite()->m_hInstance, m_hWnd, L"IDataInitialize::WriteStringToStorage", wszFileName, MAX_QUERY_LEN, L"udl", L"Microsoft Data Link Files (.udl)\0*.udl\0All Files (*.*)\0*.*\0\0");
					if(SUCCEEDED(hr))
						pCServiceComp->SaveInitString(wszFileName, pwszInitString);
				}
				SAFE_FREE(pwszInitString);
			}
			return TRUE;


		ON_COMMAND(IDM_CREATEFILEMONIKER,						OnCreateFileMoniker()) 

		ON_COMMAND(IDM_CREATE_ROOTBINDER,						OnRootBinder()) 
		ON_COMMAND(IDM_CREATE_ROOTENUMERATOR,					OnRootEnumerator()) 
		ON_COMMAND(IDM_CREATE_SERVICECOMPONENTS,				OnServiceComponents()) 
		ON_COMMAND(IDM_CREATE_DATALINKS,						OnDataLinks()) 

		ON_COMMAND(IDM_RECENTCONFIG1,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG2,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG3,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG4,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG5,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG6,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG7,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG8,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG9,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 
		ON_COMMAND(IDM_RECENTCONFIG10,							OnLoadRecentConfig(iID - IDM_RECENTCONFIG1)) 

		ON_COMMAND(IDM_RECENTFILE1,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE2,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE3,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE4,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE5,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE6,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE7,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE8,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE9,								OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
		ON_COMMAND(IDM_RECENTFILE10,							OnLoadRecentFile(iID - IDM_RECENTFILE1)) 
									
		//DataSource Menu

		//Session Menu

		//Command Menu

		//Rowset Menu
		case IDM_CREATE_ROWPOSITION:
			if(m_pCRootEnumerator)
			{
				CWaitCursor waitCursor;
				IUnknown* pIUnknown = NULL;

				//Create the Row Position object...
				hr = m_pCRootEnumerator->CreateInstance(NULL, CLSID_OLEDB_ROWPOSITIONLIBRARY, CLSCTX_ALL, IID_IUnknown, (IUnknown**)&pIUnknown);
				if(SUCCEEDED(hr))
					HandleObjectType(pCObject, pIUnknown, IID_IUnknown, eCRowPosition, 0, NULL, pCObject ? CREATE_INITIALIZE : 0);
				TRACE_RELEASE(pIUnknown, L"IUnknown");
			}
			return TRUE;

		case IDM_IROWPOS_INITIALIZE:
			if(pCRowPosition && pCRowPosition->m_pIRowPosition)
			{
				CWaitCursor waitCursor;
				IUnknown* pIUnknown = NULL;
				CMDIChild* pCMDIChild = (CMDIChild*)FindWindow(L"MDICHILD");
				if(pCMDIChild)
					pIUnknown = SOURCE_GETINTERFACE(pCMDIChild->GetObject(eCRowset), IUnknown);

				//Initialize RowPosition Object to use the current rowset object...
				pCRowPosition->Initialize(pIUnknown);	
			}
			return TRUE;
		
		case IDM_IROWPOS_GETROWSET:
			if(pCRowPosition && pCRowPosition->m_pIRowPosition)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IRowPosition::GetRowset", IID_IRowset); 
				if(interfaceDlg.DoModal(m_hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//IRowPosition::GetRowset
					IUnknown* pIUnknown = NULL;
					
					//What type of object, let our helper determine...
					if(SUCCEEDED(hr = pCRowPosition->GetRowset(interfaceDlg.GetSelInterface(), &pIUnknown)))
						HandleObjectType(pCRowPosition, pIUnknown, interfaceDlg.GetSelInterface(), eCRowset, 0, NULL, CREATE_FINDWINDOW);
					TRACE_RELEASE(pIUnknown, L"IUnknown");
				}
			}
			return TRUE;

		case IDM_IROWPOS_CLEARROWPOSITION:
			if(pCRowPosition && pCRowPosition->m_pIRowPosition)
			{
				CWaitCursor waitCursor;

				XTEST(hr = pCRowPosition->m_pIRowPosition->ClearRowPosition());
				TRACE_METHOD(hr, L"IRowPosition::ClearRowPosition()");
			}
			return TRUE;

		case IDM_IROWPOS_SETROWPOSITION:
			if(pCRowPosition && pCRowPosition->m_pIRowPosition)
			{
				//Find the SelectedRow
				HROW hRow = NULL;
				HCHAPTER hChapter = NULL;
				CMDIChild* pCMDIChild = (CMDIChild*)FindWindow(L"MDICHILD");
				if(pCMDIChild)
				{
					pCMDIChild->m_pCDataGrid->GetSelectedRow(&hRow);
					CRowset* pCRowset = (CRowset*)pCMDIChild->GetObject(eCRowset);
					if(pCRowset)
						hChapter = pCRowset->m_hChapter;
				}
				
				CWaitCursor waitCursor;

				//Set RowPosition
				XTEST(hr = pCRowPosition->m_pIRowPosition->SetRowPosition(hChapter, hRow, DBPOSITION_OK));
				TRACE_METHOD(hr, L"IRowPosition::SetRowPosition(0x%p, 0x%p, %d)", hChapter, hRow, DBPOSITION_OK);
			}
			return TRUE;

		case IDM_IROWPOS_GETROWPOSITION:
			if(pCRowPosition && pCRowPosition->m_pIRowPosition)
			{
				CWaitCursor waitCursor;

				HCHAPTER hChapter = NULL;
				HROW hRow = NULL;
				DBPOSITIONFLAGS dwPositionFlag = DBPOSITION_OK;
				IRowset* pIRowset = NULL;

				//IRowPosition::GetRowPoisition
				XTEST(hr = pCRowPosition->m_pIRowPosition->GetRowPosition(&hChapter, &hRow, &dwPositionFlag));
				TRACE_METHOD(hr, L"IRowPosition::GetRowPosition(&0x%p, &0x%p, &%d)", hChapter, hRow, dwPositionFlag);

				//Now release the info...
				if(SUCCEEDED(hr) && hRow)
				{
					//The simplest is to just obtain the Rowset that this RowPosition object is 
					//initialized to, and then call ReleaseRows...
					if(SUCCEEDED(pCRowPosition->GetRowset(IID_IRowset, (IUnknown**)&pIRowset)))
					{
						ULONG ulRefCount = 0;
						DBROWSTATUS ulRowStatus = 0;

						//IRowset::ReleaseRows
						hr = pIRowset->ReleaseRows(1, &hRow, NULL, &ulRefCount, &ulRowStatus);
						TRACE_METHOD(hr, L"IRowset::ReleaseRows(%lu, &0x%p, NULL, &%d, &%d)", 1, hRow, ulRefCount, ulRowStatus);

						//Release the rowset...
						TRACE_RELEASE(pIRowset, L"IUnknown");
					}
				}
			}
			return TRUE;


		//Row Menu

		//Error Menu
		ON_COMMAND(IDM_DISPLAYERRORINFO,						DisplayErrorInfo(S_OK))
		
		case IDM_GETERRORINFO:
		{
			IErrorInfo* pIErrorInfo = NULL;
			
			//Obtain an errorinfo...
			XTEST(hr = GetErrorInfo(0, &pIErrorInfo));
			TRACE_METHOD(hr, L"GetErrorInfo(0, &0x%p)", pIErrorInfo);

			if(SUCCEEDED(hr) && pIErrorInfo)
				HandleObjectType(NULL/*pCSource*/, pIErrorInfo, IID_IErrorInfo, eCError, 0/*cPropSets*/, NULL/*rgPropSets*/, 0/*dwFlags*/);
			TRACE_RELEASE(pIErrorInfo, L"IErrorInfo");
			break;
		}

		case IDM_SETERRORINFO:
		{
			XTEST(hr = SetErrorInfo(0, NULL));
			TRACE_METHOD(hr, L"SetErrorInfo(0, NULL)");
			break;
		}

		//IErrorInfo
		case IDM_ERRORINFO_GETDESCRIPTION:
			if(pCError)
			{
				//IErrorInfo::GetDescription
				CComBSTR bstrDescription;
				pCError->GetDescription(&bstrDescription);
			}
			return TRUE;	

		case IDM_ERRORINFO_GETGUID:
			if(pCError)
			{
				GUID guid;
				
				//IErrorInfo::GetGUID
				pCError->GetGUID(&guid);
			}
			return TRUE;

		case IDM_ERRORINFO_GETHELPCONTEXT:
			if(pCError)
			{
				DWORD dwHelpContext = 0;
				
				//IErrorInfo::GetHelpContext
				pCError->GetHelpContext(&dwHelpContext);
			}
			return TRUE;
		
		case IDM_ERRORINFO_GETHELPFILE:
			if(pCError)
			{
				//IErrorInfo::GetHelpFile
				CComBSTR bstrHelpFile;
				pCError->GetHelpFile(&bstrHelpFile);
			}
			return TRUE;

		case IDM_ERRORINFO_GETSOURCE:
			if(pCError)
			{
				//IErrorInfo::GetSource
				CComBSTR bstrSource;
				pCError->GetSource(&bstrSource);
			}
			return TRUE;

		//IErrorRecords
		case IDM_ERRORRECORDS_GETRECORDCOUNT:
			if(pCError)
			{
				ULONG ulCount = 0;

				//IErrorRecords::GetRecordCount
				pCError->GetRecordCount(&ulCount);
			}
			return TRUE;
		
		case IDM_ERRORRECORDS_GETCUSTOMERROROBJECT:
			if(pCError)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IErrorRecords::GetCustomErrorObject", IID_ISQLErrorInfo); 
				if(interfaceDlg.DoModal(m_hWnd) == IDOK)
				{
					CWaitCursor waitCursor;
					ULONG ulRecord = 0;

					//IErrorRecords::GetCustomErrorObject
					//What type of object, let our helper determine...
					if(SUCCEEDED(hr = pCError->GetCustomErrorObject(ulRecord, interfaceDlg.GetSelInterface(), interfaceDlg.ppUnknown())))
						HandleObjectType(pCError, interfaceDlg.pUnknown(), interfaceDlg.GetSelInterface(), eCCustomError, 0, NULL, 0/*dwFlags*/);
				}
			}
			return TRUE;

		case IDM_ERRORRECORDS_GETERRORINFO:
			if(pCError)
				DisplayDialog(IDD_ERRORRECORDS, m_hWnd, CMainWindow::ErrorRecordsProc, pCError);
			return TRUE;
		

		case IDM_SQLERRORINFO_GETSQLINFO:
		{	
			CCustomError* pCCustomError = SOURCE_GETOBJECT(pCObject, CCustomError);
			if(pCCustomError && pCCustomError->m_pISQLErrorInfo)
			{
				CComBSTR bstr;
				LONG lNativeError = 0;

				//ISQLErrorInfo::GetSQLInfo
				hr = pCCustomError->GetSQLInfo(&bstr, &lNativeError);
			}
			return TRUE;
		}


		//Objects Menu

		//Tools Menu
		ON_COMMAND(IDM_OPTIONS,				OnOptions())
		ON_COMMAND(IDM_TRACEWINDOW,			OnTraceWindow())
		ON_COMMAND(IDM_OBJECTSWINDOW,		OnObjectsWindow())

		//Window Menu
		ON_COMMAND(IDM_CLOSEWINDOW,			OnDisconnect())
									
		ON_COMMAND(IDM_TILEHORIZONTAL,		OnTile(TRUE))
		ON_COMMAND(IDM_TILEVERTICAL,		OnTile(FALSE))
		ON_COMMAND(IDM_CASCADE,				OnCascade())
		ON_COMMAND(IDM_ARRANGEICONS,		OnArrangeIcons())
		ON_COMMAND(IDM_AUTOPOSITION,		OnAutoPosition())
		ON_COMMAND(IDM_CLOSEALL,			OnCloseAll())
		ON_COMMAND(IDM_NEXTWINDOW,			OnNextWindow(TRUE))
		ON_COMMAND(IDM_PREVWINDOW,			OnNextWindow(FALSE))

		//Help Menu
		ON_COMMAND(IDM_ABOUT,				OnAbout())
	};	

	return CMDIFrameLite::OnCommand(iID, hWndCtrl);
}



////////////////////////////////////////////////////////////////
// CMainWindow::OnMenuSelect
//
/////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnMenuSelect(UINT uID)
{
	//TODO - status text should probably also be for free...
	
	if(uID)
	{
		WCHAR wszBuffer[MAX_NAME_LEN+1] = {0};

		if(IsUnicodeOS())
		{
			//Load the string from the StringTable in the resource
			//To diaply the text associated with this menu item...
			LoadStringW(GetAppLite()->m_hInstance, uID, wszBuffer, MAX_NAME_LEN);
			m_CStatusBar.SetText(wszBuffer, STATUSBAR_TEXT, 0/*SBT_NOBORDERS*/);
		}
		else
		{
			CHAR szBuffer[MAX_NAME_LEN];

			LoadStringA(GetAppLite()->m_hInstance, uID, szBuffer, MAX_NAME_LEN);
			ConvertToWCHAR(szBuffer, wszBuffer, MAX_NAME_LEN);
			m_CStatusBar.SetText(wszBuffer, STATUSBAR_TEXT, 0/*SBT_NOBORDERS*/);
		}

	}
	else
	{
		//Otherwise we are using the Status Bar to indicate
		//the number of selected variations
		UpdateControls();
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMainWindow::OnInitMenuPopup
//
/////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnInitMenuPopup(HMENU hMenu, UINT uPos, BOOL fSysMenu)
{
	//NOTE: Having every menu item in all objects menu have unique ids, makes adding the same interface
	//to another object very painful, and causes manually editing the menu resources, making sure its
	//added to OnUpdateCommand for enabling/disabling the menu item, as well as adding another
	//case in OnCommand for the same interface! 

	//Instead we have devised a design where we can intitally set the item data of every menu
	//to contain the object identifier.  This way we can always obtain the stored data of the menu
	//and instantly know the type of object this interface/method is called from.  Saves hundreads 
	//of lines of code, and almost ellimiates all places to remember to add code to...

	//To do this we must first know what menu we have.  This is also a designed that all object
	//menu have as the first item an "IUnknown" menu, which contains as the first element
	//a IDM_*_ADDREF call, so we know exactly the object source...

	//Obtain the first SubMenu
	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	if(hSubMenu)
	{
		SOURCE eSource = eInvalid;
		ULONG ulMenuID = GetMenuItemID(hSubMenu, 0);

		switch(ulMenuID)
		{
			case IDM_UNKNOWN_ADDREF:
				eSource = eCUnknown;
				break;
			case IDM_DATASOURCE_ADDREF:
				eSource = eCDataSource;
				break;
			case IDM_SESSION_ADDREF:
				eSource = eCSession;
				break;
			case IDM_COMMAND_ADDREF:
				eSource = eCCommand;
				break;
			case IDM_MULTIPLERESULTS_ADDREF:
				eSource = eCMultipleResults;
				break;
			case IDM_ROWSET_ADDREF:
				eSource = eCRowset;
				break;
			case IDM_ROW_ADDREF:
				eSource = eCRow;
				break;
			case IDM_STREAM_ADDREF:
				eSource = eCStream;
				break;
			case IDM_ENUMERATOR_ADDREF:
				eSource = eCEnumerator;
				break;
			case IDM_BINDER_ADDREF:
				eSource = eCBinder;
				break;
			case IDM_SERVICECOMP_ADDREF:
				eSource = eCServiceComp;
				break;
			case IDM_DATALINKS_ADDREF:
				eSource = eCDataLinks;
				break;
			case IDM_DATASET_ADDREF:
				eSource = eCDataset;
				break;
			case IDM_TRANSACTION_ADDREF:
				eSource = eCTransaction;
				break;
			case IDM_TRANSACTIONOPTIONS_ADDREF:
				eSource = eCTransactionOptions;
				break;
			case IDM_ERROR_ADDREF:
				eSource = eCError;
				break;
			case IDM_CUSTOMERROR_ADDREF:
				eSource = eCCustomError;
				break;
			case IDM_ROWPOS_ADDREF: 
				eSource = eCRowPosition;
				break;
			case IDM_CONNECTIONPOINT_ADDREF:
				eSource = eCConnectionPoint;
				break;
			default:
	//			ASSERT(!"Unknown Menu?");
				break;
		};

		//Store this object as an data value of the menu and all sub menus...
		if(eSource != eInvalid)
			SetSubMenuData(hMenu, 0, TRUE/*fByPosition*/, eSource);

		CBase* pCBase		  = NULL;
		
		//Now that we know what object the menu refers to, select
		//the corresponding object in the tree...
		CMDIChild* pCMDIChild = (CMDIChild*)GetActiveWindow(L"MDICHILD");
		if(pCMDIChild)
		{
			pCBase = pCMDIChild->GetObject(eSource);
			if(!pCBase)
			{
				//Try and work backwards...
				pCBase = pCMDIChild->GetObject();
				if(pCBase && (pCBase->GetObjectType() != eSource))
					pCBase = pCBase->GetParent(eSource);
			}
		}
		
		//If the MDIChild window is not active, try and guess which object this action
		//is for by the selected object in the tree...
		if(!pCBase)
		{
			//If the objects window is active
			CMDIObjects* pCMDIObjects = m_pCMDIObjects;
			if(pCMDIObjects)
			{
				//we know exactly what object this refers to...
				pCBase = pCMDIObjects->m_pCObjTree->GetSelectedObject();
				if(pCBase)
				{
					//If its not the correct type, then maybe its the parent...
					if(pCBase->GetObjectType() != eSource)
						pCBase = pCBase->GetParent(eSource);
				}
			}
		}
		
		//Maybe its part of the main window...
		if(!pCBase)
			pCBase = GetObject(eSource);

		//Select the object
		if(pCBase)
			m_pCMDIObjects->m_pCObjTree->SelectObject(pCBase);
	}

	//Delegate
	return CMDIFrameLite::OnInitMenuPopup(hMenu, uPos, fSysMenu);
}



////////////////////////////////////////////////////////////////
// CMainWindow::GetObject
//
/////////////////////////////////////////////////////////////////
CBase* CMainWindow::GetObject(SOURCE eSource, BOOL fAlways)
{
	CBase** ppCBase = GetObjectAddress(eSource);
	if(ppCBase)
	{
		CBase* pCBase = *ppCBase;
		if(pCBase && (eSource==eCUnknown || pCBase->GetObjectType()==eSource || pCBase->GetBaseType() & eSource) && (fAlways || pCBase->m_pIUnknown))
			return pCBase;
	}

	return NULL;
}


////////////////////////////////////////////////////////////////
// CMainWindow::GetObjectAddress
//
/////////////////////////////////////////////////////////////////
CBase** CMainWindow::GetObjectAddress(SOURCE eSource)
{
	CBase** ppCObject = NULL;
	
	switch(eSource)
	{
		case eCBinder:
			ppCObject = (CBase**)&m_pCRootBinder;
			break;
	
		case eCEnumerator:
			ppCObject = (CBase**)&m_pCRootEnumerator;
			break;
	
		case eCServiceComp:
			ppCObject = (CBase**)&m_pCServiceComp;
			break;
	
		case eCDataLinks:
			ppCObject = (CBase**)&m_pCDataLinks;
			break;

		case eCUnknown:
			//Determine "Last" object type...
			if(m_pCRootEnumerator && m_pCRootEnumerator->m_pIUnknown)
				return (CBase**)&m_pCRootEnumerator;
			else if(m_pCRootBinder && m_pCRootBinder->m_pIUnknown)
				return (CBase**)&m_pCRootBinder;
			else if(m_pCServiceComp && m_pCServiceComp->m_pIUnknown)
				return (CBase**)&m_pCServiceComp;
			else if(m_pCDataLinks && m_pCDataLinks->m_pIUnknown)
				return (CBase**)&m_pCDataLinks;
			break;
		
		default:
			break;
	};

	return ppCObject;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnSize
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::OnSize(WPARAM nType, REFPOINTS pts)
{
	ASSERT(m_CToolBar.m_hWnd);
	ASSERT(m_CStatusBar.m_hWnd);
	
	// calculate proper text height for tool and status bars
	SIZE sizeToolBar	= GetWindowSize(m_CToolBar.m_hWnd);
	SIZE sizeStatusBar	= GetWindowSize(m_CStatusBar.m_hWnd);

	switch(nType)
	{
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
		{
			if(pts.x && pts.y)
			{
				//ToolBar
				MoveWindow(m_CToolBar.m_hWnd,	0, 0, pts.x, sizeToolBar.cy, /*FALSE*/TRUE);
				
				//StatusBar
				MoveWindow(m_CStatusBar.m_hWnd, 0, pts.y-sizeStatusBar.cy, pts.x, sizeStatusBar.cy, /*FALSE*/TRUE);

				//MDI Client Area
				MoveWindow(m_hWndMDIClient,		0, sizeToolBar.cy, pts.x, pts.y-sizeToolBar.cy-sizeStatusBar.cy, /*FALSE*/TRUE);
				return TRUE;
			}

			return FALSE;
		}
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::LoadSettings
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::LoadSettings() 
{
	HKEY hOptionsKey = NULL;
	memset(&m_wndPlacement,	0, sizeof(m_wndPlacement));

	//Open the key to make things a little quicker and direct...
	TESTC_(OpenRegKey(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, 0, KEY_READ, &hOptionsKey),S_OK);

	//Window Positions
	GetRegEntry(hOptionsKey, NULL, L"WinPosition",		&m_wndPlacement,	sizeof(m_wndPlacement), NULL);
	
CLEANUP:
	CloseRegKey(hOptionsKey);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::SaveSettings
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMainWindow::SaveSettings() 
{
	HKEY hOptionsKey = NULL;

	//Open the key to make things a little quicker and direct...
	TESTC_(CreateRegKey(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, &hOptionsKey),S_OK);
	
	//Window Positions
	if(GetWindowPlacement())
		SetRegEntry(hOptionsKey, NULL, L"WinPosition",		&m_wndPlacement,	sizeof(m_wndPlacement));

CLEANUP:
	CloseRegKey(hOptionsKey);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnAbout
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal(m_hWnd);
}


////////////////////////////////////////////////////////////////
// CMainWindow::UpdateControls
//
/////////////////////////////////////////////////////////////////
BOOL CMainWindow::UpdateControls()
{
	//Obtain the Active MDI window
	CMDIChild*		pCMDIChild		= (CMDIChild*)GetActiveWindow(L"MDICHILD");
	CDataSource*	pCDataSource	= pCMDIChild ? (CDataSource*)pCMDIChild->GetObject(eCDataSource) : NULL;
	CSession*		pCSession		= pCMDIChild ? (CSession*)pCMDIChild->GetObject(eCSession) : NULL;
	CCommand*		pCCommand		= pCMDIChild ? (CCommand*)pCMDIChild->GetObject(eCCommand) : NULL;
	CRowset*		pCRowset		= pCMDIChild ? (CRowset*)pCMDIChild->GetObject(eCRowset) : NULL;

	//Update the ToolBar
	m_CToolBar.EnableButton(IDM_FULLCONNECT,						TRUE);
	m_CToolBar.EnableButton(IDM_FULLDISCONNECT,						pCMDIChild != NULL);
	m_CToolBar.EnableButton(IDM_IDBCREATESESSION_CREATESESSION,		pCDataSource	&& pCDataSource->m_pIDBCreateSession);
	m_CToolBar.EnableButton(IDM_OPENROWSET,							pCSession && pCSession->m_pIOpenRowset);
	m_CToolBar.EnableButton(IDM_EXECUTE,							pCCommand		&& pCCommand->m_pICommand);
	m_CToolBar.EnableButton(IDM_RESTARTPOSITION,					pCRowset		&& pCRowset->m_pIRowset);
	m_CToolBar.EnableButton(IDM_REFRESH,							pCRowset		&& pCRowset->m_pIRowset);
	m_CToolBar.EnableButton(IDM_GETSCHEMAROWSET,					pCSession		&& pCSession->m_pIDBSchemaRowset);
	m_CToolBar.EnableButton(IDM_INSERTROW,							pCRowset		&& pCRowset->m_pIRowsetChange);
	m_CToolBar.EnableButton(IDM_DELETEROWS,							pCRowset		&& pCRowset->m_pIRowsetChange);
	m_CToolBar.EnableButton(IDM_SETDATA,							pCRowset		&& pCRowset->m_pIRowsetChange);
	m_CToolBar.EnableButton(IDM_UPDATE,								pCRowset		&& pCRowset->m_pIRowsetUpdate);
	m_CToolBar.EnableButton(IDM_UNDO,								pCRowset		&& pCRowset->m_pIRowsetUpdate);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMainWindow::RemoveAllChildren
//
/////////////////////////////////////////////////////////////////
BOOL CMainWindow::RemoveAllChildren()
{
	CMDIChild* pCMDIChild = NULL;

	//hide MDI Client Windows to avoid repaints
	::ShowWindow(m_hWndMDIClient, SW_HIDE);
	
	//First remove all MDI Child windows
	while(pCMDIChild = (CMDIChild*)FindWindow(L"MDICHILD"))
	{
		MDIDestroy(pCMDIChild->m_hWnd);
		SAFE_DELETE(pCMDIChild);
	}

	::ShowWindow(m_hWndMDIClient, SW_SHOW);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnAddRef
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnAddRef(CBase* pCBase)
{
	if(pCBase && pCBase->m_pIUnknown)
	{
		CWaitCursor waitCursor;
		pCBase->ObjectAddRef();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnRelease
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnRelease(CBase* pCBase)
{
	if(pCBase && pCBase->m_pIUnknown)
	{
		CWaitCursor waitCursor;
	
		//Release this object
		pCBase->ObjectRelease();
		UpdateControls();
		
		//Update the Window
		CMDIChild* pCMDIChild = pCBase->m_pCMDIChild;
		if(pCMDIChild)
		{
			//if there are no more objects remaining for this window,
			//there is no real reason to display it...
			if(!pCMDIChild->GetObject())
			{
				//No objects left, remove the window...
				MDIDestroy(pCMDIChild->m_hWnd);
				SAFE_DELETE(pCMDIChild);
			}
			else
			{
				//There is at least one object, just update the controls...
				pCMDIChild->UpdateControls();
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnReleaseAll
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnReleaseAll(CBase* pCBase, BOOL fChildren)
{
	if(pCBase && pCBase->m_pIUnknown)
	{
		CWaitCursor waitCursor;

		//Release children
		if(fChildren)
			pCBase->ReleaseChildren();
		
		//Now, Release this object
		pCBase->ReleaseObject();
		UpdateControls();

		//Update the Window
		CMDIChild* pCMDIChild = pCBase->m_pCMDIChild;
		if(pCMDIChild)
		{
			//if there are no more objects remaining for this window,
			//there is no real reason to display it...
			CBase* pCBase = pCMDIChild->GetObject();
			if(!pCBase || pCBase->m_pCMDIChild!=pCMDIChild)
			{
				//No objects left, remove the window...
				MDIDestroy(pCMDIChild->m_hWnd);
				SAFE_DELETE(pCMDIChild);
			}
			else
			{
				//There is at least one object, just update the controls...
				pCMDIChild->UpdateControls();
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnQueryInterface
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnQueryInterface(CBase* pCBase)
{
	if(pCBase && pCBase->m_pIUnknown)
	{
		CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IUnknown::QueryInterface", pCBase->GetDefaultInterface()); 
		if(interfaceDlg.DoModal(GetFocus()) == IDOK)
		{
			CWaitCursor waitCursor;

			//IUnknown::QueryInterface
			pCBase->ObjectQI(interfaceDlg.GetSelInterface(), interfaceDlg.ppUnknown());

			UpdateControls();
			if(pCBase->m_pCMDIChild)
				pCBase->m_pCMDIChild->UpdateControls();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnAutoQI
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnAutoQI(CBase* pCBase)
{
	if(pCBase && pCBase->m_pIUnknown)
	{
		CWaitCursor waitCursor;
		
		//Obtain all interfaces
		pCBase->AutoQI(CREATE_QI_MANDATORY | CREATE_QI_OPTIONAL);

		//Also "redraw" the object now that more interfaces are available for use
		//For Example: If the rowset was originally obtained with AutiQI off, and 
		//now the user wished to see the rowset, we display the object...
		pCBase->DisplayObject();

		//And update any common controls...
		UpdateControls();
		if(pCBase->m_pCMDIChild)
			pCBase->m_pCMDIChild->UpdateControls();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnSupportErrorInfo
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnSupportErrorInfo(CBase* pCBase)
{
	if(pCBase && pCBase->m_pISupportErrorInfo)
	{
		HRESULT hr = S_OK;

		CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"ISupportErrorInfo::InterfaceSupportsErrorInfo", pCBase->GetDefaultInterface()); 
		if(interfaceDlg.DoModal(GetFocus()) == IDOK)
		{
			CWaitCursor waitCursor;

			//ISupportErrorInfo::InterfaceSupportsErrorInfo
			XTEST(hr = pCBase->m_pISupportErrorInfo->InterfaceSupportsErrorInfo(interfaceDlg.GetSelInterface()));
			TRACE_METHOD(hr, L"ISupportErrorInfo::InterfaceSupportsErrorInfo(%s)", GetInterfaceName(interfaceDlg.GetSelInterface()));
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnFindConnectionPoint
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnFindConnectionPoint(CContainerBase* pCCPoint, REFIID riid)
{
	if(pCCPoint && pCCPoint->m_pIConnectionPointContainer)
	{
		//Display the Generic interface list...
		CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IConnectionPointContainer::FindConnectionPoint", riid); 
		if(interfaceDlg.DoModal(GetFocus()) == IDOK)
		{
			CWaitCursor waitCursor;
			IConnectionPoint* pIConnectionPoint = NULL;

			//IConnectionPointContainer::FindConnectionPoint
			if(SUCCEEDED(pCCPoint->FindConnectionPoint(interfaceDlg.GetSelInterface(), &pIConnectionPoint)))
				HandleObjectType(pCCPoint, pIConnectionPoint, interfaceDlg.GetSelInterface(), eCConnectionPoint, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
			TRACE_RELEASE(pIConnectionPoint, L"IConnectionPoint");
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnAdvise
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnAdvise(CConnectionPoint* pCConnectionPoint)
{
	if(pCConnectionPoint && pCConnectionPoint->m_pIConnectionPoint)
	{
		m_pCListener->Advise(pCConnectionPoint->m_pIConnectionPoint, &pCConnectionPoint->m_dwCookie);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnUnadvise
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnUnadvise(CConnectionPoint* pCConnectionPoint)
{
	if(pCConnectionPoint && pCConnectionPoint->m_pIConnectionPoint)
	{
		m_pCListener->Unadvise(pCConnectionPoint->m_pIConnectionPoint, &pCConnectionPoint->m_dwCookie);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnGetConnectionInterface
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnGetConnectionInterface(CConnectionPoint* pCConnectionPoint)
{
	if(pCConnectionPoint && pCConnectionPoint->m_pIConnectionPoint)
	{
		CWaitCursor waitCursor;
		IID iid;

		pCConnectionPoint->GetConnectionInterface(&iid);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnAbort
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnAbort(CAsynchBase* pCAsynchBase)
{
	if(pCAsynchBase && pCAsynchBase->m_pIDBAsynchStatus)
	{
		CWaitCursor waitCursor;

		//IDBAsynchStatus::Abort
		XTEST(pCAsynchBase->Abort(NULL, DBASYNCHOP_OPEN));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnGetStatus
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnGetStatus(CAsynchBase* pCAsynchBase)
{
	if(pCAsynchBase && pCAsynchBase->m_pIDBAsynchStatus)
	{
		CWaitCursor waitCursor;

		DBCOUNTITEM	ulProgress = 0;
		DBCOUNTITEM	ulProgressMax = 0;
		DBASYNCHPHASE	ulAsynchPhase = 0;
		LPOLESTR pwszStatusText = NULL;

		//IDBAsynchStatus::GetStatus
		XTEST(pCAsynchBase->GetStatus(NULL, DBASYNCHOP_OPEN, &ulProgress, &ulProgressMax, &ulAsynchPhase, &pwszStatusText));
		SAFE_FREE(pwszStatusText);
	}
}

			
/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnInitialize
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnInitialize(CAsynchBase* pCAsynchBase)
{
	if(pCAsynchBase && pCAsynchBase->m_pIDBInitialize)
	{
		CWaitCursor waitCursor;

		pCAsynchBase->Initialize();
		UpdateControls();
	}
}

		
/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnUninitialize
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnUninitialize(CAsynchBase* pCAsynchBase)
{
	if(pCAsynchBase && pCAsynchBase->m_pIDBInitialize)
	{
		CWaitCursor waitCursor;

		pCAsynchBase->Uninitialize();
		UpdateControls();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnConnect
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnConnect() 
{
	CWaitCursor waitCursor;

	//Bring up FullConnect dialog
	m_pCFullConnect->Display(m_hWnd);
	UpdateControls();
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnLoadStringFromStorage
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnLoadStringFromStorage(WCHAR* pwszFileName) 
{
	CWaitCursor waitCursor;
	WCHAR wszFileName[MAX_QUERY_LEN] = {0};
	HRESULT hr = S_OK;

	//Display Common Dialog to obtain File To Load...
	if(!pwszFileName)
	{
		hr = BrowseOpenFileName(GetAppLite()->m_hInstance, m_hWnd, L"IDataInitialize::LoadStringFromStorage", wszFileName, MAX_QUERY_LEN, L"udl", L"Microsoft Data Link Files (.udl)\0*.udl\0All Files (*.*)\0*.*\0\0");
		pwszFileName = wszFileName;
	}
	
	//Load the UDL file
	if(SUCCEEDED(hr))
	{
		WCHAR* pwszInitString = NULL;	

		//Now LoadStringFromStorage
		hr = m_pCServiceComp->LoadInitString(pwszFileName, &pwszInitString);
		if(SUCCEEDED(hr))
		{
			SAFE_FREE(m_pCServiceComp->m_pwszInitString);
			m_pCServiceComp->m_pwszInitString = pwszInitString;
		
			//Need to bring up the GetDataSource Dialog
			DisplayDialog(IDD_DATAINIT_GETDATASOURCE, m_hWnd, CMainWindow::GetDataSourceProc, m_pCServiceComp);
			UpdateControls();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnGetDataSource
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnGetDataSource() 
{
	//Default Operation for DataLinks is GetDataSource...
	m_pCServiceComp->OnDefOperation();
}
					
/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnCreateFileMoniker
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnCreateFileMoniker() 
{
	WCHAR wszFileName[MAX_QUERY_LEN] = {0};
	IMoniker* pIMoniker = NULL;
	IBindCtx* pIBindCtx = NULL;

	//Display Common Dialog to obtain File To Load...
	HRESULT hr = BrowseOpenFileName(GetAppLite()->m_hInstance, m_hWnd, L"CreateFileMoniker", wszFileName, MAX_QUERY_LEN, L"udl", L"Microsoft Data Link Files (.udl)\0*.udl\0All Files (*.*)\0*.*\0\0");
	if(SUCCEEDED(hr))
	{
		//CreateFileMoniker
		//TODO: Do we just want to create this as a tree object, and let the user play with
		//it or always just call BindToObject?
		XTEST(hr = CreateFileMoniker(wszFileName, &pIMoniker));
		TESTC(TRACE_METHOD(hr, L"CreateFileMoniker(\"%s\", &0x%p)", wszFileName, pIMoniker));

		//Need to ask for the riid...
		CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IMoniker::BindToObject", IID_IDBInitialize); 
		if(interfaceDlg.DoModal(GetFocus()) == IDOK)
		{
			CWaitCursor waitCursor;

			//Setup BIND_OPTS
			BIND_OPTS2 BindOps2;
			BindOps2.cbStruct		= sizeof(BIND_OPTS2);
			BindOps2.grfFlags		= BIND_MAYBOTHERUSER;
			BindOps2.grfMode		= STGM_READWRITE;
			BindOps2.dwTickCountDeadline = 0;
			BindOps2.dwTrackFlags	= 1;
			BindOps2.dwClassContext = CLSCTX_INPROC_SERVER;
			BindOps2.locale			= GetSystemDefaultLCID();
			BindOps2.pServerInfo	= NULL; 
			
			//If this fails for some reason, we may be albe to still continue...
			hr = CreateBindCtx(0, &pIBindCtx);
			if(SUCCEEDED(hr) && pIBindCtx)
				hr = pIBindCtx->SetBindOptions(&BindOps2);

			//IMoniker::BindToObject
			XTEST(hr = pIMoniker->BindToObject(pIBindCtx, NULL, interfaceDlg.GetSelInterface(), (void**)interfaceDlg.ppUnknown()));
			TESTC(TRACE_METHOD(hr, L"IMoniker::BindToObject(0x%p, 0x%p, %s, &0x%p)", pIBindCtx, NULL, GetInterfaceName(interfaceDlg.GetSelInterface()), interfaceDlg.pUnknown()));

			//Now dump the resultant object into our tree...
			//Make a guess that its probably a datasource
			//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
			if(!HandleObjectType(NULL, interfaceDlg.pUnknown(), interfaceDlg.GetSelInterface(), eCDataSource, 0, NULL, CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE | GetOptions()->m_dwCreateOpts)) 
				TESTC(hr = E_FAIL);
		}
	}

CLEANUP:
	TRACE_RELEASE(pIMoniker, L"IMoniker");
	TRACE_RELEASE(pIBindCtx, L"IBindCtx");
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnPromptDataSource
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnPromptDataSource(BOOL fAdvanced) 
{
	if(m_pCDataLinks && m_pCDataLinks->m_pIDBPromptInitialize)
	{
		if(fAdvanced)
		{
			//Bring up dialog to allow setting of all parameters...
			DisplayDialog(IDD_PROMPTDATASOURCE, m_hWnd, CMainWindow::PromptDataSourceProc, m_pCDataLinks);
		}
		else
		{
			//Default Operation for DataLinks is to Prompt...
			m_pCDataLinks->OnDefOperation();
		}
		
		UpdateControls();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnPromptFileName
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnPromptFileName(BOOL fAdvanced) 
{
	if(m_pCDataLinks && m_pCDataLinks->m_pIDBPromptInitialize)
	{
		if(fAdvanced)
		{
			//Bring up dialog to allow setting of all parameters...
			m_pCSource = m_pCDataLinks;
			DisplayDialog(IDD_PROMPTFILENAME, m_hWnd, CMainWindow::PromptFileNameProc, m_pCDataLinks);
		}
		else
		{
			WCHAR* pwszSelectedFile = NULL;
			
			//Just display the PromptFileName dialog directly (common case)
			if(SUCCEEDED(m_pCDataLinks->PromptFileName(
								m_hWnd,								// hWndParent
								0,									// dwPromptOptions
            					NULL,								// pwszInitialDirectory
            					NULL,								// pwszInitialFile
            					&pwszSelectedFile					// pwszSelectedFile
								)))
			{
				//Delegate, and Connect from the Selected File
				m_pCServiceComp->ConnectFromFile(pwszSelectedFile);
			}
					
			SAFE_FREE(pwszSelectedFile);
		}

		UpdateControls();
	}
}




////////////////////////////////////////////////////////////////
// CMainWindow::ErrorRecordsProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::ErrorRecordsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxLite		s_CComboRecord;
	static CComboBoxLite		s_CComboLCID;
	static ULONG	ulRecord	= 0;								//Default
	static LCID		lcid		= GetSystemDefaultLCID();			//Default
	static BOOL		fOutput		= TRUE;								//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMainWindow* pThis	= (CMainWindow*)SetThis(hWnd, (void*)lParam);
			CError* pCError		= SOURCE_GETOBJECT(pThis->m_pCSource, CError);
			ULONG cRecords = 0;
			WCHAR wszBuffer[POINTER_DISPLAYSIZE] = {0};
			HRESULT hr = S_OK;

			//Obtain the total number of error records...
			hr = pCError->GetRecordCount(&cRecords);
			if(FAILED(hr) || !cRecords)
				cRecords=0;

			//Record List...
			s_CComboRecord.CreateIndirect(hWnd, IDC_RECORD);
			for(ULONG i=0; i<cRecords; i++)
			{
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", i);
				s_CComboRecord.AddString(wszBuffer, i);
			}
			StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", ulRecord);
			s_CComboRecord.SetSelText(wszBuffer, TRUE/*fAddItem*/);

			//LCID List...
			s_CComboLCID.CreateIndirect(hWnd, IDC_LCID);
			s_CComboLCID.Populate(g_cLCID, g_rgLCID);
			if(s_CComboLCID.SetSelValue(lcid) == CB_ERR)
			{
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", lcid);
				s_CComboLCID.SetSelText(wszBuffer, TRUE/*fAddItem*/);
			}

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));
			
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow* pThis	= (CMainWindow*)GetThis(hWnd);
					CError* pCError		= SOURCE_GETOBJECT(pThis->m_pCSource, CError);
					IErrorInfo* pIErrorInfo = NULL;
					HRESULT hr = S_OK;

					//Obtain the Output (ppIUnknown) argument
					fOutput		= ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Record
					if(!GetEditBoxValue(s_CComboRecord.m_hWnd, (LONG*)&ulRecord, 0/*Min*/))
						return FALSE;
					
					//LCID
					lcid = (LCID)s_CComboLCID.GetItemParam(s_CComboLCID.GetCurSel());
					if(lcid == CB_ERR)
					{
						if(!GetEditBoxValue(s_CComboLCID.m_hWnd, (LONG*)&lcid, 0/*Min*/))
							return FALSE;
					}

					//IErrorRecords::GetErrorInfo
					hr = pCError->GetErrorInfo(ulRecord, lcid, fOutput ? &pIErrorInfo : NULL);

					//Display the ErrorInfo object returned
					if(SUCCEEDED(hr))	
						pThis->HandleObjectType(pCError, pIErrorInfo, IID_IErrorInfo, eCError, 0, NULL, 0/*dwFlags*/);
					TRACE_RELEASE(pIErrorInfo, L"IErrorInfo");
					
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMainWindow::StreamViewerProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::StreamViewerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CDialogLite			s_CDialog;
	static CRichEditLite		s_editBox;
	static CComboBoxLite		s_comboTypes;
	static CButtonLite			s_button;
	static CWebBrowser			s_webBrowser;

	static CRichEditLite		s_editOffset;
	static CRichEditLite		s_editCount;
	static LONG					s_lOffset		= 0;
	static LONG					s_cCount		= 1000;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMainWindow* pThis	= (CMainWindow*)SetThis(hWnd, (void*)lParam);
			CStream* pCStream	= SOURCE_GETOBJECT(pThis->m_pCSource, CStream);

			//Controls
			s_CDialog.CreateIndirect(hWnd);
			s_comboTypes.CreateIndirect(hWnd, IDC_BINDINGTYPE);
			s_button.CreateIndirect(hWnd, IDOK);

			s_editBox.CreateIndirect(hWnd, IDE_VALUE);
			s_editBox.SetWordWrap(TRUE);

			s_editOffset.CreateIndirect(hWnd, IDE_OFFSET);
			s_editOffset.EnableWindow();
			s_editCount.CreateIndirect(hWnd, IDE_COUNT);
			s_editCount.EnableWindow();

			//Setup the webbrowser
			s_webBrowser.CreateIndirect(s_editBox.m_hWnd);

			switch(pThis->m_idSource)
			{
				case IDM_ISEQSTREAM_READ:
					//Default text
//					s_CDialog.SetWindowText(L"ISequentialStream::Read");
//					s_button.SetWindowText(L"&Read");
				
					//Offsets are not used in ISequentialStream
					s_editOffset.EnableWindow(FALSE);
					break;

				case IDM_ISTREAM_READ:
					//Default text
					s_CDialog.SetWindowText(L"IStream::Read");
//					s_button.SetWindowText(L"&Read");
					break;

				case IDM_ISEQSTREAM_WRITE:
					s_CDialog.SetWindowText(L"ISequentialStream::Write");
					s_button.SetWindowText(L"&Write");

					//Offsets are not used in ISequentialStream
					s_editOffset.EnableWindow(FALSE);
					break;

				case IDM_ISTREAM_WRITE:
					s_CDialog.SetWindowText(L"ISequentialStream::Write");
					s_button.SetWindowText(L"&Write");
					break;

				default:
					ASSERT(!L"Unhandled Case");
					break;
			};

			//Supply Defaults to the lOffset and cRows
			wSendMessageFmt(s_editOffset.m_hWnd, WM_SETTEXT, 0, L"%ld", s_lOffset);
			wSendMessageFmt(s_editCount.m_hWnd, WM_SETTEXT, 0, L"%ld", s_cCount);
			
			//Viewer
			::CheckDlgButton(hWnd, IDB_XML_EDITBOX,		TRUE);
			::CheckDlgButton(hWnd, IDB_XML_BROWSER,		FALSE);
			::CheckDlgButton(hWnd, IDB_XML_DOM,			FALSE);

			//Initialize the Type Dropdown
			for(ULONG i=0; i<g_cDBTypes; i++)
				s_comboTypes.Populate(g_cDBTypes, g_rgDBTypes);
			s_comboTypes.SetSelValue(pCStream->m_wType);

			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch(GET_WM_COMMAND_CMD(wParam, lParam))
			{
				case CBN_SELCHANGE:
				{
					//Convert the Data to the new selected type...
					//TODO:
					break;
				}
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_XML_EDITBOX:
				{
					//Get the "this" pointer
					CMainWindow* pThis	= (CMainWindow*)GetThis(hWnd);
					CStream* pCStream	= SOURCE_GETOBJECT(pThis->m_pCSource, CStream);

					//Hide the Web Browser
					s_webBrowser.Show(OLEIVERB_HIDE);
					break;
				}

				case IDB_XML_BROWSER:
				{
					//Get the "this" pointer
					CMainWindow* pThis	= (CMainWindow*)GetThis(hWnd);
					CStream* pCStream	= SOURCE_GETOBJECT(pThis->m_pCSource, CStream);

					if(pCStream->m_pIUnknown)
					{
						//TODO: I cannot get IE (Web Browser control) to load the stream directly
						//The best that I have found is to dump the stream to a file, and then
						WCHAR* pwszFileName = L"RowsetViewer_Temp.xml";
						WCHAR wszFullPath[MAX_PATH+1] = {0};
						BOOL  fUnicodeStream = -1;
						
						DBLENGTH cbRead = 0;
						DBLENGTH lOffset = 0;
						BYTE rgBytes[MAX_BLOCK_SIZE];
						BYTE* pBytes = rgBytes;

						//Save the stream of bytes to a file
						//Open file as "CreateAlways" so we don't create numerous temp files...
						CFileLite cFile;
						XTEST(cFile.Open(pwszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS));
						
						//NOTE: The SQLXML Stream returned does not have the root node, only data.
						//So we have to output the Header/Root node ourseleves.  But we have to do it 
						//after we know weither its a unicode stream or not, since we need to place
						//the BOM

						//Write out the header
						//NOTE: Our stream has to store the type of it, since the Stream returned from
						//the provider doesn't have the BOM mark included, even for unicode stream...
						if(pCStream->m_wType == DBTYPE_WSTR)
							cFile.SetUnicode();
						cFile.Write("<RowsetViewer>");

						//Write out the stream (in chunks)
						while(SUCCEEDED(pCStream->ReadBytes(IID_IUnknown, lOffset, sizeof(rgBytes), sizeof(rgBytes), rgBytes, &cbRead)) 
							&& cbRead)
						{
							cFile.WriteBytes((ULONG)cbRead, pBytes, NULL);
							lOffset += cbRead;
						}
					
						//Write Footer
						cFile.Write("</RowsetViewer>");

						//Flush the file
						cFile.Close();

						//Now load the file - using the IE Browser
						//NOTE: IE requires a full path and fails with a relative path,
						//so formulate the full path of the current directory
						XTEST(CreateDefFileName(pwszFileName, wszFullPath, NUMELE(wszFullPath)));
						XTEST(s_webBrowser.Navigate(wszFullPath));
					}
					break;
				}

				case IDB_XML_DOM:
				{
					//Get the "this" pointer
					CMainWindow* pThis	= (CMainWindow*)GetThis(hWnd);
					CStream* pCStream	= SOURCE_GETOBJECT(pThis->m_pCSource, CStream);

					//Hide the Web Browser
					s_webBrowser.Show(OLEIVERB_HIDE);
					if(pCStream->m_pIUnknown)
					{
						CXmlDOM		xmlDOM;
						CComPtr<IXMLDOMNode>		spXMLNode;
						CStorageBuffer				cStorageBuffer;
						HRESULT hr = S_OK;

						//Create the DOM
						XTEST(hr = xmlDOM.Create());
						
						//NOTE: We need to add the root node, since the stream returned from the 
						//provider only contains the data and the DOM requires only 1 root element.
						//We can obtain the stream from the document, and manually build
						//the xml from pieces...
						DBLENGTH cbRead = 0;
						DBLENGTH lOffset = 0;
						BYTE rgBytes[MAX_BLOCK_SIZE];

						//Write the Header (not the null terminator)
						if(pCStream->m_wType == DBTYPE_WSTR)
						{
							//BOM
							XTEST(cStorageBuffer.Write(&UNICODE_BYTE_ORDER_MARK, sizeof(UNICODE_BYTE_ORDER_MARK), NULL));
							XTEST(cStorageBuffer.Write(L"<RowsetViewer>", sizeof(L"<RowsetViewer>")-sizeof(WCHAR), NULL));
						}
						else
						{
							XTEST(cStorageBuffer.Write("<RowsetViewer>", sizeof("<RowsetViewer>")-sizeof(CHAR), NULL));
						}
					
						//Dump the entire provider stream into the DOM (manually)
						//Write out the stream (in chunks)
						while(SUCCEEDED(pCStream->ReadBytes(IID_IUnknown, lOffset, sizeof(rgBytes), sizeof(rgBytes), rgBytes, &cbRead)) 
							&& cbRead)
						{
							XTEST(cStorageBuffer.Write(rgBytes, (ULONG)cbRead, NULL));
							lOffset += cbRead;
						}
						
						//Write the Footer (including the null terminator)
						if(pCStream->m_wType == DBTYPE_WSTR)
						{
							XTEST(cStorageBuffer.Write(L"</RowsetViewer>", sizeof(L"</RowsetViewer>")-sizeof(WCHAR), NULL));
						}
						else
						{
							XTEST(cStorageBuffer.Write("</RowsetViewer>", sizeof("</RowsetViewer>")-sizeof(CHAR), NULL));
						}
						cStorageBuffer.Seek(0);

						//Load the XML Stream
						XTEST(hr = xmlDOM.LoadStream((IStream*)&cStorageBuffer));

						//Walk the tree...
						if(SUCCEEDED(hr))
						{
							XTEST(xmlDOM.m_spXMLDocument.QueryInterface(&spXMLNode));
							XTEST(xmlDOM.PrintNode(spXMLNode, s_editBox));
						}
					}
					break;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow* pThis	= (CMainWindow*)GetThis(hWnd);
					CStream* pCStream	= SOURCE_GETOBJECT(pThis->m_pCSource, CStream);
					BOOL fRead = TRUE;
					IID iid = IID_NULL;
											
					switch(pThis->m_idSource)
					{
						case IDM_ISEQSTREAM_READ:
							fRead = TRUE;
							iid = IID_ISequentialStream;
							break;

						case IDM_ISTREAM_READ:
							fRead = TRUE;
							iid = IID_IStream;
							break;

						case IDM_ISEQSTREAM_WRITE:
							fRead = FALSE;
							iid = IID_ISequentialStream;
							break;

						case IDM_ISTREAM_WRITE:
							fRead = FALSE;
							iid = IID_IStream;
							break;
						
						default:
							ASSERT(!L"Unhandled Case");
							break;
					};

					//Obtain Defaults to the lOffset and cRows
					GetEditBoxValue(s_editOffset.m_hWnd, &s_lOffset);
					GetEditBoxValue(s_editCount.m_hWnd, &s_cCount, LONG_MIN/*Min*/, MAX_COL_SIZE/*Max*/);

					//Viewer
					BOOL bEditBox = ::IsDlgButtonChecked(hWnd, IDB_XML_EDITBOX);

					//Obtain the selected type to convert the stream to...
					pCStream->m_wType = (DBTYPE)s_comboTypes.GetSelValue();

					//Do we need to read or write the data?
					if(fRead)
					{
						//Read the stream into a buffer to display...
						WCHAR wszBuffer[MAX_COL_SIZE] = {0};
						if(SUCCEEDED(pCStream->ReadString(iid, 0, s_cCount, MAX_COL_SIZE, wszBuffer)))
						{
							if(bEditBox)
							{
								//Display the new data at the end
								s_editBox.SetSel(LONG_MAX, LONG_MAX);
							
								//Plase the new data at the end (ie: read in chunks)
								s_editBox.ReplaceSel(wszBuffer, TRUE);
							}
						}
					}
					else
					{
						//Obtain the data entered
						WCHAR* pwszValue = NULL;
						if(bEditBox)
						{
							pwszValue = s_editBox.GetWindowText();
						}

						//ISequentialStream::Write
						if(SUCCEEDED(pCStream->WriteString(iid, 0, pwszValue ? wcslen(pwszValue)*sizeof(WCHAR) : 0, pwszValue)))
						{
						}
					
						SAFE_FREE(pwszValue);
					}
					
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND

		case WM_NOTIFY:
		{
			NM_UPDOWN* pUpDown = (NM_UPDOWN*)lParam;
			
			//lOffset
			if(pUpDown->hdr.idFrom == IDC_SPIN_OFFSET)
			{
				s_lOffset += pUpDown->iDelta;
				wSendMessageFmt(::GetDlgItem(hWnd, IDE_OFFSET), WM_SETTEXT, 0, L"%ld", s_lOffset);
			}

			//cRows
			if(pUpDown->hdr.idFrom == IDC_SPIN_COUNT)
			{
				s_cCount += pUpDown->iDelta;
				wSendMessageFmt(::GetDlgItem(hWnd, IDE_COUNT), WM_SETTEXT, 0, L"%ld", s_cCount);
			}

			break;
		}//WM_NOTIFY
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnRootBinder
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnRootBinder()
{
	//NOTE:  Creating the root binder is deferred.
	if(!m_pCRootBinder->m_pIBindResource)
	{
		CWaitCursor waitCursor;
		m_pCRootBinder->CreateBinder(CLSID_RootBinder);
	}

	//Not only create the object so it ends up in the tree, but actually
	//automatically bring up the dialog, so its not always a 2 step process...
	m_pCRootBinder->OnDefOperation();
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnRootEnumerator
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnRootEnumerator()
{
	//NOTE:  Creating the Root Enumerator is deferred.
	if(!m_pCRootEnumerator->m_pISourcesRowset)
	{
		CWaitCursor waitCursor;
		m_pCRootEnumerator->Create(CLSID_OLEDB_ENUMERATOR);
	}

	//Not only create the object so it ends up in the tree, but actually
	//automatically bring up the dialog, so its not always a 2 step process...
	m_pCRootEnumerator->OnDefOperation();
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnServiceComponents
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnServiceComponents()
{
	//NOTE:  Creating Service Components is deferred.
	if(!m_pCServiceComp->m_pIDataInitialize)
	{
		CWaitCursor waitCursor;
		m_pCServiceComp->Create(NULL);
	}

	//Not only create the object so it ends up in the tree, but actually
	//automatically bring up the dialog, so its not always a 2 step process...
	OnGetDataSource();
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnDataLinks
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnDataLinks()
{
	//NOTE:  Creating Service Components is deferred.
	if(!m_pCDataLinks || !m_pCDataLinks->m_pIDBPromptInitialize)
	{
		//Deferred Object Creation
		if(!m_pCDataLinks)
			m_pCDataLinks = new CDataLinks(this);
					
		CWaitCursor waitCursor;
		m_pCDataLinks->Create(NULL);
	}

	//Not only create the object so it ends up in the tree, but actually
	//automatically bring up the dialog, so its not always a 2 step process...
	m_pCDataLinks->OnDefOperation();
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnLoadRecentConfig
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnLoadRecentConfig(UINT iID)
{
	if(m_pCFullConnect)
		m_pCFullConnect->LoadRecentConfig(iID);
}

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnLoadRecentFile
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnLoadRecentFile(UINT iID)
{
	if(m_pCFullConnect)
		m_pCFullConnect->LoadRecentFile(iID);
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnOptions
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnOptions()
{
	if(m_pCFullConnect)
	{
		//Bring up the Property Sheet, and save if user pressed APPLY (OK)
		if(m_pCOptionsSheet->DoModal(m_hWnd) == IDOK)
		{
			CWaitCursor waitCursor;
			m_pCOptionsSheet->SaveOptions();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnTraceWindow
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnTraceWindow()
{
	//Is the trace window already active?
	CWndLite* pCWndLite = FindWindow(L"MDITRACE");
	if(pCWndLite)
	{
		//Just activate the existing window...
		MDIActivate(pCWndLite->m_hWnd);
		pCWndLite->ShowWindow(SW_SHOWNORMAL);
	}
	else
	{
		//Create and Display the Window...
		m_pCMDITrace->Create(m_hWndMDIClient, L"MDITRACE", L"Output", IDR_ROWSETVIEWER, ImageList_GetIcon(m_hImageList, IMAGE_FORM, ILD_NORMAL));
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnObjectsWindow
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnObjectsWindow()
{
	//Is the Objects window already active?
	CWndLite* pCWndLite = FindWindow(L"MDIOBJECTS");
	if(pCWndLite)
	{
		//Just activate the existing window...
		MDIActivate(pCWndLite->m_hWnd);
		pCWndLite->ShowWindow(SW_SHOWNORMAL);
	}
	else
	{
		//Create and Display the Window...
		m_pCMDIObjects->Create(m_hWndMDIClient, L"MDIOBJECTS", L"Objects", IDR_ROWSETVIEWER, ImageList_GetIcon(m_hImageList, IMAGE_OBJECTS, ILD_NORMAL));
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnDisconnect
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnDisconnect()
{
	CWaitCursor waitCursor;
	
	//Disconnect and destroy this window
	CMDIChild* pCMDIChild = (CMDIChild*)GetActiveWindow(L"MDICHILD");
	if(pCMDIChild)
	{
		//Tell the window to destroy itself
		MDIDestroy(pCMDIChild->m_hWnd);
		SAFE_DELETE(pCMDIChild);
		UpdateControls();
	}
}
					

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnTile
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnTile(BOOL fHorizontal)
{
	//let MDI Client tile the MDI children
	SendMessage(m_hWndMDIClient, WM_MDITILE, fHorizontal ? MDITILE_HORIZONTAL : MDITILE_VERTICAL, 0);
}


/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnCascade
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnCascade()
{
	// let MDI Client cascade MDI children
	SendMessage(m_hWndMDIClient, WM_MDICASCADE, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnArrangeIcons
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnArrangeIcons()
{
	// let MDI Client arrange iconic MDI children
	SendMessage(m_hWndMDIClient, WM_MDIICONARRANGE, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnCloseAll
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnCloseAll()
{
	CWaitCursor waitCursor;

	RemoveAllChildren();
	UpdateControls();
}

/////////////////////////////////////////////////////////////////////////////
// CMainWindow::OnNextWindow
//
/////////////////////////////////////////////////////////////////////////////
void CMainWindow::OnNextWindow(BOOL fNextWindow)
{
	CWndLite* pCWndLite = GetActiveWindow();
	if(pCWndLite)
		SendMessage(m_hWndMDIClient, WM_MDINEXT, (WPARAM)pCWndLite->m_hWnd, fNextWindow ? 1 : 0);
}


////////////////////////////////////////////////////////////////
// CMainWindow::BindResourceProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::BindResourceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid		s_CComboInterface;
	static INDEX				iSavedType			= 3;  //DBGUID_ROW
	static DWORD				dwSavedBindFlags	= DBBINDURLFLAG_READ;  
	static BOOL fAggregation	= FALSE;			//Default
	static BOOL fOutput			= TRUE;				//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMainWindow* pThis		= (CMainWindow*)SetThis(hWnd, (void*)lParam);

			INDEX iSel = 0;
			HWND hWndURL		= ::GetDlgItem(hWnd, IDE_URL);
			HWND hWndObjectType	= ::GetDlgItem(hWnd, IDC_OBJECTTYPE);
			HWND hWndBindFlags	= ::GetDlgItem(hWnd, IDL_BINDFLAGS);
			HWND hWndProperties	= ::GetDlgItem(hWnd, IDB_SETPROPERTIES);
			
			//Set the Title (since this is shared with ICreateRow)
			if(pThis->m_idSource == IDM_ICREATEROW_CREATEROW)
			{
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"ICreateRow::CreateRow");
				SendMessage(::GetDlgItem(hWnd, IDOK), WM_SETTEXT, 0, (LPARAM)"CreateRow");
			}

			//Enable Properties
			switch(pThis->m_pCSource->GetObjectType())
			{
				case eCRow:
				case eCSession:
					::EnableWindow(hWndProperties, FALSE);
					break;
			}

			//Fill In URL default
			wSendMessage(hWndURL, WM_SETTEXT, 0, pThis->m_pCRootBinder->m_pwszURL);

			ULONG i;
			//Fill In ObjectType drop down
			for(i=0; i<g_cObjectTypeMaps; i++)
			{
				iSel = (INDEX)wSendMessage(hWndObjectType, CB_ADDSTRING, 0, g_rgObjectTypeMaps[i].pwszName);
				SendMessage(hWndObjectType, CB_SETITEMDATA, iSel, (LPARAM)&g_rgObjectTypeMaps[i]);
			}
			SendMessage(hWndObjectType, CB_SETCURSEL, iSavedType, 0);

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IRow);
			
			//BindFlags List...
			SendMessage(hWndBindFlags, LB_RESETCONTENT, 0, 0);
			for(i=0; i<g_cBindURLMaps; i++)
			{
				//BindFlags Name
				iSel = (INDEX)wSendMessage(hWndBindFlags, LB_ADDSTRING,	0, g_rgBindURLMaps[i].pwszName);
				SendMessage(hWndBindFlags, LB_SETITEMDATA, iSel, (LPARAM)g_rgBindURLMaps[i].lItem);

				//Select Saved BindFlags
				if((dwSavedBindFlags & g_rgBindURLMaps[i].lItem) == (DWORD)g_rgBindURLMaps[i].lItem)
					 SendMessage(hWndBindFlags, LB_SETSEL, TRUE, i);
			}
			
			//Allow the horizontal scroll bar
			SendMessage(hWndBindFlags, LB_SETHORIZONTALEXTENT, 500, 0);
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_SETPROPERTIES:
				{
					//Get the "this" pointer
					CMainWindow* pThis				= (CMainWindow*)GetThis(hWnd);
					IDBProperties* pIDBProperties	= SOURCE_GETINTERFACE(pThis->m_pCSource, IDBProperties);

					//SetProperties
					CPropertiesDlg sCPropertiesDlg(pThis);
					sCPropertiesDlg.SetProperties(hWnd, NULL/*No Default PropSet*/, IID_IDBProperties, pIDBProperties, pIDBProperties);
					return 0;
				}

				case IDB_AGGREGATION:
				{
					//Aggregation Combo Selection has changed...
					//If we are now using Aggregation, automatically change the requested
					//riid to IID_IUnknown, since its an error otherwise...
					if(::IsDlgButtonChecked(hWnd, IDB_AGGREGATION))
						s_CComboInterface.SetGuid(IID_IUnknown);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow* pThis		= (CMainWindow*)GetThis(hWnd);
					IUnknown* pIUnknown		= NULL;

					HWND hWndURL		= ::GetDlgItem(hWnd, IDE_URL);
					HWND hWndObjectType	= ::GetDlgItem(hWnd, IDC_OBJECTTYPE);
					HWND hWndBindFlags	= ::GetDlgItem(hWnd, IDL_BINDFLAGS);
					BOOL dwWindowFlags	= CREATE_NEWWINDOW;
					HRESULT hr = S_OK;
					
					DBBINDURLSTATUS dwBindStatus = 0;
					WCHAR* pwszNewURL = NULL;

					//URL
					SAFE_FREE(pThis->m_pCRootBinder->m_pwszURL);
					pThis->m_pCRootBinder->m_pwszURL = wGetWindowText(hWndURL);

					//ObjectType
					iSavedType = (INDEX)SendMessage(hWndObjectType, CB_GETCURSEL, 0, 0);
					WIDEGUIDMAP* pObjectMap = (WIDEGUIDMAP*)SendMessage(hWndObjectType, CB_GETITEMDATA, iSavedType, 0);
					GUID guidObjectType = *pObjectMap->pGuid;

					//Obtain the Aggregation argument
					CAggregate* pCAggregate = NULL;
					fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
					if(fAggregation)
						pCAggregate = new CAggregate();
					
					//Obtain the Output (ppIUnknown) argument
					fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Interface
					REFIID riid = s_CComboInterface.GetGuid();

					//BindFlags
					//Obtain all Selected BindFlags...
					INDEX iSelCount = (INDEX)SendMessage(hWndBindFlags, LB_GETSELCOUNT, 0, 0);
					ASSERT(iSelCount < 20);
					LONG rgSelItems[20];
					SendMessage(hWndBindFlags, LB_GETSELITEMS, (WPARAM)20, (LPARAM)rgSelItems);

					dwSavedBindFlags = 0;
					for(LONG i=0; i<iSelCount; i++)
						dwSavedBindFlags |= SendMessage(hWndBindFlags, LB_GETITEMDATA, rgSelItems[i], 0);
					
					switch(pThis->m_idSource)
					{
						case IDM_ICREATEROW_CREATEROW:
						{
							ICreateRow* pICreateRow = SOURCE_GETINTERFACE(pThis->m_pCSource, ICreateRow);
							ASSERT(pICreateRow);

							//ICreateRow::CreateRow
							XTEST(hr = pICreateRow->CreateRow(pCAggregate, pThis->m_pCRootBinder->m_pwszURL, dwSavedBindFlags, guidObjectType,  riid, NULL, NULL, &dwBindStatus, &pwszNewURL, fOutput ? &pIUnknown : NULL));
							TESTC(TRACE_METHOD(hr, L"ICreateRow::CreateRow(0x%p, \"%s\", 0x%08x, %s, %s, NULL, NULL, &0x%p, &\"%s\", &0x%p)", pCAggregate, pThis->m_pCRootBinder->m_pwszURL, dwSavedBindFlags, GetObjectTypeName(guidObjectType), GetInterfaceName(riid), &dwBindStatus, pwszNewURL, pIUnknown));
							break;
						}
					
						case IDM_IBINDRESOURCE_BIND:
						{
							IBindResource* pIBindResource = SOURCE_GETINTERFACE(pThis->m_pCSource, IBindResource);
							ASSERT(pIBindResource);

							//IBindResource::Bind
							XTEST(hr = pIBindResource->Bind(pCAggregate, pThis->m_pCRootBinder->m_pwszURL, dwSavedBindFlags, guidObjectType,  riid, NULL, NULL, &dwBindStatus, fOutput ? &pIUnknown : NULL));
							TESTC(TRACE_METHOD(hr, L"IBindResource::Bind(0x%p, \"%s\", 0x%08x, %s, %s, NULL, NULL, &0x%p, &0x%p)", pCAggregate, pThis->m_pCRootBinder->m_pwszURL, dwSavedBindFlags, GetObjectTypeName(guidObjectType), GetInterfaceName(riid), &dwBindStatus, pIUnknown));
						}
							break;

						default:
							ASSERT(!"Unhandled Type!");
							break;					
					};

					//Do we create in a new window?  If this is called from a session object
					//and we are interested in a DSO, or Session, then just use the same 
					//Window, since Bind returns the same existing object addref'd
					if(pThis->m_pCSource->GetObjectType() == eCSession && (guidObjectType == DBGUID_DSO || guidObjectType == DBGUID_SESSION))
						dwWindowFlags = CREATE_FINDWINDOW;

					//Handle Aggregation
					if(pCAggregate)
						pCAggregate->HandleAggregation(riid, &pIUnknown);

					//Handle the returned object type...
					//All bind objects are by default returned in there own window...
					//NOTE: We already know the object type, or eCUnknown is passed
					if(!pThis->HandleObjectType(pThis->m_pCSource, pIUnknown, riid, GuidToSourceType(guidObjectType), 0, NULL, dwWindowFlags)) 
						TESTC(hr = E_FAIL);
					
				CLEANUP:
					SAFE_RELEASE(pCAggregate);
					SAFE_RELEASE(pIUnknown);

					SAFE_FREE(pwszNewURL);
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMainWindow::RegisterProviderProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::RegisterProviderProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMainWindow* pThis		= (CMainWindow*)SetThis(hWnd, (void*)lParam);
			CEnumerator* pCRootEnum		= pThis->m_pCRootEnumerator;

			INDEX iSel				= 0;
			HWND hWndURL			= ::GetDlgItem(hWnd, IDE_URL);
			HWND hWndProvider		= ::GetDlgItem(hWnd, IDC_PROVIDER);

			//Update the Window title, since this is called from either method of IRegisterProvider
			if(pThis->m_idSource == IDM_IREGISTERPROVIDER_GETURLMAPPING)
			{
				wSendMessage(hWnd, WM_SETTEXT, 0, L"IRegisterProvider::GetURLMapping");
			}
			else if(pThis->m_idSource == IDM_IREGISTERPROVIDER_UNREGISTERPROVIDER)
			{
				wSendMessage(hWnd, WM_SETTEXT, 0, L"IRegisterProvider::UnregisterProvider");
			}

			//Fill In URL default
			//TODO can we provide a better default?
			wSendMessage(hWndURL, WM_SETTEXT, 0, L"http://");

			//GetURLMapping, output is CLSID, not input...
			if(pThis->m_idSource == IDM_IREGISTERPROVIDER_GETURLMAPPING)
			{
				::EnableWindow(hWndProvider, FALSE);
				SendMessage(hWndProvider, WM_SETTEXT, 0, (LPARAM)"");
			}
			else
			{
				//Do we need to connect to the root enumerator...
				pCRootEnum->CreateEnumInfo(CLSID_OLEDB_ENUMERATOR);
				
				//Fill In Provider drop down
				for(ULONG i=0; i<pCRootEnum->m_cEnumInfo; i++)
				{
					iSel = (INDEX)wSendMessage(hWndProvider, CB_ADDSTRING, 0, pCRootEnum->m_rgEnumInfo[i].wszName);
					SendMessage(hWndProvider, CB_SETITEMDATA, iSel, (LPARAM)&pCRootEnum->m_rgEnumInfo[i]);
				}
				//TODO can we provide a better default?
				SendMessage(hWndProvider, CB_SETCURSEL, 0, 0);
			}

			
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow* pThis		= (CMainWindow*)GetThis(hWnd);

					HWND hWndURL			= ::GetDlgItem(hWnd, IDE_URL);
					HWND hWndProvider		= ::GetDlgItem(hWnd, IDC_PROVIDER);
					WCHAR	wszProgID[MAX_NAME_LEN] = {0};
					WCHAR*	pwszURL = NULL;
					WCHAR*	pwszProgID = NULL;
					CLSID clsid = GUID_NULL;
					HRESULT hr = S_OK;
					
					//Currently this is only for Provider Binder callers...
					CBinder* pCBinder = SOURCE_GETOBJECT(pThis->m_pCSource, CBinder);
					ASSERT(pCBinder);

					//URL
					pwszURL = wGetWindowText(hWndURL);

					//Provider
					wSendMessage(hWndProvider, WM_GETTEXT, MAX_NAME_LEN, wszProgID);
					CLSIDFromProgID(wszProgID, &clsid);

					if(pThis->m_idSource == IDM_IREGISTERPROVIDER_SETURLMAPPING)
					{
						//IRegisterProvider::SetURLMapping
						XTEST(hr = pCBinder->m_pIRegisterProvider->SetURLMapping(pwszURL, 0, clsid));
						TESTC(TRACE_METHOD(hr, L"IRegisterProvider::SetURLMapping(\"%s\", 0x%08x, \"%s\")", pwszURL, 0, wszProgID));
					}
					else if(pThis->m_idSource == IDM_IREGISTERPROVIDER_GETURLMAPPING)
					{
						//IRegisterProvider::GetURLMapping
						XTEST(hr = pCBinder->m_pIRegisterProvider->GetURLMapping(pwszURL, 0, &clsid));
						pwszProgID = GetProgID(clsid);
						TESTC(TRACE_METHOD(hr, L"IRegisterProvider::GetURLMapping(\"%s\", 0x%08x, \"%s\")", pwszURL, 0, pwszProgID));
					}
					else
					{
						//IRegisterProvider::UnregisterProvider
						//NOTE:  NULL has special allowed meaning in unregister provider, so if the 
						//edit box contains empty we will treat it as null...
						XTEST(hr = pCBinder->m_pIRegisterProvider->UnregisterProvider(pwszURL, 0, clsid));
						TESTC(TRACE_METHOD(hr, L"IRegisterProvider::UnregisterProvider(\"%s\", 0x%08x, \"%s\")", pwszURL, 0, wszProgID));
					}
					
				CLEANUP:
					SAFE_FREE(pwszProgID);
					SAFE_FREE(pwszURL);

					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMainWindow::GetSourcesRowsetProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::GetSourcesRowsetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid	s_CComboInterface;
	static CPropSets		s_CPropSets;
	
	static BOOL fUseProps		= TRUE;			//Default
	static BOOL fAggregation	= FALSE;		//Default
	static BOOL fOutput			= TRUE;			//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMainWindow* pThis = (CMainWindow*)SetThis(hWnd, (void*)lParam);

			//Use Properties
			::CheckDlgButton(hWnd, IDB_USEPROPERTIES,	BST2STATE(fUseProps));

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Only set these "Default" properties, if requested by the user
			if(pThis->GetOptions()->m_dwRowsetOpts & ROWSET_SETDEFAULTPROPS)
			{
				//DBPROP_CANHOLDROWS is required by the OLE DB Spec - Level-0 Conformance
				//Since it is also legal to set a ReadOnly property, just blindy set it...
				s_CPropSets.SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBTYPE_BOOL, (void*)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);
			}

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IRowset);

			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_SETPROPERTIES:
				{
					//Get the "this" pointer
					CMainWindow* pThis			= (CMainWindow*)GetThis(hWnd);
					CEnumerator* pCEnumerator	= SOURCE_GETPARENT(pThis->m_pCSource, CEnumerator);

					CPropertiesDlg sCPropertiesDlg(pThis);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_IRowsetInfo, NULL, pCEnumerator ? pCEnumerator->m_pIDBProperties : NULL, &s_CPropSets);
					return 0;
				}

				case IDB_AGGREGATION:
				{
					//Aggregation Combo Selection has changed...
					//If we are now using Aggregation, automatically change the requested
					//riid to IID_IUnknown, since its an error otherwise...
					if(::IsDlgButtonChecked(hWnd, IDB_AGGREGATION))
						s_CComboInterface.SetGuid(IID_IUnknown);
					return 0;
				}
				
				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow* pThis			= (CMainWindow*)GetThis(hWnd);
					CEnumerator* pCEnumerator	= SOURCE_GETOBJECT(pThis->m_pCSource, CEnumerator);
					CMDIChild*	 pCMDIChild		= pCEnumerator ? pCEnumerator->m_pCMDIChild : NULL;

					HRESULT hr = S_OK;
					IUnknown* pIUnknown = NULL;

					ULONG cPropSets = 0;
					DBPROPSET* rgPropSets = NULL;

					//Obtain the Aggregation argument
					CAggregate* pCAggregate = NULL;
					fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
					if(fAggregation)
						pCAggregate = new CAggregate();
					
					//Obtain the Output (ppIUnknown) argument
					fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Interface
					REFIID riid = s_CComboInterface.GetGuid();
					
					//Use Properties
					fUseProps = ::IsDlgButtonChecked(hWnd, IDB_USEPROPERTIES);
					if(fUseProps)
					{
						cPropSets = s_CPropSets.GetCount();
						rgPropSets = s_CPropSets.GetPropSets();
					}

					//ISourcesRowset::GetSourcesRowset
					hr = pCEnumerator->GetSourcesRowset(pCAggregate, riid, cPropSets, rgPropSets, fOutput ? &pIUnknown : NULL);
					
					//Process the Rowset
					if(SUCCEEDED(hr))
					{
						//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
						if(!pThis->HandleObjectType(pCEnumerator, pIUnknown, riid, eCRowset, 0, NULL, pCMDIChild ? CREATE_NEWWINDOW_IFEXISTS | CREATE_DETERMINE_TYPE : CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE))
							hr = E_FAIL;
					}

					SAFE_RELEASE(pIUnknown);
					SAFE_RELEASE(pCAggregate);
					
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;
}




////////////////////////////////////////////////////////////////
// CMainWindow::GetDataSourceProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::GetDataSourceProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid	s_CComboInterface;
	static CInitStringBox	s_CInitStringBox;
	static BOOL	fUseExistingDSO		= FALSE;
	static BOOL fAggregation		= FALSE;			//Default
	static BOOL fOutput				= TRUE;				//Default

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMainWindow* pThis				= (CMainWindow*)SetThis(hWnd, (void*)lParam);
			CServiceComp*  pCServiceComp	= SOURCE_GETOBJECT(pThis->m_pCSource, CServiceComp);
			ASSERT(pCServiceComp);

			//Set the Saved InitString
			s_CInitStringBox.CreateIndirect(hWnd, IDE_INITSTRING);
			s_CInitStringBox.SetWindowText(pCServiceComp->m_pwszInitString);
			s_CInitStringBox.SubClassWindow(WndProc);

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IDBInitialize);

			//Existing DSO
			::CheckDlgButton(hWnd, IDB_EXISTING_DSO, BST2STATE(fUseExistingDSO));
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
				break;

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_OPTIONS:
				{
					//Get the "this" pointer
					CMainWindow* pThis = (CMainWindow*)GetThis(hWnd);

					//Bring up the Main Window Options
					pThis->OnOptions();
					return 0;
				}

				case IDB_AGGREGATION:
				{
					//Aggregation Combo Selection has changed...
					//If we are now using Aggregation, automatically change the requested
					//riid to IID_IUnknown, since its an error otherwise...
					if(::IsDlgButtonChecked(hWnd, IDB_AGGREGATION))
						s_CComboInterface.SetGuid(IID_IUnknown);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow*	pThis			= (CMainWindow*)GetThis(hWnd);
					IUnknown*		pIUnknown		= NULL;
					HRESULT hr = S_OK;

					CServiceComp*  pCServiceComp	= SOURCE_GETOBJECT(pThis->m_pCSource, CServiceComp);
					ASSERT(pCServiceComp);

					//Get the InitString from the EditBox
					SAFE_FREE(pCServiceComp->m_pwszInitString);
					pCServiceComp->m_pwszInitString = s_CInitStringBox.GetWindowText();
										
					//Obtain the Aggregation argument
					CAggregate* pCAggregate = NULL;
					fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
					if(fAggregation)
						pCAggregate = new CAggregate();
					
					//Obtain the Output (ppIUnknown) argument
					fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Interface
					REFIID riid = s_CComboInterface.GetGuid();

					//Obtain the correct MDI Child
					CMDIChild* pCMDIChild = (CMDIChild*)pThis->FindWindow(L"MDICHILD");

					//Existing DataSource
					fUseExistingDSO = ::IsDlgButtonChecked(hWnd, IDB_EXISTING_DSO);
					if(fUseExistingDSO && pCMDIChild && fOutput)
					{
						//Release is called on input, for successful cases...
						//I don't want them to release my DataSource, so AddRef...
						pIUnknown = SOURCE_GETINTERFACE(pCMDIChild->GetObject(eCDataSource), IUnknown);
						SAFE_ADDREF(pIUnknown);
					}

					//GetDataSource - with the specified InitString
					//NOTE:  The release of the input/ouput datasource is a likely tricky
					// 1.  *ppIUnknown == NULL on input - Succeeded	- output 1 reference
					// 2.  *ppIUnknown == NULL on input - Failed	- output NULL
					// 3.  *ppIUnknown != NULL on input - Succeeded - input is released, and output is QI'd. (in effect refcount has not changed)
					// 4.  *ppIUnknown != NULL on input - Failed	- input is untouched, output is NULL. (in effect refcount has not changed)
					//Although this seems a little odd, its entirly optimized for the consumer
					//All we have to do is AddRef the input and always release the output 
					TESTC(hr = pCServiceComp->GetDataSource(pCAggregate, pThis->GetOptions()->m_dwCLSCTX, pCServiceComp->m_pwszInitString, riid, fOutput ? &pIUnknown : NULL));
					
					//Handle the returned object...
					//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
					if(!pThis->HandleObjectType(pCServiceComp, pIUnknown, riid, eCDataSource, 0, NULL, CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE | pThis->GetOptions()->m_dwCreateOpts))
						TESTC(hr = E_FAIL);
				
				CLEANUP:
					SAFE_RELEASE(pCAggregate);
					SAFE_RELEASE(pIUnknown);
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDB_BROWSE_PROVIDER:
				{
					CMainWindow* pThis = (CMainWindow*)GetThis(hWnd);
					pThis->DisplayDialog(IDD_SIMPLE_LISTBOX, hWnd, CMainWindow::EnumPropertiesProc, pThis->m_pCSource, pThis->m_idSource);
					return 0;
				}
					
				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMainWindow::EnumPropertiesProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::EnumPropertiesProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CDSLContextBox		s_CDSLContextBox;
	static CRichEditLite		s_CEditSource;

	switch(message)
	{
		case WM_INITDIALOG:
		{	
			CWaitCursor waitCursor;

			//save off the this pointer
			CMainWindow* pThis			= (CMainWindow*)SetThis(hWnd, (void*)lParam);
			HRESULT hr = S_OK;

			IDBProperties* pIDBProperties = NULL;
			ULONG iPropSet,cPropInfoSets = 0;
			DBPROPINFOSET* rgPropInfoSets = NULL;
			WCHAR* pStringBuffer = NULL;
			IUnknown* pIUnknown = NULL;

			INDEX iStart = 0;
			INDEX iEnd = 0;
			RECT rect, rectDlg;
			POINT pt = {0, 0};
			
			//Create the ListBox
			s_CDSLContextBox.CreateIndirect(hWnd, IDL_VALUES);
			s_CDSLContextBox.SubClassWindow(WndProc);

			//Obtain the currently entered connection string
			//NOTE: We don't subclass the initstring here, since we have already done it 
			//in the caller function - GetDataSourceProc
			s_CEditSource.CreateIndirect(GetParent(hWnd), IDE_INITSTRING);
			WCHAR* pwszInitString = s_CEditSource.GetWindowText();
			BOOL fDisplayedInfo = FALSE;

			if(pwszInitString)
			{
				CHAR szBuffer[MAX_NAME_LEN] = {0};

				//Obtain the current cursor location, to determine what info the user really wants...
				s_CEditSource.GetSel(&iStart, &iEnd);
				if(iStart)
				{
					//End is limited to our buffer size...
					iEnd = min(iEnd, MAX_NAME_LEN-1);
					
					//If the last character was an = sign, then we probably want a list of providers.
					CHARRANGE cr = { 0, (LONG)iEnd }; 
					s_CEditSource.GetTextRange(cr, szBuffer);
				}
				
				//Value Context
				if(szBuffer[iEnd ? iEnd-1 : 0] == '=')
				{
					//Provider Enumerator?
					if( (iEnd >= 12 /*"Data Source"*/ && (_stricmp(&szBuffer[iEnd-12], "Data Source=")==0 || _stricmp(&szBuffer[iEnd-12], "data Source=")==0)) || 
						(iEnd >= 4 /*"DSN"*/ && (_stricmp(&szBuffer[iEnd-4], "DSN=")==0 || _stricmp(&szBuffer[iEnd-4], "dsn=")==0)) )
					{
						//Root Enumerator
						CEnumerator* pCRootEnum = pThis->m_pCRootEnumerator;
					
						//We may need to connect to the RootEnumerator, if not done already
						pCRootEnum->CreateEnumInfo(CLSID_OLEDB_ENUMERATOR);
						
						//Fill out the ListBox
						for(ULONG i=0; i<pCRootEnum->m_cEnumInfo; i++)
						{
							ENUMINFO* pEnumInfo = &pCRootEnum->m_rgEnumInfo[i];
							if(pEnumInfo->eType ==  DBSOURCETYPE_ENUMERATOR)
							{
								WCHAR wszSubName[10] = {0};
								wcsncpy_s(wszSubName, NUMELE(wszSubName),pEnumInfo->wszName, 7);
								

								//See if anything in the string assoicates it with this enumerator...
								//If no provider is specified, the default provider is MSDASQL so use that enumerator
								if(wcsstr(pwszInitString, wszSubName) || 
									(wcsstr(pwszInitString, L"Provider")==NULL && wcsstr(pwszInitString, L"provider")==NULL && StringCompare(pEnumInfo->wszName, L"MSDASQL Enumerator")) )
								{
									CEnumerator cEnum(pThis);

									//Create this enumerator.
									TESTC(hr = pCRootEnum->ParseDisplayName(pEnumInfo->wszParseName, CLSCTX_INPROC_SERVER, IID_IUnknown, &pIUnknown, NULL));
									TESTC(hr = cEnum.CreateObject(pCRootEnum, IID_IUnknown, pIUnknown));
									TESTC(hr = cEnum.CreateEnumInfo());

									//Now display the enuminfo
									for(ULONG iEnum=0; iEnum<cEnum.m_cEnumInfo; iEnum++)
									{
										//Add the name to the list
										s_CDSLContextBox.AddString(cEnum.m_rgEnumInfo[iEnum].wszName);
									}
									fDisplayedInfo = TRUE;
									break;
								}
							}
						}
					}
					//Catalog?
					else if(iEnd >= 8 /*"Catalog"*/ && (_stricmp(&szBuffer[iEnd-8], "Catalog=")==0 || _stricmp(&szBuffer[iEnd-8], "catalog=")==0) )
					{
						CDataSource cDataSource(pThis);
						CSession cSession(pThis);
						
						//Obtain the current provider...
						//If the user has typed enough, (Provider=) then they will get that provider
						//otherwise DSL defaults to MSDASQL
						TESTC(hr = pThis->m_pCServiceComp->GetDataSource(NULL, pThis->GetOptions()->m_dwCLSCTX, pwszInitString, IID_IDBProperties, (IUnknown**)&pIDBProperties));
						TESTC(hr = cDataSource.CreateObject(pThis->m_pCServiceComp, IID_IDBProperties, pIDBProperties));
						TESTC(hr = cDataSource.Initialize());

						//Create Session
						XTESTC(hr = cDataSource.CreateSession(NULL, IID_IUnknown, &pIUnknown));
						TESTC(hr = cSession.CreateObject(&cDataSource, IID_IUnknown, pIUnknown));

						//Obtain SchemaInfo
						if(cSession.m_pIDBSchemaRowset)
						{
							SAFE_RELEASE(pIUnknown);
							WCHAR wszBuffer[MAX_NAME_LEN];
							HACCESSOR hAccessor = NULL;
							DBCOUNTITEM cRowsObtained = 0;
							HROW rghRows[MAX_BLOCK_SIZE];
							HROW* phRows = rghRows;
							CRowset cRowset(pThis);

							//DBSCHEMA_CATALOGS
							TESTC(hr = cSession.GetSchemaRowset(NULL, DBSCHEMA_CATALOGS, 0, NULL, IID_IUnknown, 0, NULL, &pIUnknown));
							TESTC(hr = cRowset.CreateObject(&cSession, IID_IUnknown, pIUnknown));

							//Bind only the CATALOG_NAME column...
							const static DBCOUNTITEM cBindings = 1;
							DBBINDING rgBindings[cBindings] = 
							{
									1,		//iOrinal
									0,		//pData = phChapter + 0 offset = phChapter
									0,
									0,	
									NULL,			
									NULL, 		
									NULL,		
									DBPART_VALUE,
									DBMEMOWNER_CLIENTOWNED,		
									DBPARAMIO_NOTPARAM, 
									sizeof(wszBuffer),
									0, 				
									DBTYPE_WSTR,
									0,	
									0, 				
							};

							//Create the Accessor...
							TESTC(hr = cRowset.CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &hAccessor));

							//Loop through all the rows...
							while(TRUE)
							{
								hr = cRowset.GetNextRows(0, MAX_BLOCK_SIZE, &cRowsObtained, &phRows);
								if(FAILED(hr) || cRowsObtained==0) 
									break;
								
								//Loop over rows obtained...
								for(ULONG i=0; i<cRowsObtained; i++) 
								{	
									//Get the Data
									wszBuffer[0] = wEOL;	//Since were not binding status...
									TESTC(hr = cRowset.GetData(rghRows[i], hAccessor, wszBuffer));
								
									//Add the name to the list
									s_CDSLContextBox.AddString(wszBuffer);
									fDisplayedInfo = TRUE;
								}
									
								//Release all the rows
								TESTC(hr = cRowset.ReleaseRows(cRowsObtained, rghRows));
							}

							//Release Accessor
							TESTC(cRowset.ReleaseAccessor(&hAccessor));
						}
					}
					//Root Enumerator?
					else if(iEnd >= 9 /*"Provider"*/ && (_stricmp(&szBuffer[iEnd-9], "Provider=")==0 || _stricmp(&szBuffer[iEnd-9], "provider=")==0) )
					{
						//Root Enumerator
						CEnumerator* pCRootEnum = pThis->m_pCRootEnumerator;
					
						//We may need to connect to the RootEnumerator, if not done already
						pCRootEnum->CreateEnumInfo(CLSID_OLEDB_ENUMERATOR);
											
						//Fill out the ListBox
						for(ULONG i=0; i<pCRootEnum->m_cEnumInfo; i++)
						{
							//Add the name to the list
							s_CDSLContextBox.AddString(pCRootEnum->m_rgEnumInfo[i].wszName);
						}
						fDisplayedInfo = TRUE;
					}
					else
					{
						//Default Values?
						s_CDSLContextBox.AddString(L";");
						s_CDSLContextBox.AddString(L"True");
						s_CDSLContextBox.AddString(L"False");
						fDisplayedInfo = TRUE;
					}
				}
				//Not a value context
				else
				{
					//Property Description Context?
					if(szBuffer[iEnd ? iEnd-1 : 0] == ' ' || szBuffer[iEnd ? iEnd-1 : 0] == ';')
					{
						//Obtain the current provider...
						//If the user has typed enough, (Provider=) then they will get that provider
						//otherwise DSL defaults to MSDASQL
						TESTC(hr = pThis->m_pCServiceComp->GetDataSource(NULL, pThis->GetOptions()->m_dwCLSCTX, pwszInitString, IID_IDBProperties, (IUnknown**)&pIDBProperties));

						//Obtain all property descriptions...
						TESTC(hr = pIDBProperties->GetPropertyInfo(0, NULL, &cPropInfoSets, &rgPropInfoSets, &pStringBuffer));

						//Loop through and display properties...
						for(iPropSet=0; iPropSet<cPropInfoSets; iPropSet++)
						{
							DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[iPropSet];
							for(ULONG iProp=0; iProp<pPropInfoSet->cPropertyInfos; iProp++)
							{
								//Add this property to the list...
								DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[iProp];
								s_CDSLContextBox.AddString(pPropInfo->pwszDescription);
							}
						}
						fDisplayedInfo = TRUE;
					}
				}
			}
			
			if(!fDisplayedInfo)
			{
				//No InitString, so just populate listbox with special reserved words for DSL
				s_CDSLContextBox.AddString(L"Provider=");
				s_CDSLContextBox.AddString(L"=");
				s_CDSLContextBox.AddString(L";");
				s_CDSLContextBox.AddString(L"\"");
				s_CDSLContextBox.AddString(L"'");
			}

			//Obain the current cursor location, where to place the dialog...
			GetWindowRect(s_CEditSource.m_hWnd, &rect);
			GetWindowRect(hWnd, &rectDlg);

			//NOTE: RichEdit 1.0 EM_POSFROMCHAR requires an a &point for the first param and 
			//charpos for the second, RichEdit 2.0 is completly different!  Since this RichEdit
			//box was created with the resource editor, we can be assured that its always
			//"RICHEDIT" which is 1.0, if this ever changes this will crash!
			SendMessage(s_CEditSource.m_hWnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)iStart);

			//Place the dialog
			MoveWindow(hWnd, rect.left + pt.x, rect.bottom, rectDlg.right-rectDlg.left, rectDlg.bottom-rectDlg.top, TRUE);

		CLEANUP:
			FreeProperties(&cPropInfoSets, &rgPropInfoSets);
			SAFE_FREE(pStringBuffer);
			SAFE_RELEASE(pIDBProperties);
			SAFE_RELEASE(pIUnknown);
			SAFE_FREE(pwszInitString);
			
			if(FAILED(hr))
				EndDialog(hWnd, FALSE);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				switch(GET_WM_COMMAND_CMD(wParam, lParam))
				{
					//Selection change in a list box occurred
					case LBN_DBLCLK:
					{	
						//Double-Click, send OK to end dialog...
						SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDOK, hWnd, 0));
						return 0;
					}
				};

				break;
			};

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
				{
					CWaitCursor waitCursor;
					WCHAR wszBuffer[MAX_QUERY_LEN] = {0};

					//Obtain all Options Selected Items...
					INDEX iSel = s_CDSLContextBox.GetCurSel();
					if(iSel != LB_ERR)
					{
						//Obtain this property description
						s_CDSLContextBox.GetText(iSel, wszBuffer);

						//Now replace the entire edit box with the new selected value
						s_CEditSource.ReplaceAll(wszBuffer);
					}
					
					EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;   
}


////////////////////////////////////////////////////////////////
// CMainWindow::PromptDataSourceProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::PromptDataSourceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static DWORD	dwPromptOptions = DBPROMPTOPTIONS_PROPERTYSHEET;	//Default
	static BOOL		fUseExistingDSO = FALSE;

	static ULONG	cTypeFilters	= 1;
	static DWORD	rgTypeFilters[20] = { DBSOURCETYPE_DATASOURCE };
	static WCHAR	wszProvFilter[MAX_QUERY_LEN] = { 0 };

	static CComboBoxGuid		s_CComboInterface;
	static BOOL fAggregation	= FALSE;		//Default
	static BOOL fOutput			= TRUE;			//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			
			//Save the "this" pointer
			CMainWindow* pThis	= (CMainWindow*)SetThis(hWnd, (void*)lParam);
			CEnumerator* pCRootEnum	=  pThis->m_pCRootEnumerator;
			INDEX	iSel = 0;

			HWND	hWndOptions			= ::GetDlgItem(hWnd, IDL_OPTIONS);
			HWND	hWndTypeFilter		= ::GetDlgItem(hWnd, IDL_TYPEFILTER);
			HWND	hWndProvFilter		= ::GetDlgItem(hWnd, IDL_PROVIDERFILTER);

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IDBInitialize);
			
			ULONG i;

			//Populate the Options ListBox
			SendMessage(hWndOptions, LB_RESETCONTENT, 0, 0);

			for(i=0; i<g_cPromptOptions; i++)
			{
				iSel = (INDEX)wSendMessage(hWndOptions, LB_ADDSTRING, 0, g_rgPromptOptions[i].pwszName);
				SendMessage(hWndOptions,	LB_SETITEMDATA,	iSel, (LPARAM)g_rgPromptOptions[i].lItem);
				
				//Reselect all previously selected items...
				SendMessage(hWndOptions, LB_SETSEL, (dwPromptOptions & g_rgPromptOptions[i].lItem) == (DWORD)g_rgPromptOptions[i].lItem, i);
			}

			//Populate the TypeFilter ListBox
			SendMessage(hWndTypeFilter, LB_RESETCONTENT, 0, 0);
			for(i=0; i<g_cSourceType; i++)
			{
				iSel = (INDEX)wSendMessage(hWndTypeFilter, LB_ADDSTRING, 0, g_rgSourceType[i].pwszName);
				SendMessage(hWndTypeFilter,	LB_SETITEMDATA,	iSel, (LPARAM)g_rgSourceType[i].lItem);
				
				//Reselect all previously selected items...
				for(ULONG iFilter=0; iFilter<cTypeFilters; iFilter++)
					SendMessage(hWndTypeFilter, LB_SETSEL, rgTypeFilters[iFilter] == (DWORD)g_rgSourceType[i].lItem, i);
			}


			//We may need to connect to the RootEnumerator, if not done already
			pCRootEnum->CreateEnumInfo(CLSID_OLEDB_ENUMERATOR);

			//Populate the Provider Filter ListBox
			SendMessage(hWndProvFilter, LB_RESETCONTENT, 0, 0);
			for(i=0; i<pCRootEnum->m_cEnumInfo; i++)
			{
				iSel = (INDEX)wSendMessage(hWndProvFilter, LB_ADDSTRING, 0, pCRootEnum->m_rgEnumInfo[i].wszName);
				SendMessage(hWndProvFilter,	LB_SETITEMDATA,	iSel, (LPARAM)&pCRootEnum->m_rgEnumInfo[i]);
				
				//Reselect all previously selected items...
				SendMessage(hWndProvFilter, LB_SETSEL, wcsstr(wszProvFilter, pCRootEnum->m_rgEnumInfo[i].wszName)!=NULL, i);
			}

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Existing DSO
			::CheckDlgButton(hWnd, IDB_EXISTING_DSO,	BST2STATE(fUseExistingDSO));
			
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_AGGREGATION:
				{
					//Aggregation Combo Selection has changed...
					//If we are now using Aggregation, automatically change the requested
					//riid to IID_IUnknown, since its an error otherwise...
					if(::IsDlgButtonChecked(hWnd, IDB_AGGREGATION))
						s_CComboInterface.SetGuid(IID_IUnknown);
					return 0;
				}
				
				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow* pThis			= (CMainWindow*)GetThis(hWnd);
					CDataLinks*	pCDataLinks		= SOURCE_GETOBJECT(pThis->m_pCSource, CDataLinks);
					ASSERT(pCDataLinks);

					HRESULT	hr				= S_OK;
					HWND	hWndOptions		= ::GetDlgItem(hWnd, IDL_OPTIONS);
					HWND	hWndTypeFilter	= ::GetDlgItem(hWnd, IDL_TYPEFILTER);
					HWND	hWndProvFilter	= ::GetDlgItem(hWnd, IDL_PROVIDERFILTER);
					IUnknown*	pIUnknown	= NULL;

					//Obtain the Aggregation argument
					CAggregate* pCAggregate = NULL;
					fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
					if(fAggregation)
						pCAggregate = new CAggregate();
					
					//Obtain the Output (ppIUnknown) argument
					fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Interface
					REFIID riid = s_CComboInterface.GetGuid();
					
					LONG i;

					//Obtain all Options Selected Items...
					INDEX iSelCount = (INDEX)SendMessage(hWndOptions, LB_GETSELCOUNT, 0, 0);
					INDEX rgSelItems[MAX_NAME_LEN];
					SendMessage(hWndOptions, LB_GETSELITEMS, (WPARAM)MAX_NAME_LEN, (LPARAM)rgSelItems);
					dwPromptOptions = 0;

					for(i=0; i<iSelCount; i++)
						dwPromptOptions |= SendMessage(hWndOptions, LB_GETITEMDATA, rgSelItems[i], 0);

					//Obtain all TypeFilter Options Selected Items...
					iSelCount = (INDEX)SendMessage(hWndTypeFilter, LB_GETSELCOUNT, 0, 0);
					SendMessage(hWndTypeFilter, LB_GETSELITEMS, (WPARAM)MAX_NAME_LEN, (LPARAM)rgSelItems);
					for(i=0; i<iSelCount; i++)
						rgTypeFilters[i] = (DWORD)SendMessage(hWndOptions, LB_GETITEMDATA, rgSelItems[i], 0);
					cTypeFilters = (ULONG)iSelCount;

					//Provider Filter
					iSelCount = (INDEX)SendMessage(hWndProvFilter, LB_GETSELCOUNT, 0, 0);
					SendMessage(hWndProvFilter, LB_GETSELITEMS, (WPARAM)MAX_NAME_LEN, (LPARAM)rgSelItems);

					memset(wszProvFilter, 0, sizeof(wszProvFilter));
					WCHAR* pwsz = wszProvFilter;
					size_t cchRemaining = NUMELE(wszProvFilter)-1;	// reserve one char for the double null terminator
					for(i=0; i<iSelCount; i++)
					{
						ENUMINFO* pEnumInfo = (ENUMINFO*)SendMessage(hWndProvFilter, LB_GETITEMDATA, rgSelItems[i], 0);
						CLSID clsid;
						WCHAR* pwszProgID = NULL;

						//Protect overflowing the buffer...
						//NOTE: The Provider Filter requires a ProgID, not Provider Name.
						//So we need to convert the ParseName -> CLSID -> ProgID first...
						if(SUCCEEDED(CLSIDFromString(pEnumInfo->wszParseName, &clsid)))
						{
							if(SUCCEEDED(ProgIDFromCLSID(clsid, &pwszProgID)))
							{
								size_t ulStrLen = wcslen(pwszProgID);
								if(cchRemaining < ulStrLen + 1 )
									break;

								wcscpy_s(pwsz, cchRemaining, pwszProgID);
								pwsz += ulStrLen + 1;
								cchRemaining -= ulStrLen + 1;
							}
						}
						SAFE_FREE(pwszProgID);
					}

					*pwsz = L'\0';	// double null terminate

					//Obtain the Active MDI Child
					CMDIChild* pCMDIChild = (CMDIChild*)pThis->FindWindow(L"MDICHILD");
					
					//Existing DataSource
					fUseExistingDSO = ::IsDlgButtonChecked(hWnd, IDB_EXISTING_DSO);
					if(fUseExistingDSO && pCMDIChild && fOutput)
					{
						//Release is called on input, for successful cases...
						//I don't want them to release my DataSource, so AddRef...
						pIUnknown = SOURCE_GETINTERFACE(pCMDIChild->GetObject(eCDataSource), IUnknown);
						SAFE_ADDREF(pIUnknown);
					}

					//IDBPromptInitialize::PromptDataSource
					//NOTE:  The release of the input/ouput datasource is a likely tricky
					// 1.  *ppIUnknown == NULL on input - Succeeded	- output 1 reference
					// 2.  *ppIUnknown == NULL on input - Failed	- output NULL
					// 3.  *ppIUnknown != NULL on input - Succeeded - input is released, and output is QI'd. (in effect refcount has not changed)
					// 4.  *ppIUnknown != NULL on input - Failed	- input is untouched, output is NULL. (in effect refcount has not changed)
					//Although this seems a little odd, its entirly optimized for the consumer
					//All we have to do is AddRef the input and always release the output 
					TESTC(hr = pCDataLinks->PromptDataSource(pCAggregate, hWnd, dwPromptOptions, cTypeFilters, rgTypeFilters, wszProvFilter[0] ? wszProvFilter : NULL, riid, fOutput ? &pIUnknown : NULL));

					//Handle the returned object...
					//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
					if(!pThis->HandleObjectType(pCDataLinks, pIUnknown, riid, eCDataSource, 0, NULL, CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE | pThis->GetOptions()->m_dwCreateOpts))
						TESTC(hr = E_FAIL);
					
				CLEANUP:
					SAFE_RELEASE(pCAggregate);
					SAFE_RELEASE(pIUnknown);
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMainWindow::PromptFileNameProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMainWindow::PromptFileNameProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static DWORD dwPromptOptions = DBPROMPTOPTIONS_PROPERTYSHEET; //Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			
			//Save the "this" pointer
			CMainWindow* pThis = (CMainWindow*)SetThis(hWnd, (void*)lParam);

			HWND	hWndOptions		= ::GetDlgItem(hWnd, IDL_OPTIONS);
			INDEX	iSel = 0;

			//Directory
			//FileName

			//Populate the Options ListBox
			SendMessage(hWndOptions, LB_RESETCONTENT, 0, 0);
			for(ULONG i=0; i<g_cPromptOptions; i++)
			{
				iSel = (INDEX)wSendMessage(hWndOptions, LB_ADDSTRING, 0, g_rgPromptOptions[i].pwszName);
				SendMessage(hWndOptions,	LB_SETITEMDATA,	iSel, (LPARAM)g_rgPromptOptions[i].lItem);
				
				//Reselect all previously selected items...
				SendMessage(hWndOptions, LB_SETSEL, (dwPromptOptions & g_rgPromptOptions[i].lItem) == (DWORD)g_rgPromptOptions[i].lItem, i);
			}
			
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_DIRECTORY:
				case IDB_FILENAME:
				{
					HWND	hWndEdit	= ::GetDlgItem(hWnd, IDE_FILENAME);
					WCHAR*	pwszTitle	= NULL;
					WCHAR	wszBuffer[MAX_NAME_LEN] = {0};
					DWORD	dwFlags		= 0;

					if(GET_WM_COMMAND_ID(wParam, lParam) == IDB_DIRECTORY)
					{
						hWndEdit	= ::GetDlgItem(hWnd, IDE_DIRECTORY);
						pwszTitle	= L"Directory Name";
						dwFlags		= OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT	| OFN_SHAREAWARE;
					}
					else
					{
						hWndEdit	= ::GetDlgItem(hWnd, IDE_FILENAME);
						pwszTitle	= L"File Name";
						dwFlags		= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT	| OFN_SHAREAWARE;
					}

					//Display Common Dialog to obtain DataSource...
					//This is for providers that take a filename/path for this property
					if(SUCCEEDED(BrowseOpenFileName(GetAppLite()->m_hInstance, hWnd, pwszTitle, wszBuffer, MAX_NAME_LEN, NULL, L"All Files (*.*)\0*.*\0\0", NULL, dwFlags)))
					{
						//Just update value
						wSendMessage(hWndEdit, WM_SETTEXT, 0, wszBuffer);
					}
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMainWindow*	pThis		= (CMainWindow*)GetThis(hWnd);
					CDataLinks*		pCDataLinks	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataLinks);
					ASSERT(pCDataLinks);

					HRESULT hr = S_OK;
					WCHAR* pwszSelectedFile = NULL;
					IUnknown* pIUnknown = NULL;

					HWND	hWndDirectory	= ::GetDlgItem(hWnd, IDE_DIRECTORY);
					HWND	hWndFileName	= ::GetDlgItem(hWnd, IDE_FILENAME);
					HWND	hWndOptions		= ::GetDlgItem(hWnd, IDL_OPTIONS);

					//Directory
					WCHAR* pwszDirectory = wGetWindowText(hWndDirectory);

					//FileName
					WCHAR* pwszFileName = wGetWindowText(hWndFileName);

					//Obtain all Options Selected Items...
					INDEX iSelCount = (INDEX)SendMessage(hWndOptions, LB_GETSELCOUNT, 0, 0);
					ASSERT(iSelCount < 20);
					LONG rgSelItems[20];
					SendMessage(hWndOptions, LB_GETSELITEMS, (WPARAM)20, (LPARAM)rgSelItems);
					dwPromptOptions = 0;
					for(LONG i=0; i<iSelCount; i++)
						dwPromptOptions |= SendMessage(hWndOptions, LB_GETITEMDATA, rgSelItems[i], 0);

					//IDBPromptInitalize::PromptFileName
					TESTC(hr = pCDataLinks->PromptFileName(
								hWnd,								// hWndParent
								dwPromptOptions,					// dwPromptOptions
            					pwszDirectory,						// pwszInitialDirectory
            					pwszFileName,						// pwszInitialFile
            					&pwszSelectedFile					// pwszSelectedFile
								));
					
					//Delegate, and Connect from the Selected File
					TESTC(hr = pThis->m_pCServiceComp->ConnectFromFile(pwszSelectedFile));
					
				CLEANUP:
					SAFE_FREE(pwszDirectory);
					SAFE_FREE(pwszFileName);
					SAFE_FREE(pwszSelectedFile);
					SAFE_RELEASE(pIUnknown);
					
					if(SUCCEEDED(hr))
					{
						EndDialog(hWnd, TRUE);
					}
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}

	return FALSE;
}



////////////////////////////////////////////////////////////////
// CMainWindow::HandleObjectType
//
/////////////////////////////////////////////////////////////////
CBase* CMainWindow::HandleObjectType(CBase* pCSource, IUnknown* pIUnknown, REFIID riid, SOURCE eSource, ULONG cPropSets, DBPROPSET* rgPropSets, DWORD dwFlags)
{
	CWaitCursor waitCursor;
	HRESULT hr = S_OK;
	CMDIChild*	pCMDIChild	= pCSource ? pCSource->m_pCMDIChild : NULL;
	CMDIChild*	pCNewChild	= NULL;
	CBase*		pCObject	= NULL;
	DWORD dwCreateOpts		= GetOptions()->m_dwCreateOpts;
	
	//No-op
	if(!pIUnknown)
		return NULL;

	if(dwFlags & CREATE_DETERMINE_TYPE)
	{
		//Don't rely upon the caller knowing exactly what the object type is...
		//Since many OLE DB methods can return different objects depending upon interface or properties
		//requested.  So use the "suggested" type as an optimizing starting point, and if not
		//then proceed the hard way to determine exactly what it is...
		eSource = DetermineObjectType(pIUnknown, eSource);
	}

	//Does the object already exists, we may need to create a new window
	if(pCMDIChild && (dwFlags & CREATE_NEWWINDOW_IFEXISTS))
	{
		//NOTE: Some objects occupy the same location (address) - (ie: only one is active at a time).
		//For example: Only one DataSet, Row, or Rowset will be within a MDIChild, but all reside 
		//in the base class CDataAccess for simplity (no special casing).  Because of this you can't
		//just do ->GetObject(eSource) since it will return NULL if not the exact object type, but
		//their could still be another type of object present.  For example, a rowset -> row,
		//if we asked for GetObject(eCRow) we would get NULL, and think we don't need to create 
		//antoher window, but in reality the "address" is already taken for another object.  The
		//reason we use GetObjectAddress instread of GetObject.
		CBase** ppCBase = pCMDIChild->GetObjectAddress(eSource);
		if(ppCBase)
		{
			CBase* pCBase = *ppCBase;
			if(pCBase && pCBase->m_pIUnknown)
			{
				//If its not the same object then create a new window.  This way the 
				//user knows if the object returned is the same as before or not...
				if(!pCBase->IsSameObject(pIUnknown))
					dwFlags |= CREATE_NEWWINDOW;
			}
		}
	}

	//Do we need to find the parent window
	if(pCSource && (dwFlags & CREATE_FINDWINDOW))
	{
		//Unless we find the window, we will have to create a new one...
		dwFlags |= CREATE_NEWWINDOW;
		
		//Try to find the parent window...
		CBase* pCParent = pCSource->GetParent(eSource);
		if(pCParent)
		{
			if(pCParent->m_pIUnknown && !pCParent->IsSameObject(pIUnknown))
			{
				//Found window/object, but an object already exists of this type, 
				//and its not the same object.  Looks like a different object was 
				//returned.  Display the new object, so the user visually sees that 
				//a new object was created, and not just the parent returned...
				dwFlags |= CREATE_NEWWINDOW;
			}
			else
			{
				//Since we found the parent, we need to use the parent for this object/display
				pCSource	= pCParent->m_pCParent;
				pCMDIChild	= pCParent->m_pCMDIChild;

				//Found window and object.
				if(pCMDIChild)
					MDIActivate(pCMDIChild->GetWnd());
				dwFlags &= ~CREATE_NEWWINDOW;
			}
		}
	}

	//Create a new window...
	if(dwFlags & CREATE_NEWWINDOW)
	{
		pCNewChild = NULL;
		pCMDIChild = NULL;
	}

	//Now that we know the type, do the appropiate thing...
	switch(eSource)
	{
		case eCBinder:
		{	
			//Stand Alone Object (no window required)
			CBinder* pCBinder = new CBinder(this);
			pCObject = pCBinder;

			//Binder - Connect using the passed in Instance
			TESTC(hr = pCBinder->CreateObject(pCSource, riid, pIUnknown));

			//Binder - SetProperties
			if(cPropSets)
				TESTC(hr = pCBinder->SetProperties(cPropSets, rgPropSets));
			break;
		}

		case eCEnumerator:
		{	
			//Stand Alone Object (no window required)
			CEnumerator* pCEnumerator = new CEnumerator(this);
			pCObject = pCEnumerator;

			//Enum - Connect using the passed in Instance
			TESTC(hr = pCEnumerator->CreateObject(pCSource, riid, pIUnknown));

			//Enum - SetProperties
			if(cPropSets && dwFlags & CREATE_SETPROPERTIES)
				TESTC(hr = pCEnumerator->SetProperties(cPropSets, rgPropSets));

			//Enum - Initialize
			if(dwFlags & CREATE_INITIALIZE)
			{
				//Initialize and Obtain interfaces
				TESTC(hr = pCEnumerator->Initialize());
			}
			break;
		}

		case eCDataSource:
		{
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//If the user has requested to Initialize the object, then don't AutoQI
			//for optional interfaces until after Initialization.  This way we improve
			//performance and help to allow pooling.  We do want to QI for Optional
			//interfaces if not going to initialize since the object may already
			//be initialized or the user may want optional interfaces before initialization
			if(dwFlags & CREATE_INITIALIZE)
				ENABLE_BIT(dwCreateOpts, CREATE_QI_OPTIONAL, FALSE);
			
			//Defferred Object
			if(!pCMDIChild->m_pCDataSource)
				pCMDIChild->m_pCDataSource = new CDataSource(NULL, pCMDIChild);
			CDataSource* pCDataSource = pCMDIChild->m_pCDataSource;
			pCObject = pCDataSource;

			//Create the Object
			TESTC(hr = pCDataSource->CreateObject(pCSource, riid, pIUnknown, dwCreateOpts));

			//DataSource - SetProperties
			if(cPropSets && (dwFlags & CREATE_SETPROPERTIES))
				TESTC(hr = pCDataSource->SetProperties(cPropSets, rgPropSets));

			//DataSource - Initialize
			if(dwFlags & CREATE_INITIALIZE)
			{
				//Initialize and Obtain datasource interfaces
				TESTC(hr = pCDataSource->Initialize());
			}

			//Create Session
			//Since this is a generic routine, we are not totally sure that we are initialized 
			//or not.  The CREATE_INITIALIZE flag is a good indication, but this could be called
			//from other places that return initialized DSOs (Bind for example) and we want to provide
			//the user with a helper to provide the sub objects.  So if IDBCreateSession is exposed
			//try and create a session, but don't popup the error to the user. It will be in the trace.
			if((dwCreateOpts & CREATE_QI_MANDATORY) /*&& d(wFlags & CREATE_INITIALIZE)*/)
			{
				//NOTE: We only "automatically" create a session if the user requests
				//MANDATORY interfaces, we have IDBCreateSession, 
				//and there already isn't an existing session
				if(pCDataSource->m_pIDBCreateSession && !pCMDIChild->GetObject(eCSession))
				{
					//Recurse
					IUnknown* pIUnkSession = NULL;
				
					//IDBCreateSession::CreateSession
					//NOTE: Don't fail creation of the object if CreateSession fails...
					//NOTE: We don't use IID_IOpenRowset since its not a required session interface
					//if the DataSource if of type MDP exclusively...
					if(SUCCEEDED(pCDataSource->CreateSession(NULL, IID_IUnknown, (IUnknown**)&pIUnkSession)))
						HandleObjectType(pCDataSource, pIUnkSession, IID_IUnknown, eCSession, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
					SAFE_RELEASE(pIUnkSession);
				}
			}
			break;
		}
	
		case eCSession:
		{
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//Defferred Object
			if(!pCMDIChild->m_pCSession)
				pCMDIChild->m_pCSession = new CSession(NULL, pCMDIChild);
			CSession* pCSession = pCMDIChild->m_pCSession;
			pCObject = pCSession;

			//Update our Session object
			TESTC(hr = pCSession->CreateObject(pCSource, riid, pIUnknown));

			//Command (optional - provider may not support commands)
			//NOTE: We only "automatically" create a command if the user requests
			//MANDATORY interfaces, we have IDBCreateCommand, 
			//and there already isn't an existing command
			if(dwCreateOpts & CREATE_QI_MANDATORY)
			{
				if(pCSession->m_pIDBCreateCommand && !pCMDIChild->GetObject(eCCommand))
				{
					//Recurse
					ICommand* pICommand = NULL;
					
					//NOTE: Don't fail creation of the object if CreateCommand fails...
					if(SUCCEEDED(pCSession->CreateCommand(NULL, IID_ICommand, (IUnknown**)&pICommand)))
						HandleObjectType(pCSession, pICommand, IID_ICommand, eCCommand, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
					SAFE_RELEASE(pICommand);
				}
			}
			break;
		}

		case eCCommand:
		{
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//Defferred Object
			if(!pCMDIChild->m_pCCommand)
				pCMDIChild->m_pCCommand = new CCommand(NULL, pCMDIChild);
			CCommand* pCCommand = pCMDIChild->m_pCCommand;
			pCObject = pCCommand;
			
			//Update our Command object (and set default properties)
			TESTC(hr = pCCommand->CreateObject(pCSource, riid, pIUnknown)); 

			//Command - SetProperties
			if(cPropSets)
				TESTC(hr = pCCommand->SetProperties(cPropSets, rgPropSets));

			//We should set the default properties on the Command object as well...
			if(GetOptions()->m_dwRowsetOpts & ROWSET_SETDEFAULTPROPS)
			{
				if(pCMDIChild->m_CDefPropSets.GetCount())
					pCCommand->SetProperties(pCMDIChild->m_CDefPropSets.GetCount(), pCMDIChild->m_CDefPropSets.GetPropSets());
			}
			break;
		}

		case eCMultipleResults:
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//Defferred Object
			if(!pCMDIChild->m_pCMultipleResults)
				pCMDIChild->m_pCMultipleResults = new CMultipleResults(NULL, pCMDIChild);
			pCObject = pCMDIChild->m_pCMultipleResults;

			//Obtain IMultipleResults
			TESTC(hr = pCMDIChild->m_pCMultipleResults->CreateObject(pCSource, riid, pIUnknown));
			break;

		case eCRowset:
		{
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//NOTE: We delete the previous class, since we may be creating a different type
			//Only if its of a different type, (so we don't have duplicate similar objects in the tree...
			CRowset* pCRowset = SOURCE_GETOBJECT(pCMDIChild->m_pCDataAccess, CRowset);
			if(!pCRowset)
			{
				SAFE_RELEASE(pCMDIChild->m_pCDataAccess);
				pCRowset = new CRowset(NULL, pCMDIChild);
			}
			pCObject = pCMDIChild->m_pCDataAccess = pCRowset;


			//Now, update our rowset object with the rowset pointer
			TESTC(hr = pCRowset->CreateObject(pCSource, riid, pIUnknown));
			

			//Do we need to restart the cursor before displaying this object?
			if(dwFlags & CREATE_RESTARTPOSITION)
			{
				pCRowset->RestartPosition();
			}
			break;
		}

		case eCRow:
		{
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//NOTE: We delete the previous class, since we may be creating a different type
			//Only if its of a different type, (so we don't have duplicate similar objects in the tree...
			CRow* pCRow = SOURCE_GETOBJECT(pCMDIChild->m_pCDataAccess, CRow);
			if(!pCRow)
			{
				SAFE_RELEASE(pCMDIChild->m_pCDataAccess);
				pCRow = new CRow(NULL, pCMDIChild);
			}
			pCObject = pCMDIChild->m_pCDataAccess = pCRow;

			//Update our row object (in new window always)
			TESTC(hr = pCRow->CreateObject(pCSource, riid, pIUnknown));

			//Create the command object off the row by default...
			//NOTE: We only "automatically" create a command if the user requests
			//MANDATORY interfaces, we have IDBCreateCommand, 
			//and there already isn't an existing command
			if(dwCreateOpts & CREATE_QI_MANDATORY)
			{
				if(pCRow->m_pIDBCreateCommand && !pCMDIChild->GetObject(eCCommand))
				{
					//Recurse
					ICommand* pICommand = NULL;
					if(SUCCEEDED(pCRow->CreateCommand(NULL, IID_ICommand, (IUnknown**)&pICommand)))
						HandleObjectType(pCRow, pICommand, IID_ICommand, eCCommand, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
					SAFE_RELEASE(pICommand);
				}
			}
			break;
		}

		case eCStream:
		{	
			//Stand Alone Object (no window required)
			CStream* pCStream = new CStream(this);
			pCObject = pCStream;

			//Update our Stream object
			TESTC(hr = pCStream->CreateObject(pCSource, riid, pIUnknown));
			break;
		}

		case eCDataset:
		{
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//NOTE: We delete the previous class, since we may be creating a different type
			//Only if its of a different type, (so we don't have duplicate similar objects in the tree...
			CDataset* pCDataset = SOURCE_GETOBJECT(pCMDIChild->m_pCDataAccess, CDataset);
			if(!pCDataset)
			{
				SAFE_RELEASE(pCMDIChild->m_pCDataAccess);
				pCDataset = new CDataset(NULL, pCMDIChild);
			}
			pCObject = pCMDIChild->m_pCDataAccess = pCDataset;

			//Update our data set
			TESTC(hr = pCDataset->CreateObject(pCSource, riid, pIUnknown));
			break;
		}

		case eCServiceComp:
			pCObject = m_pCServiceComp;

			//Update our object
			TESTC(hr = m_pCServiceComp->CreateObject(pCSource, riid, pIUnknown));
			break;

		case eCDataLinks:
			//Object requires a window
			if(!pCMDIChild)
				pCNewChild = pCMDIChild = new CMDIChild(this);

			//Defferred Object
			if(!m_pCDataLinks)
				m_pCDataLinks = new CDataLinks(this);
			pCObject = m_pCDataLinks;

			//Update our object
			TESTC(hr = m_pCDataLinks->CreateObject(pCSource, riid, pIUnknown));
			break;

		case eCTransaction:
		{	
			//Stand Alone Object (no window required)
			CTransaction* pCTransaction = new CTransaction(this);
			pCObject = pCTransaction;

			//Update our object
			TESTC(hr = pCTransaction->CreateObject(pCSource, riid, pIUnknown));
			break;
		}

		case eCTransactionOptions:
		{	
			//Stand Alone Object (no window required)
			CTransactionOptions* pCTransactionOptions = new CTransactionOptions(this);
			pCObject = pCTransactionOptions;

			//Update our object
			TESTC(hr = pCTransactionOptions->CreateObject(pCSource, riid, pIUnknown));
			break;
		}

		case eCError:
		{	
			//Stand Alone Object (no window required)
			CError* pCError = new CError(this);
			pCObject = pCError;

			//Update our object
			TESTC(hr = pCError->CreateObject(pCSource, riid, pIUnknown));
			break;
		 }

		case eCCustomError:
		{	
			//Stand Alone Object (no window required)
			CCustomError* pCCustomError = new CCustomError(this);
			pCObject = pCCustomError;

			//Update our object
			TESTC(hr = pCCustomError->CreateObject(pCSource, riid, pIUnknown));
			break;
		 }

		case eCRowPosition:
		{	
			//Stand Alone Object (no window required)
			CRowPosition* pCRowPosition = new CRowPosition(this);
			pCObject = pCRowPosition;

			//Update our object
			TESTC(hr = pCRowPosition->CreateObject(pCSource, riid, pIUnknown));

			//Now Initialize (if source was a rowset)...
			if(dwFlags & CREATE_INITIALIZE)
				TESTC(hr = pCRowPosition->Initialize(pCSource ? pCSource->m_pIUnknown : NULL));
			break;
		}


		case eCConnectionPoint:
		{	
			//Stand Alone Object (no window required)
			CConnectionPoint* pCConnectionPoint = new CConnectionPoint(this);
			pCObject = pCConnectionPoint;
			
			//Update our object
			TESTC(hr = pCConnectionPoint->CreateObject(pCSource, riid, pIUnknown));
			break;
		}

		case eCUnknown:
		{	
			//Stand Alone Object (no window required)
			CUnknown* pCUnknown = new CUnknown(this);
			pCObject = pCUnknown;
			
			//Update our object
			TESTC(hr = pCUnknown->CreateObject(pCSource, riid, pIUnknown));

			//Inform user that we don't have a clue what type of object this is?
			wMessageBox(GetFocus(), MB_OK|MB_ICONWARNING|MB_SYSTEMMODAL, wsz_WARNING, 
				L"The object returned does not support any of the mandatory interfaces for any recognized OLE DB Object?\n");
			break;
		}

		default:
			ASSERT(!"Unhandled Object Type!");
			TESTC(hr = E_FAIL);
			break;
	};

	//Now display the new window
	if(pCNewChild)
	{
		//Copy(AddRef) the previous objects for this wiindow...
		if(pCObject)
		{
			CBase* pCParent = pCObject->m_pCParent;
			while(pCParent)
			{
				//Copy into our objects, if they don't already exist...
				CBase** ppCBase = pCNewChild->GetObjectAddress(pCParent->GetObjectType());
				if(ppCBase && *ppCBase==NULL)
				{
					//NOTE: We don't want to replace objects of the same type.
					//For example: If the parent of this object is of the same
					//type, (IRowset -> ColumnsRowset) then we don't need to replace
					//the current object.  The simplest way to do this is to only
					//replace NULL addresses (ie: *ppCBase == NULL)
					SAFE_ADDREF(pCParent);
					*ppCBase = pCParent;
				}
				
				pCParent = pCParent->m_pCParent;
			}
		}

		//Create the new window...
		pCNewChild->Create(m_hWndMDIClient, L"MDICHILD", NULL, IDR_ROWSETVIEWER, ImageList_GetIcon(m_hImageList, IMAGE_OBJECTS, ILD_NORMAL));
	}
	
	//Display the object, unless the caller app wishes to...
	if(!(dwFlags & CREATE_NODISPLAY))
		pCObject->DisplayObject();

CLEANUP:
	if(FAILED(hr) || !pCObject)
	{
		SAFE_DELETE(pCNewChild);
		pCObject = NULL;
	}

	UpdateControls();
	return pCObject;
}					



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg
//
/////////////////////////////////////////////////////////////////////////////
CAboutDlg::CAboutDlg()
	: CDialogLite(IDD_ABOUT)
{
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAboutDlg::OnInitDialog()
{
	CWaitCursor waitCursor;

	//Fill in About Text
	wSendMessageFmt(GetDlgItem(IDT_FILEINFO), WM_SETTEXT, 0,
		L"%s\n"				//ProductName
		L"Version %S\n"		//Version/Build
		L"%S\n",			//CopyWrite
		
		L"Microsoft OLE DB RowsetViewer",
		VER_PRODUCTVERSION_STR,
		VER_LEGALCOPYRIGHT_STR
		);

	return CDialogLite::OnInitDialog();
}
