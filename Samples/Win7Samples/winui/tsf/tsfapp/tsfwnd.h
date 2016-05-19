/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          TSFWnd.h

   Description:   CTSFMainWnd Class Declaration

**************************************************************************/

#ifndef TSFMAINWND_H
#define TSFMAINWND_H

/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <msctf.h>
#include "resource.h"
#include "TSFEdit.h"

/**************************************************************************
   global variables and definitions
**************************************************************************/

/**************************************************************************

   CTSFMainWnd class definition

**************************************************************************/

class CTSFMainWnd
{
private:
    HINSTANCE       m_hInst;
    HWND            m_hWnd;
    CTSFEditWnd     *m_pTSFEditWnd;
    TfClientId      m_tfClientID;
   
public:
    CTSFMainWnd(HINSTANCE hInstance);
    ~CTSFMainWnd();
    
    BOOL Initialize(int nCmdShow);
   
private:
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    void _CleanupEditWnd(BOOL fNuke);
    LRESULT _OnCreate(VOID);
    LRESULT _OnDestroy(VOID);
    LRESULT _OnCommand(WORD, WORD, HWND);
    LRESULT _OnSetFocus(VOID);
    LRESULT _OnKillFocus(VOID);
    LRESULT _OnNotify(UINT, LPNMHDR);
    LRESULT _OnSize(WORD, WORD);
    LRESULT _OnActivate(WPARAM);
    LRESULT _OnInitMenuPopup(WPARAM, LPARAM);
    BOOL _GetFileName(HWND hwndOwner, LPTSTR lpszFileName, ULONG uChars, BOOL fOpen);

};

#endif   //TSFMAINWND_H
