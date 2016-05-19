/**********************************************************************/
/*                                                                    */
/*      MODE.C                                                        */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <imm.h>
#include "resource.h"
#include "imeapps.h"


LRESULT HandleModeCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HIMC hIMC = ImmGetContext(hWndCompStr);
    DWORD dwConv, dwSent;
    DWORD dwTemp;

    ImmGetConversionStatus(hIMC,&dwConv,&dwSent);

    switch(LOWORD(wParam))
    {
        case IDM_NATIVEMODE:
            dwTemp = dwConv & ~IME_CMODE_LANGUAGE;

            switch (dwConv & IME_CMODE_LANGUAGE)
            {
                case IME_CMODE_ALPHANUMERIC:
                    dwTemp |= IME_CMODE_NATIVE;
                    break;

                case IME_CMODE_NATIVE:
                    dwTemp |= (IME_CMODE_NATIVE | IME_CMODE_KATAKANA);
                    break;

                case (IME_CMODE_NATIVE | IME_CMODE_KATAKANA): /* fall-through */
                default:
                    break;
            }
            dwConv = dwTemp;
            break;

        case IDM_FULLHALF:
            if (dwConv & IME_CMODE_FULLSHAPE)
            {
                dwConv &= ~IME_CMODE_FULLSHAPE;
            }
            else
            {
                dwConv |= IME_CMODE_FULLSHAPE;
            }
            break;

        case IDM_ROMAN:
            if (dwConv & IME_CMODE_ROMAN)
            {
                dwConv &= ~IME_CMODE_ROMAN;
            }
            else
            {
                dwConv |= IME_CMODE_ROMAN;
            }
            break;

        case IDM_CHARCODE:
            if (dwConv & IME_CMODE_CHARCODE)
            {
                dwConv &= ~IME_CMODE_CHARCODE;
            }
            else
            {
                dwConv |= IME_CMODE_CHARCODE;
            }
            break;

        case IDM_HANJA:
            if (dwConv & IME_CMODE_HANJACONVERT)
            {
                dwConv &= ~IME_CMODE_HANJACONVERT;
            }
            else
            {
                dwConv |= IME_CMODE_HANJACONVERT;
            }
            break;

        case IDM_SOFTKBD:
            if (dwConv & IME_CMODE_SOFTKBD)
            {
                dwConv &= ~IME_CMODE_SOFTKBD;
            }
            else
            {
                dwConv |= IME_CMODE_SOFTKBD;
            }
            break;

        case IDM_EUDC:
            if (dwConv & IME_CMODE_EUDC)
            {
                dwConv &= ~IME_CMODE_EUDC;
            }
            else
            {
                dwConv |= IME_CMODE_EUDC;
            }
            break;

        case IDM_SYMBOL:
            if (dwConv & IME_CMODE_SYMBOL)
            {
                dwConv &= ~IME_CMODE_SYMBOL;
            }
            else
            {
                dwConv |= IME_CMODE_SYMBOL;
            }
            break;
    }

    ImmSetConversionStatus(hIMC,dwConv,dwSent);
    ImmReleaseContext(hWndCompStr,hIMC);

    return 1L;
}

LRESULT HandleConvertCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HIMC hIMC = ImmGetContext(hWndCompStr);
    DWORD dwSel;
    DWORD dwSize;
    LPCANDIDATELIST lpCL = NULL;

    switch(wParam)
    {
        case IDM_CONVERT:
            ImmNotifyIME(hIMC,NI_COMPOSITIONSTR,CPS_CONVERT,0);
            break;

        case IDM_CANCEL:
            ImmNotifyIME(hIMC,NI_COMPOSITIONSTR,CPS_CANCEL,0);
            break;

        case IDM_REVERT:
            ImmNotifyIME(hIMC,NI_COMPOSITIONSTR,CPS_REVERT,0);
            break;

        case IDM_COMPLETE:
            ImmNotifyIME(hIMC,NI_COMPOSITIONSTR,CPS_COMPLETE,0);
            break;

        case IDM_OPENCAND:
            ImmNotifyIME(hIMC,NI_OPENCANDIDATE,0,0);
            break;

        case IDM_CLOSECAND:
            ImmNotifyIME(hIMC,NI_CLOSECANDIDATE,0,0);
            break;

        case IDM_NEXTCAND: /* fall-through */
        case IDM_PREVCAND:
            if (dwSize = ImmGetCandidateList(hIMC,0x0,NULL,0))
            {
                lpCL = (LPCANDIDATELIST)GlobalAlloc(GPTR,dwSize);
        
                ImmGetCandidateList(hIMC,0x0,lpCL,dwSize);
 
                dwSel = lpCL->dwSelection;

                if (wParam == IDM_NEXTCAND)
                {
                   if (++dwSel >= lpCL->dwCount)
                   {
                      dwSel = 0;
                   }
                }
                else
                {
                   if (dwSel)
                   {
                      dwSel--;
                   }
                   else
                   {
                      dwSel = lpCL->dwCount - 1;
                   }
                }
                GlobalFree((HANDLE)lpCL);

                ImmNotifyIME(hIMC,NI_SELECTCANDIDATESTR,0,dwSel);
            }
            break;

    }

    ImmReleaseContext(hWndCompStr,hIMC);

    return 1;
}

