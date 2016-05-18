///////////////////////////////////////////////////////////////////////////
// 
// MODULE:      MOUSINFO.C
//
// DESCRIPTION: SDK sample for handling the WM_MOUSEWHEEL message and 
//              the TrackMouseEvent() API.
//
//              Applet displays MouseButton, MouseWheel, MouseMovement, and
//              any mouse messages in the title bar.
//
// PLATFORMS:   WinNT 4.0 and newer
//
//              Copyright 1995 - 2000 Microsoft Corporation. 
//              All rights reserved
// 
//
///////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include "mousinfo.h"

// Globals
HINSTANCE ghInst = NULL;

//
// Array of Mouse Message Strings
//
TCHAR garMsgStrings[][20] = 
{
    TEXT("                   "),
    TEXT("WM_LBUTTONUP       "),
    TEXT("WM_RBUTTONUP       "),
    TEXT("WM_MBUTTONUP       "),
    TEXT("WM_LBUTTONDOWN     "),
    TEXT("WM_RBUTTONDOWN     "),
    TEXT("WM_MBUTTONDOWN     "),
    TEXT("WM_LBUTTONDBLCLK   "),
    TEXT("WM_MBUTTONDBLCLK   "),
    TEXT("WM_RBUTTONDBLCLK   "),
    TEXT("WM_MOUSEWHEEL      "),
    TEXT("WM_MOUSEHOVER      "),
    TEXT("WM_MOUSELEAVE      ")
};


//////////////////////////////////////////////////////////////////////////
//
// FUNCTION:	int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int)
//
// DESCRIPTION:	Your basic WinMain ()
//
/////////////////////////////////////////////////////////////////////////
int WINAPI WinMain (
    __in HINSTANCE hInstance, 
    __in_opt HINSTANCE hPrevInstance, 
    __in_opt LPSTR lpCmdLine, 
    __in int nCmdShow )
{
    MSG msg = {0};
    WNDCLASS wc = {0};
    HWND hwndMain = NULL;
    BOOL bRet = FALSE;
    BOOL bRetZero = FALSE;   


    ghInst = hInstance;

    //
    // Register our main window
    //
    wc.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC) MainWindowProc;
    wc.hInstance     = ghInst;       
    wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(MOUSINFO_ICON));
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE (IDM_MENU);
    wc.lpszClassName = szClassName;

    if (!RegisterClass(&wc)) 
    {
        bRetZero = TRUE;
        goto exit_func;
    }

    //
    // Create the main window...
    // with an initial size of 0
    //
    hwndMain = CreateWindow (
        szClassName,
        szAppName,
        WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX,
        0,0,0,0,
        NULL, NULL, ghInst, NULL);


    //
    // Show the window
    //
    ShowWindow (hwndMain,SW_SHOW);
    UpdateWindow (hwndMain);
  
    //
    // Message pump
    //
    while ((bRet = GetMessage (&msg, NULL, 0, 0)) != 0)
    { 
        if (bRet == -1)
        {
            bRetZero = TRUE;
            goto exit_func;
        }
        else
        {
            TranslateMessage (&msg); 
            DispatchMessage (&msg); 
        }
    } 

exit_func:
    if (bRetZero)
    {
        return 0;
    }
    else
    {
        return (int) msg.wParam;
    }
}


//////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION:    LRESULT CALLBACK MainWindowProc (HWND, UINT, WPARAM, LPARAM)
//
// DESCRIPTION: Your standard Window Procedure.
//
// MESSAGE:     Handles the following Windows messages:
//
//                WM_CREATE
//                WM_DESTROY
//                WM_SYSCOMMAND
//                WM_LBUTTONUP	   
//                WM_RBUTTONUP     
//                WM_MBUTTONUP     
//                WM_LBUTTONDOWN
//                WM_RBUTTONDOWN   
//                WM_MBUTTONDOWN
//                WM_LBUTTONDBLCLK 
//                WM_MBUTTONDBLCLK 
//                WM_RBUTTONDBLCLK 
//                WM_MOUSEMOVE	
//
//              And the new mouse message
//
//                WM_MOUSEWHEEL
//
//              And the new TrackMouseEvent() Messages
//
//                WM_MOUSEHOVER       // mouse hovered specified delay over 
//                                    // client area
//                WM_MOUSELEAVE       // mouse has left the client area
//
//
//////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MainWindowProc (
    HWND hwnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam )
{
    TEXTMETRIC tm = {0};
    HDC hDC = NULL;

    int MsgIndex = NO_MESSAGE;             // Index into garMsgStrings[] array
    TCHAR szBuff[128] = {0};               // String buffer  
    size_t bufferSize = ARRAYSIZE(szBuff); // Size of szBuff    
    static int LB = 0, RB = 0, MB = 0;     // State of the mouse buttons
    static int X = 0, Y = 0;               // Mouse position  
    static TCHAR Roller = TEXT(' ');       // State of the mouse wheel. '+' or '-'
    static int fHasWheel = FALSE;          // Mouse has a wheel?? TRUE if yes, 
                                           // FALSE if no
    switch (message)
    {
        case WM_CREATE:
            //
            // See if we have a mouse with a wheel attached to the system.
            // Note that SM_MOUSEWHEELPRESENT is a new flag for GetSystemMetrics
            //  and returns TRUE if a mouse with a wheel is present, FALSE if not.
            //
            fHasWheel = GetSystemMetrics (SM_MOUSEWHEELPRESENT);

            //
            // Size the window so that it will display all the text
            //
            hDC = GetDC (hwnd);
            GetTextMetrics (hDC,&tm);
            SetWindowPos (hwnd, HWND_BOTTOM, 0, 0, 
                          tm.tmMaxCharWidth * MAX_TITLE_LEN, 
                          100, SWP_SHOWWINDOW);
            ReleaseDC (hwnd, hDC);
            break;

        case WM_DESTROY:
            PostQuitMessage (0);
            break;

        //
        // Handle WM_MOUSE messages.
        // First we setup up some variables and then let
        //   control fall through to the end of the switch
        //   construct, where we then update the title bar.
        // 
        case WM_LBUTTONUP:      LB=0; MsgIndex = LBU; break;
        case WM_RBUTTONUP:      RB=0; MsgIndex = RBU; break;
        case WM_MBUTTONUP:      MB=0; MsgIndex = MBU; break;
        case WM_LBUTTONDOWN:    LB=1; MsgIndex = LBD; break;
        case WM_RBUTTONDOWN:    RB=1; MsgIndex = RBD; break;
        case WM_MBUTTONDOWN:    MB=1; MsgIndex = MBD; break; 
        case WM_LBUTTONDBLCLK:  LB=0; MsgIndex = LB2; break; 
        case WM_MBUTTONDBLCLK:  MB=0; MsgIndex = MB2; break; 
        case WM_RBUTTONDBLCLK:  RB=0; MsgIndex = RB2; break; 
        case WM_MOUSEHOVER:     MsgIndex = MH;  break;
        case WM_MOUSELEAVE:     MsgIndex = ML;  break;
        case WM_MOUSEMOVE:      MsgIndex = NO_MESSAGE; Roller = TEXT(' ');  break; 
        
        // Handle WM_MOUSEWHEEL message.
        // We handle this message a little differently since the
        // roller movement info is in the HIWORD of wParam.
        //
        // The MouseWheel has 18 'detents'. As the wheel is rolled
        // and a detent is encountered, the OS will send a WM_MOUSEWHEEL
        // message with the HIWORD of wParam set to a value of +/- 120.
        // '+' if the wheel is being rolled forward (away from the user),
        // '-' if the wheel is being rolled backwards (towards the user). 
        case WM_MOUSEWHEEL:     
            //
            // Mouse Wheel is being rolled forward
            //
            if ((short) HIWORD(wParam) > 0)   
            {
                Roller = TEXT('+');
            }

            //
            // Mouse Wheel is being rolled backward
            //
            if ((short)HIWORD(wParam) < 0)
            {
                Roller = TEXT('-');
            }
            MsgIndex = MW;      
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                //
                // Call the TrackMouseEvent() API, and set up a WM_MOUSEHOVER
                // "one-shot" event. Exactly one and only one  WM_MOUSEHOVER
                // message will be sent to the window specified in the hwndTrack
                // member of the TRACKMOUSEEVENT structure, when the mouse has
                // 'hovered' over the client area an amount of time equal to that
                // specified in the dwHoverTime member of TRACKMOUSEEVENT.
                //
                // NOTE that this message will be generated only once. The application
                // must call the TrackMouseEvent() API again in order for the system
                // to generate another WM_MOUSEHOVER message.
                //
                case IDM_HOVER:
                {
                    TRACKMOUSEEVENT tme = {0};

                    tme.cbSize      = sizeof(TRACKMOUSEEVENT);
                    tme.dwFlags     = TME_HOVER;
                    tme.hwndTrack   = hwnd;
                    tme.dwHoverTime = HOVER_DEFAULT;

                    TrackMouseEvent(&tme);
                    break;
                }

                //
                // Call the TrackMouseEvent() API, and set up a WM_MOUSELEAVE
                // "one-shot" event. Exactly one and only one  WM_MOUSELEAVE
                // message will be sent to the window specified in the hwndTrack
                // member of the TRACKMOUSEEVENT structure, when the mouse has
                // left the client area. 
                //
                // NOTE that this message will be generated only once. The application
                // must call the TrackMouseEvent() API again in order for the system
                // to generate another WM_MOUSELEAVE message. Also note that if the
                // mouse pointer is not over the application, a call to TrackMouseEvent() 
                // will result in the immediate posting of a WM_MOUSELEAVE
                // message.
                //
                case IDM_LEAVE:
                {
                    TRACKMOUSEEVENT tme = {0};

                    tme.cbSize      = sizeof(TRACKMOUSEEVENT);
                    tme.dwFlags     = TME_LEAVE;
                    tme.hwndTrack   = hwnd;

                    TrackMouseEvent(&tme);
                    break;
                }

                case IDM_ABOUT:
                {
                    TCHAR buff[128] = {0};                    
                    size_t buffSize = sizeof(buff)/sizeof(buff[0]);                    
                    
                    StringCchPrintf (buff, buffSize, 
                                TEXT("MousInfo.Exe\r\n")
                                TEXT("Copyright 1995 - 2000 Microsoft Corp.\r\n")
                                TEXT("All rights reserved\r\n\r\n") );

                    MessageBox (hwnd, buff, szAppName, MB_OK);
                    break;
                }
            }
            break;
            
        default:
            goto exit_func;
            break;
    }  // end switch (message)
  

    //
    // If the mouse has a wheel, display the state of the mouse buttons, 
    // wheel and position in the title bar. 
    // Else if mouse does not have a wheel, do not display the state
    // of the wheel (but do display everything else!)
    //    
    if (fHasWheel)
    {
        StringCchPrintf (szBuff, bufferSize,
                    TEXT("L:%d M:%d R:%d W:%c x:%04d y:%04d %s"),
                    LB, MB, RB, Roller,
                    LOWORD(lParam), HIWORD(lParam), 
                    garMsgStrings[MsgIndex] );
    }
    else
    {
        StringCchPrintf (szBuff, bufferSize,
                    TEXT("L:%d M:%d R:%d x:%04d y:%04d %s"),
                    LB, MB, RB, LOWORD(lParam), HIWORD(lParam), 
                    garMsgStrings[MsgIndex] );
    }

    SetWindowText(hwnd,szBuff);

exit_func:
    return  DefWindowProc (hwnd, message, wParam, lParam);
}



 
