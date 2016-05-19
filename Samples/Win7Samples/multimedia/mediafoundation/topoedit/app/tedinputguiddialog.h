// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "resource.h"

class CTedInputGuidDialog : public CDialogImpl<CTedInputGuidDialog>
{
public:
    enum { IDD = IDD_INPUTGUID };

    GUID GetInputGuid() const;
    bool IsValidGuid() const { return m_fIsValidGuid; }
protected:
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    BEGIN_MSG_MAP( CTedTransformDialog )
        COMMAND_HANDLER(IDCANCEL, 0, OnCancel)
        COMMAND_HANDLER(IDOK, 0, OnOK)
    END_MSG_MAP()

private:
    GUID m_InputGUID;
    bool m_fIsValidGuid;
    static const DWORD m_dwGUIDLength = 40;
};
