// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "VCExplore.h"
#include "VCExploreDlg.h"
#include "AppExportDlg.h"
#include "AppInstallDlg.h"
#include "AppUtilDlg.h"
#include "CompImportDlg.h"
#include "CompInstallDlg.h"
#include "ConnectDlg.h"
#include "UtilitiesDlg.h"

#include <comdef.h>

#include "comadmin.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define RELEASE_ALL_PARENT_OBJECTS			0

#define DLG_APP_EXPORT						1
#define DLG_APP_INSTALL						2
#define DLG_APP_UTIL_START					3
#define DLG_APP_UTIL_STOP					4
#define DLG_COMPONENT_IMPORT				5
#define DLG_COMPONENT_INSTALL				6
#define DLG_CONNECT							7
#define DLG_UTILITIES						8
#define DLG_ABOUT							9


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEasterEgg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_APPLICATION_ICON, OnEasterEgg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnEasterEgg() 
{
	MessageBox("Built By:\tAllen Herring\n\tApplication Developer Consultant\n\tMicrosoft Corporation\n\n\tFebruary 1999", 
				"Party On!", 
				(MB_OK | MB_ICONINFORMATION));
}


/////////////////////////////////////////////////////////////////////////////
// CVCExploreDlg dialog

CVCExploreDlg::CVCExploreDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVCExploreDlg::IDD)
{
	UNREFERENCED_PARAMETER(pParent);
	//{{AFX_DATA_INIT(CVCExploreDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_APP_ICON);

	// Pointer Initialization
	m_pCatalog = NULL;
	m_pCurrCollection = NULL;
  m_bChangesArePending = FALSE;
}

void CVCExploreDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVCExploreDlg)
	DDX_Control(pDX, IDC_BUTTON_SET_PROPERTY, m_btnSetProperty);
	DDX_Control(pDX, IDC_EDIT_PROPERTY_VALUE, m_edtPropertyValue);
	DDX_Control(pDX, IDC_LIST_RELATED_COLLECTIONS, m_lstRelatedCollections);
	DDX_Control(pDX, IDC_LIST_PROPERTIES, m_lstProperties);
	DDX_Control(pDX, IDC_LIST_PARENT_COLLECTIONS, m_lstParentCollections);
	DDX_Control(pDX, IDC_LIST_OBJECTS, m_lstObjects);
	DDX_Control(pDX, IDC_LABEL_COMPUTER_NAME_VALUE, m_lblComputerName);
	DDX_Control(pDX, IDC_LABEL_COLLECTION_NAME_VALUE, m_lblCollectionName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVCExploreDlg, CDialog)
	//{{AFX_MSG_MAP(CVCExploreDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_TBTN_ABOUT, OnTBtnAbout)
	ON_COMMAND(ID_TBTN_CONNECT, OnTBtnConnect)
	ON_COMMAND(ID_TBTN_DELETE, OnTBtnDelete)
	ON_COMMAND(ID_TBTN_EXPORT_APP, OnTBtnExportApp)
	ON_COMMAND(ID_TBTN_IMPORT_COMPONENT, OnTBtnImportComponent)
	ON_COMMAND(ID_TBTN_INSTALL_APP, OnTBtnInstallApp)
	ON_COMMAND(ID_TBTN_INSTALL_COMPONENT, OnTBtnInstallComponent)
	ON_COMMAND(ID_TBTN_NEW, OnTBtnNew)
	ON_COMMAND(ID_TBTN_REFRESH, OnTBtnRefresh)
	ON_COMMAND(ID_TBTN_SAVE, OnTBtnSave)
	ON_COMMAND(ID_TBTN_START_APP, OnTBtnStartApp)
	ON_COMMAND(ID_TBTN_STOP_APP, OnTBtnStopApp)
	ON_COMMAND(ID_TBTN_UTILITY, OnTBtnUtility)
	ON_WM_CLOSE()
	ON_LBN_SELCHANGE(IDC_LIST_RELATED_COLLECTIONS, OnSelChangeRelatedCollections)
	ON_LBN_SELCHANGE(IDC_LIST_PARENT_COLLECTIONS, OnSelChangeParentCollections)
	ON_LBN_SELCHANGE(IDC_LIST_OBJECTS, OnSelChangeObjects)
	ON_LBN_SELCHANGE(IDC_LIST_PROPERTIES, OnSelChangeProperties)
	ON_BN_CLICKED(IDC_BUTTON_SET_PROPERTY, OnSetProperty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVCExploreDlg message handlers

BOOL CVCExploreDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Create the button structure
	TBBUTTON tbButtons[] = {
		{0, ID_TBTN_CONNECT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, (UCHAR)-1},
		{1, ID_TBTN_REFRESH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, (UCHAR)-1},
		{2, ID_TBTN_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{3, ID_TBTN_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{4, ID_TBTN_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, (UCHAR)-1},
		{5, ID_TBTN_START_APP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{6, ID_TBTN_STOP_APP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{7, ID_TBTN_INSTALL_APP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{8, ID_TBTN_EXPORT_APP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, (UCHAR)-1},
		{9, ID_TBTN_INSTALL_COMPONENT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{10, ID_TBTN_IMPORT_COMPONENT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, (UCHAR)-1},
		{11, ID_TBTN_UTILITY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, (UCHAR)-1},
		{12, ID_TBTN_ABOUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0}
	};

	// Create the bitmap structure that represents the toolbar
	TBADDBITMAP tbBitmap;
	tbBitmap.hInst = AfxGetApp()->m_hInstance;
	tbBitmap.nID = IDR_TOOLBAR_MAIN;

	// Create the toolbar
	m_hwndToolBar = CreateToolbarEx(this->m_hWnd,											// HWND of the toolbar's parent 
									WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS,	// Toolbar style
									IDR_TOOLBAR_MAIN,										// Toolbar Command ID
									13,														// Number of bitmaps
									AfxGetApp()->m_hInstance,								// HINST of the module that contains the resource
									IDR_TOOLBAR_MAIN,										// Toolbar Resoure ID
									(LPCTBBUTTON) &tbButtons,								// Toolbar button structure
									19,														// Number of buttons (including seperators)
									16, 16,													// Button width & height
									16, 16,													// Bitmap width & height
									sizeof(TBBUTTON));										// Size of single TBBUTTON structure (i.e., individual button)

	if (NULL != m_hwndToolBar)
	{
		CWnd* pToolBar = GetDlgItem(IDR_TOOLBAR_MAIN);
		pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_NEW, MAKELONG(false, 0));
		pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_SAVE, MAKELONG(false, 0));
		pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_DELETE, MAKELONG(false, 0));
	}
	else
		MessageBox("Failed to create toolbar.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));

	// Enable the tooltip window for this CWnd object
	this->EnableToolTips(true);
	
	// Load the COM libraries
	HRESULT hr = CoInitialize(NULL);
	if SUCCEEDED(hr)
	{
		hr = CreateCatalog();
		if SUCCEEDED(hr)
			hr = NavigateTo(m_pCurrCollection);
	}

	// Display the computer name
	_bstr_t	bstrComputerName = "( Local )";
	set_ComputerName(bstrComputerName);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVCExploreDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVCExploreDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVCExploreDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CVCExploreDlg::OnTBtnAbout() 
{
	// Continue only if we have a valid reference to a Catalog object
	if (NULL != GetCatalog())
	{
		CAboutDlg dlg;

		// Display the "About" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnConnect() 
{
	if (NULL != GetCatalog())
	{
		CConnectDlg dlg;

		// Set the dialog's catalog reference...
		dlg.set_Catalog(GetCatalog());
		//... and set the "child" dialog's parent (i.e., communication channel)
		dlg.set_ParentDlg(this);

		// Display the "Connect" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnDelete() 
{
	// Get the current object selected
	int nObjectIndex = m_lstObjects.GetCurSel();

	// Continue only if an object is selected
	if (LB_ERR == nObjectIndex)
		return;
	
	// Attempt to remove the item from the collection
	HRESULT hr = m_pCurrCollection->Remove(nObjectIndex);
  if (SUCCEEDED(hr))
  {
		CWnd* pToolBar = GetDlgItem(IDR_TOOLBAR_MAIN);
		pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_SAVE, MAKELONG(true, 0));
    m_bChangesArePending = TRUE;
  }
  else
  {
		MessageBox("Failed to delete the specified object from the current collection.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));
  }
}

void CVCExploreDlg::OnTBtnExportApp() 
{
	if (NULL != GetCatalog())
	{
		CAppExportDlg dlg;

		// Set the dialog's catalog reference... ONLY if we aren't invoking the Connect dialog
		dlg.set_Catalog(GetCatalog());

		// Display the "App Export" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnImportComponent() 
{
	if (NULL != GetCatalog())
	{
		CCompImportDlg dlg;

		// Set the dialog's catalog reference... ONLY if we aren't invoking the Connect dialog
		dlg.set_Catalog(GetCatalog());

		// Display the "Import Component" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnInstallApp() 
{
	if (NULL != GetCatalog())
	{
		CAppInstallDlg dlg;

		// Set the dialog's catalog reference... ONLY if we aren't invoking the Connect dialog
		dlg.set_Catalog(GetCatalog());

		// Display the "Import Component" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnInstallComponent() 
{
	if (NULL != GetCatalog())
	{
		CCompInstallDlg dlg;

		// Set the dialog's catalog reference... ONLY if we aren't invoking the Connect dialog
		dlg.set_Catalog(GetCatalog());

		// Display the "Import Component" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnNew() 
{
	ICatalogObject* pCatalogObject = NULL;
	
	// Attempt to add a new item to the current collection
	HRESULT hr = m_pCurrCollection->Add((IDispatch**) &pCatalogObject);

	// Validate success/fail of the requested operation
	if SUCCEEDED(hr)
	{
		 // Toggle the toolbar save button
		CWnd* pToolBar = GetDlgItem(IDR_TOOLBAR_MAIN);
		pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_SAVE, MAKELONG(true, 0));
    m_bChangesArePending = TRUE;

		// Release the new CatalogObject reference
		pCatalogObject->Release();
	}
	else
		MessageBox("Failed to add the new object to the current collection.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));

	// Refresh the UI
  NavigateTo(m_pCurrCollection, FALSE);
}

void CVCExploreDlg::OnTBtnRefresh() 
{  
	// Refresh the UI with the current collection
	NavigateTo(m_pCurrCollection);	
}

void CVCExploreDlg::OnTBtnSave() 
{
	// Attempt to save the requested changes
	long lRetValue = 0;
	HRESULT hr = m_pCurrCollection->SaveChanges(&lRetValue);
	if FAILED(hr)
  {
		MessageBox("Failed to save changes to the current collection.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));
  }

	 // Turn off the toolbar save button
	CWnd* pToolBar = GetDlgItem(IDR_TOOLBAR_MAIN);
	pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_SAVE, MAKELONG(false, 0));
  m_bChangesArePending = FALSE;

	// Refresh the UI with the current collection
	NavigateTo(m_pCurrCollection);	
}

void CVCExploreDlg::OnTBtnStartApp() 
{
	if (NULL != GetCatalog())
	{
		CAppUtilDlg dlg;

		// Set the dialog's catalog reference... ONLY if we aren't invoking the Connect dialog
		dlg.set_Catalog(GetCatalog());

		// Set the dialog "type"
		dlg.set_UtilityType(UTILITY_TYPE_START_APP);

		// Display the "Import Component" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnStopApp() 
{
	if (NULL != GetCatalog())
	{
		CAppUtilDlg dlg;

		// Set the dialog's catalog reference... ONLY if we aren't invoking the Connect dialog
		dlg.set_Catalog(GetCatalog());

		// Set the dialog "type"
		dlg.set_UtilityType(UTILITY_TYPE_STOP_APP);

		// Display the "Import Component" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnTBtnUtility() 
{
	if (NULL != GetCatalog())
	{
		CUtilitiesDlg dlg;

		// Set the dialog's catalog reference... ONLY if we aren't invoking the Connect dialog
		dlg.set_Catalog(GetCatalog());

		// Display the "Import Component" dialog
		dlg.DoModal();
	}
}

void CVCExploreDlg::OnClose() 
{
	// Release the pointers to out objects and unload the COM libraries
	RemoveParent(RELEASE_ALL_PARENT_OBJECTS);
	DestroyCatalog();
	CoUninitialize();

	CDialog::OnClose();
}

HRESULT CVCExploreDlg::CreateCatalog()
{
	// Close any previous Catalog instances
	DestroyCatalog();

	IUnknown*	pUnknown = NULL;

	// Attempt to get a reference to the objects IUnknown pointer
	HRESULT hr = CoCreateInstance(CLSID_COMAdminCatalog,
									NULL,
									CLSCTX_INPROC_SERVER,
									IID_IUnknown,
									(void**) &pUnknown);

	// Continue only if successful
	if SUCCEEDED(hr)
	{
		// Attempt to get a reference to the object's ICOMAdminCatalog interface
		hr = pUnknown->QueryInterface(IID_ICOMAdminCatalog, 
										(void**) &m_pCatalog);

		// Release the IUnknown pointer
		pUnknown->Release();

		// Continue only if successful
		if SUCCEEDED(hr)
		{
			_bstr_t	bstrRootCollection = "Root";

			// Attempt to connect to "Root" of the Catalog object
			hr = m_pCatalog->GetCollection(bstrRootCollection, 
											(IDispatch**) &m_pCurrCollection);

			// Continue only if successful
			if SUCCEEDED(hr)
			{
				// Attempt to load the "Root" collection objects
				hr = m_pCurrCollection->Populate();
				
				// Continue only if successful
				if SUCCEEDED(hr)
					return S_OK;
				else
					MessageBox("Failed to populate the current Catalog objects root collection.\n\nPress OK to continue.",
								"Error",
								(MB_OK | MB_ICONERROR));
			}
			else
				MessageBox("Failed to obtain a reference to the current Catalog objects root collection.\n\nNo processing will be performed.\n\nPress OK to continue.",
							"Error",
							(MB_OK | MB_ICONERROR));
		}
		else
			MessageBox("ICOMAdminCatalog interface is not supported.\n\nNo processing will be performed.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));
	}
	else
		MessageBox("Failed to instantiate the COMAdmin object.\n\nNo processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));
		
	// Default return value
	return hr;
}

void CVCExploreDlg::DestroyCatalog()
{
	if (NULL != m_pCurrCollection)
	{
		m_pCurrCollection->Release();
		m_pCurrCollection = NULL;
	}

	if (NULL != m_pCatalog)
	{
		m_pCatalog->Release();
		m_pCatalog = NULL;
	}
}

HRESULT CVCExploreDlg::NavigateTo(ICatalogCollection *pCatalogCollection, BOOL bPopulate)
{
	long					lObjCount = 0;						 

	_bstr_t					bstrRelatedCollectionInfo = "RelatedCollectionInfo";
	_bstr_t					bstrPropertyInfo = "PropertyInfo";

	_variant_t				vtRelatedCollInfoKey = "RelatedCollectionInfo";
	_variant_t				vtPropertyInfoKey = "PropertyInfo";
	_variant_t				vtKey = "";
	_variant_t				vtName = "";

	ICatalogObject*			pCatalogObject = NULL;
	ICatalogCollection*		pRelatedCollInfo = NULL;
	ICatalogCollection*		pPropertyInfo = NULL;

	// Make sure the current CatalogCollection object has been populated
	HRESULT hr;
  
  if (bPopulate)
    hr = pCatalogCollection->Populate();

	////////////////////////////////////////////////////////////////////////////////////////
	// FIRST...
	//	Display the current CatalogCollection's name
	
	hr = pCatalogCollection->get_Name(&vtName);
	if SUCCEEDED(hr)
		m_lblCollectionName.SetWindowText(_bstr_t(vtName));

	////////////////////////////////////////////////////////////////////////////////////////
	// SECOND...
	//	Enumerate "RelatedCollectionInfo" for the current collection
	//
	
	// Clear the listbox control
	m_lstRelatedCollections.ResetContent();

	hr = pCatalogCollection->GetCollection(bstrRelatedCollectionInfo,
													vtRelatedCollInfoKey,
													(IDispatch**) &pRelatedCollInfo);
	if SUCCEEDED(hr)
	{
		// Attempt to populate the RelatedCollectionInfo
		hr = pRelatedCollInfo->Populate();
		if SUCCEEDED(hr)
		{
			// Output the number of RelatedCollectionInfo objects in the collection
			long lCount = 0;
			hr = pRelatedCollInfo->get_Count(&lCount);

			for (long lIndex = 0; lIndex < lCount; lIndex++)
			{
				hr = pRelatedCollInfo->get_Item(lIndex, 
												(IDispatch**) &pCatalogObject);
				if SUCCEEDED(hr)
				{
					// Display the current CatalogObject's Key and Name
					pCatalogObject->get_Key(&vtKey);
					pCatalogObject->get_Name(&vtName);

					// Add the RelatedCollection's name to the listbox control
					m_lstRelatedCollections.AddString(_bstr_t(vtName));

					// Release the ICatalogObject pointer
					pCatalogObject->Release();
				}
			}
		}

		// Release the ICatalogCollection pointer
		pRelatedCollInfo->Release();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	// THIRD...
	//	Enumerate "PropertyInfo" for the current collection
	//

	// Clear the listbox control
	m_lstProperties.ResetContent();

	hr = pCatalogCollection->GetCollection(bstrPropertyInfo,
											vtPropertyInfoKey,
											(IDispatch**) &pPropertyInfo);
	if SUCCEEDED(hr)
	{
		// Populate the collection
		hr = pPropertyInfo->Populate();
		if SUCCEEDED(hr)
		{
			// Display the number of PropertyInfo objects in the collection
			long lPropCnt = 0;
			pPropertyInfo->get_Count(&lPropCnt);

			for (int lIndex = 0; lIndex < lPropCnt; lIndex++)
			{
				hr = pPropertyInfo->get_Item(lIndex, 
											(IDispatch**) &pCatalogObject);
				if SUCCEEDED(hr)
				{
					// Display the current CatalogObject's Key and Name
					pCatalogObject->get_Key(&vtKey);
					pCatalogObject->get_Name(&vtName);

					// Add property names to the listbox control
					m_lstProperties.AddString(_bstr_t(vtName));

					// Release the ICatalogObject pointer
					pCatalogObject->Release();
				}
			}
		}
		
		// Release the ICatalogCollection pointer
		pPropertyInfo->Release();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	// FORTH...
	//	Enumerate the current collection's objects
	//

	// Clear the listbox control
	m_lstObjects.ResetContent();

	// Get the number of objects available in this CatalogCollection
	hr = pCatalogCollection->get_Count(&lObjCount);

	// Output each objects (i.e., CatalogObject) properties in the current collection
	for (long lIndex = 0; lIndex < lObjCount; lIndex++)
	{
		hr = pCatalogCollection->get_Item(lIndex, 
											(IDispatch**) &pCatalogObject);

		if SUCCEEDED(hr)
		{
			// Get the current CatalogObject's Name
			pCatalogObject->get_Name(&vtName);

			// Add the Object names to the listbox control
			m_lstObjects.AddString(_bstr_t(vtName));

			// Release the temporary ICatalogObject pointer
			pCatalogObject->Release();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	// FIFTH...
	//	Set the state of the persistence (i.e., new & delete) toolbar buttons
	//

	// Set the persistence toolbar buttons state depending upon the current collections
	//	properties (i.e., whether or not save & delete are permitted
	short sSaveEnabled = 0;
	short sRemoveEnabled = 0;
	CWnd* pToolBar = GetDlgItem(IDR_TOOLBAR_MAIN);

	hr = pCatalogCollection->get_AddEnabled(&sSaveEnabled);
	if (sSaveEnabled && SUCCEEDED(hr))
		pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_NEW, MAKELONG(true, 0));

	hr = pCatalogCollection->get_RemoveEnabled(&sRemoveEnabled);
	if (sRemoveEnabled && SUCCEEDED(hr))
		pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_DELETE, MAKELONG(true, 0));
	
	////////////////////////////////////////////////////////////////////////////////////////
	// SIXTH...
	//	Clear any existing property value
	//
	m_edtPropertyValue.SetSel(0, -1, false);
	m_edtPropertyValue.Clear();
  
  m_lstObjects.SetCurSel(0);
  m_lstProperties.SetCurSel(0);

	return hr;
}


void CVCExploreDlg::OnSelChangeRelatedCollections() 
{
  TryToSaveChanges();

	// Get the index of the currently selected listbox item
	int		nCurrSelection = m_lstRelatedCollections.GetCurSel();

	// Get the currently selected items text (i.e., new collection name)
	int		nBufSize = m_lstRelatedCollections.GetTextLen(nCurrSelection) + 1;
	char*	pBuf = new char[nBufSize];
	int		nRetVal = m_lstRelatedCollections.GetText(nCurrSelection, pBuf);
	
	// Attempt to get a reference to the new CatalogCollection object
	_bstr_t				bstrCollectionName = pBuf;
	_variant_t		vtCollectionKey;
	ICatalogCollection*	pCatalogCollection = NULL;

  // We might need an object key, depending on which related collection was
  // selected
	HRESULT hr = E_UNEXPECTED;

  if (!lstrcmpW(bstrCollectionName, L"RolesForMethod") ||
      !lstrcmpW(bstrCollectionName, L"MethodsForInterface") ||
      !lstrcmpW(bstrCollectionName, L"InterfacesForComponent") ||
      !lstrcmpW(bstrCollectionName, L"RolesForComponent") ||
      !lstrcmpW(bstrCollectionName, L"Roles") ||
      !lstrcmpW(bstrCollectionName, L"SubscriptionsForComponent") ||
      !lstrcmpW(bstrCollectionName, L"UsersInRole") ||
      !lstrcmpW(bstrCollectionName, L"Components") )
  {
    //  Need to get the object key
    int   nObjectCurrSelection = m_lstObjects.GetCurSel();
  	int		nObjectBufSize = m_lstObjects.GetTextLen(nObjectCurrSelection) + 1;
	  char*	pObjectBuf = new char[nObjectBufSize];
	  nRetVal = m_lstObjects.GetText(nObjectCurrSelection, pObjectBuf);

    long  lCollectionCount; 
    int   iLoopIndex;
    ICatalogObject* pCatalogObject = NULL;

    m_pCurrCollection->get_Count(&lCollectionCount);
    
    _bstr_t bstrSelObjName = pObjectBuf;
    for (iLoopIndex = 0; iLoopIndex < lCollectionCount; iLoopIndex++)
    {
      hr = m_pCurrCollection->get_Item(iLoopIndex, (IDispatch**)&pCatalogObject);
      if (SUCCEEDED(hr))
      {
        _variant_t vtName;
        hr = pCatalogObject->get_Name(&vtName);
        if (SUCCEEDED(hr) && !lstrcmp(bstrSelObjName, (_bstr_t)vtName))
        {
          // Found it
          break;
        }
        pCatalogObject->Release();
        pCatalogObject = NULL;
      }
    }
    
    if (pCatalogObject)
    {
      hr = pCatalogObject->get_Key(&vtCollectionKey);
      pCatalogObject->Release();
    }

    delete [] pObjectBuf;
  }
  else
  {
    // No key needed
    vtCollectionKey = bstrCollectionName;
  }
        
  hr = m_pCurrCollection->GetCollection(bstrCollectionName,
												vtCollectionKey,
												(IDispatch**) &pCatalogCollection);

	if SUCCEEDED(hr)
	{
		// Navigate to the new collection
		hr = NavigateTo(pCatalogCollection);

		if SUCCEEDED(hr)
		{			
			// Increment the reference count to the current CatalogCollection
			//	before caching its value
			m_pCurrCollection->AddRef();

			// Add the current CatalogCollection to the ParentCollection 
			//	listbox (i.e., cache the current CatalogCollection object)
			AddParent(m_pCurrCollection);

			// Increment the reference count to the temporary CatalogCollection
			//	object before setting/copying it as the currenct CatalogCollection
			pCatalogCollection->AddRef();

			// Set the "new" current CatalogCollection to the user's selection
			m_pCurrCollection = pCatalogCollection;
		}

		// Release the temporary ICatalogCollection pointer
		pCatalogCollection->Release();
	}

	// Cleanup
	delete [] pBuf;
}

void CVCExploreDlg::OnSelChangeParentCollections() 
{
  TryToSaveChanges();

	// Determine which item is selected
	int nIndex = m_lstParentCollections.GetCurSel();

	// Remove the strings from the listbox and release the un-needed objects (i.e., un-wind/pop values off the stack)
	RemoveParent(nIndex);

	// Navigate to the "new" currrent CatalogCollection
	NavigateTo(m_pCurrCollection);
	
}

int CVCExploreDlg::AddParent(ICatalogCollection *pCatalogCollection)
{
	_variant_t	vtName;
	int			nIndex = -1;

	// Add the CatalogCollections name to the ParentCollection listbox
	HRESULT hr = pCatalogCollection->get_Name(&vtName);
	if SUCCEEDED(hr)
	{
		// Attempt to dd the name to the ParentCollections listbox
		nIndex = m_lstParentCollections.AddString(_bstr_t(vtName));

		// If successful, store/cache the ICatalogCollection pointer
		if ((LB_ERR != nIndex) && (LB_ERRSPACE != nIndex))
			m_lstParentCollections.SetItemDataPtr(nIndex, pCatalogCollection);
	}

	return nIndex;
}

void CVCExploreDlg::RemoveParent(int nNewCurrIndex)
{
	// Remove the strings from the listbox and release the un-needed objects (i.e., un-wind/pop values off the stack)
	for (int nPtr = (m_lstParentCollections.GetCount() - 1); nPtr >= nNewCurrIndex; nPtr--)
	{
		// Get the current listbox item's cached ICatalogCollection pointer
		ICatalogCollection* pCatalogCollection = (ICatalogCollection*) m_lstParentCollections.GetItemDataPtr(nPtr);
		
		// If we are on the item the user selected...
		if (nNewCurrIndex == nPtr)
		{
			//... release the CatalogCollection we are currently pointing to...
			m_pCurrCollection->Release();
			//... increment the number of references to the CatalogCollection we are currently using...
			pCatalogCollection->AddRef();
			//... set the "new" current CatalogCollection
			m_pCurrCollection = pCatalogCollection;
		}

		// Remove the current listbox item
		m_lstParentCollections.DeleteString(nPtr);

		// Release the temporary CatalogCollection object
		pCatalogCollection->Release();
	}
}

void CVCExploreDlg::DisplayPropertyValue()
{
	// Clear the current contents of the edit control
	m_edtPropertyValue.SetSel(0, -1, false);
	m_edtPropertyValue.Clear();

	// Determine which (if any) CatalogObject is selected
	int nObjectIndex = m_lstObjects.GetCurSel();
	int nPropertyIndex = m_lstProperties.GetCurSel();

	// Bail if no CatalogObject OR Property selected
	if ((LB_ERR == nObjectIndex) || (LB_ERR == nPropertyIndex))
		return;

	// Attempt to get a reference to the currently selected CatalogObject
	ICatalogObject* pCatalogObject = NULL;

	HRESULT hr = m_pCurrCollection->get_Item(nObjectIndex,
											(IDispatch**) &pCatalogObject);
	if SUCCEEDED(hr)
	{
		// Get the Properties name
		int nTextLength = m_lstProperties.GetTextLen(nPropertyIndex);
		char* pBuf = new char[nTextLength + 1];
		m_lstProperties.GetText(nPropertyIndex, pBuf);

		// Attempt to get the actual properties value
		_bstr_t		bstrPropertyName = pBuf;
		delete [] pBuf;

		_variant_t	vtPropertyValue = "";

		hr = pCatalogObject->get_Value(bstrPropertyName, &vtPropertyValue);

		// Set the text if successful
		if SUCCEEDED(hr)
		{
			short sFlag = 0;

			// Read only check (i.e., enable update only if allowed)
			hr = pCatalogObject->IsPropertyReadOnly(bstrPropertyName, &sFlag);
			if (sFlag || FAILED(hr))
				m_btnSetProperty.EnableWindow(false);
			else
				m_btnSetProperty.EnableWindow(true);

			// Write only check (i.e., display the value only if allowed)
			hr = pCatalogObject->IsPropertyWriteOnly(bstrPropertyName, &sFlag);
			if (!sFlag && SUCCEEDED(hr))
			{
				// Set the new edit control text
				try
				{
					m_edtPropertyValue.SetWindowText(_bstr_t(vtPropertyValue));
					// Select the text
					m_edtPropertyValue.SetSel(0, -1, false);
				}

				catch (...)
				{
					m_btnSetProperty.EnableWindow(false);
					vtPropertyValue = "";
					m_edtPropertyValue.SetWindowText(_bstr_t(vtPropertyValue));
				}
			}
		}

		// Release the temporary CatalogObject
		pCatalogObject->Release();
	}
}

void CVCExploreDlg::OnSelChangeObjects() 
{
	DisplayPropertyValue();	
}

void CVCExploreDlg::OnSelChangeProperties() 
{
	DisplayPropertyValue();	
}



ICOMAdminCatalog* CVCExploreDlg::GetCatalog()
{
	// Increment the reference count on the Catalog object only if it's valid
	if (NULL != m_pCatalog)
		m_pCatalog->AddRef();

	// Return a reference to the current Catalog object
	return m_pCatalog;
}

void CVCExploreDlg::set_CurrentCollection(ICatalogCollection* pCollection)
{
	// Clear the Object and Property listbox contents, if any
	m_lstObjects.ResetContent();
	m_lstProperties.ResetContent();
	
	// Release all of the parent Collection objects, if any
	RemoveParent(RELEASE_ALL_PARENT_OBJECTS);

	// Release the current Collection
	if (NULL != m_pCurrCollection)
		m_pCurrCollection->Release();

	// Assign the new Catalog
	m_pCurrCollection = pCollection;

	// Populate the collection
	HRESULT hr = m_pCurrCollection->Populate();
	if SUCCEEDED(hr)
		// Navigate to the new collection (i.e., fill in the UI)
		NavigateTo(m_pCurrCollection);
}

void CVCExploreDlg::set_ComputerName(_bstr_t bstrComputerName)
{
	m_lblComputerName.SetWindowText(bstrComputerName);
}

void CVCExploreDlg::OnSetProperty() 
{
	// Continue only if we have a valid reference to a Catalog object
	if (NULL != m_pCatalog)
	{
		ICatalogObject* pCatalogObject = NULL;
		int				nObjectIndex = m_lstObjects.GetCurSel();
		int				nPropertyIndex = m_lstProperties.GetCurSel();

		// Only try to set a property value if a property of an object has actually
		//	been selected
		if ((LB_ERR == nObjectIndex) || (LB_ERR == nPropertyIndex))
			return;

		// Attempt to get a reference to the current CatalogObject selected
		HRESULT hr = m_pCurrCollection->get_Item(nObjectIndex,
												(IDispatch**) &pCatalogObject);
		if SUCCEEDED(hr)
		{
			// Get the currently selected Properties name
			int		nTextLength = m_lstProperties.GetTextLen(nPropertyIndex);
			char*	pBuf = new char[nTextLength + 1];
			m_lstProperties.GetText(nPropertyIndex, pBuf);
			_bstr_t	bstrPropertyName = pBuf;
			delete [] pBuf;

			// Extract the new value from the UI
			int			nNewValueLength = m_edtPropertyValue.GetWindowTextLength() + 1;
			char*		pNewValueBuf = new char[nNewValueLength];
			m_edtPropertyValue.GetWindowText(pNewValueBuf, nNewValueLength);
			_bstr_t		bstrNewValue = pNewValueBuf;
			delete [] pNewValueBuf;
			_variant_t	vtPropertyValue = bstrNewValue;

			// Attempt to update the Properties value
			hr = pCatalogObject->put_Value(bstrPropertyName, vtPropertyValue);

			// Validate success/fail of the operation
			if SUCCEEDED(hr)
			{
				CWnd* pToolbar = GetDlgItem(IDR_TOOLBAR_MAIN);
				pToolbar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_SAVE, MAKELONG(true, 0));
        m_bChangesArePending = TRUE;
			}
			else
				MessageBox("Failed to save the selected CatalogObject's property value.\n\nPress OK to continue.",
							"Error",
							(MB_OK | MB_ICONERROR));

			// Release the temporary CatalogObject
			pCatalogObject->Release();
		}
		else
			MessageBox("Failed to obtain a reference to the selected CatalogObject.\n\nNo Processing will be performed.\n\nPress OK to continue.",
						"Error",
						(MB_OK | MB_ICONERROR));
	}
	else
		MessageBox("Invalid Catalog object.\n\nNo Processing will be performed.\n\nPress OK to continue.",
					"Error",
					(MB_OK | MB_ICONERROR));
}

void CVCExploreDlg::TryToSaveChanges()
{
  if (!m_bChangesArePending)
  {
    // nothing to do
    return;
  }
  
  int iMBRet = MessageBox("Do you wish to save your changes before continuing?", "Changes may be lost!", MB_ICONWARNING | MB_YESNO);
  if (iMBRet == IDYES)
  {
    long lCount;
    m_pCurrCollection->SaveChanges(&lCount);
  }
  CWnd* pToolBar = GetDlgItem(IDR_TOOLBAR_MAIN);
  pToolBar->SendMessage(TB_ENABLEBUTTON, ID_TBTN_SAVE, MAKELONG(false, 0));
  m_bChangesArePending = FALSE;
}
