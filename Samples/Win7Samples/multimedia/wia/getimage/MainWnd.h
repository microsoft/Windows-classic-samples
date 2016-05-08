/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __MAINWND__
#define __MAINWND__

#include "EventCallback.h"

//////////////////////////////////////////////////////////////////////////
//
// CMainWindow
//

/*++

    CMainWindow is the main MDI frame window. It is descended from IUnknown 
    for reference counting.

Methods

    Register
        Registers the window class

    DoModal
        Invokes the window and returns when the main window is closed. 

    WindowProc
        Processes window messages.

    OnFromScannerOrCamera
        Displays the WIA device selection, image selection and image tranfer 
        dialogs to transfer one or multiple images from a WIA device when the 
        user selects "From Scanner Or Camera..." menu item.

--*/

class CMainWindow : public IUnknown
{
public:
    CMainWindow();

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // CMainWindow methods

    static ATOM Register();

    int DoModal();

private:
    static
    LRESULT 
    CALLBACK
    WindowProc(
        HWND   hWnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    );

    LRESULT OnFromScannerOrCamera();

private:
    LONG  m_cRef;
    HWND  m_hMDIClient;
    LONG  m_nNumImages;
    BOOL  m_bDisplayWaitCursor;

    CComPtr<WiaWrap::CEventCallback>  m_pEventCallback;
};

#endif //__MAINWND__

