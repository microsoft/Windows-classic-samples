/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __PROGRESSDLG__
#define __PROGRESSDLG__

namespace WiaWrap
{

//////////////////////////////////////////////////////////////////////////
//
// CProgressDlg
//

/*++

    CProgressDlg implements a simple progress dialog with a status message,
    progress bar and a cancel button. The dialog is created in a separate 
    thread so that it remains responsive to the user while the main thread 
    is busy with a long operation. It is descended from IUnknown for 
    reference counting.

Methods

    CProgressDlg
        Creates a new thread that will display the progress dialog

    ~CProgressDlg
        Closes the progress dialog

    Cancelled
        Returns TRUE if the user has pressed the cancel button.

    SetTitle
        Sets the title of the progress dialog.

    SetMessage
        Sets the status message.

    SetPercent
        Sets the progress bar position. 

    ThreadProc
        Creates the dialog box

    DialogProc
        Processes window messages.

--*/

class CProgressDlg : public IUnknown
{
public:
    CProgressDlg(HWND hWndParent);
    ~CProgressDlg();

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // CProgressDlg methods

    BOOL Cancelled() const;

    VOID SetTitle(PCTSTR pszTitle);
    VOID SetMessage(PCTSTR pszMessage);
    VOID SetPercent(UINT nPercent);

private:
    static DWORD WINAPI ThreadProc(PVOID pParameter);
    static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);

private:
    LONG    m_cRef;
	HWND    m_hDlg;
	HWND    m_hWndParent;
    LONG    m_bCancelled;
    HANDLE  m_hInitDlg;
};

}; // namespace WiaWrap

#endif //__PROGRESSDLG__

