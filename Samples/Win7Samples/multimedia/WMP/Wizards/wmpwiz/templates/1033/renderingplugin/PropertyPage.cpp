/////////////////////////////////////////////////////////////////////////////
//
// C[!output Safe_root]PropPage.cpp : Implementation of the property page for C[!output Safe_root]
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include "[!output root].h"
#include "[!output Root]PropPage.h"

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::C[!output Safe_root]PropPage
// Constructor

C[!output Safe_root]PropPage::C[!output Safe_root]PropPage()
{
    m_dwTitleID = IDS_TITLEPROPPAGE;
    m_dwHelpFileID = IDS_HELPFILEPROPPAGE;
    m_dwDocStringID = IDS_DOCSTRINGPROPPAGE;
    m_Color = rgbBlue;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::~C[!output Safe_root]PropPage
// Destructor

C[!output Safe_root]PropPage::~C[!output Safe_root]PropPage()
{
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::SetObjects
//

STDMETHODIMP C[!output Safe_root]PropPage::SetObjects(ULONG nObjects, IUnknown** ppUnk)
{
    if( NULL == ppUnk )
    {
        return E_POINTER;
    }

    // find our plug-in object, if it was passed in
    for (DWORD i = 0; i < nObjects; i++)
    {
        CComPtr<I[!output Safe_root]> pPlugin;

        IUnknown    *pUnknown = ppUnk[i];
        if (pUnknown)
        {
            HRESULT hr = pUnknown->QueryInterface(__uuidof(I[!output Safe_root]), (void**)&pPlugin); // Get a pointer to the plug-in.
            if ((SUCCEEDED(hr)) && (pPlugin))
            {
                // save plug-in interface
                m_sp[!output Safe_root] = pPlugin;
                break;
            }
        }
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::Apply
//

STDMETHODIMP C[!output Safe_root]PropPage::Apply(void)
{
    // update the registry
    CRegKey key;
    LONG    lResult;

    lResult = key.Create(HKEY_CURRENT_USER, kwszPrefsRegKey);
    if (ERROR_SUCCESS == lResult)
    {
[!if VSNET]
        DWORD dwValue = (DWORD) m_Color;
        lResult = key.SetValue(kwszPrefsTextColor, REG_DWORD, &dwValue, sizeof(dwValue));
[!else]
        lResult = key.SetValue((DWORD) m_Color, kwszPrefsTextColor );
[!endif]
    }

    // update the plug-in
    if (m_sp[!output Safe_root])
    {
        m_sp[!output Safe_root]->put_color(m_Color);
    }   

    m_bDirty = FALSE; // Tell the property page to disable Apply.
    
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::OnClickedBlue
//
LRESULT C[!output Safe_root]PropPage::OnClickedBlue(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled){

    m_Color = rgbBlue;
    SetDirty(TRUE); //Enable Apply.

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::OnClickedGreen
//
LRESULT C[!output Safe_root]PropPage::OnClickedGreen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled){

    m_Color = rgbGreen;
    SetDirty(TRUE); //Enable Apply.

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::OnClickedRed
//
LRESULT C[!output Safe_root]PropPage::OnClickedRed(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled){

    m_Color = rgbRed;
    SetDirty(TRUE); //Enable Apply.

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]PropPage::OnInitDialog
//

LRESULT C[!output Safe_root]PropPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    COLORREF Color = rgbBlue;

    // read color from plug-in if it is available
    if (m_sp[!output Safe_root])
    {
        m_sp[!output Safe_root]->get_color(&Color);
    }   
    else // otherwise read color from registry
    {
        CRegKey key;
        LONG    lResult;

        lResult = key.Open(HKEY_CURRENT_USER, kwszPrefsRegKey, KEY_READ);
        if (ERROR_SUCCESS == lResult)
        {
            DWORD   dwValue = 0;
[!if VSNET]
            DWORD dwType = 0;
            ULONG uLength = sizeof(dwValue);
            lResult = key.QueryValue(kwszPrefsTextColor, &dwType, &dwValue, &uLength);
[!else]
            lResult = key.QueryValue(dwValue, kwszPrefsTextColor );
[!endif]
            if (ERROR_SUCCESS == lResult)
            {
                Color = (COLORREF) dwValue;
            }
        }
    }

    // Bullet the correct radio button. 
    switch(Color)
    {
    case rgbGreen:
        SendDlgItemMessage(IDC_GREEN, BM_CLICK, NULL, NULL);

        break;

    case rgbRed:
        SendDlgItemMessage(IDC_RED, BM_CLICK, NULL, NULL);

        break;

    case rgbBlue:
    default:
        SendDlgItemMessage(IDC_BLUE, BM_CLICK, NULL, NULL);

        break;
    }

    return 0;
}

