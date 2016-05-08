/**********************************************************************/
/*                                                                    */
/*      MULTIWND.C                                                    */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation              */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <Commdlg.h>
#include <imm.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"
#include "multiui.h"


/**********************************************************************/
/*                                                                    */
/*    InitStringBuffer (HWND)                                         */
/*                                                                    */
/**********************************************************************/
DWORD InitStringBuffer (
    HWND hWnd )
{
    DWORD error = ERROR_SUCCESS;
    PTSTR pStr = NULL;

    pStr = GlobalAlloc (GPTR, MAXSIZE_STR * sizeof(TCHAR));
    if (NULL == pStr)
    {
        error = GetLastError ();
    }
    
    if (0 == SetWindowLongPtr(hWnd, MYGWL_STR, (LONG_PTR)pStr))
    {
        error = GetLastError ();
    }

    return error;
}

/**********************************************************************/
/*                                                                    */
/*    DestroyStringBuffer (HWND)                                      */
/*                                                                    */
/**********************************************************************/
DWORD DestroyStringBuffer (
    HWND hWnd )
{
    DWORD error = ERROR_SUCCESS;
    PTSTR pStr = NULL;

    pStr = (PTSTR)GetWindowLongPtr(hWnd, MYGWL_STR);
    if (pStr)
    {
        GlobalFree((HANDLE)pStr);
    }
    else
    {
        error = GetLastError ();
    }

    return error;
}

/**********************************************************************/
/*                                                                    */
/*    MyDrawString (HWND, WPARAM, LPARAM)                             */
/*                                                                    */
/**********************************************************************/
DWORD MyDrawString (
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam)
{
    DWORD       error    = ERROR_SUCCESS;
    PTSTR       pStr     = NULL;
    HDC         hDC      = NULL;
    PAINTSTRUCT ps       = {0};
    HFONT       hOldFont = NULL;
    HFONT       hFont    = NULL;
    DWORD       dwHeight = 0;
    TEXTMETRIC  tm       = {0};
    RECT        rc       = {0};

    pStr = (PTSTR)GetWindowLongPtr(hWnd, MYGWL_STR);
    if (!pStr)
    {
        error = GetLastError ();
        goto exit_func;
    }

    hFont = (HFONT)GetWindowLongPtr(hWnd,MYGWL_FONT);
    if (!hFont)
    {
        error = GetLastError ();
        goto exit_func;
    }


    hDC = BeginPaint(hWnd, &ps);
    hOldFont = SelectObject(hDC, hFont);

    pStr = (PTSTR)GetWindowLongPtr(hWnd, MYGWL_STR);
    // Get dwHeight.
    GetTextMetrics(hDC,&tm);
    dwHeight = tm.tmHeight + tm.tmExternalLeading;


    GetClientRect(hWnd,&rc);
    rc.bottom -= dwHeight;

    DrawText(hDC, pStr, -1, &rc, DT_LEFT | DT_WORDBREAK);

    SelectObject(hDC, hOldFont);
    EndPaint(hWnd, &ps);

exit_func:
    return error;
}

/**********************************************************************/
/*                                                                    */
/*    MySetCompositionFont (HWND)                                     */
/*                                                                    */
/**********************************************************************/
void MySetCompositionFont (
    HWND hWnd)
{
    HIMC hIMC;

    if (hIMC = ImmGetContext(hWnd))
    {
        LOGFONT lf;
        HFONT   hFont = (HFONT)GetWindowLongPtr(hWnd, MYGWL_FONT);
        GetObject(hFont, sizeof(lf), &lf);

        ImmSetCompositionFont(hIMC, &lf);
        ImmReleaseContext(hWnd, hIMC);
    }
}

/**********************************************************************/
/*                                                                    */
/*    MySetCompositionForm (HWND)                                     */
/*                                                                    */
/**********************************************************************/
DWORD MySetCompositionForm (
    HWND hWnd)
{
    DWORD           error    = ERROR_SUCCESS;
    RECT            rc       = {0};
    HIMC            hIMC     = NULL;
    HDC             hDC      = NULL;
    TEXTMETRIC      tm       = {0};
    DWORD           dwHeight = 0;
    HFONT           hFont    = (HFONT)GetWindowLongPtr(hWnd,MYGWL_FONT);
    HFONT           hOldFont = NULL;
    COMPOSITIONFORM CompForm = {0};

    if (!hFont)
    {
        error = GetLastError ();
        goto exit_func;
    }

    if (hIMC = ImmGetContext(hWnd))
    {
        // Get dwHeight.
        hDC = GetDC(hWnd);
        hOldFont = SelectObject(hDC, hFont);
        GetTextMetrics(hDC,&tm);
        dwHeight = tm.tmHeight + tm.tmExternalLeading;
        SelectObject(hDC, hOldFont);
        ReleaseDC(hWnd,hDC);


        GetClientRect(hWnd,&rc);
        CompForm.dwStyle = CFS_POINT;
        CompForm.ptCurrentPos.x = rc.left;
        CompForm.ptCurrentPos.y = rc.bottom - dwHeight;
        ImmSetCompositionWindow(hIMC,&CompForm);

        ImmReleaseContext(hWnd, hIMC);
    }

exit_func:
    return error;        
}

/**********************************************************************/
/*                                                                    */
/*    MyChangeFont (HWND)                                             */
/*                                                                    */
/**********************************************************************/
void MyChangeFont (
    HWND hWnd)
{
    LOGFONT    lf    = {0};
    CHOOSEFONT cf    = {0};
    HFONT      hFont = NULL;

    /* Set all structure fields to zero. */
    memset(&cf, 0, sizeof(CHOOSEFONT));

    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = hWnd;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS;
    cf.rgbColors = RGB(0, 255, 255); /* light blue */
    cf.nFontType = SCREEN_FONTTYPE;

    if (ChooseFont(&cf))
    {
        hFont = (HFONT)GetWindowLongPtr(hWnd,MYGWL_FONT);
        DeleteObject(hFont);

        hFont = CreateFontIndirect(&lf);
        SetWindowLongPtr(hWnd,MYGWL_FONT,(LONG_PTR)hFont);
        MySetCompositionForm(hWnd);
        MySetCompositionFont(hWnd);
    }
    SetFocus(hWnd);
}

/**********************************************************************/
/*                                                                    */
/*    GetDefaultGUIFont (VOID)                                        */
/*                                                                    */
/**********************************************************************/
HFONT GetDefaultGUIFont (
    VOID)
{
    static HFONT hFont;
    OSVERSIONINFO VerInfo = {0};

    if (hFont)
    {
        return hFont;
    }

    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VerInfo);

    if (VerInfo.dwMajorVersion < 4) 
    {
        NONCLIENTMETRICS ncm = {0};

        ncm.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        hFont = CreateFontIndirect(&ncm.lfStatusFont);

        if (!hFont)
        {
            hFont = GetStockObject(SYSTEM_FONT);
        }
    } 
    else 
    {
        hFont = GetStockObject(DEFAULT_GUI_FONT);
    }

    return hFont;
}

/**********************************************************************/
/*                                                                    */
/*    OverFlowText (HWND, PTSTR, PTSTR)                               */
/*                                                                    */
/**********************************************************************/
BOOL OverFlowText (
    HWND hWnd, 
    PTSTR pStr, 
    PTSTR pResultStr)
{
    BOOL        bRet     = FALSE;
    RECT        rc       = {0};
    HDC         hDC      = NULL;
    TEXTMETRIC  tm       = {0};
    int         nHeight  = 0;
    HFONT       hFont    = (HFONT)GetWindowLongPtr(hWnd,MYGWL_FONT);
    HFONT       hOldFont = NULL;
    PTSTR       pTempStr = NULL;

    if (!hFont)
    {
        return FALSE;
    }

    if (_tcsclen(pStr) + _tcsclen(pResultStr) > MAXSIZE_STR)
    {
        bRet = TRUE;
    }
    else
    {
        pTempStr = GlobalAlloc(GPTR, MAXSIZE_STR * sizeof(TCHAR));

        if (pTempStr)
        {
            StringCchCopy(pTempStr, MAXSIZE_STR - 1,  pStr);
            StringCchCat(pTempStr, min(MAXSIZE_STR - _tcslen(pStr) - 1, _tcslen(pResultStr)), pResultStr);

            GetClientRect(hWnd,&rc);

            hDC = GetDC(hWnd);
            hOldFont = SelectObject(hDC, hFont);
            GetTextMetrics(hDC,&tm);

            rc.bottom -= tm.tmHeight + tm.tmExternalLeading;
            nHeight = rc.bottom;

            if (nHeight < DrawText(hDC, pTempStr, -1, &rc, DT_LEFT | DT_WORDBREAK | DT_CALCRECT))
            {
                bRet = TRUE;
            }

            SelectObject(hDC, hOldFont);
            ReleaseDC(hWnd,hDC);

            GlobalFree((HANDLE)pTempStr);
        }
    }


    return bRet;
}

/**********************************************************************/
/*                                                                    */
/*    AddResultString (HWND)                                          */
/*                                                                    */
/**********************************************************************/
void AddResultString(
    HWND hWnd)
{
    HIMC  hIMC       = NULL;
    DWORD dwLen      = 0;
    PTSTR pStr       = NULL;
    PTSTR pResultStr = NULL;

    pStr = (PTSTR)GetWindowLongPtr(hWnd, MYGWL_STR);

    hIMC = ImmGetContext(hWnd);

    if (!hIMC)
    {
        return;
    }

    dwLen = ImmGetCompositionString(hIMC,GCS_RESULTSTR,NULL,0L);

    if (dwLen)
    {
        pResultStr = GlobalAlloc(GPTR, dwLen + 1);

        if (pResultStr)
        {
            ImmGetCompositionString(hIMC, GCS_RESULTSTR,
                                    pResultStr,dwLen+1);

            if (OverFlowText(hWnd, pStr, pResultStr))
            {
                StringCchCopy(pStr, MAXSIZE_STR - 1, pResultStr);
            }
            else
            {
                StringCchCat(pStr, min(MAXSIZE_STR - _tcslen(pStr) - 1, _tcslen(pResultStr)), pResultStr);
            }

            InvalidateRect(hWnd,NULL,TRUE);

            GlobalFree((HANDLE)pResultStr);
        }
    }

    ImmReleaseContext(hWnd,hIMC);

}

/**********************************************************************/
/*                                                                    */
/*    NoUINoIMCWndProc (HWND, UINT, WPARAM, LPARAM)                   */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK  NoUINoIMCWndProc (
    HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
{
    HFONT hFont = NULL;
    BOOL bRetDWP = FALSE;

    switch (message) 
    {
        case WM_CREATE:
            hFont = GetDefaultGUIFont();
            SetWindowLongPtr(hWnd,MYGWL_FONT,(LONG_PTR)hFont);
            InitStringBuffer(hWnd);
            break;

        case WM_LBUTTONDBLCLK:
            MyChangeFont(hWnd);
            break;

        case WM_LBUTTONDOWN:
            SetFocus(hWnd);
            break;

        case WM_SETFOCUS:
            MySetCompositionFont(hWnd);
            MySetCompositionForm(hWnd);
            break;

        case WM_IME_COMPOSITION:
            if (lParam & GCS_RESULTSTR)
            {
                AddResultString(hWnd);
            }
            bRetDWP = TRUE;
            break;

        case WM_SIZE:
            switch (wParam)
            {
                case SIZENORMAL: /* fall-through */
                case SIZEFULLSCREEN:
                    if (hWnd == GetFocus())
                    {
                        MySetCompositionForm(hWnd);
                    }
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;


                case SIZEICONIC:
                    bRetDWP = TRUE;
                    break;
            }
            break;


        case WM_PAINT:
            MyDrawString(hWnd,wParam,lParam);
            break;

        case WM_DESTROY:
            hFont = (HFONT)GetWindowLongPtr(hWnd,MYGWL_FONT);
            DeleteObject(hFont);
            DestroyStringBuffer(hWnd);
            break;

        default:
            bRetDWP = TRUE;
            break;
    }

    if (bRetDWP)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    else
    {
        return 0;
    }
}

/**********************************************************************/
/*                                                                    */
/*    NoUIOwnIMCWndProc (HWND, UINT, WPARAM, LPARAM)                  */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK NoUIOwnIMCWndProc (
    HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
{
    HIMC  hIMC    = NULL;
    HFONT hFont   = NULL;
    BOOL  bRetDWP = FALSE;

    switch (message) 
    {
        case WM_CREATE:
            hFont = GetDefaultGUIFont();
            SetWindowLongPtr(hWnd,MYGWL_FONT,(LONG_PTR)hFont);

            hIMC = ImmCreateContext();
            ImmAssociateContext(hWnd,hIMC);

            // Save hIMC into WndExtra.
            SetWindowLongPtr(hWnd,MYGWL_IMC,(LONG_PTR)hIMC);

            MySetCompositionFont(hWnd);
            MySetCompositionForm(hWnd);

            InitStringBuffer(hWnd);
            break;

        case WM_LBUTTONDBLCLK:
            MyChangeFont(hWnd);
            break;

        case WM_LBUTTONDOWN:
            SetFocus(hWnd);
            break;

        case WM_IME_COMPOSITION:
            if (lParam & GCS_RESULTSTR)
            {
                AddResultString(hWnd);
            }
            bRetDWP = TRUE;
            break;

        case WM_SIZE:
            switch (wParam)
            {
                case SIZENORMAL: /* fall-through */
                case SIZEFULLSCREEN:
                    MySetCompositionForm(hWnd);
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                case SIZEICONIC:
                    bRetDWP = TRUE;
                    break;
            }
            break;

        case WM_PAINT:
            MyDrawString(hWnd,wParam,lParam);
            break;

        case WM_DESTROY:
            hFont = (HFONT)GetWindowLongPtr(hWnd,MYGWL_FONT);
            DeleteObject(hFont);

            hIMC = (HIMC)GetWindowLongPtr(hWnd,MYGWL_IMC);
            ImmDestroyContext(hIMC);

            DestroyStringBuffer(hWnd);
            break;

        default:
            bRetDWP = TRUE;
            break;
    }


    if (bRetDWP)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    else
    {
        return 0;
    }
}

/**********************************************************************/
/*                                                                    */
/*    OwnUIOwnIMCWndProc (HWND, UINT, WPARAM, LPARAM)                 */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK OwnUIOwnIMCWndProc (
    HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
{
    HFONT hFont   = NULL;
    HIMC  hIMC    = 0L;
    HWND  hIMEWnd = (HWND)GetWindowLongPtr(hWnd,MYGWL_IMEWND);
    BOOL  bRetDWP = FALSE;



    if (IsWindow(hIMEWnd) && ImmIsUIMessage(hIMEWnd,message,wParam,lParam))
    {
        switch (message) 
        {
            case WM_IME_COMPOSITION:
                if (lParam & GCS_RESULTSTR)
                {
                    AddResultString(hWnd);
                }
                break;

            default:
                break;
        }

        goto exit_func;
    }

    switch (message) 
    {
        case WM_CREATE:
            hFont = GetDefaultGUIFont();
            SetWindowLongPtr(hWnd,MYGWL_FONT,(LONG_PTR)hFont);

            hIMC = ImmCreateContext();
            ImmAssociateContext(hWnd,hIMC);

            // Save hIMC into WndExtra.
            SetWindowLongPtr(hWnd,MYGWL_IMC,(LONG_PTR)hIMC);

            if (!(hIMEWnd = CreateWindow(
                            TEXT("Ime"), TEXT(""),
                            WS_POPUP | WS_DISABLED,
                            0,0,0,0,
                            hWnd, NULL, hInst, NULL)))
            {
	            return -1;
            }

            // Save hIMEWnd into WndExtra.
            SetWindowLongPtr(hWnd,MYGWL_IMEWND,(LONG_PTR)hIMEWnd);

            MySetCompositionFont(hWnd);
            MySetCompositionForm(hWnd);

            InitStringBuffer(hWnd);
            break;

        case WM_LBUTTONDBLCLK:
            MyChangeFont(hWnd);
            break;

        case WM_LBUTTONDOWN:
            SetFocus(hWnd);
            break;

        case WM_SIZE:
            switch (wParam)
            {
                case SIZENORMAL: /* fall-through */
                case SIZEFULLSCREEN:
                    MySetCompositionForm(hWnd);
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                case SIZEICONIC:
                    bRetDWP = TRUE;
                    break;
            }
            break;

        case WM_PAINT:
            MyDrawString(hWnd,wParam,lParam);
            break;

        case WM_DESTROY:
            hFont = (HFONT)GetWindowLongPtr(hWnd,MYGWL_FONT);
            DeleteObject(hFont);

            hIMC = (HIMC)GetWindowLongPtr(hWnd,MYGWL_IMC);
            ImmDestroyContext(hIMC);

            DestroyStringBuffer(hWnd);
            break;

        default:
            bRetDWP = TRUE;
            break;
    }

exit_func:
    if (bRetDWP)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    else
    {
        return 0;
    }
}
