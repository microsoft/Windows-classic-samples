/**********************************************************************/
/*                                                                    */
/*      TOOLBAR.C                                                     */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "imeapps.h"

// Global Variables for toolbar control.


#define NUMIMAGES       33
#define IMAGEWIDTH      16
#define IMAGEHEIGHT     17
#define BUTTONWIDTH     0
#define BUTTONHEIGHT    0

#define IDTBB_FONT       0
#define IDTBB_OPEN       1
#define IDTBB_CLOSE      2
#define IDTBB_MODE       3
#define IDTBB_SHOWCAND   4
#define IDTBB_NOSHOWCAND 5
#define IDTBB_CONVERT    6
#define IDTBB_CANCEL     7
#define IDTBB_REVERT     8
#define IDTBB_COMPLETE   9
#define IDTBB_NEXTCAND   10
#define IDTBB_PREVCAND   11
#define IDTBB_NEXTCLAUSE 12
#define IDTBB_PREVCLAUSE 13
#define IDTBB_OPENCAND   14
#define IDTBB_CLOSECAND  15
#define IDTBB_ALPHA      16
#define IDTBB_NATIVE     17
#define IDTBB_KATAKANA   18
#define IDTBB_FULL       19
#define IDTBB_HALF       20
#define IDTBB_ROMAN      21
#define IDTBB_NOROMAN    22
#define IDTBB_CHARCODE   23
#define IDTBB_NOCHARCODE 24
#define IDTBB_HANJA      25
#define IDTBB_NOHANJA    26
#define IDTBB_SOFTKBD    27
#define IDTBB_NOSOFTKBD  28
#define IDTBB_EUDC       29
#define IDTBB_NOEUDC     30
#define IDTBB_SYMBOL     31
#define IDTBB_NOSYMBOL   32

TBBUTTON tbButton[] =           // Array defining the toolbar buttons
{
    {IDTBB_FONT, IDM_FONT,          TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},

    {0, 0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0},

    {IDTBB_OPEN, IDM_OPENSTATUS,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},

    {0, 0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0},

    {IDTBB_NATIVE,   IDM_NATIVEMODE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_FULL,     IDM_FULLHALF,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_ROMAN,    IDM_ROMAN,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_CHARCODE, IDM_CHARCODE,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_HANJA,    IDM_HANJA,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_SOFTKBD,  IDM_SOFTKBD,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_EUDC,     IDM_EUDC,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_SYMBOL,   IDM_SYMBOL,     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},

    {0, 0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0},

    {IDTBB_CONVERT,  IDM_CONVERT,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_CANCEL,   IDM_CANCEL,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_REVERT,   IDM_REVERT,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_COMPLETE, IDM_COMPLETE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},

    {0, 0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0},

    {IDTBB_NEXTCLAUSE, IDM_NEXTCLAUSE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_PREVCLAUSE, IDM_PREVCLAUSE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},

    {0, 0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0},

    {IDTBB_SHOWCAND, IDM_SHOWCAND,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_OPENCAND, IDM_OPENCAND,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_CLOSECAND,IDM_CLOSECAND, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_NEXTCAND, IDM_NEXTCAND,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {IDTBB_PREVCAND, IDM_PREVCAND,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},


};


BOOL CreateTBar(HWND hWnd)
{
    HIMC hIMC = NULL;
    BOOL fOpen;

    hWndToolBar = CreateToolbarEx(hWnd,
                                  WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,
                                  TOOLBAR_ID,
                                  NUMIMAGES,
                                  hInst,
                                  IDB_BMP,
                                  tbButton,
                                  sizeof(tbButton)/sizeof(TBBUTTON),
                                  BUTTONWIDTH,
                                  BUTTONHEIGHT,
                                  IMAGEWIDTH,
                                  IMAGEHEIGHT,
                                  sizeof(TBBUTTON));

    UpdateShowCandButton();

    hIMC = ImmGetContext(hWndCompStr);
    fOpen = ImmGetOpenStatus(hIMC);
    UpdateShowOpenStatusButton(fOpen);
    ImmReleaseContext(hWndCompStr,hIMC);

    return (hWndToolBar != 0);
}


LRESULT SetTooltipText(HWND hWnd, LPARAM lParam)
{
    LPTOOLTIPTEXT lpToolTipText = NULL;
    static TCHAR   szBuffer[64];

    lpToolTipText = (LPTOOLTIPTEXT)lParam;
    if (lpToolTipText->hdr.code == TTN_NEEDTEXT)
    {
        LoadString(hInst,
                   (UINT)lpToolTipText->hdr.idFrom,   // string ID == command ID
                   szBuffer,
                   (int)sizeof(szBuffer));

        lpToolTipText->lpszText = szBuffer;
    }
    return 0;
}

void UpdateButton(UINT iID, UINT iFlags)
{
    int iCurrentFlags;

    iCurrentFlags = (int)SendMessage(hWndToolBar, 
                                     TB_GETSTATE, 
                                     iID, 0L);

    if (iCurrentFlags & TBSTATE_PRESSED)
    {
        iFlags |= TBSTATE_PRESSED;
    }

    SendMessage(hWndToolBar, 
                TB_SETSTATE, 
                iID, 
                MAKELPARAM(iFlags, 0));
    return;
}

void UpdateShowOpenStatusButton(BOOL fOpen)
{
 
    SendMessage(hWndToolBar, 
                TB_CHANGEBITMAP, 
                IDM_OPENSTATUS, 
                MAKELPARAM((fOpen ? IDTBB_OPEN : IDTBB_CLOSE), 0));
    return;
}

void UpdateShowCandButton()
{
 
    SendMessage(hWndToolBar, 
                TB_CHANGEBITMAP, 
                IDM_SHOWCAND, 
                MAKELPARAM((fShowCand ? IDTBB_SHOWCAND : IDTBB_NOSHOWCAND), 0));
    return;
}


void UpdateModeButton(DWORD dwConv)
{
    int nIDTBB;

    switch (dwConv & IME_CMODE_LANGUAGE)
    {
        case IME_CMODE_ALPHANUMERIC:
            nIDTBB = IDTBB_ALPHA;
            break;

        case IME_CMODE_NATIVE:
            nIDTBB = IDTBB_NATIVE;
            break;

        case (IME_CMODE_NATIVE | IME_CMODE_KATAKANA):
            nIDTBB = IDTBB_KATAKANA;
            break;

        default:
            nIDTBB = IDTBB_ALPHA;
            break;

    }
    SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_NATIVEMODE, 
                MAKELPARAM(nIDTBB, 0));

    if (dwConv & IME_CMODE_FULLSHAPE)
    {
        nIDTBB = IDTBB_FULL;
    }
    else
    {
        nIDTBB = IDTBB_HALF;
        SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_FULLHALF, 
                    MAKELPARAM(nIDTBB, 0));
    }


    if (dwConv & IME_CMODE_ROMAN)
    {
        nIDTBB = IDTBB_ROMAN;
    }
    else
    {
        nIDTBB = IDTBB_NOROMAN;
        SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_ROMAN, 
                    MAKELPARAM(nIDTBB, 0));
    }


    if (dwConv & IME_CMODE_CHARCODE)
    {
        nIDTBB = IDTBB_CHARCODE;
    }
    else
    {
        nIDTBB = IDTBB_NOCHARCODE;
        SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_CHARCODE, 
                    MAKELPARAM(nIDTBB, 0));
    }


    if (dwConv & IME_CMODE_HANJACONVERT)
    {
        nIDTBB = IDTBB_HANJA;
    }
    else
    {
        nIDTBB = IDTBB_NOHANJA;
        SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_HANJA, 
                    MAKELPARAM(nIDTBB, 0));
    }


    if (dwConv & IME_CMODE_SOFTKBD)
    {
        nIDTBB = IDTBB_SOFTKBD;
    }
    else
    {
        nIDTBB = IDTBB_NOSOFTKBD;
        SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_SOFTKBD, 
                    MAKELPARAM(nIDTBB, 0));
    }


    if (dwConv & IME_CMODE_EUDC)
    {
        nIDTBB = IDTBB_EUDC;
    }
    else
    {
        nIDTBB = IDTBB_NOEUDC;
        SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_EUDC, 
                    MAKELPARAM(nIDTBB, 0));
    }


    if (dwConv & IME_CMODE_SYMBOL)
    {
        nIDTBB = IDTBB_SYMBOL;
    }
    else
    {
        nIDTBB = IDTBB_NOSYMBOL;
        SendMessage(hWndToolBar, TB_CHANGEBITMAP, IDM_SYMBOL, 
                    MAKELPARAM(nIDTBB, 0));
    }

    return;
}

