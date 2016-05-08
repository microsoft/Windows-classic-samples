// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
// RichEditMng.h/cpp implement the utility functions that are used for interacting 
// with the RichEdit control. 
//

#pragma once

#include <UIRibbon.h>
#include <Richedit.h>

#define MAX_LOADSTRING 100

// Class for managing the sample application RichEdit control.
class CFCSampleAppRichEditManager
{
public:
    
    CFCSampleAppRichEditManager(HWND hWnd, HINSTANCE hInst);

    HWND GetRichEditWnd(){return m_hWndEdit;} // Return RichEdit window handle.
    HRESULT SetHeight(UINT uHeight); // Set height of the ribbon.
    HRESULT Resize(); // Resize the RichEdit when height changes.
    
    void SetValues(__in IPropertyStore *pps); // Set values for font displayed in the selection.
    void GetValues(__in IPropertyStore *pps); // Get values for font displayed in the selection.
    void SetPreviewValues(__in IPropertyStore *pps); // Set preview font values.
    void CancelPreview(__in IPropertyStore *pps); // Cancel preview by reverting the values.

    void ShowSelection(); // Show the selection of text.

private:
    HWND m_hwnd; // Handle of the main window.
    HWND m_hWndEdit; // Handle of the RichEdit control.
    CHARFORMAT2 m_charDefaultFormat; // Default font to use in the RichEdit control.
    UINT m_uHeight; // Height of the ribbon.
    HINSTANCE m_hInst;

    HRESULT _CreateRichEdit(); // Create the RichEdit control.
};

