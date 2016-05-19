//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  cdialog.h
//
//  Purpose:
//  Defines the class CSimpleDialog, used to instantiate and control
//  the user interface.
//
//----------------------------------------------------------------------------

#pragma once
#include <Windows.h>

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

//---------------------------------------------------------------------------
class CSimpleDialog  
{
    HINSTANCE m_hInstance;
    HWND      m_hWnd;         // Handle from CreateDialogParam
    ULONG     m_ulDialogId;   // Dialog template ID
    DLGPROC   m_DlgProc;      // Points to Dialog Proc      

    static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnInitDialog( HWND hDlg, WPARAM wParam, LPARAM lParam );
    LRESULT OnOK( HWND hDlg, WPARAM wParam, LPARAM lParam );
    LRESULT OnCancel( HWND hDlg, WPARAM wParam, LPARAM lParam );
    LRESULT OnNcDestroy( HWND hDlg, WPARAM wParam, LPARAM lParam );
    LRESULT OnDestroy( HWND hDlg, WPARAM wParam, LPARAM lParam );
    LRESULT OnClose( HWND hDlg, WPARAM wParam, LPARAM lParam );

    BOOL ProcessMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    HRESULT CollectUserInput(
        IN OUT WCHAR *wszBuffVirtualDir, 
        IN     DWORD  cchBuffVirtualDir, 
        IN OUT WCHAR *wszBuffSampleText, 
        IN     DWORD  cchBuffSampleText,
        OUT    BOOL  *fRequireUploadReply 
    );

  public:
    CSimpleDialog(HINSTANCE hInstance, ULONG ulDialogId);
    ~CSimpleDialog();

    HRESULT Show(INT iShowState);
    HWND    GetHwnd();
    HRESULT AddStatusMessage(LPCWSTR wszFormat, ...);
};


