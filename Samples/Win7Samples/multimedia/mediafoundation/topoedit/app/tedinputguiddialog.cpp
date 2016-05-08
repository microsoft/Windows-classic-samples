// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tedinputguiddialog.h"
#include <strsafe.h>

GUID CTedInputGuidDialog::GetInputGuid() const
{
    return m_InputGUID;
}

LRESULT CTedInputGuidDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    USES_CONVERSION;
    
    HWND m_hEdit = GetDlgItem(IDC_INPUTGUID);

    LPTSTR strGuid = new TCHAR[m_dwGUIDLength];
    ::GetWindowText(m_hEdit, strGuid, m_dwGUIDLength);

    m_InputGUID = GUID_NULL;
    m_fIsValidGuid = true;

    size_t cchGuid;
    HRESULT hr = StringCchLength(strGuid, m_dwGUIDLength, &cchGuid);

    if(SUCCEEDED(hr) && cchGuid > 0)
    {
        hr = CLSIDFromString(CT2OLE(strGuid), &m_InputGUID);
        if(FAILED(hr))
        {
            CAtlStringW strerr;
            strerr.FormatMessage(IDS_E_GUID, hr);
            MessageBox(strerr, strerr, MB_OK);
            m_fIsValidGuid = false;
        }
    }
    
    delete[] strGuid;

    EndDialog(IDOK);
    
    return 0;
}

LRESULT CTedInputGuidDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDCANCEL);

    return 0;
}

