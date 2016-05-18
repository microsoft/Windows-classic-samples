/**********************************************************************/
/*                                                                    */
/*      MULTIUI.C                                                     */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                     */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <imm.h>
#include <strsafe.h>
#include <stdio.h>
#include "resource.h"
#include "multiui.h"


/**********************************************************************/
/*                                                                    */
/*    WinMain (HANDLE, HANDLE, LPSTR, int)                            */
/*                                                                    */
/**********************************************************************/
int APIENTRY WinMain (
    HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, 
    INT nCmdShow)
{
    MSG msg = {0};
    BOOL bSuccess = TRUE;
    BOOL bRet = FALSE;

    if (!InitApplication(hInstance))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (!InitInstance(hInstance, nCmdShow))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    
    while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
        if (bRet == -1)
        {
            // handle the error and possibly exit
            bSuccess = FALSE;
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }


exit_func:
    if (bSuccess)
    {
        return (int)(msg.wParam);
    }
    else
    {
        return 0;
    }
}

/**********************************************************************/
/*                                                                    */
/*    InitApplication (HANDLE)                                        */
/*                                                                    */
/**********************************************************************/
BOOL InitApplication (
    HANDLE hInstance)
{
    WNDCLASS wc = {0};
    BOOL bSuccess = TRUE;

    wc.lpfnWndProc   = (WNDPROC)MainWndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance,TEXT("MyIcon"));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
    wc.lpszMenuName  = TEXT("MultiUiMenu");
    wc.lpszClassName = TEXT("MultiUiWClass");

    if (! RegisterClass (&wc))
    {
        bSuccess = FALSE;
        goto exit_func;
    }


    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC)NoUINoIMCWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = WNDEXTRA_NOUINOIMC;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT("NoUINoIMCWClass");

    if (! RegisterClass (&wc))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC)NoUIOwnIMCWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = WNDEXTRA_NOUIOWNIMC;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT("NoUIOwnIMCWClass");

    if (! RegisterClass (&wc))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC)OwnUIOwnIMCWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = WNDEXTRA_OWNUIOWNIMC;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT("OwnUIOwnIMCWClass");

    if (! RegisterClass (&wc))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

exit_func:
    return bSuccess;
}

/**********************************************************************/
/*                                                                    */
/*    InitInstance (HANDLE, int)                                      */
/*                                                                    */
/**********************************************************************/
BOOL InitInstance (
    HANDLE hInstance, 
    int nCmdShow)
{
    RECT       rc          = {0};
    int        iDesc       = 0;
    HDC        hIC         = NULL;
    TEXTMETRIC tm          = {0};
    HFONT      hFont       = GetDefaultGUIFont();
    BOOL       bSuccess    = TRUE;
    TCHAR      szTitle[20] =  TEXT("MultiUi TestTool");


    hInst = hInstance;

    hWndMain = CreateWindow(TEXT("MultiUiWClass"), szTitle,
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                            NULL, NULL, hInstance, NULL);
    if (!hWndMain)
    {
        bSuccess = FALSE;
        goto exit_func;
    }


    GetClientRect(hWndMain,&rc);

    rc.right  /= 2;
    rc.bottom /= 3;

    hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);

    SelectObject(hIC, hFont);

    GetTextMetrics(hIC,&tm);

    if (rc.bottom >= tm.tmHeight)
    {
        iDesc = tm.tmHeight + tm.tmExternalLeading;
    }

    DeleteDC(hIC);

    if (!(hWndDef1 = CreateWindowEx(
                        WS_EX_CLIENTEDGE, TEXT("NoUINoIMCWClass"), TEXT(""),
                        WS_CHILD | WS_VISIBLE,
                        rc.left, rc.top + iDesc, rc.right, rc.bottom - iDesc,
                        hWndMain, NULL, hInstance, NULL)))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (!(hWndDef2 = CreateWindowEx(
                        WS_EX_CLIENTEDGE, TEXT("NoUINoIMCWClass"), TEXT(""),
                        WS_CHILD | WS_VISIBLE,
                        rc.right, rc.top + iDesc, rc.right, rc.bottom - iDesc,
                        hWndMain, NULL, hInstance, NULL)))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (!(hWndIMC1 = CreateWindowEx(
                        WS_EX_CLIENTEDGE, TEXT("NoUIOwnIMCWClass"), TEXT(""),
                        WS_CHILD | WS_VISIBLE,
                        rc.left, rc.bottom + iDesc, rc.right, rc.bottom - iDesc,
                        hWndMain, NULL, hInstance, NULL)))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (!(hWndIMC2 = CreateWindowEx(
                        WS_EX_CLIENTEDGE, TEXT("NoUIOwnIMCWClass"), TEXT(""),
                        WS_CHILD | WS_VISIBLE,
                        rc.right, rc.bottom + iDesc, rc.right, rc.bottom - iDesc,
                        hWndMain, NULL, hInstance, NULL)))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (!(hWndIMCUI1 = CreateWindowEx(
                        WS_EX_CLIENTEDGE, TEXT("OwnUIOwnIMCWClass"), TEXT(""),
                        WS_CHILD | WS_VISIBLE,
                        rc.left, rc.bottom * 2 + iDesc, rc.right, rc.bottom - iDesc,
                        hWndMain, NULL, hInstance, NULL)))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (!(hWndIMCUI2 = CreateWindowEx(
                        WS_EX_CLIENTEDGE, TEXT("OwnUIOwnIMCWClass"), TEXT(""),
                        WS_CHILD | WS_VISIBLE,
                        rc.right, rc.bottom * 2 + iDesc, rc.right, rc.bottom - iDesc,
                        hWndMain, NULL, hInstance, NULL)))
    {
        bSuccess = FALSE;
        goto exit_func;
    }


    /* display each windows */
    ShowWindow (hWndMain, nCmdShow);
    UpdateWindow (hWndMain);

exit_func:
    return bSuccess;
}


/**********************************************************************/
/*                                                                    */
/*    MainWndProc (HWND, UINT, WPARAM, LPARAM)                        */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK MainWndProc (
    HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
{
    PAINTSTRUCT ps          = {0};
    HDC         hDC         = NULL;
    RECT        rc          = {0};
    int         iDesc       = 0;
    HDC         hIC         = NULL;
    TEXTMETRIC  tm          = {0};
    HFONT       hFont       = NULL;
    HFONT       hOldFont    = NULL;
    HIMC        hIMC        = NULL;
    HWND        hIMEWnd     = NULL;
    TCHAR       szDesc[128] = {0};
    BOOL        bRetDWP     = FALSE;

    switch (message) 
    {
        case WM_CREATE:
            break;

        case WM_SIZE:
            switch (wParam)
            {
                case SIZENORMAL: /* fall-through */
                case SIZEFULLSCREEN:
                    GetClientRect(hWndMain,&rc);
                    rc.right  /= 2;
                    rc.bottom /= 3;

                    hFont = GetDefaultGUIFont();
                    hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);

                    SelectObject(hIC, hFont);

                    GetTextMetrics(hIC,&tm);

                    iDesc = 0;
                    if (rc.bottom >= tm.tmHeight)
                    {
                        iDesc = tm.tmHeight + tm.tmExternalLeading;
                    }

                    DeleteDC(hIC);

                    MoveWindow(hWndDef1,
                               rc.left,rc.top + iDesc,
                               rc.right,rc.bottom - iDesc,TRUE);
                    MoveWindow(hWndDef2,
                               rc.right,rc.top + iDesc,
                               rc.right,rc.bottom - iDesc,TRUE);
                    MoveWindow(hWndIMC1,
                               rc.left,rc.bottom + iDesc,
                               rc.right,rc.bottom - iDesc,TRUE);
                    MoveWindow(hWndIMC2,
                               rc.right,rc.bottom + iDesc,
                               rc.right,rc.bottom - iDesc,TRUE);
                    MoveWindow(hWndIMCUI1,
                               rc.left,rc.bottom * 2 + iDesc,
                               rc.right,rc.bottom - iDesc,TRUE);
                    MoveWindow(hWndIMCUI2,
                               rc.right,rc.bottom * 2 + iDesc,
                               rc.right,rc.bottom - iDesc,TRUE);

                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                case SIZEICONIC:
                    bRetDWP = TRUE;
                    goto exit_func;
                    break;
           
            }
            break;

        case WM_PAINT:
            hDC = BeginPaint (hWnd, &ps);
            hFont = GetDefaultGUIFont();
            hOldFont = SelectObject(hDC, hFont);

            GetClientRect(hWndMain,&rc);
            rc.right  /= 2;
            rc.bottom /= 3;

            StringCchPrintf(szDesc, sizeof(szDesc)/sizeof(szDesc[0]) - 1, 
                            TEXT("%08lX Default IMC and Default IME window"),
                            (DWORD_PTR)hWndDef1);
            TextOut (hDC, rc.left,rc.top, szDesc, lstrlen(szDesc));

            StringCchPrintf (szDesc, sizeof(szDesc)/sizeof(szDesc[0]) - 1, 
                        TEXT("%08lX Default IMC and Default IME window"),
                        (DWORD_PTR)hWndDef2);
            TextOut (hDC, rc.right, rc.top, szDesc, lstrlen(szDesc));

            hIMC = (HIMC)GetWindowLongPtr(hWndIMC1,MYGWL_IMC);
            StringCchPrintf (szDesc, sizeof(szDesc)/sizeof(szDesc[0]) - 1,
                        TEXT("%08lX IMC[%08lX] and Default IME window"),
                        (DWORD_PTR)hWndIMC1, (DWORD_PTR)hIMC);
            TextOut (hDC, rc.left, rc.bottom, szDesc, lstrlen(szDesc));

            hIMC = (HIMC)GetWindowLongPtr(hWndIMC2,MYGWL_IMC);
            StringCchPrintf (szDesc, sizeof(szDesc)/sizeof(szDesc[0]) - 1,
                        TEXT("%08lX IMC[%08lX] and Default IME window"),
                        (DWORD_PTR)hWndIMC2, (DWORD_PTR)hIMC);
            TextOut (hDC, rc.right, rc.bottom, szDesc, lstrlen(szDesc));

            hIMC = (HIMC)GetWindowLongPtr(hWndIMCUI1,MYGWL_IMC);
            hIMEWnd = (HWND)GetWindowLongPtr(hWndIMCUI1,MYGWL_IMEWND);
            StringCchPrintf (szDesc, sizeof(szDesc)/sizeof(szDesc[0]) - 1,
                        TEXT("%08lX IMC[%08lX] and IME window[%08lX]"),
                        (DWORD_PTR)hWndIMCUI1, (DWORD_PTR)hIMC ,(DWORD_PTR)hIMEWnd);
            TextOut (hDC, rc.left, rc.bottom * 2, szDesc, lstrlen(szDesc));

            hIMC = (HIMC)GetWindowLongPtr(hWndIMCUI2,MYGWL_IMC);
            hIMEWnd = (HWND)GetWindowLongPtr(hWndIMCUI2,MYGWL_IMEWND);
            StringCchPrintf (szDesc, sizeof(szDesc)/sizeof(szDesc[0]) - 1,
                        TEXT("%08lX IMC[%08lX] and IME window[%08lX]"),
                        (DWORD_PTR)hWndIMCUI2, (DWORD_PTR)hIMC ,(DWORD_PTR)hIMEWnd);
            TextOut (hDC, rc.right, rc.bottom * 2, szDesc, lstrlen(szDesc));

            SelectObject(hDC, hOldFont);
            EndPaint (hWnd, &ps);
            break;

        case WM_COMMAND:
            switch(WMCOMMANDWPARAM(wParam))
            {
            	case IDM_ABOUT:
            	    DialogBox(hInst, TEXT("ABOUTBOX"), hWnd, (DLGPROC)AboutDlg);
            	    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            bRetDWP = TRUE;
            goto exit_func;
    }

exit_func: 
    if (bRetDWP)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    else
    {
        return 0L;
    }
}

