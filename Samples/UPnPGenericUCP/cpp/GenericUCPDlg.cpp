// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// GenericUCPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GenericUCP.h"
#include "GenericUCPDlg.h"
#include "deviceprop.h"
#include "SCPDDisplay.h"
#include "util.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
   //{{AFX_VIRTUAL(CAboutDlg)
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   //{{AFX_MSG(CAboutDlg)
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
   // No message handlers
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()


// App command to run the dialog
void CGenericUCPDlg::OnAppAbout()
{
   CAboutDlg aboutDlg;
   aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CGenericUCPDlg dialog


//+---------------------------------------------------------------------------
//
//  Member:   CGenericUCPDlg
//
//  Purpose:    Constructor
//
//  Arguments:
//      pParent         [in]        Parent Window
//
//  Returns:    None
//
//  Notes:
//     We create the callback objects for device finder and service objects.
//

CGenericUCPDlg::CGenericUCPDlg(CWnd* pParent)
   : CDialog(CGenericUCPDlg::IDD, pParent)
{
    m_pDeviceFinder = NULL;
   // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

   // Create a device finder callback object
   m_pDeviceFinderCallback = NULL;
   m_pDeviceFinderCallback = CDeviceFinderCallback::Create();
   if (m_pDeviceFinderCallback == NULL)
   {
      MessageBox(
         L"Error: DeviceFinderCallback object creation failed"
         );
   }
   else
   {
      ((CDeviceFinderCallback*)m_pDeviceFinderCallback)->SetDialogPointer(
                                                            this
                                                            );
   }

   // Create a service callback object
   m_pServiceCallback = NULL;
   m_pServiceCallback = CServiceCallback::Create();
   if (m_pServiceCallback == NULL)
   {
      MessageBox(L"Error: ServiceCallback object creation failed");
   }
   else
   {
      ((CServiceCallback*)m_pServiceCallback)->SetDialogPointer(this);
   }

   // Initialize close handle
   m_hCloseEvent = ::CreateEvent(NULL,   // default security attributes
                                 TRUE,   // manual reset event object
                                 FALSE,  // initial state is unsignaled
                                 NULL);  // unnamed object

   if (!m_hCloseEvent)
   {
      MessageBox(L"Error: Failed to create Close event");
   }


}


//+---------------------------------------------------------------------------
//
//  Member:		~CGenericUCPDlg
//
//  Purpose:    Destructor
//
//  Arguments:
//		None
//
//  Returns:    None
//
//  Notes:
//     We stop the async find if it is proceeding, and release the callback 
//     objects
//

CGenericUCPDlg::~CGenericUCPDlg()
{
   // Destructor
  if(m_hCloseEvent)
   {
      CloseHandle(m_hCloseEvent);
   }
   ReleaseObj(m_pServiceCallback);
   m_pServiceCallback = NULL;
   ReleaseObj(m_pDeviceFinderCallback);
   m_pDeviceFinderCallback = NULL;
   StopAsyncFindIfStarted();
   ReleaseObj(m_pDeviceFinder);
   m_pDeviceFinder = NULL;
}


// MFC Function
void CGenericUCPDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CGenericUCPDlg)
   DDX_Control(pDX, IDC_VARIABLENAME, m_VariableName);
   DDX_Control(pDX, IDC_ACTIONNAME, m_ActionName);
   DDX_Control(pDX, IDC_STATUS, m_StatusText);
   DDX_Control(pDX, IDC_EVENTS, m_EventText);
   DDX_Control(pDX, IDC_VIEWSCPD, m_ViewSCPD);
   DDX_Control(pDX, IDC_SCPDURL, m_SCPDURL);
   DDX_Control(pDX, IDC_PRESENTATION, m_Presentation);
   DDX_Control(pDX, IDC_QUERYVARIABLE, m_QueryVariable);
   DDX_Control(pDX, IDC_INVOKEACTION, m_InvokeAction);
   DDX_Control(pDX, IDC_SUBSCRIBE, m_Subscribe);
   DDX_Control(pDX, IDC_DISCOVERY, m_StartDiscovery);
   DDX_Control(pDX, IDC_DEVPROP, m_DeviceProperties);
   DDX_Control(pDX, IDC_COMBOSERVICE, m_ServiceCombo);
   DDX_Control(pDX, IDC_COMBOFIND, m_FindCombo);
   DDX_Control(pDX, IDC_COMBODEVICE, m_DeviceCombo);
   DDX_Control(pDX, IDC_CLOSE, m_CloseApp);
   DDX_Control(pDX, IDC_ACTIONOUTARGUMENT, m_ActionOutArgument);
   DDX_Control(pDX, IDC_ACTIONINARGUMENT, m_ActionInArgument);
   DDX_Control(pDX, IDC_FINDBYTYPE, m_FindByType);
   DDX_Control(pDX, IDC_FINDBYUDN, m_FindByUDN);
   DDX_Control(pDX, IDC_ASYNCFIND, m_AsyncFind);
   DDX_Control(pDX, IDC_ASYNCH, m_AsynchControl);
   DDX_Control(pDX, IDC_DELAYSUBSCRIPTION, m_DelaySubscription);
   //}}AFX_DATA_MAP
}

//Message Map
BEGIN_MESSAGE_MAP(CGenericUCPDlg, CDialog)
   //{{AFX_MSG_MAP(CGenericUCPDlg)
   ON_WM_SYSCOMMAND()
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   ON_BN_CLICKED(IDC_VIEWSCPD, OnViewSCPD)
   ON_BN_CLICKED(IDC_FINDBYTYPE, OnFindByType)
   ON_BN_CLICKED(IDC_DISCOVERY, OnDiscoveryClicked)
   ON_BN_CLICKED(IDC_FINDBYUDN, OnFindbyUDN)
   ON_BN_CLICKED(IDC_ASYNCFIND, OnAsyncFind)
   ON_CBN_SELENDOK(IDC_COMBODEVICE, OnSelEndOkComboDevice)
   ON_CBN_SELENDOK(IDC_COMBOSERVICE, OnSelEndOkComboService)
   ON_BN_CLICKED(IDC_DEVPROP, OnDevpropClick)
   ON_BN_CLICKED(IDC_QUERYVARIABLE, OnQueryVariable)
   ON_BN_CLICKED(IDC_INVOKEACTION, OnInvokeAction)
   ON_BN_CLICKED(IDC_SUBSCRIBE, OnSubscribe)
   ON_BN_CLICKED(IDC_CLOSE, OnCloseClick)
   ON_BN_CLICKED(IDC_ASYNCH, OnCheckBoxClick)
   ON_BN_CLICKED(IDC_DELAYSUBSCRIPTION, OnCheckBoxClick)
   ON_WM_CLOSE()
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGenericUCPDlg message handlers



//+---------------------------------------------------------------------------
//
//  Member:		OnInitDialog
//
//  Purpose:    Initialization of the dialog box. 
//
//  Arguments:
//				None
//
//  Returns:    TRUE
//
//  Notes:
//     Default to AsyncFind search and set the flag for m_fAsyncFindRunning to 
//     FALSE
//

BOOL CGenericUCPDlg::OnInitDialog()
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
      (void) strAboutMenu.LoadString(IDS_ABOUTBOX);
      if (!strAboutMenu.IsEmpty())
      {
         pSysMenu->AppendMenu(MF_SEPARATOR);
         pSysMenu->AppendMenu(
                              MF_STRING, 
                              IDM_ABOUTBOX, 
                              strAboutMenu
                              );
      }
   }

   // Set the icon for this dialog.  The framework does this automatically
   //  when the application's main window is not a dialog
   SetIcon(m_hIcon, TRUE);			// Set big icon
   SetIcon(m_hIcon, FALSE);		// Set small icon
   m_pDeviceFinder = NULL;
   m_fAsyncFindRunning = FALSE; // Async Find is not running
   m_DeviceCombo.SetCurSel(-1);
   m_ServiceCombo.SetCurSel(-1);
   m_AsynchControl.SetCheck(BST_CHECKED);       // Set the check button of the asynchronous control box
   m_DelaySubscription.SetCheck(BST_CHECKED);   // Set the check button of the delay subscription box
   m_Subscribe.EnableWindow(FALSE);             // Subscribe is disabled at the beginning
   m_ViewSCPD.EnableWindow(TRUE);               // Enable View SCPD button

   // Grow the size of the array five at a time	
   m_astrDevType.SetSize(5,5); 
   m_AsyncFind.SetCheck(TRUE);

   // Initialize the Device Types.
   InitializeDevTypeArray();

   // Tell the user to click on Start Discovery Button
   m_StatusText.SetWindowText(
         L"Choose the type of device discovery and proceed"
         );

   return TRUE;
}


//MFC function
BOOL CGenericUCPDlg::PreTranslateMessage(_In_ MSG *pMsg)
{
   if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN)
   {
      DWORD def_id=GetDefID();
      if (def_id!=0)
      {
         CWnd *wnd=FromHandle(pMsg->hwnd);
         // you may implement other ways of testing, e.g.
         //  comparing to array of CWnd*, comparing to array of IDs etc.
         WCHAR class_name[16];
         if (GetClassName(wnd->GetSafeHwnd(),class_name,sizeof(class_name)/sizeof(WCHAR))!=0)
         {
            if (_wcsnicmp(class_name,L"edit",5)==0)
            {
               return TRUE;
               // discard the message!
            }
         }
      }
   }
   // be a good citizen - call the base class
   return CDialog::PreTranslateMessage(pMsg);
}

// MFC generated function
void CGenericUCPDlg::OnSysCommand(_In_ UINT nID, _In_ LPARAM lParam)
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

void CGenericUCPDlg::OnPaint() 
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
HCURSOR CGenericUCPDlg::OnQueryDragIcon()
{
   return (HCURSOR) m_hIcon;
}


//+---------------------------------------------------------------------------
//
//  Member:		ClearDeviceCombo
//
//  Purpose:    Clears the device combo and releases the device pointer stored
//				in the list
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:
//

void CGenericUCPDlg::ClearDeviceCombo(void)
{
   IUPnPDevice *pDevice = NULL;

   // We are going to release the device pointers we have stored in ItemDataPtr of each item
   int nCount = m_DeviceCombo.GetCount();

   for (int i=0; i < nCount; i++){
      pDevice = (IUPnPDevice *) m_DeviceCombo.GetItemDataPtr(i);
      ReleaseObj(pDevice);
   }
   m_DeviceCombo.ResetContent();
}

//+---------------------------------------------------------------------------
//
//  Member:		ClearServiceCombo
//
//  Purpose:    Clears the service combo and releases the service pointer stored
//				in the list
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:
//

void CGenericUCPDlg::ClearServiceCombo(void)
{
   IUPnPService *pService = NULL;

   // We are going to release the service pointers we have stored in ItemDataPtr of each item
   int nCount = m_ServiceCombo.GetCount();

   for (int i=0; i < nCount; i++){
      pService = (IUPnPService *) m_ServiceCombo.GetItemDataPtr(i);
      ReleaseObj(pService);
   }
   m_ServiceCombo.ResetContent();
   m_EventText.SetWindowText(L""); // Clear  the event text
}

//+---------------------------------------------------------------------------
//
//  Member:		ClearAllDataStructures
//
//  Purpose:    Clears various combo boxes, sets the text of the labels
//
//  Arguments:
//		fClearFindCombo	[in]	Flag to clear the findCombo box. 
//
//  Returns:    None
//
//  Notes:
//


void CGenericUCPDlg::ClearAllDataStructures(_In_ BOOL fClearFindCombo)
{
   // We are going to clear all the combo boxes and all the COM interfaces.
   if (fClearFindCombo)
   {
      // Clear the find combo box
      m_FindCombo.ResetContent();
      m_astrDevType.RemoveAll();
   }

   ClearDeviceCombo();
   ClearServiceCombo();

   m_StatusText.SetWindowText(L"");
   m_EventText.SetWindowText(L"");
}


//+---------------------------------------------------------------------------
//
//  Member:		OnFindByType
//
//  Purpose:    Initialize the find combo box with the device types 
//		from devType.txt and stop async find if necessary
//
//  Arguments:
//		None
//
//  Returns:    None
//
//  Notes:	Called when FindByType radio button is clicked
//

void CGenericUCPDlg::OnFindByType() 
{
   // We are going to do a FindByType 
   // Hence we are going to load the FindCombo with the list of the types 
   // from devType.txt. 

   StopAsyncFindIfStarted();
   ClearAllDataStructures(TRUE);
   InitializeDevTypeArray();
}


//+---------------------------------------------------------------------------
//
//  Member:		OnFindByUDN
//
//  Purpose:    Initialize the find combo box with the UDNs
//				from udn.txt and stop async find if necessary
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		Called when FindByUDN radio button is clicked
//
void CGenericUCPDlg::OnFindbyUDN() 
{
   // We are going to do a FindByUDN
   // Hence we are going to load the FindCombo with the list of the types 
   // from UDN.txt. 
   StopAsyncFindIfStarted();
   ClearAllDataStructures(TRUE);
   InitializeUDNList();
}


//+---------------------------------------------------------------------------
//
//  Member:		OnAsyncFind
//
//  Purpose:    Initialize the find combo box with the device types 
//              from devType.txt and stop async find if necessary
//
//  Arguments:
//		None
//
//  Returns:    None
//
//  Notes:		Called when AsyncFind radio button is clicked
//
void CGenericUCPDlg::OnAsyncFind() 
{
   // We are going to do AsyncFind
   // Hence we are going to load the FindCombo with the list of the types 
   // from devType.txt. 
   StopAsyncFindIfStarted();
   ClearAllDataStructures(TRUE);
   InitializeDevTypeArray();
}


//+---------------------------------------------------------------------------
//
//  Member:		OnDiscoveryClicked
//
//  Purpose:    Start the discovery of the type of search requested
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		Called when "Start Discovery" button is clicked
//

void CGenericUCPDlg::OnDiscoveryClicked() 
{
   // Disable the StartDiscovey Button if the find is a synchronous find.
   if (m_FindByType.GetCheck() || m_FindByUDN.GetCheck())
   {
      m_StartDiscovery.EnableWindow(FALSE);
   }

   HRESULT hr = S_OK;
   CString strWindowString;
   
   StopAsyncFindIfStarted(); // If AsyncFind is in progress, stop it
   ClearAllDataStructures(FALSE); // Don't clear the find combo box

   m_StatusText.SetWindowText(L"Discovery Clicked");
   m_EventText.SetWindowText(L"");

   // Check if we chosen an item in the list 
   m_FindCombo.GetWindowText(strWindowString);
   if(strWindowString.IsEmpty() && m_FindCombo.GetCurSel()==-1)
   {
      MessageBox(L"Select one of the items from the list");
      
      if (m_FindByType.GetCheck() || m_FindByUDN.GetCheck())
      {
         m_StartDiscovery.EnableWindow(TRUE);
      }
      
      return;
   }

   // We have to get the search parameters
   CString strSearchParameter;
   if(m_FindByType.GetCheck() || m_AsyncFind.GetCheck())
   {
      if(m_FindCombo.GetCurSel()==-1)
      {
         strSearchParameter=strWindowString; // Get it from the edit box
      }
      else
      {
         strSearchParameter=m_astrDevType[m_FindCombo.GetCurSel()];
      }
   }
   else
   {
      if(m_FindByUDN.GetCheck())
      {
         strSearchParameter=strWindowString; 
      }
   }

   ReleaseObj(m_pDeviceFinder);
   m_pDeviceFinder = NULL;

   // Instantiate the device finder object
   hr = CoCreateInstance( CLSID_UPnPDeviceFinder,
                          NULL,
                          CLSCTX_SERVER,
                          IID_IUPnPDeviceFinder,
                          (LPVOID *)&m_pDeviceFinder);

   if(hr!=S_OK)
   {
      m_StatusText.SetWindowText(
      L"CoCreateInstance failed for device finder object"
      );
      PrintErrorText(hr);
      
      if (m_FindByType.GetCheck() || m_FindByUDN.GetCheck())
      {
         m_StartDiscovery.EnableWindow(TRUE);
      }
      
      return;
   }

   BSTR bstrSearchType = NULL;
   bstrSearchType = strSearchParameter.AllocSysString();
   if (bstrSearchType != NULL)
   {
      if(m_FindByType.GetCheck())
      {
         // We have to process the FindByType. 
         ProcessFindByType(bstrSearchType);
         m_StartDiscovery.EnableWindow(TRUE);
      }
      else if (m_FindByUDN.GetCheck())
      {
         // We have to process the FindByUDN. 
         ProcessFindByUDN(bstrSearchType);
         m_StartDiscovery.EnableWindow(TRUE);
      }
      else if (m_AsyncFind.GetCheck())
      {
         // We have to process the AsyncFind. 
         ProcessAsyncFind(bstrSearchType);
         SysFreeString(bstrSearchType);

         // We should not release the device finder object. 
         // Hence we are going to handle the exit here
         return;
      }
      m_StatusText.SetWindowText(L"End of discovery process");

      if(m_DeviceCombo.GetCount()>0)
      {
         m_DeviceCombo.SetCurSel(0);
         OnSelEndOkComboDevice();
      }

      SysFreeString(bstrSearchType);

      // We are not releasing the device finder interface in the case of 
      // async find
      ReleaseObj(m_pDeviceFinder);

      m_pDeviceFinder = NULL;
   }

}	


//+---------------------------------------------------------------------------
//
//  Member:		ProcessFindByType
//
//  Purpose:    Helper function for processing the FindByType search
//
//  Arguments:
//		bstrSearchType	[in]	Search type identifier
//
//  Returns:    None
//
//  Notes:		
//


void CGenericUCPDlg::ProcessFindByType(_In_ BSTR bstrSearchType)
{
   // We have to process FindByType search
   HRESULT hr = S_OK;
   IUPnPDevices* pDevices = NULL;
   IUnknown* punkEnum = NULL;
   IEnumUnknown *pEU = NULL; 

   m_StatusText.SetWindowText(L"FindByType selected");

   ASSERT(m_pDeviceFinder);
   if (m_pDeviceFinder == NULL)
   {
      m_StatusText.SetWindowText(L"Error: Missing DeviceFinder");
      return;
   }
   
   hr = m_pDeviceFinder->FindByType(bstrSearchType, 0,  &pDevices);
   if (SUCCEEDED(hr))
   {
      // We have to enumerate the devices
      long lCount; 
      hr = pDevices->get_Count(&lCount);
      if (SUCCEEDED(hr))
      {
         if (lCount != 0)
         {
            hr = pDevices->get__NewEnum(&punkEnum);
            if (SUCCEEDED(hr))
            {
               hr = punkEnum->QueryInterface(IID_IEnumUnknown, (VOID **) &pEU);
               if (SUCCEEDED(hr))
               {
                  for (long lIndex = 0; lIndex<lCount; lIndex++)
                  {
                     IUnknown* punkDevice = NULL;
                     IUPnPDevice* pDevice=NULL;
                     hr = pEU->Next(1, &punkDevice, NULL);
                     if (SUCCEEDED(hr))
                     {
                        // Get a IUPnPDevice pointer to the device just got
                        hr = punkDevice->QueryInterface(
                                             IID_IUPnPDevice, 
                                             (VOID **)&pDevice
                                             );
                        if (SUCCEEDED(hr))
                        {
                           // Add the found device to the device list
                           // Get the friendly name of the device
                           BSTR bstrFriendlyName = NULL;
                           hr = pDevice->get_FriendlyName(&bstrFriendlyName);
                           if (SUCCEEDED(hr))
                           {
                              WCHAR wszFriendlyName[DATA_BUFSIZE];
                              _snwprintf_s(
                                      wszFriendlyName, 
                                      DATA_BUFSIZE, 
                                      _TRUNCATE,
                                      L"%ls", 
                                      bstrFriendlyName
                                      );
                              m_DeviceCombo.AddString(wszFriendlyName);
                              m_DeviceCombo.SetItemDataPtr(
                                               (int)lIndex, 
                                                pDevice
                                               );
                              SysFreeString(bstrFriendlyName);
                           }
                           else
                           {
                              m_StatusText.SetWindowText(
                                L"Error: FriendlyName failed"
                                );
                              PrintErrorText(hr);
                              ReleaseObj(pDevice);
                              ReleaseObj(punkDevice);
                              break;
                           }
                        }
                        else
                        {
                           m_StatusText.SetWindowText(
                           L"Error: QueryInterface for IUPnPDevice failed"
                               );
                           PrintErrorText(hr); 
                           ReleaseObj(punkDevice);
                           break;
                        }
                        ReleaseObj(punkDevice);
                     }
                     else
                     {
                        m_StatusText.SetWindowText(
                                    L"Error: Traversing the device list using Next"
                                    L" method failed"
                                    );
                        PrintErrorText(hr);
                        break; // Get out of the for loop
                     }
                  } // For loop
                  ReleaseObj(pEU);
               }
               else
               {
                  m_StatusText.SetWindowText(
                           L"Error: QueryInterface for IID_IEnumUnknown failed"
                           );
                  PrintErrorText(hr);		
               }
               ReleaseObj(punkEnum);
            }
            else
            {
               m_StatusText.SetWindowText(
                     L"get__NewEnum in IUPnPDevices failed"
                     );
               PrintErrorText(hr);
            }
         }
         else
         {
            MessageBox(L"Found no devices");
         }
      }
      else
      {
         m_StatusText.SetWindowText(L"get_Count in IUPnPDevices failed");
         PrintErrorText(hr);
      }
      ReleaseObj(pDevices);
   }
   else
   {
      m_StatusText.SetWindowText(L"FindByType failed");
      PrintErrorText(hr);
   }
}


//+---------------------------------------------------------------------------
//
//  Member:		ProcessFindByUDN
//
//  Purpose:    Helper function for processing the FindByUDN search
//
//  Arguments:
//				bstrSearchType	[in]	Search type identifier
//
//  Returns:    None
//
//  Notes:		
//

void CGenericUCPDlg::ProcessFindByUDN(_In_ BSTR bstrSearchType)
{
   // FindByUDN selected
   HRESULT hr= S_OK;

   m_StatusText.SetWindowText(L"FindByUDN selected");

   ASSERT(m_pDeviceFinder);
   if (m_pDeviceFinder == NULL)
   {
      m_StatusText.SetWindowText(L"Error: Missing DeviceFinder");
      return;
   }

   IUPnPDevice* pDevice = NULL;
   hr = m_pDeviceFinder->FindByUDN(bstrSearchType, &pDevice);
   if(hr == S_OK)
   {
      // Found a device with given udn
      // Add the found device to the device list
      // Get the friendly name of the device
      BSTR bstrFriendlyName = NULL;
      hr = pDevice->get_FriendlyName(&bstrFriendlyName);
      if(SUCCEEDED(hr))
      {
         WCHAR wszFriendlyName[DATA_BUFSIZE];
         _snwprintf_s(wszFriendlyName, DATA_BUFSIZE, _TRUNCATE, L"%ls", bstrFriendlyName);
         m_DeviceCombo.AddString(wszFriendlyName);
         m_DeviceCombo.SetItemDataPtr(0, pDevice);	// Add as the first element			 
         SysFreeString(bstrFriendlyName);
      }
      else
      {
         m_StatusText.SetWindowText(L"Error: FriendlyName failed");
         PrintErrorText(hr);
      }
   }
   else
   {
      MessageBox(L"Found no device with given udn");
   }
}


//+---------------------------------------------------------------------------
//
//  Member:		ProcessAsyncFind
//
//  Purpose:    Helper function for processing the AsyncFind search
//
//  Arguments:
//				bstrSearchType	[in]	Search type identifier
//
//  Returns:    None
//
//  Notes:		
//
void CGenericUCPDlg::ProcessAsyncFind(_In_ BSTR bstrSearchType)
{
   HRESULT hr = S_OK;
   
   m_StatusText.SetWindowText(L"AsyncFind selected");

   ASSERT(m_pDeviceFinder);
   if (m_pDeviceFinder == NULL)
   {
      m_StatusText.SetWindowText(L"Error: Missing DeviceFinder");
      return;
   }

   // We have to start the AsyncFind.
   if (m_pDeviceFinderCallback!=NULL)
   {
      hr = m_pDeviceFinder->CreateAsyncFind(
                             bstrSearchType, 
                             NULL, 
                             m_pDeviceFinderCallback, 
                             &m_lAsyncFindHandle
                             );
      if (SUCCEEDED(hr))
      { 
         hr = m_pDeviceFinder->StartAsyncFind(m_lAsyncFindHandle);
         if (SUCCEEDED(hr))
         {
            m_fAsyncFindRunning = TRUE;
         }
         else
         {
            PrintErrorText(hr);
            m_StatusText.SetWindowText(
                       L"Error: StartAsyncFind failed"
                       );
            hr = m_pDeviceFinder->CancelAsyncFind(m_lAsyncFindHandle);
            if (FAILED(hr))
            { 
               m_StatusText.SetWindowText(L"CancelAsyncFind failed");
               PrintErrorText(hr);
            }
         } 
      }
      else
      {
         PrintErrorText(hr);
         m_StatusText.SetWindowText(L"Error: CreateAsyncFind failed");
      }
   }
   else
   {
      m_StatusText.SetWindowText(
                   L"DeviceFinderCallback object is not available"
                   );
   }
}


//+---------------------------------------------------------------------------
//
//  Member:	InitializeDevTypeArray
//
//  Purpose:    Helper function for initializing the find combo box
//		with list of device types
//
//  Arguments:
//		None
//
//  Returns:    None
//
//  Notes:	Called when FindByType or AsyncFind is clicked
//
void CGenericUCPDlg::InitializeDevTypeArray()
{
   // First remove all the stored elements in the devType array
   m_astrDevType.RemoveAll();

   // Also remove the items in the FindCombo 
   m_FindCombo.ResetContent();

   // Check whether the file devtype.txt exists
   if (!fFileExists(L"devtype.txt"))
   {
      // devtype.txt file doesn't exist
      m_StatusText.SetWindowText(L"Warning: devType.txt doesn't exist");
      return;
   }

   // We can start reading the file
   CStdioFile fileDevType;
   if (!fileDevType.Open(L"devtype.txt", CFile::modeRead|CFile::typeText))
   {
      MessageBox(L"Error: Can't open the devtype.txt file");
      return;
   }

   CString strLine; 
   while (fileDevType.ReadString(strLine))
   {
      // We have got one line at a time
      if (!strLine.IsEmpty())
      {
         // We are going to split the string first
         int iSpacePos;
         iSpacePos = strLine.Find(' '); // Find the first space character			
         if (iSpacePos == -1) //space character not found
         {
            MessageBox(
                 L"Error: Malformed line in devType.txt, no space " 
                 L"type and description"
                 );
            fileDevType.Close();
            return; 
         }

         CString strDeviceType = strLine.Left(iSpacePos);
         CString strDeviceDesc = strLine.Right(
                                         strLine.GetLength() - iSpacePos - 1
                                         );

         // We are going to add this to the szaDevType Array
         m_astrDevType.Add(strDeviceType); 
         //Also add the device description to the findCombo 
         m_FindCombo.AddString(strDeviceDesc);
      }
   }

   fileDevType.Close();
   m_FindCombo.SetCurSel(0); // Select the first item in the list
}


//+---------------------------------------------------------------------------
//
//  Member:		fFileExists
//
//  Purpose:    Find whether the given file exists
//
//  Arguments:
//	ptszFileName [in]	File name of the file to check
//
//  Returns:    TRUE or FALSE
//
//  Notes:		
//
BOOL CGenericUCPDlg::fFileExists(_In_ LPTSTR ptszFileName)
{
   DWORD   dwValue;
   dwValue = GetFileAttributes(ptszFileName); 

   if (dwValue == -1 )
   {
   // Return false if we are not able to get the file attributes
      return FALSE; 
   }
   else
   {
   return TRUE;
   }
}


//+---------------------------------------------------------------------------
//
//  Member:		InitializeUDNList
//
//  Purpose:    Helper function for initializing the find combo box
//				with list of UDNs
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		Called when FindByUDN is clicked
//
void CGenericUCPDlg::InitializeUDNList()
{
   // Also remove the items in the FindCombo 
   m_FindCombo.ResetContent();

   // Check whether the file udn.txt exists
   if (!fFileExists(L"udn.txt"))
   {
      // udn.txt file doesn't exist
      m_StatusText.SetWindowText(L"Warning: udn.txt doesn't exist");
      return;
   }

   // We can start reading the file

   CStdioFile fileDevType;
   if (!fileDevType.Open(L"udn.txt", CFile::modeRead|CFile::typeText))
   {
      MessageBox(L"Error: Can't open the udn.txt file");
      return;
   }

   CString strLine; 
   while (fileDevType.ReadString(strLine))
   {
      // We have got one line at a time
      if (!strLine.IsEmpty())
      {
         m_FindCombo.AddString(strLine);
      }
   }

   fileDevType.Close();
   m_FindCombo.SetCurSel(0); // Select the first item in the list

}


//+---------------------------------------------------------------------------
//
//  Member:		StopAsyncFindIfStarted
//
//  Purpose:    Helper function for stopping the async find if proceeding
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//
void CGenericUCPDlg::StopAsyncFindIfStarted()
{
   HRESULT hr = S_OK;

   // This will stop the async find if it is in progress
   if (m_fAsyncFindRunning == TRUE)
   {
      ASSERT(m_pDeviceFinder);
      if (m_pDeviceFinder == NULL)
      {
         m_StatusText.SetWindowText(L"Error: Missing DeviceFinder");
      }
      else
      {
         hr = m_pDeviceFinder->CancelAsyncFind(m_lAsyncFindHandle);
         if (FAILED(hr))
         {
            m_StatusText.SetWindowText(L"CancelAsyncFind failed");
            MessageBox(L"Cancel Async Find Failed.", L"Error");
         }
      }
      
      m_fAsyncFindRunning = FALSE;
   }
}


//+---------------------------------------------------------------------------
//
//  Member:		AddDevice
//
//  Purpose:    Helper function for adding devices to the device combo box
//
//  Arguments:
//		pDevice	[in]	COM interface pointer of the device being added
//
//  Returns:    None
//
//  Notes:
//     This is called by the devicefinder callback object (DeviceAdded func)
//
void CGenericUCPDlg::AddDevice(_In_ IUPnPDevice *pDevice)
{
   //We are going to add a device 
   HRESULT hr=S_OK;
   BSTR bstrFriendlyName = NULL;

   hr = pDevice->get_FriendlyName(&bstrFriendlyName);
   if (SUCCEEDED(hr))
   {
      pDevice->AddRef(); // Add Ref for the device
      WCHAR wszFriendlyName[DATA_BUFSIZE];
      WCHAR wszStatusString[DATA_BUFSIZE];
      _snwprintf_s(wszFriendlyName, DATA_BUFSIZE, _TRUNCATE, L"%ls", bstrFriendlyName);
      _snwprintf_s(
               wszStatusString,
               DATA_BUFSIZE, 
               _TRUNCATE,
               L"Added Device %ls", 
               wszFriendlyName
               );

      m_StatusText.SetWindowText(wszStatusString);

      int nCount = m_DeviceCombo.GetCount(); // Find the end of the list

      m_DeviceCombo.AddString(wszFriendlyName);

      // Explicitly AddRef for passing off the ownership to the combolist item.
      pDevice->AddRef();

      // Add the item at the end of the combolist
      m_DeviceCombo.SetItemDataPtr(nCount, pDevice);
      pDevice->Release(); // Release the current ownership of the pointer

      if (nCount==0)
      {
         m_DeviceCombo.SetCurSel(0);
         OnSelEndOkComboDevice();
      }

      SysFreeString(bstrFriendlyName);
   }
   else
   {
      PrintErrorText(hr);
      m_StatusText.SetWindowText(
                      L"Error: FriendlyName failed for device added failed"
                      );
   }
}


//+---------------------------------------------------------------------------
//
//  Member:	RemoveDevice
//
//  Purpose:    Helper function for removing device from the device combo box
//
//  Arguments:
//		bstrUDN	[in]	UDN of the device being deleted
//
//  Returns:    None
//
//  Notes:	
//     This is called by the devicefinder callback object (DeviceRemoved func)
//
void CGenericUCPDlg::RemoveDevice(_In_ BSTR bstrUDN)
{
   //We are going to remove the device
   HRESULT hr=S_OK;
   IUPnPDevice *pDevice = NULL;
   BSTR bstrTemp = NULL;
   BOOL bFound = FALSE;

   // We are going to release the device pointers we have stored in ItemDataPtr of each item
   int nCount = m_DeviceCombo.GetCount();

   for (int i=0; !bFound && i < nCount; i++)
   {
      pDevice = (IUPnPDevice *) m_DeviceCombo.GetItemDataPtr(i);

      pDevice->AddRef();

      // Check if the device pointer is the one we want
      hr=pDevice->get_UniqueDeviceName(&bstrTemp);

      if(SUCCEEDED(hr))
      {
         if(wcscmp(bstrTemp, bstrUDN)==0)
         {
            // This is the removed device
            ReleaseObj(pDevice); // Remove the reference that was taken when the device was added to the combo
            m_DeviceCombo.DeleteString(i);

            bFound = TRUE;
         }

         SysFreeString(bstrTemp);
         bstrTemp = NULL; 
      }
      else
      {
         PrintErrorText(hr);
         m_StatusText.SetWindowText(L"Error: GetUniqueDeviceName failed for the device in the list");
      }

      ReleaseObj(pDevice);
      pDevice = NULL;
   }
}


//+---------------------------------------------------------------------------
//
//  Member:		OnSelEndOkComboDevice
//
//  Purpose:    To populate the service list for the device selected
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//
void CGenericUCPDlg::OnSelEndOkComboDevice() 
{
   // Current Device is changed
   TRACE(L"SelendokDevice\n");

   HRESULT hr = S_OK;
   IUPnPDevice *pDevice=NULL;
   IUPnPServices* pServices = NULL;
   IUnknown *punkEnum = NULL;
   IEnumUnknown *pEU = NULL;
   IUPnPServiceEnumProperty *pServiceEnum = NULL;
   int iSelectedItem;
   long lIndex;
   m_StatusText.SetWindowText(L"");
   m_EventText.SetWindowText(L"");
   iSelectedItem = m_DeviceCombo.GetCurSel();
   if(iSelectedItem!=CB_ERR){
      pDevice=(IUPnPDevice *) m_DeviceCombo.GetItemDataPtr(iSelectedItem);
      pDevice->AddRef();
      ClearServiceCombo();
      hr = pDevice->get_Services(&pServices);
      if(hr==S_OK){
         long lCount;
         hr = pServices->get_Count(&lCount);
         if(SUCCEEDED(hr)){
            if(lCount!=0){
               hr = pServices->QueryInterface(IID_IUPnPServiceEnumProperty, (VOID **) &pServiceEnum); // Set enum property
               if(SUCCEEDED(hr))
               {
                  if (BST_CHECKED == m_DelaySubscription.GetCheck())
                  {
                     // Delayed SCPD download and event subscription
                     hr = pServiceEnum->SetServiceEnumProperty(UPNP_SERVICE_DELAY_SCPD_AND_SUBSCRIPTION);
                     if (FAILED(hr))
                     {
                        PrintErrorText(hr);
                        m_StatusText.SetWindowText(L"SetServiceEnumProperty in IUPnPServiceEnumProperty failed");
                     }
                     else
                     {
                        // Enable the button because it will be used to do subscriptions
                        m_Subscribe.EnableWindow(TRUE);
                     }
                  }
                  else
                  {
                     // Opt-out of delayed SCPD download - Reset the service enum property
                     hr = pServiceEnum->SetServiceEnumProperty(0);
                     if (FAILED(hr))
                     {
                        PrintErrorText(hr);
                        m_StatusText.SetWindowText(L"SetServiceEnumProperty in IUPnPServiceEnumProperty failed");
                     }
                     else
                     {
                        //Disable button since this will not delay the subscription
                        m_Subscribe.EnableWindow(FALSE);
                     }
                  }
                  ReleaseObj(pServiceEnum);
               }
               else
               {
                  PrintErrorText(hr);
                  m_StatusText.SetWindowText(L"Error: QueryInterface for IID_IUPnPServiceEnumProperty failed");
               }
               if (SUCCEEDED(hr)){
                  // We have to get a IEnumUnknown pointer
                  hr = pServices->get__NewEnum(&punkEnum);
                  if(SUCCEEDED(hr)){
                     hr = punkEnum->QueryInterface(IID_IEnumUnknown, (VOID **) &pEU);
                     if(SUCCEEDED(hr)){
                        for(lIndex = 0; lIndex<lCount; lIndex++){
                           IUnknown *punkService = NULL;
                           IUPnPService *pService=NULL;
                           hr = pEU->Next(1, &punkService, NULL);
                           if(SUCCEEDED(hr)){
                              // Get a IUPnPService pointer to the service just got
                              hr = punkService->QueryInterface(IID_IUPnPService, (VOID **)&pService);
                              if(SUCCEEDED(hr)){
                                 BSTR bstrServiceId = NULL;
                                 hr = pService->get_Id(&bstrServiceId);
                                 if(SUCCEEDED(hr)){
                                    WCHAR wszServiceId[DATA_BUFSIZE];
                                    _snwprintf_s(wszServiceId, DATA_BUFSIZE, _TRUNCATE, L"%ls", bstrServiceId);
                                    m_ServiceCombo.AddString(wszServiceId);
                                    m_ServiceCombo.SetItemDataPtr((int)lIndex, pService);
                                    SysFreeString(bstrServiceId);
                                 }
                                 else{
                                    PrintErrorText(hr);
                                    m_StatusText.SetWindowText(L"Error: get_Id failed");
                                    ReleaseObj(pService);
                                    ReleaseObj(punkService);
                                    break;
                                 }
                              }
                              else{
                                m_StatusText.SetWindowText(L"Error: QueryInterface for IUPnPService failed");
                                ReleaseObj(punkService);
                                break;
                              }
                              ReleaseObj(punkService);
                           }
                           else{
                              m_StatusText.SetWindowText(L"Error: Traversing the service list using Next method failed");
                              break; // Get out of the for loop
                           }

                        }//For loop
                        if(m_ServiceCombo.GetCount()>0){
                           m_ServiceCombo.SetCurSel(0);
                           OnSelEndOkComboService();
                        }
                        ReleaseObj(pEU);
                     }
                     else{
                        PrintErrorText(hr);
                        m_StatusText.SetWindowText(L"Error: QueryInterface for IID_IEnumUnknown failed");
                     }
                     ReleaseObj(punkEnum);
                  }
                  else{
                     PrintErrorText(hr);
                     m_StatusText.SetWindowText(L"get__NewEnum in IUPnPServices failed");
                  }
               }
            }
            else{
               MessageBox(L"Found no services");
            }
         }
         else{
            PrintErrorText(hr);
            m_StatusText.SetWindowText(L"get_Count in IUPnPServices failed");
         }
         ReleaseObj(pServices);
      }
      else{
         PrintErrorText(hr);
         m_StatusText.SetWindowText(L"Error: Getting services for the current device failed");
      }
   ReleaseObj(pDevice); // Release the pDevice which is the currently selected device
   }
}

//+---------------------------------------------------------------------------
//
//  Member:		OnSelEndOkComboService
//
//  Purpose:    To add the callback for the service selected
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//
void CGenericUCPDlg::OnSelEndOkComboService() 
{
   if (BST_UNCHECKED == m_DelaySubscription.GetCheck())
   {
      OnSubscribe();
   }
}


//+---------------------------------------------------------------------------
//
//  Member:	OnDevpropClick
//
//  Purpose:    Show the properties of the selected device
//
//  Arguments:
//		None
//
//  Returns:    None
//
//  Notes:	
//      This starts up the modal CDeviceProp dialog and displays the properties
//
void CGenericUCPDlg::OnDevpropClick() 
{
   int iSelectedDevice;
   IUPnPDevice* pDevice = NULL;

   // We have to show the properties of the current device
   iSelectedDevice = m_DeviceCombo.GetCurSel();
   if (iSelectedDevice != CB_ERR)
   {
      pDevice = 
            (IUPnPDevice*)m_DeviceCombo.GetItemDataPtr(iSelectedDevice);
      CDeviceProp dlg(pDevice);
      dlg.DoModal();
   }
   else
   {
      MessageBox(L"No device selected");
   }

}


//+---------------------------------------------------------------------------
//
//  Member:		OnSubscribe
//
//  Purpose:    Subscribe for the selected service in the service list
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//
void CGenericUCPDlg::OnSubscribe()
{
   HRESULT hr = S_OK;
   IUPnPService* pService = NULL;

   int iSelectedItem ;

   iSelectedItem = m_ServiceCombo.GetCurSel();

   if (iSelectedItem!=CB_ERR)
   {
      pService=(IUPnPService*)m_ServiceCombo.GetItemDataPtr(
                                                    m_ServiceCombo.GetCurSel()
                                                    );
      pService->AddRef();
      m_EventText.SetWindowText(L"");

      // Synchronous service subscription
      if (BST_UNCHECKED == m_AsynchControl.GetCheck())
      {
         hr = pService->AddCallback(m_pServiceCallback);
      }

      // Asynchronous service subscription
      else if (BST_CHECKED == m_AsynchControl.GetCheck())
      {
         IUPnPServiceAsync * pServiceAsync = NULL;

         // Get a IUPnPServiceAsync pointer to the service
         hr = pService->QueryInterface(IID_IUPnPServiceAsync, (VOID **)&pServiceAsync);
         
         if(FAILED(hr))
         {
            PrintErrorText(hr);
            m_StatusText.SetWindowText(L"Error: QueryInterface for IUPnPServiceAsync failed.");
         }
         else
         {
            CUPnPAsyncResult* pAsyncResult = NULL;
            HANDLE  searchEvents[2]; // Array of events to wait for. This array is passed as a argument to CoWaitForMultipleHandles().
            DWORD   dwSignalled = 0;
            DWORD   dwWaitTime = INFINITE;
            DWORD   dwEventCount = 2;
            MSG msg;

            pAsyncResult = new CUPnPAsyncResult(); 
            pAsyncResult->AddRef();
            hr = pAsyncResult->Init();

            if(SUCCEEDED(hr))
            {
               searchEvents[0] = pAsyncResult->GetHandle();
               searchEvents[1] = m_hCloseEvent;

               hr = pServiceAsync->BeginSubscribeToEvents(m_pServiceCallback, pAsyncResult, &m_ullRequestID); 

               if(SUCCEEDED(hr))
               {
                  while(TRUE)
                  {
                     // Wait for multiple objects.
                     DWORD dwWaitResult = ::MsgWaitForMultipleObjectsEx( dwEventCount, 
                                                                     searchEvents, 
                                                                     dwWaitTime, 
                                                                     QS_ALLINPUT, 
                                                                     MWMO_INPUTAVAILABLE);

                                         
                     if(WAIT_FAILED == dwWaitResult)
                     {
                        DWORD dwError = GetLastError();
                        hr = HRESULT_FROM_WIN32(dwError);
                        m_StatusText.SetWindowText(
                                                 L"Wait Failed."
                                                 );
                        break;
                     }
                     else if(WAIT_TIMEOUT == dwWaitResult)
                     {
                        hr = pServiceAsync->CancelAsyncOperation(m_ullRequestID);
                        break;
                     }
                     else if(WAIT_OBJECT_0 == dwWaitResult)
                     {
                        if(0 == dwSignalled)    // Subscriube to events completion event triggerred..
                        {
                           hr = pServiceAsync->EndSubscribeToEvents( m_ullRequestID );
                        }
                        break;
                     }
                     else if(WAIT_OBJECT_0 + 1 == dwWaitResult)
                     {
                        // Received Close Event. Exit and prepare to close
                        hr = pServiceAsync->CancelAsyncOperation(m_ullRequestID);
                        break;
                     }

                     // Process any MSG 
                     if (::PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))         
                     {
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                        continue;
                     }
                  }
               }
               else
               {
                  PrintErrorText(hr);
               }
            }
            ReleaseObj(pServiceAsync);
         }

      }
      if (FAILED(hr))
      {
         PrintErrorText(hr);
         m_StatusText.SetWindowText(L"Error: AddCallback failed");
      }
      ReleaseObj(pService);
   }
}

//+---------------------------------------------------------------------------
//
//  Member:		OnViewSCPD
//
//  Purpose:    Displays the SCPD document in a Messagebox
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//
void CGenericUCPDlg::OnViewSCPD()
{
   HRESULT hr = S_OK;
   int iCurrentDevice, iCurrentService;

   VARIANT varValue;
   VariantInit(&varValue);
   IUPnPService* pService = NULL;
   IUPnPServiceDocumentAccess * pServiceAccess = NULL;
   // We are going to query the variable which is present in the variable 
   // edit box
   m_ActionOutArgument.SetWindowText(L"");
   
   iCurrentDevice = m_DeviceCombo.GetCurSel();
   
   if (iCurrentDevice != CB_ERR)
   {
      iCurrentService = m_ServiceCombo.GetCurSel();
      if (iCurrentService != CB_ERR)
      {
         pService=(IUPnPService*)m_ServiceCombo.GetItemDataPtr(
                                       iCurrentService
                                       );
         pService->AddRef();

         hr = pService->QueryInterface(IID_IUPnPServiceDocumentAccess, (VOID **)&pServiceAccess);
         if(FAILED(hr))
         {
            m_StatusText.SetWindowText(L"Error: QueryInterface for IUPnPServiceDocumentAccess failed.");
         }
         else
         {
            //Use the IUPnPServiceDocumentAccess interface methods to retrieve the SCPD URL and text
            
            BSTR bstrDocURL = NULL;
            BSTR bstrDocument = NULL;   
            WCHAR wszMessage[DATA_BUFSIZE];

            hr = pServiceAccess->GetDocumentURL(&bstrDocURL);

            if (S_OK == hr)
            {
               hr = pServiceAccess->GetDocument(&bstrDocument);

               _snwprintf_s(
                           wszMessage,
                           DATA_BUFSIZE, 
                           _TRUNCATE,
                           L"%ls", 
                           bstrDocURL
                           );
               m_SCPDURL.SetWindowText(wszMessage);
               
               m_StatusText.SetWindowText(
                           L"Document URL retrieved..."
                              L"Downloading Document."
                           );

               SysFreeString(bstrDocURL);
            }

            if (S_OK == hr)
            {
                CSCPDDisplay dlg(bstrDocument);
                dlg.DoModal();
                
                m_StatusText.SetWindowText(
                           L"Document Retrieval Completed"
                           );

                SysFreeString(bstrDocument);
            }
            
            ReleaseObj(pServiceAccess);
         }

         ReleaseObj(pService);        
      }
   }


}

//+---------------------------------------------------------------------------
//
//  Member:		OnQueryVariable
//
//  Purpose:    Determines how to query the variable for the selected service in the service list
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//
void CGenericUCPDlg::OnQueryVariable()
{
   CString strVariable;
   int iCurrentDevice, iCurrentService;

   // We are going to query the variable which is present in the variable 
   // edit box
   m_ActionOutArgument.SetWindowText(L"");

   iCurrentDevice = m_DeviceCombo.GetCurSel();

   if (iCurrentDevice != CB_ERR)
   {
      iCurrentService = m_ServiceCombo.GetCurSel();
      if (iCurrentService != CB_ERR)
      {
         m_VariableName.GetWindowText(strVariable);
         if (!strVariable.IsEmpty())
         {
            BSTR bstrVariableName = NULL;
            bstrVariableName=strVariable.AllocSysString(); 
            if (bstrVariableName!=NULL)
            {

               if (BST_CHECKED == m_AsynchControl.GetCheck())
               {
                  OnQueryVariableAsync(bstrVariableName, iCurrentService);
               }
               else
               {
                  OnQueryVariableSync(bstrVariableName, iCurrentService);
               }

               SysFreeString(bstrVariableName);
            }
            else
            {
               MessageBox(L"Error: Could not allocate the memory for BSTR");
            }
         }
         else
         {
            MessageBox(L"Please specify a query variable");
         }
      }
      else
      {
         MessageBox(L"No service selected");
      }
   }
   else
   {
      MessageBox(L"No device selected");
   }
}


//+---------------------------------------------------------------------------
//
//  Member:		OnQueryVariableSync
//
//  Purpose:    Query the variable for the selected service in the service list using the synchronous methods
//
//  Arguments:
//                  bstrVariableName      [in]    Variable name as obtained from the field
//                  iCurrentService         [in]    Denotes which service was selected on the service list
//
//  Returns:    None
//
//  Notes:		Displays the result in the action out edit box.
//
void CGenericUCPDlg::OnQueryVariableAsync(
   _In_ BSTR bstrVariableName,
   _In_ int iCurrentService
) 
{
   HRESULT hr=S_OK;
   WCHAR wszMessage[DATA_BUFSIZE];

   VARIANT varValue;
   VariantInit(&varValue);
   IUPnPService* pService = NULL;
   IUPnPServiceAsync * pServiceAsync = NULL;

   pService=(IUPnPService*)m_ServiceCombo.GetItemDataPtr(
                                        iCurrentService
                                        );
   pService->AddRef();

   // Get a IUPnPServiceAsync pointer to the service just got
   hr = pService->QueryInterface(IID_IUPnPServiceAsync, (VOID **)&pServiceAsync);
   if(FAILED(hr))
   {
      m_StatusText.SetWindowText(L"Error: QueryInterface for IUPnPServiceAsync failed.");
   }
   else
   {

      CUPnPAsyncResult * pAsyncResult = NULL;
      HANDLE  searchEvents[2]; // Array of events to wait for. This array is passed as a argument to CoWaitForMultipleHandles().
      DWORD   dwSignalled = 0;
      DWORD   dwWaitTime = INFINITE;
      DWORD   dwEventCount = 2;
      MSG msg;

      pAsyncResult = new CUPnPAsyncResult(); 
      pAsyncResult->AddRef();
      hr = pAsyncResult->Init();

      if(SUCCEEDED(hr))
      {
         searchEvents[0] = pAsyncResult->GetHandle();
         searchEvents[1] = m_hCloseEvent;

         hr = pServiceAsync->BeginQueryStateVariable(bstrVariableName, pAsyncResult, &m_ullRequestID); 
         m_StatusText.SetWindowText(
                                   L"State var value being queried..."
                                      L"Waiting for completion callback."
                                   );

         if(SUCCEEDED(hr))
         {
            while(TRUE)
            {
               // Wait for multiple objects. We should wait for windows UI calls too. 
               DWORD dwWaitResult = ::MsgWaitForMultipleObjectsEx( dwEventCount, 
                                                                    searchEvents, 
                                                                    dwWaitTime, 
                                                                    QS_ALLINPUT, 
                                                                    MWMO_INPUTAVAILABLE);

               if(WAIT_FAILED == dwWaitResult)
               {
                  DWORD dwError = GetLastError();
                  hr = HRESULT_FROM_WIN32(dwError);
                  m_StatusText.SetWindowText(
                                            L"Wait Failed."
                                            );
                  break;
               }
           else if(WAIT_TIMEOUT == dwWaitResult)
               {
                  m_StatusText.SetWindowText(
                                         L"Timeout period elapsed."
                                            L" Cancelling Query operation now."
                                         );
                                                  
                  hr = pServiceAsync->CancelAsyncOperation(m_ullRequestID);
                  break;
               }
               else if(WAIT_OBJECT_0 == dwWaitResult)
               {
                  if(0 == dwSignalled)    // Invoke action completion event triggerred..
                  {
                     m_StatusText.SetWindowText(
                                             L"Action successfully invoked."
                                                L" Fetching value now."
                                             );

                     hr = pServiceAsync->EndQueryStateVariable( m_ullRequestID, &varValue  ); 
                     if (SUCCEEDED(hr))
                     {
                        hr = VariantChangeType(
                                              &varValue, 
                                              &varValue, 
                                              VARIANT_ALPHABOOL, 
                                              VT_BSTR
                                              );

                        if (SUCCEEDED(hr))
                        {
                           _snwprintf_s(
                                       wszMessage,
                                       DATA_BUFSIZE, 
                                       _TRUNCATE,
                                       L"%ls", 
                                       varValue.bstrVal
                                       );
                           m_ActionOutArgument.SetWindowText(wszMessage);
                        }
                        else
                        {
                           PrintErrorText(hr);
                        }
                     }
                     else
                     {
                        PrintErrorText(hr);
                     }

                  }
                  break;

               }
               else if (WAIT_OBJECT_0 + 1 == dwWaitResult)
               {
                  // Received Close Event. Exit and prepare to close
                  hr = pServiceAsync->CancelAsyncOperation(m_ullRequestID);
                  break;
               }

               // Process any MSG 
               if (::PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))         
               {
                  ::TranslateMessage(&msg);
                  ::DispatchMessage(&msg);
                  continue;
               }
            }
         }
         else
         {
            PrintErrorText(hr);
         }
      }
      ReleaseObj(pServiceAsync);
   }

   ReleaseObj(pService);
   VariantClear (&varValue);

}


//+---------------------------------------------------------------------------
//
//  Member:		OnQueryVariableSync
//
//  Purpose:    Query the variable for the selected service in the service list using the synchronous methods
//
//  Arguments:
//                  bstrVariableName      [in]    Variable name as obtained from the field
//                  iCurrentService         [in]    Denotes which service was selected on the service list
//
//  Returns:    None
//
//  Notes:		Displays the result in the action out edit box.
//
void CGenericUCPDlg::OnQueryVariableSync(
   _In_ BSTR bstrVariableName,
   _In_ int iCurrentService
   )
{
   HRESULT hr=S_OK;
   WCHAR wszMessage[DATA_BUFSIZE];

   VARIANT varValue;
   VariantInit(&varValue);
   IUPnPService* pService;
   pService=(IUPnPService*)m_ServiceCombo.GetItemDataPtr(
                                                 iCurrentService
                                                 );
   pService->AddRef();
   hr = pService->QueryStateVariable(bstrVariableName, &varValue);

   if (SUCCEEDED(hr))
   {
      hr = VariantChangeType(
            &varValue, 
            &varValue, 
            VARIANT_ALPHABOOL, 
            VT_BSTR
            );

      if (SUCCEEDED(hr))
      {
         _snwprintf_s(
              wszMessage,
              DATA_BUFSIZE, 
              _TRUNCATE,
              L"%ls", 
              varValue.bstrVal
              );
         m_ActionOutArgument.SetWindowText(wszMessage);
      }
      else
      {
         PrintErrorText(hr);
      }
   }
   else
       {
          PrintErrorText(hr);
   }

   ReleaseObj(pService);
   VariantClear (&varValue);

}


//+---------------------------------------------------------------------------
//
//  Member:		OnInvokeAction
//
//  Purpose:    Determines how to invoke the action for the selected service in the service list
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		Displays the result in the action out edit box.
//
void CGenericUCPDlg::OnInvokeAction()
{
   CString strAction;
   CString strArg;
   WCHAR wszString[DATA_BUFSIZE] = {0};
   WCHAR wszTempString[DATA_BUFSIZE] = {0};
   int iCurrentDevice, iCurrentService;
   int i, iCount;

   // We are going to query the variable which is present in the variable edit box
   m_ActionOutArgument.SetWindowText(L"");

   iCurrentDevice=m_DeviceCombo.GetCurSel();
   if(iCurrentDevice!=CB_ERR)
   {
      iCurrentService=m_ServiceCombo.GetCurSel();
      if(iCurrentService!=CB_ERR)
      {
         m_ActionName.GetWindowText(strAction);
         if(!strAction.IsEmpty())
         {
            BSTR bstrActionName = NULL;
            bstrActionName=strAction.AllocSysString(); 
            if(bstrActionName!=NULL)
            {
               m_ActionInArgument.GetWindowText(
                                    wszString, 
                                    DATA_BUFSIZE - 1
                                    );
               
               // Make a copy of the read string so that the main string 
               // is not modified
               SafeStrCopy(wszTempString, wszString, DATA_BUFSIZE);
               
               int iArgs = iNumberOfInArguments(wszTempString); // Get the number of arguments in the inArguments edit box
               
               VARIANT  vaActionArgs, vaArray;
               VARIANT**          ppVars = NULL;
               SAFEARRAY          *psaArgs = NULL;
               long               lPos;
               
               //Initialize all the variants
               VariantInit(&vaActionArgs);
               VariantInit(&vaArray);
               
               if(!(FAILED(HrCreateSafeArray(VT_VARIANT, iArgs, &psaArgs))||
                     FAILED(HrCreateArgVariants(iArgs, &ppVars))))
               {
                  VariantSetArray(psaArgs, vaArray);
                  VariantSetVar(&vaArray, vaActionArgs);
                  vaArray.pparray=&psaArgs;
               
                  // Get the in arguments 
                  // Put the arguments as strings
               
                  WCHAR *token = NULL;
                  WCHAR *context = NULL;
                  iCount=0; // Initialize the counter
                  token = wcstok_s(wszString, L" ", &context);
                  while((NULL != token) && (iCount < iArgs) && (NULL != ppVars))
                  {
                     ASSERT(iCount<iArgs); // Make sure
                     strArg=token;
                     ppVars[iCount]->vt=VT_BSTR;
                     ppVars[iCount]->bstrVal=strArg.AllocSysString(); // Converting the argument into a BSTR
                     token = wcstok_s(NULL, L" ", &context); // We have the space character as the separator
                     iCount++;
                  }
               
                  for(i=0; i<iArgs; i++)
                  {
                     lPos=i+1;
                     if (NULL != ppVars)
                     {
                         SafeArrayPutElement(psaArgs, &lPos, ppVars[i]);
                     }
                  }

                  if (BST_CHECKED == m_AsynchControl.GetCheck())
                  {
                     OnInvokeAsyncAction(bstrActionName, iCurrentService, vaActionArgs);
                  }
                  else
                  {
                     OnInvokeSyncAction(bstrActionName, iCurrentService, vaActionArgs);
                  }

                  if(NULL != ppVars)
                     HrDestroyArgVariants(iArgs, &ppVars); 
                  if(NULL != psaArgs)
                  {
                     SafeArrayDestroy(psaArgs);
                  }
               }


               SysFreeString(bstrActionName);
            }
            else
            {
               MessageBox(L"Error: Could not allocate the memory for BSTR");
            }
         }
         else
         {
            MessageBox(L"Please specify an action name");
         }
      }
      else
      {
         MessageBox(L"No service selected");
      }
   }
   else
   {
      MessageBox(L"No device selected");
   }
}

//+---------------------------------------------------------------------------
//
//  Member:		OnInvokeAsyncAction
//
//  Purpose:    Asynchronously invoke the action for the selected service in the service list
//
//  Arguments:
//                  bstrVariableName      [in]    Variable name as obtained from the field
//                  iCurrentService         [in]    Denotes which service was selected on the service list
//                  vaActionArgs             [in]    Contains all of the input arguments for InvokeAction
//
//  Returns:    None
//
//  Notes:		Displays the result in the action out edit box.
//
void CGenericUCPDlg::OnInvokeAsyncAction(
   _In_ BSTR bstrActionName,
   _In_ int iCurrentService,
   _In_ VARIANT vaActionArgs
   ) 
{
   HRESULT hr=S_OK;
   WCHAR wszString[DATA_BUFSIZE];
   WCHAR wszTempString[DATA_BUFSIZE];
   long lIndex;
   VARIANT vaOutArgs, vaRet, vaOutElement;
   long    lLBound = 0L, lUBound = 0L; // For out arguments

   //Initialize all the variants
   VariantInit(&vaOutArgs);
   VariantInit(&vaRet);
   VariantInit(&vaOutElement);

   IUPnPService * pService = NULL;
   IUPnPServiceAsync * pServiceAsync = NULL;

   pService = (IUPnPService *)m_ServiceCombo.GetItemDataPtr(iCurrentService);
   pService->AddRef();

    
   // Get a IUPnPServiceAsync pointer to the service just got
   hr = pService->QueryInterface(IID_IUPnPServiceAsync, (VOID **)&pServiceAsync);
   if(FAILED(hr))
   {
      m_StatusText.SetWindowText(L"Error: QueryInterface for IUPnPServiceAsync failed.");
   }

   if(SUCCEEDED(hr))
   {

      CUPnPAsyncResult * pAsyncResult = NULL;
      HANDLE  searchEvents[2]; // Array of events to wait for. This array is passed as a argument to CoWaitForMultipleHandles().
      DWORD   dwSignalled = 0;
      DWORD   dwWaitTime = INFINITE;
      DWORD   dwEventCount = 2;
      MSG msg;

      pAsyncResult = new CUPnPAsyncResult(); 
      pAsyncResult->AddRef();
      hr = pAsyncResult->Init();

      if(SUCCEEDED(hr))
         {
         searchEvents[0] = pAsyncResult->GetHandle();
         searchEvents[1] = m_hCloseEvent;

         hr = pServiceAsync->BeginInvokeAction(bstrActionName,
                                               vaActionArgs,
                                               pAsyncResult,
                                               &m_ullRequestID);

         m_StatusText.SetWindowText(
                              L"Action being invoked..."
                               L"Waiting for action completion callback."
                              );

         if(SUCCEEDED(hr))
         {
            while(TRUE)
            {
               // Wait for multiple objects. We should wait for windows UI calls too. 
               DWORD dwWaitResult = ::MsgWaitForMultipleObjectsEx( dwEventCount, 
                                                                   searchEvents, 
                                                                   dwWaitTime, 
                                                                   QS_ALLINPUT, 
                                                                   MWMO_INPUTAVAILABLE);


               if(WAIT_FAILED == dwWaitResult)
               {
                  DWORD dwError = GetLastError();
                  hr = HRESULT_FROM_WIN32(dwError);
                  m_StatusText.SetWindowText(
                                           L"Wait Failed."
                                           );
                  break;
               }
               else if(WAIT_TIMEOUT == dwWaitResult)
               {
                  m_StatusText.SetWindowText(
                                           L"Timeout period elapsed."
                                              L" Cancelling invoke action operation now."
                                           );
                  hr = pServiceAsync->CancelAsyncOperation(m_ullRequestID);
                  break;
               }
               else if(WAIT_OBJECT_0 == dwWaitResult)
               {
                  if(0 == dwSignalled)    // Invoke action completion event triggerred..
                  {
                     m_StatusText.SetWindowText(
                                               L"Action successfully invoked."
                                                  L" Fetching output arguments now."
                                               );

                     hr = pServiceAsync->EndInvokeAction( m_ullRequestID, &vaOutArgs, &vaRet  ); 

                     if(SUCCEEDED(hr))
                     {

                        // We have to print the return value and out arguments
                        *wszString=0; // Empty the string
                        *wszTempString=0; // Empty the string

                        if(vaRet.vt!=VT_EMPTY)
                        {
                           hr=VariantChangeType(&vaRet, &vaRet, VARIANT_ALPHABOOL, VT_BSTR);
                           if(SUCCEEDED(hr))
                           {
                              _snwprintf_s(
                                    wszTempString, 
                                    DATA_BUFSIZE, 
                                    _TRUNCATE,
                                    L"%ls ", 
                                    vaRet.bstrVal
                                    );
                              wcsncat_s(
                                    wszString, 
                                    DATA_BUFSIZE,
                                    wszTempString, 
                                    DATA_BUFSIZE - 1
                                    );
                           }
                           else
                           {
                              PrintErrorText(hr);
                              m_StatusText.SetWindowText(
                                   L"Error getting the return value for"
                                      L" the action invoked"
                                   );
                           }
                        }

                        // We have to process the out arguments now
                        hr = HrGetSafeArrayBounds(
                                vaOutArgs.parray, 
                                &lLBound, 
                                &lUBound
                                );
                        if(SUCCEEDED(hr))
                        {
                           // We have got the bounds of the arguments
                           for (lIndex = lLBound; 
                                lIndex<=lUBound; 
                                ++lIndex
                                )
                           {
                              VariantClear(&vaOutElement);
                              hr = HrGetVariantElement(
                                        vaOutArgs.parray, 
                                        lIndex, 
                                        &vaOutElement
                                        );
                              if(SUCCEEDED(hr))
                              {
                                 hr = VariantChangeType(
                                         &vaOutElement, 
                                         &vaOutElement, 
                                         VARIANT_ALPHABOOL, 
                                         VT_BSTR
                                         );
                                 if (SUCCEEDED(hr))
                                 {
                                    _snwprintf_s(
                                        wszTempString, 
                                        DATA_BUFSIZE, 
                                        _TRUNCATE,
                                        L"%ls ", 
                                        vaOutElement.bstrVal
                                        );
                                     wcsncat_s(
                                         wszString, 
                                         DATA_BUFSIZE,
                                         wszTempString,
                                         (DATA_BUFSIZE - 
                                          wcslen(wszString) - 1)
                                         );
                                 }
                                 else
                                 {
                                    PrintErrorText(hr);
                                    m_StatusText.SetWindowText(
                                        L"Error changing the out"
                                           L" argument type for the"
                                           L" action invoked"
                                          );
                                 }
                              }
                              else
                              {
                                 m_StatusText.SetWindowText(L"Error getting the out argument for the action invoked");
                                 PrintErrorText(hr);
                              }
                           } // For loop

                           VariantClear(&vaOutElement);
                        }

                        m_ActionOutArgument.SetWindowText(wszString); // Print the return value and out arguments.
                     }
                     else
                     {
                        PrintErrorText(hr);
                     }

                  }

                  break;

               }
               else if (WAIT_OBJECT_0 + 1 == dwWaitResult)
               {
                  // Received Close Event. Exit and prepare to close
                  hr = pServiceAsync->CancelAsyncOperation(m_ullRequestID); 
                  break;
               }

               // Process any MSG 
               if (::PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))         
               {
                  ::TranslateMessage(&msg);
                  ::DispatchMessage(&msg);
                  continue;
               }

            }
         }
         else
         {
            PrintErrorText(hr);
         }
      }
      ReleaseObj(pServiceAsync);
   }
   else
   {
      PrintErrorText(hr);
   }

   ReleaseObj(pService);

   VariantClear(&vaOutArgs);
   VariantClear(&vaRet);
   VariantClear(&vaOutElement);
}


//+---------------------------------------------------------------------------
//
//  Member:		OnInvokeSyncAction
//
//  Purpose:    Synchronously invoke the action for the selected service in the service list
//
//  Arguments:
//                  bstrVariableName      [in]    Variable name as obtained from the field
//                  iCurrentService         [in]    Denotes which service was selected on the service list
//                  vaActionArgs             [in]    Contains all of the input arguments for InvokeAction
//
//  Returns:    None
//
//  Notes:		Displays the result in the action out edit box.
//

void CGenericUCPDlg::OnInvokeSyncAction(
   _In_ BSTR bstrActionName,
   _In_ int iCurrentService,
   _In_ VARIANT vaActionArgs
   ) 
{
   HRESULT hr=S_OK;
   WCHAR wszString[DATA_BUFSIZE];
   WCHAR wszTempString[DATA_BUFSIZE];
   long lIndex;
   VARIANT vaOutArgs, vaRet, vaOutElement;
   long    lLBound = 0L, lUBound = 0L; // For out arguments
   
   //Initialize all the variants
   VariantInit(&vaOutArgs);
   VariantInit(&vaRet);
   VariantInit(&vaOutElement);

   IUPnPService *pService=NULL;
   pService = (IUPnPService *)m_ServiceCombo.GetItemDataPtr(iCurrentService);
   pService->AddRef();
   hr = pService->InvokeAction( bstrActionName,
                                vaActionArgs,
                                &vaOutArgs,
                                &vaRet);

   if(SUCCEEDED(hr))
   {
      // We have to print the return value and out arguments
      *wszString=0; // Empty the string
      *wszTempString=0; // Empty the string

      if(vaRet.vt!=VT_EMPTY)
      {
         hr=VariantChangeType(&vaRet, &vaRet, VARIANT_ALPHABOOL, VT_BSTR);
         if(SUCCEEDED(hr))
         {
            _snwprintf_s(
                   wszTempString, 
                   DATA_BUFSIZE, 
                   _TRUNCATE,
                   L"%ls ", 
                   vaRet.bstrVal
                   );
            wcsncat_s(
                   wszString, 
                   DATA_BUFSIZE,
                   wszTempString, 
                   DATA_BUFSIZE - 1
                   );
         }
         else
         {
            PrintErrorText(hr);
            m_StatusText.SetWindowText(
               L"Error getting the return value for"
                  L" the action invoked"
               );
         }
      }

      // We have to process the out arguments now
      hr = HrGetSafeArrayBounds(
            vaOutArgs.parray, 
            &lLBound, 
            &lUBound
            );
      if(SUCCEEDED(hr))
      {
         // We have got the bounds of the arguments
         for (lIndex = lLBound; 
               lIndex<=lUBound; 
               ++lIndex
               )
         {
            VariantClear(&vaOutElement);
            hr = HrGetVariantElement(
                    vaOutArgs.parray, 
                    lIndex, 
                    &vaOutElement
                    );
            if(SUCCEEDED(hr))
            {
               hr = VariantChangeType(
                     &vaOutElement, 
                     &vaOutElement, 
                     VARIANT_ALPHABOOL, 
                     VT_BSTR
                     );
               if (SUCCEEDED(hr))
               {
                  _snwprintf_s(
                       wszTempString, 
                       DATA_BUFSIZE, 
                       _TRUNCATE,
                       L"%ls ", 
                       vaOutElement.bstrVal
                       );
                  wcsncat_s(
                        wszString, 
                        DATA_BUFSIZE,
                        wszTempString,
                        (DATA_BUFSIZE - 
                         wcslen(wszString) - 1)
                     );
               }
               else
               {
                  PrintErrorText(hr);
                  m_StatusText.SetWindowText(
                    L"Error changing the out"
                       L" argument type for the"
                       L" action invoked"
                       );
               }
            }
            else
            {
               m_StatusText.SetWindowText(L"Error getting the out argument for the action invoked");
               PrintErrorText(hr);
            }
         } // For loop

         VariantClear(&vaOutElement);
      }

      m_ActionOutArgument.SetWindowText(wszString); // Print the return value and out arguments.
   }
   else
   {
      PrintErrorText(hr);
   }
   ReleaseObj(pService);

   VariantClear(&vaOutArgs);
   VariantClear(&vaRet);
   VariantClear(&vaOutElement);
}


//+---------------------------------------------------------------------------
//
//  Member:		OnCloseClick
//
//  Purpose:    Close the dialog
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//

void CGenericUCPDlg::OnCloseClick() 
{
   OnClose();
   EndDialog(1);
   return;
}


//+---------------------------------------------------------------------------
//
//  Member:		OnCheckBoxClick
//
//  Purpose:    A Checkbox has been clicked so if an async find is running, it will be stopped
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//

void CGenericUCPDlg::OnCheckBoxClick()
{
   StopAsyncFindIfStarted();
}


//+---------------------------------------------------------------------------
//
//  Member:		iNumberOfInArguments
//
//  Purpose:    Calculate the number of arguments in action box
//
//  Arguments:
//				tszString	[in]	Arguments string
//
//  Returns:    None
//
//  Notes:		
//

int CGenericUCPDlg::iNumberOfInArguments(_In_ LPWSTR tszString)
{
   int iCount = 0;
   WCHAR *token = NULL;
   WCHAR *context = NULL;
   token = wcstok_s(tszString, L" ", &context);
   while(token!=NULL){
      iCount++;
      token = wcstok_s(NULL, L" ", &context);
   }
   return iCount;
}

///////////////////////////////////////////////////////////////////
//   CDeviceFinderCallback
///////////////////////////////////////////////////////////////////

CDeviceFinderCallback::CDeviceFinderCallback()
{
    m_cRef = 0;
}

CDeviceFinderCallback::~CDeviceFinderCallback()
{
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown Functions

HRESULT __stdcall CDeviceFinderCallback::QueryInterface(_In_ const IID& iid, _Out_ void** ppv)
{
    if (iid == IID_IUPnPDeviceFinderCallback)
    {
        *ppv = static_cast<IUPnPDeviceFinderCallback*>(this);
    }
    else if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    static_cast<CDeviceFinderCallback*>(*ppv)->AddRef();

    return S_OK;

}

ULONG __stdcall CDeviceFinderCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CDeviceFinderCallback::Release()
{
    int iCount = InterlockedDecrement(&m_cRef);
    if (iCount == 0)
    {
        delete this;
    }

    return iCount;
}

//+---------------------------------------------------------------------------
//
//  Member:		Create
//
//  Purpose:    Create the Device Finder callback object
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		This is a static function
//

CDeviceFinderCallback* CDeviceFinderCallback::Create()
{
   CDeviceFinderCallback * pCallback = new CDeviceFinderCallback();
   pCallback->AddRef();
   return pCallback;
}

//+---------------------------------------------------------------------------
//
//  Member:		SetDialogPointer
//
//  Purpose:    Set the CGenericUCPDlg Dialog Pointer to call back
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//

void CDeviceFinderCallback::SetDialogPointer(CGenericUCPDlg* pDialog){
   m_pGenericUCPDlg=pDialog;
}

//+---------------------------------------------------------------------------
//
//  Member:		DeviceAdded
//
//  Purpose:    Called when a device is added
//
//  Arguments:
//				lFindData	       [in]	AsyncFindHandle
//				pDevice		[in]	COM interface pointer of the device being added
//
//  Returns:    S_OK
//
//  Notes:		
//

HRESULT CDeviceFinderCallback::DeviceAdded(
   _In_ LONG            lFindData, 
   _In_ IUPnPDevice *   pDevice)
{
   UNREFERENCED_PARAMETER(lFindData);

   HRESULT HR = S_OK;
   TRACE(L"Device Added\n");
   m_pGenericUCPDlg->AddDevice(pDevice);
   return HR;
}


//+---------------------------------------------------------------------------
//
//  Member:		DeviceRemoved
//
//  Purpose:    Called when a device is removed
//
//  Arguments:
//				lFindData	       [in]	AsyncFindHandle
//				bstrUDN		[in]	UDN of the device being removed
//
//  Returns:    S_OK
//
//  Notes:		
//

HRESULT CDeviceFinderCallback::DeviceRemoved(    
   _In_ LONG    lFindData, 
   _In_ BSTR    bstrUDN)
{
   UNREFERENCED_PARAMETER(lFindData);

   TRACE(L"Device Removed\n");
   m_pGenericUCPDlg->RemoveDevice(bstrUDN);
   return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:		SearchComplete
//
//  Purpose:    Called when the search is complete
//
//  Arguments:
//				lFindData	       [in]	AsyncFindHandle
//
//  Returns:    S_OK
//
//  Notes:		
//
HRESULT CDeviceFinderCallback::SearchComplete(
   _In_ LONG    lFindData)
{
    UNREFERENCED_PARAMETER(lFindData);

    TRACE(L"Search Complete\n");
   m_pGenericUCPDlg->m_StatusText.SetWindowText(L"Device Search Completed");

   if(m_pGenericUCPDlg->m_DeviceCombo.GetCount()==0){
      m_pGenericUCPDlg->MessageBox(L"Found no devices");
   }
   return S_OK;
}



///////////////////////////////////////////////////////////////////
//   CServiceCallback
///////////////////////////////////////////////////////////////////

CServiceCallback::CServiceCallback()
{
    m_cRef = 0;
}

CServiceCallback::~CServiceCallback()
{
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown Functions

HRESULT __stdcall CServiceCallback::QueryInterface(_In_ const IID& iid, _Out_ void** ppv)
{
    if (iid == IID_IUPnPServiceCallback)
    {
        *ppv = static_cast<IUPnPServiceCallback*>(this);
    }
    else if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    static_cast<CServiceCallback*>(*ppv)->AddRef();

    return S_OK;

}

ULONG __stdcall CServiceCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CServiceCallback::Release()
{
    int iCount = InterlockedDecrement(&m_cRef);
    if (iCount == 0)
    {
        delete this;
    }

    return iCount;
}


//+---------------------------------------------------------------------------
//
//  Member:		Create
//
//  Purpose:    Create the Device Finder callback object
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		This is a static function
//

CServiceCallback* CServiceCallback::Create()
{
   CServiceCallback * pCallback = new CServiceCallback();
   pCallback->AddRef();
   return pCallback;
}


//+---------------------------------------------------------------------------
//
//  Member:		SetDialogPointer
//
//  Purpose:    Set the CGenericUCPDlg Dialog Pointer to call back
//
//  Arguments:
//				None
//
//  Returns:    None
//
//  Notes:		
//

void CServiceCallback::SetDialogPointer(CGenericUCPDlg* pDialog){
   m_pGenericUCPDlg=pDialog;
}

//+---------------------------------------------------------------------------
//
//  Member:		StateVariableChanged
//
//  Purpose:    Called when the state variable is changed
//
//  Arguments:
//				pus					[in]	COM interface pointer of the service 
//				pcwszStateVarName	[in]	State Variable Name
//				varValue			       [in]	State Variable Value
//
//  Returns:    HRESULT
//
//  Notes:		
//

HRESULT CServiceCallback::StateVariableChanged(
   _In_ IUPnPService *pus,
   _In_ LPCWSTR pcwszStateVarName, 
   _In_ VARIANT varValue)
{
   HRESULT hr = S_OK;
   WCHAR wszMessage[DATA_BUFSIZE];
   BSTR bstrServiceId = NULL;	
   VARIANT varTemp;
   TRACE(L"State Variable Changed\n");

   VariantInit(&varTemp);

   hr = pus->get_Id(&bstrServiceId);
   if(SUCCEEDED(hr)){
      hr = VariantCopy(&varTemp, &varValue);
      if (SUCCEEDED(hr)){
         hr=VariantChangeType(&varTemp, &varTemp, VARIANT_ALPHABOOL, VT_BSTR);
         if(SUCCEEDED(hr)){
            _snwprintf_s(wszMessage, DATA_BUFSIZE, _TRUNCATE, L"State variable %ls changed to %ls in %ls", pcwszStateVarName, varTemp.bstrVal, bstrServiceId);
            Sleep(350);
            m_pGenericUCPDlg->m_EventText.SetWindowText(wszMessage);
         }
         else{
            PrintErrorText(hr);
         }
      }
      SysFreeString(bstrServiceId);
   }
   else{
      PrintErrorText(hr);
      m_pGenericUCPDlg->m_StatusText.SetWindowText(L"Error: ServiceId failed");
   }

   VariantClear(&varTemp);
   return hr;
}


//+---------------------------------------------------------------------------
//
//  Member:		ServiceInstanceDied
//
//  Purpose:    Called when the service dies
//
//  Arguments:
//				pus					[in]	COM interface pointer of the service 
//
//  Returns:    HRESULT
//
//  Notes:		
//

HRESULT CServiceCallback::ServiceInstanceDied(
   _In_ IUPnPService *pus)
{
   HRESULT hr=S_OK;
   WCHAR wszMessage[DATA_BUFSIZE];
   TRACE(L"Service instance died\n");

   BSTR bstrServiceId = NULL;
   hr = pus->get_Id(&bstrServiceId);
   if(SUCCEEDED(hr)){
      _snwprintf_s(wszMessage, DATA_BUFSIZE, _TRUNCATE, L"Service %ls died",bstrServiceId);
      m_pGenericUCPDlg->m_EventText.SetWindowText(wszMessage);
      SysFreeString(bstrServiceId);
   }
   else{
      PrintErrorText(hr);
      m_pGenericUCPDlg->m_StatusText.SetWindowText(L"Error: ServiceId failed");
   }
   return hr;
}


//+---------------------------------------------------------------------------
//
//  Member:		OnClose
//
//  Purpose:    Called when the dialog box closes
//
//  Arguments:
//				None
//
//  Returns:    S_OK
//
//  Notes:		
//


void CGenericUCPDlg::OnClose() 
{
   SetEvent(m_hCloseEvent);
   ClearAllDataStructures(TRUE);

   CDialog::OnClose();
}
