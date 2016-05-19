/**********************************************************************/
/*                                                                    */
/*      SUBS.C                                                        */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include "resource.h"
#include "imeapps.h"

void MoveCompCandWindow(HWND hWnd)
{
    RECT rc = {0};
    RECT rcClient = {0};
    int cx;

    GetClientRect(hWnd, &rcClient);

    GetWindowRect(hWndToolBar,&rc);
    ScreenToClient(hWnd,(LPPOINT)&rc.right);
    rcClient.top = rc.bottom;

    GetWindowRect(hWndStatus,&rc);
    ScreenToClient(hWnd,(LPPOINT)&rc.left);
    rcClient.bottom = rc.top;

    if (fShowCand)
    {
        if (rcClient.right >= CAND_CX * 3)
        {
            cx = rcClient.right-rcClient.left - CAND_CX;
        }
        else
        {
            cx = (rcClient.right-rcClient.left) * 2 / 3;
        }

        MoveWindow(hWndCompStr,
                   rcClient.left,
                   rcClient.top,
                   cx,
                   rcClient.bottom-rcClient.top,
                   TRUE);
        MoveWindow(hWndCandList,
                   cx,
                   rcClient.top,
                   rcClient.right-rcClient.left - cx,
                   rcClient.bottom-rcClient.top,
                   TRUE);
    }
    else
    {
        MoveWindow(hWndCompStr,
                   rcClient.left,
                   rcClient.top,
                   rcClient.right-rcClient.left,
                   rcClient.bottom-rcClient.top,
                   TRUE);
        MoveWindow(hWndCandList, 0, 0, 0, 0, TRUE);
    }

}

