/**********************************************************************/
/*                                                                    */
/*      STATUS.C                                                      */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <imm.h>
#include "resource.h"
#include "imeapps.h"

const char szImeNull[]     = "";
const char szImeOpen[]     = "On";
const char szImeClose[]    = "Off";
const char szImeAlpha[]    = "Alpha";
const char szImeNative[]   = "Native";
const char szImeKatakana[] = "Kata";
const char szImeHalf[]     = "Half";
const char szImeFull[]     = "Full";
const char szImeRoman[]    = "Roman";
const char szImeCode[]     = "Code";
const char szImeHanja[]    = "Hanja";
const char szImeSoftKbd[]  = "SoftKbd";
const char szImeNoConv[]   = "NoConv";
const char szImeEUDC[]     = "EUDC";
const char szImeSymbol[]   = "Symbol";

#define SBITEM_OPENSTATUS    0
#define SBITEM_NATIVEMODE    1
#define SBITEM_FULLSHAPE     2
#define SBITEM_ROMAN         3
#define SBITEM_CHARCODE      4
#define SBITEM_HANJA         5
#define SBITEM_SOFTKBD       6
#define SBITEM_NOCONVERSION  7
#define SBITEM_EUDC          8
#define SBITEM_SYMBOL        9

#define SBITEM_CMODEFIRST    1
#define SBITEM_CMODELAST     9

#define NUM_PARTS 10
int nPartsWidthTbl[NUM_PARTS] = {30,50,30,30,30,30,30,30,30,30};

BOOL CreateStatus(HWND hWnd)
{
    int *pPartsWidth;
    int i;
    int nWidth = 0;

    hWndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, NULL ,
                                    hWnd,  STATUS_ID);


    if (!hWndStatus)
    {
        return FALSE; 
    }

    pPartsWidth = (int *)LocalAlloc(LPTR,sizeof(int) * NUM_PARTS);

    for (i = 0; i < NUM_PARTS; i++)
    {
        nWidth += nPartsWidthTbl[i];
        pPartsWidth[i] = nWidth;
    }

    SendMessage(hWndStatus, SB_SETPARTS,
                (WPARAM)NUM_PARTS,(LPARAM)pPartsWidth);

    LocalFree((HANDLE)pPartsWidth);

    return TRUE;
}

void SetStatusItems(HWND hWnd)
{
    HIMC hIMC = NULL;
    BOOL fOpen;
    DWORD dwConvMode,dwSentMode;

    hIMC = ImmGetContext(hWnd);

    fOpen = ImmGetOpenStatus(hIMC);
    SetOpenStatusParts(fOpen);
    ImmGetConversionStatus(hIMC,&dwConvMode,&dwSentMode);

    if (fOpen)
    {
        SetConvModeParts(dwConvMode);
    }
    else
    {
        ClearConvModeParts();
    }
    
    ImmReleaseContext(hWnd,hIMC);
}


void SetOpenStatusParts(BOOL fOpen)
{
    if (fOpen)
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_OPENSTATUS,(LPARAM)&szImeOpen);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_OPENSTATUS,(LPARAM)&szImeClose);
    }
}

void SetConvModeParts(DWORD dwConvMode)
{
    switch (dwConvMode & IME_CMODE_LANGUAGE)
    {
        case IME_CMODE_ALPHANUMERIC:
            SendMessage(hWndStatus,SB_SETTEXT,
                        SBITEM_NATIVEMODE,(LPARAM)&szImeAlpha);
            break;

        case IME_CMODE_NATIVE:
            SendMessage(hWndStatus,SB_SETTEXT,
                        SBITEM_NATIVEMODE,(LPARAM)&szImeNative);

            break;

        case (IME_CMODE_NATIVE | IME_CMODE_KATAKANA):
            SendMessage(hWndStatus,SB_SETTEXT,
                        SBITEM_NATIVEMODE,(LPARAM)&szImeKatakana);

            break;

        default:
            SendMessage(hWndStatus,SB_SETTEXT,
                        SBITEM_NATIVEMODE,(LPARAM)&szImeNull);
            break;
    }


    if (dwConvMode & IME_CMODE_ROMAN)
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_ROMAN,(LPARAM)&szImeRoman);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_ROMAN,(LPARAM)&szImeNull);
    }


    if (dwConvMode & IME_CMODE_CHARCODE)
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_CHARCODE,(LPARAM)&szImeCode);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_CHARCODE,(LPARAM)&szImeNull);
    }


    if (dwConvMode & IME_CMODE_FULLSHAPE)
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_FULLSHAPE,(LPARAM)&szImeFull);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_FULLSHAPE,(LPARAM)&szImeHalf);
    }


    if (dwConvMode & IME_CMODE_HANJACONVERT)
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_HANJA,(LPARAM)&szImeHanja);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_HANJA,(LPARAM)&szImeNull);
    }


    if (dwConvMode & IME_CMODE_SOFTKBD)
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_SOFTKBD,(LPARAM)&szImeSoftKbd);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_SOFTKBD,(LPARAM)&szImeNull);
    }


    if (dwConvMode & IME_CMODE_NOCONVERSION)
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_NOCONVERSION,(LPARAM)&szImeNoConv);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,
                    SBITEM_NOCONVERSION,(LPARAM)&szImeNull);
    }


    if (dwConvMode & IME_CMODE_EUDC)
    {
        SendMessage(hWndStatus,SB_SETTEXT,SBITEM_EUDC,(LPARAM)&szImeEUDC);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,SBITEM_EUDC,(LPARAM)&szImeNull);
    }
    

    if (dwConvMode & IME_CMODE_SYMBOL)
    {
        SendMessage(hWndStatus,SB_SETTEXT,SBITEM_SYMBOL,(LPARAM)&szImeSymbol);
    }
    else
    {
        SendMessage(hWndStatus,SB_SETTEXT,SBITEM_SYMBOL,(LPARAM)&szImeNull);
    }
}


void ClearConvModeParts()
{
    int i;

    for (i = SBITEM_CMODEFIRST; i <= SBITEM_CMODELAST; i++)
    {
        SendMessage(hWndStatus,SB_SETTEXT, i,(LPARAM)&szImeNull);
    }
}

