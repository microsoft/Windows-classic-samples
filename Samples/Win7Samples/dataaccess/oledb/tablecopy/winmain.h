//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module WINMAIN.H
//
//-----------------------------------------------------------------------------

#ifndef _WINMAIN_H_
#define _WINMAIN_H_


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////

//We want type checking on Window Handles
#define STRICT


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>	//InitCommonControls

#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <wchar.h>

#include "resource.h"



////////////////////////////////////////////////////////////////////////////
// Windows Defines
//
////////////////////////////////////////////////////////////////////////////
#define CHECK_MEMORY(pv)	if(!pv) { OutOfMemory(NULL); goto CLEANUP; }

//Dialog Box procedures want to know if you handled the MSG
//or not.  If you do, thenit just returns, if not then it calls
//the default windialog procedure to try and handle it
const BOOL HANDLED_MSG	 = TRUE;
const BOOL UNHANDLED_MSG = FALSE;
#define LVM_ERR (-1)

////////////////////////////////////////////////////////////////////////////
// Windows functions
//
////////////////////////////////////////////////////////////////////////////
void Busy(BOOL bValue = TRUE);
void OutOfMemory(HWND hwnd);

INT wMessageBox(HWND hDlg, UINT uiStyle, WCHAR* pwszTitle, WCHAR* pwszFmt, ...);
LRESULT wSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszName);

void wSetDlgItemText(HWND hWnd, INT DlgItem, WCHAR* pwszFmt, ...);
UINT wGetDlgItemText(HWND hWnd, INT DlgItem, WCHAR* pwsz, INT nMaxSize);

BOOL CenterDialog(HWND hDlg);
void SyncSibling(HWND hwndLstChg,HWND hwndLstSrc);

BOOL GetEditBoxValue(HWND hEditWnd, ULONG ulMin, ULONG ulMax, ULONG* pulCount);


/////////////////////////////////////////////////////////////////////
// ListView Helpers
//
/////////////////////////////////////////////////////////////////////
LONG LV_InsertColumn(HWND hWnd, LONG iColumn, CHAR* szName);
LONG LV_InsertItem(HWND hWnd, LONG iItem, LONG iSubItem, CHAR* szName, LONG iParam = 0, LONG iIMage = NULL);
LONG LV_SetItemState(HWND hWnd, LONG iItem, LONG iSubItem, LONG lState, LONG lStateMask);
LONG LV_SetItemText(HWND hWnd, LONG iItem, LONG iSubItem, CHAR* szName);
LONG LV_FindItem(HWND hWnd, CHAR* szName, LONG iStart);


/////////////////////////////////////////////////////////////////////
// TreeView Helpers
//
/////////////////////////////////////////////////////////////////////
HTREEITEM TV_InsertItem(HWND hWnd, HTREEITEM hParent, HTREEITEM hInsAfter, CHAR* szName, LONG iParam = 0, LONG iImage = 0, LONG iSelectedImage = 0);


/////////////////////////////////////////////////////////////////////
// Memory debugging code
//
/////////////////////////////////////////////////////////////////////
int InternalAssert(char* pszExp, char* pszFile, UINT iLine);
void InternalTrace(CHAR* pszExp, ...);
void InternalTrace(WCHAR* pwszExp, ...);

#undef ASSERT
#undef TRACE

#ifdef _DEBUG
#if     defined(_M_IX86)
#define _DbgBreak() __asm { int 3 }
#else
#define _DbgBreak() DebugBreak()
#endif
#define ASSERT(expr) \
        do { if (!(expr) && \
                (1 == InternalAssert(#expr, __FILE__, __LINE__))) \
             _DbgBreak(); } while (0)

#define TRACE	InternalTrace
#else  //_DEBUG
#define ASSERT(exp)
#define TRACE	if(0) InternalTrace
#endif //_DEBUG


#endif //_WINMAIN_H_
