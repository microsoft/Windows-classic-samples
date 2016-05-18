//------------------------------------------------------------------------------
// File: DvdSample.cpp
//
// Desc: DVD Playback sample app using IDvdGraphBuilder,
//       IDvdInfo2, and IDvdControl2 interfaces.
//
//       This contains all Windows-related "plumbing" for the application
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#include "resource.h"

#include "DvdCore.h"
#include "Dialogs.h"
#include "DvdSample.h"
#include <strsafe.h>

//------------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------------
const PTSTR APPNAME = TEXT("DVDSample");
CApp g_App;


//------------------------------------------------------------------------------
// Type Definitions
//------------------------------------------------------------------------------
typedef UINT(CALLBACK* PFNDLL_STES)(UINT);



//------------------------------------------------------------------------------
// Name: WinMain()
// Desc: This method is the standard Windows program entry point.
//------------------------------------------------------------------------------

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     PSTR      pCmdLine,
                     int       nCmdShow) 
{    
    DbgInitialise(hInstance);
    g_App.SetAppValues(hInstance, APPNAME, IDS_APP_TITLE) ;

    // Perform application initialization:
    if(! g_App.InitInstance(nCmdShow)) {
        DbgTerminate();
        return (FALSE) ;
    }

    HACCEL hAccelTable ;
    hAccelTable = LoadAccelerators(hInstance, g_App.GetAppName()) ;

    // Main message loop:
    MSG msg ;
    while(GetMessage(&msg, NULL, 0, 0)) {
        if(! TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg) ;
            DispatchMessage(&msg) ;
        }
    }

    DbgTerminate();
    return ((int) msg.wParam) ;
}


//------------------------------------------------------------------------------
// Name: CApp::CApp()
// Desc: This is the constructor for CApp.  It sets default values and disables 
//       power saving.
//------------------------------------------------------------------------------

CApp::CApp() : m_hInstance(NULL), 
               m_hWnd(NULL), 
               m_bCaptionsOn(false), 
               m_pDvdCore(NULL),
               m_bFullScreenOn(false),
               m_pParentLevels(NULL),
               m_pLangLookup(NULL),
               m_hwndToolBar(0),
               m_dwProhibitedTime(0) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::CApp()"))) ;

    ZeroMemory(m_szAppTitle, sizeof(m_szAppTitle)) ;
    ZeroMemory(m_szAppName, sizeof(m_szAppName)) ;

    m_pParentLevels = new CParentLevels();
    m_pLangLookup   = new CDVDLanguages();

    // Notify the power manager that the display is in use so it won't go to sleep
    // SetThreadExecutionState() is a Win98/Win2k API.
    // The following code lets it fail gracefully on Win95

    PFNDLL_STES pfn;
    pfn = (PFNDLL_STES) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
        "SetThreadExecutionState");
    if(pfn) {
        pfn(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
    }
}


//------------------------------------------------------------------------------
// Name: CApp::~CApp()
// Desc: This is the destructor for CApp.
//------------------------------------------------------------------------------

CApp::~CApp() 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::~CApp()"))) ;

    delete m_pParentLevels;
    delete m_pLangLookup;
    delete m_pDvdCore;
    m_pDvdCore = NULL;

    // Notify the power manager that the display is no longer in use and it can go to sleep
    // SetThreadExecutionState() is a Win98/Win2k API.
    // The following code lets it fail gracefully on Win95
    PFNDLL_STES pfn;
    pfn = (PFNDLL_STES) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
        "SetThreadExecutionState");
    if(pfn) {
        pfn(ES_CONTINUOUS);
    }
}


//------------------------------------------------------------------------------
// Name: CApp::SetAppValues()
// Desc: This method sets the basic application values like szAppName and hInstance
//------------------------------------------------------------------------------

void CApp::SetAppValues(HINSTANCE hInst, PTSTR szAppName, int iAppTitleResId) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::SetAppValues()"))) ;

        m_hInstance = hInst ;

    StringCchCopy(m_szAppName, NUMELMS(m_szAppName), APPNAME);
    LoadString(m_hInstance, IDS_APP_TITLE, m_szAppTitle, NUMELMS(m_szAppTitle)) ;
}


//------------------------------------------------------------------------------
// Name: CApp::InitInstance()
// Desc: This method registers and creates our window and toolbar.
//------------------------------------------------------------------------------

bool CApp::InitInstance(int nCmdShow) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::InitInstance()"))) ;

    // Win32 will always set hPrevInstance to NULL, so check
    // things a little closer.  This is because we only want a single
    // version of this app to run at a time.
    m_hWnd = FindWindow(m_szAppName, m_szAppTitle) ;
    if(m_hWnd) {
        // We found another instance of ourself. Lets use that one:
        if(IsIconic(m_hWnd)) {
            ShowWindow(m_hWnd, SW_RESTORE);
        }
        SetForegroundWindow(m_hWnd);

        // If this app actually had any methodality, we would
        // also want to communicate any action that our 'twin'
        // should now perform based on how the user tried to
        // execute us.
        return false;
    }

    // Register the app main window class
    WNDCLASSEX  wc ;
    wc.cbSize        = sizeof(wc) ;
    wc.style         = CS_HREDRAW | CS_VREDRAW ;
    wc.lpfnWndProc   = (WNDPROC) WndProc ;
    wc.cbClsExtra    = 0 ;
    wc.cbWndExtra    = 0 ;
    wc.hInstance     = m_hInstance ;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1) ;
    wc.lpszMenuName  = TEXT("DvdSample_Menu");
    wc.lpszClassName = m_szAppName ;
    wc.hIconSm       = NULL ;

    if(0 == RegisterClassEx(&wc)) {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: RegisterClassEx() for app class failed (Error %ld)"), 
            GetLastError())) ;
        return false ;
    }

    // Determine where to put the Application Window
    RECT rDesktop;
    SystemParametersInfo(SPI_GETWORKAREA, NULL, &rDesktop, NULL);

    // Create an instance of the window we just registered
    // locate it at the bottom of the screen (bottom of screen - height of player)
    m_hWnd = CreateWindowEx(0, m_szAppName, m_szAppTitle, WS_OVERLAPPEDWINDOW, //& ~WS_THICKFRAME,
        160, rDesktop.bottom - 150, 300, 150, 
        NULL, NULL, m_hInstance, NULL);
    if(!m_hWnd) {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: CreateWindowEx() failed (Error %ld)"), 
            GetLastError())) ;
        return false ;
    }

    // We now create the toolbar
    INITCOMMONCONTROLSEX cc;
    cc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    cc.dwICC = ICC_BAR_CLASSES; // register only the toolbar control
    InitCommonControlsEx(&cc);

#ifdef _WIN64
    // BYTE      bReserved[6]     // padding for alignment
#define PAD 0,0,0,0,0,0,  0,0,
#elif defined(_WIN32)
    // BYTE      bReserved[2]     // padding for alignment
#define PAD 0,0,  0,0,
#endif

        // Configure toolbar buttons    
        TBBUTTON tbb[] = 
    {
        0, ID_PLAYBACK_PREVIOUSCHAPTER, TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        1, ID_PLAYBACK_REWIND,          TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        2, ID_PLAYBACK_PAUSE,           TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        3, ID_PLAYBACK_PLAY,            TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        4, ID_PLAYBACK_STOP,            TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        5, ID_PLAYBACK_FASTFORWARD,     TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        6, ID_PLAYBACK_NEXTCHAPTER,     TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        9, 0,                           TBSTATE_ENABLED, TBSTYLE_SEP,    PAD
        7, ID_PLAYBACK_MENUROOT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        8, ID_OPTIONS_FULLSCREEN,       TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
        9, ID_PLAYBACK_STEPFORWARD,     TBSTATE_ENABLED, TBSTYLE_BUTTON, PAD
    };

    m_hwndToolBar = CreateToolbarEx(m_hWnd, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_TOP
        | TBSTYLE_TOOLTIPS, 1, 10, m_hInstance, IDR_TOOLBAR1, tbb, 11, 0, 0, 0, 0,
        sizeof(TBBUTTON));
    if(!m_hwndToolBar) {
        DbgLog((LOG_ERROR, 0, 
            TEXT("ERROR: CreateToolbarEx() failed (Error %ld)"), 
            GetLastError())) ;
        return false ;
    }

    // we now set up the dvd playback class
    m_pDvdCore = new CDvdCore(m_hInstance, this);
    if(!m_pDvdCore->Init()) {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: CDvdCore::Init() failed"))) ;
        return false;
    }

    m_pDvdCore->SetVideoWindowTitle(TEXT("DvdSample Video Window"));

    // and finally, we make the window visible
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd) ;

    return true;
}


//------------------------------------------------------------------------------
// Name: WndProc()
// Desc: This method is our main window procedure.  It parses the messages
//       and farms out the work to other procedures.
//------------------------------------------------------------------------------

LRESULT CApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static HDC  hDC ;
    static PAINTSTRUCT ps;

    switch(message) {

        case WM_PAINT:
            hDC = BeginPaint(hWnd, &ps) ;
            ASSERT(hDC);
            if (hDC)
                g_App.DrawStatus(hDC);
            EndPaint(hWnd, &ps) ;
            break ;

        case WM_KEYUP:
            g_App.KeyProc(wParam, lParam) ;
            break ;

        case WM_COMMAND:
            g_App.MenuProc(hWnd, wParam, lParam) ;
            break;

        case WM_NOTIFY:
            return g_App.ToolTipProc(hWnd, message, wParam, lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            // we do this to cause the toolbar to resize correctly
            SendMessage(g_App.m_hwndToolBar, WM_SIZE, wParam, lParam);
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    return 0; // let Windows know we handled the message
}


//------------------------------------------------------------------------------
// Name: ToolTipProc()
// Desc: This method is a MessageProc for our toolbar tooltips.
//       This checks whether the WM_NOTIFY message is really a tooltip.
//       If it is, we compare it to the buttons on the toolbar and send the right text back.
//       Note that older code would use TTN_NEEDTEXT instead of TTN_GETDISPINFO.
//------------------------------------------------------------------------------

LRESULT CApp::ToolTipProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::ToolTipProc()"))) ;

    LPNMTTDISPINFO TText;
    TText = (LPNMTTDISPINFO) lParam;
    
    if(TTN_GETDISPINFO == TText->hdr.code) 
    {
        switch(TText->hdr.idFrom) {
            case ID_PLAYBACK_PREVIOUSCHAPTER:
                TText->lpszText = TEXT("Previous Chapter\0");
                break;
            case ID_PLAYBACK_REWIND:
                TText->lpszText = TEXT("Rewind\0");
                break;
            case ID_PLAYBACK_PAUSE:
                TText->lpszText = TEXT("Pause\0");
                break;
            case ID_PLAYBACK_PLAY:
                TText->lpszText = TEXT("Play\0");
                break;
            case ID_PLAYBACK_STOP:
                TText->lpszText = TEXT("Stop\0");
                break;
            case ID_PLAYBACK_FASTFORWARD:
                TText->lpszText = TEXT("FastForward\0");
                break;
            case ID_PLAYBACK_NEXTCHAPTER:
                TText->lpszText = TEXT("Next Chapter\0");
                break;
            case ID_PLAYBACK_MENUROOT:
                TText->lpszText = TEXT("Root Menu / Resume\0");
                break;
            case ID_OPTIONS_FULLSCREEN:
                TText->lpszText = TEXT("Full Screen\0");
                break;
            case ID_PLAYBACK_STEPFORWARD:
                TText->lpszText = TEXT("Step Forward\0");
                break;
        }

        return 0;
    }
    else
        return DefWindowProc(hWnd, message, wParam, lParam); // not a tooltip message
}


//------------------------------------------------------------------------------
// Name: CApp::MenuProc()
// Desc: This method handles all of the menu messages for our application.  It is
//       passed these messages by the main message proc.
//------------------------------------------------------------------------------

LRESULT CApp::MenuProc(HWND hWnd, WPARAM wParam, LPARAM lParam) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::MenuProc()"))) ;

    HMENU hMenu = GetMenu(hWnd) ;

    //Parse the menu selections:
    switch(LOWORD(wParam)) 
    {
        case ID_PLAYBACK_PLAY:
            m_pDvdCore->Play();
            break;

        case ID_PLAYBACK_PREVIOUSCHAPTER:
            m_pDvdCore->PrevChapter();
            break;

        case ID_PLAYBACK_REWIND:
            m_pDvdCore->Rewind();
            break;

        case ID_PLAYBACK_PAUSE:
            m_pDvdCore->Pause();
            break;

        case ID_PLAYBACK_STOP:
            m_pDvdCore->Stop();
            break;

        case ID_PLAYBACK_FASTFORWARD:
            m_pDvdCore->FastForward();
            break;

        case ID_PLAYBACK_NEXTCHAPTER:
            m_pDvdCore->NextChapter();
            break;

        case ID_PLAYBACK_MENUROOT:
            m_pDvdCore->RootMenu();
            break;

        case ID_OPTIONS_FULLSCREEN:
            m_pDvdCore->ToggleFullScreen();
            break;

        case ID_PLAYBACK_TITLEMENU:
            m_pDvdCore->TitleMenu();
            break;

        case ID_FILE_EXIT:
            DestroyWindow(m_hWnd);
            break;

        case ID_OPTIONS_CLOSEDCAPTION:
            if(false == m_bCaptionsOn) // turn them on
            {
                HMENU hSubMenu = GetSubMenu(hMenu, 2); // Options is the 3rd menu starting with 0
                CheckMenuItem(hSubMenu, ID_OPTIONS_CLOSEDCAPTION, MF_BYCOMMAND | MF_CHECKED);
                if(m_pDvdCore->EnableCaptions(true))
                    m_bCaptionsOn = true;
            }
            else // turn them off
            {
                HMENU hSubMenu = GetSubMenu(hMenu, 2); // Options is the 3rd menu starting with 0
                CheckMenuItem(hSubMenu, ID_OPTIONS_CLOSEDCAPTION, MF_BYCOMMAND | MF_UNCHECKED);
                if(m_pDvdCore->EnableCaptions(false))
                    m_bCaptionsOn = false;
            }
            break;

        case ID_OPTIONS_SAVEBOOKMARK:
            m_pDvdCore->SaveBookmark();
            break;

        case ID_OPTIONS_RESTOREBOOKMARK:
            m_pDvdCore->RestoreBookmark();
            break;

        case ID_OPTIONS_PARENTALLEVEL:
            if(Nav_Stopped != m_pDvdCore->GetState() && Graph_Stopped2 != m_pDvdCore->GetState())
                // can't change parental level except in stop state
            {
                MessageBox(m_hWnd, 
                    TEXT("Can't change parental control level during playback. Please stop (twice) first."), 
                    TEXT("Error"), MB_OK | MB_ICONINFORMATION) ;
                break ;
            }

            DialogBox(m_hInstance, MAKEINTRESOURCE(IDD_PARENTLEVELS), m_hWnd,
                reinterpret_cast<DLGPROC>(SelectParentalLevel));

            break;

        case ID_HELP_ABOUTDVDSAMPLE: {
                    CAboutDlg aDlg(m_hInstance, m_hWnd);
                    aDlg.DoModal();
            }
            break;

        case ID_OPTIONS_SUBPICTURE: {
                    CSPLangDlg aDlg(m_hInstance, m_hWnd);
                    aDlg.DoModal();
            }
            break;

        case ID_OPTIONS_AUDIOLANGUAGE: {
                    CAudioLangDlg aDlg(m_hInstance, m_hWnd);
                    aDlg.DoModal();
            }
            break;

        case ID_OPTIONS_ANGLE: {
                    CAngleDlg aDlg(m_hInstance, m_hWnd);
                    aDlg.DoModal();
            }
            break;

        case ID_FILE_SELECTDISC:
            OnSelectDisc();
            break;

        case ID_PLAYBACK_STEPFORWARD:
            m_pDvdCore->FrameStep();
            break;

        case ID_PLAYBACK_GOTO_CHAPTER: {
                CChapterDlg aDlg(m_hInstance, m_hWnd);
                if(false == aDlg.DoModal())
                    break;

                m_pDvdCore->PlayChapter(aDlg.GetChapter());
            }
            break;

        case ID_PLAYBACK_GOTO_TITLE: {
                CTitleDlg aDlg(m_hInstance, m_hWnd);
                if(false == aDlg.DoModal())
                    break;

                m_pDvdCore->PlayChapterInTitle(aDlg.GetTitle(), aDlg.GetChapter());
            }
            break;

        case ID_PLAYBACK_GOTO_TIME: {
                CTimeDlg aDlg(m_hInstance, m_hWnd);

                DVD_HMSF_TIMECODE time = m_pDvdCore->GetTime();
                aDlg.SetTime(time);

                if(false == aDlg.DoModal())
                    break;

                m_pDvdCore->PlayTime(aDlg.GetTime());
            }
            break;

        case ID_OPTIONS_GETDISCTEXT:
            m_pDvdCore->GetDvdText();
            break;

        case ID_OPTIONS_GETAUDIOATTRIBUTES:
            m_pDvdCore->GetAudioAttributes();
            break;

        case ID_OPTIONS_GETVIDEOATTRIBUTES:
            m_pDvdCore->GetVideoAttributes();
            break;

        case ID_OPTIONS_GETSUBPICTUREATTRIBUTES:
            m_pDvdCore->GetSPAttributes();
            break;

        case ID_OPTIONS_SETKARAOKEMIXING: {
                CKaraokeDlg aDlg(m_hInstance, m_hWnd);
                aDlg.DoModal(); // all work happens in the dialog code
            }
            break;

        case ID_OPTIONS_USEVMR9: {
                UINT uChecked = (m_pDvdCore->ToggleVMR9AndRebuildGraph()) ? (MF_CHECKED) : (MF_UNCHECKED);
                HMENU hOptMenu = GetSubMenu(hMenu, 2);  //  options menu
                CheckMenuItem(hOptMenu, ID_OPTIONS_USEVMR9, uChecked | MF_BYCOMMAND);
                break;
            }

        default:
            break;
    }

    return 0;
}


//------------------------------------------------------------------------------
// Name: CApp::KeyProc()
// Desc: This method will process all key presses sent to our application window.
//       At present it passes all of the keys along to the DvdCore but this is where
//       you would implement shortcut keys, etc.
//------------------------------------------------------------------------------

LRESULT CApp::KeyProc(WPARAM wParam, LPARAM lParam) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::KeyProc()"))) ;

    switch(wParam) {
        case VK_ESCAPE: // exit full screen
        case VK_RETURN: // activate the currently selected button
        case VK_LEFT: // select the left button
        case VK_RIGHT: // select the right button
        case VK_UP: // select the upper button
        case VK_DOWN: // select the lower button
            return m_pDvdCore->OnKeyEvent(wParam, lParam); // pass these keys on to the Core.
    }

    return DefWindowProc(m_hWnd, WM_KEYUP, wParam, lParam);
}


//------------------------------------------------------------------------------
// Name: CApp::SelectParentalLevel()
// Desc: This method is the MessageProc for the Parental Level Dialog.  It is used
//       to allow the user to select the parental level setting.
//
//       The Dialog wrapper class is not used to demonstrate an alternate way to 
//       handle dialogs.
//------------------------------------------------------------------------------

BOOL CALLBACK CApp::SelectParentalLevel(HWND hDlg, UINT message, 
                                        WPARAM wParam, LPARAM lParam) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::SelectParentalLevel()"))) ;

    switch(message) 
    {
        case WM_INITDIALOG:
            g_App.MakeParentLevelList(hDlg, IDC_LEVEL_LIST) ;
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDOK: {
                        LONG  lLevel ;
                        lLevel = (LONG) SendDlgItemMessage(hDlg, IDC_LEVEL_LIST, LB_GETCURSEL, 
                            static_cast<WPARAM>(0), static_cast<LPARAM>(0));
                        if(CB_ERR == lLevel)
                            DbgLog((LOG_ERROR, 1, 
                                    TEXT("WARNING: Couldn't get selected parental control level (Error %d)"), lLevel)) ;
                        else
                            g_App.m_pDvdCore->SetParentalLevel(g_App.m_pParentLevels->GetValue(lLevel)) ;
                    }

                    // Now fall through to just end the dialog

                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    return TRUE;
            }
            break;

        default:
            break;
    }

    return FALSE;
}


//------------------------------------------------------------------------------
// Name: CApp::UpdateStatus()
// Desc: This method is notified every time that status of the player changes
//       (Time, title, Chapter, etc.).  This invalidates the client rectangle to force
//       a redraw of the screen.
//------------------------------------------------------------------------------

void CApp::UpdateStatus(void) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::UpdateStatus()"))) ;

    RECT Rect;
    if(FALSE != GetClientRect(m_hWnd, &Rect)) {
        Rect.top += 30; // so we don't redraw the toolbar - 30 is just a rough number
        InvalidateRect(m_hWnd, &Rect, TRUE);
    }
    else {
        DWORD res = GetLastError();
        DbgLog((LOG_ERROR, 1, TEXT("GetClientRect failed: %#x"), res)) ;
    }
}


//------------------------------------------------------------------------------
// Name: CApp::DrawStatus()
// Desc: This method draws our status test (time, title, chapter) on the screen
//------------------------------------------------------------------------------

void CApp::DrawStatus(HDC hDC) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::DrawStatus()"))) ;

    TCHAR location[35];
    HRESULT hr = StringCchPrintf(location, NUMELMS(location), TEXT("Title: %-6uChapter: %u\0"), m_pDvdCore->GetTitle(), 
        m_pDvdCore->GetChapter());
    TextOut(hDC, 10, 50, location, lstrlen(location));

    TCHAR time[25];
    hr = StringCchPrintf(time, NUMELMS(time), TEXT("Time: %02d:%02d:%02d\0"), m_pDvdCore->GetTime().bHours, 
        m_pDvdCore->GetTime().bMinutes, m_pDvdCore->GetTime().bSeconds);
    TextOut(hDC, 10, 65, time, lstrlen(time));

    if(timeGetTime() <= (m_dwProhibitedTime + 5000)) // if less than 5 seconds has passed
    {
        SetTextColor(hDC, RGB(255, 0, 0));
        TextOut(hDC, 180, 80, TEXT("Prohibited!"), 11);
    }
}


//------------------------------------------------------------------------------
// Name: CApp::OnSelectDisc()
// Desc: This method brings up a common file dialog to ask the user to find the
//       dvd disc they wish to watch.  We look for the video_ts.ifo file.
//------------------------------------------------------------------------------

bool CApp::OnSelectDisc() 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::OnSelectDisc()"))) ;

    OPENFILENAME  ofn ; // structure used by the common file dialog
    TCHAR         szFileName[MAX_PATH] ;

    // Init the filename buffer with *.ifo
    HRESULT hr = StringCchPrintf(szFileName, NUMELMS(szFileName), TEXT("*.ifo\0")) ;

    ZeroMemory(&ofn, sizeof(OPENFILENAME)) ;
    ofn.lStructSize = sizeof(OPENFILENAME) ;
    ofn.hwndOwner = m_hWnd ;
    ofn.lpstrFilter = TEXT("IFO Files\0*.ifo\0All Files\0*.*\0") ;
    ofn.nFilterIndex = 1 ;
    ofn.lpstrFile = szFileName ;
    ofn.nMaxFile = sizeof(szFileName) ;
    ofn.lpstrFileTitle = NULL ;
    ofn.lpstrTitle = TEXT("Select DVD-Video Volume\0") ;
    ofn.nMaxFileTitle = 0 ;
    ofn.lpstrInitialDir = NULL ;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY ;

    // GetOpenFileName will bring up the common file dialog in open mode
    if(GetOpenFileName(&ofn)) // user specified a file
    {      
        return m_pDvdCore->SetDirectory(szFileName);
    }

    // Either failed or user hit Esc.
    DWORD dw = CommDlgExtendedError() ;
    DbgLog((LOG_TRACE, 3, TEXT("GetOpenFileName() cancelled/failed with error %lu"), dw)) ;

    return false ; // DVD-Video volume not changed
}


//------------------------------------------------------------------------------
// Name: CApp::Prohibited()
// Desc: This method is called by the DVDCore whenever an operation is attempted that
//       is prohibited by UOP.  We set a time value and then invalidate the rectangle.
//------------------------------------------------------------------------------

void CApp::Prohibited() 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::Prohibited()"))) ;

    m_dwProhibitedTime = timeGetTime();

    RECT Rect;
    if(FALSE != GetClientRect(m_hWnd, &Rect)) {
        Rect.top += 30; // so we don't redraw the toolbar - 30 is just a rough number
        InvalidateRect(m_hWnd, &Rect, TRUE);
    }
    else {
        DWORD res = GetLastError();
        DbgLog((LOG_ERROR, 1, TEXT("GetClientRect failed: %#x"), res)) ;
    }
}


//------------------------------------------------------------------------------
// Name: CApp::Exit()
// Desc: This method is part of IDvdCallBack.  It is called by the DVDCore whenever 
//       the playback window si closed.  We then close down the application. 
//------------------------------------------------------------------------------

void CApp::Exit() 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::Exit()"))) ;
    PostQuitMessage(0);
}


//------------------------------------------------------------------------------
// Name: CApp::GetAppWindow()
// Desc: This method is a member of IDVDCallback.  It returns the location and size
//       of the player window to the application.
//------------------------------------------------------------------------------

RECT CApp::GetAppWindow() 
{
    RECT rWindow;
    GetWindowRect(m_hWnd, &rWindow);
    return rWindow;
}


//------------------------------------------------------------------------------
// Name: CApp::MakeParentLevelList()
// Desc: This method creates a list of parental levels and adds them to the
//       parental level dialog box's listbox.
//------------------------------------------------------------------------------

int CApp::MakeParentLevelList(HWND hDlg, int iListID) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CApp::MakeParentalLevelList(0x%lx, %d)"),
        hDlg, iListID)) ;

    int iLevels = m_pParentLevels->GetCount() ;
    int iResult ;

    // add all defined parental levels
    for(int i = 0 ; i < iLevels ; i++) 
    {
        // Add to the listbox now
        iResult = (int) SendDlgItemMessage(hDlg, iListID, LB_ADDSTRING, (WPARAM) 0, 
            (LPARAM)(LPVOID) m_pParentLevels->GetName(i)) ;

        if(LB_ERR == iResult || LB_ERRSPACE == iResult) {
            DbgLog((LOG_ERROR, 1, 
                TEXT("Error (%d) adding parental level '%s'(%d) to list"), 
                iResult, m_pParentLevels->GetName(i), i)) ;
        }
    }

    if(iLevels > 0) 
    {
        iResult = (int) SendDlgItemMessage(hDlg, iListID, LB_SETCURSEL, 
            (WPARAM) m_pDvdCore->GetParentalLevel(), (LPARAM) 0) ;

        if(LB_ERR == iResult) {
            DbgLog((LOG_ERROR, 1, 
                TEXT("WARNING: Couldn't set %ld as selected parent level (Error %d)"),
                m_ulParentCtrlLevel, iResult)) ;
        }
    }
    return iLevels ;
}


//------------------------------------------------------------------------------
// Name: CDVDLanguages::CDVDLanguages()
// Desc: This is the constructor.  It sets up our lookup table
//       Only 10 languages have been used here as a sample.
//       The list can be extended to include any language listed in ISO 639.
//------------------------------------------------------------------------------
CDVDLanguages::CDVDLanguages() 
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDLanguages::CDVDLanguages()"))) ;

    m_lcidCodes[0] = 0x0407 ;    m_apszLangNames[0] = TEXT("German\0") ;
    m_lcidCodes[1] = 0x0409 ;    m_apszLangNames[1] = TEXT("English\0") ;
    m_lcidCodes[2] = 0x040a ;    m_apszLangNames[2] = TEXT("Spanish\0") ;
    m_lcidCodes[3] = 0x040c ;    m_apszLangNames[3] = TEXT("French\0") ;
    m_lcidCodes[4] = 0x0411 ;    m_apszLangNames[4] = TEXT("Japanese\0") ;
    m_lcidCodes[5] = 0x0412 ;    m_apszLangNames[5] = TEXT("Korean\0") ;
    m_lcidCodes[6] = 0x0413 ;    m_apszLangNames[6] = TEXT("Dutch\0") ;
    m_lcidCodes[7] = 0x0816 ;    m_apszLangNames[7] = TEXT("Portuguese\0") ;
    m_lcidCodes[8] = 0x041d ;    m_apszLangNames[8] = TEXT("Swedish\0") ;
    m_lcidCodes[9] = 0x0804 ;    m_apszLangNames[9] = TEXT("Chinese\0") ;
}


//------------------------------------------------------------------------------
// Name: CDVDLanguages::GetLangString()
// Desc: This method is our lookup function.  It takes an LCID language code
//       and returns an English string language name.
//------------------------------------------------------------------------------

bool CDVDLanguages::GetLangString(LCID LangCode, PTSTR pszLang, int iMaxLen) 
{
    DbgLog((LOG_TRACE, 5, TEXT("CDVDLanguages::GetLangString()"))) ;

    HRESULT hr;

    for(int i = 0 ; i < 10 ; i++) {
        if(LangCode == m_lcidCodes[i])  // match!!
        {
            hr = StringCchCopy(pszLang, iMaxLen, m_apszLangNames[i]) ;
            if (hr == S_OK)
            {
                return true ;  // got a match
            }
            // let it fall through if that fails
        }
    }

    hr = StringCchCopy(pszLang, iMaxLen, TEXT("Unknown\0")) ;
    return false;
}


//------------------------------------------------------------------------------
// Name: CParentLevels::CParentLevels()
// Desc: This is the constructor for CParentLevels.  It can be modified to change
//       the parental levels shown to the user.
//       Remember that the mapping of parental levels to MPAA ratings shown here
//       is only valid for the U.S. and Canada.  In other countries, the parental
//       levels will have different specific meanings, although parental level 1
//       is always the least restrictive level and level 8 is the most restrictive.
//------------------------------------------------------------------------------

CParentLevels::CParentLevels() 
{
    DbgLog((LOG_TRACE, 5, TEXT("CParentLevels::CParentLevels()"))) ;

    m_iCount = LEVELS ;
    m_alpszNames[0] = TEXT(" G \0") ;         m_aiValues[0] = 1 ;
    m_alpszNames[1] = TEXT(" PG \0") ;        m_aiValues[1] = 3 ;
    m_alpszNames[2] = TEXT(" PG-13 \0") ;     m_aiValues[2] = 4 ;
    m_alpszNames[3] = TEXT(" R \0") ;         m_aiValues[3] = 6 ;
    m_alpszNames[4] = TEXT(" NC-17 \0") ;     m_aiValues[4] = 7 ;
    m_alpszNames[5] = TEXT(" Not Rated \0") ; m_aiValues[5] = 8 ;
}




