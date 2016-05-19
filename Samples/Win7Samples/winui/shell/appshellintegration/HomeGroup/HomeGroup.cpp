// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This sample demonstrates how to search the HomeGroup Namespace using the ISearchFolderItemFactory interface.

#define STRICT_TYPED_ITEMIDS
#include <windows.h>
#include <shlobj.h>
#include "Resource.h"

// Since this sample displays UI, set the manifest to pick the right version of comctl32.dll
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WM_HOMEGROUP_CHANGED (WM_USER + 1)

class CHomeGroupUsersDialog
{
public:
    CHomeGroupUsersDialog();
    void ShowDialog(HINSTANCE hInstance);

private:
    void _Initialize(HWND hwndDlg);
    void _FillUserList(HWND hwndList);
    void _ShowSharingWizard(HWND hwndDlg);
    static INT_PTR CALLBACK s_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM);

    ULONG _ulChangeNotify;
    BOOL  _fCurrentlyFillingUserList;
    BOOL  _fNeedToRefillUserList;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CHomeGroupUsersDialog dlg;
        dlg.ShowDialog(hInstance);
        CoUninitialize();
    }
    return 0;
}

CHomeGroupUsersDialog::CHomeGroupUsersDialog() : _ulChangeNotify(NULL),
                                                 _fCurrentlyFillingUserList(FALSE),
                                                 _fNeedToRefillUserList(FALSE)
{
}

void CHomeGroupUsersDialog::ShowDialog(HINSTANCE hInstance)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, s_DialogProc, (LPARAM)this);
}

void CHomeGroupUsersDialog::_Initialize(HWND hwndDlg)
{
    PIDLIST_ABSOLUTE pidlHomeGroup;
    if (SUCCEEDED(SHGetKnownFolderIDList(FOLDERID_HomeGroup, KF_FLAG_DONT_VERIFY, NULL, &pidlHomeGroup)))
    {
        SHChangeNotifyEntry notifyEntry;
        notifyEntry.pidl = pidlHomeGroup;
        notifyEntry.fRecursive = FALSE;

        _ulChangeNotify = SHChangeNotifyRegister(hwndDlg,
            SHCNRF_InterruptLevel | SHCNRF_ShellLevel,
            SHCNE_MKDIR | SHCNE_RMDIR | SHCNE_UPDATEDIR,
            WM_HOMEGROUP_CHANGED,
            1,
            &notifyEntry);

        ILFree(pidlHomeGroup);
    }

    _FillUserList(hwndDlg);
}

void CHomeGroupUsersDialog::_FillUserList(HWND hwndDlg)
{
    // Avoids re-entrancy
    if (_fCurrentlyFillingUserList)
    {
        _fNeedToRefillUserList = TRUE;
    }
    else
    {
        _fNeedToRefillUserList = FALSE;
        _fCurrentlyFillingUserList = TRUE;

        // Finds the HomeGroup Users ListBox Control
        HWND hwndList = GetDlgItem(hwndDlg, IDC_USERLIST);
        if (hwndList)
        {
            SendMessage(hwndList, LB_RESETCONTENT, NULL, NULL);

            // Checks whether the computer is joined to a homegroup
            IHomeGroup *pHomeGroup;
            if (SUCCEEDED(CoCreateInstance(CLSID_HomeGroup, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pHomeGroup))))
            {
                BOOL fIsMember;
                if (SUCCEEDED(pHomeGroup->IsMember(&fIsMember)))
                {
                    // Finds the Change Sharing Button and enable/disable it as necessary
                    HWND hwndButton = GetDlgItem(hwndDlg, IDC_CHANGESHARING);
                    if (hwndButton)
                    {
                        EnableWindow(hwndButton, fIsMember);
                    }

                    if (fIsMember)
                    {
                        // Gets the HomeGroup node
                        IShellItem *psiHomeGroup;
                        if (SUCCEEDED(SHGetKnownFolderItem(FOLDERID_HomeGroup, KF_FLAG_DEFAULT, NULL, IID_PPV_ARGS(&psiHomeGroup))))
                        {
                            // Gets the list of HomeGroup users
                            IEnumShellItems *penumHomeGroupUsers;
                            if (SUCCEEDED(psiHomeGroup->BindToHandler(NULL, BHID_EnumItems, IID_PPV_ARGS(&penumHomeGroupUsers))))
                            {
                                // Adds each HomeGroup user to the ListBox
                                IShellItem *psiUser;
                                while (S_OK == penumHomeGroupUsers->Next(1, &psiUser, NULL))
                                {
                                    PWSTR pszUserName;
                                    if (SUCCEEDED(psiUser->GetDisplayName(SIGDN_PARENTRELATIVE, &pszUserName)))
                                    {
                                        SendMessage(hwndList, LB_ADDSTRING, NULL, (LPARAM)pszUserName);
                                        CoTaskMemFree(pszUserName);
                                    }
                                    psiUser->Release();
                                }
                                penumHomeGroupUsers->Release();
                            }
                            psiHomeGroup->Release();
                        }
                    }
                    else
                    {
                        SendMessage(hwndList, LB_ADDSTRING, NULL, (LPARAM)L"Not joined to a HomeGroup");
                    }
                }
                pHomeGroup->Release();
            }
        }

        _fCurrentlyFillingUserList = FALSE;
        if (_fNeedToRefillUserList)
        {
            _FillUserList(hwndDlg);
        }
     }
}

void CHomeGroupUsersDialog::_ShowSharingWizard(HWND hwndDlg)
{
    IHomeGroup *pHomeGroup;
    if (SUCCEEDED(CoCreateInstance(CLSID_HomeGroup, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pHomeGroup))))
    {
        HOMEGROUPSHARINGCHOICES sharingchoices;

        // ShowSharingWizard returns E_UNEXPECTED if sharing is not allowed from this computer
        // This could be due to a variety of reasons including:
        // > The computer is not joined to a homegroup
        // > The computer is domain-joined
        // > The computer has no network connectivity
        if (E_UNEXPECTED == pHomeGroup->ShowSharingWizard(hwndDlg, &sharingchoices))
        {
            MessageBox(hwndDlg, L"Sharing to the homegoup is not allowed from this computer.", L"Sharing Not Allowed", MB_OK | MB_ICONERROR | MB_APPLMODAL);
        }
        pHomeGroup->Release();
    }
}

INT_PTR CALLBACK CHomeGroupUsersDialog::s_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = FALSE;
    CHomeGroupUsersDialog *pThis = reinterpret_cast<CHomeGroupUsersDialog*>(GetWindowLongPtr(hwndDlg, DWLP_USER));
    if (uMsg == WM_INITDIALOG)
    {
        pThis = reinterpret_cast<CHomeGroupUsersDialog*>(lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pThis));
    }

    if (pThis)
    {
        switch (uMsg)
        {
        case WM_INITDIALOG:
            pThis->_Initialize(hwndDlg);
            fHandled = TRUE;
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDC_CHANGESHARING:
                pThis->_ShowSharingWizard(hwndDlg);
                fHandled = TRUE;
                break;

            case IDCANCEL:
                EndDialog(hwndDlg, NULL);
                fHandled = TRUE;
                break;
            }
            break;
        case WM_DESTROY:
            if (pThis->_ulChangeNotify)
            {
                SHChangeNotifyDeregister(pThis->_ulChangeNotify);
            }
            fHandled = TRUE;
            break;
        case WM_HOMEGROUP_CHANGED:
            pThis->_FillUserList(hwndDlg);
            fHandled = TRUE;
            break;
        }

    }
    return fHandled;
}