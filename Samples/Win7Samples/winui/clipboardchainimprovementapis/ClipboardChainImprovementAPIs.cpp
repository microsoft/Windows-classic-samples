///////////////////////////////////////////////////////////////////////////////
//
// ClipboardChainImprovementAPIs.cpp
//
// This shows a sample usage of the following APIs:
//      AddClipboardFormatListener
//      RemoveClipboardFormatListener
//      GetUpdatedClipboardFormats
//
// History:
//      2/9/2006 -- a-erical -- created
//
///////////////////////////////////////////////////////////////////////////////

#pragma prefast(suppress: 309, _T("NULL is valid in call to GetUpdatedClipboardFormats"))
#pragma prefast(suppress: 387, _T("0 is valid in call to GetUpdatedClipboardFormats"))

#include "stdafx.h"
#include "ClipboardChainImprovementAPIs.h"

VOID ClipboardSample(
    __in HWND hWindow)
{    
    //
    // Call GetUpdatedClipboardFormats to get the number of clipboard formats,
    // without getting the clipboard formats themselves
    //
    UINT numberOfClipboardFormats = 0;
    GetUpdatedClipboardFormats(NULL, 0, &numberOfClipboardFormats);
    _tprintf(_T("GetUpdatedClipboardFormats: initial number of clipboard formats: %d\n\n"), 
        numberOfClipboardFormats);
    
    //
    // Add the sample window to the list of clipboard format listeners
    //
    if (AddClipboardFormatListener(hWindow))
    {
        _tprintf(_T("AddClipboardFormatListener: Sample window added to list of clipboard format listeners\n\n"));
    }

    //
    // Add bitmap data to the clipboard, which will generate a WM_CLIPBOARDUPDATE message
    //
    AddBitmapDataToClipboard(hWindow);
    
    //
    // Peek for a WM_CLIPBOARDUPDATE message
    //
    MSG message = {0};
    PeekMessage(&message, hWindow, WM_CLIPBOARDUPDATE, WM_CLIPBOARDUPDATE, PM_REMOVE);
    if (message.message == WM_CLIPBOARDUPDATE)
    {
        _tprintf(_T("Sample window received WM_CLIPBOARDUPDATE message\n\n"));
    }

    //
    // Call GetUpdatedClipboardFormats, getting the list of formats
    //
    UINT* clipboardFormats = new UINT[numberOfClipboardFormats + 10];
    if (GetUpdatedClipboardFormats(
        clipboardFormats, 
        numberOfClipboardFormats + 10, 
        &numberOfClipboardFormats))
    {
        _tprintf(_T("GetUpdatedClipboardFormats: number of clipboard formats written: %d\n\n"),
            numberOfClipboardFormats);
    }

    //
    // Remove the sample window from the list of clipboard format listeners
    //
    if (RemoveClipboardFormatListener(hWindow))
    {
        _tprintf(_T("RemoveClipboardFormatListener: sample window removed from list of clipboard format change listeners\n\n"));
    }

    //
    // Add bitmap data to the clipboard, which will generate a WM_CLIPBOARDUPDATE message
    //
    AddBitmapDataToClipboard(hWindow);

    //
    // Peek for a WM_CLIPBOARDUPDATE message
    //
    message.message = 0;
    PeekMessage(&message, hWindow, WM_CLIPBOARDUPDATE, WM_CLIPBOARDUPDATE, PM_REMOVE);
    if (message.message != WM_CLIPBOARDUPDATE)
    {
        _tprintf(_T("Sample window did NOT receive the WM_CLIPBOARDUPDATE message\n\n"));
    }

    DestroyWindow(hWindow);
	delete [] clipboardFormats;
}

INT _cdecl _tmain(INT argc, __in_ecount(argc) TCHAR* argv[])
{
    //
    // Create the sample window to use with the clipboard functions
    //
    HWND hWindow = CreateWindow(
        _T("Edit"),
        _T("Sample Window"),
        WS_VISIBLE | WS_OVERLAPPED,
        100,
        100,
        100,
        100,
        NULL,
        NULL,
        NULL,
        NULL);

    if (!hWindow)
    {
        _tprintf(_T("Cannot create sample window\n"));
    }
    else
    {
        ClipboardSample(hWindow);
    }

    return 0;
}

BOOL AddBitmapDataToClipboard(
    __in HWND hWindow)
{
    BOOL result = TRUE;

    HBITMAP hBitmap = NULL;
    
    if (!OpenClipboard(hWindow))
    {
        _tprintf(_T("Cannot open clipboard\n"));
        result = FALSE;
    }

    if (result)
    {
        hBitmap = LoadBitmap(
            NULL,
            MAKEINTRESOURCE(OBM_CHECK));

        if (!hBitmap)
        {
            _tprintf(_T("Cannot load bitmap\n"));
            result = FALSE;
        }
    }

    if (result)
    {
        if (!SetClipboardData(CF_BITMAP, hBitmap))
        {
            _tprintf(_T("Cannot set clipboard data\n"));
            result = FALSE;
        }
    }

    if (result)
    {
        if (!CloseClipboard())
        {
            _tprintf(_T("Cannot close clipboard\n"));
            result = FALSE;
        }
    }

    if (result)
    {
        _tprintf(_T("Loaded bitmap data onto the clipboard... WM_CLIPBOARDUPDATE message generated\n"));
    }

    return result;
}
