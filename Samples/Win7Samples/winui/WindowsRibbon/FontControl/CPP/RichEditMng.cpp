// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "RichEditMng.h"
#include "PropertyStore.h"
#include <strsafe.h>
#include "resource.h"

CFCSampleAppRichEditManager::CFCSampleAppRichEditManager(HWND hWnd, HINSTANCE hInst) : m_hwnd(hWnd)
                                                                             , m_hInst(hInst)
                                                                             , m_uHeight(0)
                                                                             , m_hWndEdit(NULL)
{
    ZeroMemory(&m_charDefaultFormat, sizeof(m_charDefaultFormat));
    m_charDefaultFormat.cbSize = sizeof(CHARFORMAT2);

    // Don't change these properties
    m_charDefaultFormat.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_SUPERSCRIPT | CFM_SUBSCRIPT;

    // Change these.
    m_charDefaultFormat.dwMask |= CFM_FACE | CFM_SIZE | CFM_COLOR | CFM_BACKCOLOR;
    m_charDefaultFormat.dwEffects |= CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR;
    
    WCHAR wszDefaultFont[MAX_LOADSTRING] = {0};
    LoadString(m_hInst, IDS_DEFAULTTEXTFONT, wszDefaultFont, MAX_LOADSTRING);
    
    StringCchCopyW(m_charDefaultFormat.szFaceName, ARRAYSIZE(m_charDefaultFormat.szFaceName), wszDefaultFont);
    m_charDefaultFormat.yHeight = (LONG)(24 * TWIPS_PER_POINT);
}

// Set current height of the ribbon.
HRESULT CFCSampleAppRichEditManager::SetHeight(UINT uHeight)
{
    m_uHeight = uHeight;

    // Resize the RichEdit control.
    return Resize();
}

// Create RichEdit control for text formatting.
HRESULT CFCSampleAppRichEditManager::_CreateRichEdit()
{
    HRESULT hr = E_FAIL;
    HMODULE hModule = LoadLibrary(L"Riched20.dll");
    if (NULL != hModule)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        m_hWndEdit = CreateWindowEx(0, RICHEDIT_CLASS, L"",
            ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, 
            0, m_uHeight, rc.right, rc.bottom - m_uHeight, 
            m_hwnd, NULL, m_hInst, NULL);
        if (m_hWndEdit)
        {
            SendMessage(m_hWndEdit, (UINT) EM_SETEVENTMASK, 0, (LPARAM) ENM_SELCHANGE);
            SendMessage(m_hWndEdit, (UINT) EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM)&m_charDefaultFormat);

            hr =S_OK;
        }
        else
        {
            FreeLibrary(hModule);
        }
    }

    return hr;
}

// Resize the RichEdit control.
HRESULT CFCSampleAppRichEditManager::Resize()
{
    HRESULT hr = S_OK;
    if (m_uHeight != 0)
    {
        // Make sure the RichEdit control covers the client area left by the ribbon.
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        if (m_hWndEdit == NULL)
        {
            // Control hasn't been created yet so create it using the height.
            hr = _CreateRichEdit();
        }
        else
        {
            // Resize the control.
            if(!SetWindowPos (m_hWndEdit, 0, 0, 0, rc.right, rc.bottom - m_uHeight, SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER))
            {
                hr = E_FAIL;
            }
        }
    }
    return hr;
}

// Set the values for the font to use for the selection in the RichEdit control.
void CFCSampleAppRichEditManager::SetValues(__in IPropertyStore *pps)
{
    if (m_hWndEdit)
    {
        CHARFORMAT2 charFormat;
        GetCharFormat2FromIPropertyStore(pps, &charFormat);
    
        SendMessage(m_hWndEdit, (UINT) EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM)&charFormat);
    }
    else
    {
        GetCharFormat2FromIPropertyStore(pps, &m_charDefaultFormat);
    }
}

// Get the values for the font used for the selection in the RichEdit control.
void CFCSampleAppRichEditManager::GetValues(__in IPropertyStore *pps)
{
    if (m_hWndEdit)
    {
        CHARFORMAT2 charFormat;
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(CHARFORMAT2);
        SendMessage(m_hWndEdit, (UINT) EM_GETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM)&charFormat);
        GetIPropStoreFromCharFormat2(&charFormat, pps);
    }
}

// Preview the given font values for the selection in the RichEdit control.
void CFCSampleAppRichEditManager::SetPreviewValues(__in IPropertyStore *pps)
{
    CHARFORMAT2 charFormat;
    GetCharFormat2FromIPropertyStore(pps, &charFormat);
    
    SendMessage(m_hWndEdit, (UINT) EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM)&charFormat);
}
     
// Cancel preview for the selection in the RichEdit control.
void CFCSampleAppRichEditManager::CancelPreview(__in IPropertyStore *pps)
{
    SetPreviewValues(pps);
}

// Show the selection of text.
void CFCSampleAppRichEditManager::ShowSelection()
{
    SendMessage(m_hWndEdit, (UINT) EM_HIDESELECTION, (WPARAM) 0, 0);
}
