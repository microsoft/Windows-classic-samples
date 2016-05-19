// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//------------------------------------------------------------------------------
// File: DVApp.cpp
//
// Desc: DirectShow sample code - DV control/capture example.
//------------------------------------------------------------------------------

#include "CDVGraph.h"

#include <commdlg.h>
#include <commctrl.h>
#include <dbt.h>
#include "resource.h"



// ------------ constants-----------------
#define DV_CAPLIMIT_NONE         10L
#define DV_CAPLIMIT_TIME         11L
#define DV_CAPLIMIT_SIZE         12L

#define DV_BYTESPERMSEC          352  //DV captures at 3600K per second (3600 / 1024 == ~3.52)
#define DV_TIMERFREQ             55   //milliseconds between timer ticks
#define DV_BYTES_IN_MEG          1048576L  //(1024 * 1024)

// ----------macros-----------------------
#define MBOX(s)                 MessageBox(g_hwndApp, s, APPNAME, MB_OK);

// ----------function prototypes----------
BOOL            FInitMain(int nCmdShow);
BOOL            DV_InitControls(HWND hwnd, HINSTANCE hInst);
int             NMsgLoop(void);

HRESULT         DV_AppSetup(void);
HRESULT         SetPreviewWindow(void);

// UI help functions
void Mark_ToolBar_Button(BOOL bEnableRecord, BOOL bEnableOthers);
void Mark_GraphMode_Menu(HWND hwnd, int idmVal);
void GetSelectedGraphMode( WPARAM wParam, GRAPH_TYPE* pGraphType);

// Message Processing Functions
LRESULT CALLBACK    MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void CALLBACK       DV_TransportCommand_WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK       DV_GraphModeCommand_WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK       DV_CapSizeDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CALLBACK       DV_DroppedFrameProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
void CALLBACK       DV_TimecodeTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
void CALLBACK       DV_StopRecProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
BOOL CALLBACK       DV_AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void                DV_DisplayTimecode();
BOOL                DV_SeekATN(void);
BOOL                DV_UpdateTapeInfo(void);
BOOL                DV_RefreshMode(void);

// ------------global data----------------

// Handles to the windows
HINSTANCE       g_hinst;
HWND            g_hwndApp      = NULL;
HWND            g_hwndTBar     = NULL; 
HWND            g_hwndStatus   = NULL;
HMENU           g_hmenu        = 0;
HWND            g_hwndTCCheck  = NULL;//checkbox

// metrics of the windows
int             g_iAppHeight   = DEFAULT_VIDEO_HEIGHT + HEIGHT_EDGE;
int             g_iAppWidth    = DEFAULT_VIDEO_WIDTH  + WIDTH_EDGE;        
int             g_iVWHeight    = DEFAULT_VIDEO_HEIGHT;
int             g_iVWWidth     = DEFAULT_VIDEO_WIDTH;        
DWORD           g_statusHeight = 0;

// device notification globals
PVOID           g_hDevNotify   = NULL;
BOOL            g_bDeviceFound = FALSE;

// capture variables
DWORD          g_dwCaptureLimit = DV_CAPLIMIT_NONE; //track whether we are using time, disk, or no based capture limits
DWORD          g_dwDiskSpace    = 120;              //roughly the same
DWORD          g_dwTimeLimit    = 30;               //default to 30 seconds of capture

// graph related variables
CDVGraph*       g_pGraph        = NULL; 
GRAPH_TYPE      g_iGraphType    = GRAPH_PREVIEW;    //need to track the current graph

// file names
TCHAR           g_InputFileName[_MAX_PATH]          = {DEFAULT_CAP_FILE_NAME};
TCHAR           g_OutputFileName[_MAX_PATH]         = {DEFAULT_CAP_FILE_NAME};
TCHAR           g_FilterGraphFileName[_MAX_PATH]    = {DEFAULT_FG_FILE_NAME};

DWORD           g_CapStartTime   = 0;
BOOL            g_bUseAtnTimer   = FALSE;  //track if we want to constantly update the timer
BOOL            g_bHalfFrameRate = FALSE;



// ------------inline functions-------------------
// put the VCR Mode
inline HRESULT DV_PutVcrMode(long Mode)
{
    if(!g_pGraph->m_pIAMExtTransport)
        return S_FALSE;

    return g_pGraph->m_pIAMExtTransport->put_Mode(Mode);
} 

// update the status windows with appropriate text
inline LRESULT DV_StatusText(LPCTSTR statusText, UINT nPart)
{
    return SendMessage(g_hwndStatus, SB_SETTEXT, (WPARAM) 0 | nPart, (LPARAM)statusText);
} 

//divide the status bar into thirds, and give the middle frame an extra 100 pixels.
inline LRESULT DV_StatusParts(UINT width)
{
    int rg[3];
    rg[0] = (width / 3) - 50;
    rg[1] = ((rg[0]+50) * 2) + 50;
    rg[2] = -1;

    return SendMessage(g_hwndStatus, SB_SETPARTS, 3, (LPARAM)(LPINT) rg);
}    


/*-------------------------------------------------------------------------
Routine:        WinMain
Purpose:        Program entry point      
Arguments:      Usual
Returns:        Usual
Notes:          Sets up window capabilities, initializes & 
                    creates the required DirectShow interface & filters.
------------------------------------------------------------------------*/
int WINAPI WinMain( HINSTANCE hinst,        // instance handle
                    HINSTANCE hinstPrev,    // always NULL
                    LPSTR pszCmd,           // pointer to ANSI command line arguments
                    int nCmdShow)           // initial app window state
{
    ASSERT(!hinstPrev);
    g_hinst = hinst;
    if (!FInitMain(nCmdShow))
    {
        return -1;
    }

    // Register for device add/remove notifications.
    DEV_BROADCAST_DEVICEINTERFACE filterData;
    ZeroMemory(&filterData, sizeof(DEV_BROADCAST_DEVICEINTERFACE));   
    filterData.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    filterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filterData.dbcc_classguid = AM_KSCATEGORY_CAPTURE;
    g_hDevNotify = RegisterDeviceNotification(g_hwndApp, &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);        
    ASSERT(g_hDevNotify != NULL);

    // dvapp setup 
    DV_AppSetup();

    return NMsgLoop();
}

/*--------------------------------------------------------------------------
Routine:  FInitMain
Purpose:  Initialize the main application window.
Arguments:int initial app window state
Returns:  Return TRUE if successful.
------------------------------------------------------------------------*/
BOOL FInitMain( int nCmdShow)
{
    WNDCLASSEX      wc;

    INITCOMMONCONTROLSEX icc;
    // register common control classes
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    // register the window class
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0 ;
    wc.cbWndExtra       = 0 ;
    wc.hInstance        = g_hinst;
    wc.hIcon            = LoadIcon (g_hinst, TEXT("DVICON")) ;
    wc.hCursor          = LoadCursor (NULL, IDC_ARROW) ;
    wc.hbrBackground    = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
    wc.lpszMenuName     = NULL ;
    wc.lpszClassName    = APPNAME ;
    wc.hIconSm          = LoadIcon(g_hinst, TEXT("DVICON"));   
    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, TEXT("cannot register the window class"), TEXT("Error"), 0);
        return FALSE;
    }

    g_hmenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDR_MENU));

    // create the main application window
    g_hwndApp = CreateWindowEx(
        0,
        APPNAME,    // window class name
        DV_APPTITLE,    // window caption
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, // window style
        CW_USEDEFAULT,  // initial x position
        CW_USEDEFAULT,  // initial y position
        g_iAppWidth,    // initial x size
        g_iAppHeight,   // initial y size
        NULL,           // parent window handle
        g_hmenu,        // window menu handle
        g_hinst,        // program instance handle
        0) ;            // creation parameters
    
    if (!g_hwndApp)
    {
        MBOX(TEXT("Create main window failed"));
        return FALSE;
    }

    // show the window
    ShowWindow(g_hwndApp, nCmdShow);
    UpdateWindow(g_hwndApp);
    SetWindowText(g_hwndApp, TEXT("Initializing..."));

    return TRUE;
}

/*--------------------------------------------------------------------------
Routine:  NMsgLoop
Purpose:  Execute the program message loop.
Arguments:None
Returns:  Return the wParam field of the final WM_QUIT message.
------------------------------------------------------------------------*/ 
int NMsgLoop(void)
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {   
        TranslateMessage(&msg);
        DispatchMessage(&msg);              
    }

    return ((int) msg.wParam);
}

/*--------------------------------------------------------------------------
Routine:  MainWndProc
Purpose:  Window procedure for the main application window.
Arguments:Usual
Returns:  Usual
------------------------------------------------------------------------*/ 
LRESULT CALLBACK MainWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam)
{
    HRESULT hr;
    switch (msg)
    {
        case WM_CREATE:
            if (!DV_InitControls(hwnd, (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE)))
            {
                MBOX(TEXT("DV_InitControls failed to create one of the control windows"));
            }
            break;

        case WM_SIZE:
        {
            RECT  rcAppWin, rcClient, rcTB;
            //re-size the App window
            int cxBorder = GetSystemMetrics(SM_CXBORDER);
            int cyBorder = GetSystemMetrics(SM_CYBORDER);
            GetWindowRect(g_hwndApp, &rcAppWin);
            GetWindowRect(g_hwndTBar, &rcTB);
            MoveWindow(g_hwndApp, rcAppWin.left, rcAppWin.top, g_iAppWidth, g_iAppHeight, TRUE);

            // Tell the toolbar to resize itself to fill the top of the window.
            SendMessage(g_hwndTBar, TB_AUTOSIZE, 0L, 0L);

            //handle the status bar height
            GetClientRect(g_hwndApp, &rcClient);
            cxBorder = GetSystemMetrics(SM_CXBORDER);
            cyBorder = GetSystemMetrics(SM_CYBORDER);
            MoveWindow(g_hwndStatus, -cxBorder, rcClient.bottom - (g_statusHeight + cyBorder), 
                       rcClient.right + (2 * cxBorder), (g_statusHeight + (2 * cyBorder)), TRUE);  

            DV_StatusParts(rcClient.right);
        
            //re-size the video window
            GetWindowRect(g_hwndTBar, &rcTB);
            if (g_pGraph )
            {
                if(g_pGraph->m_pVideoWindow)
                {
                    g_pGraph->m_pVideoWindow->SetWindowPosition(0, rcTB.bottom - rcTB.top, g_iVWWidth, g_iVWHeight);
                }
            }
            break;          
        }           

        case WM_CLOSE:
            return SendMessage(hwnd, WM_DESTROY, 0,0);

        case WM_DESTROY:
            // Unregister device notifications
            if (g_hDevNotify != NULL)
            {
                UnregisterDeviceNotification(g_hDevNotify);
                g_hDevNotify = NULL;
            }      
            
            if(g_pGraph != NULL){         
                delete g_pGraph;
                g_pGraph = NULL;
            }

            PostQuitMessage(0);
            break;

        case WM_DEVICECHANGE:
        {
            // We are interested in only device arrival events
            if (DBT_DEVICEARRIVAL != wparam)
                break;
            PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR) lparam;
            if (pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
                break;

            // Check for capture devices.
            PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE) lparam;
            if (pdbi->dbcc_classguid != AM_KSCATEGORY_CAPTURE)
                break;

            // Check for device arrival.
            if (g_bDeviceFound == FALSE)
            {
                MessageBox(g_hwndApp, TEXT("There is a new capture device on this system..."), APPNAME, MB_OK);
                DV_AppSetup();
            }
            break;
        }

        case WM_FGNOTIFY :            //Filter Graph Events Notify Message handler
        {
            if(g_pGraph == NULL)
                break;
           if(g_pGraph->m_pMediaEvent == NULL)
               break;

            long event;
            LONG_PTR l1, l2;
            // Get the corresponding filtergraph directshow media event to handle
            while (SUCCEEDED(g_pGraph->m_pMediaEvent->GetEvent(&event, &l1, &l2, 0L)))
            {
                switch (event)
                {
                    case EC_USERABORT :
                    case EC_COMPLETE :
                        // filtergraph is done, time to stop the filtergraph or transport state
                        SendMessage(g_hwndApp, WM_COMMAND, IDM_STOP, 0);  
                        break;
            
                    case EC_ERRORABORT :
                        //something bad happened during capture
                        MBOX(TEXT("Error during preview, capture or transmit"));
                        break;

                    case EC_DEVICE_LOST :   // Check if we have lost a capture filter being used.
                        if (l2 == 0)        //0 indicates device removed;1 indicates removed device added again
                        {
                            IBaseFilter *pf;
                            IUnknown *punk = (IUnknown *) l1;
                            hr = punk->QueryInterface(IID_IBaseFilter, (void **) &pf);
                            if(FAILED(hr))
                                break;
                    
                            if (AreComObjectsEqual(g_pGraph->m_pDeviceFilter, pf))
                            {
                                MBOX(TEXT("DV Camcorder Device in use was removed"));

                                // handle the timers accordingly
                                if (g_bUseAtnTimer)
                                    KillTimer(hwnd, DV_TIMER_ATN);

                                KillTimer(hwnd, DV_TIMER_CAPLIMIT);
                                KillTimer(hwnd, DV_TIMER_FRAMES);
                                g_bDeviceFound = FALSE;

                                // should call delete filters....????
                                if(g_pGraph != NULL)
                                    delete g_pGraph; //  pf is released
                                DV_AppSetup();
                            }
                            else
                                pf->Release();               
                        }
                        break;
                    default:
                        break;
                }// end of switch

                if(g_pGraph->m_pMediaEvent == NULL)
                    break;

                // Free any memory associated with this event
                g_pGraph->m_pMediaEvent->FreeEventParams(event, l1, l2);    

            } // end of while
            break;
        }

        case WM_COMMAND:
        switch (LOWORD(wparam))
        {
            // The File Menu 
            /*  
                The globals for input file and output file are TCHAR.  When compiled ANSI,
                GetOpenFileName (obviously) return ANSI buffers.  DirectShow functions use Unicode
                names exclusively, so these are converted to Unicode within the functions 
                that need to use these variables (DV_Make...To... functions).
            */
            case IDM_SETINPUT :    //fall through
            case IDM_SETOUTPUT :
            case IDM_OPTIONS_SAVEGRAPH :
            {
                OPENFILENAME ofn = {0};
                OSVERSIONINFO osi = {0};
                //need to adjust the ofn struct size if we are running on win98 vs. nt5
                osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                GetVersionEx(&osi);
                if (osi.dwMajorVersion >=5 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT)
                    ofn.lStructSize = sizeof (OPENFILENAME);
                else
                    ofn.lStructSize = sizeof(OPENFILENAME);
            
                ofn.hwndOwner = g_hwndApp;
                ofn.nMaxFile = _MAX_PATH;

                if (IDM_OPTIONS_SAVEGRAPH == LOWORD (wparam))
                {
                    ofn.lpstrFilter = TEXT("Filter Graph (*.grf)\0*.grf\0\0");
                    ofn.lpstrTitle  = TEXT("Set FilterGraph File Name...\0");
                    ofn.lpstrFile   = g_FilterGraphFileName;
                    ofn.Flags       = OFN_HIDEREADONLY;
                } 
                else if (IDM_SETOUTPUT == LOWORD (wparam))
                {
                    ofn.lpstrFilter = TEXT("Microsoft AVI (*.avi)\0*.avi\0\0");
                    ofn.lpstrTitle  = TEXT("Set Output File Name...\0");
                    ofn.lpstrFile   = g_OutputFileName;
                    ofn.Flags       = OFN_HIDEREADONLY;
                } 
                else
                {
                    ofn.lpstrFilter = TEXT("Microsoft AVI (*.avi)\0*.avi\0\0");
                    ofn.lpstrTitle  = TEXT("Set Input File Name...\0");
                    ofn.lpstrFile   = g_InputFileName;
                    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_READONLY;
                } 

                if (GetOpenFileName(&ofn))
                {
                    if (IDM_OPTIONS_SAVEGRAPH == LOWORD (wparam))
                    {
                        hr = StringCchCopy(g_FilterGraphFileName, NUMELMS(g_FilterGraphFileName), ofn.lpstrFile);
                        // Save the current built filter graph to a *.grf file
                        if(g_pGraph != NULL)
                            g_pGraph->SaveGraphToFile(g_FilterGraphFileName);
                        
                    } 
                    else if (IDM_SETOUTPUT == LOWORD (wparam))
                    {
                        hr = StringCchCopy(g_FilterGraphFileName, NUMELMS(g_FilterGraphFileName), ofn.lpstrFile);
                    } 
                    else
                    {
                        hr = StringCchCopy(g_FilterGraphFileName, NUMELMS(g_FilterGraphFileName), ofn.lpstrFile);
                    } 
                }
                break;    
            } 
            case IDM_ABOUT:
                DialogBox((HINSTANCE)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)DV_AboutDlgProc);
                break;

            // Toolbar Button Commands
            // Transport State & Graph State Control
            case IDM_PLAY :
            case IDM_RECORD :
            case IDM_STOP : 
            case IDM_PAUSE :
            case IDM_FF :
            case IDM_REW :
            case IDM_PLAY_FAST_FF :
            case IDM_PLAY_FAST_REV :        
            case IDM_STEP_FWD :
            case IDM_STEP_REV :
            case IDM_SEEKTIMECODE :
                DV_TransportCommand_WndProc(hwnd, msg, wparam, lparam);
                break;

            // The Options Menu
            case IDM_DECODESIZE :
                g_pGraph->StopGraph();
                g_pGraph->GetVideoWindowDimensions(&g_iVWWidth, &g_iVWHeight, TRUE, g_hwndApp);
                SetPreviewWindow();

                break;

            case IDM_CHECKTAPE :
                DV_UpdateTapeInfo();
                break;

            case IDM_REFRESHMODE :
                DV_RefreshMode();
                break;

            case IDM_FRAMERATE :
                if( g_bHalfFrameRate )
                {
                    g_bHalfFrameRate = FALSE;
                    CheckMenuItem(g_hmenu, IDM_FRAMERATE, MF_UNCHECKED);
                }
                else
                {
                    g_bHalfFrameRate = TRUE;
                    CheckMenuItem(g_hmenu, IDM_FRAMERATE, MF_CHECKED);                   
                }
                g_pGraph->ChangeFrameRate(g_bHalfFrameRate);
                break;

            case IDM_CAPSIZE:
              DialogBox((HINSTANCE)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG_CAPSIZE), hwnd, (DLGPROC)DV_CapSizeDlgProc);
              break;

            case IDM_EXIT:
                return SendMessage(hwnd, WM_CLOSE, 0, 0);
                
            // Graph Mode Menu
            // Change the Current Graph Mode
            case IDM_PREVIEW :
            case IDM_FILETODV :
            case IDM_FILETODV_NOPRE :
            case IDM_DVTOFILE :
            case IDM_DVTOFILE_NOPRE :
            case IDM_FILETODV_TYPE2 :
            case IDM_FILETODV_NOPRE_TYPE2 :
            case IDM_DVTOFILE_TYPE2 :
            case IDM_DVTOFILE_NOPRE_TYPE2 :            
                DV_GraphModeCommand_WndProc (hwnd, msg, wparam, lparam);
                
                break;
        }
        break;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return 0;
}

/*-------------------------------------------------------------------------
Routine:        DV_InitControls
Purpose:        Initializer for app window controls (toolbars, edit controls, etc.)
Arguments:    window handle and hinstance
Returns:        FALSE if creation of any of the controls fails.
Notes:          
------------------------------------------------------------------------*/
BOOL DV_InitControls(HWND hwnd, HINSTANCE hInst)
{
    HWND hwndEdit1 = NULL;
    HWND hwndEdit2 = NULL;
    HWND hwndEdit3 = NULL;
    HWND hwndEdit4 = NULL;

    RECT rect = {0};

    // create status bar windows
    g_hwndStatus = CreateWindowEx( 0,
                                   STATUSCLASSNAME,
                                   TEXT(""),
                                   WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS,
                                   -100, -100,
                                   10, 10,
                                   hwnd,
                                   HMENU(IDB_STATUS),
                                   g_hinst,
                                   NULL);

    DV_StatusParts(g_iAppWidth);
    g_statusHeight = rect.bottom;

    // create toolbar window
    g_hwndTBar = CreateToolbarEx(  hwnd, 
                                   WS_CHILD | WS_VISIBLE | WS_BORDER | TBSTYLE_TOOLTIPS, 
                                   ID_TOOLBAR, 
                                   10, 
                                   g_hinst, 
                                   IDB_TOOLBAR, 
                                   g_rgTbButtons, 
                                   sizeof(g_rgTbButtons) / sizeof(TBBUTTON), 
                                   16,16,16,16, 
                                   sizeof(TBBUTTON)); 

    // create timecode text boxes on the toolbar        
    rect.right = 350;
    hwndEdit1 = CreateWindow(TEXT("edit"), TEXT("00"), WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 
                    rect.right + 6, 4, 22, 18, g_hwndTBar, (HMENU)IDC_EDIT_HOUR, (HINSTANCE) hInst, NULL);
    
    hwndEdit2 = CreateWindow(TEXT("edit"), TEXT("00"), WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP | ES_NUMBER,  
                    rect.right + 30, 4, 22, 18, g_hwndTBar, (HMENU)IDC_EDIT_MINUTE, (HINSTANCE) hInst, NULL);
    
    hwndEdit3 = CreateWindow(TEXT("edit"), TEXT("00"), WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 
                    rect.right + 54, 4, 22, 18, g_hwndTBar, (HMENU)IDC_EDIT_SECOND, (HINSTANCE) hInst, NULL);

    hwndEdit4 = CreateWindow(TEXT("edit"), TEXT("00"), WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 
                    rect.right + 78, 4, 22, 18, g_hwndTBar, (HMENU)IDC_EDIT_FRAME, (HINSTANCE) hInst, NULL);

    g_hwndTCCheck = CreateWindow(TEXT("button"), TEXT("Display Timecodes"), WS_CHILD | BS_AUTOCHECKBOX | WS_VISIBLE | WS_TABSTOP, 
                    rect.right + 106, 5, 190, 18, g_hwndTBar, (HMENU)IDC_TCCHECKBOX, (HINSTANCE) hInst, NULL); 
    Button_SetCheck (g_hwndTCCheck, BST_CHECKED) ;                    

    return (!( !hwndEdit1) || (!hwndEdit2) || (!hwndEdit3) || (!hwndEdit4) || 
             (!g_hwndTCCheck) || (!g_hwndStatus));
} 

/*-------------------------------------------------------------------------
Routine:        DV_AppSetup
Purpose:        look for a DV device, initialize it, get its subunit mode, 
                    Create the filtergraph and instantiate the filters
Arguments:    None
Returns:        None
Notes:          
------------------------------------------------------------------------*/
HRESULT DV_AppSetup(void)
{   
    HRESULT hr =S_OK;
    g_pGraph = new CDVGraph();
    if ( !g_pGraph ){
       hr = E_OUTOFMEMORY;
       return hr;
    }
    
    //look for a DV device and initialize it, or show an error if one does not exist 
    hr = g_pGraph->BuildBasicGraph();
    if (S_OK != hr) 
    {
        g_bDeviceFound = FALSE;
        int iOption = MessageBox(g_hwndApp, TEXT("There are no DV camcorder devices on this system.\n\n")
                                 TEXT("Do you want to exit the app?"), APPNAME, MB_YESNO);
        if(iOption == IDYES) 
            SendMessage(g_hwndApp, WM_DESTROY, 0,0);
        else
        {
            ShowWindow(g_hwndTBar, SW_HIDE);

            EnableMenuItem(g_hmenu, IDM_REFRESHMODE, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_CHECKTAPE, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_DECODESIZE, MF_GRAYED);

            EnableMenuItem(g_hmenu, IDM_PREVIEW, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_DVTOFILE, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_DVTOFILE_NOPRE, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_FILETODV, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_FILETODV_NOPRE, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_DVTOFILE_TYPE2, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_DVTOFILE_NOPRE_TYPE2, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_FILETODV_TYPE2, MF_GRAYED);
            EnableMenuItem(g_hmenu, IDM_FILETODV_NOPRE_TYPE2, MF_GRAYED);
        }
    }
    else
    {
        g_bDeviceFound = TRUE;
        ShowWindow(g_hwndTBar, SW_SHOWNORMAL);

        EnableMenuItem(g_hmenu, IDM_REFRESHMODE, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_CHECKTAPE, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_DECODESIZE, MF_ENABLED);

        EnableMenuItem(g_hmenu, IDM_PREVIEW, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_DVTOFILE, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_DVTOFILE_NOPRE, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_FILETODV, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_FILETODV_NOPRE, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_DVTOFILE_TYPE2, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_DVTOFILE_NOPRE_TYPE2, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_FILETODV_TYPE2, MF_ENABLED);
        EnableMenuItem(g_hmenu, IDM_FILETODV_NOPRE_TYPE2, MF_ENABLED);

        //determine if we are in camera mode or vcr mode and disable unavailable menu items
        DV_RefreshMode();
        DV_StatusText(g_pGraph->m_DeviceName, 0);

        // make preview graph
        hr= g_pGraph->MakePreviewGraph( );
        if(FAILED(hr))
        {
            delete g_pGraph;
            g_pGraph = NULL;
    
            MBOX(TEXT("MakePreviewGraph() failed"));
            return hr;
        }  

        //get and set the display window size
        g_pGraph->GetVideoWindowDimensions(&g_iVWWidth, &g_iVWHeight, FALSE, NULL);
 
        // register a window to process event notifications.     
        hr = g_pGraph->m_pMediaEvent->SetNotifyWindow((LONG_PTR) g_hwndApp, WM_FGNOTIFY, 0);
        if(FAILED(hr))
        {
            MBOX(TEXT("g_pGraph->m_pMediaEvent->SetNotifyWindow() failed"));
            return hr;
        }
     
        // update globals, log, toolbar, menu items for preview mode
        g_iGraphType = GRAPH_PREVIEW;
        Mark_GraphMode_Menu( g_hwndApp, IDM_PREVIEW );   
        Mark_ToolBar_Button( FALSE, TRUE );    
        SetPreviewWindow();
        
        // ready for user
        SetWindowText(g_hwndApp, DV_APPTITLE);     
    }

    return hr;
}

/*-------------------------------------------------------------------------
Routine:        DV_GraphModeCommand_WndProc
Purpose:        Message Handler for Graph Mode menu items
Arguments:    Usual message processing parameters
Returns:        Usual
Notes:          Builds the various kinds of Graph types preview/capture/transmit
------------------------------------------------------------------------*/
void CALLBACK DV_GraphModeCommand_WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{ 
    GRAPH_TYPE graphType= (GRAPH_TYPE)0;
    GetSelectedGraphMode( wParam, &graphType );

    // dont rebuild if the required graph already exists
    if( graphType == g_iGraphType )
        return;

    //stop the graph - this should never fail before rebuilding
    if(FAILED( g_pGraph->m_pMediaControl->Stop()))
        return;

    //let's look at the former graph
    //Disconnect everything
    if (GRAPH_FILE_TO_DV == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE == g_iGraphType ||
        GRAPH_FILE_TO_DV_TYPE2 == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE_TYPE2 == g_iGraphType)
    {
        //DisconnectAll removes only the downstream filters - we need to remove the file filter by hand.
        g_pGraph->RemoveFilters( g_pGraph->m_pInputFileFilter, TRUE);
        if (g_pGraph->m_pInputFileFilter )
        {
            g_pGraph->m_pGraph->RemoveFilter( g_pGraph->m_pInputFileFilter );
            SAFE_RELEASE( g_pGraph->m_pInputFileFilter );        
        }
    } 
    else
    {
        //DisconnectAll removes only the downstream filters         
        g_pGraph->RemoveFilters( g_pGraph->m_pDeviceFilter, TRUE);
    }
    g_iGraphType = graphType;

    switch (LOWORD (wParam))
    {   
        case IDM_PREVIEW :
            if(SUCCEEDED(g_pGraph->MakePreviewGraph( )))
            {
                // update globals, log, toolbar, menu items
                SetPreviewWindow();
                Mark_GraphMode_Menu( hwnd, IDM_PREVIEW );   
                Mark_ToolBar_Button( TRUE, TRUE );    
            }
            else{
                delete g_pGraph;
                g_pGraph = NULL;
                MBOX(TEXT("MakePreviewGraph() failed"));
            }

            break;

        // Type 1 file (transmit & playback)    
        case IDM_FILETODV :
            if(SUCCEEDED(g_pGraph->MakeFileToDvGraph_Type1( g_InputFileName )))
            {
                // update globals, log, toolbar, menu items
                SetPreviewWindow();
                Mark_GraphMode_Menu( hwnd,IDM_FILETODV );   
                Mark_ToolBar_Button( TRUE, FALSE ); 
            } 
            else
                MBOX(TEXT("MakeFileToDvGraph_Type1() failed. Please set input file."));
    
            break;

        // Type 1 file (transmit)   
        case IDM_FILETODV_NOPRE :
            if(SUCCEEDED(g_pGraph->MakeFileToDvGraph_NoPre_Type1( g_InputFileName )))
            {
                // update globals, log, toolbar, menu items
                Mark_GraphMode_Menu( hwnd,IDM_FILETODV_NOPRE );   
                Mark_ToolBar_Button( TRUE, FALSE ); 
            }
            else
                MBOX(TEXT("MakeFileToDvGraph_NoPre_Type1() failed. Please set input file.")); 
            break;
        
        // Type 1 file (capture & preview)  
        case IDM_DVTOFILE :
            if(SUCCEEDED(g_pGraph->MakeDvToFileGraph_Type1( g_OutputFileName )))
            {
                // update globals, log, toolbar, menu items
                SetPreviewWindow();
                Mark_GraphMode_Menu( hwnd,IDM_DVTOFILE );   
                Mark_ToolBar_Button( TRUE, TRUE );  
            }
            else
                MBOX(TEXT("MakeDvToFileGraph_Type1() failed."));
            
            break;

        // Type 1 file (capture)    
        case IDM_DVTOFILE_NOPRE :
            if(SUCCEEDED(g_pGraph->MakeDvToFileGraph_NoPre_Type1( g_OutputFileName )))
            {
                // update globals, log, toolbar, menu items
                Mark_GraphMode_Menu( hwnd,IDM_DVTOFILE_NOPRE );   
                Mark_ToolBar_Button( TRUE, TRUE );  
            }
            else
                MBOX(TEXT("MakeDvToFileGraph_NoPre_Type1() failed."));
        
            break;

        // Type 2 File (transmit & playback)
        case IDM_FILETODV_TYPE2 :
            if(SUCCEEDED(g_pGraph->MakeFileToDvGraph_Type2( g_InputFileName )))
            {
                // update globals, log, toolbar, menu items
                SetPreviewWindow();
                Mark_GraphMode_Menu( hwnd,IDM_FILETODV_TYPE2 );   
                Mark_ToolBar_Button( TRUE, FALSE ); 
            }
            else
                MBOX(TEXT("MakeFileToDvGraph_Type2() failed. Please set input file."));
                
            break;

        // Type 2 File (transmit)
        case IDM_FILETODV_NOPRE_TYPE2 :
            if(SUCCEEDED(g_pGraph->MakeFileToDvGraph_NoPre_Type2( g_InputFileName )))
            {
                // update globals, log, toolbar, menu items
                Mark_GraphMode_Menu( hwnd,IDM_FILETODV_NOPRE_TYPE2 );   
                Mark_ToolBar_Button( TRUE, FALSE ); 
            }
            else
                MBOX(TEXT("MakeFileToDvGraph_NoPre_Type2() failed. Please set input file."));
                
            break;
        
        // Type 2 File (capture & preview)
        case IDM_DVTOFILE_TYPE2 :
            if(SUCCEEDED(g_pGraph->MakeDvToFileGraph_Type2( g_OutputFileName )))
            {
                // update globals, log, toolbar, menu items
                SetPreviewWindow();
                Mark_GraphMode_Menu( hwnd,IDM_DVTOFILE_TYPE2 );   
                Mark_ToolBar_Button( TRUE, TRUE );  
            } 
            else
                MBOX(TEXT("MakeDvToFileGraph_Type2() failed"));
    
            break;

        // Type 2 File (capture)
        case IDM_DVTOFILE_NOPRE_TYPE2 :
            if(SUCCEEDED(g_pGraph->MakeDvToFileGraph_NoPre_Type2( g_OutputFileName )))
            {
                // update globals, log, toolbar, menu items
                Mark_GraphMode_Menu( hwnd,IDM_DVTOFILE_NOPRE_TYPE2 );   
                Mark_ToolBar_Button( TRUE, TRUE );  
            } 
            else
                MBOX(TEXT("MakeDvToFileGraph_NoPre_Type2() failed"));
        
            break;

    } // switch (LOWORD(wParam))
}

/*-------------------------------------------------------------------------
Routine:        DV_TransportCommand_WndProc 
Purpose:        Message Handler to control transport state of the device & the filtergraph state
Arguments:    Usual message processing parameters
Returns:        Usual
Notes:          Handles for the Toolbar button controls
------------------------------------------------------------------------*/
void CALLBACK DV_TransportCommand_WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)
    {
    case WM_COMMAND :
        switch (LOWORD (wParam))
        {
        /*
            The VCR Tool bar commands need to behave differently depending on the 
            current graph.  

            In Preview (default) mode, the VCR commands should simply
            control the VCR functions on the vcr device.

            In DV To File mode, or File To DV mode the commands should start and stop the graph, 
            as  well as control the vcr mode (although we will disable some buttons to avoid confusion)
        */
        case IDM_PLAY :
        {
            //check if the current graph is a transmit graph
            if (GRAPH_FILE_TO_DV == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE == g_iGraphType || 
                GRAPH_FILE_TO_DV_TYPE2 == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE_TYPE2 == g_iGraphType)
            {
                // update the toolbar accordingly
                SendMessage(g_hwndTBar, TB_SETSTATE, IDM_PLAY, MAKELONG(TBSTATE_INDETERMINATE, 0L));

                // update the status bar with the dropped frames information
                g_CapStartTime = GetTickCount();
                SetTimer(hwnd, DV_TIMER_FRAMES,  DV_TIMERFREQ, (TIMERPROC) DV_DroppedFrameProc);

                // run the filter graph & wait for DirectShow events
                g_pGraph->StartGraph();
            } 
            else //capture and preview
            {
                // play the tape for the capture or preview graph
                DV_PutVcrMode(ED_MODE_PLAY);

                if(GRAPH_PREVIEW == g_iGraphType)
                    g_pGraph->StartGraph();

                //do we want to display timecodes?
                if (IsDlgButtonChecked(g_hwndTBar, IDC_TCCHECKBOX))
                {
                    SetTimer(hwnd, DV_TIMER_ATN,  DV_TIMERFREQ, (TIMERPROC) DV_TimecodeTimerProc);              
                    g_bUseAtnTimer = TRUE;
                }
                // disable checkbox
                EnableWindow( g_hwndTCCheck,   false);
            } 
            break;
        }

        /*
            The record button starts the *entire* graph, so preview (if selected), won't
            start until the recording starts.  
        */
        case IDM_RECORD :
            // update the toolbar accordingly
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));

            // check to see if it is a capture graph
            if (GRAPH_DV_TO_FILE == g_iGraphType || GRAPH_DV_TO_FILE_NOPRE == g_iGraphType ||
                GRAPH_DV_TO_FILE_TYPE2 == g_iGraphType || GRAPH_DV_TO_FILE_NOPRE_TYPE2 == g_iGraphType)
            {
                //do something here to record to an avi file on the disk - or to start recording on the vcr.
                switch (g_dwCaptureLimit)
                {
                    case DV_CAPLIMIT_NONE :
                        break;
                    case DV_CAPLIMIT_TIME :
                        SetTimer(hwnd, DV_TIMER_CAPLIMIT, g_dwTimeLimit * 1000, (TIMERPROC) DV_StopRecProc);
                        break;
                    case DV_CAPLIMIT_SIZE :
                        //rather than monitor disk usage, we'll just do the math and set a timer
                        SetTimer(hwnd, DV_TIMER_CAPLIMIT, ((g_dwDiskSpace * 100000) / DV_BYTESPERMSEC), (TIMERPROC) DV_StopRecProc);
                        break;
                    default :
                        //MBOX(TEXT("Bad value for g_dwCaptureLimit (%d)"), g_dwCaptureLimit);
                        break;
                }

                //update the status bar with the dropped frames information
                g_CapStartTime = GetTickCount();
                SetTimer(hwnd, DV_TIMER_FRAMES,  DV_TIMERFREQ, (TIMERPROC) DV_DroppedFrameProc);
                //run the graph - assume that the camera is already playing if in Vcr mode
                g_pGraph->StartGraph();             
            } 
            else if (GRAPH_FILE_TO_DV == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE == g_iGraphType ||
                     GRAPH_FILE_TO_DV_TYPE2 == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE_TYPE2 == g_iGraphType)
            {
                // if transmit graph then record on tape of the device
                DV_PutVcrMode(ED_MODE_RECORD);
            } 
            else
            {
                //we shouldn't get here
                MBOX( TEXT("Undefined graph mode (maybe GRAPH_PREVIEW) in IDM_RECORD message"));
            } 
            break;

        case IDM_STOP :   
            // handle the timers accordingly
            if (g_bUseAtnTimer){
                KillTimer(hwnd, DV_TIMER_ATN);
               
            }


            //if we're here, these timers were set
            KillTimer(hwnd, DV_TIMER_CAPLIMIT);
            KillTimer(hwnd, DV_TIMER_FRAMES);

            g_pGraph->StopGraph();
 
            // Stop the transport on the device
            DV_PutVcrMode(ED_MODE_STOP);
            // update the toolbar 
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_PLAY, MAKELONG(TBSTATE_ENABLED, 0L));
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));

            EnableWindow( g_hwndTCCheck,   true);
            break;

        case IDM_PAUSE :
            if (GRAPH_FILE_TO_DV == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE == g_iGraphType ||
                GRAPH_FILE_TO_DV_TYPE2 == g_iGraphType || GRAPH_FILE_TO_DV_NOPRE_TYPE2 == g_iGraphType)
            {   // transmit graph
                g_pGraph->PauseGraph();
            } 
            else
            {   // capture or preview graph
                DV_PutVcrMode(ED_MODE_FREEZE);
                SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_ENABLED, 0L));
                SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_ENABLED, 0L));
            } 
            //SendMessage(g_hwndTBar, TB_SETSTATE, IDM_PLAY, MAKELONG(TBSTATE_ENABLED, 0L));
            break;    

        case IDM_FF :
            // all graphs just forward the tape & update the toolbar
            DV_PutVcrMode(ED_MODE_FF);
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            break;

        case IDM_REW :
            // all graphs just rewind the tape & update the toolbar
            DV_PutVcrMode(ED_MODE_REW);
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            break;

        case IDM_PLAY_FAST_FF :
            // all graphs just forward the tape & update the toolbar
            DV_PutVcrMode(ED_MODE_PLAY_FASTEST_FWD);
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            break;

        case IDM_PLAY_FAST_REV :
            // all graphs just rewind the tape & update the toolbar
            DV_PutVcrMode(ED_MODE_PLAY_FASTEST_REV);
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));
            break;            

        case IDM_STEP_FWD :
            // all graphs just forward the tape & update the toolbar
            DV_PutVcrMode(ED_MODE_STEP_FWD);
            g_bUseAtnTimer = FALSE;
            DV_DisplayTimecode();
            break;

        case IDM_STEP_REV :
            // all graphs just rewind the tape & update the toolbar
            DV_PutVcrMode(ED_MODE_STEP_REV);
            g_bUseAtnTimer = FALSE;
            DV_DisplayTimecode();
            break;

        case IDM_SEEKTIMECODE :
            // ATN Seek & display on the toolbar
            DV_SeekATN();
            DV_DisplayTimecode();
            break;

        } // switch (LOWORD(wParam))
    } // switch (iMsg)
}


void Mark_GraphMode_Menu(HWND hwnd, int idmVal)
{
    //uncheck everything first
    CheckMenuItem(g_hmenu, IDM_PREVIEW, MF_UNCHECKED);
    CheckMenuItem(g_hmenu, IDM_FILETODV, MF_UNCHECKED);
    CheckMenuItem(g_hmenu, IDM_DVTOFILE, MF_UNCHECKED);
    CheckMenuItem(g_hmenu, IDM_FILETODV_NOPRE, MF_UNCHECKED);
    CheckMenuItem(g_hmenu, IDM_DVTOFILE_NOPRE, MF_UNCHECKED);

    CheckMenuItem(g_hmenu, IDM_FILETODV_TYPE2, MF_UNCHECKED);
    CheckMenuItem(g_hmenu, IDM_FILETODV_NOPRE_TYPE2, MF_UNCHECKED);
    CheckMenuItem(g_hmenu, IDM_DVTOFILE_TYPE2, MF_UNCHECKED);
    CheckMenuItem(g_hmenu, IDM_DVTOFILE_NOPRE_TYPE2, MF_UNCHECKED);

    // check the selected graph mode
    CheckMenuItem(g_hmenu, idmVal, MF_CHECKED);
}

void Mark_ToolBar_Button(BOOL bEnableRecord, BOOL bEnableOthers)
{
    if(bEnableRecord == TRUE)
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_RECORD, MAKELONG(TBSTATE_ENABLED, 0L));
    else
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_RECORD, MAKELONG(TBSTATE_INDETERMINATE, 0L));

    if(bEnableOthers == TRUE)
    {
        //re-enable everything 
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_ENABLED, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_REW, MAKELONG(TBSTATE_ENABLED, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_PLAY_FAST_REV, MAKELONG(TBSTATE_ENABLED, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_PLAY_FAST_FF, MAKELONG(TBSTATE_ENABLED, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_FF, MAKELONG(TBSTATE_ENABLED, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_ENABLED, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_SEEKTIMECODE, MAKELONG(TBSTATE_ENABLED, 0L));
    }
    else
    {
        //disable everything except for play, pause, and stop
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_REW, MAKELONG(TBSTATE_INDETERMINATE, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_PLAY_FAST_REV, MAKELONG(TBSTATE_INDETERMINATE, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_PLAY_FAST_FF, MAKELONG(TBSTATE_INDETERMINATE, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_FF, MAKELONG(TBSTATE_INDETERMINATE, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_STEP_FWD, MAKELONG(TBSTATE_INDETERMINATE, 0L));
        SendMessage(g_hwndTBar, TB_SETSTATE, IDM_SEEKTIMECODE, MAKELONG(TBSTATE_INDETERMINATE, 0L));
    }
}


void GetSelectedGraphMode( WPARAM wParam, GRAPH_TYPE* pGraphType)
{
    switch (LOWORD (wParam))
    {  
        case IDM_PREVIEW :
            *pGraphType = GRAPH_PREVIEW;
            break;
        case IDM_FILETODV :
            *pGraphType = GRAPH_FILE_TO_DV;
            break;
        case IDM_FILETODV_NOPRE :
            *pGraphType = GRAPH_FILE_TO_DV_NOPRE;
            break;
        case IDM_DVTOFILE :
            *pGraphType = GRAPH_DV_TO_FILE;
            break;
        case IDM_DVTOFILE_NOPRE :
            *pGraphType = GRAPH_DV_TO_FILE_NOPRE;
            break;
        case IDM_FILETODV_TYPE2 :
            *pGraphType = GRAPH_FILE_TO_DV_TYPE2;
            break;
        case IDM_FILETODV_NOPRE_TYPE2 :
            *pGraphType = GRAPH_FILE_TO_DV_NOPRE_TYPE2;
            break;
        case IDM_DVTOFILE_TYPE2 :
            *pGraphType = GRAPH_DV_TO_FILE_TYPE2;
            break;
        case IDM_DVTOFILE_NOPRE_TYPE2 :
            *pGraphType = GRAPH_DV_TO_FILE_NOPRE_TYPE2;
            break;
    }
}

/*-------------------------------------------------------------------------
Routine:        SetPreviewWindow
Purpose:        Hooks up stream  *from camera* to preview window
                    Note that the preview for the playback from the file is handled within DV_MakeFileToDvGraph() stuff
Arguments:    None
Returns:        HRESULT as appropriate
Notes:          
------------------------------------------------------------------------*/
HRESULT SetPreviewWindow(void)
{
    HRESULT hr = S_OK;

    ASSERT(g_pGraph->m_pVideoWindow!= NULL);
 
    //Set the video window.
    hr = g_pGraph->m_pVideoWindow->put_Owner((OAHWND)g_hwndApp);     // We own the window now
    if(FAILED(hr))
        return hr;

    // you are now a child
    hr = g_pGraph->m_pVideoWindow->put_WindowStyle(WS_CHILD| WS_CLIPSIBLINGS);     
    if(FAILED(hr))
        return hr;

    // give the preview window all our space but where the tool bar and status bar are        
    RECT clientRect, toolbarRect;
    GetClientRect(g_hwndApp, &clientRect);
    GetWindowRect(g_hwndTBar, &toolbarRect);

    g_pGraph->m_pVideoWindow->SetWindowPosition(0, toolbarRect.bottom - toolbarRect.top, g_iVWWidth, g_iVWHeight);

    hr = g_pGraph->m_pVideoWindow->put_Visible(OATRUE);
    if(FAILED(hr))
        return hr;

    return hr;
} 

/*-------------------------------------------------------------------------
Routine:        DV_DroppedFrameProc
Purpose:        Callback proc to display dropped frame info
Arguments:    DWORD current time
Returns:        None
Notes:          For both Capture & Transmit graphs
------------------------------------------------------------------------*/
void CALLBACK DV_DroppedFrameProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    HRESULT hr = S_OK;
    long dropped = 0, notdropped = 0;
    TCHAR buffer[128];
    DWORD time = dwTime - g_CapStartTime;

    BOOL bIsModeTransmit = TRUE;      
    hr = g_pGraph->getDroppedFrameNum( &bIsModeTransmit, &dropped, &notdropped);
    if(FAILED(hr))
    {
        DV_StatusText(TEXT("Cannot report dropped frame information"), 1);
        KillTimer(hwnd, DV_TIMER_FRAMES);
    }

    if( bIsModeTransmit == FALSE)
        hr = StringCchPrintf(buffer, NUMELMS(buffer), TEXT("Captured %d frames (%d dropped) %d.%d sec."), notdropped, dropped, time / 1000, time / 100 - time / 1000 * 10);
    else
        hr = StringCchPrintf(buffer, NUMELMS(buffer), TEXT("Transmitted %d frames (%d dropped) %d.%d sec."), notdropped, dropped, time / 1000, time / 100 - time / 1000 * 10);

    DV_StatusText(buffer, 1);
} 

/*-------------------------------------------------------------------------
Routine:        DV_TimecodeTimerProc
Purpose:        Callback function for the timer proc
Arguments:    Usual Timer Processing Parameters
Returns:        None
Notes:          
------------------------------------------------------------------------*/
void CALLBACK DV_TimecodeTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    DV_DisplayTimecode();
} 

/*-------------------------------------------------------------------------
Routine:        DV_DisplayTimecode
Purpose:        Routine to display timecodes
Arguments:    None
Returns:        None
Notes:          This is TimeCode Read not Absolute Track Number Read
------------------------------------------------------------------------*/
void DV_DisplayTimecode(void)
{
    TIMECODE_SAMPLE TimecodeSample;
    TimecodeSample.timecode.dwFrames = 0;
    static DWORD i1 = 0, i2 = 0, i3 = 0;
    TCHAR szBuf[4];

    TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;

    // Query the TimeCode sample data
    HRESULT hr = g_pGraph->m_pIAMTCReader->GetTimecode(&TimecodeSample);
    if(FAILED(hr))
    {
        //MBOX(TEXT("DV_DisplayTimecode::g_pGraph->m_pIAMTCReader->GetTimecode() failed"));
    }

    hr = StringCchPrintf(szBuf, NUMELMS(szBuf), TEXT("%.2x"),((TimecodeSample.timecode.dwFrames & 0xff000000) >> 24));
    SetDlgItemText(g_hwndTBar, IDC_EDIT_HOUR, szBuf);

    hr = StringCchPrintf(szBuf, NUMELMS(szBuf), TEXT("%.2x"),((TimecodeSample.timecode.dwFrames & 0x00ff0000) >> 16));
    SetDlgItemText(g_hwndTBar, IDC_EDIT_MINUTE, szBuf);

    hr = StringCchPrintf(szBuf, NUMELMS(szBuf), TEXT("%.2x"),((TimecodeSample.timecode.dwFrames & 0x0000ff00) >>  8));
    SetDlgItemText(g_hwndTBar, IDC_EDIT_SECOND, szBuf);

    hr = StringCchPrintf(szBuf, NUMELMS(szBuf), TEXT("%.2x"),(TimecodeSample.timecode.dwFrames & 0x000000ff));
    SetDlgItemText(g_hwndTBar, IDC_EDIT_FRAME, szBuf); 
} 

/*-------------------------------------------------------------------------
Routine:      DV_SeekATN
Purpose:      ATN Seek function - uses GetTransportBasicParameters to send RAW AVC command 
Arguments:    None
Returns:      TRUE if successful
Notes:        This is Absolute Track Number Seek not TimeCode Seek but uses the timecode display as input
------------------------------------------------------------------------*/
BOOL DV_SeekATN(void)
{
    BOOL bStatus = FALSE;
    HRESULT hr = S_OK;
    int iHr, iMn, iSc, iFr;

    //get the values from the edit fields
    iHr = GetDlgItemInt(g_hwndTBar, IDC_EDIT_HOUR,   &bStatus, FALSE);
    iMn = GetDlgItemInt(g_hwndTBar, IDC_EDIT_MINUTE, &bStatus, FALSE);
    iSc = GetDlgItemInt(g_hwndTBar, IDC_EDIT_SECOND, &bStatus, FALSE);
    iFr = GetDlgItemInt(g_hwndTBar, IDC_EDIT_FRAME,  &bStatus, FALSE);

    hr = g_pGraph->SeekATN( iHr, iMn, iSc, iFr );
    if(FAILED(hr))
    {
        MBOX(TEXT("Invalid Parameter - Time entered should be:\nHour:Minute:Second:Frame"));
        bStatus = FALSE;
    }

    return bStatus;
} 

/*-------------------------------------------------------------------------
Routine:        DV_UpdateTapeInfo
Purpose:        Get Frame rate and availability of dvcr tape
Arguments:    None
Returns:        HRESULT as appropriate
Notes:          
------------------------------------------------------------------------*/
BOOL DV_UpdateTapeInfo(void)
{
   //check information about the tape
    if (S_OK != g_pGraph->GetTapeInfo())
    {
        MBOX(TEXT("Tape is not inserted, or it has an improper format.\nReinsert the tape and select Options - Check Tape"));
        DV_StatusText(TEXT("VCR Mode - No tape, or unknown format"), 2);
        return FALSE;
    } 
    else
    {
        switch(g_pGraph->m_VideoFormat)
        {
            case DVENCODERVIDEOFORMAT_NTSC:
                DV_StatusText(TEXT("VCR Mode - NTSC"), 2);
                break;
            case DVENCODERVIDEOFORMAT_PAL:
                DV_StatusText(TEXT("VCR Mode - PAL"), 2);
                break;
            default:
                MBOX(TEXT("Unsupported or unrecognized tape format type"));
                break;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
Routine:        DV_RefreshMode
Purpose:        Use this to rebuild the necessary stuff to switch between VCR and camera mode
Arguments:    None
Returns:        TRUE if successful
Notes:          
------------------------------------------------------------------------*/
BOOL DV_RefreshMode(void)
{
    BOOL bStatus    = FALSE;

    // Query the current device type
    DV_MODE SubunitMode;
    g_pGraph->GetDVMode( &SubunitMode );

    switch(SubunitMode)
    {
        case CameraMode :
            // update the Graph Mode menu items & status window
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV, MF_GRAYED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV_NOPRE, MF_GRAYED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV_TYPE2, MF_GRAYED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV_NOPRE_TYPE2, MF_GRAYED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_CHECKTAPE, MF_GRAYED);
            DV_StatusText(TEXT("Camera Mode"), 2);
            bStatus = TRUE;
            break;

        case VcrMode :
            // Query the tape info & update the status bar
            DV_UpdateTapeInfo();  
            // update the Graph Mode menu items & status window
            EnableMenuItem(GetMenu(g_hwndApp), IDM_CHECKTAPE, MF_ENABLED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV, MF_ENABLED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV_NOPRE, MF_ENABLED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV_TYPE2, MF_ENABLED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_FILETODV_NOPRE_TYPE2, MF_ENABLED);
            EnableMenuItem(GetMenu(g_hwndApp), IDM_CHECKTAPE, MF_ENABLED);
            bStatus = TRUE;
            break;

        case UnknownMode :
            MBOX(TEXT("Cannot determine camera / VCR mode"));
            DV_StatusText(TEXT("Unknown Mode"), 2);            
            break;
            
        default :
            MBOX(TEXT("Bad return value from DV_RefreshMode"));
            break;
    }

    return bStatus;
} 

/*-------------------------------------------------------------------------
Routine:        DV_CapSizeDlgProc
Purpose:        Dialog proc for cap size dialog
Arguments:    Usual Dialog Processing parameters
Returns:        BOOL
Notes:          
------------------------------------------------------------------------*/
BOOL CALLBACK DV_CapSizeDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_INITDIALOG:
        {
            TCHAR           capDisk[8];
            ULARGE_INTEGER  ulFreeBytes;
            ULARGE_INTEGER  ulTotalBytes;
            ULARGE_INTEGER  ulAvailBytes;

            // make sure output file name has been input for capture
            if (!g_OutputFileName[0])
            {
                MBOX(TEXT("Please set up an output file name for recording"));
                EndDialog(hwnd, FALSE);
                return FALSE;
            } 

            //need to determine disk space and init the dialog appropriately
            StringCchCopyN(capDisk, NUMELMS(capDisk), g_OutputFileName, 3);  
            capDisk[4] = '\0';
            GetDiskFreeSpaceEx(capDisk, &ulFreeBytes, &ulTotalBytes, &ulAvailBytes);

            //let's see what our top limits are, and set our limits appropriately
            if ((ulAvailBytes.QuadPart / DV_BYTES_IN_MEG) < 120)
            {
                //less than 120 MB available - subtract 10 MB at a time until we get a usable amount
                UINT i = 110;
                while ((ulAvailBytes.QuadPart / DV_BYTES_IN_MEG) < i)
                {
                    i -= 10;
                } 
                SendMessage(GetDlgItem(hwnd, IDC_SPIN_TIME), UDM_SETRANGE, 0, MAKELONG(i / 4, 1));
                SendMessage(GetDlgItem(hwnd, IDC_SPIN_SIZE), UDM_SETRANGE, 0, MAKELONG(i, 1));
            }
            else
            {
                SendMessage(GetDlgItem(hwnd, IDC_SPIN_TIME), UDM_SETRANGE, 0, MAKELONG( ((ulAvailBytes.QuadPart / (1024 * 1024) ) - 10) / 4, 1));
                SendMessage(GetDlgItem(hwnd, IDC_SPIN_SIZE), UDM_SETRANGE, 0, MAKELONG( (ulAvailBytes.QuadPart / (1024 * 1024) ) - 10, 1));
            }

            //enable / disable the controls as appropriate
            switch (g_dwCaptureLimit)
            {
                case DV_CAPLIMIT_NONE :
                    Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_NOLIMIT), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SIZE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_SIZE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_TIME), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_TIME), FALSE);
                    break;
                
                case DV_CAPLIMIT_TIME :
                {
                    /*check the radio button, disable the size based controls */
                    Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_TIME), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SIZE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_SIZE), FALSE);

                    break;
                }
                case DV_CAPLIMIT_SIZE :
                {
                    /*check the radio button, disable the time based controls */
                    Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_SIZE), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_TIME), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_TIME), FALSE);
                    
                    break;                    
                }
            }
            SetDlgItemInt(hwnd, IDC_EDIT_TIME, g_dwTimeLimit, FALSE);
            SetDlgItemInt(hwnd, IDC_EDIT_SIZE, g_dwDiskSpace, FALSE);
            break;
        } 
        case WM_COMMAND :
            switch LOWORD(wParam)
            {
                // Update the controls ui according to the choices made
                case IDC_RADIO_NOLIMIT :
                {
                    Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_NOLIMIT), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SIZE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_SIZE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_TIME), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_TIME), FALSE);
                    break;
                }
                    
                case IDC_RADIO_TIME :
                {
                    Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_TIME), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SIZE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_SIZE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_TIME), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_TIME), TRUE);
                    break;
                } 
                
                case IDC_RADIO_SIZE :
                {
                    Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_SIZE), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_TIME), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_TIME), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SIZE), TRUE);
                    EnableWindow(GetDlgItem(hwnd, IDC_SPIN_SIZE), TRUE);
                    break;
                } 

                case IDOK :
                {
                    BOOL bTranslated = FALSE;
                    // The selections are made
                    // update the new global capture flag
                    if (Button_GetCheck(GetDlgItem(hwnd, IDC_RADIO_NOLIMIT)))
                    {
                        g_dwCaptureLimit = DV_CAPLIMIT_NONE;
                        //DV_LogOut(LOG_PRIORITY_INFO, LOG_LEVEL_MEDIUM, TEXT("No capture size limit"));
                    } 
                    else if (Button_GetCheck(GetDlgItem(hwnd, IDC_RADIO_TIME)))   
                    {
                        g_dwTimeLimit = GetDlgItemInt(hwnd, IDC_EDIT_TIME, &bTranslated, FALSE);
                        g_dwCaptureLimit = DV_CAPLIMIT_TIME;
                        //DV_LogOut(LOG_PRIORITY_INFO, LOG_LEVEL_MEDIUM, TEXT("Time based limit - %d"), g_dwTimeLimit);
                    } 
                    else if (Button_GetCheck(GetDlgItem(hwnd, IDC_RADIO_SIZE)))   
                    {
                        g_dwDiskSpace = GetDlgItemInt(hwnd, IDC_EDIT_SIZE, &bTranslated, FALSE);
                        g_dwCaptureLimit = DV_CAPLIMIT_SIZE;
                        //DV_LogOut(LOG_PRIORITY_INFO, LOG_LEVEL_MEDIUM, TEXT("Disk based limit - %d MB"), g_dwDiskSpace);
                    } 
                    EndDialog(hwnd, TRUE);
                    return TRUE;
                }
                
                case IDCANCEL :
                    // update nothing much
                    EndDialog(hwnd, FALSE);
                    return FALSE;

                default :
                    return FALSE;
            }            
            return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
Routine:        DV_StopRecProc
Purpose:        Callback to stop recording after a specified time
Arguments:    Usual Timer Processing Parameters
Returns:        None
Notes:          
------------------------------------------------------------------------*/
void CALLBACK DV_StopRecProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    SendMessage(g_hwndApp, WM_COMMAND, IDM_STOP, 0);
} 

/*-------------------------------------------------------------------------
Routine:        DV_AboutDlgProc
Purpose:        simple standard about dialog box proc 
Arguments:    Usual Dialog Processing parameters
Returns:        BOOL
Notes:          
------------------------------------------------------------------------*/
BOOL CALLBACK DV_AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
       //the only command we process are those that close the dialog   
        case WM_COMMAND:
            EndDialog(hwnd, TRUE);
            return TRUE;

        case WM_INITDIALOG:
            return TRUE;
    }

    return FALSE;
}


