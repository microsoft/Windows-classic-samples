/**********************************************************************/
/*                                                                    */
/*      COMP.C                                                        */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <imm.h>
#include "resource.h"
#include "imeapps.h"

#define DEBUG 1

LRESULT HandleStartComposition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    COMPOSITIONFORM cpf = {0};
    HIMC hIMC = NULL;
 
#ifdef DEBUG
    OutputDebugString(TEXT("WM_STARTCOMPOSITIONSTRING!!!\r\n"));
#endif

    if (fdwProperty & IME_PROP_SPECIAL_UI)
    {
        // Normally, we need to set the composition window
        // position to caret position for a special UI IME
    }
    else if (fdwProperty & IME_PROP_AT_CARET)
    {
        // If an application show composition string by itself, we do not
        // need to set the position of composition window for an at caret
        // IME.

        return 1;
    }
    else
    {
        // Normally, we need to set the composition window
        // position to caret position for a near caret IME
    }

    hIMC = ImmGetContext(hWnd);

    if (!hIMC)
    {
        return 1;
    }

    cpf.dwStyle = CFS_POINT;
    cpf.ptCurrentPos.x = ptImeUIPos.x;
    cpf.ptCurrentPos.y = ptImeUIPos.y;

    ImmSetCompositionWindow(hIMC,&cpf);

    ImmReleaseContext(hWnd,hIMC);

    return 1;
}

LRESULT HandleEndComposition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
    OutputDebugString(TEXT("WM_ENDCOMPOSITIONSTRING!!!\r\n"));
#endif
    dwCompStrLen      = 0;
    dwCompAttrLen     = 0;
    dwCompClsLen      = 0;
    dwCompReadStrLen  = 0;
    dwCompReadAttrLen = 0;
    dwCompReadClsLen  = 0;
    dwResultStrLen      = 0;
    dwResultClsLen      = 0;
    dwResultReadStrLen  = 0;
    dwResultReadClsLen  = 0;
    InvalidateRect(hWnd,NULL,TRUE);

    return 1;
}

void MakePaintString(HWND hWnd, LPMYSTR lpStr, DWORD dwStrLen, LPDWORD lpCls, DWORD dwClsLen, LPMYSTR lpPaintStr, DWORD dwPaintStrSize)
{
    LPMYSTR lpPaintStart = lpPaintStr;

    lpPaintStr += Mylstrlen(lpPaintStr);

    if (dwStrLen)
    {
        if (dwClsLen)
        {
            lpCls[127] = 0;

            while (*(lpCls+1) && *lpCls < dwStrLen)
            {
                DWORD dwTextLen = *(lpCls+1) - *lpCls;
                LPMYSTR lpT = lpStr + *lpCls;

                memcpy(lpPaintStr,lpT,dwTextLen * sizeof(MYCHAR));
                lpPaintStr += dwTextLen;
                *lpPaintStr = MYTEXT(',');
                lpPaintStr++;
                lpCls++;
            }
            *lpPaintStr = MYTEXT('\0');
        }
        else
        {
            StringCchCopy((LPTSTR)lpPaintStr,dwPaintStrSize, (LPTSTR)lpStr);
            StringCchCat((LPTSTR)lpPaintStr,dwPaintStrSize, (LPTSTR)MYTEXT(","));
        }
    }

}

LRESULT HandleComposition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HIMC hIMC = NULL;
    BOOL fRedraw = FALSE;

    hIMC = ImmGetContext(hWnd);

    if (lParam & GCS_COMPSTR)
    {
        dwCompStrLen = MyImmGetCompositionString(hIMC,GCS_COMPSTR,szCompStr,sizeof(szCompStr));
        dwCompStrLen /= sizeof(MYCHAR);
        szCompStr[dwCompStrLen] = MYTEXT('\0');
        fRedraw = TRUE;
    }
    else
    {
        dwCompStrLen = 0;
        szCompStr[0] = MYTEXT('\0');
    }

    if (lParam & GCS_COMPATTR)
    {
        dwCompAttrLen = MyImmGetCompositionString(hIMC,GCS_COMPATTR,bCompAttr,sizeof(bCompAttr));
        fRedraw = TRUE;
    }
    else
    {
        dwCompAttrLen = 0;
        bCompAttr[0] = 0;
    }

    if (lParam & GCS_COMPCLAUSE)
    {
        dwCompClsLen = MyImmGetCompositionString(hIMC,GCS_COMPCLAUSE,dwCompCls,sizeof(dwCompCls));
        fRedraw = TRUE;
    }
    else
    {
        dwCompClsLen = 0;
        dwCompCls[0] = 0;
        dwCompCls[1] = 0;
    }


    if (lParam & GCS_COMPREADSTR)
    {
        dwCompReadStrLen = MyImmGetCompositionString(hIMC,GCS_COMPREADSTR,szCompReadStr,sizeof(szCompReadStr));
        dwCompReadStrLen /= sizeof(MYCHAR);
        szCompReadStr[dwCompReadStrLen] = MYTEXT('\0');
        fRedraw = TRUE;
    }
    else
    {
        dwCompReadStrLen = 0;
        szCompReadStr[0] = MYTEXT('\0');
    }

    if (lParam & GCS_COMPREADATTR)
    {
        dwCompReadAttrLen = MyImmGetCompositionString(hIMC,GCS_COMPREADATTR,bCompReadAttr,sizeof(bCompReadAttr));
        fRedraw = TRUE;
    }
    else
    {
        dwCompReadAttrLen = 0;
        bCompReadAttr[0] = 0;
    }

    if (lParam & GCS_COMPREADCLAUSE)
    {
        dwCompReadClsLen = MyImmGetCompositionString(hIMC,GCS_COMPREADCLAUSE,dwCompReadCls,sizeof(dwCompReadCls));
        fRedraw = TRUE;
    }
    else
    {
        dwCompReadClsLen = 0;
        dwCompReadCls[0] = 0;
        dwCompReadCls[1] = 0;
    }


    if (lParam & GCS_RESULTSTR)
    {
        RECT rc = {0};
        HDC hIC = NULL;
        SIZE sz0, sz1;
        HFONT hOldFont = NULL;

        if (lParam & GCS_RESULTCLAUSE)
        {
            dwResultClsLen = MyImmGetCompositionString(hIMC,GCS_RESULTCLAUSE,dwResultCls,sizeof(dwResultCls));
        }
        else
        {
            dwResultClsLen = 0;
            dwResultCls[0] = 0;
            dwResultCls[1] = 0;
        }

        dwResultStrLen = MyImmGetCompositionString(hIMC,GCS_RESULTSTR,szResultStr,sizeof(szResultStr));
        dwResultStrLen /= sizeof(MYCHAR);
        szResultStr[dwResultStrLen] = MYTEXT('\0');


        // szPaintResult may overflow..
        GetClientRect(hWnd,&rc);
        hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
        hOldFont = SelectObject(hIC,hFont);
#ifdef USEWAPI
        GetTextExtentPointW(hIC,szPaintResult,Mylstrlen(szPaintResult),&sz0);
        GetTextExtentPointW(hIC,szResultStr,Mylstrlen(szResultStr),&sz1);
#else
        GetTextExtentPoint(hIC,szPaintResult,Mylstrlen(szPaintResult),&sz0);
        GetTextExtentPoint(hIC,szResultStr,Mylstrlen(szResultStr),&sz1);
#endif
        if (sz0.cx + sz1.cx >= rc.right)
        {
            szPaintResult[0] = MYTEXT('\0');
            szPaintResultRead[0] = MYTEXT('\0');
        }
        SelectObject(hIC,hOldFont);
        DeleteDC(hIC);

        MakePaintString(hWnd,szResultStr,dwResultStrLen,dwResultCls,dwResultClsLen,szPaintResult, ARRAYSIZE(szPaintResult));

        fRedraw = TRUE;
    }
    else
    {
        dwResultStrLen = 0;
        szResultStr[0] = MYTEXT('\0');
        dwResultClsLen = 0;
        dwResultCls[0] = 0;
        dwResultCls[1] = 0;
    }



    if (lParam & GCS_RESULTREADSTR)
    {
        if (lParam & GCS_RESULTREADCLAUSE)
        {
            dwResultReadClsLen = MyImmGetCompositionString(hIMC,GCS_RESULTREADCLAUSE,dwResultReadCls,sizeof(dwResultReadCls));
            fRedraw = TRUE;
        }
        else
        {
            dwResultReadClsLen = 0;
            dwResultReadCls[0] = 0;
            dwResultReadCls[1] = 0;
        }
        dwResultReadStrLen = MyImmGetCompositionString(hIMC,GCS_RESULTREADSTR,szResultReadStr,sizeof(szResultReadStr));
        dwResultReadStrLen /= sizeof(MYCHAR);
        szResultReadStr[dwResultReadStrLen] = MYTEXT('\0');
        MakePaintString(hWnd,szResultReadStr,dwResultReadStrLen,dwResultReadCls,dwResultReadClsLen,szPaintResultRead, ARRAYSIZE(szPaintResultRead));
        fRedraw = TRUE;
    }
    else
    {
        dwResultReadStrLen = 0;
        szResultReadStr[0] = MYTEXT('\0');
        dwResultReadClsLen = 0;
        dwResultReadCls[0] = 0;
        dwResultReadCls[1] = 0;
    }



    if (fRedraw)
    {
        InvalidateRect(hWnd,NULL,TRUE);
        UpdateWindow(hWnd);
    }
    return 1;
}


LRESULT HandleChar(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    RECT rc = {0};
    HDC hIC = NULL;
    SIZE sz0, sz1;
    HFONT hOldFont = NULL;
    LPMYSTR lp = NULL;
    // is the previous received char is a DBCS lead byte char ?
    static BOOL fIsPrevLeadByte = FALSE;

    GetClientRect(hWnd,&rc);
    hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
    hOldFont = SelectObject(hIC,hFont);
#ifdef USEWAPI
    GetTextExtentPointW(hIC,szPaintResult,Mylstrlen(szPaintResult),&sz0);
    GetTextExtentPointW(hIC,szResultStr,Mylstrlen(szResultStr),&sz1);
#else
    GetTextExtentPoint(hIC,szPaintResult,Mylstrlen(szPaintResult),&sz0);
    GetTextExtentPoint(hIC,szResultStr,Mylstrlen(szResultStr),&sz1);
#endif
    if (sz0.cx + sz1.cx >= rc.right)
    {
        szPaintResult[0] = MYTEXT('\0');
        szPaintResultRead[0] = MYTEXT('\0');
    }
    SelectObject(hIC,hOldFont);
    DeleteDC(hIC);

    lp = szPaintResult + Mylstrlen(szPaintResult);

#ifndef USEWAPI
    if (fIsPrevLeadByte) 
    {
        // remove , and append second byte for showing DBCS char
        if (*(lp - 1) == ',') 
        {
            lp--;
        }
    }
#endif

    // append second byte
    *lp++ = (MYCHAR)(BYTE)wParam;
    *lp++ = MYTEXT(',');
    *lp++ = MYTEXT('\0');

    lp = szPaintResultRead + Mylstrlen(szPaintResultRead);

#ifndef USEWAPI
    if (fIsPrevLeadByte) 
    {
        // remove , and append second byte for showing DBCS char
        if (*(lp - 1) == ',') 
        {
            lp--;
        }

        fIsPrevLeadByte = FALSE;
    } 
    else 
    {
        fIsPrevLeadByte = IsDBCSLeadByte((BYTE)wParam);
    }
#endif

    *lp++ = (BYTE)wParam;
    *lp++ = MYTEXT(',');
    *lp++ = MYTEXT('\0');

    InvalidateRect(hWnd,NULL,TRUE);
    UpdateWindow(hWnd);

    return 1;
}


LRESULT HandleNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HIMC hIMC = NULL;
    BOOL fOpen = FALSE;
    DWORD dwConvMode, dwSentMode;


    switch (wParam)
    {
        case IMN_OPENSTATUSWINDOW: /* fall-through */
        case IMN_CLOSESTATUSWINDOW:
            break;

        case IMN_SETOPENSTATUS:
            SetStatusItems(hWnd);

            hIMC = ImmGetContext(hWnd);
            fOpen = ImmGetOpenStatus(hIMC);
            UpdateShowOpenStatusButton(fOpen);

            ImmReleaseContext(hWnd,hIMC);
            break;

        case IMN_SETCONVERSIONMODE:
            hIMC = ImmGetContext(hWnd);
            fOpen = ImmGetOpenStatus(hIMC);
            ImmGetConversionStatus(hIMC,&dwConvMode,&dwSentMode);
            if (fOpen)
            {
                SetConvModeParts(dwConvMode);
                UpdateModeButton(dwConvMode);
            }
            else
            {
                ClearConvModeParts();
            }
            ImmReleaseContext(hWnd,hIMC);
            break;

        case IMN_OPENCANDIDATE:
            if (!fShowCand || (lParam != 0x01))
            {
                if (fdwProperty & IME_PROP_SPECIAL_UI)
                {
                    // Normally, we only need to set the composition window
                    // position for a special UI IME
                }
                else if (fdwProperty & IME_PROP_AT_CARET)
                {
                    CANDIDATEFORM cdf;
                    HIMC          hIMC;
 
                    hIMC = ImmGetContext(hWnd);

                    cdf.dwIndex = 0;
                    cdf.dwStyle = CFS_CANDIDATEPOS;
                    cdf.ptCurrentPos.x = ptImeUIPos.x;
                    cdf.ptCurrentPos.y = ptImeUIPos.y;
                    ImmSetCandidateWindow(hIMC,&cdf);

                    ImmReleaseContext(hWnd,hIMC);
                }
                else
                {
                    // Normally, we only need to set the composition window
                    // position for a near caret IME
                }

                return (DefWindowProc(hWnd, message, wParam, lParam));
            }

        case IMN_CHANGECANDIDATE:

#ifdef _DEBUG
            {
                TCHAR szDev[80];
                DWORD dwSize;
                LPCANDIDATELIST lpC;

                hIMC = ImmGetContext(hWnd);
                if (dwSize = ImmGetCandidateList(hIMC,0x0,NULL,0))
                {
                    lpC = (LPCANDIDATELIST)GlobalAlloc(GPTR,dwSize);
                   
                    ImmGetCandidateList(hIMC,0x0,lpC,dwSize);

                    OutputDebugString(TEXT("DumpCandList!!!\r\n"));
                    StringCchPrintf((LPTSTR)szDev,ARRAYSIZE(szDev),TEXT("dwCount %d\r\n"),lpC->dwCount);
                    OutputDebugString((LPTSTR)szDev);
                    StringCchPrintf((LPTSTR)szDev,ARRAYSIZE(szDev),TEXT("dwSelection %d\r\n"),lpC->dwSelection);
                    OutputDebugString((LPTSTR)szDev);
                    StringCchPrintf((LPTSTR)szDev,ARRAYSIZE(szDev),TEXT("dwPageStart %d\r\n"),lpC->dwPageStart);
                    OutputDebugString((LPTSTR)szDev);
                    StringCchPrintf((LPTSTR)szDev,ARRAYSIZE(szDev),TEXT("dwPageSize %d\r\n"),lpC->dwPageSize);
                    OutputDebugString((LPTSTR)szDev);
                    GlobalFree((HANDLE)lpC);
                }
            }
#endif
            if (fShowCand && (lParam == 0x01))
            {
                DWORD dwSize;

                if (!lpCandList)
                    lpCandList = (LPCANDIDATELIST)GlobalAlloc(GPTR,sizeof(CANDIDATELIST));

                hIMC = ImmGetContext(hWnd);
                if (dwSize = ImmGetCandidateList(hIMC,0x0,NULL,0))
                {
                    GlobalFree((HANDLE)lpCandList);
                    lpCandList = (LPCANDIDATELIST)GlobalAlloc(GPTR,dwSize);
                   
                    ImmGetCandidateList(hIMC,0x0,lpCandList,dwSize);
                }
                else
                {
                    memset(lpCandList, 0, sizeof(CANDIDATELIST));
                }

                InvalidateRect(hWndCandList,NULL,TRUE);
                UpdateWindow(hWndCandList);
                    

                ImmReleaseContext(hWnd,hIMC);
            }
            else
            {
                return (DefWindowProc(hWnd, message, wParam, lParam));
            }
            break;

        case IMN_CLOSECANDIDATE:
            if (fShowCand && (lParam == 0x01))
            {
                if (!lpCandList)
                {
                    lpCandList = (LPCANDIDATELIST)GlobalAlloc(GPTR,sizeof(CANDIDATELIST));
                }

                memset(lpCandList, 0, sizeof(CANDIDATELIST));
                InvalidateRect(hWndCandList,NULL,TRUE);
                UpdateWindow(hWndCandList);
            }
            else
            {
                return (DefWindowProc(hWnd, message, wParam, lParam));
            }
            break;


       default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    return 0;
}
