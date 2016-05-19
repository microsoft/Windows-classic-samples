/**********************************************************************/
/*                                                                    */
/*      DATA.C                                                        */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include <windows.h>
#include <imm.h>
#include "wapi.h"

HANDLE hInst      = NULL;
HWND hWndMain     = NULL;
HWND hWndCompStr  = NULL;
HWND hWndToolBar  = NULL;
HWND hWndStatus   = NULL;
HWND hWndCandList = NULL;

LOGFONT lf = {0};
HFONT hFont = NULL;

int nStatusHeight = 0;
int nToolBarHeight = 0;
BOOL fShowCand = FALSE;
DWORD fdwProperty = 0;

DWORD  dwCompStrLen      = 0;
DWORD  dwCompAttrLen     = 0;
DWORD  dwCompClsLen      = 0;
DWORD  dwCompReadStrLen  = 0;
DWORD  dwCompReadAttrLen = 0;
DWORD  dwCompReadClsLen  = 0;
DWORD  dwResultStrLen      = 0;
DWORD  dwResultClsLen      = 0;
DWORD  dwResultReadStrLen  = 0;
DWORD  dwResultReadClsLen  = 0;

MYCHAR szCompStr[512]         = {0};
BYTE   bCompAttr[512]         = {0};
DWORD  dwCompCls[128]         = {0};
MYCHAR szCompReadStr[512]     = {0};
BYTE   bCompReadAttr[512]     = {0};
DWORD  dwCompReadCls[128]     = {0};
MYCHAR szResultStr[512]       = {0};
DWORD  dwResultCls[128]       = {0};
MYCHAR szResultReadStr[512]   = {0};
DWORD  dwResultReadCls[128]   = {0};
MYCHAR szPaintResult[512]     = {0};
MYCHAR szPaintResultRead[512] = {0};

LPCANDIDATELIST lpCandList = NULL;

POINT ptImeUIPos = {0};

