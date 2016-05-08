// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

// 1) define the GROUPID enum
//      enum GROUPID = {GROUPID_DEFAULT, GROUPID_1, GROUPID_2, ... };
// 2) define the group mapping structure
//      typedef struct
//      {
//          GROUPID groupid;
//          PCWSTR pszGroupName;
//      } LOG_GROUP;
//
// 3) declare c_rgGroupInfo[]
//    const LOG_GROUP c_rgGroupInfo[] = { };
// 4) declare the member var that represents the log window
//      CLogWindow<LOG_GROUP, GROUPID> _logWindow
// 5) in the constructor of your main window construct call CLogWindow's constructor
//      _logWindow(c_rgGroupInfo, ARRAYSIZE(c_rgGroupInfo))
// 6) in WM_INITDIALOG call _logWindow.InitListView(GetDlgItem(_hdlg, IDC_LISTVIEW))
// 7) in your .RC file add the definition of the listview window control
//    CONTROL "",IDC_LISTVIEW,"SysListView32",WS_CLIPCHILDREN | WS_TABSTOP, 7,7,281,191
// 8) to enable the right click menu in the log window forward the WM_NOTIFY message
//    to CLogWindow list this
//        case WM_NOTIFY:
//            {
//                NMHDR *pnm = (NMHDR*)lParam;
//                if (pnm->idFrom == IDC_LISTVIEW)
//                {
//                    _logWindow.OnNotify(pnm);
//                }
//            }
//            break;

template <class TGroupIDMap, class TGroupID> class CLogWindow
{
public:
    CLogWindow(const TGroupIDMap *pGroupInfo, UINT cGroupInfo) :
      _pGroupInfo(pGroupInfo), _cGroupInfo(cGroupInfo), _fDebugOutput(false)
    {
    }

    CLogWindow & operator=(const CLogWindow &)
    {
        // User-defined assignment operator is necessary for /W4 /WX since a default
        // one cannot be created by the compiler because this class contains const members.
    }

    void InitListView(HWND hwndList)
    {
        _hwndList = hwndList;
        // Enable ListView for Grouping mode.
        SetWindowLongPtr(_hwndList, GWL_STYLE, GetWindowLongPtr(_hwndList, GWL_STYLE) |
                        LVS_REPORT | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS);
        ListView_SetExtendedListViewStyle(_hwndList, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
        ListView_EnableGroupView(_hwndList, TRUE);

        // Setup up common values.
        LVCOLUMN lvc = {};
        lvc.mask     = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT; // Links are not active in this dialog, so just show text without markup
        lvc.fmt      = LVCFMT_LEFT;
        lvc.pszText  = L"Name";

        // Add Column 0
        lvc.iSubItem = 0;
        ListView_InsertColumn(_hwndList, 0, &lvc);

        // Add Column 1
        lvc.iSubItem = 1;
        lvc.pszText = L"Value";
        ListView_InsertColumn(_hwndList, 1, &lvc);

        AutoAdjustListView();

        // Init group IDs and display names
        for (UINT i = 0; i < _cGroupInfo; i++)
        {
            LogGroup(_pGroupInfo[i].groupid, _pGroupInfo[i].pszGroupName);
        }
    }

    void AutoAdjustListView()
    {
        // Auto-adjust the column widths making sure that the first column doesn't
        // make itself too big.

        ListView_SetColumnWidth(_hwndList, 0, LVSCW_AUTOSIZE_USEHEADER);

        RECT rect;
        BOOL bRet = GetClientRect(_hwndList, &rect);
        if (bRet)
        {
            LVCOLUMN lvc;
            lvc.mask = LVCF_WIDTH;
            bRet = ListView_GetColumn(_hwndList, 0, &lvc);
            if (bRet)
            {
                int iSize = rect.right / 2;
                int cxScroll = GetSystemMetrics(SM_CXVSCROLL);

                if (lvc.cx > iSize)
                {
                    ListView_SetColumnWidth(_hwndList, 0, iSize);
                    ListView_SetColumnWidth(_hwndList, 1, iSize - cxScroll);
                }
                else
                {
                    ListView_SetColumnWidth(_hwndList, 1, rect.right - lvc.cx - cxScroll);
                }
            }
        }

        if (!bRet)
        {
            ListView_SetColumnWidth(_hwndList, 1, LVSCW_AUTOSIZE_USEHEADER);
        }
    }

    void LogGroup(TGroupID groupid, PCWSTR pszGroupName)
    {
        LVGROUP lvg = {};
        lvg.cbSize    = sizeof(lvg);
        lvg.mask      = LVGF_HEADER | LVGF_GROUPID | LVGF_STATE;
        lvg.state     = LVGS_COLLAPSIBLE;

        int iGroupID = (int)groupid;
        if (groupid == GROUPID_DEFAULT)
        {
            iGroupID = (int)ListView_GetGroupCount(_hwndList);
        }
        lvg.iGroupId  = iGroupID;
        lvg.pszHeader = const_cast<PWSTR>(pszGroupName);
        ListView_InsertGroup(_hwndList, -1, &lvg);
    }

    void LogMessage(TGroupID groupid, PCWSTR pszName, PCWSTR pszBuf)
    {
        // Add an item name
        LVITEM lvi = {};
        lvi.mask      = LVIF_TEXT | LVIF_GROUPID;
        lvi.iItem     = MAXLONG;

        int iGroupID = (int)groupid;
        if (groupid == GROUPID_DEFAULT)
        {
            iGroupID = (int)ListView_GetGroupCount(_hwndList) - 1;  // groups are numbered 0, 1, ... n-1
        }
        lvi.iGroupId  = iGroupID;
        lvi.pszText   = const_cast<PWSTR>(pszName);

        int iItem = ListView_InsertItem(_hwndList, &lvi);
        if (-1 != iItem)
        {
            // Add the formatted value.
            ListView_SetItemText(_hwndList, iItem, 1, const_cast<PWSTR>(pszBuf));

            if (_fDebugOutput)
            {
                OutputDebugString(pszName);
                OutputDebugString(L"\t");
                OutputDebugString(pszBuf);
                OutputDebugString(L"\r\n");
            }
        }
    }

    void LogMessagePrintf(TGroupID groupid, PCWSTR pszName, PCWSTR pszFormatString, ...)
    {
        va_list argList;
        va_start(argList, pszFormatString);

        WCHAR szBuf[512];
        HRESULT hr = StringCchVPrintf(szBuf, ARRAYSIZE(szBuf), pszFormatString, argList);
        if (SUCCEEDED(hr))
        {
            LogMessage(groupid, pszName, szBuf);
        }

        va_end(argList);
    }

    void ResetContents()
    {
        ListView_DeleteAllItems(_hwndList);
    }

    void SetDebugOutput(bool fDebugOutput)
    {
        _fDebugOutput = fDebugOutput;
    }

    PWSTR GetText(bool fSelectionOnly)
    {
        size_t const cchAlloc = 64 * 1024;   // fixed size buffer for simplicity of impl
        PWSTR pszResult = (PWSTR)GlobalAlloc(GPTR, cchAlloc * sizeof(*pszResult));
        if (pszResult)
        {
            PWSTR psz = pszResult;  // accumulate results using this pointer
            size_t cch = cchAlloc;  // size left in buffer

            const int itemCount = ListView_GetItemCount(_hwndList);
            for (int i = 0; i < itemCount; i++)
            {
                if (fSelectionOnly ? (LVIS_SELECTED == ListView_GetItemState(_hwndList, i, LVIS_SELECTED)) : true)
                {
                    LVCOLUMN column = {LVCF_WIDTH}; // query a dummy value so we can probe for columns presence
                    for (int j = 0; ListView_GetColumn(_hwndList, j, &column); j++)
                    {
                        WCHAR szBuffer[512];
                        LV_ITEM item;
                        item.iItem = i;
                        item.iSubItem = j;
                        item.mask = LVIF_TEXT;
                        item.pszText = szBuffer;
                        item.cchTextMax = ARRAYSIZE(szBuffer);
                        if (ListView_GetItem(_hwndList, &item))
                        {
                            if (j)
                            {
                                StringCchCatEx(psz, cch, L"\t", &psz, &cch, 0);
                            }
                            StringCchCatEx(psz, cch, szBuffer, &psz, &cch, 0);
                        }
                    }
                    StringCchCatEx(psz, cch, L"\r\n", &psz, &cch, 0);
                }
            }
        }
        return pszResult;
    }

    void CopyTextToClipboard(bool fSelectionOnly, HWND hwnd)
    {
        PWSTR psz = GetText(fSelectionOnly);
        if (psz)
        {
            HRESULT hr = OpenClipboard(hwnd) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                EmptyClipboard();
                hr = (SetClipboardData(CF_UNICODETEXT, psz) == psz) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    psz = NULL; // ownership transfered to clipboard
                }
                CloseClipboard();
            }
            GlobalFree(psz);
        }
    }

    void OnNotify(NMHDR *pnm)
    {
        if (pnm->code == NM_RCLICK)
        {
            NMITEMACTIVATE *pnmItem = (NMITEMACTIVATE *)pnm;
            POINT ptMenu = pnmItem->ptAction;
            ClientToScreen(pnmItem->hdr.hwndFrom, &ptMenu);

            HMENU hMenu = CreatePopupMenu();
            if (hMenu)
            {
                AppendMenu(hMenu, MF_ENABLED, 1, L"Copy");
                AppendMenu(hMenu, MF_ENABLED, 2, L"Copy All");

                int idCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, ptMenu.x, ptMenu.y, 0, _hwndList, NULL);
                if (idCmd == 1)
                {
                    CopyTextToClipboard(true, GetParent(_hwndList));
                }
                else if (idCmd == 2)
                {
                    CopyTextToClipboard(false, GetParent(_hwndList));
                }
                DestroyMenu(hMenu);
            }
        }
    }

private:
    HWND _hwndList;
    const TGroupIDMap *_pGroupInfo;
    const UINT _cGroupInfo;
    bool _fDebugOutput;
};
