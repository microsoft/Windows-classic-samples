// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "TasksPropPage.h"

CTasksPropertyPage::CTasksPropertyPage()
{
    m_dwTitleID     = IDS_TITLE;
    m_dwHelpFileID  = IDS_HELPFILE;
    m_dwDocStringID = IDS_DOCSTRING;
}

//
// Store all the settings back to tasks.ini file
//
HRESULT CTasksPropertyPage::Apply()
{
    WritePrivateProfileString(TASK_SECTION, SHOW_WORK_TASKS, (BST_CHECKED == IsDlgButtonChecked(IDC_WORKTASKS)) ? L"1" : L"0", (LPCWSTR)GetConfigFile());
    WritePrivateProfileString(TASK_SECTION, SHOW_FAMILY_TASKS, (BST_CHECKED == IsDlgButtonChecked(IDC_FAMILYTASKS)) ? L"1" : L"0", (LPCWSTR)GetConfigFile());

    return S_OK;
}

//
// Initial rendering of the property dialog
//
HRESULT CTasksPropertyPage::Show(UINT nCmdShow)
{
    WCHAR wszEnabled[2] = {0};

    GetPrivateProfileString(TASK_SECTION, SHOW_WORK_TASKS, L"0", wszEnabled, 2, (LPCWSTR)GetConfigFile());
    CheckDlgButton(IDC_WORKTASKS, (0 == wcscmp(wszEnabled, L"0")) ? BST_UNCHECKED : BST_CHECKED);
    GetPrivateProfileString(TASK_SECTION, SHOW_FAMILY_TASKS, L"0", wszEnabled, 2, (LPCWSTR)GetConfigFile());
    CheckDlgButton(IDC_FAMILYTASKS, (0 == wcscmp(wszEnabled, L"0")) ? BST_UNCHECKED : BST_CHECKED);

    ShowWindow(nCmdShow);
    InvalidateRect(NULL,TRUE);

    return NOERROR;
}