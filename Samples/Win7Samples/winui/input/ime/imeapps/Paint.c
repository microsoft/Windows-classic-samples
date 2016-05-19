/**********************************************************************/
/*                                                                    */
/*      PAINT.C                                                       */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <imm.h>
#include "resource.h"
#include "imeapps.h"

#define DEBUG 1



void SetAttrColor(HDC hDC, BYTE bAttr)
{
    switch (bAttr)
    {
        case ATTR_INPUT:
            SetTextColor(hDC,RGB(0,0,0));
            SetBkMode(hDC,TRANSPARENT);
            break;
            
        case ATTR_TARGET_CONVERTED:
            SetTextColor(hDC,RGB(255,255,255));
            SetBkMode(hDC,OPAQUE);
            SetBkColor(hDC,RGB(0,0,255));
            break;
            
        case ATTR_CONVERTED:
            SetTextColor(hDC,RGB(0,0,255));
            SetBkMode(hDC,TRANSPARENT);
            break;
            
        case ATTR_TARGET_NOTCONVERTED:
            SetTextColor(hDC,RGB(255,255,255));
            SetBkMode(hDC,OPAQUE);
            SetBkColor(hDC,RGB(0,255,0));
            break;
            
        case ATTR_INPUT_ERROR:
            SetTextColor(hDC,RGB(255,255,0));
            SetBkMode(hDC,TRANSPARENT);
            break;
            
        case ATTR_FIXEDCONVERTED:
            SetTextColor(hDC,RGB(255,0,0));
            SetBkMode(hDC,TRANSPARENT);
            break;
            
        default:
            SetTextColor(hDC,RGB(0,0,0));
            SetBkMode(hDC,TRANSPARENT);
            break;
    }
}

LRESULT HandlePaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HDC hDC = NULL;
    PAINTSTRUCT ps = {0};
    UINT i;
    int x = ORG_X;
    int y = ORG_Y;
    SIZE sz;
    HFONT hOldFont = NULL;
    HFONT hDefFont = GetStockObject(DEFAULT_GUI_FONT);
    int height,defheight;
    const TCHAR  szResult[] = TEXT("Result String");
    const TCHAR  szComp[] = TEXT("Composition String");
    RECT rc = {0};

    GetClientRect(hWnd,&rc);

    hDC = BeginPaint(hWnd, &ps);

    if (hFont)
    {
        hOldFont = SelectObject(hDC,hDefFont);
    }


    // Get the height of the default gui font.
    GetTextExtentPoint(hDC,TEXT("A"),1,&sz);
    defheight = sz.cy + 1;

    // Get the height of the font.
    SelectObject(hDC,hFont);
    GetTextExtentPoint(hDC,TEXT("A"),1,&sz);
    height = sz.cy + 1;

    SelectObject(hDC,hDefFont);
    SetTextColor(hDC,RGB(0,0,0));
    SetBkMode(hDC,TRANSPARENT);
    TextOut(hDC,ORG_X,y,szResult,lstrlen(szResult));
    y += defheight;

    if (Mylstrlen(szPaintResult))
    {
        x = ORG_X;
        SelectObject(hDC,hFont);
        SetTextColor(hDC,RGB(255,0,0));
        SetBkMode(hDC,TRANSPARENT);
#ifdef USEWAPI
        TextOutW(hDC,x,y,szPaintResult,Mylstrlen(szPaintResult));
#else
        TextOut(hDC,x,y,szPaintResult,Mylstrlen(szPaintResult));
#endif
    }

    y += height;

    if (Mylstrlen(szPaintResultRead))
    {
        x = ORG_X;
        SelectObject(hDC,hFont);
        SetTextColor(hDC,RGB(255,0,0));
        SetBkMode(hDC,TRANSPARENT);
#ifdef USEWAPI
        TextOutW(hDC,x,y,szPaintResultRead,Mylstrlen(szPaintResultRead));
#else
        TextOut(hDC,x,y,szPaintResultRead,Mylstrlen(szPaintResultRead));
#endif
    }
#if 0
    if (dwResultReadStrLen)
    {
        x = ORG_X;
        SelectObject(hDC,hFont);
        SetTextColor(hDC,RGB(0,0,0));
        SetBkMode(hDC,TRANSPARENT);

        if (dwResultReadClsLen)
        {
            dwResultReadCls[127] = 0;
            i = 1;

            SetTextColor(hDC,RGB(255,0,0));
            while (dwResultReadCls[i] && dwResultReadCls[i-1] < dwResultReadStrLen)
            {
                DWORD dwTextLen = dwResultReadCls[i] - dwResultReadCls[i-1];
                LPSTR lpStart = szResultReadStr + dwResultReadCls[i-1];

                TextOut(hDC,x,y,lpStart,dwTextLen);
                GetTextExtentPoint(hDC,lpStart,dwTextLen,&sz);
                x += sz.cx;

                TextOut(hDC,x,y,",",1);
                GetTextExtentPoint(hDC,",",1,&sz);
                x += (sz.cx + 2);

                i++;
            }
        }
        else
        {
            SetTextColor(hDC,RGB(255,0,0));
            SetBkMode(hDC,TRANSPARENT);
            TextOut(hDC,x,y,szResultReadStr,dwResultReadStrLen);
        }


    }
#endif

    y += height;

    SelectObject(hDC,hDefFont);
    SetTextColor(hDC,RGB(0,0,0));
    SetBkMode(hDC,TRANSPARENT);
    TextOut(hDC,ORG_X,y,szComp,lstrlen(szComp));
    y += defheight;

    if (dwCompStrLen)
    {
        x = ORG_X;
        SelectObject(hDC,hFont);
        SetTextColor(hDC,RGB(0,0,0));

        if (dwCompClsLen && dwCompAttrLen)
        {
            dwCompCls[127] = 0;
            i = 1;
          
            while (dwCompCls[i] && dwCompCls[i-1] < dwCompStrLen)
            {
                DWORD dwTextLen = dwCompCls[i] - dwCompCls[i-1];
                LPMYSTR lpStart = szCompStr + dwCompCls[i-1];

                SetAttrColor(hDC, bCompAttr[dwCompCls[i-1]]);
#ifdef USEWAPI
                TextOutW(hDC,x,y,lpStart,dwTextLen);
                GetTextExtentPointW(hDC,lpStart,dwTextLen,&sz);
#else
                TextOut(hDC,x,y,lpStart,dwTextLen);
                GetTextExtentPoint(hDC,lpStart,dwTextLen,&sz);
#endif
                x += sz.cx;

                SetTextColor(hDC,RGB(0,0,0));
                SetBkMode(hDC,TRANSPARENT);
#ifdef USEWAPI
                TextOutW(hDC,x,y,MYTEXT(","),1);
                GetTextExtentPointW(hDC,MYTEXT(","),1,&sz);
#else
                TextOut(hDC,x,y,TEXT(","),1);
                GetTextExtentPoint(hDC,TEXT(","),1,&sz);
#endif
                x += (sz.cx + 2);

                i++;
            }
        }
        else
        {
            SetBkMode(hDC,TRANSPARENT);
#ifdef USEWAPI
            TextOutW(hDC,x,y,szCompStr,dwCompStrLen);
#else
            TextOut(hDC,x,y,szCompStr,dwCompStrLen);
#endif
        }

    }

    y += height;

    if (dwCompReadStrLen)
    {
        x = ORG_X;
        SelectObject(hDC,hFont);
        SetTextColor(hDC,RGB(0,0,0));
        SetBkMode(hDC,TRANSPARENT);

        if (dwCompReadClsLen && dwCompReadAttrLen)
        {
            dwCompReadCls[127] = 0;
            i = 1;

            while (dwCompReadCls[i] && dwCompReadCls[i-1] < dwCompReadStrLen)
            {
                DWORD dwTextLen = dwCompReadCls[i] - dwCompReadCls[i-1];
                LPMYSTR lpStart = szCompReadStr + dwCompReadCls[i-1];

                SetAttrColor(hDC, bCompReadAttr[dwCompReadCls[i-1]]);
#ifdef USEWAPI
                TextOutW(hDC,x,y,lpStart,dwTextLen);
                GetTextExtentPointW(hDC,lpStart,dwTextLen,&sz);
#else
                TextOut(hDC,x,y,lpStart,dwTextLen);
                GetTextExtentPoint(hDC,lpStart,dwTextLen,&sz);
#endif
                x += sz.cx;

                SetTextColor(hDC,RGB(0,0,0));
                SetBkMode(hDC,TRANSPARENT);
#ifdef USEWAPI
                TextOutW(hDC,x,y,MYTEXT(","),1);
                GetTextExtentPointW(hDC,MYTEXT(","),1,&sz);
#else
                TextOut(hDC,x,y,TEXT(","),1);
                GetTextExtentPoint(hDC,TEXT(","),1,&sz);
#endif
                x += (sz.cx + 2);

                i++;
            }
        }
        else
        {
            SetBkMode(hDC,TRANSPARENT);
#ifdef USEWAPI
            TextOutW(hDC,x,y,szCompReadStr,dwCompReadStrLen);
#else
            TextOut(hDC,x,y,szCompReadStr,dwCompReadStrLen);
#endif
        }

    }

    y += height;

    ptImeUIPos.y = y;

    SelectObject(hDC,hOldFont);
    EndPaint(hWnd,&ps);
    return 1;
}

LRESULT HandleCandPaint(HWND hWnd,WPARAM wParam, LPARAM lParam)
{
    HDC hDC = NULL;
    PAINTSTRUCT ps = {0};
    UINT i;
    int x = ORG_X;
    int y = ORG_Y;
    SIZE sz;
    HFONT hOldFont = NULL;
    HFONT hDefFont = GetStockObject(DEFAULT_GUI_FONT);
    int height,defheight;
    LPDWORD lpdwOffset = NULL;
    RECT rect = {0};
    const TCHAR  szCand[] = TEXT("Candidate List");
    const TCHAR  szCandNull[] = TEXT("");
    const TCHAR  szCandRead[] = TEXT("Reading");
    const TCHAR  szCandCode[] = TEXT("Code");
    const TCHAR  szCandMean[] = TEXT("Meaning");
    const TCHAR  szCandRadi[] = TEXT("Radical");
    const TCHAR  szCandStrk[] = TEXT("Stroke");


    GetClientRect(hWnd,&rect);

    hDC = BeginPaint(hWnd, &ps);

    if (!lpCandList)
    {
        goto pt_cand_10;
    }

    if (hFont)
    {
        hOldFont = SelectObject(hDC,hDefFont);
    }


    // Get the height of the default gui font.
    GetTextExtentPoint(hDC,TEXT("A"),1,&sz);
    defheight = sz.cy + 1;

    // Get the height of the font.
    SelectObject(hDC,hFont);
    GetTextExtentPoint(hDC,TEXT("A"),1,&sz);
    height = sz.cy + 1;

    SelectObject(hDC,hDefFont);
    SetTextColor(hDC,RGB(0,0,0));
    SetBkMode(hDC,TRANSPARENT);
    TextOut(hDC,ORG_X,y,szCand,lstrlen(szCand));
    y += defheight;

    switch (lpCandList->dwStyle)
    {
        case IME_CAND_READ:
            TextOut(hDC,ORG_X,y,szCandRead,lstrlen(szCandRead));
            break;

        case IME_CAND_CODE:
            TextOut(hDC,ORG_X,y,szCandCode,lstrlen(szCandCode));
            break;

        case IME_CAND_MEANING:
            TextOut(hDC,ORG_X,y,szCandMean,lstrlen(szCandMean));
            break;

        case IME_CAND_RADICAL:
            TextOut(hDC,ORG_X,y,szCandRadi,lstrlen(szCandRadi));
            break;

        case IME_CAND_STROKE:
            TextOut(hDC,ORG_X,y,szCandStrk,lstrlen(szCandStrk));
            break;

        default:
            break;

    }
    y += defheight;

    if (!lpCandList->dwCount)
    {
        goto pt_cand_10;
    }

    lpdwOffset = &lpCandList->dwOffset[0];

    lpdwOffset += lpCandList->dwPageStart;

    for (i = lpCandList->dwPageStart;
         (i < lpCandList->dwCount) && 
         (i < lpCandList->dwPageStart + lpCandList->dwPageSize) &&
         (y <= rect.bottom + height); i++)
    {
        LPTSTR lpstr = (LPTSTR)((BYTE *)lpCandList + *lpdwOffset++);

        x = ORG_X;

        SelectObject(hDC,hFont);
        if (i != lpCandList->dwSelection)
        {
            SetTextColor(hDC,RGB(0,0,0));
            SetBkMode(hDC,TRANSPARENT);
        }
        else
        {
            SetTextColor(hDC,RGB(255,255,255));
            SetBkColor(hDC,RGB(0,0,255));
            SetBkMode(hDC,OPAQUE);
        }

        TextOut(hDC,x,y,lpstr,lstrlen(lpstr));
        y += height;
    }

    SelectObject(hDC,hOldFont);

pt_cand_10:
    EndPaint(hWnd,&ps);
    return 1;
}

