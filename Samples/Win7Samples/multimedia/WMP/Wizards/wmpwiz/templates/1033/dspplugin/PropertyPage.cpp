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
// C[!output Safe_root]Prop::C[!output Safe_root]Prop
// Constructor

C[!output Safe_root]PropPage::C[!output Safe_root]PropPage()
{
    m_dwTitleID = IDS_TITLEPROPPAGE;
    m_dwHelpFileID = IDS_HELPFILEPROPPAGE;
    m_dwDocStringID = IDS_DOCSTRINGPROPPAGE;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]Prop::~C[!output Safe_root]Prop
// Destructor

C[!output Safe_root]PropPage::~C[!output Safe_root]PropPage()
{
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]Prop::SetObjects
//

STDMETHODIMP C[!output Safe_root]PropPage::SetObjects(ULONG nObjects, IUnknown** ppUnk)
{
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
// C[!output Safe_root]Prop::Apply
//

STDMETHODIMP C[!output Safe_root]PropPage::Apply(void)
{
    WCHAR   wszStr[MAXSTRING] = { 0 };
    double  fScaleFactor = 1.0;

    GetDlgItemText(IDC_SCALEFACTOR, wszStr, sizeof(wszStr) / sizeof(wszStr[0]));
    swscanf_s(wszStr, L"%lf", &fScaleFactor);    

    // make sure scale factor is valid
    if ((fScaleFactor < 0.0) ||
        (fScaleFactor > 1.0))
    {
        if (::LoadString(_Module.GetResourceInstance(), IDS_SCALERANGEERROR, wszStr, sizeof(wszStr) / sizeof(wszStr[0])))
        {
            MessageBox(wszStr);
        }

        return E_FAIL;
    }

    // update the registry
    CRegKey key;
    LONG    lResult;

    lResult = key.Create(HKEY_CURRENT_USER, kwszPrefsRegKey);
    if (ERROR_SUCCESS == lResult)
    {
[!if VSNET]
        DWORD dwValue = (DWORD) (fScaleFactor * 65536);
        lResult = key.SetValue(kwszPrefsScaleFactor, REG_DWORD, &dwValue, sizeof(dwValue));
[!else]
        lResult = key.SetValue((DWORD) (fScaleFactor * 65536), kwszPrefsScaleFactor );
[!endif]
    }

    // update the plug-in
    if (m_sp[!output Safe_root])
    {
        m_sp[!output Safe_root]->put_scale(fScaleFactor);
    }   

    m_bDirty = FALSE; // Tell the property page to disable Apply.
    
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]Prop::OnChangeScale
//

LRESULT C[!output Safe_root]PropPage::OnChangeScale(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled){

    SetDirty(TRUE); // Enable Apply.
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]Prop::OnInitDialog
//

LRESULT C[!output Safe_root]PropPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    double  fScaleFactor = 1.0;

    // read scale factor from plug-in if it is available
    if (m_sp[!output Safe_root])
    {
        m_sp[!output Safe_root]->get_scale(&fScaleFactor);
    }   
    else // otherwise read scale factor from registry
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
            lResult = key.QueryValue(kwszPrefsScaleFactor, &dwType, &dwValue, &uLength);
[!else]
            lResult = key.QueryValue(dwValue, kwszPrefsScaleFactor );
[!endif]

            if (ERROR_SUCCESS == lResult)
            {
                fScaleFactor = dwValue / 65536.0;
            }
        }
    }

    WCHAR   wszStr[MAXSTRING];

    swprintf_s(wszStr, L"%0.2f", fScaleFactor);  
    SetDlgItemText(IDC_SCALEFACTOR, wszStr);

    return 0;
}

