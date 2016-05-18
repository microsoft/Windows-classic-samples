//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module WINMAIN.CPP
//
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////////////
#include "winmain.h"
#include "wizard.h"
#include "common.h"
#include "tablecopy.h"
#include "table.h"

//////////////////////////////////////////////////////////////////
// int WINAPI WinMain
//
// Main application entry point
//////////////////////////////////////////////////////////////////
int WINAPI WinMain(	HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    CHAR*  pszCmdLine,
                    INT    nCmdShow)
{
    HRESULT     hr = E_FAIL;
    CWizard*	pCWizard = NULL;

    pCWizard = new CWizard(NULL, hInstance);
    if (!pCWizard)
    {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
    
    //CoInitialize - COM/OLE
    XTESTC(hr = CoInitialize(NULL));

    //Initlialize Window Controls
    InitCommonControls();
    
    //Main Execution
    pCWizard->Display();
    
CLEANUP:
    if (pCWizard)
        delete pCWizard;

    CoUninitialize();
   
    return (hr==S_OK);
}


//////////////////////////////////////////////////////////////////
// void SyncSibling
//
//////////////////////////////////////////////////////////////////
void SyncSibling(HWND hToWnd, HWND hFromWnd)
{
    ASSERT(hToWnd && hFromWnd);

    //Make both windows synched, 
    //Get the current selection from the Source
    LONG iItem = (LONG)SendMessage(hFromWnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
    
    //Tell the Target to select the same selection
    if(iItem != LVM_ERR)
    {
        //Get the current selection from the Target and Unselect it
        LONG iOldItem = (LONG)SendMessage(hToWnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
        if(iItem != iOldItem)
        {
            //Unselect previous one
            LV_SetItemState(hToWnd, iOldItem, 0, 0, LVIS_SELECTED);

            //Select the new one
            LV_SetItemState(hToWnd, iItem, 0, LVIS_SELECTED, LVNI_SELECTED);

            //Ensure that it is visible
            SendMessage(hToWnd, LVM_ENSUREVISIBLE, (WPARAM)iItem, (LPARAM)FALSE);
        }
    }
}                


//////////////////////////////////////////////////////////////////
// int InternalAssert
//
//////////////////////////////////////////////////////////////////
int InternalAssert(					// 1 to break, 0 to skip.
    char*	pszExp,					// The expression causing assert
    char*	pszFile,				// The file name
    UINT	iLine					// Line number of assert
    )
{
    CHAR	szMsg[MAX_QUERY_LEN];
    StringCchPrintfA(szMsg, MAX_NAME_LEN, "Assertion Error!\n File '%s', Line '%lu'\n"
                    "Expression '%s'\n\n"
                    "Do you wish to Continue?  (Press 'OK' to ignore the assertion."
                    "  Press 'Cancel to debug.)",pszFile, iLine, pszExp);
    
    //Popup a MessageBox
    LONG dwSelection = MessageBoxA(NULL, szMsg,	"Microsoft OLE DB TableCopy - Error",
        MB_TASKMODAL | MB_ICONSTOP | MB_OKCANCEL | MB_DEFBUTTON1 );

    switch(dwSelection)
    {
        case IDOK:
            return 0;
        case IDCANCEL:
            return 1;
        default:
            ASSERT(!L"Unhandled Choice");
    }

    return 0;
}


//////////////////////////////////////////////////////////////////
// void InternalTrace
//
//////////////////////////////////////////////////////////////////
void InternalTrace(WCHAR*	pwszFmt, ...)
{
    va_list		marker;
    WCHAR		wszBuffer[MAX_NAME_LEN];
    CHAR		szBuffer[MAX_NAME_LEN];

    // Use format and arguements as input
    //This version will not overwrite the stack, since it only copies
    //upto the max size of the array
    va_start(marker, pwszFmt);
    _vsnwprintf_s(wszBuffer, MAX_NAME_LEN, _TRUNCATE, pwszFmt, marker);
    va_end(marker);

    //Make sure there is a NULL Terminator, vsnwprintf will not copy
    //the terminator if length==MAX_NAME_LEN
    wszBuffer[MAX_NAME_LEN-1] = EOL;
    
    //Convert to MBCS
    ConvertToMBCS(wszBuffer, szBuffer, MAX_NAME_LEN);
    
    //Output to the DebugWindow
    OutputDebugString(szBuffer);
}


//////////////////////////////////////////////////////////////////
// void InternalTrace
//
//////////////////////////////////////////////////////////////////
void InternalTrace(CHAR*	pszFmt, ...)
{
    va_list		marker;
    CHAR		szBuffer[MAX_NAME_LEN];

    // Use format and arguements as input
    //This version will not overwrite the stack, since it only copies
    //upto the max size of the array
    va_start(marker, pszFmt);
    _vsnprintf_s(szBuffer, MAX_NAME_LEN, _TRUNCATE, pszFmt, marker);
    va_end(marker);

    //Make sure there is a NULL Terminator, vsnwprintf will not copy
    //the terminator if length==MAX_NAME_LEN
    szBuffer[MAX_NAME_LEN-1] = '\0';
    
    OutputDebugStringA(szBuffer);
}


//////////////////////////////////////////////////////////////////
// void Busy
//
//////////////////////////////////////////////////////////////////
void Busy(BOOL bValue)
{
    static HCURSOR	hWaitCursor = LoadCursor(NULL, IDC_WAIT);

    if(bValue) 
        SetCursor(hWaitCursor);
    else 
        SetCursor(NULL);
}


//////////////////////////////////////////////////////////////////
// void OutOfMemory
//
//////////////////////////////////////////////////////////////////
void OutOfMemory(HWND hWnd)
{
    //Unicode version is supported on Win95/WinNT
    MessageBoxW(hWnd, L"Out of memory", wsz_ERROR, MB_TASKMODAL | MB_OK);
}


//////////////////////////////////////////////////////////////////
// BOOL CenterDialog
//
//////////////////////////////////////////////////////////////////
BOOL CenterDialog(HWND hdlg)
{
    RECT  rcParent;                         // Parent window client rect
    RECT  rcDlg;                            // Dialog window rect
    int   nLeft, nTop;                      // Top-left coordinates
    int   cWidth, cHeight;                  // Width and height
    HWND	hwnd;

    // Get frame window client rect in screen coordinates
    hwnd = GetParent(hdlg);
    if(hwnd == NULL || hwnd == GetDesktopWindow()) 
    {
        rcParent.top = rcParent.left = 0;
        rcParent.right = GetSystemMetrics(SM_CXFULLSCREEN);
        rcParent.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    }
    else 
        GetWindowRect(hwnd, &rcParent);

    // Determine the top-left point for the dialog to be centered
    GetWindowRect(hdlg, &rcDlg);
    cWidth  = rcDlg.right  - rcDlg.left;
    cHeight = rcDlg.bottom - rcDlg.top;
    nLeft   = rcParent.left + 
            (((rcParent.right  - rcParent.left) - cWidth ) / 2);
    nTop    = rcParent.top  +
            (((rcParent.bottom - rcParent.top ) - cHeight) / 2);
    if (nLeft < 0) nLeft = 0;
    if (nTop  < 0) nTop  = 0;

    // Place the dialog
    return MoveWindow(hdlg, nLeft, nTop, cWidth, cHeight, TRUE);
}


//////////////////////////////////////////////////////////////////
// ULONG wMessageBox
//
//////////////////////////////////////////////////////////////////
INT wMessageBox(
    HWND hwnd,							// Parent window for message display
    UINT uiStyle,						// Style of message box
    WCHAR* pwszTitle,					// Title for message
    WCHAR* pwszFmt,						// Format string
    ...									// Substitution parameters
    )
{
    va_list		marker;
    WCHAR		wszBuffer[MAX_QUERY_LEN];

    // Use format and arguements as input
    //This version will not overwrite the stack, since it only copies
    //upto the max size of the array
    va_start(marker, pwszFmt);
    _vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszFmt, marker);
    va_end(marker);
   
    //Make sure there is a NULL Terminator, vsnwprintf will not copy
    //the terminator if length==MAX_QUERY_LEN
    wszBuffer[MAX_QUERY_LEN-1] = EOL;

    //Unicode version is supported on both Win95 / WinNT do need to convert
    return MessageBoxW(hwnd, wszBuffer, pwszTitle, uiStyle);
}




//////////////////////////////////////////////////////////////////
// void wSetDlgItemText
//
//////////////////////////////////////////////////////////////////
void wSetDlgItemText(HWND hWnd, INT DlgItem, WCHAR* pwszFmt, ...)
{
    va_list		marker;
    WCHAR		wszBuffer[MAX_NAME_LEN];
    CHAR		szBuffer[MAX_NAME_LEN];

    // Use format and arguements as input
    //This version will not overwrite the stack, since it only copies
    //upto the max size of the array
    va_start(marker, pwszFmt);
    _vsnwprintf_s(wszBuffer, MAX_NAME_LEN, _TRUNCATE, pwszFmt, marker);
    va_end(marker);

    //Make sure there is a NULL Terminator, vsnwprintf will not copy
    //the terminator if length==MAX_NAME_LEN
    wszBuffer[MAX_NAME_LEN-1] = EOL;

    //convert to MBCS
    ConvertToMBCS(wszBuffer, szBuffer, MAX_NAME_LEN);
    
    SetDlgItemTextA(hWnd, DlgItem, szBuffer);
}


//////////////////////////////////////////////////////////////////
// UINT wGetDlgItemText
//
//////////////////////////////////////////////////////////////////
UINT wGetDlgItemText(HWND hWnd, INT DlgItem, WCHAR* pwsz, INT nMaxSize)
{
    ASSERT(pwsz);
    CHAR szBuffer[MAX_NAME_LEN];

    UINT iReturn = GetDlgItemTextA(hWnd, DlgItem, szBuffer, MAX_NAME_LEN);

    //convert to WCHAR
    ConvertToWCHAR(szBuffer, pwsz, MAX_NAME_LEN);
    return iReturn;
}



//////////////////////////////////////////////////////////////////
// LRESULT wSendMessage
//
//////////////////////////////////////////////////////////////////
LRESULT wSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszBuffer)
{
    CHAR szBuffer[MAX_NAME_LEN];						  
    szBuffer[0] = '\0';
    
    if(pwszBuffer && Msg != WM_GETTEXT && Msg != LVM_GETITEM && Msg != CB_GETLBTEXT)
    {
        //Convert to ANSI before sending, since we don't know if this was a GET/SET message
        ConvertToMBCS(pwszBuffer, szBuffer, MAX_NAME_LEN);
    }

    //Send the message with an ANSI Buffer 
    LRESULT lResult = SendMessageA(hWnd, Msg, (WPARAM)wParam, (LPARAM)szBuffer);

    if(pwszBuffer && Msg == WM_GETTEXT || Msg == LVM_GETITEM || Msg == CB_GETLBTEXT)
    {
        //Now convert the result into the users WCHAR buffer
        ConvertToWCHAR(szBuffer, pwszBuffer, MAX_NAME_LEN);
    }
    return lResult;
}


//////////////////////////////////////////////////////////////////
// BOOL GetEditBoxValue
//
//////////////////////////////////////////////////////////////////
BOOL GetEditBoxValue(HWND hEditWnd, ULONG ulMin, ULONG ulMax, ULONG* pulCount)
{
    ASSERT(hEditWnd);
    ASSERT(pulCount);

    ULONG	ulCount = 0;
    WCHAR	wszBuffer[MAX_NAME_LEN];
    WCHAR*  pwszEnd = NULL;
    
    //Get the EditText
    wSendMessage(hEditWnd, WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
        
    //Convert to ULONG
    ulCount = wcstoul(wszBuffer, &pwszEnd, 10);
    if(!wszBuffer[0] || ulCount<ulMin || ulCount>ulMax || pwszEnd==NULL || pwszEnd[0]!=EOL) 
    {
        wMessageBox(hEditWnd, MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK, 	wsz_ERROR, 
            wsz_INVALID_VALUE_, wszBuffer, ulMin, ulMax);
        SetFocus(hEditWnd);
        return FALSE;
    }

    *pulCount = ulCount;
    return TRUE;
}


//////////////////////////////////////////////////////////////////
// LONG LV_InsertColumn
//
//////////////////////////////////////////////////////////////////
LONG LV_InsertColumn(HWND hWnd, LONG iColumn, CHAR* szName)
{
    //Setup LV_COLUMNINFO
    LV_COLUMN lvColumnHeader = { LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM, LVCFMT_LEFT, 0, szName, MAX_NAME_LEN, 0};
    
    //LVM_INSERTCOLUMN
    return (LONG)SendMessage(hWnd, LVM_INSERTCOLUMN, (WPARAM)iColumn, (LPARAM)&lvColumnHeader);
}


//////////////////////////////////////////////////////////////////
// LONG LV_InsertItem
//
//////////////////////////////////////////////////////////////////
LONG LV_InsertItem(HWND hWnd, LONG iItem, LONG iSubItem, CHAR* szName, LONG iParam, LONG iImage)
{
    //LVM_INSERTITEM
    if(iSubItem==0)
    {
        LV_ITEM lvItem = { LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, iItem, iSubItem, 0, 0, szName, 0, iImage, iParam};
        return (LONG)SendMessage(hWnd, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvItem);
    }
    //LVM_SETITEM
    else
    {
        LV_ITEM lvItem = { LVIF_TEXT, iItem, iSubItem, 0, 0, szName, 0, 0, iParam};
        return (LONG)SendMessage(hWnd, LVM_SETITEM, (WPARAM)0, (LPARAM)&lvItem);
    }
}


//////////////////////////////////////////////////////////////////
// LONG LV_SetItemState
//
//////////////////////////////////////////////////////////////////
LONG LV_SetItemState(HWND hWnd, LONG iItem, LONG iSubItem, LONG lState, LONG lStateMask)
{
    //LVM_SETITEM
    LV_ITEM lvItem = { LVIF_STATE, iItem, iSubItem, lState, lStateMask, NULL, 0, 0, 0};
    return (LONG)SendMessage(hWnd, LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&lvItem);
}


//////////////////////////////////////////////////////////////////
// LONG LV_SetItemText
//
//////////////////////////////////////////////////////////////////
LONG LV_SetItemText(HWND hWnd, LONG iItem, LONG iSubItem, CHAR* szName)
{
    //LVM_SETITEM
    LV_ITEM lvItem = { LVIF_TEXT, iItem, iSubItem, 0, 0, szName, 0, 0, 0};
    return (LONG)SendMessage(hWnd, LVM_SETITEMTEXT, (WPARAM)iItem, (LPARAM)&lvItem);
}


//////////////////////////////////////////////////////////////////
// LONG LV_FindItem
//
//////////////////////////////////////////////////////////////////
LONG LV_FindItem(HWND hWnd, CHAR* szName, LONG iStart)
{
    //LVM_FINDITEM
    LV_FINDINFO lvFindInfo = { LVFI_STRING, szName, 0, 0, 0};
    return (LONG)SendMessage(hWnd, LVM_FINDITEM, (WPARAM)iStart, (LPARAM)&lvFindInfo);
}


//////////////////////////////////////////////////////////////////
// HTREEITEM TV_InsertItem
//
//////////////////////////////////////////////////////////////////
HTREEITEM TV_InsertItem(HWND hWnd, HTREEITEM hParent, HTREEITEM hInsAfter, CHAR* szName, LONG iParam, LONG iImage, LONG iSelectedImage)
{
    TV_INSERTSTRUCT tvInsertStruct = { hParent, hInsAfter, { TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM, 0, 0, 0, szName, 0, iImage, iSelectedImage, 0, iParam} };
    
    //TVM_INSERTITEM
    return (HTREEITEM)SendMessage(hWnd, TVM_INSERTITEM, (WPARAM)0, (LPARAM)&tvInsertStruct);
}


