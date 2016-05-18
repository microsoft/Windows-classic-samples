//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  cdialog.cpp
//
//  Purpose:
//  This module implements the CSimpleDialog class, which is 
//  used in the sample to display the User Interface.
// 
//  Note that the actual file upload is triggered by pressing "OK" on the UI.
//  So looking at the implementation of the method OnOK() is a good
//  starting point for tracking the sample's main code path.
//
//----------------------------------------------------------------------------


#include <windows.h>
#include <crtdbg.h>
#include <strsafe.h>
#include <wininet.h>

#include "resource.h"
#include "main.h"
#include "util.h"
#include "cdialog.h"
#include "cpack.h"



const WCHAR STR_VIRTUALDIR_PLAIN[]         = L"http://localhost/UploadSample";
const WCHAR STR_VIRTUALDIR_NOTIFICATIONS[] = L"http://localhost/UploadSampleWithNotifications";

const WCHAR STR_SAMPLETEXT_DEFAULT[] = L"Enter some text here";
const WCHAR STR_JOBNAME_DEFAULT[]    = L"BITS Upload Sample File";


// ==========================================================================
// Constructor/Destructor for CSimpleDialog
// ==========================================================================

//---------------------------------------------------------------------------
CSimpleDialog::CSimpleDialog(HINSTANCE hInstance, ULONG ulDialogId)
{
    m_hInstance   = hInstance;
    m_ulDialogId  = ulDialogId;
    m_hWnd        = NULL;           // Handle returned by CreateDialogParam 
    m_DlgProc     = DlgProc;        // Point to default dialog proc
}

//---------------------------------------------------------------------------
CSimpleDialog::~CSimpleDialog()    
{
}

//----------------------------------------------------------------------------
// Public methods exposed by CSimpleDialog
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
HRESULT CSimpleDialog::Show(INT iShowState)
{
    HRESULT hr = S_OK;

    m_hWnd = CreateDialogParam(
        m_hInstance,
        MAKEINTRESOURCE(m_ulDialogId),
        NULL,                           // hWndParent
        m_DlgProc,
        (LPARAM)this
        );

    if (!m_hWnd)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        // the return just says if the window was previously visible or not
        BOOL bReturn = ShowWindow(m_hWnd, iShowState);  
    }

    return hr;
}

//---------------------------------------------------------------------------
HWND CSimpleDialog::GetHwnd()
{
    return m_hWnd;
}

//---------------------------------------------------------------------------
HRESULT CSimpleDialog::AddStatusMessage(LPCWSTR wszFormat, ...)
{
    HRESULT    hr                          = S_OK;
    HWND       hJobStatus;
    WCHAR      *pwszExistingText           = NULL;
    DWORD      cchExistingText             = 0;
    WCHAR      *pwszBuf                    = NULL;
    DWORD      cchBuf                      = 0;
    SYSTEMTIME Time;
    WCHAR      wszTimeBuf[20]              = {0};
    WCHAR      wszNewText[MAX_BUFFER_SIZE] = {0};
    

    if (!wszFormat)
    {
        return S_OK;
    }

    // 
    // Build a string with the text passed as a parameter
    //
    va_list arglist;
    va_start(arglist, wszFormat);

    hr = StringCchVPrintfW(wszNewText, MAX_BUFFER_SIZE, wszFormat, arglist);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    //
    // Retrieve the text that is currently displayed in the Edit box
    //

    hJobStatus      = GetDlgItem(m_hWnd, IDC_JOBSTATUS);
    cchExistingText = GetWindowTextLength(hJobStatus) + 1;

    if (cchExistingText)
    {
        pwszExistingText = new WCHAR[cchExistingText];
        if (!pwszExistingText)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }

        GetWindowText(hJobStatus, pwszExistingText, cchExistingText);
    }

    //
    // Get a string for the current time
    //

    GetLocalTime(&Time);
    hr = StringCchPrintfW(wszTimeBuf, ARRAYSIZE(wszTimeBuf), L"[%.2u:%.2u %.2u.%.3us] ", Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    //
    // Allocate a buffer for the new contents of the edit box
    //
    cchBuf  = cchExistingText + static_cast<DWORD>(wcslen(wszTimeBuf)) + static_cast<DWORD>(wcslen(wszNewText)) + 2 + 1;  // 2 for the break line
    pwszBuf = new WCHAR[cchBuf];
    if (!pwszBuf)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }
    pwszBuf[0] = L'\0';

    //
    // Build the new string
    //
    hr = StringCchPrintfW(pwszBuf, cchBuf, L"%s%s%s\r\n", (pwszExistingText? pwszExistingText : L""), wszTimeBuf, wszNewText);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    //
    // Update the control with the new string
    //
    if (!SetWindowText(hJobStatus, pwszBuf))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    //
    // Automatically scroll down the edit box
    //
    SendMessage(hJobStatus, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
	SendMessage(hJobStatus, EM_SCROLLCARET, 0, 0);

cleanup:

    va_end(arglist);

    if (pwszExistingText)
    {
        delete [] pwszExistingText;
        pwszExistingText = NULL;
    }

    if (pwszBuf)
    {
        delete [] pwszBuf;
        pwszBuf = NULL;
    }

    return hr;
}

// ==========================================================================
// Message handlers (private)
// ==========================================================================

//---------------------------------------------------------------------------
LRESULT CSimpleDialog::OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    HWND    hVirtualDir   = NULL;
    HWND    hSampleText   = NULL;
    HWND    hUploadReply  = NULL;

    // Set combobox contents
	SendDlgItemMessage(hDlg, IDC_VIRTUALDIR, CB_ADDSTRING, 0, (LPARAM)STR_VIRTUALDIR_PLAIN);
	SendDlgItemMessage(hDlg, IDC_VIRTUALDIR, CB_ADDSTRING, 0, (LPARAM)STR_VIRTUALDIR_NOTIFICATIONS);
    SendDlgItemMessage(hDlg, IDC_VIRTUALDIR, CB_LIMITTEXT, (WPARAM)(INTERNET_MAX_URL_LENGTH-1), (LPARAM)0);
    SendDlgItemMessage(hDlg, IDC_VIRTUALDIR, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

    //hVirtualDir = GetDlgItem(hDlg, IDC_VIRTUALDIR);
    //SetWindowText(hVirtualDir, STR_VIRTUALDIR_DEFAULT); 

    hSampleText = GetDlgItem(hDlg, IDC_SAMPLETEXT);
    SetWindowText(hSampleText, STR_SAMPLETEXT_DEFAULT); 

    hUploadReply = GetDlgItem(hDlg, IDC_UPLOADREPLY_CHECKBOX);
    SendMessage(hUploadReply, BM_SETCHECK, BST_UNCHECKED, 0 );    


    SetFocus(hSampleText);

    return 1;
}

//---------------------------------------------------------------------------
LRESULT CSimpleDialog::OnOK(HWND hDlg, WPARAM wParam, LPARAM lParam)
{ 
    HRESULT hr = S_OK;
    WCHAR   wszBuffSampleText[MAX_BUFFER_SIZE]         = {0};
    WCHAR   wszBuffVirtualDir[INTERNET_MAX_URL_LENGTH] = {0};
    BOOL    fRequireUploadReply = FALSE;
    LRESULT lRet = 1;
    CPack   XMLPack;

    //
    // Grab input values from the UI
    //
    hr = CollectUserInput(
        wszBuffVirtualDir, 
        ARRAYSIZE(wszBuffVirtualDir),
        wszBuffSampleText, 
        ARRAYSIZE(wszBuffSampleText),
        &fRequireUploadReply
    );
    if (FAILED(hr))
    {

        DisplayErrorMessage(L"Failed to collect input values from the UI.", hr);
        lRet = 0;
        goto done;
    }


    //
    //  Pack the text as an XML file and UPLOAD it to the server!!
    //
    AddStatusMessage(L"START OF PROCESS TO UPLOAD FILE");

    hr = XMLPack.PackText(wszBuffSampleText);
    if (FAILED(hr))
    {
        lRet = 0;
        goto done;
    }

    hr = XMLPack.Upload(STR_JOBNAME_DEFAULT, wszBuffVirtualDir, fRequireUploadReply);
    if (FAILED(hr))
    {
        lRet = 0;
        goto done;
    }

done:

    if (!lRet)
    {
        AddStatusMessage(L"END OF UPLOAD PROCESS -- FAILED");
    }

    return lRet;
}

//---------------------------------------------------------------------------
//  This is the first message sent by the DetroyWindow() function.
//  It is sent after the window is removed from the screen, but it should
//  be assumed that any child windows still exist.
//
LRESULT CSimpleDialog::OnDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    PostQuitMessage(0);
    return 1;
}

//---------------------------------------------------------------------------
//  This is sent after WM_DESTROY, and informs the app that the non-client
//  area is being destroyed.  This is where allocated resources should be
//  cleaned up.
//
LRESULT CSimpleDialog::OnNcDestroy(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    m_hWnd = NULL;             // Since this is a modeless-only dialog 

    return 1;
}


//---------------------------------------------------------------------------
LRESULT CSimpleDialog::OnCancel(HWND hDlg, WPARAM wParam, LPARAM lParam)
{ 
    return DestroyWindow(m_hWnd);
}

//---------------------------------------------------------------------------
LRESULT CSimpleDialog::OnClose(HWND hDlg, WPARAM wParam, LPARAM lParam)
{ 
    HRESULT hr = S_OK;
    CSmartComPtr<IEnumBackgroundCopyJobs> JobsEnum;
    CSmartComPtr<IBackgroundCopyJob>      Job;
    WCHAR *pwszJobName = NULL;

    //
    // As we are leaving the application, cancel jobs that are still
    // in progress. This will only apply to Jobs submitted by this 
    // user AND with our predefined description string
    //
    hr = g_JobManager->EnumJobs(0, JobsEnum.GrabOutPtr());
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to create a BITS job enumerator. Error id is 0x%X.", hr);
        goto done;
    }

    while((JobsEnum->Next(1, Job.GrabOutPtr(), NULL) == S_OK))
    {
        hr = Job->GetDisplayName(&pwszJobName);
        if (SUCCEEDED(hr))
        {
            if (wcscmp(pwszJobName, STR_JOBNAME_DEFAULT) == 0)
            {
                // Found a job that was created by this app -- remove the
                // notification callback and then cancel it
                hr = Job->SetNotifyInterface(NULL);
                if (FAILED(hr))
                {
                    DisplayErrorMessage(L"Failed to remove the notification callback for a job while cleaning up jobs created by this application.", hr);
                }

                hr = Job->Cancel();
                if (FAILED(hr))
                {
                    DisplayErrorMessage(L"Failed to cancel a job while cleaning up jobs created by this application.", hr);
                }
            }

            CoTaskMemFree(pwszJobName);
            pwszJobName = NULL;
        }
    }

done:

    return DestroyWindow(m_hWnd);
}


// ==========================================================================
// Main dialog procedure
// ==========================================================================

//---------------------------------------------------------------------------
//  Since this is a static function we can't use a 'this' pointer.
//  We get around this by storing the 'this' pointer in the
//  lParam of CreateDialogParam.  When we get WM_INITDIALOG we get this 
//  pointer back in the lParam.  Now we can use SetWindowLongPtr to store
//  it, and later retireve it with GetWindowLongPtr.  Once we have a 
//  valid 'this' pointer we can cast it back to the base class and then
//  call the base class's ProcessMessage function.
//

INT_PTR CALLBACK CSimpleDialog::DlgProc (
  HWND     hDlg,           //[in] Handle to dialog box
  UINT     uMsg,           //[in] Message
  WPARAM   wParam,         //[in] First message parameter
  LPARAM   lParam          //[in] Second message parameter
  )
{
    BOOL bReturn = FALSE;
    if (WM_INITDIALOG == uMsg)
    {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);  
    }

    CSimpleDialog* pThis = reinterpret_cast<CSimpleDialog *>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
    if (pThis != NULL)
    {
        bReturn = pThis->ProcessMessage(hDlg, uMsg, wParam, lParam);  
    }

    return bReturn;
}

//---------------------------------------------------------------------------
//  This is the instance specific method used to process messages.  The
//  DialogProc delegates this responsibility to each instance of the class.
//
BOOL CSimpleDialog::ProcessMessage (
  HWND     hDlg,           //[in] Handle to dialog box
  UINT     uMsg,           //[in] Message
  WPARAM   wParam,         //[in] First message parameter
  LPARAM   lParam          //[in] Second message parameter
  )
{
    LRESULT lResult = 0;
    switch (uMsg)
    {
        case WM_INITDIALOG:
            lResult = OnInitDialog(hDlg, wParam, lParam);
            break;

        case WM_DESTROY:
            lResult = OnDestroy(hDlg, wParam, lParam);
            break;

        case WM_NCDESTROY:
            lResult = OnNcDestroy(hDlg, wParam, lParam);
            break;

        case WM_CLOSE:
            lResult = OnClose(hDlg, wParam, lParam);
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                    lResult = OnOK(hDlg, wParam, lParam);
                    break;

                case IDCANCEL:
                    lResult = OnCancel(hDlg, wParam, lParam);
                    break;

                default:
                    break;
            }
        default:
            break;
    }

    return(BOOL)lResult;
} 

// ==========================================================================
// Auxiliary methods (private)
// ==========================================================================

HRESULT CSimpleDialog::CollectUserInput(
    IN OUT WCHAR *wszBuffVirtualDir, 
    IN     DWORD  cchBuffVirtualDir, 
    IN OUT WCHAR *wszBuffSampleText, 
    IN     DWORD  cchBuffSampleText,
    OUT    BOOL  *fRequireUploadReply 
)
{ 
    HRESULT hr                      = S_OK;
    HWND    hSampleText             = NULL;
    HWND    hVirtualDir             = NULL;
    HWND    hUploadReply            = NULL;
    INT     iEnd                    = 0;
    LRESULT idx                     = CB_ERR;
    LRESULT len                     = 0;

    if (!wszBuffVirtualDir || !wszBuffSampleText || !fRequireUploadReply)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    *fRequireUploadReply = FALSE;

    hVirtualDir = GetDlgItem(m_hWnd, IDC_VIRTUALDIR);

     // get the index of the selection on the combobox
    idx = SendMessage(hVirtualDir, CB_GETCURSEL, (LPARAM)0, 0);
    if (idx == CB_ERR)
    {
        // the user probably edited something and used the combobox as an edit box
        len = SendMessage(hVirtualDir, WM_GETTEXTLENGTH, 0, 0);

        if ((len == CB_ERR) || ((static_cast<DWORD>(len)+1) >= cchBuffVirtualDir))   // cchBuff includes the \0
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            goto cleanup;
        }

        SendMessage( 
          hVirtualDir,                // handle to destination window 
          WM_GETTEXT,                 // message to send
          (LPARAM)len,                // number of characters to copy
          (WPARAM)wszBuffVirtualDir   // text buffer
        );
    }
    else
    {
        len = SendMessage( 
          hVirtualDir,                 // handle to destination window 
          CB_GETLBTEXTLEN,             // message to send
          (WPARAM)idx,                 // item index
          (LPARAM)0                    // not used; must be zero
        );

        if ((len == CB_ERR) || ((static_cast<DWORD>(len)+1) >= cchBuffVirtualDir)) // len + 1 includes the \0
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            goto cleanup;
        }

        // get the string of the current selection
        SendMessage(hVirtualDir, CB_GETLBTEXT, (LPARAM)idx, (WPARAM)wszBuffVirtualDir);
    }


    // if virtual directory doesn't end with a /, add it
    iEnd = static_cast<INT>(wcslen(wszBuffVirtualDir));
    if (iEnd > 0 && wszBuffVirtualDir[iEnd-1] != L'/')
    {
        hr = StringCchCat(wszBuffVirtualDir, cchBuffVirtualDir, L"/");
        if (FAILED(hr))
        {
            // we should never get here;
            goto cleanup;
        }
    }

    hSampleText = GetDlgItem(m_hWnd, IDC_SAMPLETEXT);
    GetWindowText(hSampleText, wszBuffSampleText, cchBuffSampleText);

    hUploadReply = GetDlgItem(m_hWnd, IDC_UPLOADREPLY_CHECKBOX);
    if (SendMessage(hUploadReply, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        *fRequireUploadReply = TRUE;
    }

cleanup:
    
    return hr;
}
