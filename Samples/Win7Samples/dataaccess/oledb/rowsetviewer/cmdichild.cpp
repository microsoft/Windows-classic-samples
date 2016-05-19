//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CMDICHILD.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//												   
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"

#ifdef MTSTXN
	#include <xolehlp.h> 
	#include <txdtc.h>
#endif //MTSTXN


ULONG	CMDIChild::m_iChildWindow	= 0;	//Static member
///////////////////////////////////////////////////////////////
// CMDIChild::CMDIChild
//
/////////////////////////////////////////////////////////////////
CMDIChild::CMDIChild(CMainWindow* pCMainWindow)
{
	//Objects
	ASSERT(pCMainWindow);
	m_pCMainWindow = pCMainWindow;
	
	//Controls
	m_pCDataGrid			= new CDataGrid(this);
	m_pCQueryBox			= new CQueryBox(this);

	//Objects
	m_pCDataSource			= NULL;		//Deferred
	m_pCSession				= NULL;		//Deferred
	m_pCCommand				= NULL;		//Deferred
	m_pCMultipleResults		= NULL;		//Deferred
	m_pCDataAccess			= NULL;		//Deferred
	
	//Data
	m_lastSizedEdge = 0;
	m_iChildWindow++; 
	m_pwszConfig	= NULL;

	//Only set these "Default" properties, if requested by the user
	if(GetOptions()->m_dwRowsetOpts & ROWSET_SETDEFAULTPROPS)
	{
		//DBPROP_CANHOLDROWS is required by the OLE DB Spec - Level-0 Conformance
		//Since it is also legal to set a ReadOnly property, just blindy set it...
		m_CDefPropSets.SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBTYPE_BOOL, (void*)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);
	}
}


////////////////////////////////////////////////////////////////
// CMDIChild::~CMDIChild
//
/////////////////////////////////////////////////////////////////
CMDIChild::~CMDIChild()
{
	SAFE_RELEASE(m_pCDataAccess);
 	SAFE_RELEASE(m_pCMultipleResults);
	SAFE_RELEASE(m_pCCommand);
	SAFE_RELEASE(m_pCSession);
	SAFE_RELEASE(m_pCDataSource);

	//Controls
	SAFE_DELETE(m_pCDataGrid);
	SAFE_DELETE(m_pCQueryBox);

	//Data
	m_iChildWindow--;
	SAFE_FREE(m_pwszConfig);
}


////////////////////////////////////////////////////////////////
// CMDIChild::GetOptions
//
/////////////////////////////////////////////////////////////////
COptionsSheet*	CMDIChild::GetOptions()
{
	return m_pCMainWindow->GetOptions();
}


////////////////////////////////////////////////////////////////
// CMDIChild::SetConfig
//
/////////////////////////////////////////////////////////////////
void CMDIChild::SetConfig(WCHAR* pwszConfig, BOOL fCopy)
{
	SAFE_FREE(m_pwszConfig);

	//Optmization:  If the caller no longer needs the name, 
	//no sense in reallocing, copying, freeing, freeing.  Just reference it...
	m_pwszConfig = fCopy ? wcsDuplicate(pwszConfig) : pwszConfig;

	//Obtain the last Saved Query in the Query Box
	if(m_pCQueryBox && m_pwszConfig)
	{
		WCHAR* pwszQuery = NULL;
		
		//Formulate the key
		static WCHAR wszKeyName[MAX_NAME_LEN];
		StringFormat(wszKeyName, NUMELE(wszKeyName), L"%s\\%s\\Command", wszCONFIG_KEY, m_pwszConfig); 

		//Save the Query
		GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Query", &pwszQuery);

		//Update the Query Box
		m_pCQueryBox->SetWindowText(pwszQuery);
		SAFE_FREE(pwszQuery);
	}
}


	
////////////////////////////////////////////////////////////////
// CMDIChild::AutoPosition
//
/////////////////////////////////////////////////////////////////
BOOL CMDIChild::AutoPosition(BOOL fDefaultPosition)
{
	if(fDefaultPosition || m_wndPlacement.length == 0)
	{
		//Default setting, upper right corner (3/4 of the client area)...
		SIZE sizeClient = GetClientSize(m_pCMainWindow->m_hWndMDIClient);
		return MoveWindow(m_hWnd, (INT)((float)sizeClient.cx * 0.25), 0, (INT)((float)sizeClient.cx * 0.75), (INT)((float)sizeClient.cy * 0.75), TRUE);
	}
	else
	{
		return SetWindowPlacement();
	}
}


////////////////////////////////////////////////////////////////
// CMDIChild::OnCreate
//
/////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// create child windows
	// 1. ListView for rowset data
	// 2. EditBox for entering SQL Text
	// 3. ListBox for notifcations
	// 4. ScrollBar for scrolling through the rowset
	GetWindowPlacement();
	SIZE size = { m_wndPlacement.rcNormalPosition.right - m_wndPlacement.rcNormalPosition.left, 
					m_wndPlacement.rcNormalPosition.bottom - m_wndPlacement.rcNormalPosition.top };

	//Create the SQL Query Box
	m_pCQueryBox->Create(m_hWnd, m_pCMainWindow->m_hLibRichEdit20 ? (IsUnicodeOS() ? L"RichEdit20W" : L"RichEdit20A") : L"RICHEDIT", NULL, IDC_EDITBOX, 
		WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE /* | ES_DISABLENOSCROLL | ES_SELECTIONBAR*/, 0,
		0, 0, size.cx, (INT)((float)size.cy * 0.25));
	m_pCQueryBox->OnCreate(NULL);
	m_pCQueryBox->SetWordWrap(TRUE);

	//By default the RichEdit controls have a HUGE text.  Instead of setting it
	//to a small size font which is difficult for some international lcids, we will
	//set it to the same font as the Tree control...
	m_pCQueryBox->SetFont(m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->GetFont());

	//Create the DataGrid
	m_pCDataGrid->Create(m_hWnd, WC_LISTVIEWW, NULL, IDC_LISTVIEW, 
		WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | /*LVS_SINGLESEL |*/ LVS_AUTOARRANGE | LVS_REPORT | LVS_EDITLABELS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE,
		0, (INT)((float)size.cy * 0.25), size.cx - GetSystemMetrics(SM_CXVSCROLL), (INT)((float)size.cy * 0.75));
	m_pCDataGrid->EnableWindow(FALSE);

	//Use Extended ListView Styles!
	SendMessage(m_pCDataGrid->m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
	ListView_SetImageList(m_pCDataGrid->m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
	m_pCDataGrid->OnCreate(NULL);
	m_pCDataGrid->ClearAll(L"No Rowset");

	//Set Focus to the Query Box
	m_pCQueryBox->SetFocus();

	//Setup Splitter Windows...
	m_pCQueryBox->SetSplitter(NULL, m_pCDataGrid, NULL, NULL);
	m_pCDataGrid->SetSplitter(m_pCQueryBox, NULL, NULL, NULL);

	//Load Saved Window Positions
	if(m_iChildWindow == 1)
	{
		memset(&m_wndPlacement,	0, sizeof(m_wndPlacement));
		GetRegEntry(HKEY_ROWSETVIEWER, wszMDICHILD_KEY, L"WinPosition",	&m_wndPlacement, sizeof(m_wndPlacement), NULL);

		//Window Position
		AutoPosition(m_wndPlacement.length == 0/*fDefaultPosition*/);
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::OnInitialUpdate
//
/////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnInitialUpdate()
{
    //Update Window Title
	UpdateWndTitle();
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::UpdateControls
//
/////////////////////////////////////////////////////////////////
BOOL CMDIChild::UpdateControls()
{
	CDataAccess* pCDataAccess = m_pCDataAccess && m_pCDataAccess->m_pIUnknown ? m_pCDataAccess : NULL;

	//ToolBar Buttons
	m_pCMainWindow->UpdateControls();

	//Disable ListView window
	m_pCDataGrid->EnableWindow(pCDataAccess != NULL);

	//Update Window Title (in-case things have changed...)
	UpdateWndTitle();
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::DisplayDialog
//
/////////////////////////////////////////////////////////////////
INT_PTR		CMDIChild::DisplayDialog(UINT uID, HWND hWndParent, DLGPROC lpDialogFunc, CBase* pCObject, UINT idSource)
{
	m_idSource = idSource;
	m_pCSource = pCObject;
	return ::DisplayDialog(uID, hWndParent, lpDialogFunc, (LPARAM)this);
}

	
/////////////////////////////////////////////////////////////////////////////
// CMDIChild::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	//Default
	switch(nID)
	{
		//Edit Window
		ON_COMMAND_UI_ENABLED(IDM_PASTEEDITWINDOW,				m_pCQueryBox->CanPaste())
		ON_COMMAND_UI_ENABLED(IDM_CLEAREDITWINDOW,				TRUE)

		//Cut/Copy is only enabled - if something is selected.
		ON_COMMAND_UI_ENABLED(IDM_COPYEDITWINDOW,				m_pCQueryBox->GetSel(NULL, NULL))
		ON_COMMAND_UI_ENABLED(IDM_CUTEDITWINDOW,				m_pCQueryBox->GetSel(NULL, NULL))
			
		//Menu Items that are always on
		ON_COMMAND_UI_ENABLED(IDM_PROVIDERINFO,					TRUE)
		ON_COMMAND_UI_ENABLED(IDM_BROWSE_TABLENAME,				TRUE)
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CMDIChild::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	//CDataGrid
	if(m_pCDataGrid->OnNotify(idCtrl, pNMHDR)) 
		return TRUE;
	
	//CQueryBox
	if(m_pCQueryBox->OnNotify(idCtrl, pNMHDR)) 
		return TRUE;
	
	//Delegate
	return CMDIChildLite::OnNotify(idCtrl, pNMHDR);
}


/////////////////////////////////////////////////////////////////////
// CMDIChild::OnSizing
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnSizing(WPARAM nSide, REFPOINTS pts)
{
	//Record which edge was touched...
	m_lastSizedEdge = nSide;
	return TRUE;
}
		
/////////////////////////////////////////////////////////////////////
// CMDIChild::OnSize
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnSize(WPARAM nType, REFPOINTS pts)
{
	ASSERT(m_pCDataGrid->m_hWnd);
	ASSERT(m_pCQueryBox->m_hWnd);

	switch(nType)
	{
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
		{
			if(pts.x && pts.y)
			{
				//Obtain window sizes...
				SIZE sizeEditBox	= GetWindowSize(m_pCQueryBox->m_hWnd);
				SIZE sizeListView	= GetWindowSize(m_pCDataGrid->m_hWnd);

				//The MainWindow has been resized, we need to readjust all
				//Child controls, (ScrollBar, ListBox, ListView...)
				//Move Windows with respect to the new 

				switch(m_lastSizedEdge)
				{
					//Top was moved, resize the Editbox
					case WMSZ_TOP:
					case WMSZ_TOPLEFT:
					case WMSZ_TOPRIGHT:
						sizeEditBox.cy = max(0, pts.y - sizeListView.cy);
						break;	
				}

				MoveWindow(m_pCQueryBox->m_hWnd, 0, 0, pts.x, sizeEditBox.cy, TRUE);
				MoveWindow(m_pCDataGrid->m_hWnd, 0, sizeEditBox.cy, pts.x, pts.y-sizeEditBox.cy, TRUE);
				m_lastSizedEdge = 0;
			}

			//Call default procedure first, 
			//to let MDI position the child & then move its children
			//We simply do this by returning false, to indicate we didn't handle it...
			return FALSE;
		}
	};

	return FALSE;
}                


/////////////////////////////////////////////////////////////////////
// CMDIChild::OnClose
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnClose()
{
	//Send Message to the MainWindow to Close
	//Mainly used so the MainWindow updates the ToolBar if there 
	//Are no more child windows...
	SendMessage(m_pCMainWindow->GetWnd(), WM_COMMAND, GET_WM_COMMAND_MPS(IDM_CLOSEWINDOW, m_hWnd, 0));
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMDIChild::OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnDestroy()
{
	//Save the Query in the Query Box (before exiting)
	if(m_pCQueryBox && m_pwszConfig)
	{
		WCHAR* pwszQuery = m_pCQueryBox->GetWindowText();
		
		//Formulate the key
		static WCHAR wszKeyName[MAX_NAME_LEN];
		StringFormat(wszKeyName, NUMELE(wszKeyName), L"%s\\%s\\Command", wszCONFIG_KEY, m_pwszConfig); 

		//Save the Query
		SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Query",	pwszQuery);
		SAFE_FREE(pwszQuery);
	}

	//Save Window Positions
	if(m_iChildWindow == 1)
	{
		//Window Positions
		if(GetWindowPlacement())
			SetRegEntry(HKEY_ROWSETVIEWER, wszMDICHILD_KEY, L"WinPosition",	&m_wndPlacement,	sizeof(m_wndPlacement));
	}

	//Delegate
	return CMDIChildLite::OnDestroy();
}


/////////////////////////////////////////////////////////////////////
// CMDIChild::OnMDIActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate)
{
	//So see if this window is being activated or deactivated
	if(hWndActivate == m_hWnd)
	{
		CObjTree* pCObjTree = m_pCMainWindow->m_pCMDIObjects->m_pCObjTree;
		ASSERT(pCObjTree);

		//Select the Object in Tree that this Window corresponds to...
		//NOTE: If there is already an object selected that belongs to this
		//window, then there is no need to switch the objects
		CBase* pCObject = pCObjTree->GetSelectedObject();
		if(!pCObject || (pCObject->m_pCMDIChild != this))
			pCObjTree->SelectObject(GetObject());
		
		//Refresh all Controls
		UpdateControls();
	}

	return TRUE;
}
		
/////////////////////////////////////////////////////////////////////////////
// CMDIChild::OnSetFocus
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnSetFocus(HWND hWndPrevFocus)
{
	m_pCQueryBox->SetFocus();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CMDIChild::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChild::OnCommand(UINT iID, HWND hWndCtrl)
{
	//Get the "this" pointer
	CMainWindow*	pCMainWindow	= m_pCMainWindow;
	HWND			hWnd			= m_hWnd;
	HRESULT	hr = S_OK;

	//Is there an active object...
	CBase*			pCObject			= m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->GetSelectedObject();
	CDataAccess*	pCDataAccess		= SOURCE_GETOBJECT(pCObject, CDataAccess);
	CRowset*		pCRowset			= SOURCE_GETOBJECT(pCDataAccess, CRowset);
	CRow*			pCRow				= SOURCE_GETOBJECT(pCDataAccess, CRow);
	CDataset*		pCDataset			= SOURCE_GETOBJECT(pCDataAccess, CDataset);

	//Object Types
	CBinder*		pCBinder			= SOURCE_GETOBJECT(pCObject, CBinder);
	CCommand*		pCCommand			= m_pCCommand;
	CSession*		pCSession			= m_pCSession;
	CDataSource*	pCDataSource		= m_pCDataSource;
	HCHAPTER hChapter	= pCRowset ? pCRowset->m_hChapter : NULL;

	//Setup the Source info
	m_idSource = iID;
	m_pCSource = pCObject;

	//Most of these commands are "delegates" from CMainWindow, when dealing
	//with the ToolBar or Menu for items dealing with MDI children...
	switch(iID)
	{
		case IDM_IDBINFO_GETLITERALINFO:
			if(SOURCE_GETINTERFACE(pCObject, IDBInfo))
				DisplayDialog(IDD_GETLISTVIEW, hWnd, GetLiteralInfoProc, pCObject, iID);
			return TRUE;
		
		case IDM_PROVIDERINFO:
			if(pCDataSource)
				DisplayDialog(IDD_PROVIDERINFO, hWnd, ProviderInfoProc, pCDataSource, iID);
			return TRUE;
			
		case IDM_BROWSE_TABLENAME:
			if(m_pCQueryBox->m_hWnd)
			{
				WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
				
				//Display Common Dialog to obtain TableName...
				//This is for providers that use a file for TableName
				hr = BrowseOpenFileName(GetAppLite()->m_hInstance, hWnd, L"Browse for TableName", wszBuffer, MAX_QUERY_LEN, NULL, L"DataSource Files (.dsn;.kag;.sav)\0*.dsn;*.kag;*.sav;\0DataBase Files (.mdb;.db;.dbf)\0*.mdb;*.db;*.dbf;\0Program Files (.xls;.clb)\0*.xls;*.clb;\0Text Files (.txt;.csv)\0*.txt;*.csv\0All Files (*.*)\0*.*\0\0");
				if(SUCCEEDED(hr))
				{
					//Now just need to place this name in the EditBox
					//Inserted after the current "caret"
					m_pCQueryBox->ReplaceAll(wszBuffer);
				}
			}
			return TRUE;

		case IDM_IDBINFO_GETKEYWORDS:
			if(pCObject)
			{
				CWaitCursor waitCursor;
				WCHAR* pwszKeywords = NULL;
				IDBInfo* pIDBInfo = SOURCE_GETINTERFACE(pCObject, IDBInfo);
			
				//IDBInfo::GetKeywords
				if(pIDBInfo)
				{
					XTEST(hr = pIDBInfo->GetKeywords(&pwszKeywords));
					TRACE_METHOD(hr, L"IDBInfo::GetKeywords(&\"%s\")", pwszKeywords);
					SAFE_FREE(pwszKeywords);
				}
			}
			return TRUE;

		//Create a Session...
		case IDM_IDBCREATESESSION_CREATESESSION:
			if(pCDataSource)
			{
				CInterfaceDlg interfaceDlg(IDD_AGGREGATION, L"IDBCreateSession::CreateSession", IID_IOpenRowset);
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//Create a new session (in its own window)
					XTEST(hr = pCDataSource->CreateSession(interfaceDlg.GetAggregate(), interfaceDlg.GetSelInterface(), interfaceDlg.ppUnknown()));
					if(SUCCEEDED(hr))
						m_pCMainWindow->HandleObjectType(pCObject, interfaceDlg.pUnknown(), interfaceDlg.GetSelInterface(), eCSession, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
				}
			}
			return TRUE;

		//IDBDataSourceAdmin
		case IDM_IDATASOURCEADMIN_CREATEDATASOURCE:
			if(pCDataSource && pCDataSource->m_pIDBDataSourceAdmin)
				DisplayDialog(IDD_DATASOURCEADMIN_CREATEDATASOURCE, hWnd, AdminCreateDataSourceProc, pCDataSource, iID);
			return TRUE;

		case IDM_IDATASOURCEADMIN_DESTROYDATASOURCE:
			if(pCDataSource && pCDataSource->m_pIDBDataSourceAdmin)
			{
				CWaitCursor waitCursor;

				//IDBDataSourceAdmin::DestroyDataSource
				XTEST(hr = pCDataSource->m_pIDBDataSourceAdmin->DestroyDataSource());
				TRACE_METHOD(hr, L"IDBDataSourceAdmin::DestroyDataSource()");
			}
			return TRUE;

		case IDM_IDATASOURCEADMIN_GETCREATIONPROPERTIES:
			if(pCDataSource && pCDataSource->m_pIDBDataSourceAdmin)
			{
				CPropertiesDlg sCPropertiesDlg(pCMainWindow);
				sCPropertiesDlg.GetPropertyInfo(hWnd, &DBPROPSET_DBINITALL, IID_IDBDataSourceAdmin, NULL, pCDataSource->m_pIDBDataSourceAdmin);
			}
			return TRUE;

		case IDM_IDATASOURCEADMIN_MODIFYDATASOURCE:
			if(pCDataSource && pCDataSource->m_pIDBDataSourceAdmin)
			{
				CPropertiesDlg sCPropertiesDlg(pCMainWindow);
				sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_DBINITALL, IID_IDBDataSourceAdmin, pCDataSource->m_pIDBDataSourceAdmin, NULL, NULL, pCDataSource->m_pIDBDataSourceAdmin);
			}
			return TRUE;

		//IOpenRowset::OpenRowset
		case IDM_OPENROWSET:
			if(pCSession && pCSession->m_pIOpenRowset)
				DisplayDialog(IDD_OPENROWSET, hWnd, OpenRowsetProc, pCSession, iID);
			return TRUE;

		//IGetDataSource::GetDataSource
		case IDM_GETDATASOURCE:
			if(pCSession && pCSession->m_pIGetDataSource)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IGetDataSource::GetDataSource", IID_IDBInitialize);
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//IGetDataSource::GetDataSource
					if(SUCCEEDED(hr = pCSession->GetDataSource(interfaceDlg.GetSelInterface(), interfaceDlg.ppUnknown())))
						m_pCMainWindow->HandleObjectType(pCSession, interfaceDlg.pUnknown(), interfaceDlg.GetSelInterface(), eCDataSource, 0, NULL, CREATE_FINDWINDOW);
				}
			}
			return TRUE;

		//ICommand
		case IDM_EXECUTE:
  			if(pCCommand && pCCommand->m_pICommand)
				DisplayDialog(IDD_EXECUTE, hWnd, ExecuteProc, pCCommand, iID);
			return TRUE;
								
		case IDM_COMMANDTEXT_EXECUTE:
  			if(pCCommand && pCCommand->m_pICommandText)
				DisplayDialog(IDD_EXECUTE, hWnd, ExecuteProc, pCCommand, iID);
			return TRUE;

		case IDM_COMMAND_CANCEL:
			if(pCCommand && pCCommand->m_pICommand)
			{
				CWaitCursor waitCursor;

				//ICommand::Cancel
				XTEST(hr = pCCommand->m_pICommand->Cancel());
				TRACE_METHOD(hr, L"ICommand::Cancel()");
			}
			return TRUE;

		case IDM_COMMAND_GETDBSESSION:
			if(pCCommand && pCCommand->m_pICommand)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"ICommand::GetDBSession", IID_IOpenRowset);
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//ICommand::GetDBSession
					CComPtr<IUnknown> spUnknown;
					XTEST(hr = pCCommand->m_pICommand->GetDBSession(interfaceDlg.GetSelInterface(), &spUnknown));
					TRACE_METHOD(hr, L"ICommand::GetDBSession(%s, &0x%p)", GetInterfaceName(interfaceDlg.GetSelInterface()), spUnknown);
					
					//Update the Session Object
					if(SUCCEEDED(hr))
						m_pCMainWindow->HandleObjectType(pCCommand, spUnknown, interfaceDlg.GetSelInterface(), eCSession, 0, NULL, CREATE_FINDWINDOW);
				}
			}
			return TRUE;

		////////////////////////////////////////////////////////////////////////
		// IMDDataset
		//
		////////////////////////////////////////////////////////////////////////
		case IDM_DATASET_GETAXISINFO:
  			if(pCDataset && pCDataset->m_pIMDDataset)
				DisplayDialog(IDD_GETAXISINFO, hWnd, GetAxisInfoProc, pCDataset, iID);
			return TRUE;

		case IDM_DATASET_GETCELLDATA:
  			if(pCDataset && pCDataset->m_pIMDDataset)
  			{
				INDEX cRows = 0;
				INDEX* rgItems = NULL;
				HROW* rghRows = NULL;
				
				//Find all Selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
				if(cRows == 0)
				{
					wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONHAND | MB_OK | MB_DEFBUTTON1, 
						wsz_ERROR, L"Must first select a row...");
				}

				CWaitCursor waitCursor;

				//Display all rows...
				for(INDEX i=0; i<cRows; i++)
				{
					//Display the Data for this row...
					m_pCDataGrid->DisplayData(rghRows[i], rgItems[i], MDPROP_AXES, true/*fAlways*/);
				}
				SAFE_FREE(rgItems);
				SAFE_FREE(rghRows);
  			}
  			return TRUE;

  		case IDM_DATASET_GETAXISROWSET:
  			if(pCDataset && pCDataset->m_pIMDDataset)
				DisplayDialog(IDD_GETAXISROWSET, hWnd, GetAxisRowsetProc, pCDataset, iID);
			return TRUE;

		case IDM_DATASET_FREEAXISINFO:
  			if(pCDataset && pCDataset->m_pIMDDataset)
				pCDataset->FreeAxisInfo(&pCDataset->m_cAxis, &pCDataset->m_rgAxisInfo);
			return TRUE;

		case IDM_DATASET_GETSPECIFICATION:
  			if(pCDataset && pCDataset->m_pIMDDataset)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IMDDataset::GetSpecification", IID_ICommand);
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//IMDDataset::GetSpecification
					CComPtr<IUnknown> spUnknown;
					XTEST(hr = pCDataset->m_pIMDDataset->GetSpecification(interfaceDlg.GetSelInterface(), &spUnknown));
					TRACE_METHOD(hr, L"IMDDataset::GetSpecification(%s, &0x%p)", GetInterfaceName(interfaceDlg.GetSelInterface()), spUnknown);
					
					//What type of object, let our helper determine...
					//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
					if(SUCCEEDED(hr))
						m_pCMainWindow->HandleObjectType(pCDataset, spUnknown, interfaceDlg.GetSelInterface(), eCCommand, 0, NULL, CREATE_FINDWINDOW | CREATE_DETERMINE_TYPE);
				}
			}
			return TRUE;
		
		
		
		case IDM_IMULTIPLERESULTS_GETRESULT: 
		{
			CMultipleResultsDlg multipleResultsDlg(this);
			multipleResultsDlg.DoModal(hWnd);
			return TRUE;
		}
		
		case IDM_ITRANSACTION_ABORT:
			if(SOURCE_GETINTERFACE(pCObject, ITransaction))
				DisplayDialog(IDD_TRANSACTION_ABORT, hWnd, AbortTransactionProc, pCObject, iID);
			return TRUE;

		case IDM_ITRANSACTION_COMMIT:
			if(SOURCE_GETINTERFACE(pCObject, ITransaction))
				DisplayDialog(IDD_TRANSACTION_COMMIT, hWnd, CommitTransactionProc, pCObject, iID);
			return TRUE;

		case IDM_ITRANSACTION_GETTRANSACTIONINFO:
			if(SOURCE_GETINTERFACE(pCObject, ITransaction))
				DisplayDialog(IDD_TRANSACTION_GETINFO, hWnd, GetTransactionInfo, pCObject, iID);
			return TRUE;

		case IDM_ITRANSACTIONLOCAL_STARTTRANSACTION:
			if(SOURCE_GETINTERFACE(pCObject, ITransactionLocal))
				DisplayDialog(IDD_TRANSACTION_START, hWnd, StartTransactionProc, pCObject, iID);
 			return TRUE;
		
		case IDM_ITRANSACTIONLOCAL_GETOPTIONSOBJECT:
		{
			ITransactionLocal* pITransactionLocal = SOURCE_GETINTERFACE(pCObject, ITransactionLocal);
			if(pITransactionLocal)
			{
				CWaitCursor waitCursor;
				CComPtr<ITransactionOptions>	spTransactionOptions;

				//ITransactionLocal::GetOptionsObject
				XTEST(hr = pITransactionLocal->GetOptionsObject(&spTransactionOptions));
				TRACE_METHOD(hr, L"ITransactionLocal::GetOptionsObject(&0x%p)", spTransactionOptions);

				//Delegate
				if(SUCCEEDED(hr))
					m_pCMainWindow->HandleObjectType(pCObject, spTransactionOptions, IID_ITransactionOptions, eCTransactionOptions, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
			}
			return TRUE;
		}

		case IDM_ITRANSACTIONOBJECT_GETTRANSACTIONOBJECT:
		{
			ITransactionObject* pITransactionObject = SOURCE_GETINTERFACE(pCObject, ITransactionObject);
			if(pITransactionObject)
			{
				CWaitCursor waitCursor;
				CComPtr<ITransaction> spTransaction;

				//ITransactionObject::GetTransactionObject
				XTEST(hr = pITransactionObject->GetTransactionObject(1, &spTransaction));
				TRACE_METHOD(hr, L"ITransactionObject::GetTransactionObject(1, &0x%p)", spTransaction);

				//Delegate
				if(SUCCEEDED(hr))
					m_pCMainWindow->HandleObjectType(pCObject, spTransaction, IID_ITransaction, eCTransaction, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
			}
			return TRUE;
		}

		case IDM_ITRANSACTIONOPTIONS_GETOPTIONS:
		{
			ITransactionOptions* pITransactionOptions = SOURCE_GETINTERFACE(pCObject, ITransactionOptions);
			if(pITransactionOptions)
			{
				CWaitCursor waitCursor;
				XACTOPT XactOptions = {0};

				//ITransactionOptions::GetOptions
				XTEST(hr = pITransactionOptions->GetOptions(&XactOptions));
				TRACE_METHOD(hr, L"ITransactionOptions::GetOptions(%d, \"%s\")", XactOptions.ulTimeout, XactOptions.szDescription);
			}
			return TRUE;
		}

		case IDM_ITRANSACTIONOPTIONS_SETOPTIONS:
		{
			ITransactionOptions* pITransactionOptions = SOURCE_GETINTERFACE(pCObject, ITransactionOptions);
			if(pITransactionOptions)
				DisplayDialog(IDD_TRANSACTION_SETOPTIONS, hWnd, SetTransactionOptionsProc, pCObject, iID);
			return TRUE;
		}

		case IDM_ITRANSACTIONJOIN_RELEASETRANSACTION:
			DisplayDialog(IDD_TRANSACTION_RELEASE, hWnd, ReleaseTransaction, pCObject, iID);
			return TRUE;

		case IDM_ITRANSACTIONDISPENSOR_BEGINTRANSACTION:
			DisplayDialog(IDD_TRANSACTION_START, hWnd, StartTransactionProc, pCObject, iID);
			return TRUE;
		
		case IDM_ITRANSACTIONJOIN_GETOPTIONSOBJECT:
		{
			ITransactionJoin* pITransactionJoin = SOURCE_GETINTERFACE(pCObject, ITransactionJoin);
			if(pITransactionJoin)
			{
				CWaitCursor waitCursor;
				CComPtr<ITransactionOptions>	spTransactionOptions;

				//ITransactionJoin::GetOptionsObject
				XTEST(hr = pITransactionJoin->GetOptionsObject(&spTransactionOptions));
				TRACE_METHOD(hr, L"ITransactionJoin::GetOptionsObject(&0x%p)", spTransactionOptions);

				//Delegate
				if(SUCCEEDED(hr))
					m_pCMainWindow->HandleObjectType(pCObject, spTransactionOptions, IID_ITransactionOptions, eCTransactionOptions, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
			}
			return TRUE;
		}
	
    	case IDM_ITRANSACTIONJOIN_JOINTRANSACTION:
		{
			ITransactionJoin* pITransactionJoin = SOURCE_GETINTERFACE(pCObject, ITransactionJoin);
			if(pITransactionJoin)
	 			DisplayDialog(IDD_TRANSACTION_JOIN, hWnd, JoinTransactionProc, pCObject, iID);
    		return TRUE;
		}

		case IDM_ICOLUMNSINFO_GETCOLUMNINFO:
			if(pCDataAccess && pCDataAccess->m_pIColumnsInfo)
				DisplayDialog(IDD_GETLISTVIEW, hWnd, GetColInfoProc, pCDataAccess, iID);
			return TRUE;

		//ColumnsRowset
		case IDM_ICOLUMNSROWSET_GETCOLUMNSROWSET:
			if(SOURCE_GETINTERFACE(pCObject, IColumnsRowset))
				DisplayDialog(IDD_GETCOLUMNSROWSET, hWnd, GetColumnsRowsetProc, pCObject, iID);
			return TRUE;

		case IDM_DELETEROWS:    
			if(pCRowset && pCRowset->m_pIRowsetChange)
			{
				CWaitCursor waitCursor;
				DeleteSelectedRows();
			}
			return TRUE;
		
		case IDM_SETDATA:    
			if(pCRowset && pCRowset->m_pIRowsetChange)
				ChangeSelectedRow(pCRowset, IDM_SETDATA);
			return TRUE;

		case IDM_INSERTROW:           
			if(pCRowset && pCRowset->m_pIRowsetChange)
				InsertNewRow();
			return TRUE;

		case IDM_CLEAREDITWINDOW:				
			//Select all the Text and Cut it...
			//This method allows the text to be undone...
			m_pCQueryBox->SetSel(0, -1);
			m_pCQueryBox->Cut();
			return TRUE;

		case IDM_CUTEDITWINDOW:				
			m_pCQueryBox->Cut();
			return TRUE;

		case IDM_COPYEDITWINDOW:				
			m_pCQueryBox->Copy();
			return TRUE;

		case IDM_PASTEEDITWINDOW:
			m_pCQueryBox->Paste();
			return TRUE;

		case IDM_RESTARTPOSITION:					
			if(pCRowset && pCRowset->m_pIRowset)
				m_pCDataGrid->RestartPosition();
			return TRUE;
						
		case IDM_GETNEXTROWS:
			if(pCRowset && pCRowset->m_pIRowset)
				DisplayDialog(IDD_GETNEXTROWS, hWnd, GetNextRowsProc, pCRowset, iID);
			return TRUE;

		case IDM_RELEASEROWS:
			if(pCRowset && pCRowset->m_pIRowset)
				m_pCDataGrid->ReleaseRows(LV_ALLSELITEMS, FALSE/*fOnlyValidRows*/);
			return TRUE;

		case IDM_ADDREFROWS:
			if(pCRowset && pCRowset->m_pIRowset)
				m_pCDataGrid->AddRefRows(LV_ALLSELITEMS);
			return TRUE;

		case IDM_RELEASEALLROWS:
			if(pCRowset && pCRowset->m_pIRowset)
				m_pCDataGrid->ReleaseRows(LV_ALLITEMS, FALSE/*fOnlyValidRows*/);
			return TRUE;

		case IDM_GETROWFROMHROW:
			if(pCRowset && pCRowset->m_pIGetRow)
			{
				//Obtain the First selected Row
				HROW hRow = NULL;
				if(m_pCDataGrid->GetSelectedRow(&hRow) == LVM_ERR)
					return TRUE;

				CComPtr<IUnknown> spUnknown;
				CInterfaceDlg interfaceDlg(IDD_AGGREGATION, L"IGetRow::GetRowFromHROW", IID_IRow); 
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//IGetRow::GetRowFromHROW
					if(SUCCEEDED(pCRowset->GetRowFromHROW(NULL, hRow, interfaceDlg.GetSelInterface(), &spUnknown)))
					{
						//Create in new window
						CBase* pCObject = m_pCMainWindow->HandleObjectType(pCRowset, spUnknown, interfaceDlg.GetSelInterface(), eCRow, 0, NULL, CREATE_NEWWINDOW | CREATE_NODISPLAY);
						if(pCObject)
						{
							//Fill in which row handle this row was created from...
							//Just for display purposes...
							CRow* pCRow = SOURCE_GETOBJECT(pCObject, CRow);
							if(pCRow)
								pCRow->m_hSourceRow = hRow;
							
							pCObject->DisplayObject();
						}
					}
				}
			}
			return TRUE;

		case IDM_GETURLFROMHROW:
			if(pCRowset && pCRowset->m_pIGetRow)
			{
				CWaitCursor waitCursor;
				WCHAR* pwszURL = NULL;

				//Obtain the First selected Row
				HROW hRow = NULL;
				if(m_pCDataGrid->GetSelectedRow(&hRow) == LVM_ERR)
					return TRUE;

				//IGetRow::GetURLFromHROW
				XTEST(hr = pCRowset->m_pIGetRow->GetURLFromHROW(hRow, &pwszURL));
				TRACE_METHOD(hr, L"IGetRow::GetURLFromHROW(0x%p, \"%s\")", hRow, pwszURL);

				//Update our saved Binding...
				if(pCBinder)
				{
					SAFE_FREE(pCBinder->m_pwszURL);
					pCBinder->m_pwszURL = pwszURL;
				}
			}
			return TRUE;

		//IRow - methods
		case IDM_IROW_GETCOLUMNS: 
			if(SOURCE_GETINTERFACE(pCObject, IRow))
			{
				CWaitCursor waitCursor;

				//Display the Data for this row...
				m_pCDataGrid->DisplayData(0, 1, DBPROP_IRow, true/*fAlways*/);
			}
			return TRUE;

		case IDM_IROW_GETSOURCEROWSET:
		{
			IRow* pIRow = SOURCE_GETINTERFACE(pCObject, IRow);
			if(pIRow)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IRow::GetSourceRowset", IID_IRowset); 
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;
					CComPtr<IUnknown> spUnknown;
					HROW hRow = NULL;
					
					//IRow::GetSourceRowset
					XTEST(hr = pIRow->GetSourceRowset(interfaceDlg.GetSelInterface(), &spUnknown, &hRow));
					TRACE_METHOD(hr, L"IRow::GetSourceRowset(%s, &0x%p, &0x%p)", GetInterfaceName(interfaceDlg.GetSelInterface()), spUnknown, hRow);
					
					//Update the Rowset Object
					//NOTE: We don't want to popup a new window if the parent rowset 
					//is already active in its own window.  We will first try to find the 
					//parent, otherwise activate a new window...
					if(SUCCEEDED(hr))
						m_pCMainWindow->HandleObjectType(pCObject, spUnknown, interfaceDlg.GetSelInterface(), eCRowset, 0, NULL, CREATE_FINDWINDOW | CREATE_RESTARTPOSITION);

					//Release the Row Handle...
					CRowset* pCRowset = SOURCE_GETPARENT(pCObject, CRowset);
					if(pCRowset)
						pCRowset->ReleaseRows(1, &hRow, NULL);
				}
			}
			return TRUE;
		}

		case IDM_IDBCREATECOMMAND_CREATECOMMAND:
			if(SOURCE_GETINTERFACE(pCObject, IDBCreateCommand))
			{
				CInterfaceDlg interfaceDlg(IDD_AGGREGATION, L"IDBCreateCommand::CreateCommand", IID_ICommandText); 
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;
					CSession* pCSession = SOURCE_GETOBJECT(pCObject, CSession);
					CRow*	pCRow		= SOURCE_GETOBJECT(pCObject, CRow);
					ASSERT(pCSession || pCRow);

					//Create a new Command Object (with default properties)
					if(pCSession)
						hr = pCSession->CreateCommand(interfaceDlg.GetAggregate(), interfaceDlg.GetSelInterface(), interfaceDlg.ppUnknown());
					else
						hr = pCRow->CreateCommand(interfaceDlg.GetAggregate(), interfaceDlg.GetSelInterface(), interfaceDlg.ppUnknown());

					if(SUCCEEDED(hr))
						m_pCMainWindow->HandleObjectType(pCObject, interfaceDlg.pUnknown(), interfaceDlg.GetSelInterface(), eCCommand, 0, NULL, CREATE_NEWWINDOW_IFEXISTS);
				}
			}
			return TRUE;

			//IRowChange - methods
		case IDM_IROW_SETCOLUMNS: 
			if(pCRow && pCRow->m_pIRowChange)
				ChangeSelectedRow(pCRow, IDM_IROW_SETCOLUMNS);
			return TRUE;

		//IRowSchemaChange - methods
		case IDM_IROW_DELETECOLUMNS: 
			if(pCRow && pCRow->m_pIRowSchemaChange)
				ChangeSelectedRow(pCRow, IDM_IROW_DELETECOLUMNS);
			return TRUE;

		case IDM_IGETSESSION_GETSESSION: 
		{
			IGetSession* pIGetSession = SOURCE_GETINTERFACE(pCObject, IGetSession);
			if(pIGetSession)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IGetSession::GetSession", IID_IOpenRowset); 
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//IGetSession::GetSession
					CComPtr<IUnknown>	spUnknown;
					XTEST(hr = pIGetSession->GetSession(interfaceDlg.GetSelInterface(), &spUnknown));
					TRACE_METHOD(hr, L"IGetSession::GetSession(%s, &0x%p)", GetInterfaceName(interfaceDlg.GetSelInterface()), spUnknown);
					
					//Update the Session
					if(SUCCEEDED(hr))
						m_pCMainWindow->HandleObjectType(pCObject, spUnknown, interfaceDlg.GetSelInterface(), eCSession, 0, NULL, CREATE_FINDWINDOW);
				}
			}
			return TRUE;
		}

		case IDM_IROW_OPEN: 
			if(SOURCE_GETINTERFACE(pCObject, IRow))
				DisplayDialog(IDD_ROW_OPEN, hWnd, RowOpenProc, pCObject, iID);
			return TRUE;
			
		case IDM_ISCOPEDOPERATIONS_OPENROWSET:
			if(SOURCE_GETINTERFACE(pCObject, IScopedOperations))
				DisplayDialog(IDD_OPENROWSET, hWnd, OpenRowsetProc, pCObject, iID);
			return TRUE;

		case IDM_ISCOPEDOPERATIONS_DELETE:
			if(SOURCE_GETINTERFACE(pCObject, IScopedOperations))
				DisplayDialog(IDD_ISCO_DELETE, hWnd, ISCO_DeleteProc, pCObject, iID);
			return TRUE;

		case IDM_ISCOPEDOPERATIONS_COPY:
			if(SOURCE_GETINTERFACE(pCObject, IScopedOperations))
				DisplayDialog(IDD_ISCO_COPY, hWnd, ISCO_Proc, pCObject, iID);
			return TRUE;

		case IDM_ISCOPEDOPERATIONS_MOVE:
			if(SOURCE_GETINTERFACE(pCObject, IScopedOperations))
				DisplayDialog(IDD_ISCO_COPY, hWnd, ISCO_Proc, pCObject, iID);
			return TRUE;

		case IDM_SETCOMMANDTEXT:
			if(pCCommand && pCCommand->m_pICommandText)
				DisplayDialog(IDD_SETCOMMANDTEXT, hWnd, SetCommandTextProc, pCCommand, iID);
			return TRUE;

		case IDM_GETCOMMANDTEXT:
			if(pCCommand && pCCommand->m_pICommandText)
			{
				CWaitCursor waitCursor;

				GUID guidDialec = GUID_NULL;
				WCHAR wszBuffer[MAX_NAME_LEN+1] = {0};
				WCHAR* pwszDialect = NULL;
				WCHAR* pwszBuffer = NULL;
				
				XTEST(hr = pCCommand->m_pICommandText->GetCommandText(&guidDialec, &pwszBuffer));
				
				//Try to find a string respentation of the guidDialec
				if(!(pwszDialect = GetDialectName(guidDialec)))
				{
					pwszDialect = wszBuffer;
					StringFromGUID2(guidDialec, wszBuffer, MAX_NAME_LEN);
				}

				TRACE_METHOD(hr, L"ICommandText::GetCommandText(&%s, &\"%s\")", pwszDialect, pwszBuffer);
				SAFE_FREE(pwszBuffer);
			}
			return TRUE;

		case IDM_SETCOMMANDSTREAM:
			if(pCCommand && pCCommand->m_pICommandStream)
				DisplayDialog(IDD_SETCOMMANDSTREAM, hWnd, SetCommandTextProc, pCCommand, iID);
			return TRUE;

		case IDM_GETCOMMANDSTREAM:
			if(pCCommand && pCCommand->m_pICommandStream)
			{
				CWaitCursor waitCursor;

				GUID guidDialec = GUID_NULL;
				IID  iid = IID_NULL;
				CComPtr<IUnknown>	spUnknown;
				WCHAR wszBuffer[MAX_NAME_LEN+1] = {0};
				WCHAR* pwszDialect = NULL;

				//ICommandStream::GetCommandStream
				XTEST(hr = pCCommand->m_pICommandStream->GetCommandStream(&iid, &guidDialec, (IUnknown**)&spUnknown));

				//Try to find a string respentation of the guidDialec
				if(!(pwszDialect = GetDialectName(guidDialec)))
				{
					pwszDialect = wszBuffer;
					StringFromGUID2(guidDialec, wszBuffer, MAX_NAME_LEN);
				}
				TRACE_METHOD(hr, L"ICommandStream::GetCommandStream(&%s, &%s, &0x%p)", GetInterfaceName(iid), pwszDialect, spUnknown);

				//Stand Alone Object (no window required)
				if(SUCCEEDED(hr))
				{
					CStream* pCStream = new CStream(m_pCMainWindow);
					pCStream->CreateObject(pCCommand, iid, spUnknown);
				}
			}
			return TRUE;

		case IDM_COMMANDPERSIST_GETCURRENTCOMMAND:
			if(pCCommand && pCCommand->m_pICommandPersist)
			{
				CWaitCursor waitCursor;
				DBID* pCommandID = NULL;
				
				//ICommandPersist::GetCurrentCommand
				XTEST(pCCommand->GetCurrentCommand(&pCommandID));

				DBIDFree(pCommandID);
				SAFE_FREE(pCommandID);
			}
			return TRUE;

		case IDM_COMMANDPERSIST_DELETECOMMAND:
		case IDM_COMMANDPERSIST_LOADCOMMAND:
		case IDM_COMMANDPERSIST_SAVECOMMAND:
			if(pCCommand && pCCommand->m_pICommandPersist)
				DisplayDialog(IDD_COMMANDPERSIST, hWnd, CommandPersistProc, pCCommand, iID);
			return TRUE;

		case IDM_COMMANDPREPARE:
			if(pCCommand && pCCommand->m_pICommandPrepare)
			{
				CWaitCursor waitCursor;

				//ICommand::Prepare
				XTEST(hr = pCCommand->Prepare(0));
			}
			return TRUE;

		case IDM_COMMANDUNPREPARE:
			if(pCCommand && pCCommand->m_pICommandPrepare)
			{
				CWaitCursor waitCursor;

				//ICommand::Unprepare
				XTEST(hr = pCCommand->m_pICommandPrepare->Unprepare());
				TRACE_METHOD(hr, L"ICommandPrepare::Unprepare()");
			}
			return TRUE;

		case IDM_GETSCHEMAS:
		case IDM_GETSCHEMAROWSET:
			if(pCSession && pCSession->m_pIDBSchemaRowset)
			{
				CSchemaDlg	schemaDlg(this);
				schemaDlg.Display();
			}
			return TRUE;
		
		case IDM_ISESSIONPROPERTIES_GETPROPERTIES:
			if(pCSession && pCSession->m_pISessionProperties)
			{
				CPropertiesDlg sCPropertiesDlg(pCMainWindow);
				sCPropertiesDlg.GetProperties(hWnd, &DBPROPSET_SESSIONALL, IID_ISessionProperties, pCSession->m_pISessionProperties, pCDataSource ? pCDataSource->m_pIDBProperties : NULL);
			}
			return TRUE;

		case IDM_COMMAND_GETPROPERTIES:
			if(pCCommand && pCCommand->m_pICommandProperties)
			{
				CPropertiesDlg sCPropertiesDlg(pCMainWindow);
				sCPropertiesDlg.GetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_ICommandProperties, pCCommand->m_pICommandProperties, pCDataSource ? pCDataSource->m_pIDBProperties : NULL);
			}
			return TRUE;

		case IDM_IROWSET_GETPROPERTIES:
			if(pCRowset && pCRowset->m_pIRowsetInfo)
			{
				CPropertiesDlg sCPropertiesDlg(pCMainWindow);
				sCPropertiesDlg.GetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_IRowsetInfo, pCRowset->m_pIRowsetInfo, pCDataSource ? pCDataSource->m_pIDBProperties : NULL);
			}
			return TRUE;

		//IRowsetInfo
		case IDM_GETSPECIFICATION:
			if(pCRowset && pCRowset->m_pIRowsetInfo)
			{
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IRowsetInfo::GetSpecification", IID_IOpenRowset); 
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;

					//IRowset::GetSpecification
					CComPtr<IUnknown> spUnknown;
					XTEST(hr = pCRowset->m_pIRowsetInfo->GetSpecification(interfaceDlg.GetSelInterface(), &spUnknown));
					TRACE_METHOD(hr, L"IRowset::GetSpecification(%s, &0x%p)", GetInterfaceName(interfaceDlg.GetSelInterface()), spUnknown);
					
					//What type of object, let our helper determine...
					//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
					if(SUCCEEDED(hr))
						m_pCMainWindow->HandleObjectType(pCRowset, spUnknown, interfaceDlg.GetSelInterface(), eCSession, 0, NULL, CREATE_FINDWINDOW | CREATE_DETERMINE_TYPE);
				}
			}
			return TRUE;

		case IDM_GETREFERENCEDROWSET:
			if(pCRowset && pCRowset->m_pIRowsetInfo)
				DisplayDialog(IDD_GETREFERENCEDROWSET, hWnd, GetReferencedRowsetProc, pCRowset, iID);
			return TRUE;

		case IDM_ISESSIONPROPERTIES_SETPROPERTIES:
			if(pCSession && pCSession->m_pISessionProperties)
			{
				pCDataSource = SOURCE_GETPARENT(pCSession, CDataSource);
				
				CPropertiesDlg sCPropertiesDlg(pCMainWindow);
				sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_SESSIONALL, IID_ISessionProperties, pCSession->m_pISessionProperties, pCDataSource ? pCDataSource->m_pIDBProperties : NULL);
			}
			return TRUE;

		case IDM_COMMAND_SETPROPERTIES:
			if(pCCommand && pCCommand->m_pICommandProperties)
			{
				pCDataSource = SOURCE_GETPARENT(pCCommand, CDataSource);
				
				CPropertiesDlg sCPropertiesDlg(pCMainWindow);
				sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_ICommandProperties, pCCommand->m_pICommandProperties, pCDataSource ? pCDataSource->m_pIDBProperties : NULL);
			}
			return TRUE;

		case IDM_COMMAND_CLEARCOMMAND:
			if(pCCommand && pCSession && pCCommand->m_pIUnknown)
			{
				CWaitCursor waitCursor;
				CComPtr<ICommand>	spCommand;
				
				//A little more difficult than ROWSET_CLEARPROPERTIES
				//Since any properties set on a command object are set,
				//the only way to remove them is to either try and set them
				//to the opposite value, or create a new command object...

				//Create a new command 
				//And then re-create our object with the new command (with no properties)...
				pCSession->CreateCommand(NULL, IID_ICommand, (IUnknown**)&spCommand);
				pCCommand->CreateObject(pCSession, IID_ICommand, spCommand);
			}
			return TRUE;
		
		///////////////////////////////////////////////////////////////////
		// IConvertType
		//
		///////////////////////////////////////////////////////////////////
		case IDM_ICONVERTTYPE_CANCONVERT:
			if(pCDataAccess->m_pIConvertType)
				DisplayDialog(IDD_CANCONVERT, hWnd, CanConvertProc, pCDataAccess, iID);
			return TRUE;


		///////////////////////////////////////////////////////////////////
		// IAccessor
		//
		///////////////////////////////////////////////////////////////////
		case IDM_IACCESSOR_GETBINDINGS: 
			if(pCDataAccess && pCDataAccess->m_pIAccessor)
				DisplayDialog(IDD_GETLISTVIEW, hWnd, GetBindingsProc, pCDataAccess, iID);
			return TRUE;

		case IDM_IACCESSOR_ADDREFACCESSOR: 
			if(pCDataAccess && pCDataAccess->m_pIAccessor)
			{
				CWaitCursor waitCursor;

				//IAccessor::AddRefAccessor
				pCDataAccess->AddRefAccessor(pCDataAccess->m_hAccessor);
			}
			return TRUE;
		
		case IDM_IACCESSOR_CREATEACCESSOR: 
			if(pCDataAccess && pCDataAccess->m_pIAccessor)
				DisplayDialog(IDD_CREATEACCESSOR, hWnd, CreateAccessorProc, pCDataAccess, iID);
			return TRUE;

		case IDM_IACCESSOR_RELEASEACCESSOR: 
			if(pCDataAccess && pCDataAccess->m_pIAccessor)
				pCDataAccess->ReleaseAccessor(&pCDataAccess->m_hAccessor, TRUE/*fReleaseAlways*/);
			return TRUE;


		//ITableDefinition
		case IDM_TABLE_CREATETABLE:
			if(pCSession && pCSession->m_pITableDefinition)
				DisplayDialog(IDD_CREATETABLE, hWnd, CreateTableProc, pCSession, iID);
			return TRUE;

		case IDM_TABLE_ADDCOLUMN:
			if(pCSession && pCSession->m_pITableDefinition)
				DisplayDialog(IDD_ADDCOLUMN, hWnd, AddColumnProc, pCSession, iID);
			return TRUE;

		case IDM_TABLE_DROPCOLUMN:
			if(pCSession && pCSession->m_pITableDefinition)
				DisplayDialog(IDD_DROPCOLUMN, hWnd, DropColumnProc, pCSession, iID);
			return TRUE;

		case IDM_TABLE_DROPTABLE:
			if(pCSession && pCSession->m_pITableDefinition)
				DisplayDialog(IDD_DROPTABLE, hWnd, DropTableProc, pCSession, iID);
			return TRUE;

		//ITableDefinitionWithConstraints
		case IDM_TABLE_CREATETABLEWITHCONSTRAINTS:
//			if(pCSession && pCSession->m_pITableDefinitionWithConstraints)
//				DisplayDialog(IDD_CREATETABLE, hWnd, CreateTableProc, pCSession, iID);
			return TRUE;

		case IDM_TABLE_ADDCONSTRAINT:
			if(pCSession && pCSession->m_pITableDefinitionWithConstraints)
			{
				CWaitCursor waitCursor;
				DBORDINAL			cConsDesc = 1;
				DBCONSTRAINTDESC	ConsDesc;
				DBCONSTRAINTDESC*	rgConsDesc = &ConsDesc;
				DBID				TableID;
				WCHAR				*pwszTableName = NULL;
				CConstraintDlg		ConsDlg(pCMainWindow);

				memset(&ConsDesc, 0, sizeof(DBCONSTRAINTDESC));
				
				if (S_OK == ConsDlg.SetConstraintAndTable(hWnd, &ConsDesc, &pwszTableName))
				{
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = pwszTableName;
					XTEST(hr = pCSession->m_pITableDefinitionWithConstraints->AddConstraint(&TableID, &ConsDesc));
					
					TRACE_METHOD(hr, L"ITableDefinitionWithConstraints::AddConstraint(&%s, \"%p\")", pwszTableName, &ConsDesc);
				}
				SAFE_FREE(pwszTableName);
				FreeConstraintDesc(&cConsDesc, &rgConsDesc, FALSE);

			}
			return TRUE;

		case IDM_TABLE_DROPCONSTRAINT:
			if(pCSession && pCSession->m_pITableDefinitionWithConstraints)
				DisplayDialog(IDD_DROPCONSTRAINT, hWnd, DropConstraintProc, pCSession, iID);
			return TRUE;

		//IIndexDefinition
		case IDM_INDEX_CREATEINDEX:
			if(pCSession && pCSession->m_pIIndexDefinition)
				DisplayDialog(IDD_CREATEINDEX, hWnd, CreateIndexProc, pCSession, iID);
			return TRUE;

		case IDM_INDEX_DROPINDEX:
			if(pCSession && pCSession->m_pIIndexDefinition)
				DisplayDialog(IDD_DROPINDEX, hWnd, DropIndexProc, pCSession, iID);
			return TRUE;

		// IAlterIndex
		case IDM_INDEX_ALTERINDEX:
			if(pCSession && pCSession->m_pIAlterIndex)
			{
				CWaitCursor		waitCursor;
				CAlterIndexDlg	AlterIndexDlg(this);
				DBID			*pTableID		= NULL;
				DBID			*pIndexID		= NULL;
				DBID			*pNewIndexID	= NULL;
				BOOL			fUseIndexProps	= FALSE;
				ULONG			cPropSets		= 0;
				DBPROPSET*		rgPropSets		= NULL;

				// get index updates
				if (S_OK == AlterIndexDlg.AlterIndex(hWnd, &pTableID, &pIndexID, &pNewIndexID, 
					&fUseIndexProps))
				{
					//alter the index
					if (fUseIndexProps)
					{
						cPropSets	= AlterIndexDlg.m_CPropSets.GetCount();
						rgPropSets	= AlterIndexDlg.m_CPropSets.GetPropSets();
					}

					XTEST(hr = pCSession->m_pIAlterIndex->AlterIndex(pTableID, pIndexID,
						pNewIndexID, cPropSets, rgPropSets));
					
					TRACE_METHOD(hr, L"IAlterIndex::AlterIndex(&%p, &%p, &%p, %d, %p)", 
						pTableID, pIndexID, pNewIndexID, cPropSets, rgPropSets);
				}

				DBIDFree(pTableID);
				SAFE_FREE(pTableID);
				DBIDFree(pIndexID);
				SAFE_FREE(pIndexID);
				DBIDFree(pNewIndexID);
				SAFE_FREE(pNewIndexID);
			}
			return TRUE;


		// IAlterTable
		case IDM_TABLE_ALTERTABLE:
			if(pCSession && pCSession->m_pIAlterTable)
			{
				CWaitCursor		waitCursor;
				CAlterTableDlg	AlterTableDlg(this);
				ULONG			cPropSets		= 0;
				DBPROPSET*		rgPropSets		= NULL;

				// get table updates
				if (S_OK == AlterTableDlg.AlterTable(hWnd))
				{		
					//alter table
					if (AlterTableDlg.m_fUseTableProps)
					{
						cPropSets	= AlterTableDlg.m_CPropSets.GetCount();
						rgPropSets	= AlterTableDlg.m_CPropSets.GetPropSets();
					}

					XTEST(hr = pCSession->m_pIAlterTable->AlterTable(
						AlterTableDlg.m_pTableID, 
						AlterTableDlg.m_pNewTableID, 
						cPropSets, 
						rgPropSets));
					
					TRACE_METHOD(hr, L"IAlterTable::AlterTable(&%p, &%p, %d, %p)", 
						AlterTableDlg.m_pTableID, AlterTableDlg.m_pNewTableID, cPropSets, rgPropSets);
				}
			}
			return TRUE;

		case IDM_TABLE_ALTERCOLUMN:
			if(pCSession && pCSession->m_pIAlterTable)
			{
				CWaitCursor		waitCursor;
				CAlterColumnDlg	AlterColumnDlg(this);

				// get table updates
				if (S_OK == AlterColumnDlg.AlterColumn(hWnd))
				{
					//alter column
					XTEST(hr = pCSession->m_pIAlterTable->AlterColumn(
						AlterColumnDlg.m_pTableID, 
						AlterColumnDlg.m_pColumnID, 
						AlterColumnDlg.m_dwColFlags, 
						&AlterColumnDlg.m_ColDesc));
					
					TRACE_METHOD(hr, L"IAlterTable::AlterColumn(&%p, &%p, %d, %p)", 
						AlterColumnDlg.m_pTableID, AlterColumnDlg.m_pColumnID, 
						AlterColumnDlg.m_dwColFlags, &AlterColumnDlg.m_ColDesc);
				}
			}
			return TRUE;


		//IRowsetIdentity
		case IDM_IROWSETINDENTITY_ISSAMEROW:
			if(pCRowset && pCRowset->m_pIRowsetIdentity)
			{
				CWaitCursor waitCursor;

				//Obtain the First selected Row
				INDEX iSelRow		= m_pCDataGrid->GetSelectedRow(NULL, FALSE);
				HROW hRow1			= m_pCDataGrid->GetItemParam(iSelRow);

				//Obtain the Second selected Row
				INDEX iSelRow2		= m_pCDataGrid->GetNextItem(iSelRow, LVNI_SELECTED);
				HROW hRow2			= m_pCDataGrid->GetItemParam(iSelRow2 != LVM_ERR ? iSelRow2 : iSelRow);
				
				//IRowsetIdentity::IsSameRow
				XTEST(hr = pCRowset->m_pIRowsetIdentity->IsSameRow(hRow1, hRow2));
				TRACE_METHOD(hr, L"IRowsetIdentity::IsSameRow(0x%p, 0x%p)", hRow1, hRow2);
			}
			return TRUE;

		
		//IRowsetScroll
		case IDM_IROWSETSCROLL_GETAPPROXIMATEPOSITION:
			if(pCRowset && pCRowset->m_pIRowsetScroll)
			{
				CWaitCursor waitCursor;
				DBBKMARK cbBookmark = 0;
				BYTE* pBookmark = NULL;
				DBCOUNTITEM ulPosition = 0;
				DBCOUNTITEM cRows = 0;
				HROW  hRow = NULL;

				//Obtain the bookmark for the selected row...
				if(m_pCDataGrid->GetSelectedRow(&hRow, FALSE)!=LVM_ERR)
					hr = pCRowset->GetBookmark(hRow, &cbBookmark, &pBookmark);
				
				//IRowsetScroll::GetApproximatePosition
				XTEST(hr = pCRowset->m_pIRowsetScroll->GetApproximatePosition(hChapter, cbBookmark, pBookmark, &ulPosition, &cRows));
				TRACE_METHOD(hr, L"IRowsetScroll::GetApproximatePosition(0x%p, %lu, 0x%p, &%lu, &%lu)", hChapter, cbBookmark, pBookmark, ulPosition, cRows);

				//Cleanup
				SAFE_FREE(pBookmark);
			}
			return TRUE;


		//IRowsetLocate
		case IDM_IROWSETLOCATE_COMPARE:
			if(pCRowset && pCRowset->m_pIRowsetLocate)
			{
				CWaitCursor waitCursor;

				HROW  rghRows[2] = {NULL, NULL};
				DBBKMARK rgcbBookmarks[2] = {0, 0};
				const BYTE* rgpBookmarks[2] = {NULL, NULL};
				DBCOMPARE dwComparison = 0;

				//Obtain the First selected Row
				INDEX iSelRow		= m_pCDataGrid->GetSelectedRow(NULL, FALSE);
				rghRows[0]			= m_pCDataGrid->GetItemParam(iSelRow);

				//Obtain the Second selected Row
				INDEX iSelRow2		= m_pCDataGrid->GetNextItem(iSelRow, LVNI_SELECTED);
				rghRows[1]			= m_pCDataGrid->GetItemParam(iSelRow2 != LVM_ERR ? iSelRow2 : iSelRow);
				
				//Obtain the bookmark(s) for the selected row(s)...
				if(iSelRow!=LVM_ERR)
				{
					hr = pCRowset->GetBookmark(rghRows[0], &rgcbBookmarks[0], (BYTE**)&rgpBookmarks[0]);
					hr = pCRowset->GetBookmark(rghRows[1], &rgcbBookmarks[1], (BYTE**)&rgpBookmarks[1]);
				}

				//IRowsetLocate::Compare
				XTEST(hr = pCRowset->m_pIRowsetLocate->Compare(hChapter, rgcbBookmarks[0], rgpBookmarks[0], rgcbBookmarks[1], rgpBookmarks[1], &dwComparison));
				TRACE_METHOD(hr, L"IRowsetLocate::Compare(0x%p, %Iu, 0x%p, %Iu, 0x%p, &%d)", hChapter, rgcbBookmarks[0], rgpBookmarks[0], rgcbBookmarks[1], rgpBookmarks[1], dwComparison);

				//Cleanup
				CoTaskMemFree((void*)rgpBookmarks[0]);
				CoTaskMemFree((void*)rgpBookmarks[1]);
			}
			return TRUE;

		case IDM_IROWSETLOCATE_HASH:
			if(pCRowset && pCRowset->m_pIRowsetLocate)
			{
				CWaitCursor waitCursor;

				INDEX cRows = 0;
				HROW* rghRows = NULL;
				DBBKMARK rgcbBookmarks[MAX_OPENROWS+1];
				const BYTE* rgpBookmarks[MAX_OPENROWS+1];
				DBHASHVALUE rgHashedValues[MAX_OPENROWS+1];
				DBROWSTATUS rgBookmarkStatus[MAX_OPENROWS+1];

				//Obtain all Selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, NULL, (LPARAM**)&rghRows);
				cRows = min(cRows, MAX_OPENROWS);

				INDEX i;

				//Obtain the bookmark(s) for the selected row(s)...
				for(i=0; i<cRows; i++)
				{
					hr = pCRowset->GetBookmark(rghRows[i], &rgcbBookmarks[i], (BYTE**)&rgpBookmarks[i]);
				}

				//IRowsetLocate::Hash
				XTEST(hr = pCRowset->m_pIRowsetLocate->Hash(hChapter, cRows, rgcbBookmarks, rgpBookmarks, rgHashedValues, rgBookmarkStatus));
				TRACE_METHOD(hr, L"IRowsetLocate::Hash(0x%p, %Iu, 0x%p, 0x%p, 0x%p, 0x%p)", hChapter, cRows, rgcbBookmarks, rgpBookmarks, rgHashedValues, rgBookmarkStatus);

				//Cleanup
				SAFE_FREE(rghRows);
				for(i=0; i<cRows; i++)
					CoTaskMemFree((void*)rgpBookmarks[i]);
			}
			return TRUE;

		case IDM_IROWSETLOCATE_GETROWSBYBOOKMARK:
			if(pCRowset && pCRowset->m_pIRowsetLocate)
			{
				CWaitCursor waitCursor;

				INDEX cSelItems = 0;
				INDEX* rgSelItems = 0;
				HROW* rghRows = NULL;
				DBBKMARK rgcbBookmarks[MAX_OPENROWS+1];
				const BYTE* rgpBookmarks[MAX_OPENROWS+1];
				DBROWSTATUS rgRowStatus[MAX_OPENROWS+1];

				//Obtain all Selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cSelItems, &rgSelItems, (LPARAM**)&rghRows);
				cSelItems = min(cSelItems, MAX_OPENROWS);

				INDEX i;
				//Obtain the bookmark(s) for the selected row(s)...

				for(i=0; i<cSelItems; i++)
					hr = pCRowset->GetBookmark(rghRows[i], &rgcbBookmarks[i], (BYTE**)&rgpBookmarks[i]);

				//Release previously fetched rows, (if user requested)
				hr = m_pCDataGrid->ReleaseHeldRows();

				//IRowsetLocate::GetRowsByBookmark
				XTEST(hr = pCRowset->m_pIRowsetLocate->GetRowsByBookmark(hChapter, cSelItems, rgcbBookmarks, rgpBookmarks, rghRows, rgRowStatus));
				TRACE_METHOD(hr, L"IRowsetLocate::GetRowsByBookmark(0x%p, %Iu, 0x%p, 0x%p, 0x%p, 0x%p)", hChapter, cSelItems, rgcbBookmarks, rgpBookmarks, rghRows, rgRowStatus);

				//Display the data rows retrieved...
				for(i=0; i<cSelItems; i++)
				{
					//Some rows might have not been successfully retrived,
					//According to the spec, this is denoted with a NULL hRow
					if(rghRows[i])
						m_pCDataGrid->DisplayData(rghRows[i], rgSelItems[i]);
				}

				//Cleanup
				for(i=0; i<cSelItems; i++)
					CoTaskMemFree((void*)rgpBookmarks[i]);
				SAFE_FREE(rghRows);
				SAFE_FREE(rgSelItems);
			}
			return TRUE;
			
		case IDM_IROWSETBOOKMARK_POSITIONONBOOKMARK:
			if(pCRowset && pCRowset->m_pIRowsetBookmark)
			{
				CWaitCursor waitCursor;

				HROW  hRow = NULL;
				DBBKMARK cbBookmark = 0;
				const BYTE* pBookmark = NULL;

				//Obtain the first selected row...
				INDEX iSelRow = m_pCDataGrid->GetSelectedRow(&hRow, FALSE);
				if(iSelRow != LVM_ERR && hRow)
				{
					//Obtain the bookmark
					if(SUCCEEDED(hr = pCRowset->GetBookmark(hRow, &cbBookmark, (BYTE**)&pBookmark)))
					{
						//IRowsetBookmark::PositionOnBookmark
						XTEST(hr = pCRowset->m_pIRowsetBookmark->PositionOnBookmark(hChapter, cbBookmark, pBookmark));
						TRACE_METHOD(hr, L"IRowsetBookmark::PositionOnBookmark(0x%p, %Iu, 0x%p)", hChapter, cbBookmark, pBookmark);
					
						//Move the NFP indicator...
						if(SUCCEEDED(hr))
						{
							//Display the new Fetch Position indicator...
							m_pCDataGrid->DisplayFetchPosition(iSelRow, TRUE/*fLastFetchForward*/);
						}
					}
				}

				//Cleanup
				SAFE_FREE(pBookmark);
			}
			return TRUE;

		case IDM_IROWSETLOCATE_GETROWSAT:
			if(pCRowset && pCRowset->m_pIRowsetLocate)
				DisplayDialog(IDD_GETNEXTROWS, hWnd, GetNextRowsProc, pCRowset, iID);
			return TRUE;

		//IRowsetFind
		case IDM_IROWSETFIND_FINDNEXTROW:
			if(pCRowset && pCRowset->m_pIRowsetFind)
				DisplayDialog(IDD_FINDNEXTROW, hWnd, FindNextRowProc, pCRowset, iID);
			return TRUE;


		//CommandWithParameters
		case IDM_MAPPARAMETERNAMES: 
			return TRUE;

		case IDM_SETPARAMETERINFO: 
			if(pCCommand && pCCommand->m_pICommandWithParameters)
				DisplayDialog(IDD_SETPARAMETERINFO, hWnd, SetParameterInfoProc, pCCommand, iID);
			return TRUE;

		case IDM_GETPARAMETERINFO: 
			if(pCCommand && pCCommand->m_pICommandWithParameters)
				DisplayDialog(IDD_GETLISTVIEW, hWnd, GetParameterInfoProc, pCCommand, iID);
			return TRUE;

		case IDM_IPERSIST_GETCLASSID:
			if(pCDataSource && pCDataSource->m_pIPersist)
			{
				CLSID clsid;
				pCDataSource->GetClassID(&clsid);
			}
			return TRUE;
		
		case IDM_IPERSISTFILE_LOAD:
			if(pCDataSource && pCDataSource->m_pIPersistFile)
			{
				WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
				
				//Display Common Dialog to obtain File To Load...
				hr = BrowseOpenFileName(GetAppLite()->m_hInstance, hWnd, L"IPersistFile::Load", wszBuffer, MAX_QUERY_LEN, NULL, L"DataSource Files (.dsn;.kag;.sav)\0*.dsn;*.kag;*.sav;\0DataBase Files (.mdb;.db;.dbf)\0*.mdb;*.db;*.dbf;\0Program Files (.xls;.clb)\0*.xls;*.clb;\0Text Files (.txt;.csv)\0*.txt;*.csv\0All Files (*.*)\0*.*\0\0");
				if(SUCCEEDED(hr))
				{
					CWaitCursor waitCursor;
	
					//IPersistFile::Load
					XTEST(hr = pCDataSource->m_pIPersistFile->Load(wszBuffer, STGM_READ));
					TRACE_METHOD(hr, L"IPersistFile::Load(\"%s\", STGM_READ)", wszBuffer);
				}
			}
			return TRUE;

		case IDM_IPERSISTFILE_SAVE:
			if(pCDataSource && pCDataSource->m_pIPersistFile)
			{
				WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
				
				//Display Common Dialog to obtain File To Save...
				hr = BrowseSaveFileName(GetAppLite()->m_hInstance, hWnd, L"IPersistFile::Save", wszBuffer, MAX_QUERY_LEN, NULL, L"DataSource Files (.dsn;.kag;.sav)\0*.dsn;*.kag;*.sav;\0DataBase Files (.mdb;.db;.dbf)\0*.mdb;*.db;*.dbf;\0Program Files (.xls;.clb)\0*.xls;*.clb;\0Text Files (.txt;.csv)\0*.txt;*.csv\0All Files (*.*)\0*.*\0\0");
				if(SUCCEEDED(hr))
				{
					CWaitCursor waitCursor;
	
					//IPersistFile::Save
					XTEST(hr = pCDataSource->m_pIPersistFile->Save(wszBuffer, TRUE));
					TRACE_METHOD(hr, L"IPersistFile::Save(\"%s\", TRUE)", wszBuffer);
				}
			}
			return TRUE;

		case IDM_IPERSISTFILE_SAVECOMPLETED:
			if(pCDataSource && pCDataSource->m_pIPersistFile)
			{
				WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
				
				//Display Common Dialog to obtain File To Save...
				hr = BrowseSaveFileName(GetAppLite()->m_hInstance, hWnd, L"IPersistFile::SaveCompleted", wszBuffer, MAX_QUERY_LEN, NULL, L"DataSource Files (.dsn;.kag;.sav)\0*.dsn;*.kag;*.sav;\0DataBase Files (.mdb;.db;.dbf)\0*.mdb;*.db;*.dbf;\0Program Files (.xls;.clb)\0*.xls;*.clb;\0Text Files (.txt;.csv)\0*.txt;*.csv\0All Files (*.*)\0*.*\0\0");
				if(SUCCEEDED(hr))
				{
					CWaitCursor waitCursor;
	
					//IPersistFile::Save
					XTEST(hr = pCDataSource->m_pIPersistFile->SaveCompleted(wszBuffer));
					TRACE_METHOD(hr, L"IPersistFile::SaveCompleted(\"%s\")", wszBuffer);
				}
			}
			return TRUE;

		case IDM_IPERSISTFILE_GETCURFILE:
			if(pCDataSource && pCDataSource->m_pIPersistFile)
			{
				CWaitCursor waitCursor;
				WCHAR* pwszFileName = NULL;

				//IPersistFile::GetCurFile
				XTEST(hr = pCDataSource->m_pIPersistFile->GetCurFile(&pwszFileName));
				TRACE_METHOD(hr, L"IPersistFile::GetCurFile(\"%s\")", pwszFileName);
			}
			return TRUE;
		
		case IDM_IPERSISTFILE_ISDIRTY:
			if(pCDataSource && pCDataSource->m_pIPersistFile)
			{		 
				CWaitCursor waitCursor;

				//IPersistFile::IsDirty
				XTEST(hr = pCDataSource->m_pIPersistFile->IsDirty());
				TRACE_METHOD(hr, L"IPersistFile::IsDirty()");
			}
			return TRUE;

		case IDM_GETDATA:
		case IDM_IROWSETLOCATE_GETDATA:
		case IDM_IROWSETSCROLL_GETDATA:
			if(pCRowset && pCRowset->m_pIRowset)
			{
				INDEX cRows = 0;
				INDEX* rgItems = NULL;
				HROW* rghRows = NULL;
				
				//Find all Selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
				if(cRows == 0)
				{
					wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONHAND | MB_OK | MB_DEFBUTTON1, 
						wsz_ERROR, L"Must first select a row...");
				}

				DBPROPID dwSourceID = DBPROP_IRowset;
				if(iID == IDM_IROWSETLOCATE_GETDATA)
					dwSourceID = DBPROP_IRowsetLocate;
				if(iID == IDM_IROWSETSCROLL_GETDATA)
					dwSourceID = DBPROP_IRowsetScroll;
				
				CWaitCursor waitCursor;

				//Display all rows...
				for(INDEX i=0; i<cRows; i++)
				{
					//Display the Data for this row...
					m_pCDataGrid->DisplayData(rghRows[i], rgItems[i], dwSourceID, true/*fAlways*/);
				}
				SAFE_FREE(rgItems);
				SAFE_FREE(rghRows);
			}
			return TRUE;

		case IDM_REFRESH:					
			if(pCRowset && pCRowset->m_pIRowset)
			{
				CWaitCursor waitCursor;

				//Similar to GetData except for all rows
				if(SUCCEEDED(m_pCDataGrid->RestartPosition()))
					m_pCDataGrid->RefreshData();
			}
			return TRUE;
		
		case IDM_GETVISIBLEDATA:
			if(pCRowset && pCRowset->m_pIRowsetResynch)
			{
				INDEX cRows = 0;
				INDEX* rgItems = NULL;
				HROW* rghRows = NULL;
				
				//Find all Selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
				if(cRows == 0)
				{
					wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONHAND | MB_OK | MB_DEFBUTTON1, 
						wsz_ERROR, L"Must first select a row...");
				}

				CWaitCursor waitCursor;
				for(INDEX i=0; i<cRows; i++)
				{
					//Display the Data for this row...
					m_pCDataGrid->DisplayData(rghRows[i], rgItems[i], DBPROP_IRowsetResynch, true/*fAlways*/);
				}
				SAFE_FREE(rgItems);
				SAFE_FREE(rghRows);
			}
			return TRUE;

		case IDM_RESYNCHROWS:
			if(pCRowset && pCRowset->m_pIRowsetResynch)
			{
				CWaitCursor waitCursor;

				INDEX cRows = 0;
				HROW* rghRows = NULL;
				DBCOUNTITEM cRowsResynched = 0;
				HROW* rghRowsResynched = 0;
				DBROWSTATUS* rgRowStatus = NULL;
				INDEX* rgItems = NULL;

				//Get all selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);

				//IRowsetResynch::ResynchRows
				XTEST(hr = pCRowset->m_pIRowsetResynch->ResynchRows(cRows, rghRows, &cRowsResynched, &rghRowsResynched, &rgRowStatus));
				DisplayRowErrors(hr, cRowsResynched, rghRowsResynched, rgRowStatus);
				TRACE_METHOD(hr, L"IRowsetResynch::ResynchRows(%Iu, 0x%p, &%Iu, &0x%p, &0x%p)", cRows, rghRows, cRowsResynched, rghRowsResynched, rgRowStatus);

				//Now GetData for all SelectedRows (a nice helper)
				//We won't worry about updating the case for (0,NULL), its not worth
				//trying to match the rhgRows returned to the correct hRows.  Worse case
				//is the user has to call GetData for one or all rows...
				for(INDEX i=0; i<cRows; i++)
				{
					//Display the Data for this row...
					m_pCDataGrid->DisplayData(rghRows[i], rgItems[i]);
				}

				//Cleanup
				SAFE_FREE(rghRows);
				SAFE_FREE(rghRowsResynched);
				SAFE_FREE(rgRowStatus);
				SAFE_FREE(rgItems);
			}
			return TRUE;

		case IDM_GETLASTVISIBLEDATA:
			if(pCRowset && pCRowset->m_pIRowsetRefresh)
			{
				CWaitCursor waitCursor;

				INDEX cRows = 0;
				INDEX* rgItems = NULL;
				HROW* rghRows = NULL;
				
				//Find all Selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
				if(cRows == 0)
				{
					wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONHAND | MB_OK | MB_DEFBUTTON1, 
						wsz_ERROR, L"Must first select a row...");
				}

				for(INDEX i=0; i<cRows; i++)
				{
					//Display the Data for this row...
					m_pCDataGrid->DisplayData(rghRows[i], rgItems[i], DBPROP_IRowsetRefresh, true/*fAlways*/);
				}
				SAFE_FREE(rgItems);
				SAFE_FREE(rghRows);
			}
			return TRUE;

		case IDM_REFRESHVISIBLEDATA:
			if(pCRowset && pCRowset->m_pIRowsetRefresh)
			{
				CWaitCursor waitCursor;

				INDEX i,cRows = 0;
				HROW* rghRows = NULL;
				BOOL fOverWrite = FALSE;
				DBCOUNTITEM cRowsRefreshed = 0;
				HROW* rghRowsRefreshed = 0;
				DBROWSTATUS* rgRowStatus = NULL;
				INDEX* rgItems = NULL;

				//Get all selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);

				//IRowsetRefresh::RefreshVisibleData
				XTEST(hr = pCRowset->m_pIRowsetRefresh->RefreshVisibleData(hChapter, cRows, rghRows, fOverWrite, &cRowsRefreshed, &rghRowsRefreshed, &rgRowStatus));
				DisplayRowErrors(hr, cRowsRefreshed, rghRowsRefreshed, rgRowStatus);
				TRACE_METHOD(hr, L"IRowsetRefresh::RefreshVisibleData(0x%p, %Iu, 0x%p, %d, &%Iu, &0x%p, &0x%p)", hChapter, cRows, rghRows, fOverWrite, cRowsRefreshed, rghRowsRefreshed, rgRowStatus);

				//Now GetData for all SelectedRows (a nice helper)
				//We won't worry about updating the case for (0,NULL), its not worth
				//trying to match the rhgRows returned to the correct hRows.  Worse case
				//is the user has to call GetData for one or all rows...
				for(i=0; i<cRows; i++)
				{
					//Display the Data for this row...
					m_pCDataGrid->DisplayData(rghRows[i], rgItems[i]);
				}

				//Cleanup
				SAFE_FREE(rghRows);
				SAFE_FREE(rghRowsRefreshed);
				SAFE_FREE(rgRowStatus);
				SAFE_FREE(rgItems);
			}
			return TRUE;

		case IDM_GETORIGINALDATA:			
			if(pCRowset && pCRowset->m_pIRowsetUpdate)
			{
				CWaitCursor waitCursor;

				INDEX cRows = 0;
				INDEX* rgItems = NULL;
				HROW* rghRows = NULL;
				
				//Find all Selected Rows
				LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
				if(cRows == 0)
				{
					wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONHAND | MB_OK | MB_DEFBUTTON1, 
						wsz_ERROR, L"Must first select a row...");
				}

				for(INDEX i=0; i<cRows; i++)
				{
					//Display the Data for this row...
					m_pCDataGrid->DisplayData(rghRows[i], rgItems[i], DBPROP_IRowsetUpdate, true/*fAlways*/);
				}
				SAFE_FREE(rgItems);
				SAFE_FREE(rghRows);
			}
			return TRUE;

		case IDM_GETPENDINGROWS:			
			if(pCRowset && pCRowset->m_pIRowsetUpdate)
			{
				CWaitCursor waitCursor;

				DBCOUNTITEM cPendingRows = 0;
				HROW* rgPendingRows = 0;
				DBPENDINGSTATUS* rgPendingStatus = NULL;
			
				//IRowsetUpdate::GetPendingStatus
				XTEST(hr = pCRowset->m_pIRowsetUpdate->GetPendingRows(hChapter, DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_CHANGED | DBPENDINGSTATUS_DELETED, &cPendingRows, &rgPendingRows, &rgPendingStatus));
				TRACE_METHOD(hr, L"IRowsetUpdate::GetPendingRows(0x%p, DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_CHANGED | DBPENDINGSTATUS_DELETED, &%lu, &0x%p, &0x%p)", hChapter, cPendingRows, rgPendingRows, rgPendingStatus);

				//Cleanup
				SAFE_FREE(rgPendingRows);
				SAFE_FREE(rgPendingStatus);
			}
			return TRUE;

		case IDM_GETROWSTATUS:
			if(pCRowset && pCRowset->m_pIRowsetUpdate)
			{
				CWaitCursor waitCursor;

				//Find the SelectedRow
				HROW hRow;
				if(m_pCDataGrid->GetSelectedRow(&hRow, FALSE) != LVM_ERR)
				{
					DBPENDINGSTATUS dwPendingStatus = 0;

					//IRowsetUpdate::GetRowStatus
					XTEST(hr = pCRowset->m_pIRowsetUpdate->GetRowStatus(hChapter, 1, &hRow, &dwPendingStatus));
					TRACE_METHOD(hr, L"IRowsetUpdate::GetRowStatus(0x%p, %lu, &0x%p, &0x%p)", hChapter, 1, hRow, dwPendingStatus);
				}
			}
			return TRUE;

		case IDM_UNDO:			//Cancel the chagnes that were made.
			if(pCRowset && pCRowset->m_pIRowsetUpdate)
			{
				CWaitCursor waitCursor;
				UndoChanges();
			}
			return TRUE;

		case IDM_UPDATE:			//Update the changes that were made.
			if(pCRowset && pCRowset->m_pIRowsetUpdate)
			{
				CWaitCursor waitCursor;
				UpdateChanges();
			}
			return TRUE;

		//IServiceProvider
		case IDM_ISERVICEPROVIDER:
			if(pCDataSource && pCDataSource->m_pIServiceProvider)
			{
				//Display the Generic interface list...
				CInterfaceDlg interfaceDlg(IDD_INTERFACE, L"IServiceProvider::QueryService", IID_ISpecifyPropertyPages); 
				if(interfaceDlg.DoModal(hWnd) == IDOK)
				{
					CWaitCursor waitCursor;
					CComPtr<IUnknown> spUnknown;

					XTEST(hr = pCDataSource->m_pIServiceProvider->QueryService(OLEDB_SVC_DSLPropertyPages, interfaceDlg.GetSelInterface(), (void**)&spUnknown));
					TRACE_METHOD(hr, L"IServiceProvider::QueryService(OLEDB_SVC_DSLPropertyPages, %s, &0x%p)", GetInterfaceName(interfaceDlg.GetSelInterface()), spUnknown);
				}
			}
			return TRUE;


	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////
// CMDIChild::UpdateWndTitle
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::UpdateWndTitle()
{
	//Default Info
	CBase* pCBase				= GetObject();
	CDataSource* pCDataSource	= (CDataSource*)GetObject(eCDataSource);
	WCHAR* pwszProvider			= NULL;
	WCHAR* pwszDesc				= NULL;

	if(pCDataSource)
	{
		pwszProvider = pCDataSource->m_cstrProviderDesc &&  pCDataSource->m_cstrProviderDesc[0] ? pCDataSource->m_cstrProviderDesc : pCDataSource->m_cstrProviderName;
		pwszDesc	 = pCDataSource->m_cstrDataSource && pCDataSource->m_cstrDataSource[0] ? pCDataSource->m_cstrDataSource : pCDataSource->m_cstrDBMS;
	}

	//Format text and output to window
	wSendMessageFmt(
				m_hWnd, WM_SETTEXT, 0, L"%s: %s %s%s%s", 
				pCBase ? pCBase->GetObjectName() : L"Unknown",
				pwszProvider ? pwszProvider : L"", 
				pwszDesc && pwszDesc[0] ? L"(" : L"",
				pwszDesc ? pwszDesc : L"",
				pwszDesc && pwszDesc[0] ? L")" : L""
				);
	
	return S_OK;
}



////////////////////////////////////////////////////////////////
// CMDIChild::CreateEnumChild()
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::CreateEnumChild()
{
	HRESULT hr = S_OK;
	INDEX iSelRow = 0;
	WCHAR wszBuffer[MAX_NAME_LEN+1];

	CRowset* pCRowset			= (CRowset*)GetObject(eCRowset);
	CEnumerator* pCEnumerator	= SOURCE_GETPARENT(pCRowset, CEnumerator);
	if(pCRowset && pCEnumerator)
	{
		//If enumertor rowset, double-clicking on a row
		//brings up the ParseName object in another window
		ULONG iColumn = 1;  //Skip RowHandle column

		//Skip bookmark column...
		if(pCRowset->m_Bindings.GetCount() && pCRowset->m_Bindings[0].iOrdinal==0)
			iColumn = 2;

		//Obtain SelectedRow
		iSelRow = m_pCDataGrid->GetSelectedRow();
		ASSERT(iSelRow != LVM_ERR);
				   
		//Obtain SourcesName and description from SelectedRow
		ENUMINFO EnumInfo;
		//SOURCES_NAME
		m_pCDataGrid->GetItemText(iSelRow, iColumn+0, EnumInfo.wszName, MAX_NAME_LEN);
		//SOURCES_PARSENAME
		m_pCDataGrid->GetItemText(iSelRow, iColumn+1, EnumInfo.wszParseName, MAX_NAME_LEN);
		//SOURCES_DESCRIPTION
		m_pCDataGrid->GetItemText(iSelRow, iColumn+2, EnumInfo.wszDescription, MAX_NAME_LEN);
		//SOURCES_TYPE
		m_pCDataGrid->GetItemText(iSelRow, iColumn+3, wszBuffer, MAX_NAME_LEN);
		EnumInfo.eType = (DBTYPE)wcstoul(wszBuffer, NULL, 10);
		//SOURCES_ISPARENT
		m_pCDataGrid->GetItemText(iSelRow, iColumn+4, wszBuffer, MAX_NAME_LEN);
		EnumInfo.fIsParent = StringCompare(wszBuffer, L"True") ? TRUE : FALSE;

		//Now Connect using the Connect Dialog
		ASSERT(m_pCMainWindow);
		ASSERT(m_pCMainWindow->m_pCFullConnect);
		m_pCMainWindow->m_pCFullConnect->Display(m_hWnd, pCEnumerator, &EnumInfo);
	}
	return hr;
}



////////////////////////////////////////////////////////////////
// CMDIChild::DeleteSelectedRows
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::DeleteSelectedRows()
{
	//Delete the Selected row in the ListView...
	HRESULT hr = E_FAIL;
	INDEX cRows = 0;
	INDEX* rgItems = NULL;
	HROW* rghRows = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	LONG dwSelection = IDYES;
	CRowset*	pCRowset = (CRowset*)GetObject(eCRowset);

	//Is editing the Rowset allowed?
	if(!pCRowset || !pCRowset->m_pIRowsetChange)
		return E_FAIL;
	
	//Find the Selected Items in the ListView
	HCHAPTER hChapter = pCRowset->m_hChapter;
	LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
	SAFE_ALLOC(rgRowStatus, DBROWSTATUS, cRows); 
	
	//Popup a MessageBox, Asking user if really wants to DeleteRow
	if(cRows)
	{	
		dwSelection = wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1, 
			L"IRowsetChange::DeleteRows", L"Do you wish to delete the selected row(s)?");
	}

	if(dwSelection == IDYES)
	{
		//DeleteRows from Rowset
		XTEST(hr = pCRowset->m_pIRowsetChange->DeleteRows(hChapter, cRows, rghRows, rgRowStatus));
		DisplayRowErrors(hr, cRows, rghRows, rgRowStatus);
		TESTC(TRACE_METHOD(hr, L"IRowsetChange::DeleteRows(0x%p, %Iu, 0x%p, 0x%p)", hChapter, cRows, rghRows, rgRowStatus));

		//Loop over the deleted rows...
		for(INDEX i=0; i<cRows; i++)
		{
			//We are "lazy" about updating the ListView.  Since we are not 
			//sure wither the provider compacts or not after deleted rows,
			//its easier and more correct to just display a "deleted" icon form
			//of the row, rather than guessing...

			//NOTE: We can go off the property DBPROP_REMOVEDELETED, but then we 
			//need to adjust the "cursor" position we store, forward/backward fetch, etc.
			//Probably more useful and useable if its the same for all providers, rather
			//than have it just "disappear" on some providers and others it remains...

			//NOTE: Also this way allows you to call operations for deleted rows, 
			//otherwise you can't since its removed from the view...
//			if(pCRowset->m_fRemoveDeleted)
//			{
//				//Remove the row from the listview - since the provider compacts
//				m_pCDataGrid->DeleteItem(rgItems[i]);
//			}
//			else
//			{
				//Need to indicate row was deleted, but not compacted, (use "deleted" icon)
				m_pCDataGrid->SetItemImage(rgItems[i], 0, IMAGE_DELETE);
//			}
		}
	}
	
CLEANUP:
	SAFE_FREE(rgRowStatus);
	SAFE_FREE(rgItems);
	SAFE_FREE(rghRows);
	return hr;
}


/////////////////////////////////////////////////////////////////
// CMDIChild::ChangeSelectedRow
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::ChangeSelectedRow(CBase* pCSource, UINT idSource)
{
	//If this is a row object - theres only 1 row...
	if(SOURCE_GETOBJECT(pCSource, CRow))
	{
		m_pCDataGrid->m_iSelRow = 0;
	}
	else
	{
		//Otherwise find the Selected row
		m_pCDataGrid->m_iSelRow	= m_pCDataGrid->GetSelectedRow(NULL, TRUE);
		if(m_pCDataGrid->m_iSelRow == LVM_ERR)
			return E_INVALIDARG;
	}

	//Create Dialog box, passing the "this" pointer
	//NOTE: m_pCSource and m_idSource are already setup...
	DisplayDialog(IDD_ROWCHANGE, m_hWnd, RowChangeProc, pCSource, idSource);
	return S_OK;
}


////////////////////////////////////////////////////////////////
// CMDIChild::InsertNewRow
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::InsertNewRow()
{
	//Insert a new row into the ListView...
	HRESULT hr = E_FAIL;
	CRowset*	pCRowset = (CRowset*)GetObject(eCRowset);

	//Find the Selected Item in the ListView
	m_pCDataGrid->m_iSelRow = m_pCDataGrid->GetSelectedRow(NULL, FALSE);

	//Is editing the Rowset allowed?
	if(!pCRowset || !pCRowset->m_pIRowsetChange)
		return E_FAIL;

	//Bring up the Insert Dialog box...
	DisplayDialog(IDD_ROWCHANGE, m_hWnd, RowChangeProc, pCRowset, IDM_INSERTROW);
	return hr;
}


////////////////////////////////////////////////////////////////
// CMDIChild::UndoChanges
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::UndoChanges()
{
	//Undo all pending changes, (if in buffered mode)...
	HRESULT hr = E_FAIL;
	CRowset*	pCRowset = (CRowset*)GetObject(eCRowset);

	INDEX i,cRows = 0;
	HROW* rghRows = NULL;
	DBCOUNTITEM cRowsUndone = 0;
	HROW* rghRowsUndone = NULL;
	INDEX* rgItems = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
		
	if(pCRowset && pCRowset->m_pIRowsetUpdate)
	{
		HCHAPTER hChapter = pCRowset->m_hChapter;
		
		//Get all selected Rows
		LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);

		//IRowsetUpdate::Undo
		XTEST(hr = pCRowset->m_pIRowsetUpdate->Undo(hChapter,cRows,rghRows,&cRowsUndone,&rghRowsUndone, &rgRowStatus));
		DisplayRowErrors(hr, cRowsUndone, rghRowsUndone, rgRowStatus);
		TESTC(TRACE_METHOD(hr, L"IRowsetUpdate::Undo(0x%p, %lu, 0x%p, &%lu, &0x%p, &0x%p)", hChapter, cRows, rghRows, cRowsUndone, rghRowsUndone, rgRowStatus));

		//Now GetData for all SelectedRows (a nice helper)
		//We won't worry about updating the case for (0,NULL), its not worth
		//trying to match the rhgRows returned to the correct hRows.  Worse case
		//is the user has to call GetData for one or all rows...
		for(i=0; i<cRows; i++)
		{
			//Need to indicate row was updated, use normal icon
			m_pCDataGrid->SetItemImage(rgItems[i], 0, IMAGE_NORMAL);

			//Display the Data...
			hr = m_pCDataGrid->DisplayData(rghRows[i], rgItems[i]);
		}
	}

CLEANUP:
	SAFE_FREE(rghRows);
	SAFE_FREE(rghRowsUndone);
	SAFE_FREE(rgRowStatus);
	SAFE_FREE(rgItems);
	return hr;
}


////////////////////////////////////////////////////////////////
// CMDIChild::UpdateChanges
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::UpdateChanges()
{
	//Update all pending changes, (if in buffered mode)...
	HRESULT hr = E_FAIL;
	INDEX i,cRows = 0;
	HROW* rghRows = NULL;
	DBCOUNTITEM cRowsUpdated = 0;
	HROW* rghRowsUpdated = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	INDEX* rgItems = NULL;
	CRowset*	pCRowset = (CRowset*)GetObject(eCRowset);
	
	if(pCRowset && pCRowset->m_pIRowsetUpdate)
	{
		HCHAPTER hChapter = pCRowset->m_hChapter;
	
		//Get all selected Rows
		LV_GetSelItems(m_pCDataGrid->m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);

		//IRowsetUpdate::Update
		XTEST(hr = pCRowset->m_pIRowsetUpdate->Update(hChapter,cRows,rghRows,&cRowsUpdated,&rghRowsUpdated,&rgRowStatus));
		DisplayRowErrors(hr, cRowsUpdated, rghRowsUpdated, rgRowStatus);
		TESTC(TRACE_METHOD(hr, L"IRowsetUpdate::Update(0x%p, %lu, 0x%p, &%lu, &0x%p, &0x%p)", hChapter, cRows, rghRows, cRowsUpdated, rghRowsUpdated, rgRowStatus));

		//Now GetData for all SelectedRows (a nice helper)
		//We won't worry about updating the case for (0,NULL), its not worth
		//trying to match the rhgRows returned to the correct hRows.  Worse case
		//is the user has to call GetData for one or all rows...
		for(i=0; i<cRows; i++)
		{
			//Need to indicate row was updated, use normal icon
			m_pCDataGrid->SetItemImage(rgItems[i], 0, IMAGE_NORMAL);

			//Display the Data...
			hr = m_pCDataGrid->DisplayData(rghRows[i], rgItems[i]);
		}
	}

CLEANUP:
	SAFE_FREE(rghRows);
	SAFE_FREE(rghRowsUpdated);
	SAFE_FREE(rgRowStatus);
	SAFE_FREE(rgItems);
	return hr;
}

					
////////////////////////////////////////////////////////////////
// CMDIChild::GetListViewValues
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::GetListViewValues(HWND hWndNames, HWND hWndValues, CDataAccess* pCDataAccess, CBindings& rBindings, void* pData)
{
	ASSERT(hWndNames);
	ASSERT(hWndValues);
	ASSERT(pCDataAccess);
	HRESULT hr = S_OK;

	//Obtain the values from a specified ListView
	//And place them into our pData structure according to our bindings...
	//this method is mainly used for InsertRow and SetData operations...
	WCHAR		wszBuffer[MAX_COL_SIZE+1] = {0};
	DBSTATUS	dbStatus		= DBSTATUS_S_OK;
	DBTYPE		wBackendType	= DBTYPE_EMPTY;

	//Loop through all Columns
	for(ULONG i=0; i<rBindings.GetCount(); i++)
	{
		//Get the Status from the 'insert' ListView
		LV_GetItemText(hWndNames, i, 2, wszBuffer, MAX_COL_SIZE);
		dbStatus = GetStatusValue(wszBuffer);
		
		//Get the Backend Type (sotred as the item data of the value)
		wBackendType = (DBTYPE)LV_GetItemParam(hWndValues, i, 0);

		//Get the Item from the 'insert' ListView
		LV_GetItemText(hWndValues, i, 0, wszBuffer, MAX_COL_SIZE);

		//Get the Image and Param
		BOOL bChecked = LV_GetItemState(hWndNames, i, LVIS_STATEIMAGEMASK) & INDEXTOSTATEIMAGEMASK(STATE_CHECKED);
		if(bChecked)
		{
			//Set the ColumnData in our pData buffer
			TESTC(hr = pCDataAccess->SetColumnData(&rBindings[i], pData, dbStatus, wcslen(wszBuffer), wszBuffer, CONV_NONE, wBackendType));

			//Reset the Item in the ListView in case it got truncated
			LV_SetItemText(hWndValues, i, 0, wszBuffer);
		}
		else
		{
			//Mark this status as skipped (ignore)
			//This allows us to not have to realloc a smaller subset of selected
			//columns, and also allows us to be able to reuse the hAccessor
			if(STATUS_IS_BOUND(rBindings[i]))
				BINDING_STATUS(rBindings[i], pData) = DBSTATUS_S_IGNORE;
		}
	}

CLEANUP:
	return hr;
}
					

////////////////////////////////////////////////////////////////
// CMDIChild::RowChangeProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::RowChangeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bBeginEdit	= FALSE;
	static BOOL s_bClearAll = TRUE;
	static CListViewLite	s_listNames;
	static CListViewLite	s_listValue;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			bBeginEdit = FALSE;

			//Save the "this" pointer
			CMDIChild* pThis	= (CMDIChild*)SetThis(hWnd, (void*)lParam);

			DWORD dwConvFlags = pThis->GetOptions()->m_dwConvFlags;
			HRESULT hr = S_OK;

			WCHAR		wszBuffer[MAX_COL_SIZE+1];
			DBCOUNTITEM iBind = 0;
			DBLENGTH	dbLength = 0;
			DBSTATUS	dbStatus = DBSTATUS_S_OK;//DBSTATUS_S_ISNULL;
			DBTYPE		wSubType = 0;

			//We need to bring up a 2 ListViews.
			//left - has column names, 
			//right - is blank for entering data for the corrsponding column
			
			//Setup column headers
			s_listNames.CreateIndirect(hWnd, IDL_NAMES);
			s_listValue.CreateIndirect(hWnd, IDL_VALUES);
			s_bClearAll = TRUE;

			//SubClass the ListViews.
			SynchSubProc(s_listNames.m_hWnd, WM_INITDIALOG, 0, (LPARAM)s_listValue.m_hWnd);
			SynchSubProc(s_listValue.m_hWnd, WM_INITDIALOG, 0, (LPARAM)s_listNames.m_hWnd);
		
			//Use Extended ListView Styles!
			SendMessage(s_listNames.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES);
			SendMessage(s_listValue.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

			//Set image list to the ListView
			ListView_SetImageList(s_listNames.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
			ListView_SetImageList(s_listNames.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_STATE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_STATE);
			ListView_SetImageList(s_listValue.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

			//Insert Column Headers
			s_listNames.InsertColumn(0, L"Column");
			s_listNames.InsertColumn(1, L"Length");
			s_listNames.InsertColumn(2, L"Status");
			s_listNames.InsertColumn(3, L"Type");
			s_listNames.InsertColumn(4, L"SubType");
			s_listValue.InsertColumn(0, L"Data");
			
			//Obtain the data for the selected row
			HROW hRow = NULL;

			//Determine the object type
			CRow* pCRow					= SOURCE_GETOBJECT(pThis->m_pCSource, CRow);
			CRowset* pCRowset			= SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);
			CDataset* pCDataset			= SOURCE_GETOBJECT(pThis->m_pCSource, CDataset);
			CDataAccess* pCDataAccess	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
			CDataGrid* pCDataGrid		= pThis->m_pCDataGrid;
			void* pData					= NULL;
			
			//Determine how to get the data...
			if(pCRow)
			{
				//Row Object (extra columns)
				hr = pCRow->GetColumns(pCRow->m_cColAccess, pCRow->m_rgColAccess);
				pData = pCDataAccess->m_pData;
			}
			else if(pCDataset)
			{
				//Dataset Object
				hr = pCDataset->GetCellData(pCDataGrid->m_iSelRow, pCDataGrid->m_iSelRow);
				pData = pCDataAccess->m_pData;
			}
			else if(pCRowset)
			{
				//Rowset
				//Obtain the selected row...
				if(pCDataGrid->m_iSelRow != LVM_ERR)
					hRow = pCDataGrid->GetItemParam(pCDataGrid->m_iSelRow);
				
				//NOTE:  If the user has released the row handle, and then called
				//insert row, selecting this row handle, since of erroring out we will just not 
				//provide the default row of data to insert...
				if(hRow || pThis->m_idSource != IDM_INSERTROW)
				{
					hr = pCRowset->GetData(hRow);
					pData = pCDataAccess->m_pData;
				}
			}
						
			//Relax the errors here.  With Get*Data both DB_S/DB_E will return a bad
			//status for 1 or more columns.  Or DataGrid will just display the column
			//status if not S_OK.  So for these errors just display the data anyway...
			if(FAILED(hr) && hr!= DB_E_ERRORSOCCURRED)
				goto CLEANUP2;
			hr = S_OK;	//Otherwise dialog is not displayed

			//Display ColumnNames in LeftPane, and data in the right pane
			for(iBind=0; iBind<pCDataAccess->m_Bindings.GetCount(); iBind++)
			{
				//Get the Length, Status, Data for this Column
				const DBBINDING* pBinding		= &pCDataAccess->m_Bindings[iBind];
				const DBCOLUMNINFO* pColInfo	= pCDataAccess->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);
				wszBuffer[0] = wEOL;
				
				//Obtain the Data...
				if(pData)
					pCDataAccess->GetColumnData(pBinding, pData, &dbStatus, &dbLength, &wSubType, wszBuffer, MAX_COL_SIZE, dwConvFlags, pColInfo->wType);
				
				//Insert the Data into the list (make an "easy" edit)
				s_listValue.InsertItem(iBind, 0, wszBuffer, (LPARAM)pColInfo->wType, pCDataAccess->GetColumnImage(pColInfo, dbStatus));

				//Insert the ColumnName
				s_listNames.InsertItem(iBind, 0, GetColName(pColInfo));
				
				//Insert the Length into the List
				if(LENGTH_IS_BOUND(*pBinding))
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", dbLength);
				else
					StringCopy(wszBuffer, L"Not Bound", MAX_COL_SIZE);
				s_listNames.InsertItem(iBind, 1, wszBuffer);

				//Insert the Status into the List
				s_listNames.InsertItem(iBind, 2, STATUS_IS_BOUND(*pBinding) ? GetStatusName(dbStatus) : L"Not Bound");

				//Insert the Type into the List
				s_listNames.InsertItem(iBind, 3, GetDBTypeName(pColInfo->wType));

				//Insert the SubType into the List
				if(wSubType || pColInfo->wType == VT_VARIANT)
					s_listNames.InsertItem(iBind, 4, GetVariantTypeName(wSubType));

				//Set Item State to Checked/Unchecked
				s_listNames.SetItemState(iBind, 0,  INDEXTOSTATEIMAGEMASK(pColInfo->dwFlags & (DBCOLUMNFLAGS_WRITE|DBCOLUMNFLAGS_WRITEUNKNOWN) ? STATE_CHECKED : STATE_UNCHECKED),  LVIS_STATEIMAGEMASK);
			}

			//Update Controls
			::EnableWindow(::GetDlgItem(hWnd, IDB_CLEARALL),	(pCRowset && pCRowset->m_pIRowsetChange) || (pCRow && pCRow->m_pIRowChange));

			//Update Title and Status
			switch(pThis->m_idSource)
			{
				case IDM_IROW_DELETECOLUMNS:
					if(pCRow && pCRow->m_pIRowSchemaChange)
					{
						SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"IRowSchemaChange::DeleteColumns");
						SendMessage(::GetDlgItem(hWnd, IDT_HELPMSG), WM_SETTEXT, 0, (LPARAM)"Deletes the desired columns from the row object");
						SendMessage(::GetDlgItem(hWnd, IDOK), WM_SETTEXT, 0, (LPARAM)"Delete");
						::EnableWindow(::GetDlgItem(hWnd, IDB_CLEARALL), TRUE);
					}
					break;

				case IDM_IROW_SETCOLUMNS:
					if(pCRow && pCRow->m_pIRowChange)
					{
						SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"IRowChange::SetColumns");
						SendMessage(::GetDlgItem(hWnd, IDT_HELPMSG), WM_SETTEXT, 0, (LPARAM)"Modifies the row object columns");
						SendMessage(::GetDlgItem(hWnd, IDOK), WM_SETTEXT, 0, (LPARAM)"SetColumns");
						::EnableWindow(::GetDlgItem(hWnd, IDB_CLEARALL), TRUE);
					}
					break;

				case IDM_INSERTROW:
					if(pCRowset && pCRowset->m_pIRowsetChange)
					{
						SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"IRowsetChange::InsertRow");
						SendMessage(::GetDlgItem(hWnd, IDT_HELPMSG), WM_SETTEXT, 0, (LPARAM)"Inserts the desired row of data into the Rowset");
						SendMessage(::GetDlgItem(hWnd, IDOK), WM_SETTEXT, 0, (LPARAM)"InsertRow");
						::EnableWindow(::GetDlgItem(hWnd, IDB_CLEARALL), TRUE);
					}
					break;
			
				case IDM_SETDATA:
					if(pCRowset && pCRowset->m_pIRowsetChange)
					{
						SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"IRowsetChange::SetData");
						SendMessage(::GetDlgItem(hWnd, IDT_HELPMSG), WM_SETTEXT, 0, (LPARAM)"Modifies the row data in the Rowset");
						SendMessage(::GetDlgItem(hWnd, IDOK), WM_SETTEXT, 0, (LPARAM)"SetData");
						::EnableWindow(::GetDlgItem(hWnd, IDB_CLEARALL), TRUE);
					}
					break;

				default:
					//NOTE: The Dialog is already setup with the default window text:
					//"Detailed Row Information"
					::EnableWindow(::GetDlgItem(hWnd, IDB_CLEARALL), FALSE);
					break;
			};

			//Free outofline data
			//TODO - need to free in the case of errors as well.  But have to be careful
			//since the pData may not have been setup yet, or other errors where its undefined...
			if(pData)
				pCDataAccess->m_Bindings.FreeData(pData);

		CLEANUP2:
			
			//AutoSize Columns
			for(iBind=0; iBind<=4; iBind++)
				s_listNames.SetColumnWidth(iBind, LVSCW_AUTOSIZE_USEHEADER);
			s_listValue.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
			CenterDialog(hWnd);

			if(FAILED(hr))
				EndDialog(hWnd, FALSE);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
				break;

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_CLEARALL:
				{
					//Get the "this" pointer
					CWaitCursor waitCursor;
					
					//Now that the user has selected to clear all columns, we need to loop through 
					//all the checked columns and just uncheck them...
					INDEX cItems = s_listNames.GetItemCount();
					for(INDEX i=0; i<cItems; i++)
					{
						//Check the property state
						s_listNames.SetItemState(i, 0,  INDEXTOSTATEIMAGEMASK(s_bClearAll ? STATE_UNCHECKED : STATE_CHECKED),  LVIS_STATEIMAGEMASK);
					}

					//Clear All is a toggle...
					if(s_bClearAll)
					{
						s_bClearAll = FALSE;
						::SetWindowText(::GetDlgItem(hWnd, IDB_CLEARALL), "SelectAll");
					}
					else
					{
						s_bClearAll = TRUE;
						::SetWindowText(::GetDlgItem(hWnd, IDB_CLEARALL), "ClearAll");
					}
					return 0;
				}

				case IDM_DBSTATUS_S_OK:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_OK");
					return 0;
				}

				case IDM_DBSTATUS_S_ISNULL:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_ISNULL");
					return 0;
				}
				
				case IDM_DBSTATUS_S_DEFAULT:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_DEFAULT");
					return 0;
				}

				case IDM_DBSTATUS_S_IGNORE:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_IGNORE");
					return 0;
				}

				case IDM_DISPLAY_HEX:
				{
					WCHAR wszBuffer[MAX_COL_SIZE+1]={0};
									
					//Find the Selected Row
					INDEX iSelRow = s_listValue.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					//Get the Text from the Row
					s_listValue.GetItemText(iSelRow, 0, wszBuffer, MAX_COL_SIZE);

					//Convert using the Requested converions
					if(SUCCEEDED(PostProcessString(wszBuffer, MAX_COL_SIZE, CONV_HEX)))
					{
						//Insert the Data into the list (make an "easy" edit)
						s_listValue.SetItemText(iSelRow, 0, wszBuffer);
					}
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);
					HRESULT hr = S_OK;

					ULONG i=0;
					HROW hRow = NULL;
					INDEX iSelItems = 0;
					
					ULONG cColumnIDs = 0;
					DBID* rgColumnIDs = NULL;
					DBSTATUS* rgStatus = NULL;
					

					//Determine the object type
					CRow* pCRow					= SOURCE_GETOBJECT(pThis->m_pCSource, CRow);
					CRowset* pCRowset			= SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);
					CDataAccess* pCDataAccess	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
					CDataGrid* pCDataGrid		= pThis->m_pCDataGrid;

					CBindings& rBindings		= pCDataAccess->m_Bindings;
					void* pData					= pCDataAccess->m_pData;

					//The ListView will produce IDOK if hitting return after entering
					//a value.  The easist way arround this is to just exit if 
					//we haven't yet received the ENDEDIT message
					if(bBeginEdit || (!SOURCE_GETINTERFACE(pCRow, IRowChange) && !SOURCE_GETINTERFACE(pCRowset, IRowsetChange)))
						goto CLEANUP;
					
					//Create Accessor binding all selected columns...
					iSelItems = s_listNames.GetItemCount();
					ASSERT(iSelItems != LVM_ERR);

					if(pThis->m_idSource == IDM_IROW_DELETECOLUMNS)
					{
						SAFE_ALLOC(rgColumnIDs, DBID, iSelItems);
						SAFE_ALLOC(rgStatus, DBSTATUS, iSelItems);
					}

					//Obtain the ListView Values into our pData buffer
					//NOTE:  We do this before we filter the selected columns, since the dialog
					//contains all columns, and needs to be indexed in the same order, 
					//without missing columns in the middle...
					TESTC(hr = pThis->GetListViewValues(s_listNames.m_hWnd, s_listValue.m_hWnd, pCDataAccess, rBindings, pData));

					//Loop through selected bindings
					for(i=0; i<rBindings.GetCount(); i++)
					{
						//Create compacted array of ColumnIDs for IRowSchemaChange::DeleteColumns
						if(pThis->m_idSource == IDM_IROW_DELETECOLUMNS)
						{
							//Get the Image and Param
							BOOL bChecked = s_listNames.GetItemState(i, LVIS_STATEIMAGEMASK) & INDEXTOSTATEIMAGEMASK(STATE_CHECKED);
							if(bChecked)
							{
								memcpy(&rgColumnIDs[cColumnIDs], &pCRow->m_rgColAccess[i].columnid, sizeof(DBID));
								rgStatus[cColumnIDs] = DBSTATUS_S_OK;
								cColumnIDs++;
							}
						}
					}

					switch(pThis->m_idSource)
					{
						case IDM_INSERTROW:
							if(pCRowset && pCRowset->m_pIRowsetChange)
							{
								HCHAPTER hChapter = pCRowset->m_hChapter;
							
								//Release previously fetched rows, (if user requested)
								hr = pCDataGrid->ReleaseHeldRows();

								//Now that we have setup the buffer, Insert the data into the rowset
								XTEST(hr = pCRowset->m_pIRowsetChange->InsertRow(hChapter, pCRowset->m_hAccessor, pData, &hRow));
								TRACE_METHOD(hr, L"IRowsetChange::InsertRow(0x%p, 0x%p, 0x%p, &0x%p)", hChapter, pCRowset->m_hAccessor, pData, hRow);

								//Display Data Errors
								TESTC(hr = DisplayBindingErrors(hr, rBindings.GetCount(), rBindings.GetElements(), pData));

								//Release the Retreived Row handle...
								//We could have just passed phRow = NULL, since we don't save the row handle,
								//but its nice for output/trace purposes so the user knows a handle 
								//was received...
								
								//NOTE:  We don't insert the returned row handle into the ListView
								//since we are not sure where the row was inserted.  Inserting it blindly
								//at the end would appear as if the cursor positioning was oof with 
								//GetNextRows if it was not truely inserted at the end.
								if(SUCCEEDED(hr))
									pCRowset->ReleaseRows(1, &hRow);
							}
							break;
												
						case IDM_IROW_DELETECOLUMNS:
							if(pCRow && pCRow->m_pIRowSchemaChange)
							{
								//DeleteColumns
								XTEST(hr = pCRow->m_pIRowSchemaChange->DeleteColumns(cColumnIDs, rgColumnIDs, rgStatus));
								TRACE_METHOD(hr, L"IRowSchemaChange::DeleteColumns(%lu, 0x%p, 0x%p)", cColumnIDs, rgColumnIDs, rgStatus);

								//Display Data Errors
								TESTC(hr = DisplayColumnErrors(hr, cColumnIDs, rgColumnIDs, rgStatus));
							}
							break;
					
						case IDM_IROW_SETCOLUMNS:
							if(pCRow && pCRow->m_pIRowChange)
							{
								//Have to call SetColumns for extra columns
								XTEST(hr = pCRow->m_pIRowChange->SetColumns(pCRow->m_cColAccess, pCRow->m_rgColAccess));
								TRACE_METHOD(hr, L"IRowChange::SetColumns(%lu, 0x%p)", pCRow->m_cColAccess, pCRow->m_rgColAccess);

								//Display Data Errors
								TESTC(hr = DisplayColAccessErrors(hr, pCRow->m_cColAccess, pCRow->m_rgColAccess));
							}
							break;
						
						case IDM_SETDATA:
							if(pCRowset && pCRowset->m_pIRowsetChange)
							{
								//Need to obtain the Current hRow value (lParam)
								hRow = pCDataGrid->GetItemParam(pCDataGrid->m_iSelRow);
								
								//Now that we have setup the buffer, SetData into the rowset
								XTEST(hr = pCRowset->m_pIRowsetChange->SetData(hRow, pCRowset->m_hAccessor, pData));
								TRACE_METHOD(hr, L"IRowsetChange::SetData(0x%p, 0x%p, 0x%p)", hRow, pCRowset->m_hAccessor, pData);

								//Display Data Errors
								TESTC(hr = DisplayBindingErrors(hr, rBindings.GetCount(), rBindings.GetElements(), pData));
							}
							break;
					};

					//Its our responsiblity to free the allocated (outofline) data...
					//TODO - need to free in the case of errors as well.  But have to be careful
					//since the pData may not have been setup yet, or other errors where its undefined...
					TESTC(rBindings.FreeData(pData, TRUE/*fSetData*/));

					if(pThis->m_idSource != IDM_INSERTROW)
					{
						//Need to indicate row was modified, use "changed" icon
						pCDataGrid->SetItemImage(pCDataGrid->m_iSelRow, 0, IMAGE_CHANGE);

						//Display the Data for this Row
						TESTC(hr = pCDataGrid->DisplayData(hRow, pCDataGrid->m_iSelRow, pCRow ? DBPROP_IRow : DBPROP_IRowset));
					}

				CLEANUP:
					SAFE_FREE(rgColumnIDs);
					SAFE_FREE(rgStatus);

					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					bBeginEdit = FALSE;
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND


		case WM_CONTEXTMENU:
		{	
			HWND hWndSelected = (HWND)wParam;
			CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
			
			//Must have selected a row to change status
			INDEX iSel = s_listNames.GetNextItem(-1, LVNI_SELECTED);
			if(iSel == LVM_ERR)
				return FALSE;

			//IDL_NAMES
			if(hWndSelected == s_listNames.m_hWnd)
			{
				//Don't need to change the status if readonly
				if(!SOURCE_GETINTERFACE(pThis->m_pCSource, IRowChange) && !SOURCE_GETINTERFACE(pThis->m_pCSource, IRowsetChange))
					return FALSE;

				//Display Menu
				DisplayContextMenu( 
								hWnd,
								IDM_CHANGESTATUS, 
								MAKEPOINTS(lParam),
								hWnd
								);
			}

			//IDL_VALUES
			if(hWndSelected == s_listValue.m_hWnd)
			{
				//Display Menu
				DisplayContextMenu( 
								hWnd,
								IDM_CHANGEVALUE, 
								MAKEPOINTS(lParam),
								hWnd
								);
			}
			return FALSE;
		}

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
			
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
				{
					//Obtain the SelectedRow
					INDEX iSel = (INDEX)SendMessage(::GetDlgItem(hWnd, IDL_VALUES), LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
					if(iSel == LVM_ERR)
						return 0;

					//Need to Send LVM_EDITLABEL
					SendMessage(::GetDlgItem(hWnd, IDL_VALUES), LVM_EDITLABEL, iSel, 0);
					return 0;
				}

				case LVN_BEGINLABELEDIT:
					bBeginEdit = TRUE;
					return FALSE;//allow the user to change the value of the item.
					
				case LVN_ENDLABELEDIT:
				{
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					bBeginEdit = FALSE;								

					//Now update the ListView with the new value
					//If IRowsetChange is supported...
					if(pDispInfo->item.pszText && (SOURCE_GETINTERFACE(pThis->m_pCSource, IRowChange) || SOURCE_GETINTERFACE(pThis->m_pCSource, IRowsetChange)))
					{
						WCHAR wszBuffer[MAX_NAME_LEN]={0};
						ConvertToWCHAR(pDispInfo->item.pszText, wszBuffer, MAX_NAME_LEN);
						s_listValue.SetItemText(pDispInfo->item.iItem, 0, wszBuffer);
					}

					return TRUE; //Allow the edited change
				}

				case LVN_ITEMCHANGED:
				{
					bBeginEdit = FALSE;								

					if(pNMListView->uNewState & LVNI_FOCUSED &&
				 		pNMListView->uNewState & LVNI_SELECTED)
					{
						if(wParam == IDL_VALUES)
						{
							SyncSibling(s_listNames.m_hWnd, s_listValue.m_hWnd);
	           				return HANDLED_MSG;
						}

						if(wParam == IDL_NAMES)
						{
							SyncSibling(s_listValue.m_hWnd, s_listNames.m_hWnd);
	                		return HANDLED_MSG;
						}
						return UNHANDLED_MSG; //No return Value
					}
				}
			}
		}//WM_NOTIFY
	}//switch(message);

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::ColumnChangeProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::ColumnChangeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bBeginEdit = FALSE;
	static HACCESSOR hAccessor = NULL;
	static CListViewLite	s_listNames;
	static CListViewLite	s_listValue;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			bBeginEdit = FALSE;
			WCHAR wszBuffer[MAX_COL_SIZE+1];
			DBLENGTH dbLength = 0;
			DBSTATUS dbStatus = 0;
			DBTYPE   wSubType = 0;

			//Save the "this" pointer
			CMDIChild* pThis	= (CMDIChild*)SetThis(hWnd, (void*)lParam);

			DWORD dwConvFlags = pThis->GetOptions()->m_dwConvFlags;
			INDEX i,iItemCount = 0;
			HRESULT hr = S_OK;

			//We need to bring up a 2 ListViews.
			//left - has column names, 
			//right - is blank for entering data for the corrsponding column
			
			//Setup column headers
			s_listNames.CreateIndirect(hWnd, IDL_NAMES);
			s_listValue.CreateIndirect(hWnd, IDL_VALUES);
			
			//Use Extended ListView Styles!
			SendMessage(s_listNames.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE , LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);
			SendMessage(s_listValue.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE , LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

			//Set image list to the ListView
			ListView_SetImageList(s_listValue.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
			ListView_SetImageList(s_listNames.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

			//Update Controls
			::EnableWindow(::GetDlgItem(hWnd, IDB_CLEARALL), FALSE);

			//Determine the object type
			CRow* pCRow					= SOURCE_GETOBJECT(pThis->m_pCSource, CRow);
			CRowset* pCRowset			= SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);
			CDataset* pCDataset			= SOURCE_GETOBJECT(pThis->m_pCSource, CDataset);
			CDataAccess* pCDataAccess	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
			CDataGrid*	pCDataGrid		= pThis->m_pCDataGrid;
			
			//Update Title and Status
			switch(pThis->m_idSource)
			{
				case IDM_IROW_SETCOLUMNS:
					if(pCRow && pCRow->m_pIRowChange)
					{
						SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"IRowChange::SetColumns");
						SendMessage(::GetDlgItem(hWnd, IDT_HELPMSG), WM_SETTEXT, 0, (LPARAM)"Modifies the row object columns");
						SendMessage(::GetDlgItem(hWnd, IDOK), WM_SETTEXT, 0, (LPARAM)"SetColumns");
					}
					break;


				case IDM_SETDATA:
					if(pCRowset && pCRowset->m_pIRowsetChange)
					{
						SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"IRowsetChange::SetData");
						SendMessage(::GetDlgItem(hWnd, IDT_HELPMSG), WM_SETTEXT, 0, (LPARAM)"Modifies the row data in the Rowset");
						SendMessage(::GetDlgItem(hWnd, IDOK), WM_SETTEXT, 0, (LPARAM)"SetData");
					}
					break;
			};

			//Insert Column Headers
			s_listNames.InsertColumn(0, pCDataAccess->GetObjectType() == eCDataset ? L"  Cell Ordinal  " : L"  Row Handle  ");
			s_listNames.InsertColumn(1, L"Length");
			s_listNames.InsertColumn(2, L"Status");
			s_listNames.InsertColumn(3, L"Type");
			s_listNames.InsertColumn(4, L"SubType");
			
			
			
			const DBBINDING* pBinding		= &pCDataAccess->m_Bindings[pCDataGrid->m_iSelCol];
			const DBCOLUMNINFO* pColInfo	= pCDataAccess->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);
			void* pData						= pCDataAccess->m_pData;
			
			if(!pCRow)
			{
				//Create An Accessor binding only this column
				//Much quicker to get and set using only the column we need.
				hr = pCDataAccess->CreateAccessor(DBACCESSOR_ROWDATA, 1, pBinding, 0, &hAccessor);
				if(FAILED(hr))
					goto CLEANUP2;			
			}

			//Column header is the ColumnName
			s_listValue.InsertColumn(0, GetColName(pColInfo), pCDataAccess->GetColumnImage(pColInfo));
			
			//Display Data in the ListView
			iItemCount = pCDataGrid->GetItemCount();
			for(i=0; i<iItemCount; i++)
			{
				//Get row handle for the selected row
				HROW hRow = pCDataGrid->GetItemParam(i);

				if(pCRow)
				{
					ULONG cColAccess = 1;
					DBCOLUMNACCESS* rgColAccess = &pCRow->m_rgColAccess[pCDataGrid->m_iSelCol];

					//GetColumn for this row object
					//Much quicker to get and set using only the column we need.
					hr = pCRow->GetColumns(cColAccess, rgColAccess);
				}
				else if(pCDataset)
				{
					//GetCellData
					//Note: the "hRow" stored in the listview is the index
					hr = pCDataset->GetCellData(hRow, hRow);
				}
				else if(pCRowset)
				{
					//GetData for this row from the rowset
					//Much quicker to get and set using only the column we need.
					hr = pCRowset->GetData(hRow, hAccessor, pData);
				}
				
				//Relax the errors here.  With Get*Data both DB_S/DB_E will return a bad
				//status for 1 or more columns.  Or DataGrid will just display the column
				//status if not S_OK.  So for these errors just display the data anyway...
				if(FAILED(hr) && hr!= DB_E_ERRORSOCCURRED)
					goto CLEANUP2;
				hr = S_OK;	//Otherwise dialog is not displayed

				//Get the Length, Status, Data for this Column
				pCDataAccess->GetColumnData(pBinding, pData, &dbStatus, &dbLength, &wSubType, wszBuffer, MAX_COL_SIZE, dwConvFlags, pColInfo->wType);

				//Insert the Data into the list (make an "easy" edit)
				s_listValue.InsertItem(i, 0, wszBuffer, (LPARAM)pColInfo->wType, pCDataAccess->GetColumnImage(pColInfo, dbStatus));

				//Display the hRow in the ListView
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", hRow);
				s_listNames.InsertItem(i, 0, wszBuffer, 0, 0);
				
				//Insert the Length into the List
				if(LENGTH_IS_BOUND(*pBinding))
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", dbLength);
				else
					StringCopy(wszBuffer, L"Not Bound", MAX_COL_SIZE);
				s_listNames.InsertItem(i, 1, wszBuffer);

				//Insert the Status into the List
				s_listNames.InsertItem(i, 2, STATUS_IS_BOUND(*pBinding) ? GetStatusName(dbStatus) : L"Not Bound");

				//Insert the Type into the List
				s_listNames.InsertItem(i, 3, GetDBTypeName(pColInfo->wType));

				//Insert the SubType into the List
				if(pBinding->wType == DBTYPE_VARIANT)
					s_listNames.InsertItem(i, 4, GetVariantTypeName(wSubType));
			}

			//Free any outofline data
			//TODO - need to free in the case of errors as well.  But have to be careful
			//since the pData may not have been setup yet, or other errors where its undefined...
			if(SUCCEEDED(hr))
				FreeBindingData(1, pBinding, pData);

			//AutoSize Columns
			for(i=0; i<=4; i++)
				s_listNames.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
			s_listValue.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

		CLEANUP2:
			CenterDialog(hWnd);
			if(FAILED(hr))
			{
				pCDataAccess->ReleaseAccessor(&hAccessor);
				EndDialog(hWnd, FALSE);
			}

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
				case IDM_DBSTATUS_S_OK:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_OK");
					return 0;
				}

				case IDM_DBSTATUS_S_ISNULL:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_ISNULL");
					return 0;
				}
				
				case IDM_DBSTATUS_S_DEFAULT:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_DEFAULT");
					return 0;
				}

				case IDM_DBSTATUS_S_IGNORE:
				{
					//Insert the Status into the List
					INDEX iSelRow = s_listNames.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					s_listNames.InsertItem(iSelRow, 2, L"DBSTATUS_S_IGNORE");
					return 0;
				}

				case IDM_DISPLAY_HEX:
				{
					WCHAR wszBuffer[MAX_COL_SIZE+1] = {0};
									
					//Find the Selected Row
					INDEX iSelRow = s_listValue.GetNextItem(-1, LVNI_SELECTED);
					if(iSelRow == LVM_ERR)
						return 0;

					//Get the Text from the Row
					s_listValue.GetItemText(iSelRow, 0, wszBuffer, MAX_COL_SIZE);

					//Convert using the Requested converions
					if(SUCCEEDED(PostProcessString(wszBuffer, MAX_COL_SIZE, CONV_HEX)))
					{
						//Insert the Data into the list (make an "easy" edit)
						s_listValue.SetItemText(iSelRow, 0, wszBuffer);
					}
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;
					WCHAR wszBuffer[MAX_COL_SIZE+1];

					//Get the "this" pointer
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);
					INDEX i,iItemCount = 0;
					HRESULT hr = S_OK;

					//Determine the object type
					CRow* pCRow					= SOURCE_GETOBJECT(pThis->m_pCSource, CRow);
					CRowset* pCRowset			= SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);
					CDataAccess* pCDataAccess	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
					CDataGrid*	pCDataGrid		= pThis->m_pCDataGrid;

					const DBBINDING* pBinding		= &pCDataAccess->m_Bindings[pCDataGrid->m_iSelCol];
					const DBCOLUMNINFO* pColInfo	= pCDataAccess->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);
					void* pData						= pCDataAccess->m_pData;
					
					//The ListView will produce IDOK if hitting return after entering
					//a value.  The easist way arround this is to just exit if 
					//we haven't yet received the ENDEDIT message
					if(bBeginEdit)
						goto CLEANUP;

					if(!SOURCE_GETINTERFACE(pCRow, IRowChange) && !SOURCE_GETINTERFACE(pCRowset, IRowsetChange))
						goto CLEANUP;
					
					//This dialog is a little more complex since were allowing 
					//the user to change an entire column so every row must be set
					//To make our lives a little easier, we will first Get the Data
					//for the row, and then Set only the affected column...
					iItemCount = s_listValue.GetItemCount();
					for(i=0; i<iItemCount; i++)
					{
						//Get row handle
						HROW hRow = pCDataGrid->GetItemParam(i);

						//Get the ColumnStatus
						s_listNames.GetItemText(i, 2, wszBuffer, MAX_COL_SIZE);
						DBSTATUS dbStatus = GetStatusValue(wszBuffer);
						
						//Set the ColumnData in our pData buffer
						s_listValue.GetItemText(i, 0, wszBuffer, MAX_COL_SIZE);
						TESTC(hr = pCDataAccess->SetColumnData(pBinding, pData, dbStatus, wcslen(wszBuffer), wszBuffer, CONV_NONE, pColInfo->wType));
					
						if(pCRow)
						{
							ULONG cColAccess = 1;
							DBCOLUMNACCESS* rgColAccess = &pCRow->m_rgColAccess[pCDataGrid->m_iSelCol];
							
							//Have to call SetColumns for extra columns
							XTEST(hr = pCRow->m_pIRowChange->SetColumns(cColAccess, rgColAccess));
							TRACE_METHOD(hr, L"IRowChange::SetColumns(%lu, 0x%p)", cColAccess, rgColAccess);

							//Display Data Errors...
							TESTC(hr = DisplayColAccessErrors(hr, cColAccess, rgColAccess));
						}
						else if(pCRowset)
						{
							//Now that we have setup the buffer, SetData into the rowset
							XTEST(hr = pCRowset->m_pIRowsetChange->SetData(hRow, hAccessor, pData));
							TRACE_METHOD(hr, L"IRowsetChange::SetData(0x%p, 0x%p, 0x%p)", hRow, hAccessor, pData);

							//Display Data Errors...
							TESTC(hr = DisplayBindingErrors(hr, 1, pBinding, pData));
						}
						
						//Need to indicate row was modified, use "changed" icon
						pCDataGrid->SetItemImage(pCDataGrid->m_iSelRow, 0, IMAGE_CHANGE);

						//Display the Data for this Row
						TESTC(hr = pCDataGrid->DisplayData(hRow, i, pCRow ? DBPROP_IRow : DBPROP_IRowset));
					}
					
					//Free outofline data
					//TODO - need to free in the case of errors as well.  But have to be careful
					//since the pData may not have been setup yet, or other errors where its undefined...
					FreeBindingData(1, pBinding, pData, TRUE/*fSetData*/);

				CLEANUP:
					if(SUCCEEDED(hr))
					{
						pCDataAccess->ReleaseAccessor(&hAccessor);
						EndDialog(hWnd, TRUE);
					}
					return 0;
				}

				case IDCANCEL:
				{
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CDataAccess* pCDataAccess	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
					bBeginEdit = FALSE;
					
					pCDataAccess->ReleaseAccessor(&hAccessor);
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND

		case WM_CONTEXTMENU:
		{	
			HWND hWndSelected = (HWND)wParam;
			CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
			
			//Must have selected a row to change status
			INDEX iSel = s_listNames.GetNextItem(-1, LVNI_SELECTED);
			if(iSel == LVM_ERR)
				return FALSE;

			//IDL_NAMES
			if(hWndSelected == s_listNames.m_hWnd)
			{
				//Don't need to change the status if readonly
				if(SOURCE_GETINTERFACE(pThis->m_pCSource, IRowChange) && SOURCE_GETINTERFACE(pThis->m_pCSource, IRowsetChange))
					return FALSE;

				//Display Menu
				DisplayContextMenu( 
									hWnd,
									IDM_CHANGESTATUS, 
									MAKEPOINTS(lParam),
									hWnd
									);
			}

			//IDL_VALUES
			if(hWndSelected == s_listValue.m_hWnd)
			{
				//Display Menu
				DisplayContextMenu(
								hWnd,
								IDM_CHANGEVALUE, 
								MAKEPOINTS(lParam),
								hWnd
								);
			}
			return FALSE;
		}

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
			
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
				{
					//Save the SelectedRow so the new dialog box knows which row were concerned with...
					INDEX iSel = (INDEX)SendMessage(::GetDlgItem(hWnd, IDL_VALUES), LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
					if(iSel == LVM_ERR)
						return 0;

					//Need to Send LVM_EDITLABEL
					SendMessage(::GetDlgItem(hWnd, IDL_VALUES), LVM_EDITLABEL, iSel, 0);
					return FALSE;
				}

				case LVN_BEGINLABELEDIT:
					bBeginEdit = TRUE;
					return FALSE;//allow the user to change the value of the item.
					
				case LVN_ENDLABELEDIT:
				{
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					bBeginEdit = FALSE;								

					//Now update the ListView with the new value
					if(pDispInfo->item.pszText && (SOURCE_GETINTERFACE(pThis->m_pCSource, IRowChange) || SOURCE_GETINTERFACE(pThis->m_pCSource, IRowsetChange)))
					{
						WCHAR wszBuffer[MAX_NAME_LEN]={0};
						ConvertToWCHAR(pDispInfo->item.pszText, wszBuffer, MAX_NAME_LEN);
						s_listValue.SetItemText(pDispInfo->item.iItem, 0, wszBuffer);
					}

					return TRUE; //Allow the edited change
				}

				case LVN_ITEMCHANGED:
				{
					if(pNMListView->uNewState & LVNI_FOCUSED &&
				 		pNMListView->uNewState & LVNI_SELECTED)
					{
						if(wParam == IDL_VALUES)
						{
							SyncSibling(s_listNames.m_hWnd, s_listValue.m_hWnd);
	           				return HANDLED_MSG;
						}

						if(wParam == IDL_NAMES)
						{
							SyncSibling(s_listValue.m_hWnd, s_listNames.m_hWnd);
	                		return HANDLED_MSG;
						}
						return UNHANDLED_MSG; //No return Value
					}
				}
			}
		}//WM_NOTIFY
	}//switch(message);

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::GetAxisInfoProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetAxisInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bBeginEdit = FALSE;
	static CListViewLite	s_listAxisInfo;
	static CComboBoxLite	s_comboAxis;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			bBeginEdit = FALSE;
			HRESULT hr = S_OK;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CDataset* pCDataset = SOURCE_GETOBJECT(pThis->m_pCDataAccess, CDataset);
			DBCOUNTITEM cAxis;
			MDAXISINFO *rgAxisInfo;
			WCHAR wszBuffer[MAX_NAME_LEN];

			//Setup the ListView
			s_listAxisInfo.CreateIndirect(hWnd, IDL_AXISINFO);
			s_comboAxis.CreateIndirect(hWnd, IDCOMBO_AXIS);
		
			//Use Extended ListView Styles!
			SendMessage(s_listAxisInfo.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE , LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

			//Insert Column Headers
			s_listAxisInfo.InsertColumn(0, L"Dimension Names");
			s_listAxisInfo.InsertColumn(1, L"Columns");
			
			// get the axis info from the dataset
			ASSERT(pCDataset);
			hr = pCDataset->GetAxisInfo(&cAxis, &rgAxisInfo);
			
			if (SUCCEEDED(hr) && cAxis != 0) 
			{
				// fill in the axis selection combo
				for (ULONG iAxis=0; iAxis<cAxis-1; iAxis++)
				{
					switch (iAxis)
					{
						case 0:
							s_comboAxis.AddString(L"Columns");
							break;

						case 1:
							s_comboAxis.AddString(L"Rows");
							break;

						default:
							StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", iAxis);
							s_comboAxis.AddString(wszBuffer);
							break;
					};
				}
				
				s_comboAxis.AddString(L"Slicer");
				s_comboAxis.SetCurSel(0);

				wSendMessageFmt(::GetDlgItem(hWnd, IDC_DIMENSIONS), WM_SETTEXT, 0, L"%Iu", rgAxisInfo->cDimensions);
				wSendMessageFmt(::GetDlgItem(hWnd, IDC_COORDINATES), WM_SETTEXT, 0, L"%Iu", rgAxisInfo->cCoordinates);

				//Display Dimension Names in LeftPane, and Column Count in right pane
				for(ULONG iDim=0; iDim<rgAxisInfo->cDimensions; iDim++)
				{
					s_listAxisInfo.InsertItem(iDim, 0, rgAxisInfo->rgpwszDimensionNames[iDim]);
				
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", rgAxisInfo->rgcColumns[iDim]);
					s_listAxisInfo.InsertItem(iDim, 1, wszBuffer);
				}
			}

			//AutoSize Columns
			s_listAxisInfo.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
			s_listAxisInfo.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
			CenterDialog(hWnd);
			
			//Cleanup
			pCDataset->FreeAxisInfo(&cAxis, &rgAxisInfo);
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
				{
					CWaitCursor waitCursor;

					//The ListView will produce IDOK if hitting return after 
					//entering a value.  The easist way arround this is to just exit if 
					//we haven't yet received the ENDEDIT message
					if(bBeginEdit)
						return FALSE;
					
					EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCOMBO_AXIS:
				{
					switch(GET_WM_COMMAND_CMD(wParam, lParam))
					{
						//Selection change in axis combo box
						case CBN_SELCHANGE:
						{	
							HRESULT hr = S_OK;
							DBCOUNTITEM cAxis;
							MDAXISINFO *rgAxisInfo;
							WCHAR wszBuffer[MAX_NAME_LEN];
							CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
							CDataset* pCDataset = SOURCE_GETOBJECT(pThis->m_pCDataAccess, CDataset);
														
							INDEX iAxis = s_comboAxis.GetCurSel();
							s_listAxisInfo.DeleteAllItems();

							// get the axis info from the dataset
							ASSERT(pCDataset);
							hr = pCDataset->GetAxisInfo(&cAxis, &rgAxisInfo);
							if (SUCCEEDED(hr) && cAxis >= (DBCOUNTITEM)iAxis) 
							{
								wSendMessageFmt(::GetDlgItem(hWnd, IDC_DIMENSIONS), WM_SETTEXT, 0, L"%Iu", rgAxisInfo[iAxis].cDimensions);
								wSendMessageFmt(::GetDlgItem(hWnd, IDC_COORDINATES), WM_SETTEXT, 0, L"%Iu", rgAxisInfo[iAxis].cCoordinates);

								//Display Dimension Names in LeftPane, and Column Count in right pane
								for(ULONG iDim=0; iDim<rgAxisInfo[iAxis].cDimensions; iDim++)
								{
									s_listAxisInfo.InsertItem(iDim, 0, rgAxisInfo[iAxis].rgpwszDimensionNames[iDim]);
								
									StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", rgAxisInfo[iAxis].rgcColumns[iDim]);
									s_listAxisInfo.InsertItem(iDim, 1, wszBuffer);
								}
							}

							//Cleanup
							pCDataset->FreeAxisInfo(&cAxis, &rgAxisInfo);
							return 0;
						}
					}
				}
			}
			break;
		}//WM_COMMAND
	}//switch(message);
	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::GetAxisRowsetProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetAxisRowsetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid		s_CComboInterface;
	static CComboBoxLite		s_comboAxis;

	static BOOL fUseProps		= TRUE;			//Default
	static BOOL fAggregation	= FALSE;		//Default
	static BOOL fOutput			= TRUE;			//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			HRESULT hr = S_OK;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CDataset* pCDataset = SOURCE_GETOBJECT(pThis->m_pCDataAccess, CDataset);
			DBCOUNTITEM cAxis;
			MDAXISINFO *rgAxisInfo;
			WCHAR wszBuffer[MAX_NAME_LEN];

			//Setup the Combo
			s_comboAxis.CreateIndirect(hWnd, IDCOMBO_AXIS);
		
			// get the axis info from the dataset
			ASSERT(pCDataset);
			hr = pCDataset->GetAxisInfo(&cAxis, &rgAxisInfo);
			
			if(SUCCEEDED(hr) && cAxis) 
			{
				// fill in the axis selection combo
				for (ULONG iAxis=0; iAxis<cAxis-1; iAxis++)
				{
					switch (iAxis)
					{
						case 0:
							s_comboAxis.AddString(L"Columns");
							break;

						case 1:
							s_comboAxis.AddString(L"Rows");
							break;

						default:
							StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", iAxis);
							s_comboAxis.AddString(wszBuffer);
							break;
					};
				}
				
				s_comboAxis.AddString(L"Slicer");
				s_comboAxis.SetCurSel(0);
			}

			//Use Properties
			::CheckDlgButton(hWnd, IDB_USEPROPERTIES,	BST2STATE(fUseProps));

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IRowset);
			
			//Cleanup
			pCDataset->FreeAxisInfo(&cAxis, &rgAxisInfo);
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_SETPROPERTIES:
				{
					//Get the "this" pointer
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pThis->m_pCSource, CDataSource);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &pThis->m_CDefPropSets);
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
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CDataset* pCDataset = SOURCE_GETOBJECT(pThis->m_pCDataAccess, CDataset);
					CComPtr<IUnknown> spUnknown;
					HRESULT hr = S_OK;

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
						cPropSets = pThis->m_CDefPropSets.GetCount();
						rgPropSets = pThis->m_CDefPropSets.GetPropSets();
					}

					//Obtain the selected Axis					
					ASSERT(pCDataset);
					DBCOUNTITEM iAxis = s_comboAxis.GetCurSel();
					
					//GetAxisRowset
					if(SUCCEEDED(hr = pCDataset->GetAxisRowset(pCAggregate, iAxis, riid, cPropSets, rgPropSets, fOutput ? &spUnknown : NULL)))
					{
						//Now that we have the rowset, display it...
						//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
						if(!pThis->m_pCMainWindow->HandleObjectType(pCDataset, spUnknown, riid, eCRowset, 0, NULL, CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE))
							TESTC(hr = E_FAIL);
					}

				CLEANUP:
					SAFE_RELEASE(pCAggregate);
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, TRUE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND
	}//switch(message);
	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::GetColInfoProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetColInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			WCHAR wszBuffer[MAX_NAME_LEN+1];

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CDataAccess* pCDataAccess = NULL;
			HRESULT hr = S_OK;

			ULONG i=0;
			DBORDINAL cColumns = 0;
			DBCOLUMNINFO* rgColInfo = NULL;
			WCHAR* pStringBuffer = NULL;
			DBORDINAL cHiddenColumns = 0;
			CColumnInfo rColumnInfo;

			//Setup column headers
			//We have 2 listviews.
			// 1.  Left - ColInfo types...
			// 2.  Right - All ColInfo for all columns
			HWND hWndName	= ::GetDlgItem(hWnd, IDL_NAMES);
			HWND hWndCol	= ::GetDlgItem(hWnd, IDL_VALUES);
			HWND hWndHelp	= ::GetDlgItem(hWnd, IDT_HELPMSG);
			
			//Set Window Titles
			SendMessage(hWnd,		WM_SETTEXT, 0, (LPARAM)"IColumnsInfo::GetColumnInfo");
			SendMessage(hWndHelp,	WM_SETTEXT, 0, (LPARAM)"Displays the (meta-data) Column Information");
			
			//Use Extended ListView Styles!
			SendMessage(hWndName, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
			SendMessage(hWndCol, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);

			//Set image list to the Table Window 
			ListView_SetImageList(hWndCol, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
			ListView_SetImageList(hWndName, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

			//Ensure this is a CDataAccess derived class...
			pCDataAccess = SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
			ASSERT(pCDataAccess);

			//Now Obtain ColumnsInfo
			TESTC(hr = pCDataAccess->GetColInfo(&cColumns, &rgColInfo, &pStringBuffer, &cHiddenColumns));
			rColumnInfo.Attach(cColumns, rgColInfo, pStringBuffer, cHiddenColumns);

			//We need to the ListView
			//Headers/Columns contain ColInfo information 
			//Rows are per columns
			
			//ListView NAMES
			LV_InsertColumn(hWndName,	0,			L"ColName");
			LV_InsertItem(hWndName,		1,		0,	L"Ordinal");
			LV_InsertItem(hWndName,		2,		0,	L"Type");
			LV_InsertItem(hWndName,		3,		0,	L"ColumnSize");
			LV_InsertItem(hWndName,		4,		0, 	L"Precision");
			LV_InsertItem(hWndName,		5,		0,	L"Scale");
			LV_InsertItem(hWndName,		6,		0,	L"TypeInfo");
			LV_InsertItem(hWndName,		7,		0,	L"ColumnID");
			LV_InsertItem(hWndName,		8,		0,	L"dwFlags");
			
			//Loop through the ColumnFlags
			for(i=0; i<g_cColFlagsMaps; i++)
				LV_InsertItem(hWndName,		9+i,	0,	g_rgColFlagsMaps[i].pwszName);
					
			//AutoSize column
			SendMessage(hWndName, LVM_SETCOLUMNWIDTH, 0,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);

			for(i=0; i<cColumns; i++)
			{	
				ASSERT(rgColInfo);
				const DBCOLUMNINFO* pColInfo = &rColumnInfo[i];

				//Column Header
				LV_InsertColumn(hWndCol, i, GetColName(pColInfo), pCDataAccess->GetColumnImage(pColInfo)==IMAGE_LOCK ? IMAGE_LOCK : IMAGE_NONE);

				//Ordinal (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Id", pColInfo->iOrdinal);
				LV_InsertItem(hWndCol, 0, i, wszBuffer);

				//TYPE (subitem)
				LV_InsertItem(hWndCol, 1, i, GetDBTypeName(pColInfo->wType));
				
				//ColumnSize (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pColInfo->ulColumnSize);
				LV_InsertItem(hWndCol, 2, i, wszBuffer);

				//Precision (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pColInfo->bPrecision);
				LV_InsertItem(hWndCol, 3, i, wszBuffer);

				//Scale (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pColInfo->bScale);
				LV_InsertItem(hWndCol, 4, i, wszBuffer);
				
				//TypeInfo (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pColInfo->pTypeInfo);
				LV_InsertItem(hWndCol, 5, i, wszBuffer);

				//ColumnID (SubItem)  DBID
				DBIDToString(&pColInfo->columnid, wszBuffer, MAX_NAME_LEN);
				LV_InsertItem(hWndCol, 6, i, wszBuffer);

				//dwFlags
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pColInfo->dwFlags);
				LV_InsertItem(hWndCol, 7, i, wszBuffer);

				//FLAGS (SubItem)
				for(ULONG iFlag=0; iFlag<g_cColFlagsMaps; iFlag++)
					LV_InsertItem(hWndCol, 8+iFlag,	i, NULL, PARAM_NONE, pColInfo->dwFlags & g_rgColFlagsMaps[iFlag].lItem	? IMAGE_CHECKED : IMAGE_UNCHECKED);
			}

			//AutoSize all columns
			for(i=0; i<cColumns; i++)
				SendMessage(hWndCol, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
						
		CLEANUP:
			CenterDialog(hWnd);
			if(FAILED(hr))
				EndDialog(hWnd, FALSE);
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

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
			
			//ListView
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
					return 0;

				case LVN_ITEMCHANGED:
				{
					HWND hWndName  = ::GetDlgItem(hWnd, IDL_NAMES);
					HWND hWndCol   = ::GetDlgItem(hWnd, IDL_VALUES);

					if(pNMListView->uNewState & LVNI_FOCUSED &&
				 		pNMListView->uNewState & LVNI_SELECTED)
					{
						if(wParam == IDL_VALUES)
						{
							SyncSibling(hWndName, hWndCol);
	           				return FALSE;
						}

						if(wParam == IDL_NAMES)
						{
							SyncSibling(hWndCol, hWndName);
	                		return FALSE;
						}
						return UNHANDLED_MSG; //No return Value
					}
				}
			}


		}//WM_NOTIFY
	}//switch message

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::GetNextRowsProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetNextRowsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LONG s_lOffset	= 0;	//Default to lOffset==0
	static LONG s_cRows		= 1;	//Default to cRows==1
	
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Setup Window Handles
			HWND hWndOffset		= ::GetDlgItem(hWnd, IDE_OFFSET);
			HWND hWndcRows		= ::GetDlgItem(hWnd, IDE_COUNT);
			
			//SetWindowTitle (default is Rowset::GetNextRows)
			if(pThis->m_idSource == IDM_IROWSETLOCATE_GETROWSAT)
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"IRowsetLocate::GetRowsAt");
				
			//Supply Defaults to the lOffset and cRows
			wSendMessageFmt(hWndOffset, WM_SETTEXT, 0, L"%ld", s_lOffset);
			wSendMessageFmt(hWndcRows, WM_SETTEXT, 0, L"%ld", s_cRows);

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
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CRowset* pCRowset			= SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);
					CDataGrid*	pCDataGrid		= pThis->m_pCDataGrid;
					HRESULT hr = S_OK;

					//Setup Window Handles
					HWND hWndOffset		= ::GetDlgItem(hWnd, IDE_OFFSET);
					HWND hWndcRows		= ::GetDlgItem(hWnd, IDE_COUNT);
					
					//Obtain Defaults to the lOffset and cRows
					GetEditBoxValue(hWndOffset, &s_lOffset);
					GetEditBoxValue(hWndcRows, &s_cRows);

					switch(pThis->m_idSource)
					{
						case IDM_GETNEXTROWS:
						{
							//Display the indicated rows
							TESTC(hr = pCDataGrid->GetNextRows(s_lOffset, s_cRows));
							break;
						}
					
						case IDM_IROWSETLOCATE_GETROWSAT:
						{
							INDEX  cSelItems = 0;
							INDEX* rgSelItems = 0;
							HROW* rghRowsSel = NULL;
							DBBKMARK cbBookmark = 0;
							BYTE* pBookmark = NULL;
							DBCOUNTITEM cRowsObtained = 0;
							HROW* rghRows = NULL;

							//Obtain all Selected Rows
							LV_GetSelItems(pCDataGrid->m_hWnd, &cSelItems, &rgSelItems, (LPARAM**)&rghRowsSel);

							//Obtain the bookmark for the first row selected...
							if(cSelItems)
								hr = pCRowset->GetBookmark(rghRowsSel[0], &cbBookmark, &pBookmark);

							//Release previously fetched rows, (if user requested)
							hr = pCDataGrid->ReleaseHeldRows();

							//IRowsetLocate::GetRowsAt
							XTEST(hr = pCRowset->m_pIRowsetLocate->GetRowsAt(pCRowset->m_hChapter, NULL, cbBookmark, pBookmark, s_lOffset, s_cRows, &cRowsObtained, &rghRows));
							TRACE_METHOD(hr, L"IRowsetLocate::GetRowsAt(0x%p, NULL, %Iu, 0x%p, %ld, %ld, &%Iu, &0x%p)", pCRowset->m_hChapter, cbBookmark, pBookmark, s_lOffset, s_cRows, cRowsObtained, rghRows);

							//Display the rows retrieved...
							//NOTE: GetRowsAt has a much simplier starting for than GetNextRows.  Its basically
							//the bookmark + offset = starting row.  Whereas GetNextRows also depends upon
							//what the last fetch direction was, forward or backward.
							pCDataGrid->DisplayRows(rgSelItems[0] + s_lOffset, s_cRows, cRowsObtained, rghRows, FALSE/*fAdjustFetchPosition*/);

							//Cleanup
							SAFE_FREE(rgSelItems);
							SAFE_FREE(rghRowsSel);
							SAFE_FREE(rghRows);
							SAFE_FREE(pBookmark);
							break;
						}
					};

				CLEANUP:
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
				s_cRows += pUpDown->iDelta;
				wSendMessageFmt(::GetDlgItem(hWnd, IDE_COUNT), WM_SETTEXT, 0, L"%ld", s_cRows);
			}

			break;
		}//WM_NOTIFY
	}//switch message

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::GetBindingsProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetBindingsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			WCHAR wszBuffer[MAX_NAME_LEN+1];

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HRESULT hr = S_OK;
			
			//Determine which object this is called from
			CDataAccess* pCDataAccess = SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
			ASSERT(pCDataAccess);

			ULONG i = 0;
			DBCOUNTITEM cBindings = 0;
			DBBINDING* rgBindings = NULL;
			DBACCESSORFLAGS dwAccessorFlags = 0;

			//Setup column headers
			HWND hWndName		= ::GetDlgItem(hWnd, IDL_NAMES);
			HWND hWndValue		= ::GetDlgItem(hWnd, IDL_VALUES);
			HWND hWndHelp		= ::GetDlgItem(hWnd, IDT_HELPMSG);
			
			//Set Window Titles
			SendMessage(hWnd,		WM_SETTEXT, 0, (LPARAM)"IAccessor::GetBindings");
			SendMessage(hWndHelp,	WM_SETTEXT, 0, (LPARAM)"Displays the rgBindings from the Accessor");

			//Use Extended ListView Styles!
			SendMessage(hWndName,	LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);
			SendMessage(hWndValue,	LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

			//Set image list to the Window 
			ListView_SetImageList(hWndName, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
			ListView_SetImageList(hWndValue, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

			//Now IAccessor::GetBindings
			ASSERT(pCDataAccess->m_pIAccessor);
			XTEST(hr = pCDataAccess->m_pIAccessor->GetBindings(pCDataAccess->m_hAccessor, &dwAccessorFlags, &cBindings, &rgBindings));
			TESTC(TRACE_METHOD(hr, L"IAccessor::GetBindings(0x%p, &0x%08x, &%lu, &0x%p)", pCDataAccess->m_hAccessor, dwAccessorFlags, cBindings, rgBindings));

			//We need to the ListView
			//Headers/Columns contain ColInfo information 
			//Rows are per columns
			
			//ListView Columns
			LV_InsertColumn(hWndName,	0,		L"ColName");
			LV_InsertItem(hWndName,		0,		0,	L"Ordinal");
			LV_InsertItem(hWndName,		1,		0,	L"obValue");
			LV_InsertItem(hWndName,		2,		0,	L"obLength");
			LV_InsertItem(hWndName,		3,		0, 	L"obStatus");
			LV_InsertItem(hWndName,		4,		0,	L"pTypeInfo");
			LV_InsertItem(hWndName,		5,		0,	L"pObject");
			LV_InsertItem(hWndName,		6,		0,	L"pBindExt");
			LV_InsertItem(hWndName,		7,		0,	L"dwPart");
			LV_InsertItem(hWndName,		8,		0,	L"dwMemOwner");
			LV_InsertItem(hWndName,		9,		0,	L"eParamIO");
			LV_InsertItem(hWndName,		10,		0,	L"cbMaxLen");
			LV_InsertItem(hWndName,		11,		0,	L"dwFlags");
			LV_InsertItem(hWndName,		12,		0,	L"wType");
			LV_InsertItem(hWndName,		13,		0,	L"bPrecision");
			LV_InsertItem(hWndName,		14,		0,	L"bScale");
			
			//AutoSize column
			SendMessage(hWndName, LVM_SETCOLUMNWIDTH, 0,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);

			for(i=0; i<cBindings; i++)
			{	
				ASSERT(rgBindings);
				const DBBINDING* pBinding = &rgBindings[i];
				const DBCOLUMNINFO* pColInfo = pCDataAccess->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);

				//Column Header
				LV_InsertColumn(hWndValue, i, GetColName(pColInfo), pCDataAccess->GetColumnImage(pColInfo)==IMAGE_LOCK ? IMAGE_LOCK : IMAGE_NONE);

				//Ordinal (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Id", pBinding->iOrdinal);
				LV_InsertItem(hWndValue, 0, i, wszBuffer);

				//obValue (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->obValue);
				LV_InsertItem(hWndValue, 1, i, wszBuffer);

				//obLength (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->obLength);
				LV_InsertItem(hWndValue, 2, i, wszBuffer);
				
				//obStatus (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->obStatus);
				LV_InsertItem(hWndValue, 3, i, wszBuffer);
				
				//pTypeInfo (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->pTypeInfo);
				LV_InsertItem(hWndValue, 4, i, wszBuffer);

				//pObject (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->pObject);
				LV_InsertItem(hWndValue, 5, i, wszBuffer);

				//pBindExt (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->pBindExt);
				LV_InsertItem(hWndValue, 6, i, wszBuffer);

				//dwPart (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pBinding->dwPart);
				LV_InsertItem(hWndValue, 7, i, wszBuffer);

				//dwMemOwner (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pBinding->dwMemOwner);
				LV_InsertItem(hWndValue, 8, i, wszBuffer);

				//eParamIO (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pBinding->eParamIO);
				LV_InsertItem(hWndValue, 9, i, wszBuffer);

				//cbMaxLen (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pBinding->cbMaxLen);
				LV_InsertItem(hWndValue, 10, i, wszBuffer);

				//dwFlags (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pBinding->dwFlags);
				LV_InsertItem(hWndValue, 11, i, wszBuffer);

				//TYPE (subitem)
				LV_InsertItem(hWndValue, 12, i, GetDBTypeName(pBinding->wType));
				
				//Precision (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pBinding->bPrecision);
				LV_InsertItem(hWndValue, 13, i, wszBuffer);

				//Scale (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pBinding->bScale);
				LV_InsertItem(hWndValue, 14, i, wszBuffer);
			}

			//AutoSize all columns
			for(i=0; i<cBindings; i++)
				SendMessage(hWndValue, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
						
		CLEANUP:
			CenterDialog(hWnd);
			FreeBindings(&cBindings, &rgBindings);
			if(FAILED(hr))
				EndDialog(hWnd, FALSE);
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

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
			
			//ListView
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
					return 0;

				case LVN_ITEMCHANGED:
				{
					HWND hWndName		= ::GetDlgItem(hWnd, IDL_NAMES);
					HWND hWndValue	= ::GetDlgItem(hWnd, IDL_VALUES);

					if(pNMListView->uNewState & LVNI_FOCUSED &&
				 		pNMListView->uNewState & LVNI_SELECTED)
					{
						if(wParam == IDL_VALUES)
						{
							SyncSibling(hWndName, hWndValue);
	           				return FALSE;
						}

						if(wParam == IDL_NAMES)
						{
							SyncSibling(hWndValue, hWndName);
	                		return FALSE;
						}
						return UNHANDLED_MSG; //No return Value
					}
				}
			}


		}//WM_NOTIFY
	}//switch message

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::CreateAccessorProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::CreateAccessorProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Bindings
	static DBCOUNTITEM	cBindings	= 0;
	static DBBINDING*	rgBindings	= NULL;
	static DBLENGTH		cbRowSize	= 0;
	static BOOL			s_bClearAll	= TRUE;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			WCHAR wszBuffer[MAX_NAME_LEN+1];

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Which object are we calling this from
			CDataAccess* pCDataAccess = SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
			ASSERT(pCDataAccess);

			HRESULT hr = S_OK;

			//Setup column headers
			HWND hWndValue	= ::GetDlgItem(hWnd, IDL_COLUMNS);
			s_bClearAll = TRUE;
			
			//Use Extended ListView Styles!
			SendMessage(hWndValue, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES);
						
			//Set image list to the Window 
			ListView_SetImageList(hWndValue, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
			ListView_SetImageList(hWndValue, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_STATE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_STATE);

			//Setup Bindings
			pCDataAccess->GetColInfo();
			pCDataAccess->SetupBindings(BIND_ALLCOLS, &cBindings, &rgBindings, &cbRowSize); 
			
			//We need to the ListView
			//Headers/Columns contain ColInfo information 
			//Rows are per columns
			
			//ListView Columns
			LV_InsertColumn(hWndValue,		0,		L"ColName");
			LV_InsertColumn(hWndValue,		1,		L"Ordinal");
			LV_InsertColumn(hWndValue,		2,		L"obValue");
			LV_InsertColumn(hWndValue,		3,		L"obLength");
			LV_InsertColumn(hWndValue,		4,		L"obStatus");
			LV_InsertColumn(hWndValue,		5,		L"pTypeInfo");
			LV_InsertColumn(hWndValue,		6,		L"pObject");
			LV_InsertColumn(hWndValue,		7,		L"pBindExt");
			LV_InsertColumn(hWndValue,		8,		L"dwPart");
			LV_InsertColumn(hWndValue,		9,		L"dwMemOwner");
			LV_InsertColumn(hWndValue,		10,		L"eParamIO");
			LV_InsertColumn(hWndValue,		11,		L"cbMaxLen");
			LV_InsertColumn(hWndValue,		12,		L"dwFlags");
			LV_InsertColumn(hWndValue,		13,		L"wType");
			LV_InsertColumn(hWndValue,		14,		L"bPrecision");
			LV_InsertColumn(hWndValue,		15,		L"bScale");
			
			ULONG i;
			for(i=0; i<cBindings; i++)
			{	
				ASSERT(rgBindings);
				const DBBINDING* pBinding = &rgBindings[i];
				const DBCOLUMNINFO* pColInfo = pCDataAccess->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);

				//Column Header
				LV_InsertItem(hWndValue, i, 0, GetColName(pColInfo), PARAM_NONE, pCDataAccess->GetColumnImage(pColInfo)==IMAGE_LOCK ? IMAGE_LOCK : IMAGE_NONE);

				//Ordinal (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Id", pBinding->iOrdinal);
				LV_InsertItem(hWndValue, i, 1, wszBuffer);

				//obValue (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->obValue);
				LV_InsertItem(hWndValue, i, 2, wszBuffer);

				//obLength (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->obLength);
				LV_InsertItem(hWndValue, i, 3, wszBuffer);
				
				//obStatus (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->obStatus);
				LV_InsertItem(hWndValue, i, 4, wszBuffer);
				
				//pTypeInfo (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->pTypeInfo);
				LV_InsertItem(hWndValue, i, 5, wszBuffer);

				//pObject (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->pObject);
				LV_InsertItem(hWndValue, i, 6, wszBuffer);

				//pBindExt (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pBinding->pBindExt);
				LV_InsertItem(hWndValue, i, 7, wszBuffer);

				//dwPart (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pBinding->dwPart);
				LV_InsertItem(hWndValue, i, 8, wszBuffer);

				//dwMemOwner (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pBinding->dwMemOwner);
				LV_InsertItem(hWndValue, i, 9, wszBuffer);

				//eParamIO (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pBinding->eParamIO);
				LV_InsertItem(hWndValue, i, 10, wszBuffer);

				//cbMaxLen (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pBinding->cbMaxLen);
				LV_InsertItem(hWndValue, i, 11, wszBuffer);

				//dwFlags (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pBinding->dwFlags);
				LV_InsertItem(hWndValue, i, 12, wszBuffer);

				//TYPE (subitem)
				LV_InsertItem(hWndValue, i, 13, GetDBTypeName(pBinding->wType));
				
				//Precision (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pBinding->bPrecision);
				LV_InsertItem(hWndValue, i, 14, wszBuffer);

				//Scale (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pBinding->bScale);
				LV_InsertItem(hWndValue, i, 15, wszBuffer);

				//Default to unchecked
				LV_SetItemState(hWndValue, i, 0,  INDEXTOSTATEIMAGEMASK(STATE_UNCHECKED),  LVIS_STATEIMAGEMASK);
			}


			//Go through and check the existing columns, that are in our current bindings
			for(i=0; i<pCDataAccess->m_Bindings.GetCount(); i++)
			{
				DBORDINAL iOrdinal = pCDataAccess->m_Bindings[i].iOrdinal;
				DBORDINAL iIndex = (iOrdinal && !pCDataAccess->m_ColumnInfo[0].iOrdinal==0) ? iOrdinal-1 : iOrdinal;
				LV_SetItemState(hWndValue, (INDEX)iIndex, 0,  INDEXTOSTATEIMAGEMASK(STATE_CHECKED),  LVIS_STATEIMAGEMASK);
			}

			//AutoSize columns (column headers);
			for(i=0; i<=15; i++)
				SendMessage(hWndValue, LVM_SETCOLUMNWIDTH, i, (LPARAM)LVSCW_AUTOSIZE_USEHEADER);

			CenterDialog(hWnd);
			
			if(FAILED(hr))
			{
				//Free Bindings
				FreeBindings(&cBindings, &rgBindings);
				EndDialog(hWnd, FALSE);
			}

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
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);

					//Which object are we calling this from
					CDataAccess* pCDataAccess = SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
					ASSERT(pCDataAccess);

					DBCOUNTITEM cSelBindings = 0;
					DBBINDING* rgSelBindings = NULL;
					HWND hWndValue	= ::GetDlgItem(hWnd, IDL_COLUMNS);
					HACCESSOR hAccessor = NULL;
					DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA;

					INDEX i=0;
					HRESULT hr = S_OK;

					//Now that the user has selected all the columns they wish to 
					//have in the Accessor, we need to loop through all the 
					//checked columns and add them to our list...
					INDEX cItems = (INDEX)SendMessage(hWndValue, LVM_GETITEMCOUNT, 0, 0);
					SAFE_ALLOC(rgSelBindings, DBBINDING, cItems);
					
					for(i=0; i<cItems; i++)
					{
						//Get the Image
						BOOL bChecked = LV_GetItemState(hWndValue, i, LVIS_STATEIMAGEMASK) & INDEXTOSTATEIMAGEMASK(STATE_CHECKED);

						//Only interested in checked items
						if(!bChecked)
							continue;

						ASSERT(cBindings);
						ASSERT(rgBindings);

						//Mark this Ordinal as checked
					 	CopyBinding(&rgSelBindings[cSelBindings], &rgBindings[i]);
						cSelBindings++;
					}

					//Now that we have the columns, Create the Accessor
					hr = pCDataAccess->CreateAccessor(dwAccessorFlags, cSelBindings, rgSelBindings, 0, &hAccessor);

					//Free Bindings
					FreeBindings(&cBindings, &rgBindings);
					
					if(SUCCEEDED(hr))
					{
						//Release Previous Accessor
						pCDataAccess->ReleaseAccessor(&pCDataAccess->m_hAccessor);
						
						//Use the new Bindings
						pCDataAccess->m_hAccessor	= hAccessor;
						pCDataAccess->m_cbRowSize	= cbRowSize;
						pCDataAccess->m_Bindings.Attach(cSelBindings, rgSelBindings);
						
						//The new bindings may not be the same size as our original
						//Or might not have existined original if AutiQI was off...
						SAFE_REALLOC(pCDataAccess->m_pData, BYTE, cbRowSize);

						//Now actually refetch the Data with the new Accessor
						if(SUCCEEDED(hr = pThis->m_pCDataGrid->RestartPosition()))
							hr = pThis->m_pCDataGrid->RefreshData();

						EndDialog(hWnd, TRUE);
						return 0;
					}
				
				CLEANUP:
					//Free Bindings
					FreeBindings(&cBindings, &rgBindings);
					FreeBindings(&cSelBindings, &rgSelBindings);
					return 0;
				}

				case IDCANCEL:
				{
					//Free Bindings
					FreeBindings(&cBindings, &rgBindings);
					EndDialog(hWnd, FALSE);
					return 0;
				}
				
				case IDB_CLEARALL:
				{
					HWND hWndValue  = ::GetDlgItem(hWnd, IDL_COLUMNS);

					//Now that the user has selected to select all columns
					//We need to loop through all the unchecked and check them...
					INDEX cItems = (INDEX)SendMessage(hWndValue, LVM_GETITEMCOUNT, 0, 0);
					for(INDEX i=0; i<cItems; i++)
					{
						//Check the property state
						LV_SetItemState(hWndValue, i, 0,  INDEXTOSTATEIMAGEMASK(s_bClearAll ? STATE_UNCHECKED : STATE_CHECKED),  LVIS_STATEIMAGEMASK);
					}

					//Clear All is a toggle...
					if(s_bClearAll)
					{
						s_bClearAll = FALSE;
						::SetWindowText(::GetDlgItem(hWnd, IDB_CLEARALL), "SelectAll");
					}
					else
					{
						s_bClearAll = TRUE;
						::SetWindowText(::GetDlgItem(hWnd, IDB_CLEARALL), "ClearAll");
					}
					return 0;
				}
			}
			break;
		}//WM_COMMAND

	}//switch message

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::SetParameterInfoProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::SetParameterInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static DWORD	dwSavedFlags	= 0;
	static INDEX	iSavedName		= CB_ERR;
	static CComboBoxLite	s_CComboTypes;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis	= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			INDEX iSel = 0;
			HRESULT hr = S_OK;

			//Controls
			HWND hWndOrdinal	= ::GetDlgItem(hWnd, IDE_ORDINAL);
			HWND hWndParamName	= ::GetDlgItem(hWnd, IDC_NAME);
			HWND hWndFlags		= ::GetDlgItem(hWnd, IDL_FLAGS);
			
			//Set Ordinal
			SendMessage(hWndOrdinal, WM_SETTEXT, 0, (LPARAM)"1");

			//Set Parameter Name		
			SendMessage(hWndParamName, CB_ADDSTRING, 0, (LPARAM)"<NULL>");
			iSel = (INDEX)SendMessage(hWndParamName, CB_ADDSTRING, 0, (LPARAM)"Parameter1");
			iSavedName = (INDEX)SendMessage(hWndParamName, CB_SETCURSEL, iSavedName!=CB_ERR ? iSavedName : iSel, 0);
		
			//Add support for indicating NULL
			s_CComboTypes.CreateIndirect(hWnd, IDC_TYPE);
			s_CComboTypes.AddString(L"<NULL>");
			
			//Fill in Combo with "Standard" Defined Type Names (defined in SetParameterInfo)
			//NOTE: We could just use the DBTYPE list (g_rgDBTypes), but that gives many
			//confusing types (DBTYPE_STR) which people are apt to use but are not standard
			//so currently no provider supports them.  So just list all those defined by the spec
			//and the combo box allows the user to enter other (not in the list)
			s_CComboTypes.AddString(L"DBTYPE_I1");
			s_CComboTypes.AddString(L"DBTYPE_I2");
			s_CComboTypes.AddString(L"DBTYPE_I4");
			s_CComboTypes.AddString(L"DBTYPE_I8");
			s_CComboTypes.AddString(L"DBTYPE_UI1");
			s_CComboTypes.AddString(L"DBTYPE_UI2");
			s_CComboTypes.AddString(L"DBTYPE_UI4");
			s_CComboTypes.AddString(L"DBTYPE_UI8");
			s_CComboTypes.AddString(L"DBTYPE_R4");
			s_CComboTypes.AddString(L"DBTYPE_R8");
			s_CComboTypes.AddString(L"DBTYPE_CY");
			s_CComboTypes.AddString(L"DBTYPE_DECIMAL");
			s_CComboTypes.AddString(L"DBTYPE_NUMERIC");
			s_CComboTypes.AddString(L"DBTYPE_BOOL");
			s_CComboTypes.AddString(L"DBTYPE_ERROR");
			s_CComboTypes.AddString(L"DBTYPE_UDT");
			s_CComboTypes.AddString(L"DBTYPE_VARIANT");
			s_CComboTypes.AddString(L"DBTYPE_IDISPATCH");
			s_CComboTypes.AddString(L"DBTYPE_IUNKNOWN");
			s_CComboTypes.AddString(L"DBTYPE_GUID");
			s_CComboTypes.AddString(L"DBTYPE_DATE");
			s_CComboTypes.AddString(L"DBTYPE_DBDATE");
			s_CComboTypes.AddString(L"DBTYPE_DBTIME");
			s_CComboTypes.AddString(L"DBTYPE_DBTIMESTAMP");
			s_CComboTypes.AddString(L"DBTYPE_BSTR");
			s_CComboTypes.AddString(L"DBTYPE_CHAR");
			iSel = s_CComboTypes.AddString(L"DBTYPE_VARCHAR");
			s_CComboTypes.AddString(L"DBTYPE_LONGVARCHAR");
			s_CComboTypes.AddString(L"DBTYPE_WCHAR");
			s_CComboTypes.AddString(L"DBTYPE_WVARCHAR");
			s_CComboTypes.AddString(L"DBTYPE_WLONGVARCHAR");
			s_CComboTypes.AddString(L"DBTYPE_BINARY");
			s_CComboTypes.AddString(L"DBTYPE_VARBINARY");
			s_CComboTypes.AddString(L"DBTYPE_LONGVARBINARY");
			s_CComboTypes.AddString(L"DBTYPE_FILENAME");
			s_CComboTypes.AddString(L"DBTYPE_VARNUMERIC");
			s_CComboTypes.AddString(L"DBTYPE_PROPVARIANT");

			//Set current Selection
			if(s_CComboTypes.RestoreSelection() == CB_ERR)
				s_CComboTypes.SetCurSel(iSel);

			const static WIDENAMEMAP g_rgParamFlags[] = 
			{
				VALUE_WCHAR(DBPARAMFLAGS_ISINPUT),
				VALUE_WCHAR(DBPARAMFLAGS_ISOUTPUT),
				VALUE_WCHAR(DBPARAMFLAGS_ISSIGNED),
				VALUE_WCHAR(DBPARAMFLAGS_ISNULLABLE),
				VALUE_WCHAR(DBPARAMFLAGS_ISLONG),
				VALUE_WCHAR(DBPARAMFLAGS_SCALEISNEGATIVE),
			};

			//Populate the Flags ListBox
			SendMessage(hWndFlags, LB_RESETCONTENT, 0, 0);
			for(ULONG i=0; i<NUMELE(g_rgParamFlags); i++)
			{
				INDEX iSel = (INDEX)wSendMessage(hWndFlags, LB_ADDSTRING, 0, g_rgParamFlags[i].pwszName);
				SendMessage(hWndFlags,	LB_SETITEMDATA,	iSel, (LPARAM)g_rgParamFlags[i].lItem);
				
				//Reselect all previously selected items...
				SendMessage(hWndFlags, LB_SETSEL, (dwSavedFlags & g_rgParamFlags[i].lItem) == (DWORD)g_rgParamFlags[i].lItem, i);
			}
			
			//Send a selection change to the ComboBox so it updates everything
			//Size, Precision, and Scale
			SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDC_TYPE, hWnd, CBN_SELCHANGE));

			CenterDialog(hWnd);
			if(FAILED(hr))
				EndDialog(hWnd, FALSE);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			//CBN_SELCHANGE
			switch(GET_WM_COMMAND_CMD(wParam, lParam))
			{
				//Selection change in a combo box occurred
				case CBN_SELCHANGE:
				{	
					//See which combo box has changed
					switch(GET_WM_COMMAND_ID(wParam, lParam))
					{
						case IDC_TYPE:
						{
							WCHAR wszBuffer[MAX_NAME_LEN+1];
							HWND hWndSize		= ::GetDlgItem(hWnd, IDE_SIZE);
							HWND hWndPrecision	= ::GetDlgItem(hWnd, IDE_PRECISION);
							HWND hWndScale		= ::GetDlgItem(hWnd, IDE_SCALE);

							//Get the Selected Type
							s_CComboTypes.GetSelText(wszBuffer, MAX_NAME_LEN);
							DBTYPE wType = GetDBType(wszBuffer);
							s_CComboTypes.SaveSelection();
			
							//Get Default Size,Prec,Scale for this type...
							DBLENGTH ulMaxSize = 0;
							BYTE  bPrecision, bScale;
							GetDBTypeMaxSize(wType, &ulMaxSize, &bPrecision, &bScale);

							//Set Size
							wSendMessageFmt(hWndSize,		WM_SETTEXT, 0, L"%lu", ulMaxSize!=0 ? ulMaxSize : 255);
							//Set Precision
							wSendMessageFmt(hWndPrecision,	WM_SETTEXT, 0, L"%d", bPrecision);
							//Set Scale
							wSendMessageFmt(hWndScale,		WM_SETTEXT, 0, L"%d", bScale);
							return 0;
						}
					}
					break;
				}
			}
							
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);
					CCommand* pCCommand = SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
					WCHAR wszNameBuffer[MAX_NAME_LEN];
					WCHAR wszTypeBuffer[MAX_NAME_LEN];
					HRESULT hr = S_OK;

					//Controls
					HWND hWndOrdinal	= ::GetDlgItem(hWnd, IDE_ORDINAL);
					HWND hWndFlags		= ::GetDlgItem(hWnd, IDL_FLAGS);
					HWND hWndSize		= ::GetDlgItem(hWnd, IDE_SIZE);
					HWND hWndPrecision	= ::GetDlgItem(hWnd, IDE_PRECISION);
					HWND hWndScale		= ::GetDlgItem(hWnd, IDE_SCALE);
			
					LONG lValue = 0;
					ULONG cParams = 0;

					//Alloc Params
					cParams = 1;
					DB_UPARAMS ulParamOrdinal = 0;
					DBPARAMBINDINFO dbParamBindInfo;

					//Setup ParamBindInfo
					CComboBoxLite comboParamName(hWnd, IDC_NAME);
					iSavedName = comboParamName.GetSelText(wszNameBuffer, MAX_NAME_LEN);
					dbParamBindInfo.pwszName = iSavedName == 0 ? NULL : wszNameBuffer;
					
					GetEditBoxValue(hWndOrdinal,	&lValue, 0/*Min*/);
					ulParamOrdinal = lValue;

					GetEditBoxValue(hWndSize,		&lValue, 0/*Min*/);
					dbParamBindInfo.ulParamSize = lValue;
						
					GetEditBoxValue(hWndPrecision,	&lValue, 0, UCHAR_MAX);
					dbParamBindInfo.bPrecision = (BYTE)lValue;
					
					GetEditBoxValue(hWndScale,		&lValue, 0, UCHAR_MAX);
					dbParamBindInfo.bScale = (BYTE)lValue;

					//Get TypeName
					INDEX iSel = s_CComboTypes.GetSelText(wszTypeBuffer, MAX_NAME_LEN);
					dbParamBindInfo.pwszDataSourceType = iSel == 0 ? NULL : wszTypeBuffer;

					//Obtain all Flags Selected Items...
					INDEX iSelCount = (INDEX)SendMessage(hWndFlags, LB_GETSELCOUNT, 0, 0);
					ASSERT(iSelCount < 20);
					LONG rgSelItems[20];
					SendMessage(hWndFlags, LB_GETSELITEMS, (WPARAM)20, (LPARAM)rgSelItems);

					dwSavedFlags = 0;
					for(LONG i=0; i<iSelCount; i++)
						dwSavedFlags |= SendMessage(hWndFlags, LB_GETITEMDATA, rgSelItems[i], 0);
					dbParamBindInfo.dwFlags = dwSavedFlags;

					//SetParameterInfo
					ASSERT(pCCommand->m_pICommandWithParameters);
					XTEST(hr = pCCommand->m_pICommandWithParameters->SetParameterInfo(cParams, &ulParamOrdinal, &dbParamBindInfo));
					TESTC(TRACE_METHOD(hr, L"ICommandWithParameters::SetParameterInfo(%lu, &%Id, 0x%p)", cParams, ulParamOrdinal, &dbParamBindInfo));
									
				CLEANUP:
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}

				case IDB_RESET:
				{
					//Get the "this" pointer
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);
					CCommand* pCCommand = SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
					HRESULT hr = S_OK;

					//SetParameterInfo
					ASSERT(pCCommand->m_pICommandWithParameters);
					XTEST(hr = pCCommand->m_pICommandWithParameters->SetParameterInfo(0, NULL, NULL));
					TRACE_METHOD(hr, L"ICommandWithParameters::SetParameterInfo(0, NULL, NULL)");
					return 0;
				}
			}
			break;
		}//WM_COMMAND

	}//switch message

	return FALSE;
}



////////////////////////////////////////////////////////////////
// CMDIChild::GetParameterInfoProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetParameterInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			WCHAR wszBuffer[MAX_NAME_LEN+1];

			//Save the "this" pointer
			CMDIChild* pThis	= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CCommand* pCCommand = SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
			HRESULT hr = S_OK;

			ULONG i = 0;
			DB_UPARAMS cParams = 0;
			DBPARAMINFO* rgParamInfo = NULL;
			WCHAR* pwszNamesBuffer = NULL;

			//Setup column headers
			HWND hWndName		= ::GetDlgItem(hWnd, IDL_NAMES);
			HWND hWndValue		= ::GetDlgItem(hWnd, IDL_VALUES);
			HWND hWndHelp		= ::GetDlgItem(hWnd, IDT_HELPMSG);
			
			//Set Window Titles
			SendMessage(hWnd,		WM_SETTEXT, 0, (LPARAM)"ICommandWithParameters::GetParameterInfo");
			SendMessage(hWndHelp,	WM_SETTEXT, 0, (LPARAM)"ParameterInfo");

			//Use Extended ListView Styles!
			SendMessage(hWndName,	LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);
			SendMessage(hWndValue,	LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

			//Set image list to the Window 
			ListView_SetImageList(hWndValue, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
			ListView_SetImageList(hWndName, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

			//Now ICommandWithParameters::GetParameterInfo
			XTESTC(hr = pCCommand->GetParameterInfo(&cParams, &rgParamInfo, &pwszNamesBuffer));

			//We need to the ListView
			//Headers/Columns contain ColInfo information 
			//Rows are per columns
			
			//ListView Columns
			LV_InsertColumn(hWndName,	0,		L"ParamName");
			LV_InsertItem(hWndName,		0,		0,	L"Ordinal");
			LV_InsertItem(hWndName,		1,		0,	L"dwFlags");
			LV_InsertItem(hWndName,		2,		0,	L"pTypeInfo");
			LV_InsertItem(hWndName,		3,		0,	L"wType");
			LV_InsertItem(hWndName,		4,		0, 	L"ulParamSize");
			LV_InsertItem(hWndName,		5,		0,	L"bPrecision");
			LV_InsertItem(hWndName,		6,		0,	L"bScale");
			
			//AutoSize column
			SendMessage(hWndName, LVM_SETCOLUMNWIDTH, 0,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);

			for(i=0; i<cParams; i++)
			{	
				ASSERT(rgParamInfo);
				DBPARAMINFO* pParamInfo = &rgParamInfo[i];

				//Column Header
				LV_InsertColumn(hWndValue, i, pParamInfo->pwszName, IMAGE_NONE);

				//Ordinal (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Id", pParamInfo->iOrdinal);
				LV_InsertItem(hWndValue, 0, i, wszBuffer);

				//dwFlags (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pParamInfo->dwFlags);
				LV_InsertItem(hWndValue, 1, i, wszBuffer);

				//pTypeInfo (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", pParamInfo->pTypeInfo);
				LV_InsertItem(hWndValue, 2, i, wszBuffer);
				
				//TYPE (subitem)
				LV_InsertItem(hWndValue, 3, i, GetDBTypeName(pParamInfo->wType));

				//ulParamSize (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pParamInfo->ulParamSize);
				LV_InsertItem(hWndValue, 4, i, wszBuffer);
				
				//Precision (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pParamInfo->bPrecision);
				LV_InsertItem(hWndValue, 5, i, wszBuffer);

				//Scale (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pParamInfo->bScale);
				LV_InsertItem(hWndValue, 6, i, wszBuffer);
			}

			//AutoSize all columns
			for(i=0; i<cParams; i++)
				SendMessage(hWndValue, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
						
		CLEANUP:
			CenterDialog(hWnd);
			SAFE_FREE(rgParamInfo);
			SAFE_FREE(pwszNamesBuffer);
			if(FAILED(hr))
				EndDialog(hWnd, FALSE);
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

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
			
			//ListView
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
					return 0;

				case LVN_ITEMCHANGED:
				{
					HWND hWndName		= ::GetDlgItem(hWnd, IDL_NAMES);
					HWND hWndValue	= ::GetDlgItem(hWnd, IDL_VALUES);

					if(pNMListView->uNewState & LVNI_FOCUSED &&
				 		pNMListView->uNewState & LVNI_SELECTED)
					{
						if(wParam == IDL_VALUES)
						{
							SyncSibling(hWndName, hWndValue);
	           				return FALSE;
						}

						if(wParam == IDL_NAMES)
						{
							SyncSibling(hWndValue, hWndName);
	                		return FALSE;
						}
						return UNHANDLED_MSG; //No return Value
					}
				}
			}


		}//WM_NOTIFY
	}//switch message

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::GetLiteralInfoProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetLiteralInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;
			WCHAR wszBuffer[MAX_NAME_LEN+1];

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			IDBInfo* pIDBInfo = SOURCE_GETINTERFACE(pThis->m_pCSource, IDBInfo);
			HRESULT hr = S_OK;

			ULONG i,cLiteralInfo = 0;
			DBLITERALINFO* rgLiteralInfo = NULL;
			WCHAR* pStringBuffer = NULL;

			//Setup column headers
			HWND hWndLiterals   = ::GetDlgItem(hWnd, IDL_VALUES);
			HWND hWndNames		= ::GetDlgItem(hWnd, IDL_NAMES);
			HWND hWndHelp		= ::GetDlgItem(hWnd, IDT_HELPMSG);
			
			//Set Window Titles
			SendMessage(hWnd,		WM_SETTEXT, 0, (LPARAM)"IDBInfo::GetLiteralInfo");
			SendMessage(hWndHelp,	WM_SETTEXT, 0, (LPARAM)"Information about literals used in text DDL");

			const static WIDENAMEMAP rgLiterals[] =
			{
				VALUE_WCHAR(DBLITERAL_INVALID	),
				VALUE_WCHAR(DBLITERAL_BINARY_LITERAL	),
				VALUE_WCHAR(DBLITERAL_CATALOG_NAME	),
				VALUE_WCHAR(DBLITERAL_CATALOG_SEPARATOR	),
				VALUE_WCHAR(DBLITERAL_CHAR_LITERAL	),
				VALUE_WCHAR(DBLITERAL_COLUMN_ALIAS	),
				VALUE_WCHAR(DBLITERAL_COLUMN_NAME	),
				VALUE_WCHAR(DBLITERAL_CORRELATION_NAME	),
				VALUE_WCHAR(DBLITERAL_CURSOR_NAME	),
				VALUE_WCHAR(DBLITERAL_ESCAPE_PERCENT_PREFIX	),
				VALUE_WCHAR(DBLITERAL_ESCAPE_PERCENT_SUFFIX  ),
				VALUE_WCHAR(DBLITERAL_ESCAPE_UNDERSCORE_PREFIX ),
				VALUE_WCHAR(DBLITERAL_ESCAPE_UNDERSCORE_SUFFIX ),
				VALUE_WCHAR(DBLITERAL_INDEX_NAME	),
				VALUE_WCHAR(DBLITERAL_LIKE_PERCENT	),
				VALUE_WCHAR(DBLITERAL_LIKE_UNDERSCORE	),
				VALUE_WCHAR(DBLITERAL_PROCEDURE_NAME	),
				VALUE_WCHAR(DBLITERAL_QUOTE_PREFIX	),
				VALUE_WCHAR(DBLITERAL_SCHEMA_NAME	),
				VALUE_WCHAR(DBLITERAL_TABLE_NAME	),
				VALUE_WCHAR(DBLITERAL_TEXT_COMMAND	),
				VALUE_WCHAR(DBLITERAL_USER_NAME	),
				VALUE_WCHAR(DBLITERAL_VIEW_NAME	),
				
				VALUE_WCHAR(DBLITERAL_CUBE_NAME	),
				VALUE_WCHAR(DBLITERAL_DIMENSION_NAME	),
				VALUE_WCHAR(DBLITERAL_HIERARCHY_NAME	),
				VALUE_WCHAR(DBLITERAL_LEVEL_NAME	),
				VALUE_WCHAR(DBLITERAL_MEMBER_NAME	),
				VALUE_WCHAR(DBLITERAL_PROPERTY_NAME	),

				//2.0
				VALUE_WCHAR(DBLITERAL_SCHEMA_SEPARATOR	),
				VALUE_WCHAR(DBLITERAL_QUOTE_SUFFIX	),
			};

			//Use Extended ListView Styles!
			SendMessage(hWndLiterals,	LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
			SendMessage(hWndNames,		LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);

			//Set image list to the Table Window 
			ListView_SetImageList(hWndLiterals, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
			ListView_SetImageList(hWndNames, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

			//Obtain GetLiteralInfo (for all supported literals)
			ASSERT(pIDBInfo);
			XTEST(hr = pIDBInfo->GetLiteralInfo(0, NULL, &cLiteralInfo, &rgLiteralInfo, &pStringBuffer));
			TESTC(TRACE_METHOD(hr, L"IDBInfo::GetLiteralInfo(&%lu, &0x%p, &0x%p)", cLiteralInfo, rgLiteralInfo, pStringBuffer));

			//We need to the ListView
			//Headers/Columns contain ColInfo information 
			//Rows are per columns
			
			//ListView NAMES
			LV_InsertColumn(hWndNames,	0,		L"DBLITERAL");
			LV_InsertItem(hWndNames,	0,		0,	L"LiteralValue");
			LV_InsertItem(hWndNames,	1,		0,	L"InvalidChars");
			LV_InsertItem(hWndNames,	2,		0,	L"InvalidStartingChars");
			LV_InsertItem(hWndNames,	3,		0, 	L"Supported");
			LV_InsertItem(hWndNames,	4,		0,	L"MaxLen");
			
			//AutoSize column
			SendMessage(hWndNames, LVM_SETCOLUMNWIDTH, 0,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);

			for(i=0; i<cLiteralInfo; i++)
			{	
				ASSERT(rgLiteralInfo);
				DBLITERALINFO* pLiteralInfo = &rgLiteralInfo[i];

				//Try to find the Actual "DBLITERAL" name...
				WCHAR* pwszLiteralName = NULL;
				for(LONG j=0; j<NUMELE(rgLiterals); j++)
				{
					if(pLiteralInfo->lt == (DBLITERAL)rgLiterals[j].lItem)
						pwszLiteralName = rgLiterals[j].pwszName;
				}
				

				//Column Header (DBLITERAL)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pLiteralInfo->lt);
				LV_InsertColumn(hWndLiterals, i, pwszLiteralName ? pwszLiteralName : wszBuffer);

				//LiteralValue (SubItem)
				LV_InsertItem(hWndLiterals, 0, i, pLiteralInfo->pwszLiteralValue ? pLiteralInfo->pwszLiteralValue : NULL);

				//InvalidChars (subitem)
				LV_InsertItem(hWndLiterals, 1, i, pLiteralInfo->pwszInvalidChars ? pLiteralInfo->pwszInvalidChars : NULL);
				
				//InvalidChars (subitem)
				LV_InsertItem(hWndLiterals, 2, i, pLiteralInfo->pwszInvalidStartingChars ? pLiteralInfo->pwszInvalidStartingChars : NULL);

				//Supported (SubItem)
				LV_InsertItem(hWndLiterals, 3, i, NULL, PARAM_NONE, pLiteralInfo->fSupported ? IMAGE_CHECKED : IMAGE_UNCHECKED);

				//cbMaxLen (SubItem)
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pLiteralInfo->cchMaxLen);
				LV_InsertItem(hWndLiterals, 4, i, wszBuffer);
			}	

			//AutoSize all columns
			for(i=0; i<cLiteralInfo; i++)
				SendMessage(hWndLiterals, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
						
		CLEANUP:
			CenterDialog(hWnd);
			SAFE_FREE(rgLiteralInfo);
			SAFE_FREE(pStringBuffer);
			if(FAILED(hr))
				EndDialog(hWnd, FALSE);
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

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
			
			//ListView
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
					return 0;

				case LVN_ITEMCHANGED:
				{
					HWND hWndLiterals  = ::GetDlgItem(hWnd, IDL_VALUES);
					HWND hWndNames   = ::GetDlgItem(hWnd, IDL_NAMES);

					if(pNMListView->uNewState & LVNI_FOCUSED &&
				 		pNMListView->uNewState & LVNI_SELECTED)
					{
						if(wParam == IDL_VALUES)
						{
							SyncSibling(hWndNames, hWndLiterals);
	           				return FALSE;
						}

						if(wParam == IDL_NAMES)
						{
							SyncSibling(hWndLiterals, hWndNames);
	                		return FALSE;
						}
						return UNHANDLED_MSG; //No return Value
					}
				}
			}


		}//WM_NOTIFY
	}

	return FALSE;
}



////////////////////////////////////////////////////////////////
// CMDIChild::AbortTransactionProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::AbortTransactionProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild*		pThis			= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND			hWndRetaining	= ::GetDlgItem(hWnd, IDC_RETAINING);
			HWND			hWndAsynch		= ::GetDlgItem(hWnd, IDC_ASYNCH);

			//Need to fill in the RETAINING ComboBox
			INDEX iSel = (INDEX)SendMessage(hWndRetaining, CB_ADDSTRING,	0, (LPARAM)"TRUE");
			SendMessage(hWndRetaining, CB_SETITEMDATA, iSel, (LPARAM)TRUE);
			iSel = (INDEX)SendMessage(hWndRetaining, CB_ADDSTRING,	0, (LPARAM)"FALSE");
			SendMessage(hWndRetaining, CB_SETITEMDATA, iSel, (LPARAM)FALSE);
			//SetDefault - TRUE
			SendMessage(hWndRetaining, CB_SETCURSEL,	0, (LPARAM)0);
			
			//Need to fill in the ASYNCH ComboBox
			iSel = (INDEX)SendMessage(hWndAsynch, CB_ADDSTRING,	0, (LPARAM)"TRUE");
			SendMessage(hWndAsynch, CB_SETITEMDATA, iSel, (LPARAM)TRUE);
			iSel = (INDEX)SendMessage(hWndAsynch, CB_ADDSTRING,	0, (LPARAM)"FALSE");
			SendMessage(hWndAsynch, CB_SETITEMDATA, iSel, (LPARAM)FALSE);
			//SetDefault - FALSE
			SendMessage(hWndAsynch, CB_SETCURSEL,	1, 0);

			//Need to fill in the Txn ComboBox
			pThis->SetupTransactionCombo(::GetDlgItem(hWnd, IDC_TRANSACTION));

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
					CMDIChild*		pThis			= (CMDIChild*)GetThis(hWnd);

					ITransaction*	pITransaction	= NULL;
					HRESULT			hr				= S_OK;

					HWND			hWndRetaining	= ::GetDlgItem(hWnd, IDC_RETAINING);
					HWND			hWndAsynch		= ::GetDlgItem(hWnd, IDC_ASYNCH);
					HWND			hWndTransaction	= ::GetDlgItem(hWnd, IDC_TRANSACTION);

					//Just Need to Obtain selected RETAINING from Combo
					INDEX iSel = (INDEX)SendMessage(hWndRetaining, CB_GETCURSEL, 0, 0);
					BOOL fRetaining = (BOOL)SendMessage(hWndRetaining, CB_GETITEMDATA, iSel, 0);
					
					//Just Need to Obtain selected ASYNCH from Combo
					iSel = (INDEX)SendMessage(hWndAsynch, CB_GETCURSEL, 0, 0);
					BOOL fAsynch = (BOOL)SendMessage(hWndAsynch, CB_GETITEMDATA, iSel, 0);

					//get the ITransaction Pointer
					iSel = (INDEX)SendMessage(hWndTransaction, CB_GETCURSEL, 0, 0);
					if(iSel != CB_ERR)
						pITransaction = (ITransaction*)SendMessage(hWndTransaction, CB_GETITEMDATA, iSel, 0);
					
					//Obtain the Transaction interface
					if(!pITransaction)
						pITransaction = SOURCE_GETINTERFACE(pThis->m_pCSource, ITransaction);
					ASSERT(pITransaction);

					//Now Abort the Transaction
					XTEST(hr = pITransaction->Abort(NULL, fRetaining, fAsynch));
					TRACE_METHOD(hr, L"ITransaction::Abort(NULL, %s, %s)", fRetaining ? L"TRUE" : L"FALSE", fAsynch ? L"TRUE" : L"FALSE");
					
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
// CMDIChild::CommitTransactionProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::CommitTransactionProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Controls
	static CComboBoxLite	s_comboXACTTC;
	static CComboBoxLite	s_comboRetaining;
	static CComboBoxLite	s_comboTransaction;
	
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild*		pThis			= (CMDIChild*)SetThis(hWnd, (void*)lParam);
		
			//Controls
			s_comboXACTTC.CreateIndirect(hWnd, IDC_XACTTC);
			s_comboRetaining.CreateIndirect(hWnd, IDC_RETAINING);
			s_comboTransaction.CreateIndirect(hWnd, IDC_TRANSACTION);
			
			//Need to fill in the RETAINING ComboBox
			s_comboRetaining.AddString(L"TRUE",		TRUE);
			s_comboRetaining.AddString(L"FALSE",	FALSE);
			s_comboRetaining.SetSelValue(FALSE);
			
			//Need to fill in the XACTTC ComboBox - and set the defualt
			s_comboXACTTC.Populate(g_cXACTTC, g_rgXACTTC);
			s_comboXACTTC.AddString(L"0",	0);
			s_comboXACTTC.SetSelValue(XACTTC_SYNC_PHASETWO);

			//Need to fill in the Txn ComboBox
			pThis->SetupTransactionCombo(s_comboTransaction.m_hWnd);

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
					CMDIChild*		pThis			= (CMDIChild*)GetThis(hWnd);
					HRESULT			hr				= S_OK;

					//Obtain selected RETAINING from Combo
					BOOL fRetaining = (BOOL)s_comboRetaining.GetSelValue();
					
					//Obtain selected hWndXACTTC from Combo
					DWORD xacttc = (DWORD)s_comboXACTTC.GetSelValue();

					//get the ITransactionPointer
					ITransaction* pITransaction = (ITransaction*)s_comboTransaction.GetItemParam(s_comboTransaction.GetCurSel());

					//Obtain the Transaction interface
					if(!pITransaction || (LPARAM)pITransaction==CB_ERR)
						pITransaction = SOURCE_GETINTERFACE(pThis->m_pCSource, ITransaction);
					ASSERT(pITransaction);

					//Now Commit the Transaction
					XTEST(hr = pITransaction->Commit(fRetaining, xacttc, 0));
					TRACE_METHOD(hr, L"ITransaction::Commit(%s, %d, 0)", fRetaining ? L"TRUE" : L"FALSE", xacttc);
						
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
// CMDIChild::GetTransactionInfo
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetTransactionInfo(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild*		pThis				= (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Need to fill in the Txn ComboBox
			pThis->SetupTransactionCombo(::GetDlgItem(hWnd, IDC_TRANSACTION));

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
					CMDIChild*		pThis				= (CMDIChild*)GetThis(hWnd);

					HWND			hWndTransaction		= ::GetDlgItem(hWnd, IDC_TRANSACTION);
					ITransaction*	pITransaction		= NULL;
					XACTTRANSINFO	XactInfo			= {0};
					HRESULT			hr					= S_OK;

					//get the ITransactionPointer
					INDEX iSel = (INDEX)SendMessage(hWndTransaction, CB_GETCURSEL, 0, 0);
					if(iSel != CB_ERR)
						pITransaction = (ITransaction*)SendMessage(hWndTransaction, CB_GETITEMDATA, iSel, 0);

					//Obtain the Transaction interface
					if(!pITransaction)
						pITransaction = SOURCE_GETINTERFACE(pThis->m_pCSource, ITransaction);
					ASSERT(pITransaction);

					//ITransaction::GetTransactionInfo
					XTEST(hr = pITransaction->GetTransactionInfo(&XactInfo));
					TRACE_METHOD(hr, L"ITransaction::GetTransactionInfo(%s, %d, %s, %d, %d, %d)", /*XactInfo.uow,*/ GetMapName(XactInfo.isoLevel, g_cIsoLevels, g_rgIsoLevels), XactInfo.isoFlags, GetMapName(XactInfo.grfTCSupported, g_cXACTTC, g_rgXACTTC) , XactInfo.grfRMSupported, XactInfo.grfTCSupportedRetaining, XactInfo.grfRMSupportedRetaining);

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
// CMDIChild::StartTransactionProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::StartTransactionProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild*		pThis			= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND			hWndIsoLevel	= ::GetDlgItem(hWnd, IDC_ISOLEVEL);

			//Title
			if(pThis->m_idSource == IDM_ITRANSACTIONDISPENSOR_BEGINTRANSACTION)
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)"TransactionDispensor::BeginTransaction");

			//Need to fill in the ISOLEVEL ComboBox
			for(ULONG i=0; i<g_cIsoLevels; i++)
			{
				INDEX iSel = (INDEX)wSendMessage(hWndIsoLevel, CB_ADDSTRING,	0, g_rgIsoLevels[i].pwszName);
				SendMessage(hWndIsoLevel, CB_SETITEMDATA, iSel, (LPARAM)g_rgIsoLevels[i].lItem);
			}

			//SetDefault - READUNCOMMITED
			SendMessage(hWndIsoLevel, CB_SETCURSEL,	2, (LPARAM)0);

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
					CMDIChild*		pThis			= (CMDIChild*)GetThis(hWnd);

					HWND			hWndIsoLevel	= ::GetDlgItem(hWnd, IDC_ISOLEVEL);
					WCHAR			wszBuffer[MAX_NAME_LEN+1] = {0};
					HRESULT			hr = S_OK;

					//Just Need to Obtain selected ISOLEVEL from Combo
					INDEX		iSel		= (INDEX)SendMessage(hWndIsoLevel, CB_GETCURSEL, 0, 0);
					wSendMessage(hWndIsoLevel, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
					ISOLEVEL	isoLevel	= (ISOLEVEL)SendMessage(hWndIsoLevel, CB_GETITEMDATA, iSel, 0);

					if(pThis->m_idSource != IDM_ITRANSACTIONDISPENSOR_BEGINTRANSACTION)
					{
						ULONG ulTransactionLevel = 0;
						ITransactionLocal* pITransactionLocal = SOURCE_GETINTERFACE(pThis->m_pCSource, 	ITransactionLocal);
						ASSERT(pITransactionLocal);

						//ITransactionLocal::StartTransaction
						XTEST(hr = pITransactionLocal->StartTransaction(isoLevel, 0, NULL, &ulTransactionLevel));
						TRACE_METHOD(hr, L"ITransactionLocal::StartTransaction(%s, 0, NULL, &%d)", wszBuffer, ulTransactionLevel);
					}
					else
					{
#ifdef MTSTXN
						ITransaction*			pITransaction			= NULL;
						ITransactionDispenser*	pITransactionDispenser	= NULL;

						// Obtain the ITransactionDispenser Interface pointer
						// by calling DtcGetTransactionManager()
						XTEST(hr = DtcGetTransactionManager	
											(
												NULL,								// LPTSTR	 pszHost,
												NULL,								// LPTSTR	 pszTmName,
												IID_ITransactionDispenser,			// /* in  */ REFIID rid,
												0,									// /* in  */ DWORD	dwReserved1,
												0, 									// /* in  */ WORD	wcbReserved2,
												NULL,								// /* in  */ void FAR * pvReserved2,
												(void**)&pITransactionDispenser 	// /* out */ void** ppvObject
											));
						TRACE_METHOD(hr, L"DtcGetTransactionManager(NULL, NULL, IID_ITransactionDispenser, 0, 0, NULL, &0x%p)", pITransactionDispenser);

						//start a global transaction
						if(SUCCEEDED(hr))
						{
							XTEST(hr = pITransactionDispenser->BeginTransaction	
													(
														NULL,							//	/* [in]  */ IUnknown __RPC_FAR *punkOuter,
														isoLevel,						//	/* [in]  */ ISOLEVEL isoLevel,
														0,								// 	/* [in]  */ ULONG isoFlags,
														NULL,							//	/* [in]  */ ITransactionOptions *pOptions 
														(ITransaction**)&pITransaction	//	/* [out] */ ITransaction **ppTransaction
													));
							TRACE_METHOD(hr, L"ITransactionDispenser::BeginTransaction(NULL, %s, 0, &0x%p)", wszBuffer, pITransaction);
						
							//Add this transaciton to the list
							if(SUCCEEDED(hr))
								pThis->m_pCMainWindow->m_listTransactions.AddTail(pITransaction);
						}
						
						SAFE_RELEASE(pITransactionDispenser);
#endif //MTSTXN
					}
					
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
// CMDIChild::JoinTransactionProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::JoinTransactionProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild*		pThis				= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND			hWndIsoLevel		= ::GetDlgItem(hWnd, IDC_ISOLEVEL);

			//Need to fill in the ISOLEVEL ComboBox
			for(ULONG i=0; i<g_cIsoLevels; i++)
			{
				INDEX iSel = (INDEX)wSendMessage(hWndIsoLevel, CB_ADDSTRING,	0, g_rgIsoLevels[i].pwszName);
				SendMessage(hWndIsoLevel, CB_SETITEMDATA, iSel, (LPARAM)g_rgIsoLevels[i].lItem);
			}
			//SetDefault - READUNCOMMITED
			SendMessage(hWndIsoLevel, CB_SETCURSEL,	2, (LPARAM)0);

			//Need to fill in the Txn ComboBox
			pThis->SetupTransactionCombo(::GetDlgItem(hWnd, IDC_TRANSACTION));

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
					CMDIChild*		pThis				= (CMDIChild*)GetThis(hWnd);

					WCHAR			wszBuffer[MAX_NAME_LEN+1] = {0};
					HWND			hWndIsoLevel		= ::GetDlgItem(hWnd, IDC_ISOLEVEL);
					HWND			hWndTransaction		= ::GetDlgItem(hWnd, IDC_TRANSACTION);
					ITransaction*	pITransaction		= NULL;
					HRESULT			hr					= S_OK;

					//Just Need to Obtain selected ISOLEVEL from Combo
					INDEX iSel = (INDEX)SendMessage(hWndIsoLevel, CB_GETCURSEL, 0, 0);
					wSendMessage(hWndIsoLevel, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
					ISOLEVEL isoLevel = (ISOLEVEL)SendMessage(hWndIsoLevel, CB_GETITEMDATA, iSel, 0);
					
					//get the ITransactionPointer
					iSel = (INDEX)SendMessage(hWndTransaction, CB_GETCURSEL, 0, 0);
					if(iSel != CB_ERR)
						pITransaction = (ITransaction*)SendMessage(hWndTransaction, CB_GETITEMDATA, iSel, 0);

					ITransactionJoin* pITransactionJoin = SOURCE_GETINTERFACE(pThis->m_pCSource, ITransactionJoin);
					ASSERT(pITransactionJoin);

					//ITransactionJoin::JoinTransaction
					XTEST(hr = pITransactionJoin->JoinTransaction(pITransaction, isoLevel, NULL, NULL));
					TRACE_METHOD(hr, L"ITransactionJoin::JoinTransaction(0x%p, %s, 0, NULL)", pITransaction, wszBuffer);

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
// CMDIChild::SetupTransactionCombo
//
/////////////////////////////////////////////////////////////////
BOOL CMDIChild::SetupTransactionCombo(HWND hWnd)
{
	INDEX	iSel	= 0;
	ULONG	cCount	= 0;
	WCHAR	wszBuffer[MAX_NAME_LEN+1] = {0};


	//Need to fill in the Txn ComboBox
	POSITION pos = m_pCMainWindow->m_listTransactions.GetHeadPosition();
	while(pos)
	{
		ITransaction* pITransaction = m_pCMainWindow->m_listTransactions.GetNext(pos);
		StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d - 0x%p", cCount++, pITransaction);
		
		iSel = (INDEX)wSendMessage(hWnd, CB_ADDSTRING,	0, wszBuffer);
		SendMessage(hWnd, CB_SETITEMDATA, iSel, (LPARAM)pITransaction);
	}
	
	//SetDefault - 1st txn
	SendMessage(hWnd, CB_SETCURSEL, 0, (LPARAM)0);

	//Enable or Diable the Combo...
	::EnableWindow(hWnd, !m_pCMainWindow->m_listTransactions.IsEmpty());
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDIChild::ReleaseTransaction
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::ReleaseTransaction(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild*		pThis				= (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Need to fill in the Txn ComboBox
			pThis->SetupTransactionCombo(::GetDlgItem(hWnd, IDC_TRANSACTION));

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
					CMDIChild*		pThis				= (CMDIChild*)GetThis(hWnd);

					HWND			hWndTransaction		= ::GetDlgItem(hWnd, IDC_TRANSACTION);
					ITransaction*	pITransaction		= NULL;
					
					//get the ITransactionPointer
					INDEX iSel = (INDEX)SendMessage(hWndTransaction, CB_GETCURSEL, 0, 0);
					if(iSel != CB_ERR)
						pITransaction = (ITransaction*)SendMessage(hWndTransaction, CB_GETITEMDATA, iSel, 0);

					if(pITransaction)
					{
						//Remove from list
						POSITION iPos = pThis->m_pCMainWindow->m_listTransactions.Find(pITransaction);
						pThis->m_pCMainWindow->m_listTransactions.RemoveAt(iPos);
						
						//ITransaction::Release
						TRACE_RELEASE(pITransaction, L"ITransaction");
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
// CMDIChild::SetTransactionOptionsProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::SetTransactionOptionsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			HWND hWndTimeout	= ::GetDlgItem(hWnd, IDE_TIMEOUT);
			HWND hWndDescription= ::GetDlgItem(hWnd, IDE_DESCRIPTION);
			
			//Defaults
			SendMessage(hWndTimeout, WM_SETTEXT, 0, (LPARAM)"0");
			SendMessage(hWndDescription, WM_SETTEXT, 0, (LPARAM)"");
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
					CMDIChild* pThis							= (CMDIChild*)GetThis(hWnd);
					CTransactionOptions* pCTransactionOptions	= SOURCE_GETOBJECT(pThis->m_pCSource, CTransactionOptions);
					HRESULT hr = S_OK;

					HWND hWndTimeout	= ::GetDlgItem(hWnd, IDE_TIMEOUT);
					HWND hWndDescription= ::GetDlgItem(hWnd, IDE_DESCRIPTION);

					XACTOPT XactOptions;
					XactOptions.ulTimeout = 0;
					XactOptions.szDescription[0] = EOL;

					//Need to Obtain Entered Timeout Value
					if(!GetEditBoxValue(hWndTimeout, (LONG*)&XactOptions.ulTimeout, 0/*Min*/))
						return FALSE;

					//Need to Obtain Description
					SendMessage(hWndDescription, WM_GETTEXT, 40-1, (LPARAM)XactOptions.szDescription);

					//ITransactionOptions::SetOptions
					ASSERT(pCTransactionOptions->m_pITransactionOptions);
					XTEST(hr = pCTransactionOptions->m_pITransactionOptions->SetOptions(&XactOptions));
					TRACE_METHOD(hr, L"ITransactionOptions::SetOptions(%d, \"%S\")", XactOptions.ulTimeout, XactOptions.szDescription);
					
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
// CMDIChild::CanConvertProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::CanConvertProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static DBTYPE wSavedFromType = 0;
	static DBTYPE wSavedToType = 0; 

	static INDEX iSavedFromSel = 28;  //DBTYPE_WSTR
	static INDEX iSavedToSel = iSavedFromSel;
	
	//We have a problem with DBCONVERTFLAGS being orable, but _COLUMN == 0!
	//We will need to keep track of the 0 and non-0 flags, 
	//inorder to restore the users previous selections correctly...
	static LONG fSelColumnFlag = TRUE;						//Default to COLUMN
	static LONG dwSavedConvFlags = DBCONVERTFLAGS_COLUMN;	//Default to COLUMN

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			HWND hWndFromType	= ::GetDlgItem(hWnd, IDC_FROMTYPE);
			HWND hWndToType		= ::GetDlgItem(hWnd, IDC_TOTYPE);
			HWND hWndConvFlags	= ::GetDlgItem(hWnd, IDL_CONVFLAGS);
			INDEX iSel = 0;

			const static WIDENAMEMAP rgConvertFlags[] =
			{						   
				VALUE_WCHAR(DBCONVERTFLAGS_COLUMN	),
				VALUE_WCHAR(DBCONVERTFLAGS_PARAMETER	),
				VALUE_WCHAR(DBCONVERTFLAGS_ISLONG	),
				VALUE_WCHAR(DBCONVERTFLAGS_ISFIXEDLENGTH	),
				VALUE_WCHAR(DBCONVERTFLAGS_FROMVARIANT	),
			};

			ULONG i;
			//Need to fill in the From and To Type ComboBoxs
			for(i=0; i<g_cDBTypes; i++)
			{
				WCHAR* pwszName = g_rgDBTypes[i].pwszName;
				DBTYPE wType = (DBTYPE)g_rgDBTypes[i].lItem;

				//FromType
				iSel = (INDEX)wSendMessage(hWndFromType, CB_ADDSTRING,	0, pwszName);
				SendMessage(hWndFromType, CB_SETITEMDATA, iSel, (LPARAM)wType);
				//ToType
				iSel = (INDEX)wSendMessage(hWndToType, CB_ADDSTRING,	0, pwszName);
				SendMessage(hWndToType, CB_SETITEMDATA, iSel, (LPARAM)wType);
			}

			//Need to select Defaults
			SendMessage(hWndFromType, CB_SETCURSEL, iSavedFromSel, 0);
			SendMessage(hWndToType, CB_SETCURSEL, iSavedToSel, 0);
			
			//Need to also select Default Modifiers
			::CheckDlgButton(hWnd, IDB_FROM_BYREF,		BST2STATE(wSavedFromType & DBTYPE_BYREF));
			::CheckDlgButton(hWnd, IDB_FROM_ARRAY,		BST2STATE(wSavedFromType & DBTYPE_ARRAY));
			::CheckDlgButton(hWnd, IDB_FROM_VECTOR,		BST2STATE(wSavedFromType & DBTYPE_VECTOR));
			::CheckDlgButton(hWnd, IDB_TO_BYREF,		BST2STATE(wSavedToType & DBTYPE_BYREF));
			::CheckDlgButton(hWnd, IDB_TO_ARRAY,		BST2STATE(wSavedToType & DBTYPE_ARRAY));
			::CheckDlgButton(hWnd, IDB_TO_VECTOR,		BST2STATE(wSavedToType & DBTYPE_VECTOR));
			
			//Need to fill in the ConvertFlags (ListBox)
			SendMessage(hWndConvFlags, LB_RESETCONTENT, 0, 0);
			for(i=0; i<NUMELE(rgConvertFlags); i++)
			{
				INDEX iSel = (INDEX)wSendMessage(hWndConvFlags, LB_ADDSTRING, 0, rgConvertFlags[i].pwszName);
				SendMessage(hWndConvFlags,	LB_SETITEMDATA,	iSel, (LPARAM)rgConvertFlags[i].lItem);

				//Select Saved ConvFlags
				if((dwSavedConvFlags & rgConvertFlags[i].lItem) || (fSelColumnFlag && rgConvertFlags[i].lItem == DBCONVERTFLAGS_COLUMN))
					 SendMessage(hWndConvFlags, LB_SETSEL, TRUE, i);
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
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CDataAccess* pCDataAccess	= NULL;
					HRESULT hr = S_OK;

					HWND hWndFromType	= ::GetDlgItem(hWnd, IDC_FROMTYPE);
					HWND hWndToType		= ::GetDlgItem(hWnd, IDC_TOTYPE);
					HWND hWndConvFlags	= ::GetDlgItem(hWnd, IDL_CONVFLAGS);
					HWND hWndResult		= ::GetDlgItem(hWnd, IDT_RESULT);

					//Just Need to Obtain selected FromType from Combo
					iSavedFromSel = (INDEX)SendMessage(hWndFromType, CB_GETCURSEL, 0, 0);
					wSavedFromType = (DBTYPE)SendMessage(hWndFromType, CB_GETITEMDATA, iSavedFromSel, 0);
					
					//Need to obtain any type modifier for the FromType
					if(::IsDlgButtonChecked(hWnd, IDB_FROM_BYREF))
						wSavedFromType |= DBTYPE_BYREF;
					if(::IsDlgButtonChecked(hWnd, IDB_FROM_ARRAY))
						wSavedFromType |= DBTYPE_ARRAY;
					if(::IsDlgButtonChecked(hWnd, IDB_FROM_VECTOR))
						wSavedFromType |= DBTYPE_VECTOR;

					//Just Need to Obtain selected ToType from Combo
					iSavedToSel = (INDEX)SendMessage(hWndToType, CB_GETCURSEL, 0, 0);
					wSavedToType = (DBTYPE)SendMessage(hWndToType, CB_GETITEMDATA, iSavedToSel, 0);

					//Need to obtain any type modifier for the ToType
					if(::IsDlgButtonChecked(hWnd, IDB_TO_BYREF))
						wSavedToType |= DBTYPE_BYREF;
					if(::IsDlgButtonChecked(hWnd, IDB_TO_ARRAY))
						wSavedToType |= DBTYPE_ARRAY;
					if(::IsDlgButtonChecked(hWnd, IDB_TO_VECTOR))
						wSavedToType |= DBTYPE_VECTOR;

					//Obtain all Selected ConvertFlag items...
					INDEX iSelCount = (INDEX)SendMessage(hWndConvFlags, LB_GETSELCOUNT, 0, 0);
					ASSERT(iSelCount < 20);
					LONG rgSelItems[20];
					SendMessage(hWndConvFlags, LB_GETSELITEMS, (WPARAM)20, (LPARAM)rgSelItems);

					fSelColumnFlag = FALSE;
					dwSavedConvFlags = 0;
					for(LONG i=0; i<iSelCount; i++)
					{
						DWORD dwSelFlag = (DWORD)SendMessage(hWndConvFlags, LB_GETITEMDATA, rgSelItems[i], 0);
						if(dwSelFlag == DBCONVERTFLAGS_COLUMN)
							fSelColumnFlag = TRUE;
						dwSavedConvFlags |= dwSelFlag;
					}

					//Obtain the Correct IConvertType interface
					pCDataAccess = SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);
					ASSERT(pCDataAccess);

					//Now Just call IConvertType::CanConvert
					ASSERT(pCDataAccess->m_pIConvertType);
					hr = pCDataAccess->m_pIConvertType->CanConvert(wSavedFromType, wSavedToType, dwSavedConvFlags);
					TRACE_METHOD(hr, L"IConvertType::CanConvert(%d=0x%08x, %d=0x%08x, 0x%08x)", wSavedFromType, wSavedFromType, wSavedToType, wSavedToType, dwSavedConvFlags);
					
					//Now display the results
					//We always want to display the result even on error...
					wSendMessageFmt(hWndResult, WM_SETTEXT, 0, L" %S", GetErrorName(hr));
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
// CMDIChild::OpenRowsetProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::OpenRowsetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid		s_CComboInterface;
	static CRichEditLite		s_editTableID;
	static CRichEditLite		s_editIndexID;

	static BOOL fUseProps		= TRUE;			//Default
	static BOOL fAggregation	= FALSE;		//Default
	static BOOL fOutput			= TRUE;			//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Window Title (Default is IOpenRowset::OpenRowset)
			if(pThis->m_idSource == IDM_ISCOPEDOPERATIONS_OPENROWSET)
				::SetWindowText(hWnd, "IScopedOperations::OpenRowset");
			
			//Fill In TableID as Default
			s_editTableID.CreateIndirect(hWnd, IDE_TABLEID);
			WCHAR* pwszTableName = pThis->m_pCQueryBox->GetSelectedText();
			s_editTableID.SetWindowText(pwszTableName);

			//Fill in IndexID as default
			s_editIndexID.CreateIndirect(hWnd, IDE_INDEXID);

			//Use Properties
			::CheckDlgButton(hWnd, IDB_USEPROPERTIES,	BST2STATE(fUseProps));

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IRowset);
			
			SAFE_FREE(pwszTableName);
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
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pThis->m_pCSource, CDataSource);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &pThis->m_CDefPropSets);
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
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CSession* pCSession	= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					CRow* pCRow			= SOURCE_GETOBJECT(pThis->m_pCSource, CRow);

					IUnknown* pIUnknown = NULL;
					HRESULT hr = S_OK;

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
					
					//TableID
					CComWSTR cstrTableName;
					cstrTableName.Attach(s_editTableID.GetWindowText());
					
					DBID TableID;
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = cstrTableName;
					DBID* pTableID = cstrTableName ? &TableID : NULL;

					//IndexID
					//NOTE: Most providers don't support IndexIDs and fail if a non-NULL pointer is 
					//passed in, so unless the user specified something just use NULL...
					CComWSTR cstrIndexName;
					cstrIndexName.Attach(s_editIndexID.GetWindowText());
					
					DBID IndexID;
					IndexID.eKind = DBKIND_NAME;
					IndexID.uName.pwszName = cstrIndexName;
					DBID* pIndexID = cstrIndexName ? &IndexID : NULL;

					//Use Properties
					fUseProps = ::IsDlgButtonChecked(hWnd, IDB_USEPROPERTIES);
					if(fUseProps)
					{
						cPropSets = pThis->m_CDefPropSets.GetCount();
						rgPropSets = pThis->m_CDefPropSets.GetPropSets();
					}

					//OpenRowset
					ASSERT(pCSession || pCRow);
					if(pCSession)
						hr = pCSession->OpenRowset(pCAggregate, pTableID, pIndexID, riid, cPropSets, rgPropSets, fOutput ? &pIUnknown : NULL);
					else
						hr = pCRow->OpenRowset(pCAggregate, pTableID, pIndexID, riid, cPropSets, rgPropSets, fOutput ? &pIUnknown : NULL);

					if(SUCCEEDED(hr))
					{
						//Create an object description from the TableName (and index if used)
						//1.  TableName only		- "TableName"
						//2.  TableName and Index	- "TableName.IndexName"
						//3.  IndexName only		- "IndexName"
						CComWSTR cstrDesc;
						cstrDesc.CopyFrom(cstrTableName);
						if(cstrIndexName)
						{	
							if(cstrTableName)
								cstrDesc += L".";
							cstrDesc += cstrIndexName;
						}
						
						//Delegate to display the object
						hr = pThis->HandleRowset(pThis->m_pCSource, pIUnknown, riid, CREATE_NEWWINDOW_IFEXISTS, pCSession ? IID_IOpenRowset : IID_IScopedOperations, cstrDesc);
					}
										
					if(SUCCEEDED(hr))
					{
						//Now just need to place this name in the EditBox
						//Inserted after the current "caret"
						pThis->m_pCQueryBox->ReplaceAll(cstrTableName, FALSE/*bReplaceAll*/, TRUE/*fHighlight*/);
						EndDialog(hWnd, TRUE);
					}

					SAFE_RELEASE(pCAggregate);
					SAFE_RELEASE(pIUnknown);
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
// CMDIChild::RowOpenProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::RowOpenProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static INDEX iSavedType	= 5;  //DBGUID_STREAM
	static CComboBoxGuid		s_CComboInterface;
	static BOOL fAggregation	= FALSE;		//Default
	static BOOL fOutput			= TRUE;			//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis	= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CRow* pCRow			= SOURCE_GETOBJECT(pThis->m_pCSource, CRow);
			
			INDEX iSel = 0;

			HWND hWndColumns	= ::GetDlgItem(hWnd, IDC_COLUMNS);
			HWND hWndObjectType	= ::GetDlgItem(hWnd, IDC_OBJECTTYPE);
			
			ULONG i;
			//Fill In ColumnID drop down
			INDEX iDefColumn = -1;
			for(i=0; i<pCRow->m_ColumnInfo.GetCount(); i++)
			{
				iSel = (INDEX)wSendMessage(hWndColumns, CB_ADDSTRING, 0, GetColName(&pCRow->m_ColumnInfo[i]));
				SendMessage(hWndColumns, CB_SETITEMDATA, iSel, (LPARAM)&pCRow->m_ColumnInfo[i].columnid);
				
				//Default the selected column to the first BLOB, Stream, or IUnknown object
				if(iDefColumn==-1 && ((pCRow->m_ColumnInfo[i].wType == DBTYPE_IUNKNOWN) || (pCRow->m_ColumnInfo[i].dwFlags & (DBCOLUMNFLAGS_ISLONG | DBCOLUMNFLAGS_ISDEFAULTSTREAM))))
					iDefColumn = i;
			}
			
			//Now also add the special column guids, which may not be a part of the ColumnInfo
			for(i=0; i<g_cRowColMaps; i++)
			{
				iSel = (INDEX)wSendMessage(hWndColumns, CB_ADDSTRING, 0, g_rgRowColMaps[i].pwszName);
				SendMessage(hWndColumns, CB_SETITEMDATA, iSel, (LPARAM)g_rgRowColMaps[i].pDBID);
			}

			//Set the Default Selection
			if(iDefColumn == -1)
				iDefColumn = iSel;
			SendMessage(hWndColumns, CB_SETCURSEL, iDefColumn, 0);

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
				s_CComboInterface.SetGuid(IID_ISequentialStream);
			
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
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);
					CRow* pCRow			= SOURCE_GETOBJECT(pThis->m_pCSource, CRow);
					IUnknown* pIUnknown = NULL;
					DWORD dwCreateOpts = CREATE_NEWWINDOW;

					HWND hWndColumns	= ::GetDlgItem(hWnd, IDC_COLUMNS);
					HWND hWndObjectType	= ::GetDlgItem(hWnd, IDC_OBJECTTYPE);
					HRESULT hr = S_OK;

					//ColumnID
					INDEX iSel = (INDEX)SendMessage(hWndColumns, CB_GETCURSEL, 0, 0);
					DBID* pColID = (DBID*)SendMessage(hWndColumns, CB_GETITEMDATA, iSel, 0);

					//ObjectType
					iSavedType = (INDEX)SendMessage(hWndObjectType, CB_GETCURSEL, 0, 0);
					WIDEGUIDMAP* pObjectMap = (WIDEGUIDMAP*)SendMessage(hWndObjectType, CB_GETITEMDATA, iSavedType, 0);
					GUID guidObjectType = *pObjectMap->pGuid;
					if(guidObjectType == GUID_NULL)
						dwCreateOpts |= CREATE_DETERMINE_TYPE;

					//Obtain the Aggregation argument
					CAggregate* pCAggregate = NULL;
					fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
					if(fAggregation)
						pCAggregate = new CAggregate();
					
					//Obtain the Output (ppIUnknown) argument
					fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Interface
					REFIID riid = s_CComboInterface.GetGuid();

					//IRow::Open
					TESTC(hr = pCRow->Open(pCAggregate, pColID, guidObjectType, riid, fOutput ? &pIUnknown : NULL));

					//Handle the returned object type...
					//NOTE: We know the object returned, or eCUnknown is passed to determine it
					if(!pThis->m_pCMainWindow->HandleObjectType(pCRow, pIUnknown, riid, GuidToSourceType(guidObjectType), 0, NULL, dwCreateOpts))
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
// CMDIChild::GetObject
//
/////////////////////////////////////////////////////////////////
CBase* CMDIChild::GetObject(SOURCE eSource, BOOL fAlways)
{
	//First try to obtain the last object...
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
// CMDIChild::GetObjectAddress
//
/////////////////////////////////////////////////////////////////
CBase** CMDIChild::GetObjectAddress(SOURCE eSource)
{
	CBase** ppCObject = NULL;
	
	switch(eSource)
	{
		case eCDataSource:
			ppCObject = (CBase**)&m_pCDataSource;
			break;

		case eCSession:
			ppCObject = (CBase**)&m_pCSession;
			break;
	
		case eCCommand:
			ppCObject = (CBase**)&m_pCCommand;
			break;
	
		case eCMultipleResults:
			ppCObject = (CBase**)&m_pCMultipleResults;
			break;

		case eCDataset:
		case eCRowset:
		case eCRow:
			//Only one of these objects is active at any one time
			//Since all of them actually use the "DataGrid" we can only have one per MDIChild
			ppCObject = (CBase**)&m_pCDataAccess;
			break;

		case eCUnknown:
			//Determine "Last" object type...
			if(m_pCDataAccess && m_pCDataAccess->m_pIUnknown)
				return (CBase**)&m_pCDataAccess;
			else if(m_pCMultipleResults && m_pCMultipleResults->m_pIUnknown)
				return (CBase**)&m_pCMultipleResults;
			else if(m_pCCommand && m_pCCommand->m_pIUnknown)
				return (CBase**)&m_pCCommand;
			else if(m_pCSession && m_pCSession->m_pIUnknown)
				return (CBase**)&m_pCSession;
			else if(m_pCDataSource && m_pCDataSource->m_pIUnknown)
				return (CBase**)&m_pCDataSource;
			break;
		
		default:
			break;
	};

	return ppCObject;
}


////////////////////////////////////////////////////////////////
// CMDIChild::HandleRowset
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::HandleRowset(CBase* pCSource, IUnknown* pIUnknown, REFIID riid, DWORD dwFlags, REFGUID guidSource, WCHAR* pwszTableName, BOOL fSchemaRowset)
{
	HRESULT hr = S_OK;

	//Handle the Result 
	//NOTE: Result can be NULL for non-row returning results...
	if(pIUnknown)
	{
		//Don't display the object
		dwFlags |= CREATE_NODISPLAY;

		//Now Create the object...
		//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
		CBase* pCObject = m_pCMainWindow->HandleObjectType(pCSource, pIUnknown, riid, eCRowset, 0, NULL, dwFlags | CREATE_DETERMINE_TYPE);
		if(pCObject)
		{
			//Record the source of the object...
			pCObject->m_guidSource = guidSource;
			CDataAccess* pCDataAccess = SOURCE_GETOBJECT(pCObject, CDataAccess);
			if(pCDataAccess)
			{
				pCDataAccess->m_bSchemaRowset	= fSchemaRowset;

				//Save the passed in TableName...
				pCDataAccess->SetObjectDesc(pwszTableName);
			}

			//Now Display the object...
			pCObject->DisplayObject();
		}
		else
		{
			TESTC(hr = E_FAIL);
		}
	}

CLEANUP:
	UpdateControls();
	return hr;
}


	
////////////////////////////////////////////////////////////////
// CMDIChild::AdminCreateDataSourceProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::AdminCreateDataSourceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL fUseProps = TRUE;	//Default to TRUE

	static CPropSets			s_CAdminPropSets;
	static CComboBoxGuid		s_CComboInterface;
	static BOOL fAggregation	= FALSE;		//Default
	static BOOL fOutput			= TRUE;			//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Use Properties
			::CheckDlgButton(hWnd, IDB_USEPROPERTIES,	BST2STATE(fUseProps));
			//Reset our static properties...
			s_CAdminPropSets.RemoveAll();

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IOpenRowset);

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
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CDataSource* pCDataSource	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataSource);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_DBINITALL, IID_IDBProperties, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &s_CAdminPropSets);
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
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CDataSource* pCDataSource	= SOURCE_GETOBJECT(pThis->m_pCSource, CDataSource);

					HRESULT hr = S_OK;

					ULONG cPropSets = 0;
					DBPROPSET* rgPropSets = NULL;
					IUnknown* pIUnknown = NULL;

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
						cPropSets = s_CAdminPropSets.GetCount();
						rgPropSets = s_CAdminPropSets.GetPropSets();
					}

					//IDBDataSourceAdmin::CreateDataSource
					//NOTE: Our helper takes care of aggregation...
					hr = pCDataSource->AdminCreateDataSource(pCAggregate, cPropSets, rgPropSets, riid, fOutput ? &pIUnknown : NULL);

					//Display the Result
					if(SUCCEEDED(hr) && fOutput)
					{
						//Just pass in a guess for the object type and let our helper figure out what object it is...
						if(!pThis->m_pCMainWindow->HandleObjectType(pCDataSource, pIUnknown, riid, eCSession, 0, NULL, CREATE_NEWWINDOW_IFEXISTS))
							hr = E_FAIL;
					}

					//Release
					SAFE_RELEASE(pIUnknown);
					SAFE_RELEASE(pCAggregate);
					pThis->UpdateControls();

					if(SUCCEEDED(hr))
					{	
						s_CAdminPropSets.RemoveAll();
						EndDialog(hWnd, TRUE);
						return 0;
					}
					return 0;
				}

				case IDCANCEL:
				{
					s_CAdminPropSets.RemoveAll();
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
// CMDIChild::CreateIndexProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::CreateIndexProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//NOTE:  These are static variables, just to hold the PropertyInfo
	//Information temporarily.  There are messages that need this info which 
	//gets setup on INITDIALOG.  On OK or CANCEL this info will be freed...
	static WCHAR		*pStringBuffer	= NULL;
	ULONG				i;
	LONG				iSel = 0;
	static WCHAR		wszBuffer[MAX_NAME_LEN+1];

	switch(message)
	{
		case WM_INITDIALOG:	
		{	
			//Save the "this" pointer
			CWaitCursor waitCursor;
			CMDIChild		*pThis			= (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Use Extended ListView Styles!
			HWND		hWndIndexColumnDesc	= ::GetDlgItem(hWnd, IDL_INDEXCOLUMNDESC);
			HWND		hWndTableID			= ::GetDlgItem(hWnd, IDE_TABLEID);
			HWND		hWndOrder			= ::GetDlgItem(hWnd, IDC_ORDER);
			HWND		hWndColumnID		= ::GetDlgItem(hWnd, IDE_COLUMNID);
			RECT		rect;

			SendMessage(hWndIndexColumnDesc, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

			//Fill In TableID as Default
			WCHAR* pwszTableName = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndTableID, WM_SETTEXT, 0, pwszTableName);

			//ColumnHeaders for INDEXCOLUMNDESC report
			LV_InsertColumn(hWndIndexColumnDesc,   COLUMN_ID,	L"Column ID", IMAGE_NONE);
			LV_InsertColumn(hWndIndexColumnDesc,   COLUMN_ORDER,L"Order", IMAGE_NONE);

			//AutoSize Columns
			GetWindowRect(hWndColumnID, (LPRECT)&rect);
			SendMessage(hWndIndexColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_ID, (LPARAM)(rect.right-rect.left));
			GetWindowRect(hWndOrder, (LPRECT)&rect);
			SendMessage(hWndIndexColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_ORDER, (LPARAM)(rect.right-rect.left));
			
			// Order list...
			SendMessage(hWndOrder, CB_ADDSTRING, 0, (LPARAM)"Ascendent");
			SendMessage(hWndOrder, CB_ADDSTRING, 0, (LPARAM)"Descendent");
			SendMessage(hWndOrder, CB_SETCURSEL, 0, 0);	// ascendent
		
			//Placement of Dialog, want it just below the "Init" button...
			CenterDialog(hWnd);

			//Disable fields
			::EnableWindow(::GetDlgItem(hWnd, IDE_COLUMNID), FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDC_ORDER), FALSE);
			
			SAFE_FREE(pwszTableName)
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			switch (GET_WM_COMMAND_CMD(wParam, lParam))
			{
				case EN_KILLFOCUS:
				{
					switch((int)LOWORD(wParam))
					{
						case IDE_COLUMNID:
						{
							HWND		hWndIndexColumnDesc	= ::GetDlgItem(hWnd, IDL_INDEXCOLUMNDESC);
							HWND		hWndColumnID		= ::GetDlgItem(hWnd, IDE_COLUMNID);
							HWND		hWndColName			= ::GetDlgItem(hWnd, IDE_COLNAME);
							WCHAR		wszBuffer2[MAX_NAME_LEN+1];
							
							// get the selected item
							iSel = ListView_GetNextItem(hWndIndexColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							// and to Column Name static field
							wSendMessage(hWndColumnID, WM_GETTEXT, MAX_NAME_LEN, wszBuffer2);
							StringFormat(wszBuffer, NUMELE(wszBuffer), L"Current item %d (%s)", iSel, wszBuffer2);
							LV_SetItemText(hWndIndexColumnDesc, iSel, COLUMN_ID, wszBuffer2);
							wSendMessage(hWndColName, WM_SETTEXT, 0, wszBuffer);
							return 0;
						}
					}
					return 0;
				}
			}
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			//CBN_SELCHANGE
			switch(GET_WM_COMMAND_CMD(wParam, lParam))
			{
				case CBN_SELCHANGE:
				{
					switch(GET_WM_COMMAND_ID(wParam, lParam))
					{
						case IDC_ORDER:
						{
							HWND		hWndIndexColumnDesc	= ::GetDlgItem(hWnd, IDL_INDEXCOLUMNDESC);
							HWND		hWndOrder			= ::GetDlgItem(hWnd, IDC_ORDER);
							INDEX		iSelOrder			= (INDEX)SendMessage(hWndOrder, CB_GETCURSEL, 0, 0);

							// get the selected item
							iSel = ListView_GetNextItem(hWndIndexColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							wSendMessage(hWndOrder, CB_GETLBTEXT, (WPARAM)iSelOrder, wszBuffer);
							LV_SetItemText(hWndIndexColumnDesc, iSel, COLUMN_ORDER, wszBuffer);
							return 0;
						}
					}
					return 0;
				}
			}

			//Regular command messages
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_INDEXPROPERTIES:
				{
					//Get the "this" pointer
					CMDIChild	*pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pThis->m_pCSource, CDataSource);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_INDEXALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &pThis->m_CDefPropSets);
					return 0;
				}

				case IDB_ADD:
				{
					HWND		hWndIndexColumnDesc	= ::GetDlgItem(hWnd, IDL_INDEXCOLUMNDESC);
					ULONG		cItems				= ListView_GetItemCount(hWndIndexColumnDesc);

					// make the insertion
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"Column%lu", cItems);
					LV_InsertItem(hWndIndexColumnDesc, cItems, COLUMN_ID, wszBuffer);

					// order
					LV_InsertItem(hWndIndexColumnDesc, cItems, COLUMN_ORDER, L"Ascendent");
					LV_SetItemState(hWndIndexColumnDesc, cItems, COLUMN_ID, LVIS_SELECTED, LVIS_SELECTED);						
					return 0;
				}

				case IDB_DELETE:
				{
					HWND		hWndIndexColumnDesc	= ::GetDlgItem(hWnd, IDL_INDEXCOLUMNDESC);
					INT			cItems				= ListView_GetItemCount(hWndIndexColumnDesc);

					// get the selected item
					iSel = ListView_GetNextItem(hWndIndexColumnDesc, -1, LVNI_SELECTED);
					
					if (iSel > -1)
					{
						ListView_DeleteItem(hWndIndexColumnDesc, iSel);
						SendMessage(::GetDlgItem(hWnd, IDE_COLNAME), WM_SETTEXT, 0, (LPARAM)"");
						LV_SetItemState(hWndIndexColumnDesc, iSel >= cItems-1 ? iSel-1: iSel, 
							COLUMN_DBCID, LVIS_SELECTED, LVIS_SELECTED);						
					}
					//Disable fields
					::EnableWindow(::GetDlgItem(hWnd, IDE_COLUMNID), FALSE);
					::EnableWindow(::GetDlgItem(hWnd, IDC_ORDER), FALSE);
					return 0;
				}

				case IDCANCEL:
				{
					//Cleanup any memory allocated
					SAFE_FREE(pStringBuffer);
					EndDialog(hWnd, FALSE);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild			*pThis				= (CMDIChild*)GetThis(hWnd);
					CSession			*pCSession			= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					
					HWND				hWndIndexColumnDesc	= ::GetDlgItem(hWnd, IDL_INDEXCOLUMNDESC);
					HWND				hWndTableID			= ::GetDlgItem(hWnd, IDE_TABLEID);
					HWND				hWndIndexID			= ::GetDlgItem(hWnd, IDE_INDEXID);
					DBINDEXCOLUMNDESC	*rgIndexColumnDesc	= NULL;
					ULONG				cIndexColumnDesc	= 0;
					DBID				TableID;
					WCHAR				wszTableDBID[MAX_QUERY_LEN+1];
					DBID				IndexID;
					WCHAR				wszIndexDBID[MAX_QUERY_LEN+1];
					DBPROPSET			*rgPropSet			= NULL;
					ULONG				cPropSet			= 0;
					HRESULT				hr;

					// alloc space for the column desc array
					cIndexColumnDesc = ListView_GetItemCount(hWndIndexColumnDesc);
					SAFE_ALLOC(rgIndexColumnDesc, DBINDEXCOLUMNDESC, cIndexColumnDesc);

					// get the column desc array
					for (i=0; i<cIndexColumnDesc; i++)
					{
						// initialize the whole structure
						memset(&rgIndexColumnDesc[i], 0, sizeof(DBINDEXCOLUMNDESC));

						// get column name
						LV_GetItemText(hWndIndexColumnDesc, i, COLUMN_ID, wszBuffer, MAX_NAME_LEN);
						SAFE_ALLOC(rgIndexColumnDesc[i].pColumnID, DBID, 1);
						rgIndexColumnDesc[i].pColumnID->eKind			= DBKIND_NAME;
						rgIndexColumnDesc[i].pColumnID->uName.pwszName	= wcsDuplicate(wszBuffer);

						// get column order
						LV_GetItemText(hWndIndexColumnDesc, i, COLUMN_ORDER, wszBuffer, MAX_NAME_LEN);
						if (StringCompare(wszBuffer, L"Descendent"))
							rgIndexColumnDesc[i].eIndexColOrder = DBINDEX_COL_ORDER_DESC;
						else
							rgIndexColumnDesc[i].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
					}

					// get TableID
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = wGetWindowText(hWndTableID);
					DBIDToString(&TableID, wszTableDBID, MAX_QUERY_LEN);

					// get IndexID
					IndexID.eKind			= DBKIND_NAME;
					IndexID.uName.pwszName	= wGetWindowText(hWndIndexID);
					DBIDToString(&IndexID, wszIndexDBID, MAX_QUERY_LEN);

					// check whether rowset/table properties are used
					if(::IsDlgButtonChecked(hWnd, IDB_USEPROPERTIES))
					{
						cPropSet	= pThis->m_CDefPropSets.GetCount();
						rgPropSet	= pThis->m_CDefPropSets.GetPropSets();
					}

					XTEST(hr = pCSession->m_pIIndexDefinition->CreateIndex(&TableID, &IndexID, cIndexColumnDesc, rgIndexColumnDesc, cPropSet, rgPropSet, NULL));
					TESTC(TRACE_METHOD(hr, L"IIndexDefinition::CreateIndex(%s, %s, %lu, 0x%p, %d, 0x%p, NULL)", wszTableDBID, wszIndexDBID, cIndexColumnDesc, rgIndexColumnDesc, cPropSet, rgPropSet));

				CLEANUP:
					for (i=0; i<cIndexColumnDesc; i++)
					{
						if (DBKIND_NAME == rgIndexColumnDesc[i].pColumnID->eKind)
							SAFE_FREE(rgIndexColumnDesc[i].pColumnID->uName.pwszName);
					}
					
					if(SUCCEEDED(hr))
					{
						//Now just need to place this name in the EditBox
						//Inserted after the current "caret"
						pThis->m_pCQueryBox->ReplaceAll(TableID.uName.pwszName, FALSE/*bReplaceAll*/, TRUE/*fHighlight*/);
						EndDialog(hWnd, TRUE);
					}
	
					SAFE_FREE(rgIndexColumnDesc);
					SAFE_FREE(TableID.uName.pwszName);
					SAFE_FREE(IndexID.uName.pwszName);
					return 0;
				}
			}
			break;
		}//WM_COMMAND

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			
			//ListView
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
				{
					return FALSE;
				}

				case LVN_ITEMCHANGED:
				{
					HWND		hWndIndexColumnDesc	= ::GetDlgItem(hWnd, IDL_INDEXCOLUMNDESC);
					HWND		hWndColumnID		= ::GetDlgItem(hWnd, IDE_COLUMNID);
					HWND		hWndOrder			= ::GetDlgItem(hWnd, IDC_ORDER);
					HWND		hWndColName			= ::GetDlgItem(hWnd, IDE_COLNAME);
					WCHAR		wszBuffer2[MAX_NAME_LEN+1];
					NMLISTVIEW	*pLV = (NMLISTVIEW*)lParam;
					INDEX		iOrder = 0;

					if (pLV->uChanged != LVIF_STATE)
						return 0;

					// get the selected item
					iSel = pLV->iItem;
					if (!(pLV->uNewState & LVIS_SELECTED))
					{
						if (!(pLV->uOldState & LVIS_SELECTED))
							return 0;
						// this is the previously selected item
						// retrieve the column name and copy it to the IDE_COLUMNID field
						wSendMessage(hWndColumnID, WM_GETTEXT, MAX_NAME_LEN, wszBuffer2);
						LV_SetItemText(hWndIndexColumnDesc, iSel, COLUMN_ID, wszBuffer2);
						StringFormat(wszBuffer, NUMELE(wszBuffer), L"Current item %lu (%s)", iSel, wszBuffer2);
						wSendMessage(hWndColName, WM_SETTEXT, 0, wszBuffer);

						// retrieve the column type and copy it to the IDC_ORDER field
						iOrder = (INDEX)SendMessage(hWndOrder, CB_GETCURSEL, 0, 0);
						wSendMessage(hWndOrder, CB_GETLBTEXT, iOrder, wszBuffer);
						LV_SetItemText(hWndIndexColumnDesc, iSel, COLUMN_ORDER, wszBuffer);
					}
					else
					{
						// retrieve the column name and copy it to the IDE_COLUMNID field
						LV_GetItemText(hWndIndexColumnDesc, iSel, COLUMN_ID, wszBuffer2, MAX_NAME_LEN);
						wSendMessage(hWndColumnID, WM_SETTEXT, 0, wszBuffer2);
						StringFormat(wszBuffer, NUMELE(wszBuffer), L"Current item %lu (%s)", iSel, wszBuffer2);
						wSendMessage(hWndColName, WM_SETTEXT, 0, wszBuffer);

						// retrieve the column type and copy it to the IDC_ORDER field
						LV_GetItemText(hWndIndexColumnDesc, iSel, COLUMN_ORDER, wszBuffer, MAX_NAME_LEN);
						SendMessage(hWndOrder, CB_SELECTSTRING, -1, (LPARAM)wszBuffer);
					}
					

					//Disable fields
					::EnableWindow(::GetDlgItem(hWnd, IDE_COLUMNID), TRUE);
					::EnableWindow(::GetDlgItem(hWnd, IDC_ORDER), TRUE);
					return FALSE;
				}	

				case LVN_ITEMCHANGING:
				{
						return FALSE; //Allow the change
				}
			}
		}//WM_NOTIFY
	}//switch message
		
	return FALSE;
} //CMDIChild::CreateIndexProc


////////////////////////////////////////////////////////////////
// CMDIChild::DropIndexProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::DropIndexProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CRichEditLite		s_editTableID;
	static CRichEditLite		s_editIndexID;
	
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Fill In TableID as Default
			s_editTableID.CreateIndirect(hWnd, IDE_TABLEID);
			WCHAR* pwszIndexName = pThis->m_pCQueryBox->GetSelectedText();
			s_editTableID.SetWindowText(pwszIndexName);

			//Fill In IndexID
			s_editIndexID.CreateIndirect(hWnd, IDE_INDEXID);

			CenterDialog(hWnd);
			SAFE_FREE(pwszIndexName);
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
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CSession* pCSession = SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					WCHAR wszTableDBID[MAX_NAME_LEN+1];
					WCHAR wszIndexDBID[MAX_NAME_LEN+1];
					HRESULT hr = S_OK;

					//TableID
					DBID TableID;
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = s_editTableID.GetWindowText();
					DBID* pTableID = TableID.uName.pwszName ? &TableID : NULL;
					DBIDToString(pTableID, wszTableDBID, MAX_NAME_LEN);

					//IndexID
					DBID IndexID;
					IndexID.eKind = DBKIND_NAME;
					IndexID.uName.pwszName = s_editIndexID.GetWindowText();
					DBID* pIndexID = IndexID.uName.pwszName ? &IndexID : NULL;
					DBIDToString(pIndexID, wszIndexDBID, MAX_NAME_LEN);

					//IIndexDefinition::DropIndex
					XTEST(hr = pCSession->m_pIIndexDefinition->DropIndex(pTableID, pIndexID));
					TRACE_METHOD(hr, L"IIndexDefinition::DropIndex(%s, %s)", wszTableDBID, wszIndexDBID);
					
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					
					SAFE_FREE(TableID.uName.pwszName);
					SAFE_FREE(IndexID.uName.pwszName);
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
// CMDIChild::AddColumnProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::AddColumnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid	s_CComboInterface;
	static BOOL				fUseProps	= TRUE;	//Default to TRUE
	static WCHAR			wszBuffer[MAX_NAME_LEN+1];
	ULONG	i;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CSession* pCSession = SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
			INDEX iSel = 0;

			HWND hWndTableID	= ::GetDlgItem(hWnd, IDE_TABLEID);
			HWND hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);
			HWND hWndDataType	= ::GetDlgItem(hWnd, IDC_TYPE);
			HWND hWndSize		= ::GetDlgItem(hWnd, IDE_SIZE);
			HWND hWndPrecision	= ::GetDlgItem(hWnd, IDE_PRECISION);
			HWND hWndScale		= ::GetDlgItem(hWnd, IDE_SCALE);

			//Make sure we have the Provider Types rowset ahead of time
			pCSession->GetProviderTypes();
			
			//Fill In TableID as Default
			WCHAR* pwszTableName = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndTableID, WM_SETTEXT, 0, pwszTableName);

			//Fill In column size as Default
			wSendMessageFmt(hWndSize, WM_SETTEXT, 0, L"%d", 0);

			//Fill In column precision as Default
			wSendMessageFmt(hWndPrecision, WM_SETTEXT, 0, L"%d", 0);

			//Fill In column scale as Default
			wSendMessageFmt(hWndScale, WM_SETTEXT, 0, L"%d", 0);

			//Use Properties
			::CheckDlgButton(hWnd, IDB_USEPROPERTIES,	BST2STATE(fUseProps));

			// Type Name list...
			for(i=0; i<pCSession->m_cProvTypes; i++)
			{
				//Type Name
				iSel = (INDEX)wSendMessage(hWndTypeName, CB_ADDSTRING,	0, pCSession->m_rgProvTypes[i].wszTypeName);
			}
			SendMessage(hWndTypeName, CB_SETCURSEL, 0, 0);

			// DBType list
			for(i=0; i<g_cDBTypes; i++)
			{
				//DBTypes
				iSel = (INDEX)wSendMessage(hWndDataType, CB_ADDSTRING,	0, g_rgDBTypes[i].pwszName);
			}
			SendMessage(hWndDataType, CB_SETCURSEL, 0, 0);

			for (i=0; i<g_cDBTypes; i++)
			{
				if (pCSession->m_rgProvTypes[0].wType == g_rgDBTypes[i].lItem)
				{
					SendMessage(hWndDataType, CB_SETCURSEL, i, 0);
					break;
				}
			}
			CenterDialog(hWnd);
			SAFE_FREE(pwszTableName);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			//CBN_SELCHANGE
			switch(GET_WM_COMMAND_CMD(wParam, lParam))
			{
				//Selection change
				case CBN_SELCHANGE:
				{	
					//See which combo box has changed
					switch(GET_WM_COMMAND_ID(wParam, lParam))
					{
						case IDC_TYPE:
						{
							CMDIChild		*pThis			= (CMDIChild*)GetThis(hWnd);
							CSession		*pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
							HWND			hWndSize		= ::GetDlgItem(hWnd, IDE_SIZE);
							HWND			hWndPrecision	= ::GetDlgItem(hWnd, IDE_PRECISION);
							HWND			hWndScale		= ::GetDlgItem(hWnd, IDE_SCALE);
							INDEX			iSelTypeName;
							HWND			hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);
							//Get the Selected Type
							CComboBoxLite	comboTypes(hWnd, IDC_TYPE);
							comboTypes.GetSelText(wszBuffer, MAX_NAME_LEN);
							DBTYPE wType = GetDBType(wszBuffer);
													 
							//Get Default Size,Prec,Scale for this type...
							DBLENGTH ulMaxSize = 0;
							BYTE  bPrecision, bScale;
							GetDBTypeMaxSize(wType, &ulMaxSize, &bPrecision, &bScale);

							//Set Size
							wSendMessageFmt(hWndSize,		WM_SETTEXT, 0, L"%Iu", ulMaxSize!=0 ? ulMaxSize : 255);
							//Set Precision
							wSendMessageFmt(hWndPrecision,	WM_SETTEXT, 0, L"%d", bPrecision);
							//Set Scale
							wSendMessageFmt(hWndScale,		WM_SETTEXT, 0, L"%d", bScale);

							
							// determine a type on the DBTYPE
							for (i = 0; i < pCSession->m_cProvTypes; i++)
							{
								if (wType == pCSession->m_rgProvTypes[i].wType)
									iSelTypeName = i;
							}
							SendMessage(hWndTypeName, CB_SETCURSEL, iSelTypeName, 0);

							return 0;
						}
						break;


						case IDC_TYPENAME:
						{
							CMDIChild*	pThis			= (CMDIChild*)GetThis(hWnd);
							CSession*	pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
							HWND		hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);						
							HWND		hWndDataType	= ::GetDlgItem(hWnd, IDC_TYPE);						
							HWND		hWndColSize		= ::GetDlgItem(hWnd, IDE_SIZE);						
							HWND		hWndColPrec		= ::GetDlgItem(hWnd, IDE_PRECISION);						
							HWND		hWndColScale	= ::GetDlgItem(hWnd, IDE_SCALE);						
							INDEX		iSelTypeName	= (INDEX)SendMessage(hWndTypeName, CB_GETCURSEL, 0, 0);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							wSendMessage(hWndTypeName, CB_GETLBTEXT, (WPARAM)iSelTypeName, wszBuffer);
							if (iSelTypeName >= 0)
							{
								wSendMessage(hWndDataType, CB_SELECTSTRING, -1, GetDBTypeName(pCSession->m_rgProvTypes[iSelTypeName].wType));
								StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pCSession->m_rgProvTypes[iSelTypeName].ulColumnSize);
								wSendMessage(hWndColSize, WM_SETTEXT, 0, wszBuffer); 
								if (IsNumericType(pCSession->m_rgProvTypes[iSelTypeName].wType))
								{
									StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pCSession->m_rgProvTypes[iSelTypeName].ulColumnSize);
									wSendMessage(hWndColPrec, WM_SETTEXT, 0, wszBuffer); 
								}
								else
								{
									SendMessage(hWndColPrec, WM_SETTEXT, 0, (LPARAM)"0"); 
								}
							}														
						}
						break;


					}
					break;
				}
			
				return FALSE;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_SETPROPERTIES:
				{
					//Get the "this" pointer
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pThis->m_pCSource, CDataSource);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_COLUMNALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &pThis->m_CDefPropSets);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild*		pThis			= (CMDIChild*)GetThis(hWnd);
					CSession*		pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);

					HWND			hWndTableID		= ::GetDlgItem(hWnd, IDE_TABLEID);
					HWND			hWndColumnID	= ::GetDlgItem(hWnd, IDE_COLUMNID);
					HWND			hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);
					HWND			hWndSize		= ::GetDlgItem(hWnd, IDE_SIZE);
					HWND			hWndPrecision	= ::GetDlgItem(hWnd, IDE_PRECISION);
					HWND			hWndScale		= ::GetDlgItem(hWnd, IDE_SCALE);
					WCHAR			wszTableDBID[MAX_QUERY_LEN+1];
					WCHAR			wszBuffer[MAX_NAME_LEN+1];
					HRESULT			hr				= S_OK;
					DBCOLUMNDESC	ColumnDesc;
					LONG			lValue;
					
					// initialize column desc stru 
					memset(&ColumnDesc, 0, sizeof(DBCOLUMNDESC));

					// TableID
					DBID TableID;
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = wGetWindowText(hWndTableID);
					DBIDToString(&TableID, wszTableDBID, MAX_QUERY_LEN);

					// ColumnID of ColumnDesc
					ColumnDesc.dbcid.eKind			= DBKIND_NAME;
					ColumnDesc.dbcid.uName.pwszName	= wGetWindowText(hWndColumnID);

					// Column Size
					GetEditBoxValue(hWndSize, &lValue, 0, LONG_MAX, TRUE);
					ColumnDesc.ulColumnSize = lValue;
					
					// Column Precision
					GetEditBoxValue(hWndPrecision, &lValue, 0, 255, TRUE);
					ColumnDesc.bPrecision = (BYTE)lValue;
					
					// Column Scale
					GetEditBoxValue(hWndScale, &lValue, 0, 255, TRUE);
					ColumnDesc.bScale = (BYTE)lValue;
					
					//Use Properties
					fUseProps = ::IsDlgButtonChecked(hWnd, IDB_USEPROPERTIES);
					if(fUseProps)
					{
						ColumnDesc.cPropertySets = pThis->m_CDefPropSets.GetCount();
						ColumnDesc.rgPropertySets = pThis->m_CDefPropSets.GetPropSets();
					}

					// Type Name 
					ColumnDesc.pwszTypeName = wGetWindowText(hWndTypeName);

					// DBType
					CComboBoxLite comboTypes(hWnd, IDC_TYPE);
					comboTypes.GetSelText(wszBuffer, MAX_NAME_LEN);
					ColumnDesc.wType = GetDBType(wszBuffer);

					// ITableDefinition::AddColumn
					XTEST(hr = pCSession->m_pITableDefinition->AddColumn(&TableID, &ColumnDesc, NULL));
					TESTC(TRACE_METHOD(hr, L"ITableDefinition::AddColumn(%s, 0x%p, NULL)", wszTableDBID, &ColumnDesc));
					if(SUCCEEDED(hr))
					{
						//Now just need to place this name in the EditBox
						//Inserted after the current "caret"
						pThis->m_pCQueryBox->ReplaceAll(TableID.uName.pwszName, FALSE/*bReplaceAll*/, TRUE/*fHighlight*/);
						EndDialog(hWnd, TRUE);
					}
		
					SAFE_FREE(TableID.uName.pwszName);
					SAFE_FREE(ColumnDesc.dbcid.uName.pwszName);
					SAFE_FREE(ColumnDesc.pwszTypeName);
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

CLEANUP:
	return FALSE;
} // CMDIChild::AddColumnProc


////////////////////////////////////////////////////////////////
// CMDIChild::CreateTableProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::CreateTableProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid	s_CComboInterface;
	static 	WCHAR wszBuffer[MAX_NAME_LEN+1];
	static  BOOL fUseProps			= TRUE;
	static  BOOL fAggregation		= FALSE;			//Default
	static  BOOL fOutput			= TRUE;				//Default

	switch(message)
	{
		case WM_INITDIALOG:	
		{	
			//Save the "this" pointer
			CWaitCursor waitCursor;
			CMDIChild*		pThis			= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CSession*		pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);

			//Use Extended ListView Styles!
			HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
			HWND		hWndTableID		= ::GetDlgItem(hWnd, IDE_TABLEID);
			HWND		hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);
			HWND		hWndDataType	= ::GetDlgItem(hWnd, IDC_DATATYPE);
			HWND		hWndColumnID	= ::GetDlgItem(hWnd, IDE_COLUMNID);
			HWND		hWndColSize		= ::GetDlgItem(hWnd, IDE_SIZE);
			HWND		hWndColPrec		= ::GetDlgItem(hWnd, IDE_PRECISION);
			HWND		hWndColScale	= ::GetDlgItem(hWnd, IDE_SCALE);
			RECT		rect;

			SendMessage(hWndColumnDesc, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

			//ColumnHeaders for COLUMNDESC report
			LV_InsertColumn(hWndColumnDesc,   COLUMN_DBCID,		L"Column DBCID",	IMAGE_NONE);
			LV_InsertColumn(hWndColumnDesc,   COLUMN_TYPENAME,	L"TypeName",		IMAGE_NONE);
			LV_InsertColumn(hWndColumnDesc,   COLUMN_DATATYPE,	L"DataType",		IMAGE_NONE);
			LV_InsertColumn(hWndColumnDesc,   COLUMN_SIZE,		L"Size",			IMAGE_NONE);
			LV_InsertColumn(hWndColumnDesc,   COLUMN_PREC,		L"Precision",		IMAGE_NONE);
			LV_InsertColumn(hWndColumnDesc,   COLUMN_SCALE,		L"Scale",			IMAGE_NONE);

			//AutoSize Columns
			GetWindowRect(hWndColumnID, (LPRECT)&rect);
			SendMessage(hWndColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_DBCID, (LPARAM)(rect.right-rect.left));
			GetWindowRect(hWndTypeName, (LPRECT)&rect);
			SendMessage(hWndColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_TYPENAME, (LPARAM)(rect.right-rect.left));
			GetWindowRect(hWndDataType, (LPRECT)&rect);
			SendMessage(hWndColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_DATATYPE, (LPARAM)(rect.right-rect.left));
			GetWindowRect(hWndColSize, (LPRECT)&rect);
			SendMessage(hWndColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_SIZE, (LPARAM)(rect.right-rect.left));
			GetWindowRect(hWndColPrec, (LPRECT)&rect);
			SendMessage(hWndColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_PREC, (LPARAM)(rect.right-rect.left));
			GetWindowRect(hWndColScale, (LPRECT)&rect);
			SendMessage(hWndColumnDesc, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_SCALE, (LPARAM)(rect.right-rect.left));
			
			//Make sure we have the Provider Types rowset ahead of time
			pCSession->GetProviderTypes();
			
			ULONG i;
			// Type Name list...
			for(i=0; i<pCSession->m_cProvTypes; i++)
			{
				//Type Name
				wSendMessage(hWndTypeName, CB_ADDSTRING, 0, pCSession->m_rgProvTypes[i].wszTypeName);
			}
			SendMessage(hWndTypeName, CB_SETCURSEL, 0, 0);

			// DBType list
			for(i=0; i<g_cDBTypes; i++)
			{
				//DBTypes
				wSendMessage(hWndDataType, CB_ADDSTRING,	0, g_rgDBTypes[i].pwszName);
			}
			for (i=0; i<g_cDBTypes; i++)
			{
				if (pCSession->m_rgProvTypes[0].wType == g_rgDBTypes[i].lItem)
				{
					SendMessage(hWndDataType, CB_SETCURSEL, i, 0);
					break;
				}
			}

			//Use Properties
			::CheckDlgButton(hWnd, IDB_USEPROPERTIES,	BST2STATE(fUseProps));

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IRowset);

			pCSession->m_listCPropSets.RemoveAll();

			//Fill In TableID as Default
			WCHAR* pwszTableName = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndTableID, WM_SETTEXT, 0, pwszTableName);
			
			//Placement of Dialog, want it just below the "Init" button...
			CenterDialog(hWnd);

			//Disable fields
			::EnableWindow(::GetDlgItem(hWnd, IDB_COLPROP),		FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDE_COLUMNID),	FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDC_TYPENAME),	FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDC_DATATYPE),	FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDE_SIZE),		FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDE_PRECISION),	FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDE_SCALE),		FALSE);

			SendMessage(hWndTypeName, CB_SETCURSEL, 0, 0);
			SAFE_FREE(pwszTableName);
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			switch (GET_WM_COMMAND_CMD(wParam, lParam))
			{
				case EN_KILLFOCUS:
				{
					switch((int)LOWORD(wParam))
					{
						case IDE_COLUMNID:
						{
							HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
							HWND		hWndColumnID	= ::GetDlgItem(hWnd, IDE_COLUMNID);
							HWND		hWndColName		= ::GetDlgItem(hWnd, IDE_COLNAME);
							WCHAR		wszBuffer2[MAX_NAME_LEN+1];
							
							// get the selected item
							INDEX iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							// and to Column Name static field
							wSendMessage(hWndColumnID, WM_GETTEXT, MAX_NAME_LEN, wszBuffer2);
							StringFormat(wszBuffer, NUMELE(wszBuffer), L"Current item %Iu (%s)", iSel, wszBuffer2);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_DBCID, wszBuffer2);
							wSendMessage(hWndColName, WM_SETTEXT, 0, wszBuffer);
							break;
						}
						case IDE_SIZE:
						{
							HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
							HWND		hWndColSize		= ::GetDlgItem(hWnd, IDE_SIZE);
							
							// get the selected item
							INDEX iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							// and to Column Name static field
							wSendMessage(hWndColSize, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SIZE, wszBuffer);
							break;
						}
						case IDE_PRECISION:
						{
							HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
							HWND		hWndColPrec		= ::GetDlgItem(hWnd, IDE_PRECISION);
							
							// get the selected item
							INDEX iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							// and to Column Name static field
							wSendMessage(hWndColPrec, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_PREC, wszBuffer);
							break;
						}
						case IDE_SCALE:
						{
							HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
							HWND		hWndColScale	= ::GetDlgItem(hWnd, IDE_SCALE);
							
							// get the selected item
							INDEX iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							// and to Column Name static field
							wSendMessage(hWndColScale, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SCALE, wszBuffer);
							break;
						}
					}
					return 0;
				}
			}
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			//CBN_SELCHANGE 
			switch(GET_WM_COMMAND_CMD(wParam, lParam))
			{
				case CBN_SELCHANGE:
				{
					switch(GET_WM_COMMAND_ID(wParam, lParam))
					{
						case IDC_TYPENAME:
						{
							CMDIChild*	pThis			= (CMDIChild*)GetThis(hWnd);
							CSession*	pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
							HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
							HWND		hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);						
							HWND		hWndDataType	= ::GetDlgItem(hWnd, IDC_DATATYPE);						
							HWND		hWndColSize		= ::GetDlgItem(hWnd, IDE_SIZE);						
							HWND		hWndColPrec		= ::GetDlgItem(hWnd, IDE_PRECISION);						
							HWND		hWndColScale	= ::GetDlgItem(hWnd, IDE_SCALE);						
							INDEX		iSelTypeName	= (INDEX)SendMessage(hWndTypeName, CB_GETCURSEL, 0, 0);

							// get the selected item
							INDEX iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							wSendMessage(hWndTypeName, CB_GETLBTEXT, (WPARAM)iSelTypeName, wszBuffer);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_TYPENAME, wszBuffer);
							if (iSelTypeName >= 0)
							{
								wSendMessage(hWndDataType, CB_SELECTSTRING, -1, GetDBTypeName(pCSession->m_rgProvTypes[iSelTypeName].wType));
								LV_SetItemText(hWndColumnDesc, iSel, COLUMN_DATATYPE, GetDBTypeName(pCSession->m_rgProvTypes[iSelTypeName].wType));
								StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pCSession->m_rgProvTypes[iSelTypeName].ulColumnSize);
								wSendMessage(hWndColSize, WM_SETTEXT, 0, wszBuffer); 
								LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SIZE, wszBuffer);
								if (IsNumericType(pCSession->m_rgProvTypes[iSelTypeName].wType))
								{
									StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pCSession->m_rgProvTypes[iSelTypeName].ulColumnSize);
									wSendMessage(hWndColPrec, WM_SETTEXT, 0, wszBuffer); 
									LV_SetItemText(hWndColumnDesc, iSel, COLUMN_PREC, wszBuffer);
								}
								else
								{
									SendMessage(hWndColPrec, WM_SETTEXT, 0, (LPARAM)"0"); 
									LV_SetItemText(hWndColumnDesc, iSel, COLUMN_PREC, wszBuffer);
								}
							}							
							
							StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pCSession->m_rgProvTypes[iSelTypeName].iMaxScale);
							wSendMessage(hWndColScale, WM_SETTEXT, 0, wszBuffer);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SCALE, wszBuffer);
						}
						break;

						case IDC_DATATYPE:
						{
							CMDIChild*	pThis			= (CMDIChild*)GetThis(hWnd);
							CSession*	pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
							HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
							HWND		hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);						
							HWND		hWndDataType	= ::GetDlgItem(hWnd, IDC_DATATYPE);						
							HWND		hWndColSize		= ::GetDlgItem(hWnd, IDE_SIZE);						
							HWND		hWndColPrec		= ::GetDlgItem(hWnd, IDE_PRECISION);						
							HWND		hWndColScale	= ::GetDlgItem(hWnd, IDE_SCALE);						
							INDEX		iSelDBType		= (INDEX)SendMessage(hWndDataType, CB_GETCURSEL, 0, 0);
							ULONG		i;
							INT			iSelTypeName	= 0;
							DBTYPE		wType;

							// get the selected item
							INDEX iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);

							// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
							wSendMessage(hWndDataType, CB_GETLBTEXT, (WPARAM)iSelDBType, wszBuffer);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_DATATYPE, wszBuffer);
							
							// determine a type on the DBTYPE
							wType = (DBTYPE)g_rgDBTypes[iSelDBType].lItem;
							for (i = 0; i < pCSession->m_cProvTypes; i++)
							{
								if (wType == pCSession->m_rgProvTypes[i].wType)
									iSelTypeName = i;
							}
							
							StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pCSession->m_rgProvTypes[iSelTypeName].ulColumnSize);
							SendMessage(hWndColSize, WM_SETTEXT, 0, (LPARAM)wszBuffer); 
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SIZE, wszBuffer);
							if(IsNumericType(wType))
							{
								StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pCSession->m_rgProvTypes[iSelTypeName].ulColumnSize);
								wSendMessage(hWndColPrec, WM_SETTEXT, 0, wszBuffer); 
								LV_SetItemText(hWndColumnDesc, iSel, COLUMN_PREC, wszBuffer);
							}
							else
							{
								SendMessage(hWndColPrec, WM_SETTEXT, 0, (LPARAM)"0"); 
								LV_SetItemText(hWndColumnDesc, iSel, COLUMN_PREC, wszBuffer);
							}
							
							StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pCSession->m_rgProvTypes[iSelTypeName].iMaxScale);
							wSendMessage(hWndColScale, WM_SETTEXT, 0, wszBuffer);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SCALE, wszBuffer);
							SendMessage(hWndTypeName, CB_SETCURSEL, iSelTypeName, 0);
							LV_SetItemText(hWndColumnDesc, iSel, COLUMN_TYPENAME, pCSession->m_rgProvTypes[iSelTypeName].wszTypeName);
						}
						break;
					}
					return 0;
				}
			}

			//Regular command messages
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_ROWSETPROPERTIES:
				{
					//Get the "this" pointer
					CMDIChild	*pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CDataSource	*pCDataSource	= SOURCE_GETPARENT(pThis->m_pCSource, CDataSource);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &pThis->m_CDefPropSets);
					return 0;
				}

				case IDB_COLPROP:
				{
					//Get the "this" pointer
					CMDIChild	*pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);

					CSession*	pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pCSession, CDataSource);

					// get the selected item
					ULONG iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);
					if ((LONG)iSel <0)
						return 0;
					
					POSITION pos	= pCSession->m_listCPropSets.FindIndex(iSel);
					CPropSets* pPropEl = pCSession->m_listCPropSets.GetAt(pos);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_COLUMNALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, pPropEl);
					return 0;
				}

				case IDB_ADD:
				{
					CMDIChild	*pThis			= (CMDIChild*)GetThis(hWnd);
					HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
					CSession*	pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					ULONG		cItems			= ListView_GetItemCount(hWndColumnDesc);

					// make the insertion
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"Column%lu", cItems);
					LV_InsertItem(hWndColumnDesc, cItems, COLUMN_DBCID, wszBuffer);

					// type name
					LV_InsertItem(hWndColumnDesc, cItems, COLUMN_TYPENAME, pCSession->m_rgProvTypes[0].wszTypeName);

					ULONG i;
					// data type
					for(i=0; i<g_cDBTypes; i++)
					{
						if (pCSession->m_rgProvTypes[0].wType == g_rgDBTypes[i].lItem)
							break;
					}
					LV_InsertItem(hWndColumnDesc, cItems, COLUMN_DATATYPE, g_rgDBTypes[i].pwszName);

					// Column Size, Precision and Scale 
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"%Iu", pCSession->m_rgProvTypes[0].ulColumnSize);
					LV_InsertItem(hWndColumnDesc, cItems, COLUMN_SIZE, wszBuffer);
					LV_InsertItem(hWndColumnDesc, cItems, COLUMN_PREC, wszBuffer);
					if (IsNumericType(pCSession->m_rgProvTypes[0].wType))
						LV_InsertItem(hWndColumnDesc, cItems, COLUMN_PREC, wszBuffer);
					else
						LV_InsertItem(hWndColumnDesc, cItems, COLUMN_PREC, L"0");
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"%d", pCSession->m_rgProvTypes[0].iMaxScale);
					LV_InsertItem(hWndColumnDesc, cItems, COLUMN_SCALE, wszBuffer);

					// get the selected item
					INDEX iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);
					if(iSel < 0)
					{
						LV_SetItemState(hWndColumnDesc, 0, COLUMN_DBCID, LVIS_SELECTED, LVIS_SELECTED);						
					}

					// enable column properties
					::EnableWindow(::GetDlgItem(hWnd, IDB_COLPROP), TRUE);

					// set column properties				
					pCSession->m_listCPropSets.AddTail(new CPropSets());
					return 0;
				}

				case IDB_DELETE:
				{
					CMDIChild	*pThis			= (CMDIChild*)GetThis(hWnd);
					HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
					CSession*	pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					ULONG		cItems			= ListView_GetItemCount(hWndColumnDesc);
					
					// get the selected item
					ULONG iSel = ListView_GetNextItem(hWndColumnDesc, -1, LVNI_SELECTED);
					if(iSel != -1)
					{
						ListView_DeleteItem(hWndColumnDesc, iSel);
						SendMessage(::GetDlgItem(hWnd, IDE_COLNAME), WM_SETTEXT, 0, (LPARAM)"");
						
						// delete col props
						POSITION pos = pCSession->m_listCPropSets.FindIndex(iSel);
						CPropSets* pPropEl = pCSession->m_listCPropSets.RemoveAt(pos);
						pPropEl->RemoveAll();
					}
					
					if (cItems > 1)
					{
						LV_SetItemState(hWndColumnDesc, iSel == cItems-1 ? iSel-1: iSel, COLUMN_DBCID, LVIS_SELECTED, LVIS_SELECTED);	
					}
					else
					{
						//Disable fields
						::EnableWindow(::GetDlgItem(hWnd, IDB_COLPROP), FALSE);					
						::EnableWindow(::GetDlgItem(hWnd, IDE_COLUMNID), FALSE);
						::EnableWindow(::GetDlgItem(hWnd, IDC_TYPENAME), FALSE);
						::EnableWindow(::GetDlgItem(hWnd, IDC_DATATYPE), FALSE);
						::EnableWindow(::GetDlgItem(hWnd, IDE_SIZE), FALSE);
						::EnableWindow(::GetDlgItem(hWnd, IDE_PRECISION), FALSE);
						::EnableWindow(::GetDlgItem(hWnd, IDE_SCALE), FALSE);
					}
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

				case IDCANCEL:
				{
					//Cleanup any memory allocated
					CMDIChild*		pThis			= (CMDIChild*)GetThis(hWnd);
					CSession*		pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);

					pCSession->m_listCPropSets.RemoveAll();
					EndDialog(hWnd, FALSE);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild		*pThis			= (CMDIChild*)GetThis(hWnd);
					CSession*		pCSession		= SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					HWND			hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
					HWND			hWndTableID		= ::GetDlgItem(hWnd, IDE_TABLEID);
					DBCOLUMNDESC	*rgColumnDesc	= NULL;
					ULONG			cColumnDesc		= 0;
					IUnknown*		pIUnknown		= NULL;
					DBID			TableID;
					WCHAR			wszTableDBID[MAX_QUERY_LEN+1];
					WCHAR			wszTableName[MAX_NAME_LEN+1];
					DBPROPSET		*rgPropSet		= NULL;
					ULONG			i,cPropSet		= 0;
					HRESULT			hr;
					ULONG			ulValue;
					POSITION			pos;
					CPropSets		*pPropEl		= NULL;

					//Use Properties
					fUseProps = ::IsDlgButtonChecked(hWnd, IDB_USEPROPERTIES);

					//Obtain the Aggregation argument
					CAggregate* pCAggregate = NULL;
					fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
					if(fAggregation)
						pCAggregate = new CAggregate();
					
					//Obtain the Output (ppIUnknown) argument
					fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Interface
					REFIID riid = s_CComboInterface.GetGuid();

					// alloc space for the column desc array
					cColumnDesc = ListView_GetItemCount(hWndColumnDesc);
					SAFE_ALLOC(rgColumnDesc, DBCOLUMNDESC, cColumnDesc);

					// get the column desc array
					for(i=0; i<cColumnDesc; i++)
					{
						// initialize the whole structure
						memset(&rgColumnDesc[i], 0, sizeof(DBCOLUMNDESC));

						// get column name
						LV_GetItemText(hWndColumnDesc, i, COLUMN_DBCID, wszBuffer, MAX_NAME_LEN);
						rgColumnDesc[i].dbcid.eKind				= DBKIND_NAME;
						rgColumnDesc[i].dbcid.uName.pwszName	= wcsDuplicate(wszBuffer);

						// get column type name
						LV_GetItemText(hWndColumnDesc, i, COLUMN_TYPENAME, wszBuffer, MAX_NAME_LEN);
						rgColumnDesc[i].pwszTypeName = wcsDuplicate(wszBuffer);

						// get column data type
						LV_GetItemText(hWndColumnDesc, i, COLUMN_DATATYPE, wszBuffer, MAX_NAME_LEN);
						rgColumnDesc[i].wType = GetDBType(wszBuffer);

						// get column size
						LV_GetItemText(hWndColumnDesc, i, COLUMN_SIZE, wszBuffer, MAX_NAME_LEN);
						rgColumnDesc[i].ulColumnSize = wcstoul(wszBuffer, NULL, 10);

						// get column precision
						LV_GetItemText(hWndColumnDesc, i, COLUMN_PREC, wszBuffer, MAX_NAME_LEN);
						ulValue = wcstoul(wszBuffer, NULL, 10);
						rgColumnDesc[i].bPrecision = (BYTE)ulValue;

						// get column size
						LV_GetItemText(hWndColumnDesc, i, COLUMN_SCALE, wszBuffer, MAX_NAME_LEN);
						ulValue = wcstoul(wszBuffer, NULL, 10);
						rgColumnDesc[i].bScale = (BYTE)ulValue;

						// get properties
						pos		= pCSession->m_listCPropSets.FindIndex(i);
						pPropEl = pCSession->m_listCPropSets.GetAt(pos);
						rgColumnDesc[i].cPropertySets	= pPropEl->GetCount();
						rgColumnDesc[i].rgPropertySets	= pPropEl->GetPropSets();
					}

					// get TableID
					wSendMessage(hWndTableID, WM_GETTEXT, MAX_NAME_LEN, wszTableName);
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = wszTableName;
					DBIDToString(&TableID, wszTableDBID, MAX_QUERY_LEN);

					// check whether rowset/table properties are used
					if(fUseProps)
					{
						cPropSet	= pThis->m_CDefPropSets.GetCount();
						rgPropSet	= pThis->m_CDefPropSets.GetPropSets();
					}

					//ITableDefinition::CreateTable
					XTEST(hr = pCSession->m_pITableDefinition->CreateTable(pCAggregate, &TableID, cColumnDesc, rgColumnDesc, riid, cPropSet, rgPropSet, NULL, fOutput ? &pIUnknown : NULL));
					TESTC(TRACE_METHOD(hr, L"ITableDefinition::CreateTable(0x%p, %s, %lu, 0x%p, %s, %d, 0x%p, NULL, &0x%p)", pCAggregate, wszTableDBID, cColumnDesc, rgColumnDesc, GetInterfaceName(riid), cPropSet, rgPropSet, pIUnknown));

					if(fOutput)
					{
						//Handle Aggregation
						if(pCAggregate)
							pCAggregate->HandleAggregation(riid, &pIUnknown);
						
						//Display the Result
						//Just pass in a guess for the object type and let our helper figure out what object it is...
						//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
						if(!pThis->m_pCMainWindow->HandleObjectType(pCSession, pIUnknown, riid, eCRowset, 0, NULL, CREATE_NEWWINDOW_IFEXISTS | CREATE_DETERMINE_TYPE))
							TESTC(hr = E_FAIL);
					}

				CLEANUP:
					pThis->UpdateControls();
					for (i=0; i<cColumnDesc; i++)
					{
						SAFE_FREE(rgColumnDesc[i].pwszTypeName);
						if (DBKIND_NAME == rgColumnDesc[i].dbcid.eKind)
							SAFE_FREE(rgColumnDesc[i].dbcid.uName.pwszName);
					}
					SAFE_FREE(rgColumnDesc);
					SAFE_RELEASE(pIUnknown);
					SAFE_RELEASE(pCAggregate);

					if(SUCCEEDED(hr))
					{
						//Now just need to place this name in the EditBox
						//Inserted after the current "caret"
						pThis->m_pCQueryBox->ReplaceAll(wszTableName, FALSE/*bReplaceAll*/, TRUE/*fHighlight*/);

						pCSession->m_listCPropSets.RemoveAll(); //FreeColumnProperties();
						EndDialog(hWnd, TRUE);
						return 0;
					}
					return 0;
				}
			}
			break;
		}//WM_COMMAND

		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
			
			//ListView
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
				{
					return FALSE;
				}

				case LVN_ITEMCHANGED:
				{
					HWND		hWndColumnDesc	= ::GetDlgItem(hWnd, IDL_COLUMNDESC);
					HWND		hWndColumnID	= ::GetDlgItem(hWnd, IDE_COLUMNID);
					HWND		hWndTypeName	= ::GetDlgItem(hWnd, IDC_TYPENAME);
					HWND		hWndDataType	= ::GetDlgItem(hWnd, IDC_DATATYPE);
					HWND		hWndColSize		= ::GetDlgItem(hWnd, IDE_SIZE);
					HWND		hWndColPrec		= ::GetDlgItem(hWnd, IDE_PRECISION);
					HWND		hWndColScale	= ::GetDlgItem(hWnd, IDE_SCALE);
					HWND		hWndColName		= ::GetDlgItem(hWnd, IDE_COLNAME);
					WCHAR		wszBuffer2[MAX_NAME_LEN+1];
					NMLISTVIEW	*pLV = (NMLISTVIEW*)lParam;

					if (pLV->uChanged != LVIF_STATE)
						return 0;

					// get the selected item
					INDEX iSel = pLV->iItem;
					if (!(pLV->uNewState & LVIS_SELECTED))
					{
						if (!(pLV->uOldState & LVIS_SELECTED))
							return 0;

						// this is the previously selected item
						// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
						// and to Column Name static field
						wSendMessage(hWndColumnID, WM_GETTEXT, MAX_NAME_LEN, wszBuffer2);
						StringFormat(wszBuffer, NUMELE(wszBuffer), L"Current item %Iu (%s)", iSel, wszBuffer2);
						LV_SetItemText(hWndColumnDesc, iSel, COLUMN_DBCID, wszBuffer2);
						wSendMessage(hWndColName, WM_SETTEXT, 0, wszBuffer);

						// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
						// and to Column Name static field
						wSendMessage(hWndColSize, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
						LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SIZE, wszBuffer);

						// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
						// and to Column Name static field
						wSendMessage(hWndColPrec, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
						LV_SetItemText(hWndColumnDesc, iSel, COLUMN_PREC, wszBuffer);

						// retrieve the column type name and copy it to the COLUMN_TYPENAME field of the list view
						// and to Column Name static field
						wSendMessage(hWndColScale, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
						LV_SetItemText(hWndColumnDesc, iSel, COLUMN_SCALE, wszBuffer);
					}
					else
					{
						// retrieve the column name and copy it to the IDE_COLUMNID field
						LV_GetItemText(hWndColumnDesc, iSel, COLUMN_DBCID, wszBuffer2, MAX_NAME_LEN);
						wSendMessage(hWndColumnID, WM_SETTEXT, 0, wszBuffer2);
						StringFormat(wszBuffer, NUMELE(wszBuffer), L"Current item %Iu (%s)", iSel, wszBuffer2);
						wSendMessage(hWndColName, WM_SETTEXT, 0, wszBuffer);

						// retrieve the column type and copy it to the IDC_TYPENAME field
						LV_GetItemText(hWndColumnDesc, iSel, COLUMN_TYPENAME, wszBuffer, MAX_NAME_LEN);
						wSendMessage(hWndTypeName, CB_SELECTSTRING, -1, wszBuffer);

						// retrieve the column datatype and copy it to the IDC_DATATYPE field
						LV_GetItemText(hWndColumnDesc, iSel, COLUMN_DATATYPE, wszBuffer, MAX_NAME_LEN);
						wSendMessage(hWndDataType, CB_SELECTSTRING, -1, wszBuffer);

						// retrieve the column size and copy it to the IDE_SIZE field
						LV_GetItemText(hWndColumnDesc, iSel, COLUMN_SIZE, wszBuffer, MAX_NAME_LEN);
						wSendMessage(hWndColSize, WM_SETTEXT, 0, wszBuffer);

						// retrieve the column datatype and copy it to the IDE_PRECISION field
						LV_GetItemText(hWndColumnDesc, iSel, COLUMN_PREC, wszBuffer, MAX_NAME_LEN);
						wSendMessage(hWndColPrec, WM_SETTEXT, 0, wszBuffer);

						// retrieve the column datatype and copy it to the IDE_SCALE field
						LV_GetItemText(hWndColumnDesc, iSel, COLUMN_SCALE, wszBuffer, MAX_NAME_LEN);
						wSendMessage(hWndColScale, WM_SETTEXT, 0, wszBuffer);
						
						//Enable fields
						::EnableWindow(::GetDlgItem(hWnd, IDB_COLPROP), TRUE);
						::EnableWindow(::GetDlgItem(hWnd, IDE_COLUMNID), TRUE);
						::EnableWindow(::GetDlgItem(hWnd, IDC_TYPENAME), TRUE);
						::EnableWindow(::GetDlgItem(hWnd, IDC_DATATYPE), TRUE);
						::EnableWindow(::GetDlgItem(hWnd, IDE_SIZE),  TRUE);
						::EnableWindow(::GetDlgItem(hWnd, IDE_PRECISION),  TRUE);
						::EnableWindow(::GetDlgItem(hWnd, IDE_SCALE), TRUE);
					}
					return FALSE;
				}	

				case LVN_ITEMCHANGING:
				{
						return FALSE; //Allow the change
				}
			}


		}//WM_NOTIFY
	}//switch message
		
	return FALSE;
} // CMDIChild::CreateTableProc


////////////////////////////////////////////////////////////////
// CMDIChild::DropColumnProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::DropColumnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND hWndTableID	= ::GetDlgItem(hWnd, IDE_TABLEID);

			//Fill In TableID as Default
			WCHAR* pwszTableName = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndTableID, WM_SETTEXT, 0, pwszTableName);

			CenterDialog(hWnd);
			SAFE_FREE(pwszTableName);
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
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CSession* pCSession = SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					HWND hWndTableID	= ::GetDlgItem(hWnd, IDE_TABLEID);
					HWND hWndColumnID	= ::GetDlgItem(hWnd, IDE_COLUMNID);
					WCHAR wszTableDBID[MAX_QUERY_LEN+1];
					WCHAR wszColumnDBID[MAX_QUERY_LEN+1];
					HRESULT hr = S_OK;

					//TableID
					DBID TableID;
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = wGetWindowText(hWndTableID);
					DBIDToString(&TableID, wszTableDBID, MAX_QUERY_LEN);

					//ColumnID
					DBID ColumnID;
					ColumnID.eKind = DBKIND_NAME;
					ColumnID.uName.pwszName = wGetWindowText(hWndColumnID);
					DBIDToString(&ColumnID, wszColumnDBID, MAX_QUERY_LEN);

					//ITableDefinition::DropColumn
					XTEST(hr = pCSession->m_pITableDefinition->DropColumn(&TableID, &ColumnID));
					TESTC(TRACE_METHOD(hr, L"ITableDefinition::DropColumn(%s, %s)", wszTableDBID, wszColumnDBID));
					
					if(SUCCEEDED(hr))
					{
						EndDialog(hWnd, TRUE);
					}
			
					SAFE_FREE(TableID.uName.pwszName);
					SAFE_FREE(ColumnID.uName.pwszName);
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

CLEANUP:
	return FALSE;
} // CMDIChild::DropColumnProc


////////////////////////////////////////////////////////////////
// CMDIChild::DropTableProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::DropTableProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND hWndTableID	= ::GetDlgItem(hWnd, IDE_TABLEID);

			//Fill In TableID as Default
			WCHAR* pwszTableName = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndTableID, WM_SETTEXT, 0, pwszTableName);

			CenterDialog(hWnd);
			SAFE_FREE(pwszTableName);
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
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CSession* pCSession = SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					HWND hWndTableID	= ::GetDlgItem(hWnd, IDE_TABLEID);
					WCHAR wszTableDBID[MAX_QUERY_LEN+1];
					HRESULT hr = S_OK;

					//TableID
					DBID TableID;
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = wGetWindowText(hWndTableID);
					DBIDToString(&TableID, wszTableDBID, MAX_QUERY_LEN);

					//ITableDefinition::DropTable
					XTEST(hr = pCSession->m_pITableDefinition->DropTable(&TableID));
					TRACE_METHOD(hr, L"ITableDefinition::DropTable(%s)", wszTableDBID);
					
					if(SUCCEEDED(hr))
					{
						EndDialog(hWnd, TRUE);
					}
					
					SAFE_FREE(TableID.uName.pwszName);
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
// CMDIChild::DropConstraintProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::DropConstraintProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND hWndTableID	= ::GetDlgItem(hWnd, IDE_TABLEID);

			//Fill In TableID as Default
			WCHAR* pwszTableName = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndTableID, WM_SETTEXT, 0, pwszTableName);

			CenterDialog(hWnd);
			SAFE_FREE(pwszTableName);
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
					CMDIChild	*pThis = (CMDIChild*)GetThis(hWnd);
					CSession	*pCSession = SOURCE_GETOBJECT(pThis->m_pCSource, CSession);
					HWND		hWndTableID	= ::GetDlgItem(hWnd, IDE_TABLEID);
					HWND		hWndConsID	= ::GetDlgItem(hWnd, IDE_CONSTRAINTID);
					WCHAR		wszTableDBID[MAX_QUERY_LEN+1];
					WCHAR		wszConsDBID[MAX_QUERY_LEN+1];
					HRESULT		hr = S_OK;

					//TableID
					DBID TableID;
					TableID.eKind = DBKIND_NAME;
					TableID.uName.pwszName = wGetWindowText(hWndTableID);
					DBIDToString(&TableID, wszTableDBID, MAX_QUERY_LEN);

					//ConstraintID
					DBID ConstraintID;
					ConstraintID.eKind = DBKIND_NAME;
					ConstraintID.uName.pwszName = wGetWindowText(hWndConsID);
					DBIDToString(&ConstraintID, wszConsDBID, MAX_QUERY_LEN);

					//ITableDefinitionWithConstraints::DropConstraint
					XTEST(hr = pCSession->m_pITableDefinitionWithConstraints->DropConstraint(&TableID, &ConstraintID));
					TESTC(TRACE_METHOD(hr, L"ITableDefinitionWithConstraints::DropConstraint(%s, %s)", wszTableDBID, wszConsDBID));
					
					if(SUCCEEDED(hr))
					{
						EndDialog(hWnd, TRUE);
					}
			
					SAFE_FREE(TableID.uName.pwszName);
					SAFE_FREE(ConstraintID.uName.pwszName);
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

CLEANUP:
	return FALSE;
} // CMDIChild::DropConstraintProc


////////////////////////////////////////////////////////////////
// CMDIChild::ISCO_DeleteProc
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::ISCO_DeleteProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const static WIDENAMEMAP rgISODeleteFlags[] = 
	{
		VALUE_WCHAR(DBDELETE_ASYNC),
		VALUE_WCHAR(DBDELETE_ATOMIC),
	};

	switch(message)
	{
		case WM_INITDIALOG:	
		{	
			//Save the "this" pointer
			CWaitCursor waitCursor;
			CMDIChild*		pThis		= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND			hWndFlags	= ::GetDlgItem(hWnd, IDC_ISCO_FLAGS);

			for(ULONG i=0; i<NUMELE(rgISODeleteFlags); i++)
			{
				INDEX iSel = SendMessageW(hWndFlags, LB_ADDSTRING, 0, (LPARAM)rgISODeleteFlags[i].pwszName);
				SendMessageW(hWndFlags, LB_SETITEMDATA, iSel, rgISODeleteFlags[i].lItem);
			}

			//Placement of Dialog, want it just below the "Init" button...
			CenterDialog(hWnd);
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_ADD:
				{
					HWND		hWndEditURL		= ::GetDlgItem(hWnd, IDE_URL);
					HWND		hWndURLs		= ::GetDlgItem(hWnd, IDC_URLs);
					CHAR		szURL[MAX_NAME_LEN+1] = "";

					// get the text from the edit field
					SendMessageA(hWndEditURL, WM_GETTEXT, MAX_NAME_LEN, (LPARAM)szURL);
					if (szURL[0])
					{
						// and add it to the URL list box
						SendMessageA(hWndURLs, LB_ADDSTRING, 0, (LPARAM)szURL);
					}
					else
						MessageBox(GetFocus(), "There is no URL to ADD!", "WARNING", MB_OK);

					return 0;
				}

				case IDB_DELETE:
				{
					HWND		hWndURLs		= ::GetDlgItem(hWnd, IDC_URLs);

					INDEX iSel = (INDEX)SendMessage(hWndURLs, LB_GETCURSEL, 0, 0);
					if (LB_ERR != iSel)
						SendMessage(hWndURLs, LB_DELETESTRING, iSel, 0);

					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild	*pThis					= (CMDIChild*)GetThis(hWnd);
					HWND		hWndURLs				= ::GetDlgItem(hWnd, IDC_URLs);
					HWND		hWndFlags				= ::GetDlgItem(hWnd, IDC_ISCO_FLAGS);
					DWORD		dwDeleteFlags			= 0;
					WCHAR		**rgpwszURLs			= NULL;
					WCHAR		wszBuffer[MAX_NAME_LEN+1]= {0};
					DBSTATUS	*rgdbStatus				= NULL;
					HRESULT		hr;

					//Obtain IScopedOperations
					IScopedOperations* pIScopedOperations = SOURCE_GETINTERFACE(pThis->m_pCSource, IScopedOperations);
					ASSERT(pIScopedOperations);

					INDEX iSel;
					INDEX nCount = (INDEX)SendMessage(hWndFlags, LB_GETCOUNT, 0, 0);
					for( iSel = 0; iSel < nCount; iSel++)
					{
						//Is this a selected Item?
						if(SendMessage(hWndFlags, LB_GETSEL, iSel, 0) > 0)
						{
							//We setup the listbox with the item data as the value for each selection
							dwDeleteFlags |= SendMessage(hWndFlags, LB_GETITEMDATA, iSel, 0);
						}
					}

					// get URL array
					nCount = (ULONG)SendMessage(hWndURLs, LB_GETCOUNT, 0, 0);
					if (nCount > 0)
					{
						SAFE_ALLOC(rgpwszURLs, WCHAR*, nCount);
						SAFE_ALLOC(rgdbStatus, DBSTATUS, nCount);
					}

					for(iSel=0; iSel<nCount; iSel++)
					{
						wSendMessage(hWndURLs, LB_GETTEXT, iSel, wszBuffer);
						rgpwszURLs[iSel] = wcsDuplicate(wszBuffer);
					}

					XTEST(hr = pIScopedOperations->Delete(nCount, (const WCHAR**)rgpwszURLs, dwDeleteFlags, rgdbStatus));
					TESTC(TRACE_METHOD(hr, L"IScopedOperations::Delete(%Iu, 0x%p, 0x%08x, 0x%p)", nCount, rgpwszURLs, dwDeleteFlags, rgdbStatus));

				CLEANUP:
					pThis->UpdateControls();

					//Display Status errors...
					for(iSel=0; iSel<nCount; iSel++)
					{
						SAFE_FREE(rgpwszURLs[iSel]);
						
						if(rgdbStatus[iSel] != DBSTATUS_S_OK)
							wMessageBox(GetFocus(), MB_OK, wsz_ERROR, L"Status = %s \nrgpwszURL[%Iu] = %s", GetStatusName(rgdbStatus[iSel]), iSel, rgpwszURLs[iSel]);
					}

					SAFE_FREE(rgpwszURLs);
					SAFE_FREE(rgdbStatus);
					
					if(SUCCEEDED(hr))
						EndDialog(hWnd, TRUE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND

	}//switch message
		
	return FALSE;
} // CMDIChild::ISCO_DeleteProc


////////////////////////////////////////////////////////////////
// CMDIChild::ISCO_Proc
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::ISCO_Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const static WIDENAMEMAP rgISOCopyFlags[] = 
	{
		VALUE_WCHAR(DBCOPY_REPLACE_EXISTING),
		VALUE_WCHAR(DBCOPY_ASYNC),
		VALUE_WCHAR(DBCOPY_ALLOW_EMULATION),
		VALUE_WCHAR(DBCOPY_NON_RECURSIVE),
		VALUE_WCHAR(DBCOPY_ATOMIC),
	};

	const static WIDENAMEMAP rgISOMoveFlags[] = 
	{
		VALUE_WCHAR(DBMOVE_REPLACE_EXISTING),
		VALUE_WCHAR(DBMOVE_ASYNC),
		VALUE_WCHAR(DBMOVE_DONT_UPDATE_LINKS),
		VALUE_WCHAR(DBMOVE_ALLOW_EMULATION),
		VALUE_WCHAR(DBMOVE_ATOMIC),
	};


	switch(message)
	{
		case WM_INITDIALOG:	
		{	
			//Save the "this" pointer
			CWaitCursor waitCursor;
			CMDIChild*		pThis		= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			HWND			hWndFlags	= ::GetDlgItem(hWnd, IDC_ISCO_FLAGS);
			HWND			hWndURLs	= ::GetDlgItem(hWnd, IDL_URLs);
			RECT		rect;

			//Populate the ListBox
			if(pThis->m_idSource == IDM_ISCOPEDOPERATIONS_MOVE)
			{
				::SetWindowText(hWnd, "IScopedOperations::Move");
				for(ULONG i=0; i<NUMELE(rgISOMoveFlags); i++)
				{
					INDEX iSel = SendMessageW(hWndFlags, LB_ADDSTRING, 0, (LPARAM)rgISOMoveFlags[i].pwszName);
					SendMessageW(hWndFlags, LB_SETITEMDATA, iSel, rgISOMoveFlags[i].lItem);
				}
			}
			else
			{
				::SetWindowText(hWnd, "IScopedOperations::Copy");
				for(ULONG i=0; i<NUMELE(rgISOCopyFlags); i++)
				{
					INDEX iSel = SendMessageW(hWndFlags, LB_ADDSTRING, 0, (LPARAM)rgISOCopyFlags[i].pwszName);
					SendMessageW(hWndFlags, LB_SETITEMDATA, iSel, rgISOCopyFlags[i].lItem);
				}
			}

			SendMessage(hWndURLs, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);
			LV_InsertColumn(hWndURLs,   COLUMN_ISCO_SRCURL,		L"Source URL",		IMAGE_NONE);
			LV_InsertColumn(hWndURLs,   COLUMN_ISCO_DESTURL,	L"Destination URL", IMAGE_NONE);

			GetWindowRect(hWndURLs, (LPRECT)&rect);
			SendMessage(hWndURLs, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_ISCO_SRCURL, (LPARAM)(rect.right-rect.left)/2);
			SendMessage(hWndURLs, LVM_SETCOLUMNWIDTH, (WPARAM)COLUMN_ISCO_DESTURL, (LPARAM)(rect.right-rect.left)/2);

			//Placement of Dialog, want it just below the "Init" button...
			CenterDialog(hWnd);
			return TRUE;
		}
		
		case WM_COMMAND:
		{
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_ADD:
				{
					HWND		hWndSrcURL		= ::GetDlgItem(hWnd, IDE_ISCO_SOURCE);
					HWND		hWndDestURL		= ::GetDlgItem(hWnd, IDE_ISCO_DESTINATION);
					HWND		hWndURLs		= ::GetDlgItem(hWnd, IDL_URLs);
					WCHAR		wszSrcURL[MAX_NAME_LEN+1] = L"";
					WCHAR		wszDestURL[MAX_NAME_LEN+1] = L"";
					ULONG		cItems			= ListView_GetItemCount(hWndURLs);

					// get the text from the edit field
					wSendMessage(hWndSrcURL, WM_GETTEXT, MAX_NAME_LEN, wszSrcURL);
					wSendMessage(hWndDestURL, WM_GETTEXT, MAX_NAME_LEN, wszDestURL);
					if(wszSrcURL[0] || wszDestURL[0])
					{
						// and add it to the URL list box
						// make the insertion
						LV_InsertItem(hWndURLs, cItems, COLUMN_ISCO_SRCURL,		wszSrcURL);
						LV_InsertItem(hWndURLs, cItems, COLUMN_ISCO_DESTURL,	wszDestURL);
					}
					else
					{
						wMessageBox(GetFocus(), MB_OK, wsz_ERROR, L"There is no source or destination URL to ADD!");
					}

					return 0;
				}

				case IDB_DELETE:
				{
					HWND		hWndURLs		= ::GetDlgItem(hWnd, IDL_URLs);
					INT			cItems;
					INT			iSel;

					// get the selected item
					iSel = ListView_GetNextItem(hWndURLs, -1, LVNI_SELECTED);
					
					cItems = ListView_GetItemCount(hWndURLs);
					if (iSel > -1)
						ListView_DeleteItem(hWndURLs, iSel);
					if (cItems > 1)
						LV_SetItemState(hWndURLs, iSel == cItems-1 ? iSel-1: iSel, COLUMN_ISCO_SRCURL, LVIS_SELECTED, LVIS_SELECTED);	
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild	*pThis					= (CMDIChild*)GetThis(hWnd);
					HWND		hWndURLs				= ::GetDlgItem(hWnd, IDL_URLs);
					HWND		hWndFlags				= ::GetDlgItem(hWnd, IDC_ISCO_FLAGS);
					DWORD		dwFlags					= 0;
					WCHAR		**rgpwszSourceURLs		= NULL;
					WCHAR		**rgpwszDestURLs		= NULL;
					WCHAR		**rgpwszNewURLs			= NULL;
					OLECHAR		*pStringsBuffer			= NULL;
					DBSTATUS	*rgdbStatus				= NULL;
					
					WCHAR		wszBuffer[MAX_NAME_LEN+1]= {0};
					INDEX		cRows = 0;
					HRESULT		hr = S_OK;

					//Obtain IScopedOperations
					IScopedOperations* pIScopedOperations = SOURCE_GETINTERFACE(pThis->m_pCSource, IScopedOperations);
					ASSERT(pIScopedOperations);
					
					INDEX iSel;
					INDEX nCount = (INDEX)SendMessage(hWndFlags, LB_GETCOUNT, 0, 0);
					for(iSel = 0; iSel < nCount; iSel++)
					{
						//Is this a selected Item?
						if(SendMessage(hWndFlags, LB_GETSEL, iSel, 0) > 0)
						{
							//We setup the listbox with the item data as the value for each selection
							dwFlags |= SendMessage(hWndFlags, LB_GETITEMDATA, iSel, 0);
						}
					}

					// get URL array
					cRows = ListView_GetItemCount(hWndURLs);
					SAFE_ALLOC(rgpwszSourceURLs, WCHAR*, cRows);
					SAFE_ALLOC(rgpwszDestURLs, WCHAR*, cRows);
					SAFE_ALLOC(rgpwszNewURLs, WCHAR*, cRows);
					SAFE_ALLOC(rgdbStatus, DBSTATUS, cRows);

					// get the arrays of source and destination URLs
					for (iSel=0; iSel<cRows; iSel++)
					{
						// get source URL
						LV_GetItemText(hWndURLs, iSel, COLUMN_ISCO_SRCURL, wszBuffer, MAX_NAME_LEN);
						rgpwszSourceURLs[iSel] = wcsDuplicate(wszBuffer);

						// get destination URL
						LV_GetItemText(hWndURLs, iSel, COLUMN_ISCO_DESTURL, wszBuffer, MAX_NAME_LEN);
						rgpwszDestURLs[iSel] = wcsDuplicate(wszBuffer);
					}

					switch(pThis->m_idSource)
					{
						case IDM_ISCOPEDOPERATIONS_COPY:
							XTEST(hr = pIScopedOperations->Copy(cRows, (const WCHAR**)rgpwszSourceURLs, (const WCHAR**)rgpwszDestURLs, dwFlags, NULL, rgdbStatus, rgpwszNewURLs, &pStringsBuffer));
							TESTC(TRACE_METHOD(hr, L"IScopedOperations::Copy(%Iu, 0x%p, 0x%p, 0x%08x, NULL, 0x%p, 0x%p, &0x%p)", cRows, rgpwszSourceURLs, rgpwszDestURLs, dwFlags, rgdbStatus, rgpwszNewURLs, pStringsBuffer));
							break;
						
						case IDM_ISCOPEDOPERATIONS_MOVE:
							XTEST(hr = pIScopedOperations->Move(cRows, (const WCHAR**)rgpwszSourceURLs, (const WCHAR**)rgpwszDestURLs, dwFlags, NULL, rgdbStatus, rgpwszNewURLs, &pStringsBuffer));
							TESTC(TRACE_METHOD(hr, L"IScopedOperations::Move(%Iu, 0x%p, 0x%p, 0x%08x, NULL, 0x%p, 0x%p, &0x%p)", cRows, rgpwszSourceURLs, rgpwszDestURLs, dwFlags, rgdbStatus, rgpwszNewURLs, pStringsBuffer));
							break;
					};
				

				CLEANUP:
					pThis->UpdateControls();
					for(iSel = 0; iSel < cRows; iSel++)
					{
						SAFE_FREE(rgpwszSourceURLs[iSel]);
						SAFE_FREE(rgpwszDestURLs[iSel]);
					}
					SAFE_FREE(rgpwszSourceURLs);
					SAFE_FREE(rgpwszDestURLs);
					SAFE_FREE(rgdbStatus);
					SAFE_FREE(pStringsBuffer);
					if (S_OK == hr)
						EndDialog(hWnd, TRUE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND

	}//switch message
		
	return FALSE;
} // CMDIChild::ISCO_Proc


////////////////////////////////////////////////////////////////
// CMDIChild::ExecuteProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::ExecuteProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid	s_CComboInterface;
	static BOOL fSetCommandText		= TRUE;				//Default to TRUE
	static BOOL fSetCommandStream	= FALSE;			//Default to FALSE
	static BOOL fUseParams			= FALSE;			//Default to FALSE
	static BOOL fAggregation		= FALSE;			//Default
	static BOOL fOutput				= TRUE;				//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis			= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CDataSource* pCDataSource	= SOURCE_GETPARENT(pThis->m_pCSource, CDataSource);

			//Fill In CommandText as Default
			HWND hWndCmdText	= ::GetDlgItem(hWnd, IDE_COMMANDTEXT);
			WCHAR* pwszQuery = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndCmdText, WM_SETTEXT, 0, pwszQuery);

			//Use Parameters
			::CheckDlgButton(hWnd, IDB_USEPARAMS,		BST2STATE(fUseParams));
			//SetCommandText
			::CheckDlgButton(hWnd, IDB_SETCOMMANDTEXT,	BST2STATE(fSetCommandText));
			//SetCommandStream
			::CheckDlgButton(hWnd, IDB_SETCOMMANDSTREAM,BST2STATE(fSetCommandStream));
			//None
			::CheckDlgButton(hWnd, IDB_SETCOMMANDNONE,	BST2STATE(!(fSetCommandText || fSetCommandStream)));

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IRowset);

			//Defaults (refiid) other than those last used...
			if(pCDataSource && BIT_SET(pCDataSource->m_lDataSourceType, DBPROPVAL_DST_MDP))
				s_CComboInterface.SetGuid(IID_IMDDataset);
			else if(s_CComboInterface.GetGuid() == IID_IMDDataset)
				s_CComboInterface.SetGuid(IID_IRowset);

			CenterDialog(hWnd);
			SAFE_FREE(pwszQuery);
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
				case IDB_SETPARAMS:
				{
					//Get the "this" pointer
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);

					//Display the Parameter Dialog
					CExecuteParamDlg paramDlg(pThis);
					if(IDOK == paramDlg.DoModal(hWnd))
					{
						//Since the user setup the params correctly they probably want to
						//now use these parameters with execute.  Check the "use params" box...
						::CheckDlgButton(hWnd, IDB_USEPARAMS,	BST2STATE(TRUE));
					}
					return 0;
				}

				case IDB_SETPROPERTIES:
				{
					//Get the "this" pointer
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CCommand* pCCommand			= SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pCCommand, CDataSource);
					CRowset* pCRowset			= (CRowset*)pThis->GetObject(eCRowset);

					//We should release the Open Objects at this point (avoid DB_E_OPENOBJECT)
					if(pCCommand && pThis->GetOptions()->m_dwCommandOpts & COMMAND_RELEASE_OPENOBJECTS)
					{
						pCCommand->ReleaseChildren();
						pThis->UpdateControls();
					}	
					
					//ICommandProperties::SetProperties
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_ICommandProperties, pCCommand->m_pICommandProperties, pCDataSource ? pCDataSource->m_pIDBProperties : NULL);
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

				case IDB_SETCOMMANDSTREAM:
				{
					//By default:  If the user chooses to use ICommandStream::SetCommandStream
					//they probably want a stream back as well, set this for them.  They can always
					//override this, byt changing it in the combo afterwards...
					if(::IsDlgButtonChecked(hWnd, IDB_SETCOMMANDSTREAM))
						s_CComboInterface.SetGuid(IID_ISequentialStream);
					return 0;
				}

				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CCommand* pCCommand	= SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);

					HWND hWndCmdText	= ::GetDlgItem(hWnd, IDE_COMMANDTEXT);
					HRESULT hr = S_OK;
					IUnknown* pIUnknown = NULL;
					WCHAR* pwszCommandText = NULL;

					//Obtain the Aggregation argument
					CAggregate* pCAggregate = NULL;
					fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
					if(fAggregation)
						pCAggregate = new CAggregate();
					
					//Obtain the Output (ppIUnknown) argument
					fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

					//Interface
					IID iid			= s_CComboInterface.GetGuid();

					//Use Parameters
					fUseParams		= ::IsDlgButtonChecked(hWnd, IDB_USEPARAMS);

					//SetCommandText
					fSetCommandText = ::IsDlgButtonChecked(hWnd, IDB_SETCOMMANDTEXT);
					fSetCommandStream = ::IsDlgButtonChecked(hWnd, IDB_SETCOMMANDSTREAM);
					if(fSetCommandText || fSetCommandStream)
						pwszCommandText = wGetWindowText(hWndCmdText);
					
					//Execute the Command
					TESTC(hr = pCCommand->Execute(pCAggregate, pwszCommandText, iid, fUseParams, NULL, fOutput ? &pIUnknown : NULL, fSetCommandStream));

					//Process the Rowset
					TESTC(hr = pThis->HandleRowset(pCCommand, pIUnknown, iid, CREATE_NEWWINDOW_IFEXISTS, IID_ICommand, pwszCommandText));

				CLEANUP:
					if(SUCCEEDED(hr))
					{
						//Now just need to place this name in the EditBox
						//Inserted after the current "caret"
						if(fSetCommandText)
							pThis->m_pCQueryBox->ReplaceAll(pwszCommandText, FALSE/*bReplaceAll*/, TRUE/*fHighlight*/);
						EndDialog(hWnd, TRUE);
					}
					
					SAFE_RELEASE(pCAggregate);
					SAFE_RELEASE(pIUnknown);
					SAFE_FREE(pwszCommandText);
					pThis->UpdateControls();
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
// CMDIChild::CommandPersistProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::CommandPersistProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DBCOMMANDPERSISTFLAG dwSavedFlags = DBCOMMANDPERSISTFLAG_DEFAULT;
	
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CCommand* pCCommand = SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
			HWND hWndCommandID	= ::GetDlgItem(hWnd, IDE_COMMANDID);
			HWND hWndFlags = ::GetDlgItem(hWnd, IDL_FLAGS);
			DBID* pCommandID = NULL;
			HRESULT hr = S_OK;

			//Fill in the correct window title
			if(pThis->m_idSource == IDM_COMMANDPERSIST_DELETECOMMAND)
			{
				//Falgs not a paraemter for DeleteCommand
				::EnableWindow(hWndFlags, FALSE);
				SendMessage(hWnd,	WM_SETTEXT, 0, (LPARAM)"ICommandPersist::DeleteCommand");
			}
			else if(pThis->m_idSource == IDM_COMMANDPERSIST_LOADCOMMAND)
			{
				SendMessage(hWnd,	WM_SETTEXT, 0, (LPARAM)"ICommandPersist::LoadCommand");
			}
			else
			{
				//Default title is SaveCommand
				ASSERT(pThis->m_idSource == IDM_COMMANDPERSIST_SAVECOMMAND);
			}

				
			//CommandID
			//Default to the current command
			ASSERT(pCCommand);
			hr = pCCommand->GetCurrentCommand(&pCommandID);
			if(SUCCEEDED(hr) && pCommandID)
			{
				//Fill In CommandID as Default
				 if(pCommandID->eKind==DBKIND_NAME && pCommandID->uName.pwszName)
					 wSendMessage(hWndCommandID, WM_SETTEXT, 0, pCommandID->uName.pwszName);
			}

			//dwFlags			
			const static WIDENAMEMAP rgFlags[] = 
			{
				VALUE_WCHAR(DBCOMMANDPERSISTFLAG_NOSAVE),
				VALUE_WCHAR(DBCOMMANDPERSISTFLAG_DEFAULT),
				VALUE_WCHAR(DBCOMMANDPERSISTFLAG_PERSISTVIEW),
				VALUE_WCHAR(DBCOMMANDPERSISTFLAG_PERSISTPROCEDURE),
			};

			//Populate the Flags ListBox
			SendMessage(hWndFlags, LB_RESETCONTENT, 0, 0);
			for(ULONG i=0; i<NUMELE(rgFlags); i++)
			{
				INDEX iSel = (INDEX)wSendMessage(hWndFlags, LB_ADDSTRING, 0, rgFlags[i].pwszName);
				SendMessage(hWndFlags,	LB_SETITEMDATA,	iSel, (LPARAM)rgFlags[i].lItem);
				
				//Reselect all previously selected items...
				SendMessage(hWndFlags, LB_SETSEL, (dwSavedFlags & rgFlags[i].lItem) == (DBCOMMANDPERSISTFLAG)rgFlags[i].lItem, i);
			}
			
			CenterDialog(hWnd);
			DBIDFree(pCommandID);
			SAFE_FREE(pCommandID);
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
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);
					CCommand* pCCommand = SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
					HWND hWndCommandID	= ::GetDlgItem(hWnd, IDE_COMMANDID);
					HWND hWndFlags		= ::GetDlgItem(hWnd, IDL_FLAGS);
					HRESULT hr = S_OK;

					//Obtain the CommandID
					DBID CommandID;
					CommandID.eKind = DBKIND_NAME;
					CommandID.uName.pwszName = wGetWindowText(hWndCommandID);

					//Obtain all Selected Flags...
					INDEX iSelCount = (INDEX)SendMessage(hWndFlags, LB_GETSELCOUNT, 0, 0);
					ASSERT(iSelCount < 20);
					LONG rgSelItems[20];
					SendMessage(hWndFlags, LB_GETSELITEMS, (WPARAM)20, (LPARAM)rgSelItems);
					for(LONG i=0; i<iSelCount; i++)
					{
						DBCOMMANDPERSISTFLAG dwSelFlag = (DBCOMMANDPERSISTFLAG)SendMessage(hWndFlags, LB_GETITEMDATA, rgSelItems[i], 0);
						dwSavedFlags |= dwSelFlag;
					}

					if(pThis->m_idSource == IDM_COMMANDPERSIST_DELETECOMMAND)
					{
						//ICommandPersist::DeleteCommand
						XTEST(hr = pCCommand->m_pICommandPersist->DeleteCommand(&CommandID));
						TESTC(TRACE_METHOD(hr, L"ICommandPersist::DeleteCommand({\"%s\"})", CommandID.uName.pwszName));
					}
					else if(pThis->m_idSource == IDM_COMMANDPERSIST_LOADCOMMAND)
					{
						//ICommandPersist::LoadCommand
						XTEST(hr = pCCommand->m_pICommandPersist->LoadCommand(&CommandID, dwSavedFlags));
						TESTC(TRACE_METHOD(hr, L"ICommandPersist::LoadCommand({\"%s\"}, 0x%08x)", CommandID.uName.pwszName, dwSavedFlags));

						//The symmantics of LoadCommand replace the "underlying" command.
						//so there is no interface to "repopluate" the current command with, and 
						//all currently held interfaces should still be valid...
					}
					else
					{
						//ICommandPersist::SaveCommand
						ASSERT(pThis->m_idSource == IDM_COMMANDPERSIST_SAVECOMMAND);
						XTEST(hr = pCCommand->m_pICommandPersist->SaveCommand(&CommandID, dwSavedFlags));
						TESTC(TRACE_METHOD(hr, L"ICommandPersist::SaveCommand({\"%s\"}, 0x%08x)", CommandID.uName.pwszName, dwSavedFlags));
					}

				CLEANUP:
					SAFE_FREE(CommandID.uName.pwszName);
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
// CMDIChild::SetCommandTextProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::SetCommandTextProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL fRelOpenObjects = TRUE;	//Default
	static BOOL fUnicodeStream	= TRUE;	//Default
	static CComboBoxGuid	s_CComboDialect;
	static CComboBoxGuid	s_CComboRefiid;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis	= (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CCommand* pCCommand = SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
			HWND hWndCmdText	= ::GetDlgItem(hWnd, IDE_COMMANDTEXT);

			//Fill-in Dialect Combo
			s_CComboDialect.CreateIndirect(hWnd, IDC_DIALECT);
			s_CComboDialect.Populate(g_cDialectMaps, g_rgDialectMaps);
			s_CComboDialect.SetGuid(pCCommand->m_guidDialect);

			//Fill-in REFIID Combo (ICommandStream)
			if(pThis->m_idSource == IDM_SETCOMMANDSTREAM)
			{
				s_CComboRefiid.CreateIndirect(hWnd, IDC_REFIID);
				s_CComboRefiid.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
				s_CComboRefiid.SetGuid(IID_ISequentialStream);
			}

			//Fill In CommandText as Default
			WCHAR* pwszQuery = pThis->m_pCQueryBox->GetSelectedText();
			wSendMessage(hWndCmdText, WM_SETTEXT, 0, pwszQuery);

			//Default Options
			::CheckDlgButton(hWnd, IDB_UNICODE,				BST2STATE(fUnicodeStream));
			::CheckDlgButton(hWnd, IDB_RELEASE_OPENOBJECTS,	BST2STATE(fRelOpenObjects));

			CenterDialog(hWnd);
			SAFE_FREE(pwszQuery);
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
					CMDIChild* pThis	= (CMDIChild*)GetThis(hWnd);
					CCommand* pCCommand = SOURCE_GETOBJECT(pThis->m_pCSource, CCommand);
					CRowset* pCRowset	= (CRowset*)pThis->GetObject(eCRowset);
					WCHAR* pwszCommandText = NULL;

					HWND hWndCmdText	= ::GetDlgItem(hWnd, IDE_COMMANDTEXT);
					HRESULT hr = S_OK;

					//Options
					fUnicodeStream = ::IsDlgButtonChecked(hWnd, IDB_UNICODE);
					fRelOpenObjects = ::IsDlgButtonChecked(hWnd, IDB_RELEASE_OPENOBJECTS);
					if(pCCommand && fRelOpenObjects)
					{
						pCCommand->ReleaseChildren();
						pThis->UpdateControls();
					}	

					//Get Guid Dialect
					//NOTE: Save it into a temporary in case this fails...
					GUID guidDialect = GUID_NULL;
					INDEX iSel = s_CComboDialect.GetCurSel();
					if(iSel != CB_ERR)
					{
						//Get ItemData which is the Guid Pointer
						guidDialect = s_CComboDialect.GetGuid();
					}
					else
					{
						//Convert Users String to a Guid
						WCHAR wszBuffer[MAX_NAME_LEN] = {0};
						s_CComboDialect.GetSelText(wszBuffer, MAX_NAME_LEN);
						XTESTC(hr = CLSIDFromString(wszBuffer, &guidDialect));
					}

					//ICommand::SetCommandText
					pwszCommandText = wGetWindowText(hWndCmdText);
					if(pThis->m_idSource == IDM_SETCOMMANDSTREAM)
					{
						REFIID riid = s_CComboRefiid.GetGuid();
						TESTC(hr = pCCommand->SetCommandStream(pwszCommandText, 
																riid, 
																&guidDialect, 
																fUnicodeStream));
					}
					else
					{
						TESTC(hr = pCCommand->SetCommandText(pwszCommandText, &guidDialect));
					}
					
				CLEANUP:
					if(SUCCEEDED(hr))
					{
						//Now just need to place this name in the EditBox
						//Inserted after the current "caret"
						pThis->m_pCQueryBox->ReplaceAll(pwszCommandText, FALSE/*bReplaceAll*/, TRUE/*fHighlight*/);
						EndDialog(hWnd, TRUE);
					}

					SAFE_FREE(pwszCommandText);
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
// CMDIChild::GetColumnsRowsetProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetColumnsRowsetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid	s_CComboInterface;
	static CPropSets		s_CPropSets;
	
	static BOOL fUseProps		= TRUE;		//Default
	static bool fOptColumns		= TRUE;		//Default
	static BOOL fAggregation	= FALSE;	//Default
	static BOOL fOutput			= TRUE;		//Default

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);

			//Use Optional Columns
			::CheckDlgButton(hWnd, IDB_USEOPTCOLUMNS,	BST2STATE(fOptColumns));
			//Use Properties
			::CheckDlgButton(hWnd, IDB_USEPROPERTIES,	BST2STATE(fUseProps));

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Interface List...
			s_CComboInterface.CreateIndirect(hWnd, IDC_INTERFACE);
			s_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
			if(s_CComboInterface.RestoreSelection() == CB_ERR)
				s_CComboInterface.SetGuid(IID_IRowset);

			//Only set these "Default" properties, if requested by the user
			if(pThis->GetOptions()->m_dwRowsetOpts & ROWSET_SETDEFAULTPROPS)
			{
				//DBPROP_CANHOLDROWS is required by the OLE DB Spec - Level-0 Conformance
				//Since it is also legal to set a ReadOnly property, just blindy set it...
				s_CPropSets.SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBTYPE_BOOL, (void*)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);
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
				case IDB_SETPROPERTIES:
				{
					//Get the "this" pointer
					CMDIChild* pThis			= (CMDIChild*)GetThis(hWnd);
					CMainWindow* pCMainWindow	= pThis->m_pCMainWindow;
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pThis->m_pCSource, CDataSource);
					
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &s_CPropSets);
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
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CDataAccess* pCDataAccess = SOURCE_GETOBJECT(pThis->m_pCSource, CDataAccess);

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

					//Use Optional Columns
					fOptColumns = ::IsDlgButtonChecked(hWnd, IDB_USEOPTCOLUMNS) ? true : false;
					
					//IColumnsRowset::GetColumnsRowset
					hr = pCDataAccess->GetColumnsRowset(pCAggregate, fOptColumns, riid, cPropSets, rgPropSets, fOutput ? &pIUnknown : NULL);

					if(SUCCEEDED(hr))
					{
						//Process the Rowset
						hr = pThis->HandleRowset(pCDataAccess, pIUnknown, riid, CREATE_NEWWINDOW_IFEXISTS, IID_IColumnsRowset);
					}


					SAFE_RELEASE(pCAggregate);
					SAFE_RELEASE(pIUnknown);

					//Have to exit, even if failure.  The failure can happen at any
					//point, ie: during accessor creation.  So we end up with a 
					//valid rowset, but not neccessaryliy something that supports 
					//IColumnsRowset...
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
// CMDIChild::GetChapteredChild
//
/////////////////////////////////////////////////////////////////
HRESULT CMDIChild::GetChapteredChild(INDEX iSelectedCol, const DBBINDING* pBinding, REFIID riid)
{
	ASSERT(pBinding);
	IUnknown* pIUnknown = NULL;
	HRESULT hr = S_OK;
	CRowset*	pCRowset = (CRowset*)GetObject(eCRowset);
	HROW hRow = NULL;
	CBase* pCObject = NULL;
	INDEX iSelRow = 0;

	WCHAR wszChapter[POINTER_DISPLAYSIZE] = {0};
	HCHAPTER hChapter = NULL;

	if(pCRowset && pCRowset->m_pIRowsetInfo)
	{
		//IRowsetInfo::GetReferencedRowset
		XTEST(hr = pCRowset->m_pIRowsetInfo->GetReferencedRowset(pBinding->iOrdinal, riid, &pIUnknown));
		TESTC(TRACE_METHOD(hr, L"IRowsetInfo::GetReferencedRowset(%Id, %s, &0x%p)", pBinding->iOrdinal, GetInterfaceName(riid), pIUnknown));
		
		//Obtain the hChapter from the rowset for the selected row.  If a row is not selected this
		//would be to open the entire child rowset.  If the bookmark column is used it useally
		//means to return the current rowset - again.  Both cases theres no chapter specified...
		iSelRow = m_pCDataGrid->GetSelectedRow(&hRow, FALSE);
		if(iSelRow != LVM_ERR && pBinding->iOrdinal!=0)
		{
			if(hRow)
			{
				TESTC(hr = pCRowset->GetChapter(hRow, pBinding->iOrdinal, &hChapter));
				StringFormat(wszChapter, NUMELE(wszChapter), L"%p", hChapter);
			}
			else
			{
				//Get the Item from the 'insert' ListView
				m_pCDataGrid->GetItemText(iSelRow, iSelectedCol+1, wszChapter, NUMELE(wszChapter));
				hChapter = wcstoul(wszChapter, NULL, 16);
			}
		}
		
		//Create a new Rowset Window for the result...
		//But dont actually display the object, until we set the hChapter
		//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
		pCObject = m_pCMainWindow->HandleObjectType(pCRowset, pIUnknown, riid, eCRowset, 0, NULL, CREATE_NEWWINDOW | CREATE_NODISPLAY | CREATE_DETERMINE_TYPE);
		if(pCObject)
		{
			//Set the Chapter handle, before displaying rowset...
			CRowset* pCRowset = SOURCE_GETOBJECT(pCObject, CRowset);
			if(pCRowset)
			{
				pCRowset->m_hChapter = hChapter;
				pCRowset->SetObjectDesc(wszChapter);
			}
			pCObject->DisplayObject();
		}
		else
		{
			TESTC(hr = E_FAIL);
		}
	}

CLEANUP:
	TRACE_RELEASE(pIUnknown, L"Rowset");
	return hr;
}

					
////////////////////////////////////////////////////////////////
// CMDIChild::GetReferencedRowsetProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::GetReferencedRowsetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CComboBoxGuid	s_CComboInterface;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CRowset* pCRowset = SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);

			INDEX iSel = 0;
			HWND hWndColumn		= ::GetDlgItem(hWnd, IDC_COLUMN);

			//Column List...
			INDEX iSavedCol = CB_ERR;
			for(ULONG i=0; i<pCRowset->m_Bindings.GetCount(); i++)
			{
				//Column Ordinal - Column Name
				DBORDINAL iOrdinal = pCRowset->m_Bindings[i].iOrdinal;
				const DBCOLUMNINFO* pColInfo = pCRowset->m_ColumnInfo.GetOrdinal(iOrdinal);

				//SetDefault to first chaptered column...
				if(pColInfo->dwFlags & DBCOLUMNFLAGS_ISCHAPTER)
				{
					iSel = (INDEX)wSendMessageFmt(hWndColumn, CB_ADDSTRING,	0, L"%2Id - %s <ISCHAPTER>", iOrdinal, GetColName(pColInfo));
					if(iSavedCol == CB_ERR)
						iSavedCol = iSel;
				}
				else
				{
					iSel = (INDEX)wSendMessageFmt(hWndColumn, CB_ADDSTRING,	0, L"%2Id - %s", iOrdinal, GetColName(pColInfo));
				}

				//Save the iBinding in ther listview
				SendMessage(hWndColumn, CB_SETITEMDATA, iSel, (LPARAM)i);
			}

			//Set Default...
			SendMessage(hWndColumn, CB_SETCURSEL, iSavedCol < (INDEX)pCRowset->m_Bindings.GetCount() && iSavedCol!=CB_ERR ? iSavedCol : 0, 0);
			
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
				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CRowset* pCRowset = SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);

					HWND hWndColumn		= ::GetDlgItem(hWnd, IDC_COLUMN);
					HRESULT hr = S_OK;

					//Column
					INDEX iSavedCol = (INDEX)SendMessage(hWndColumn, CB_GETCURSEL, 0, 0);
					DBCOUNTITEM iBinding = SendMessage(hWndColumn, CB_GETITEMDATA, iSavedCol!=CB_ERR ? iSavedCol : 0, 0);
					const DBBINDING* pBinding = &pCRowset->m_Bindings[iBinding];

					//Interface
					REFIID riid = s_CComboInterface.GetGuid();
					
					//Obtain the Child rowset..
					hr = pThis->GetChapteredChild(iSavedCol, pBinding, riid);
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
// CMDIChild::FindNextRowProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::FindNextRowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static INDEX	iSavedCol		= 1;		//Default column
	static INDEX	iSavedOp		= 2;		//Default DBCOMPAREOPS_EQ
	static LONG		cSavedRows		= 1;		//Default to cRows=1
	static DWORD	dwCompareOps	= 0;		//Default to not CASESENSITIVE

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CRowset* pCRowset = SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);
			INDEX iSel = 0;
			
			HWND hWndColumn		= ::GetDlgItem(hWnd, IDC_COLUMN);
			HWND hWndCompareOp	= ::GetDlgItem(hWnd, IDC_COMPAREOP);
			HWND hWndcRows		= ::GetDlgItem(hWnd, IDE_COUNT);
			HWND hWndValue		= ::GetDlgItem(hWnd, IDE_VALUE);

			ULONG i;
			//Column List...
			for(i=0; i<pCRowset->m_Bindings.GetCount(); i++)
			{
				//Column Ordinal - Column Name
				DBORDINAL iOrdinal = pCRowset->m_Bindings[i].iOrdinal;
				const DBCOLUMNINFO* pColInfo = pCRowset->m_ColumnInfo.GetOrdinal(iOrdinal);
				iSel = (INDEX)wSendMessageFmt(hWndColumn, CB_ADDSTRING,	0, L"%2Id - %s", iOrdinal, GetColName(pColInfo));
				SendMessage(hWndColumn, CB_SETITEMDATA, iSel, (LPARAM)i);
			}

			//Set Default...
			SendMessage(hWndColumn, CB_SETCURSEL, iSavedCol < (INDEX)pCRowset->m_Bindings.GetCount() && iSavedCol!=CB_ERR ? iSavedCol : 0, 0);
			
			const static WIDENAMEMAP rgCompareOps[] = 
			{
				VALUE_WCHAR(DBCOMPAREOPS_LT),
				VALUE_WCHAR(DBCOMPAREOPS_LE),
				VALUE_WCHAR(DBCOMPAREOPS_EQ),
				VALUE_WCHAR(DBCOMPAREOPS_GE),
				VALUE_WCHAR(DBCOMPAREOPS_GT),
				VALUE_WCHAR(DBCOMPAREOPS_BEGINSWITH),
				VALUE_WCHAR(DBCOMPAREOPS_CONTAINS),
				VALUE_WCHAR(DBCOMPAREOPS_NE),
				VALUE_WCHAR(DBCOMPAREOPS_IGNORE),
				VALUE_WCHAR(DBCOMPAREOPS_NOTBEGINSWITH),
				VALUE_WCHAR(DBCOMPAREOPS_NOTCONTAINS),
			};
			
			//CompareOp List...
			for(i=0; i<NUMELE(rgCompareOps); i++)
			{
				iSel = (INDEX)wSendMessage(hWndCompareOp, CB_ADDSTRING,	0, rgCompareOps[i].pwszName);
				SendMessage(hWndCompareOp, CB_SETITEMDATA, iSel, (LPARAM)rgCompareOps[i].lItem);
			}

			//Set Default...
			iSavedOp = (INDEX)SendMessage(hWndCompareOp, CB_SETCURSEL, iSavedOp != CB_ERR ? iSavedOp : 0, 0);
			
			//dwModifiers
			//DBCOMPAREOPS_CASESENSITIVE
			//DBCOMPAREOPS_CASEINSENSITIVE
			::CheckDlgButton(hWnd, IDB_CASESENSITIVE,		BST2STATE(dwCompareOps & DBCOMPAREOPS_CASESENSITIVE));
			::CheckDlgButton(hWnd, IDB_CASEINSENSITIVE,		BST2STATE(dwCompareOps & DBCOMPAREOPS_CASEINSENSITIVE));

			//cRows
			wSendMessageFmt(hWndcRows, WM_SETTEXT, 0, L"%lu", cSavedRows);

			//Value
			SendMessage(hWndValue, WM_SETTEXT, 0, (LPARAM)"");

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
					CMDIChild* pThis = (CMDIChild*)GetThis(hWnd);
					CRowset* pCRowset = SOURCE_GETOBJECT(pThis->m_pCSource, CRowset);
					HRESULT hr = S_OK;
					
					HWND hWndColumn		= ::GetDlgItem(hWnd, IDC_COLUMN);
					HWND hWndCompareOp	= ::GetDlgItem(hWnd, IDC_COMPAREOP);
					HWND hWndcRows		= ::GetDlgItem(hWnd, IDE_COUNT);
					HWND hWndValue		= ::GetDlgItem(hWnd, IDE_VALUE);

					HACCESSOR hAccessor = NULL;

					//Bookmark
					DBBKMARK cbBookmark = 0;
					BYTE* pBookmark = NULL;
					
					HROW hRow = NULL;
					void* pData = pCRowset->m_pData;
					DBROWOFFSET lOffset = 0;
					DBCOUNTITEM cRowsObtained = 0;
					HROW* rghRows = NULL;
					HCHAPTER hChapter = pCRowset->m_hChapter;
					
					//Column
					iSavedCol = (INDEX)SendMessage(hWndColumn, CB_GETCURSEL, 0, 0);
					DBCOUNTITEM iBinding = SendMessage(hWndColumn, CB_GETITEMDATA, iSavedCol!=CB_ERR ? iSavedCol : 0, 0);
					const DBBINDING* pBinding		= &pCRowset->m_Bindings[iBinding];
					const DBCOLUMNINFO* pColInfo	= pCRowset->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);
					 
					//CompareOp
					iSavedOp = (INDEX)SendMessage(hWndCompareOp, CB_GETCURSEL, 0, 0);
					dwCompareOps = (DBCOMPAREOP)SendMessage(hWndCompareOp, CB_GETITEMDATA, iSavedOp != CB_ERR ? iSavedOp : 0, 0);
					
					//dwModifiers
					if(::IsDlgButtonChecked(hWnd, IDB_CASESENSITIVE))
						dwCompareOps |= DBCOMPAREOPS_CASESENSITIVE;
					if(::IsDlgButtonChecked(hWnd, IDB_CASEINSENSITIVE))
						dwCompareOps |= DBCOMPAREOPS_CASEINSENSITIVE;

					//cRows
					GetEditBoxValue(hWndcRows, &cSavedRows);
					
					//Value
					WCHAR* pwszValue = wGetWindowText(hWndValue);
					
					//Create An Accessor binding only this column
					TESTC(hr = pCRowset->CreateAccessor(DBACCESSOR_ROWDATA, 1, pBinding, 0, &hAccessor));

					//Setup the pData buffer containg the users value
					TESTC(hr = pCRowset->SetColumnData(pBinding, pData, DBSTATUS_S_OK, pwszValue ? wcslen(pwszValue) : 0, pwszValue ? pwszValue : L"", CONV_NONE, pColInfo->wType));

					//Obtain the bookmark(s) for the selected row(s)...
					if(pThis->m_pCDataGrid->GetSelectedRow(&hRow, FALSE)!=LVM_ERR)
						hr = pCRowset->GetBookmark(hRow, &cbBookmark, (BYTE**)&pBookmark);

					//Release previously fetched rows, (if user requested)
					TESTC(hr = pThis->m_pCDataGrid->ReleaseHeldRows());

					//IRowsetFind::FindNextRow
					XTEST(hr = pCRowset->m_pIRowsetFind->FindNextRow(hChapter, hAccessor, pData, dwCompareOps, cbBookmark, pBookmark, lOffset, cSavedRows, &cRowsObtained, &rghRows));
					TESTC(TRACE_METHOD(hr, L"IRowsetFind::FindNextRow(0x%p, 0x%p, \"%s\", 0x%08x, %Iu, 0x%p, %Id, %ld, &%Iu, &0x%p)", hChapter, hAccessor, pwszValue, dwCompareOps, cbBookmark, pBookmark, lOffset, cSavedRows,  cRowsObtained, rghRows));

				CLEANUP:
					//TODO - need to free in the case of errors as well.  But have to be careful
					//since the pData may not have been setup yet, or other errors where its undefined...
					if(SUCCEEDED(hr))
						FreeBindingData(1, pBinding, pData, TRUE/*SetData*/);
					
					pCRowset->ReleaseAccessor(&hAccessor);
					SAFE_FREE(pBookmark);
					SAFE_FREE(rghRows);
					SAFE_FREE(pwszValue);

					if(SUCCEEDED(hr))
					{
						EndDialog(hWnd, TRUE);
						return 0;
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
// CMDIChild::ProviderInfoProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CMDIChild::ProviderInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{	
			CWaitCursor waitCursor;

			CMDIChild* pThis = (CMDIChild*)SetThis(hWnd, (void*)lParam);
			CComBSTR cstrString;
			CComBSTR cstrString2;
			DWORD dwValue = 0;

			//Determine which object were on...
			IDBProperties* pIDBProperties = SOURCE_GETINTERFACE(pThis->m_pCSource, IDBProperties);
			if(pIDBProperties)
			{
				//DBPROP_PROVIDERFILENAME
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_PROVIDERFILENAME,		DBPROPSET_DATASOURCEINFO, DBTYPE_BSTR, &cstrString);
				wSendMessage(::GetDlgItem(hWnd, IDT_PROVIDERFILENAME), WM_SETTEXT, 0, cstrString ? cstrString : L"");
				cstrString.Empty();

				//DBPROP_PROVIDERFRIENDLYNAME
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_PROVIDERFRIENDLYNAME,	DBPROPSET_DATASOURCEINFO, DBTYPE_BSTR, &cstrString);
				wSendMessage(::GetDlgItem(hWnd, IDT_PROVIDERDESC), WM_SETTEXT, 0, cstrString ? cstrString : L"");
				cstrString.Empty();
				
				//DBPROP_PROVIDERVER
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_PROVIDERVER,	DBPROPSET_DATASOURCEINFO, DBTYPE_BSTR, &cstrString);
				//DBPROP_PROVIDEROLEDBVER
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_PROVIDEROLEDBVER,	DBPROPSET_DATASOURCEINFO, DBTYPE_BSTR, &cstrString2);
				wSendMessageFmt(::GetDlgItem(hWnd, IDT_PROVIDERVERSION), WM_SETTEXT, 0, L"Prov - %s   OLEDB - %s", cstrString ? cstrString : L"Unknown", cstrString2 ? cstrString2 : L"Unknown");
				cstrString.Empty();
				cstrString2.Empty();

				//DBMS / DBMSVER
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_DBMSNAME,	DBPROPSET_DATASOURCEINFO, DBTYPE_BSTR, &cstrString);
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_DBMSVER,	DBPROPSET_DATASOURCEINFO, DBTYPE_BSTR, &cstrString2);
				wSendMessageFmt(::GetDlgItem(hWnd, IDT_DBMS), WM_SETTEXT, 0, L"%s %s", cstrString ? cstrString : L"", cstrString2 ? cstrString2 : L"");
				cstrString.Empty();
				cstrString2.Empty();
				
				//DATASOURCE
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_DATASOURCENAME, DBPROPSET_DATASOURCEINFO, DBTYPE_BSTR, &cstrString);
				wSendMessage(::GetDlgItem(hWnd, IDT_DATASOURCE), WM_SETTEXT, 0, cstrString ? cstrString : L"");
				cstrString.Empty();

				//DBPROP_DATASOURCEREADONLY
				VARIANT_BOOL bReadOnly = VARIANT_FALSE;
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, DBTYPE_BOOL, &bReadOnly);
				SendMessage(::GetDlgItem(hWnd, IDT_READONLY), WM_SETTEXT, 0, bReadOnly ? (LPARAM)"Read-Only" : (LPARAM)"Updatable");

				//DBPROP_CURRENTCATALOG
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_CURRENTCATALOG,		DBPROPSET_DATASOURCE, DBTYPE_BSTR, &cstrString);
				wSendMessage(::GetDlgItem(hWnd, IDT_CATALOG), WM_SETTEXT, 0, cstrString);
				cstrString.Empty();

				//DBPROP_DSOTHREADMODEL
				GetProperty(IID_IDBProperties, pIDBProperties, DBPROP_DSOTHREADMODEL,		DBPROPSET_DATASOURCEINFO, DBTYPE_I4, &dwValue);
				SendMessage(::GetDlgItem(hWnd, IDT_THREADMODEL), WM_SETTEXT, 0, (LPARAM)(dwValue & DBPROPVAL_RT_FREETHREAD ? "FreeThreaded" : dwValue & DBPROPVAL_RT_APTMTTHREAD ? "ApartmentModel" : dwValue & DBPROPVAL_RT_SINGLETHREAD ? "SingleThreaded" : "Unknown"));
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
				case IDCANCEL:
				{
					CWaitCursor waitCursor;
					EndDialog(hWnd, TRUE);
					return TRUE;
				}
			}
			break;
		}//WM_COMMAND
	}
	
	return FALSE;   
}





////////////////////////////////////////////////////////////////
// CQueryBox::CQueryBox
//
/////////////////////////////////////////////////////////////////
CQueryBox::CQueryBox(CMDIChild* pCMDIChild)
{
	ASSERT(pCMDIChild);
	m_pCMDIChild = pCMDIChild;
}


////////////////////////////////////////////////////////////////
// CQueryBox::~CQueryBox
//
/////////////////////////////////////////////////////////////////
CQueryBox::~CQueryBox()
{
}


////////////////////////////////////////////////////////////////
// CQueryBox::OnRButtonDown
//
/////////////////////////////////////////////////////////////////
BOOL CQueryBox::OnRButtonDown(WPARAM fwKeys, REFPOINTS pts)
{
	//NOTE: The right mouse button doesn't automatically activate the MDI window...
	m_pCMDIChild->m_pCMainWindow->MDIActivate(m_hWndParent);

	//xPos, yPos are Relative to the Client Area...
	DisplayContextMenu( 
						m_hWnd,
						IDM_EDITMENU, 
						pts,
						m_pCMDIChild->m_hWnd,
						TRUE
						);
	return TRUE;
}

////////////////////////////////////////////////////////////////
// CQueryBox::OnContextMenu
//
/////////////////////////////////////////////////////////////////
BOOL CQueryBox::OnContextMenu(HWND hWnd, REFPOINTS pts)
{
	DisplayContextMenu( 
						hWnd,
						IDM_EDITMENU, 
						pts,
						m_pCMDIChild->m_hWnd
						);
	return  TRUE;
}


////////////////////////////////////////////////////////////////
// CQueryBox::OnKeyDown
//
/////////////////////////////////////////////////////////////////
BOOL CQueryBox::OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData)
{
	switch(nVirtKey)
	{
		case VK_TAB:
		{
			MSG msg;
			m_pCMDIChild->m_pCDataGrid->SetFocus();
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			return TRUE;
		}
	};

	return FALSE;
}




////////////////////////////////////////////////////////////////
// CDataGrid::CDataGrid
//
/////////////////////////////////////////////////////////////////
CDataGrid::CDataGrid(CMDIChild* pCMDIChild)
{
	ASSERT(pCMDIChild);
	
	//Data
	m_pCMDIChild			= pCMDIChild;
	m_lMaxRows				= 0;

	//Cursor
	m_fLastFetchForward		= FALSE;
	m_lCurPos				= 0;
}

////////////////////////////////////////////////////////////////
// CDataGrid::~CDataGrid
//
/////////////////////////////////////////////////////////////////
CDataGrid::~CDataGrid()
{
}


////////////////////////////////////////////////////////////////
// CDataGrid::GetOptions
//
/////////////////////////////////////////////////////////////////
COptionsSheet*	CDataGrid::GetOptions()
{
	return m_pCMDIChild->GetOptions();
}


/////////////////////////////////////////////////////////////////////
// CDataGrid::OnSize
//
/////////////////////////////////////////////////////////////////////
BOOL CDataGrid::OnSize(WPARAM nType, REFPOINTS pts)
{
	switch(nType)
	{
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
		{
			if(pts.x && pts.y)
			{
				//Obtain the total number of available items (subtract for ScrollBar)
 				m_lMaxRows = GetCountPerPage()-1;
				m_lMaxRows = max(m_lMaxRows, 0);	//At least 0
			}

			return FALSE;
		}
	};

	return FALSE;
}                


////////////////////////////////////////////////////////////////
// CDataGrid::OnRButtonDown
//
/////////////////////////////////////////////////////////////////
BOOL CDataGrid::OnRButtonDown(WPARAM fwKeys, REFPOINTS pts)
{
	//NOTE: The right mouse button doesn't automatically activate the MDI window...
	CDataAccess* pCDataAccess = m_pCMDIChild->m_pCDataAccess;

	//xPos, yPos are Relative to the Client Area...
	if(pCDataAccess)
	{
		DisplayContextMenu( 
						m_hWnd,
						pCDataAccess->GetObjectMenu(), 
						pts,
						m_pCMDIChild->m_pCMainWindow->m_hWnd,
						TRUE
						);
		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CDataGrid::OnContextMenu
//
/////////////////////////////////////////////////////////////////
BOOL CDataGrid::OnContextMenu(HWND hWnd, REFPOINTS pts)
{
	CDataAccess* pCDataAccess = m_pCMDIChild->m_pCDataAccess;
	
	if(pCDataAccess)
	{
		DisplayContextMenu( 
						m_hWnd,
						pCDataAccess->GetObjectMenu(), 
						pts,
						m_pCMDIChild->m_pCMainWindow->m_hWnd
						);
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CDataGrid::OnColumnClick
//
/////////////////////////////////////////////////////////////////////
BOOL CDataGrid::OnColumnClick(INT idCtrl, NMLISTVIEW* pNMListView)
{
	//Can't edit "Row-Handle" Column
	if(pNMListView->iSubItem == 0)
		return TRUE;

	CWaitCursor waitCursor;

	//Save the SelectedCol so the new dialog box knows which Column were concerned with...
	m_iSelCol = pNMListView->iSubItem-1;
	CRowset* pCRowset = (CRowset*)m_pCMDIChild->GetObject(eCRowset);

	//See if the selected column is a chapter column...
	if(pCRowset)
	{
		if((DBCOUNTITEM)m_iSelCol < pCRowset->m_Bindings.GetCount())
		{
			const DBBINDING* pBinding = &pCRowset->m_Bindings[m_iSelCol];
			const DBCOLUMNINFO* pColInfo = pCRowset->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);
			if(pColInfo->dwFlags & DBCOLUMNFLAGS_ISCHAPTER)
			{
				//Obtain the Child rowset..
				m_pCMDIChild->GetChapteredChild(m_iSelCol, pBinding);
				return TRUE;
			}
		}
	}

	//Otherwise we will assume the user wants to "edit" the column...
	if(GetItemCount())
	{
		CRow* pCRow			= (CRow*)m_pCMDIChild->GetObject(eCRow);
		CDataset* pCDataset	= (CDataset*)m_pCMDIChild->GetObject(eCDataset);
		CRowset* pCRowset	= (CRowset*)m_pCMDIChild->GetObject(eCRowset);

		//Determine the Object Type...
		if(pCRow)
			m_pCMDIChild->DisplayDialog(IDD_ROWCHANGE, m_hWnd, CMDIChild::ColumnChangeProc, pCRow, IDM_IROW_SETCOLUMNS);
		else if(pCDataset)
			m_pCMDIChild->DisplayDialog(IDD_ROWCHANGE, m_hWnd, CMDIChild::ColumnChangeProc, pCDataset, IDM_DATASET_GETCELLDATA);
		else if(pCRowset)
			m_pCMDIChild->DisplayDialog(IDD_ROWCHANGE, m_hWnd, CMDIChild::ColumnChangeProc, pCRowset, IDM_SETDATA);
	}

	return TRUE;	
}


/////////////////////////////////////////////////////////////////////
// CDataGrid::OnItemActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CDataGrid::OnItemActivate(INT idCtrl, NMLISTVIEW* pNMListView)
{
	CDataAccess* pCDataAccess = m_pCMDIChild->m_pCDataAccess;
	CRow* pCRow			= SOURCE_GETOBJECT(pCDataAccess, CRow);
	CDataset* pCDataset	= SOURCE_GETOBJECT(pCDataAccess, CDataset);
	INDEX iSelCol		= pNMListView->iSubItem-1;

	//If this is a row object, then bring up the SetColumns dialog...
	if(pCRow)
	{
		//IRowChange::SetColumns
		m_pCMDIChild->ChangeSelectedRow(pCRow, IDM_IROW_SETCOLUMNS);
	}
	else if(pCDataset)
	{
		//IMDDataset::GetCellData
		m_pCMDIChild->ChangeSelectedRow(pCDataset, IDM_DATASET_GETCELLDATA);
	}
	else
	{
		CRowset* pCRowset =  SOURCE_GETOBJECT(pCDataAccess, CRowset);
		if(pCRowset)
		{
			//If the parent object is an enumerator, then Double Clicking on the row
			//means you wish to "drill" into that row of the enumerator and call ParseDisplayName...
			if(pCRowset->m_pCParent && pCRowset->m_pCParent->GetObjectType() == eCEnumerator)
			{
				//If enumertor rowset, double-clicking on a row
				//brings up the ParseName object in another window
				CWaitCursor waitCursor;
				m_pCMDIChild->CreateEnumChild();
				return TRUE;
			}

			//See if the selected column is a chapter column...
			if((DBCOUNTITEM)iSelCol < pCRowset->m_Bindings.GetCount())
			{
				const DBBINDING* pBinding = &pCRowset->m_Bindings[iSelCol];
				const DBCOLUMNINFO* pColInfo = pCRowset->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);
				if(pColInfo->dwFlags & DBCOLUMNFLAGS_ISCHAPTER)
				{
					//Obtain the Child rowset..
					m_pCMDIChild->GetChapteredChild(iSelCol, pBinding);
					return TRUE;
				}
			}
			
			//Otherwise we just have a "normal" rowset...
			HROW hRow;
			if(GetSelectedRow(&hRow, FALSE) != LVM_ERR)
			{
				CWaitCursor waitCursor;
				IUnknown* pIUnknown = NULL;

				//So either we are drilling into a row object from the rowset, or 
				//were are trying to edit the actual data of the row handle
				if(SUCCEEDED(pCRowset->GetRowFromHROW(NULL, hRow, IID_IRow, &pIUnknown)))
				{
					//Create in new window
					CBase* pCObject = m_pCMDIChild->m_pCMainWindow->HandleObjectType(pCRowset, pIUnknown, IID_IRow, eCRow, 0, NULL, CREATE_NEWWINDOW | CREATE_NODISPLAY);
					if(pCObject)
					{
						//Fill in which row handle this row was created from...
						//Just for display purposes...
						CRow* pCRow = SOURCE_GETOBJECT(pCObject, CRow);
						if(pCRow)
							pCRow->m_hSourceRow = hRow;
													
						pCObject->DisplayObject();
					}
				}
				else
				{
					//Otherwise were just trying to update or view detailed info...
					m_pCMDIChild->ChangeSelectedRow(pCRowset, IDM_SETDATA);
				}
				
				SAFE_RELEASE(pIUnknown);
			}
		}
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////
// CDataGrid::OnKeyDown
//
/////////////////////////////////////////////////////////////////
BOOL CDataGrid::OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData)
{
	switch(nVirtKey)
	{
		case VK_DELETE:
		{
			PostMessage(m_pCMDIChild->m_hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_DELETEROWS, 0, 0));
			return TRUE;
		}

		case VK_INSERT:
		{
			PostMessage(m_pCMDIChild->m_hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_INSERTROW, 0, 0));
			return TRUE;
		}

		case VK_UP:
			ScrollGrid(-1);
			return TRUE;

		case VK_DOWN:
			ScrollGrid(1);
			return TRUE;

		case VK_PRIOR:
			ScrollGrid(-m_lMaxRows);
			return TRUE;

		case VK_NEXT:
			ScrollGrid(m_lMaxRows);
			return TRUE;

		case VK_TAB:
		{
			MSG msg;
			m_pCMDIChild->m_pCQueryBox->SetFocus();
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			return TRUE;
		}
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CDataGrid::OnVScroll
//
/////////////////////////////////////////////////////////////////////
BOOL CDataGrid::OnVScroll(int nScrollCode, int nPos, HWND hWnd)
{
	switch(nScrollCode)
	{
		case SB_LINEUP:
			ScrollGrid(-1);
			break;

		case SB_LINEDOWN:
			ScrollGrid(1);
			break;

		case SB_PAGEUP:
			ScrollGrid(-m_lMaxRows);
			break;

		case SB_PAGEDOWN:
			ScrollGrid(m_lMaxRows);
			break;
	};

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CDataGrid::DisplayColumnInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::DisplayColumnInfo()
{
	INDEX	iResult = 0;
	CDataAccess* pCDataAccess = m_pCMDIChild->m_pCDataAccess;

	//Insert the Column Headers.
	//Only display the columns for which are bound in the Accessor
	
	//Insert Row Header
	InsertColumn(0, pCDataAccess->GetObjectType() == eCDataset ? L"  Cell Ordinal  " : L"  Row Handle  ");

	ULONG i;
	//Loop through the columns
	for(i=0; i<pCDataAccess->m_Bindings.GetCount(); i++)
	{
		const DBCOLUMNINFO* pColInfo = pCDataAccess->m_ColumnInfo.GetOrdinal(pCDataAccess->m_Bindings[i].iOrdinal);

		//Get ColumnName
		iResult = InsertColumn(i+1, GetColName(pColInfo), pCDataAccess->GetColumnImage(pColInfo));
	}

	//AutoSize Columns, (including "Row-Handle" column)
	for(i=0; i<pCDataAccess->m_Bindings.GetCount()+1; i++)
		SetColumnWidth(i,	LVSCW_AUTOSIZE_USEHEADER);

	//CLEANUP:    
    return iResult == LVM_ERR ? E_FAIL : S_OK;    
}


////////////////////////////////////////////////////////////////
// CDataGrid::RefreshData
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::RefreshData()
{
	CWaitCursor waitCursor;
	HRESULT hr = S_OK;
	CDataAccess* pCDataAccess	= m_pCMDIChild->m_pCDataAccess;
	CRow* pCRow					= SOURCE_GETOBJECT(pCDataAccess, CRow);
	CRowset* pCRowset			= SOURCE_GETOBJECT(pCDataAccess, CRowset);
	DWORD dwRowsetOpts			= GetOptions()->m_dwRowsetOpts;

	//Clear ListView object
	ClearAll();

	//Display the Column Headers
	//TODO: Would be nice to optimize this to not require a complete dump of all
	//columns and redisplay, need to split out between create accessor and refresh...
	TESTC(hr = DisplayColumnInfo());

	//Row Object?
	if(pCRow)
	{
		//Insert an empty row into the ListView
		InsertItem(0, 0, L"", 0, IMAGE_NORMAL);
		
		//Display the row data
		TESTC(hr = DisplayData(pCRow->m_hSourceRow, 0, DBPROP_IRow));
	}
	else
	{
		//Determine the number of rows the user wants (initially)
		//Always obtain more rows than what the grid allows so we can get the scroll bar
		//visable.  This way they can use the scroll bar up/dn to populate more rows
		DBROWCOUNT cRowsToFetch = m_lMaxRows+3;
		if(GetOptions()->m_dwRowsetOpts & ROWSET_ROWSTOFETCH)
			cRowsToFetch = GetOptions()->m_cRowsToFetch;

		if(cRowsToFetch)
			TESTC(hr = GetNextRows(0, cRowsToFetch, TRUE/*fRetry*/));
	}

CLEANUP:
	m_pCMDIChild->UpdateControls();
	return hr;
}


////////////////////////////////////////////////////////////////
// CDataGrid::DisplayData
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::DisplayData
(
	HROW			hRow,
	INDEX			iIndex,
	DBPROPID		dwSourceID,
	bool			fAlways
)
{
	ASSERT(iIndex >=0 && iIndex<ULONG_MAX);
	HRESULT hr = E_FAIL;
	CDataAccess* pCDataAccess = m_pCMDIChild->m_pCDataAccess;
	CRow*		 pCRow		  = SOURCE_GETOBJECT(pCDataAccess, CRow);
	CRowset*	 pCRowset	  = SOURCE_GETOBJECT(pCDataAccess, CRowset);
	CDataset*	 pCDataset	  = SOURCE_GETOBJECT(pCDataAccess, CDataset);
	void* pData = pCDataAccess->m_pData;

	DBSTATUS dbStatus = 0;
	DWORD	i,dwConvFlags	= GetOptions()->m_dwConvFlags; 
	WCHAR	wszBuffer[MAX_COL_SIZE+1] = {0};

	//NOTE: We obtain/display the data - unless the users wishes to not obtain
	//the data by default, or this is called from a method that should always obtain data (ie: GetData)
	if(fAlways || !(GetOptions()->m_dwRowsetOpts & ROWSET_NODATA))
	{
		//This displays data from any source.
		if(pCRow && dwSourceID == DBPROP_IRow)
		{
			//IRow::GetColumns
			hr = pCRow->GetColumns(pCRow->m_cColAccess, pCRow->m_rgColAccess);
		}
		else if(pCRowset)
		{
			//IRowset::GetData
			hr = pCRowset->GetData(hRow, NULL, pData, dwSourceID);
		}
		else if(pCDataset)
		{
			hRow = iIndex;

			//IMDDataset::GetCellData
			hr = pCDataset->GetCellData(hRow, hRow);
		}

		//Relax the errors here.  With Get*Data both DB_S/DB_E will return a bad
		//status for 1 or more columns.  Or DataGrid will just display the column
		//status if not S_OK.  So for these errors just display the data anyway...
		if(SUCCEEDED(hr) || hr== DB_E_ERRORSOCCURRED)
		{
			//Display all Columns
			for(i=0; i<pCDataAccess->m_Bindings.GetCount(); i++)
			{
				const DBBINDING* pBinding		= &pCDataAccess->m_Bindings[i];
				const DBCOLUMNINFO* pColInfo	= pCDataAccess->m_ColumnInfo.GetOrdinal(pBinding->iOrdinal);

				//Obtain the column data...
				pCDataAccess->GetColumnData(pBinding, pData, &dbStatus, NULL/*pdbLength*/, NULL, wszBuffer, MAX_COL_SIZE, dwConvFlags | CONV_TYPENAME, pColInfo->wType);
			
				//Set item in ListView
	   			SetItemText(iIndex, i+1, wszBuffer);
				SetItemImage(iIndex, i+1, pCDataAccess->GetColumnImage(NULL, dbStatus));
			}

			//Clean outofline memory data
			//TODO - need to free in the case of errors as well.  But have to be careful
			//since the pData may not have been setup yet, or other errors where its undefined...
			pCDataAccess->m_Bindings.FreeData(pData);
		}
	}

	//Now add the "hRow" indicator
	if(hRow)
		StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%p", hRow);
	else
		StringFormat(wszBuffer, NUMELE(wszBuffer), L"No Handle");
	SetItemText(iIndex, 0, wszBuffer);
	SetItemParam(iIndex, 0, hRow);
    return hr;
}


////////////////////////////////////////////////////////////////
// CDataGrid::DisplayRows
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::DisplayRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM cRowsObtained, HROW* rghRows, BOOL fAdjustFetchPosition)
{
	HRESULT hr = S_OK;
	DBROWOFFSET lStartIndex = lOffset;
	BOOL fLastFetchForward	= FALSE;

	//Obtain total ListItems
	INDEX cItems = GetItemCount();
	ASSERT(cItems >= 0);

	//	Since RowsetViewer displays the abstract "NextFetchPosition" to
	//	the user and we need to make sure we put the correct rows obtained into
	//	the correct indexs, we need to map the OLE DB Fetch Position into an index
	//	based method for our listview.
	//
	//	The OLE DB Cursor (NextFetchPosition) is always "in-between" rows
	//	Meaning its either before or after the row, (not actually on the row).
	//	For example: If the NFP is before row 2, and cRows=1, then row 2 will be
	//	retrieved, but if it was after then row 3 is retrieved.  Likewise if
	//	NFP is before row and cRows=-1, then row 1 is retrived, but if it was
	//	after then row 2 is retrieved.  Doing a Forward fetch (cRows++) makes 
	//	the NFP after that row, and a Backward fetch (cRows--) makes the 
	//	NFP appear before.

	//	+---------------+---------------+---------------------------|-----------------------------------+
	//  | LastFetch		|	cRows		|	StartIndex				|	EndIndex						|
	//	+---------------+---------------+---------------------------+-----------------------------------+
	//	|#1	Forward		|	Positive	|	CurIndex + Offset + 1	|	StartIndex + cRowsObtained - 1	|
	//	|#2	Forward		|	Negative	|	CurIndex + Offset		|	StartIndex - cRowsObtained + 1	|
	//	|#3	Backward	|	Positive	|	CurIndex + Offset		|	StartIndex + cRowsObtained - 1	|
	//	|#4	Backward	|	Negative	|	CurIndex + Offset - 1	|	StartIndex - cRowsObtained + 1	|
	//	+---------------+---------------+---------------------------+-----------------------------------+

	//Calculate the StartIndex (using the Table Above)
	if(fAdjustFetchPosition)
	{
		//#1, #2, #3, #4 (Common Case) is just CurIndex + Offset
		lStartIndex = m_lCurPos + lOffset;
		fLastFetchForward = m_fLastFetchForward;

		//#1
		if(fLastFetchForward && cRows > 0)
			lStartIndex += 1;
		//#4
		if(!fLastFetchForward && cRows < 0)
			lStartIndex -= 1;
	}

	//Determine Ending Index
	DBROWOFFSET lEndIndex	= lStartIndex;
	if(cRowsObtained)
	{
		if(cRows > 0)
		{
			//#1, #3
			lEndIndex =	lStartIndex + cRowsObtained - 1;
			fLastFetchForward = TRUE;
		}
		else
		{
			//#2, #4
			lEndIndex =	lStartIndex - cRowsObtained + 1;
			fLastFetchForward = FALSE;
		}

		//Remove Previous Fetch Icon
		if(fAdjustFetchPosition)
			SetItemImage((INDEX)m_lCurPos, 0, IMAGE_NORMAL);

		//Remove Previous Selection Bars
		for(INDEX iIndex=0; iIndex<cItems; iIndex++)
			SetItemState(iIndex, 0, ~LVIS_SELECTED, LVIS_SELECTED);
	}

	//Loop over rows obtained, displaying/updating in the ListView
	for(DBCOUNTITEM iRow=0; iRow<cRowsObtained; iRow++)
	{
		ASSERT(rghRows);
		
		//Calulate which direction to start with
		HROW hRow = rghRows[iRow];
		INDEX iIndex = (INDEX)(cRows > 0 ? lStartIndex + iRow : lStartIndex - iRow);

		//We may need extra row padding, if fetched out of sequence...
		while(iIndex >= cItems)
		{
			InsertItem(iIndex, 0, L"", 0, IMAGE_NORMAL);
			cItems++;
		}

		//We may need to insert rows before the beginging if scrolling
		//around the rowset...
		while(iIndex < 0)
		{
			InsertItem(0, 0, L"", 0, IMAGE_NORMAL);
			cItems++;

			//A row is being inserted, meaning this shifts all our
			//current indexes by one...
			lStartIndex++;
			lEndIndex++;
			iIndex++;
			m_lCurPos++;
   		}

		//Display GetData in the ListView
		hr = DisplayData(hRow, iIndex);
	}

	//Scroll the new rows into view...
	if(cRowsObtained)
	{
		//Adjust our saved Fetched Position
		if(fAdjustFetchPosition)
		{
			//Display the direction of the cursor
			DisplayFetchPosition(lEndIndex, fLastFetchForward);
		}

		//Make sure the item is visible
		EnsureVisible(lEndIndex);
	}

	return hr;
}


////////////////////////////////////////////////////////////////
// CDataGrid::DisplayFetchPosition
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::DisplayFetchPosition(INDEX iIndex, BOOL fLastFetchForward)
{
	//Display the direction of the cursor
	SetItemImage(iIndex, 0, fLastFetchForward ? IMAGE_ARROW_DOWN : IMAGE_ARROW_UP);

	//Display the new Selection Bar for this row
	SetItemState(iIndex, 0, LVIS_SELECTED, LVIS_SELECTED);

	//Update our saved cursor position
	m_lCurPos			= iIndex;
	m_fLastFetchForward = fLastFetchForward;
	
	//Make sure the item is visible
	EnsureVisible(iIndex);
	return S_OK;
}

	
////////////////////////////////////////////////////////////////
// CDataGrid::RestartPosition
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::RestartPosition()
{
	CWaitCursor waitCursor;
	HRESULT hr = S_OK;
	CRowset*	pCRowset = (CRowset*)m_pCMDIChild->GetObject(eCRowset);

	if(pCRowset && pCRowset->m_pIRowset)
	{
		//Release previously fetched rows, (if user requested)
		TESTC(hr = ReleaseHeldRows());

		//RestartPosition...
		TESTC(hr = pCRowset->RestartPosition());
	}
	
	//Need to remove the "NextFetchPositon" icon from the "old" row
	SetItemImage((INDEX)m_lCurPos, 0, IMAGE_NORMAL);

	m_fLastFetchForward = FALSE;
	m_lCurPos = 0;

	//Need to show the "NextFetchPositon" icon 
	SetItemImage((INDEX)m_lCurPos, 0, IMAGE_ARROW_UP);
	EnsureVisible((INDEX)m_lCurPos);
	
CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CDataGrid::GetNextRows
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, BOOL fRetry)
{
	HRESULT hr		= S_OK;
	HRESULT hrGNR	= S_OK;
	CDataAccess* pCDataAccess	= m_pCMDIChild->m_pCDataAccess;
	CRowset*	pCRowset		= SOURCE_GETOBJECT(pCDataAccess, CRowset);
	CWaitCursor waitCursor;
		
	DBCOUNTITEM 	cRowsObtained = 0;
	HROW			rghRows[MAX_OPENROWS];
	HROW*			phRows = rghRows;

	//Only use dynamic memory allocation if more rows than our static array
	if((cRows > MAX_OPENROWS) || (cRows < -MAX_OPENROWS))
		phRows = NULL;	

	if(pCRowset)
	{
		//Retry is dependednt upon ability to release previously held rows...
		fRetry = fRetry && (GetOptions()->m_dwRowsetOpts & ROWSET_ALWAYSRELEASEROWS);
		
		while(TRUE)
		{
			//Release previously fetched rows, (if user requested)
			TESTC(hr = ReleaseHeldRows());

			//IRowset::GetNextRows
			TESTC(hr = hrGNR = pCRowset->GetNextRows(lOffset, cRows, &cRowsObtained, &phRows));

			//Now Display the rows...
			TESTC(hr = DisplayRows(lOffset, cRows, cRowsObtained, phRows, TRUE/*fAdjustFetchPosition*/));
			
			//Some providers may have limitations on the number of rows that can be returned in 
			//one call, but the consumer may actually just wish to see all the rows without having 
			//to "manually" scroll down one row at a time.  The consumer indicates they wish this 
			//behavior by having "ReleaseRowsAlways" on, so we will do whatever possible to obtain 
			//all the requested rows - assuming the calling context is "retry-able" - ie: fRetry
			if(fRetry && cRowsObtained)
			{
				//If we are not at the end of the rowset, and not all rows were retrieved
				//then retry.  Provider probably returned DB_S_ROWLIMITEXCEEDED
				if(hrGNR!=DB_S_ENDOFROWSET && cRowsObtained < (DBCOUNTITEM)ABS(cRows))
				{
					cRows -= cRowsObtained;
					continue;
				}
			}

			//Were done...
			break;
		}
	}
	else
	{
		ASSERT(cRows <= MAX_OPENROWS);
		cRowsObtained = ABS(cRows);

		//Now Display the rows...
		TESTC(hr = DisplayRows(lOffset, cRows, cRowsObtained, phRows, TRUE/*fAdjustFetchPosition*/));
	}
	
CLEANUP:
	//Delete the rows if provider allocated
	if(phRows != rghRows)
		SAFE_FREE(phRows);
	return hr;
}


////////////////////////////////////////////////////////////////
// CDataGrid::GetSelectedRow
//
/////////////////////////////////////////////////////////////////
INDEX CDataGrid::GetSelectedRow(HROW* phRow, BOOL fValidate)
{
	//Find the Selected Item in the ListView
	INDEX iSelRow = GetNextItem(-1, LVNI_SELECTED);
	if(iSelRow == LVM_ERR)
	{
		if(fValidate)
		{
			wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONHAND | MB_OK | MB_DEFBUTTON1, 
				wsz_ERROR, L"Must first select a row...");
		}
		
		return iSelRow;
	}
	
	//Otherwise we have a valid row...
	if(phRow)
		*phRow = GetItemParam(iSelRow);

	return iSelRow;
}



////////////////////////////////////////////////////////////////
// CDataGrid::ReleaseHeldRows
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::ReleaseHeldRows()
{
	//Release previously fetched rows, (if user requested)
	if(GetOptions()->m_dwRowsetOpts & ROWSET_ALWAYSRELEASEROWS)
	{
		return ReleaseRows(LV_ALLITEMS, TRUE/*fOnlyValidRows*/);
	}

	return S_OK;
}
	

////////////////////////////////////////////////////////////////
// CDataGrid::ReleaseRows
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::ReleaseRows(INDEX iIndex, BOOL fOnlyValidRows)
{
	HRESULT hr = S_OK;
	CRowset*	pCRowset = (CRowset*)m_pCMDIChild->GetObject(eCRowset);
	
	INDEX i,cRows = 0;
	INDEX* rgItems = NULL;
	HROW* rghRows = NULL;
	ULONG* rgRefCounts = NULL;

	//Release all rows
	if(iIndex == LV_ALLITEMS)
	{
		//Find all Rows
		LV_GetAllItems(m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
	}
	//Release all selected rows
	else if(iIndex == LV_ALLSELITEMS)
	{
		//Find all Selected Rows
		LV_GetSelItems(m_hWnd, &cRows, &rgItems, (LPARAM**)&rghRows);
	}
	//Release 1 Row
	else
	{
		//First need to obtain the LPARAM (hRow) to Release it...
		//TODO: Shouldn't dynamic allocation here!
		cRows = 1;
		SAFE_ALLOC(rgItems, INDEX, cRows);
		SAFE_ALLOC(rghRows, HROW, cRows);
		rgItems[0] = iIndex;
		rghRows[0] = GetItemParam(iIndex);
	}

	//Release only valid rows
	if(fOnlyValidRows)
	{
		//Loop over all rows...
		ULONG iLastValid = 0;
		for(i=0; i<cRows; i++)
		{
			//Compact out invalid row handles...
			if(rghRows[i] == NULL)
				continue;
			
			//Inplace shift...
			rghRows[iLastValid] = rghRows[i];
			rgItems[iLastValid] = rgItems[i];
			iLastValid++;
		}
		cRows = iLastValid;
	}

	//Now ReleaseRows in 1 pass
	if(pCRowset && pCRowset->GetObjectType() != eCDataset)
	{
		if(cRows || !fOnlyValidRows)
		{
			SAFE_ALLOC(rgRefCounts, ULONG, cRows);
			TESTC(hr = pCRowset->ReleaseRows(cRows, rghRows, rgRefCounts));
		}
	}

CLEANUP:
	//If any rows have a refcount of 0, we should NULL the row handle
	//so we can detect the error before GetData is called on a release row
	for(i=0; i<cRows; i++)
	{
		if(rgRefCounts[i] == 0)
		{
			//Update the row handle, and the display
			SetItemText(rgItems[i], 0, L"");
			SetItemParam(rgItems[i], 0, DB_NULL_HROW);
		}
	}

	SAFE_FREE(rgItems);
	SAFE_FREE(rghRows);
	SAFE_FREE(rgRefCounts);
	return hr;
}


////////////////////////////////////////////////////////////////
// CDataGrid::AddRefRows
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::AddRefRows(INDEX iIndex)
{
	HRESULT hr = S_OK;
	CRowset*	pCRowset = (CRowset*)m_pCMDIChild->GetObject(eCRowset);
	
	INDEX cRows = 0;
	HROW* rghRows = NULL;
	ULONG* rgRefCounts = NULL;

	//AddRefRows all Rows
	if(iIndex == LV_ALLITEMS)
	{
		//Find all Rows
		LV_GetAllItems(m_hWnd, &cRows, NULL, (LPARAM**)&rghRows);
	}
	//AddRefRows all selected rows
	else if(iIndex == LV_ALLSELITEMS)
	{
		//Find all Selected Rows
		LV_GetSelItems(m_hWnd, &cRows, NULL, (LPARAM**)&rghRows);
	}
	//AddRefRows 1 Row
	else
	{
		//First need to obtain the LPARAM (hRow) to Release it...
		cRows = 1;
		rghRows[0] = GetItemParam(iIndex);
	}

	//Now AddRefRows in 1 pass
	if(pCRowset && pCRowset->GetObjectType() != eCDataset)
	{
	 	SAFE_ALLOC(rgRefCounts, ULONG, cRows);
		TESTC(hr = pCRowset->AddRefRows(cRows, rghRows, rgRefCounts));
	}

CLEANUP:
	SAFE_FREE(rghRows);
	SAFE_FREE(rgRefCounts);
	return hr;
}



////////////////////////////////////////////////////////////////
// CDataGrid::ScrollGrid
//
/////////////////////////////////////////////////////////////////
HRESULT CDataGrid::ScrollGrid(DBROWCOUNT cItems)
{
	return GetNextRows(0, cItems, TRUE/*fRetry*/);
}

  
////////////////////////////////////////////////////////////////
// CDataGrid::ClearAll
//
/////////////////////////////////////////////////////////////////
BOOL CDataGrid::ClearAll(WCHAR* pwszEmptyName)
{
	// Delete all of the items.	(no need to release rows...)
	DeleteAllItems();

	// Delete all of the columns and their names.
	while(DeleteColumn(0));

	//Indicate no Rowset
	if(pwszEmptyName)
	{
		InsertColumn(0, pwszEmptyName);
		SetColumnWidth(0,	LVSCW_AUTOSIZE_USEHEADER);
	}

	return TRUE;
}



