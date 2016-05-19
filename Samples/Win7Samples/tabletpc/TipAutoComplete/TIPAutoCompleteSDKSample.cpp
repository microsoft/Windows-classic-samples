// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All Rights Reserved.

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <assert.h>
#include <TIPAutoComplete.h>
#include <TIPAutoComplete_i.c>
#include <PenInputPanel.h>
#include <strsafe.h>
#include <new>
#include "Resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_STRING 256
#define MAX_ENTRY 1024
#define LISTVIEW_COLUMN_WIDTH 244

const static WCHAR s_wzClassName[] = L"TipAutoCompleteSampleDropdownClass";
const static WCHAR s_wzDropdownTitle[] = L"TIP AutoComplete Sample Dropdown";
const static WCHAR s_wzTipACDialogProp[] = L"CTipACDialog_This";

WCHAR s_rgwzList[MAX_ENTRY][MAX_STRING];

const static PCWSTR s_wzListBackup[] =
{
    L"Aphrodite", 
    L"Apollo",
    L"Ares", 
    L"Artemis", 
    L"Athena", 
    L"Demeter", 
    L"Dionysus", 
    L"Hades", 
    L"Hephaestus", 
    L"Hera", 
    L"Hermes", 
    L"Hestia", 
    L"Poseidon", 
    L"Zeus"
};

HINSTANCE g_hInstance;

class CTipACDialog : public ITipAutoCompleteProvider
{
public:
    CTipACDialog();

    static INT_PTR CALLBACK s_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_EditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    static LRESULT CALLBACK s_DropDownWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK s_ListViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // ITipAutoCompleteProvider methods
    STDMETHODIMP UpdatePendingText(__RPC__in BSTR bstrPendingText);
    STDMETHODIMP Show(BOOL fShow);

private:
    long m_nRefCount;
    long m_iLineCount;

    HWND m_hwndDlg;
    HWND m_hwndEdit;
    HWND m_hwndListbox;
    BOOL m_fSynchronize;
    WCHAR m_wzPending[MAX_STRING];

    // for !m_fSynchronize mode only
    HWND m_hwndDropdown;
    HWND m_hwndListView;
    WNDPROC m_pOldListViewWndProc;
    BOOL m_fInHotTracking;
    HRESULT ShowAutoCompleteDropdown(BOOL fShow);
    LRESULT EditWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT DropDownWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void DropDownDrawItem(LPDRAWITEMSTRUCT pdis);
    BOOL DropDownNotify(LPNMHDR pnmhdr);
    void OnListViewCreate();
    LRESULT ListViewWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    ITipAutoCompleteClient *m_pTipACClient;

    BOOL OnInitDialog(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnCloseDialog(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void DisplayMatchedOutput(WCHAR * pszInput);
    void InitTabletTip();
    void SetTipMode();
    void PopulateList();
};

CTipACDialog::CTipACDialog()
    : m_nRefCount(0),
      m_iLineCount(0),
      m_hwndDlg(NULL),
      m_hwndEdit(NULL),
      m_hwndListbox(NULL),
      m_hwndDropdown(NULL),
      m_hwndListView(NULL),
      m_fSynchronize(FALSE),
      m_fInHotTracking(FALSE),
      m_pTipACClient(NULL)
{
    m_wzPending[0] = NULL;
}

/*static*/ INT_PTR CALLBACK CTipACDialog::s_DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
    }

    CTipACDialog *pThis = (CTipACDialog *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    try
    {
        if (pThis)
        {
            switch (uMsg)
            {
            case WM_COMMAND:
                pThis->OnCommand(hWnd, WM_COMMAND, wParam, lParam);
                break;
            case WM_INITDIALOG:
                pThis->OnInitDialog(hWnd, WM_INITDIALOG, wParam, lParam);
                break;
            case WM_DESTROY:
                pThis->OnCloseDialog(hWnd, WM_DESTROY, wParam, lParam);
                break;
            }
        }
    }
    catch (...)
    {
        // Got an unhandled exception, cancel the dialog
        EndDialog(hWnd, E_FAIL);
    }

    return FALSE;
}

/*static*/ LRESULT CALLBACK CTipACDialog::s_EditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwRefData)
{
    CTipACDialog *pThis = (CTipACDialog *)dwRefData;
    if (pThis)
    {
        if (pThis->m_hwndEdit == hwnd)
        {
            return pThis->EditWndProc(uMsg, wParam, lParam);
        }
    }

    assert(pThis && L"CTipACDialog::s_EditWndProc: pThis == NULL");
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*static*/ LRESULT CALLBACK CTipACDialog::s_DropDownWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CTipACDialog *pThis;
    if (WM_NCCREATE == uMsg)
    {
        pThis = (CTipACDialog *)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hwndDropdown = hwnd;
    }
    else
    {
        pThis = (CTipACDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis)
    {
        return pThis->DropDownWndProc(uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*static*/ LRESULT CALLBACK CTipACDialog::s_ListViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CTipACDialog *pThis = (CTipACDialog *)GetProp(hwnd, s_wzTipACDialogProp);
    if (pThis)
    {
        return pThis->ListViewWndProc(uMsg, wParam, lParam);
    }
    assert(FALSE && L"CTipACDialog::s_ListViewWndProc: pThis == NULL");
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT CTipACDialog::ShowAutoCompleteDropdown(BOOL fShow)
{
    HRESULT hr = E_FAIL;

    if (m_pTipACClient && m_hwndDropdown)
    {
        if (fShow)
        {
            LRESULT cItems = SendMessage(m_hwndListbox, LB_GETCOUNT, 0, (LPARAM)0);
            ListView_SetItemCountEx(m_hwndListView, cItems, 0);
            if (0 == cItems)
            {
                fShow = false;
            }
        }

        if (fShow)
        {
            BOOL fAllowShow = TRUE;
            BOOL fDroppedUp = FALSE;
            RECT rcEdit;
            GetWindowRect(m_hwndEdit, &rcEdit);

            int x = rcEdit.left + 5;
            int y = rcEdit.bottom + 200;
            int w = rcEdit.right - rcEdit.left;
            int h = 200;

            hr = m_pTipACClient->RequestShowUI(m_hwndDropdown, &fAllowShow);
            assert(SUCCEEDED(hr) && "Failed in call to RequestShowUI");
            if (SUCCEEDED(hr) && fAllowShow)
            {
                RECT rcACList = { x, y, x+w, y+h };
                RECT rcModifiedACList = { 0, 0, 0, 0 };
                hr = m_pTipACClient->PreferredRects(&rcACList, &rcEdit, &rcModifiedACList, &fDroppedUp);
                if (SUCCEEDED(hr))
                {
                    x = rcModifiedACList.left;
                    y = rcModifiedACList.top;
                    w = rcModifiedACList.right - rcModifiedACList.left;
                    h = rcModifiedACList.bottom - rcModifiedACList.top;

                    SetWindowPos(m_hwndDropdown, HWND_TOP, x, y, w, h, SWP_SHOWWINDOW | SWP_NOACTIVATE);
                }
            }
        }
        else // hide
        {
            ShowWindow(m_hwndDropdown, SW_HIDE);
        }
    }

    return hr;
}

LRESULT CTipACDialog::EditWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SETTEXT:
            ShowAutoCompleteDropdown(FALSE);
            break;

        case WM_SETFOCUS:
            break;

        case WM_KILLFOCUS:
        {
            HWND hwndGetFocus = (HWND)wParam;
            if (m_hwndEdit != hwndGetFocus)
            {
                if (m_hwndDropdown && (GetFocus() != m_hwndDropdown))
                {
                    ShowAutoCompleteDropdown(FALSE);
                }
            }
            break;
        }

        case WM_DESTROY:
        {
            RemoveWindowSubclass(m_hwndEdit, s_EditWndProc, 0);
            if (m_pTipACClient)
            {
#ifdef _DEBUG
                HRESULT hr = m_pTipACClient->UnadviseProvider(m_hwndEdit, static_cast<ITipAutoCompleteProvider *>(this));
                assert(SUCCEEDED(hr) && "Failed in call to UnadviseProvider");
#else // !_DEBUG
                m_pTipACClient->UnadviseProvider(m_hwndEdit, static_cast<ITipAutoCompleteProvider *>(this));
#endif // _DEBUG
                m_pTipACClient = NULL;
            }
            break;
        }
    }

    return DefSubclassProc(m_hwndEdit, uMsg, wParam, lParam);
}

LRESULT CTipACDialog::DropDownWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_NCCREATE:
            OnListViewCreate();
            return (NULL != m_hwndListView);

        case WM_DESTROY:
        {
            ShowAutoCompleteDropdown(FALSE);

            SetWindowLongPtr(m_hwndDropdown, GWLP_USERDATA, (LONG_PTR)NULL);

            HWND hwnd = m_hwndDropdown;
            m_hwndDropdown = NULL;
            if (NULL != m_hwndListView)
            {
                DestroyWindow(m_hwndListView);
                m_hwndListView = NULL;
            }

            /// scroll window? Grip window?

            // The dropdown incremented this object's ref count
            Release();
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        case WM_SIZE:
        {
            int nWidth = LOWORD(lParam);
            int nHeight = HIWORD(lParam);

            MoveWindow(m_hwndListView, 0, 0, nWidth, nHeight, TRUE);

            break;
        }

        case WM_MOUSEACTIVATE:
            // We don't want mouse clicks to activate us and take focus from the edit box.
            return (LRESULT)MA_NOACTIVATE;

        case WM_DRAWITEM:
            DropDownDrawItem((LPDRAWITEMSTRUCT)lParam);
            break;

        case WM_NOTIFY:
        {
            LRESULT lResult = DropDownNotify((LPNMHDR)lParam);
            if (lResult)
            {
                return lResult;
            }

            break;
        }
    }

    return DefWindowProc(m_hwndDropdown, uMsg, wParam, lParam);
}

void CTipACDialog::DropDownDrawItem(LPDRAWITEMSTRUCT pdis)
{
    if (-1 != pdis->itemID)
    {
        HDC hdc = pdis->hDC;
        RECT rc = pdis->rcItem;
        BOOL fTextHighlight = pdis->itemState & ODS_SELECTED;

        SetBkColor(hdc, GetSysColor(fTextHighlight ? COLOR_HIGHLIGHT : COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor(fTextHighlight ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        // Center the string vertically in rc
        SIZE sizeText;
        WCHAR szText[MAX_STRING];

        SendMessage(m_hwndListbox, LB_GETTEXT, pdis->itemID, (LPARAM)szText);

        int cch = lstrlen(szText);
        GetTextExtentPoint(hdc, szText, cch, &sizeText);
        int yMid = (rc.top + rc.bottom) / 2;
        int yString = yMid - (sizeText.cy / 2);
        int xString = 5;

        ExtTextOut(hdc, xString, yString, ETO_OPAQUE | ETO_CLIPPED, &rc, szText, cch, NULL);
    }
}

BOOL CTipACDialog::DropDownNotify(LPNMHDR pnmhdr)
{
    WCHAR wzBuf[MAX_STRING];

    switch (pnmhdr->code)
    {
        case LVN_GETDISPINFO:
        {
            assert(pnmhdr->hwndFrom == m_hwndListView);
            LV_DISPINFO *pdi = (LV_DISPINFO *)pnmhdr;
            if (pdi->item.mask & LVIF_TEXT)
            {
                SendMessage(m_hwndListbox, LB_GETTEXT, pdi->item.iItem, (LPARAM)pdi->item.pszText);
            }
            break;
        }

        case LVN_ITEMCHANGED:
        {
            const NMLISTVIEW *pnmv = (const NMLISTVIEW *)pnmhdr;
            if (!m_fInHotTracking &&
                (pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & (LVIS_FOCUSED | LVIS_SELECTED)))
            {
                SendMessage(m_hwndListbox, LB_GETTEXT, pnmv->iItem, (LPARAM)wzBuf);
                m_wzPending[0] = NULL;
                SetWindowText(m_hwndEdit, wzBuf);
                int cch = lstrlen(wzBuf);
                SendMessage(m_hwndEdit, EM_SETSEL, cch, cch);
            }
            break;
        }

        case LVN_ENDSCROLL:
            InvalidateRect(m_hwndListView, NULL, TRUE);
            break;

        case LVN_ITEMACTIVATE:
        {
            const NMITEMACTIVATE *pnmia = (const NMITEMACTIVATE *)pnmhdr;
            SendMessage(m_hwndListbox, LB_GETTEXT, pnmia->iItem, (LPARAM)wzBuf);
            m_wzPending[0] = NULL;

            // Inform the TIP that the user made a selection.
            // This causes the text in the TIP to be discarded.
            m_pTipACClient->UserSelection();

            SetWindowText(m_hwndEdit, wzBuf);
            SendMessage(m_hwndEdit, EM_SETSEL, 0, -1);
            ShowAutoCompleteDropdown(FALSE);
            break;
        }

        case LVN_HOTTRACK:
        {
            const NMLISTVIEW *pnmlv = (const NMLISTVIEW *)pnmhdr;
            LVHITTESTINFO lvh;
            lvh.pt = pnmlv->ptAction;
            int iItem = ListView_HitTest(m_hwndListView, &lvh);
            if (-1 != iItem)
            {
                m_fInHotTracking = TRUE;
                ListView_SetItemState(m_hwndListView, iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED | LVIS_CUT | LVIS_DROPHILITED);
                SendMessage(m_hwndListView, LVM_ENSUREVISIBLE, iItem, (LPARAM)FALSE);
                m_fInHotTracking = FALSE;
            }
            return TRUE; // we processed this
        }
    }

    return FALSE;
}

void CTipACDialog::OnListViewCreate()
{
    DWORD dwFlags = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_OWNERDATA | LVS_OWNERDRAWFIXED;
    m_hwndListView = CreateWindowEx(0, WC_LISTVIEW, s_wzDropdownTitle, dwFlags, 0, 0, LISTVIEW_COLUMN_WIDTH, 10000, m_hwndDropdown, NULL, g_hInstance, NULL);
    if (m_hwndListView)
    {
        // Subclass the listview window
        if (SetProp(m_hwndListView, s_wzTipACDialogProp, this))
        {
            m_pOldListViewWndProc = (WNDPROC)SetWindowLongPtr(m_hwndListView, GWLP_WNDPROC, (LONG_PTR)&s_ListViewWndProc);
        }

        ListView_SetExtendedListViewStyle(m_hwndListView, LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_TRACKSELECT | LVS_EX_DOUBLEBUFFER);

        LV_COLUMN lvColumn;
        lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
        lvColumn.fmt = LVCFMT_LEFT;
        lvColumn.cx = LISTVIEW_COLUMN_WIDTH;
        ListView_InsertColumn(m_hwndListView, 0, &lvColumn);
    }
}

LRESULT CTipACDialog::ListViewWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        CallWindowProc(m_pOldListViewWndProc, m_hwndListView, uMsg, wParam, lParam);
        return 0;

    case WM_DESTROY:
        // Restore old wndproc
        RemoveProp(m_hwndListView, s_wzTipACDialogProp);
        if (m_pOldListViewWndProc)
        {
            SetWindowLongPtr(m_hwndListView, GWLP_WNDPROC, (LONG_PTR)m_pOldListViewWndProc);
            CallWindowProc(m_pOldListViewWndProc, m_hwndListView, uMsg, wParam, lParam);
            m_pOldListViewWndProc = NULL;
        }
        return 0;
    }

    return CallWindowProc(m_pOldListViewWndProc, m_hwndListView, uMsg, wParam, lParam);
}

/* *** IUnknown *** */
ULONG CTipACDialog::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CTipACDialog::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);

    if (0 == uCount)
    {
        delete this;
    }
    return uCount;
}

HRESULT CTipACDialog::QueryInterface(REFIID iid, void **ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (IID_IUnknown == iid)
    {
        *ppv = static_cast<IUnknown *>(this);
    }
    else if (IID_ITipAutoCompleteProvider == iid)
    {
        *ppv = static_cast<ITipAutoCompleteProvider *>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

/* *** ITipAutoCompleteProvider *** */
HRESULT CTipACDialog::UpdatePendingText(BSTR bstrPendingText)
{
    int cchNewPending = lstrlen(bstrPendingText);
    if (m_fSynchronize)
    {
        // Set the text of m_hwndEdit to bstrPendingText
        SetWindowText(m_hwndEdit, bstrPendingText);
        SendMessage(m_hwndEdit, EM_SETSEL, cchNewPending, cchNewPending);
    }
    else
    {
        if (0 == cchNewPending)
        {
            m_wzPending[0] = NULL;
        }

        int const cchEdit = GetWindowTextLength(m_hwndEdit) + 1;
        // is there text in the edit already?
        if (1 < cchEdit)
        {
            LRESULT lRes = SendMessage(m_hwndEdit, EM_GETSEL, NULL, NULL);
            int ichStart = LOWORD(lRes);
            int ichEnd = HIWORD(lRes);

            PWSTR pszEdit = new (std::nothrow) WCHAR[cchEdit];
            if (pszEdit)
            {
                GetWindowText(m_hwndEdit, pszEdit, cchEdit);
                // copy out of the edit from 0 through ichStart, inclusive
                StringCchCopyN(m_wzPending, ARRAYSIZE(m_wzPending), pszEdit, ichStart);
                // now append bstrPendingText
                StringCchCat(m_wzPending, ARRAYSIZE(m_wzPending), bstrPendingText);
                // now append the rest of the edit string
                StringCchCat(m_wzPending, ARRAYSIZE(m_wzPending), &pszEdit[ichEnd]);
                delete [] pszEdit;
            }
        }
        else if (m_wzPending[0] || (0 < cchNewPending))
        {
            StringCchCopy(m_wzPending, ARRAYSIZE(m_wzPending), bstrPendingText);
        }

        DisplayMatchedOutput((0 < m_wzPending[0]) ? m_wzPending : L"");
        ShowAutoCompleteDropdown(0 < m_wzPending[0]);
    }
    return S_OK;
}

HRESULT CTipACDialog::Show(BOOL fShow)
{
    // ignore needing to show or hide the AutoComplete list if m_fSynchronize is set
    ShowAutoCompleteDropdown(fShow);

    return S_OK;
}

BOOL CTipACDialog::OnInitDialog(HWND hwndDlg, UINT, WPARAM, LPARAM)
{
    m_hwndDlg = hwndDlg;

    m_hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT1);
    m_hwndListbox = GetDlgItem(hwndDlg, IDC_LIST1);

    SetWindowSubclass(m_hwndEdit, &s_EditWndProc, 0, (DWORD_PTR)this);

    CheckDlgButton(hwndDlg, IDC_CHECK1, m_fSynchronize);

    InitTabletTip();

    PopulateList();

    return TRUE;
}

void CTipACDialog::OnCloseDialog(HWND, UINT, WPARAM, LPARAM)
{
    if (NULL != m_pTipACClient)
    {
        m_pTipACClient->Release();
        m_pTipACClient = NULL;
    }
}

void CTipACDialog::InitTabletTip()
{
    assert(!m_pTipACClient);

    // Initialize the AutoComplete client
    WCHAR wzCLSID[40];
    DWORD cb = ARRAYSIZE(wzCLSID) * sizeof(WCHAR);
    if (ERROR_SUCCESS == RegGetValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete\\Client\\", NULL, RRF_RT_REG_SZ, NULL, (void *)&wzCLSID, &cb))
    {
        CLSID clsidTipAutoComplete;
        if (SUCCEEDED(CLSIDFromString(wzCLSID, &clsidTipAutoComplete)))
        {
            HRESULT hrTip = CoCreateInstance(clsidTipAutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTipACClient));
            if (SUCCEEDED(hrTip))
            {
                hrTip = m_pTipACClient->AdviseProvider(m_hwndEdit, static_cast<ITipAutoCompleteProvider *>(this));
                assert(SUCCEEDED(hrTip) && "Failed in call to AdviseProvider");
                if (FAILED(hrTip))
                {
                    // we failed to talk to the TIP fully, release any reference
                    m_pTipACClient = NULL;
                }
                else
                {
                    SetTipMode();
                }
            }
        }
    }
}

void CTipACDialog::SetTipMode()
{
    if (m_fSynchronize)
    {
        // Request the TIP to operate in no Insert button mode if Syncronize checkbox is checked
        SetProp(m_hwndEdit, MICROSOFT_TIP_NO_INSERT_BUTTON_PROPERTY, (HANDLE)TRUE);
        ShowWindow(m_hwndListbox, SW_SHOW);
        DestroyWindow(m_hwndDropdown);
        m_hwndDropdown = NULL;
    }
    else
    {
        // We want the insert button
        SetProp(m_hwndEdit, MICROSOFT_TIP_NO_INSERT_BUTTON_PROPERTY, (HANDLE)FALSE);
        ShowWindow(m_hwndListbox, SW_HIDE);

        if (NULL == m_hwndDropdown) // need to create a dropdown window
        {
            WNDCLASS wc = {};
            wc.lpfnWndProc = s_DropDownWndProc;
            wc.cbWndExtra = sizeof(CTipACDialog *);
            wc.hInstance = g_hInstance;
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wc.style = CS_SAVEBITS | CS_DROPSHADOW;
            wc.lpszClassName = s_wzClassName;

            RegisterClass(&wc);

            DWORD dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY;

            // The dropdown holds a ref on this object
            AddRef();
            m_hwndDropdown = CreateWindowEx(dwExStyle, s_wzClassName, s_wzDropdownTitle,
                WS_POPUP | WS_BORDER | WS_CLIPCHILDREN, 0, 0, 100, 400,
                NULL, NULL, g_hInstance, this);
            if (NULL == m_hwndDropdown)
            {
                Release();
            }
        }

        if (m_hwndDropdown)
        {
            DisplayMatchedOutput(m_wzPending);
        }
    }
}

void CTipACDialog::PopulateList()
{
    FILE *fp = NULL;
    WCHAR szInput[MAX_STRING];
    WCHAR *pszInput;
    size_t cch;

    m_iLineCount = 0;

    _wfopen_s(&fp, L"List.txt", L"r");
    if (fp)
    {
        do
        {
            pszInput = fgetws(szInput, MAX_STRING, fp);
            cch = wcslen(szInput);
            if (cch && szInput[cch - 1] == 0x0a)
            {
                szInput[cch - 1] = NULL; // skip the linefeed
                cch--;
            }
            wcsncpy_s(s_rgwzList[m_iLineCount], MAX_STRING, szInput, cch);
            m_iLineCount++;

            SendMessage(m_hwndListbox, LB_ADDSTRING, 0, (LPARAM)szInput);   
        } while (pszInput);
    }
    else
    {
        for ( ; m_iLineCount < ARRAYSIZE(s_wzListBackup); m_iLineCount++)
        {
            wcsncpy_s(s_rgwzList[m_iLineCount], MAX_STRING, s_wzListBackup[m_iLineCount], wcslen(s_wzListBackup[m_iLineCount]));
        }
    }
}

void CTipACDialog::OnCommand(HWND hwnd, UINT, WPARAM wParam, LPARAM)
{
    int id = LOWORD(wParam);
    WCHAR szSelectedText[MAX_STRING] = {0};

    switch (id)
    {
    case IDCANCEL:
        // Explicitly destroy the dropdown window so that it releases its reference to this
        DestroyWindow(m_hwndDropdown);

        EndDialog(hwnd, S_FALSE);
        break;

    case IDC_LIST1:
        {
            LRESULT iSel = SendMessage(m_hwndListbox, LB_GETCURSEL, 0, (LPARAM)0);
            if (-1 != iSel)
            {
                SendMessage(m_hwndListbox, LB_GETTEXT, iSel, (LPARAM)szSelectedText);
                SetWindowText(m_hwndEdit, szSelectedText);
            }
        }
        break;

    case IDC_EDIT1:
        if (EN_UPDATE == HIWORD(wParam))
        {
            if (!m_fSynchronize && m_wzPending[0])
            {
                m_wzPending[0] = NULL;
            }
            else // just match what's in the textbox: m_fSynchronize or nothing pending
            {
                int const cchInput = GetWindowTextLength(m_hwndEdit) + 1;
                PWSTR pszInput = new (std::nothrow) WCHAR[cchInput];
                if (pszInput)
                {
                    GetWindowText(m_hwndEdit, pszInput, cchInput);
                    DisplayMatchedOutput(pszInput);
                    ShowAutoCompleteDropdown(0 < pszInput[0]);
                    delete [] pszInput;
                }
            }
        }
        break;

    case IDC_CHECK1:
        m_fSynchronize = IsDlgButtonChecked(hwnd, IDC_CHECK1);
        SetTipMode();
        break;
    }
}

void CTipACDialog::DisplayMatchedOutput(WCHAR * pszInput)
{
    int iCount = 0;
    int i = 0, j = 0;
    BOOL fMatch = FALSE;

    // Note that this code is NOT being efficient about filling the list or keeping the list:
    // Every time this method is called at all (on every change of the edit box!) we destroy
    // the contents of the listbox and then regenerate all of it.
    // *****
    // This is inefficient list management, and should not be used in shipping code.
    // *****
    // This method is suitable for this sample's purposes, as the list maintenance isn't important;
    // we're showing how to allow the TIP to work with an application's specialized autocomplete list(s).

    SendMessage(m_hwndListbox, LB_RESETCONTENT, 0, 0);
    for (iCount = 0; iCount < m_iLineCount; iCount++)
    {
        i = 0;
        j = 0;
        if (NULL == pszInput[0])
        {
            fMatch = TRUE;
        }
        else
        {
            while (*(pszInput + i) != '\0')
            {
                fMatch = FALSE;
                if (toupper(*(pszInput + i)) == toupper(s_rgwzList[iCount][j]))
                {
                    i++;
                    j++;
                    fMatch = TRUE;
                }
                else
                {
                    break;
                }
            }
        }

        if (fMatch)
        {
            SendMessage(m_hwndListbox, LB_ADDSTRING, 0, (LPARAM)s_rgwzList[iCount]);
        }
    }
}

STDAPI_(int) wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    g_hInstance = hInstance;
    
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CTipACDialog *pTipACDialog = new CTipACDialog();

        if (pTipACDialog)
        {
            // Open the dialog
            DialogBoxParam(g_hInstance, (LPCWSTR)IDD_MAIN, NULL, CTipACDialog::s_DialogProc, (LPARAM)pTipACDialog);
        }

        CoUninitialize();
    }

    return 0;
}
