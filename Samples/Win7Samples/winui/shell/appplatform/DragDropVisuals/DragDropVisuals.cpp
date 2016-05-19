// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Sample that demonstrates using the shell drag drop services to get the presentation features
// that shell drag drop supports for both targets and sources. this includes
// 1) drop targets rendering the drag image
// 2) drop target provided drop tips
// 3) drag source populating the drag image information when using a custom data object
// 4) drag source enable drop tips
// 5) use the shell provided IDropSource implementation by calling SHDoDragDrop()
//    this handles many of the edge cases for you dealing with different types of targets
//
// This is similar to the following article that covers this for managed apps
// http://blogs.msdn.com/adamroot/pages/shell-style-drag-and-drop-in-net-wpf-and-winforms.aspx
//
// This sample does not demonstrating rendering the drop location, applications should
// consider adding that feature to make it clear where the dropped item will go
//
// DragDropHelpers.h encapsulates a lot of the functionlaity used by this sample
// so look at its implementation to learn more about how that works

#define STRICT_TYPED_ITEMIDS

#include <windows.h>
#include "ShellHelpers.h"
#include "DragDropHelpers.h"
#include <new>  // std::nothrow
#include "resource.h"
#include "DataObject.h"

class CDragDropVisualsApp : public CDragDropHelper
{
public:
    CDragDropVisualsApp() : _cRef(1), _hdlg(NULL), _psiaDrop(NULL)
    {
    }

    HRESULT DoModal(HWND hwnd)
    {
        DialogBoxParam(GetModuleHINSTANCE(), MAKEINTRESOURCE(IDD_DIALOG), hwnd, s_DlgProc, (LPARAM)this);
        return S_OK;
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CDragDropVisualsApp, IDropTarget),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

private:
    ~CDragDropVisualsApp()
    {
        SafeRelease(&_psiaDrop);
    }

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CDragDropVisualsApp *pcd = (CDragDropVisualsApp*) (LONG_PTR) GetWindowLongPtr(hdlg, DWLP_USER);
        if (uMsg == WM_INITDIALOG)
        {
            pcd = (CDragDropVisualsApp *) lParam;
            pcd->_hdlg = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pcd);
        }
        return pcd ? pcd->_DlgProc(uMsg, wParam, lParam) : FALSE;
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnInitDlg();
    void _OnDestroyDlg();

    void _BindUI();
    void _OnOpen();
    void _BeginDrag(HWND hwndDragBegin);
    bool _UseCustomDataObject()
    {
        return IsDlgButtonChecked(_hdlg, IDC_CUSTOM_DATAOBJECT) ? true : false;
    }

    HRESULT _GetDataObject(HWND hwndDragBegin, IDataObject **ppdtobj);
    HRESULT _CopyShellItemArray(IShellItemArray *psia, IShellItemArray **ppsiaOut);
    DWORD _GetDropEffects();

    HRESULT _GetSelectedItems(IShellItemArray **ppsia)
    {
        *ppsia = NULL;
        return _psiaDrop ? _psiaDrop->QueryInterface(ppsia) : E_NOINTERFACE;
    }

    HRESULT _GetFirstItem(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        IShellItemArray *psia;
        HRESULT hr = _GetSelectedItems(&psia);
        if (SUCCEEDED(hr))
        {
            hr = GetItemAt(psia, 0, riid, ppv);
            psia->Release();
        }
        return hr;
    }

    virtual HRESULT OnDrop(IShellItemArray *psia, DWORD /* grfKeyState */)
    {
        HRESULT hr = _CopyShellItemArray(psia, &_psiaDrop);
        if (SUCCEEDED(hr))
        {
            _BindUI();
        }
        return hr;
    }

    long _cRef;
    HWND _hdlg;
    IShellItemArray *_psiaDrop;
};

void CDragDropVisualsApp::_OnInitDlg()
{
    InitializeDragDropHelper(_hdlg, DROPIMAGE_COPY, L"Drop Visuals Sample App");
    _BindUI();
}

void CDragDropVisualsApp::_OnDestroyDlg()
{
    TerminateDragDropHelper();
    // cleanup the allocated HBITMAP
    SetItemImageImageInStaticControl(GetDlgItem(_hdlg, IDC_IMAGE), NULL);
}

void CDragDropVisualsApp::_OnOpen()
{
    // If multiple items are dropped into our sample, the "Open" button will only open the first item from the array
    IShellItem *psi;
    HRESULT hr = _GetFirstItem(IID_PPV_ARGS(&psi));
    if (SUCCEEDED(hr))
    {
        hr = ShellExecuteItem(_hdlg, NULL, psi);
        psi->Release();
    }
}

void CDragDropVisualsApp::_BindUI()
{
    IShellItem2 *psi;
    HRESULT hr = _GetFirstItem(IID_PPV_ARGS(&psi));
    if (SUCCEEDED(hr))
    {
        SetDlgItemText(_hdlg, IDC_STATIC, L"Start Drag Drop by clicking on icon");

        SetItemImageImageInStaticControl(GetDlgItem(_hdlg, IDC_IMAGE), psi);

        PWSTR psz;
        hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &psz);
        if (SUCCEEDED(hr))
        {
            SetDlgItemText(_hdlg, IDC_NAME, psz);
            CoTaskMemFree(psz);
        }

        SFGAOF sfgaof;
        hr = psi->GetAttributes(SFGAO_CANCOPY| SFGAO_CANLINK | SFGAO_CANMOVE, &sfgaof);
        if (SUCCEEDED(hr))
        {
            hr = ShellAttributesToString(sfgaof, &psz);
            if (SUCCEEDED(hr))
            {
                SetDlgItemText(_hdlg, IDC_ATTRIBUTES, psz);
                CoTaskMemFree(psz);
            }
        }
        psi->Release();
    }
    else
    {
        SetItemImageImageInStaticControl(GetDlgItem(_hdlg, IDC_IMAGE), NULL);
        SetDlgItemText(_hdlg, IDC_STATIC, L"Drop An Item Here");
        SetDlgItemText(_hdlg, IDC_NAME, L"");
        SetDlgItemText(_hdlg, IDC_ATTRIBUTES, L"");
    }
    EnableWindow(GetDlgItem(_hdlg, IDC_CUSTOM_DATAOBJECT), SUCCEEDED(hr) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(_hdlg, IDC_OPEN), SUCCEEDED(hr) ? TRUE : FALSE);
    EnableWindow(GetDlgItem(_hdlg, IDC_CLEAR), SUCCEEDED(hr) ? TRUE : FALSE);
}

// this inits the SHDRAGIMAGE from a bitmap and the HWND that initiated
// the dragging. this uses the current cursor pos to compute the offsets
// in the image to get the proper positioning of that image relative to the
// drag cursors

void InitializeDragImageFromWindow(HWND hwndDragBegin, HBITMAP hbmp, SHDRAGIMAGE *pdi)
{
    ZeroMemory(pdi, sizeof(pdi));

    pdi->crColorKey = CLR_NONE;   // assume alpha image, no need for color key
    pdi->hbmpDragImage = hbmp;
    BITMAP bm;
    if (GetObject(hbmp, sizeof(bm), &bm))
    {
        pdi->sizeDragImage.cx = bm.bmWidth;
        pdi->sizeDragImage.cy = bm.bmHeight;
    }

    RECT rc;
    POINT ptDrag;
    if (GetWindowRect(hwndDragBegin, &rc) && GetCursorPos(&ptDrag))
    {
        pdi->ptOffset.x = ptDrag.x - rc.left;
        pdi->ptOffset.y = ptDrag.y - rc.top;
    }
}

// USE_ITEMS_DATAOBJECT demonstrates what happens when using the data object from
// the shell items. that data object supports all of the features needed to get drag
// images, drop tips, etc.
//
// for appliations that create their own data object you need to have support
// for SetData(), QueryGetData() and GetData() of custom formats. this is
// demonstrated in DataObject.cpp

HRESULT CDragDropVisualsApp::_GetDataObject(HWND hwndDragBegin, IDataObject **ppdtobj)
{
    HRESULT hr;
    if (_UseCustomDataObject())
    {
        hr = CDataObject_CreateInstance(IID_PPV_ARGS(ppdtobj));
        if (SUCCEEDED(hr))
        {
            IDragSourceHelper2 *pdsh;
            if (SUCCEEDED(GetDragDropHelper(IID_PPV_ARGS(&pdsh))))
            {
                // enable drop tips
                pdsh->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);

                // we need to make a copy of the HBITMAP held by the static control
                // as InitializeFromBitmap() takes owership of this
                const HBITMAP hbmp = (HBITMAP)CopyImage((HBITMAP) SendMessage(hwndDragBegin, STM_GETIMAGE, IMAGE_BITMAP, 0), IMAGE_BITMAP, 0, 0, 0);

                // alternate, load the bitmap from a resource
                // HBITMAP hbmp = (HBITMAP)LoadImage(NULL, MAKEINTRESOURCE(OBM_CLOSE), IMAGE_BITMAP, 128, 128, 0);

                SHDRAGIMAGE di;
                InitializeDragImageFromWindow(hwndDragBegin, hbmp, &di);

                // note that InitializeFromBitmap() takes ownership of the hbmp
                // so we should not free it by calling DeleteObject()
                pdsh->InitializeFromBitmap(&di, *ppdtobj);

                pdsh->Release();
            }
        }
    }
    else
    {
        *ppdtobj = NULL;
        IShellItemArray *psiaItems;
        hr = _GetSelectedItems(&psiaItems);
        if (SUCCEEDED(hr))
        {
            hr = psiaItems->BindToHandler(NULL, BHID_DataObject, IID_PPV_ARGS(ppdtobj));
            psiaItems->Release();
        }
    }
    return hr;
}


// The IDataObject passed in by OLE through the CDragDropHelper::Drop function is only valid until the Drop function returns
// This means the the IShellItemArray we are receiving may go bad as well since it is based on the incoming IDataObject
// Here we will create a stream and marshal the IShellItemArray which will create a copied IShellItemArray which does not depend on the IDataObject
HRESULT CDragDropVisualsApp::_CopyShellItemArray(IShellItemArray *psia, IShellItemArray **ppsiaOut)
{
    *ppsiaOut = NULL;
    IStream *pstm;
    HRESULT hr = CoMarshalInterThreadInterfaceInStream(__uuidof(psia), psia, &pstm);
    if (SUCCEEDED(hr))
    {
        hr = CoGetInterfaceAndReleaseStream(pstm, IID_PPV_ARGS(ppsiaOut));
        pstm = NULL; // released by CoGetInterfaceAndReleaseStream
    }
    return hr;
}

DWORD CDragDropVisualsApp::_GetDropEffects()
{
    DWORD dwEffect;
    if (_UseCustomDataObject())
    {
        dwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;
    }
    else
    {
        dwEffect = DROPEFFECT_NONE;
        IShellItemArray *psiaItems;
        if (SUCCEEDED(_GetSelectedItems(&psiaItems)))
        {
            psiaItems->GetAttributes(SIATTRIBFLAGS_AND, DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK, &dwEffect);
            psiaItems->Release();
        }
    }
    return dwEffect;
}

// x, y in client coordinates

BOOL CheckForDragBegin(HWND hwnd, int x, int y)
{
    int const cxDrag = GetSystemMetrics(SM_CXDRAG);
    int const cyDrag = GetSystemMetrics(SM_CYDRAG);

    // See if the user moves a certain number of pixels in any direction
    RECT rcDragRadius;
    SetRect(&rcDragRadius, x - cxDrag, y - cyDrag, x + cxDrag, y + cyDrag);

    MapWindowRect(hwnd, NULL, &rcDragRadius);   // client -> screen

    SetCapture(hwnd);

    do
    {
        // Sleep the thread waiting for mouse input. Prevents pegging the CPU in a
        // PeekMessage loop.
        MSG msg;
        switch (MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_MOUSE, MWMO_INPUTAVAILABLE))
        {
            case WAIT_OBJECT_0:
            {
                if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
                {
                    // See if the application wants to process the message...
                    if (CallMsgFilter(&msg, MSGF_COMMCTRL_BEGINDRAG) == 0)
                    {
                        switch (msg.message)
                        {
                            case WM_LBUTTONUP:
                            case WM_RBUTTONUP:
                            case WM_LBUTTONDOWN:
                            case WM_RBUTTONDOWN:
                                // Released the mouse without moving outside the
                                // drag radius, not beginning a drag.
                                ReleaseCapture();
                                return FALSE;

                            case WM_MOUSEMOVE:
                                if (!PtInRect(&rcDragRadius, msg.pt))
                                {
                                    // Moved outside the drag radius, beginning a drag.
                                    ReleaseCapture();
                                    return TRUE;
                                }
                                break;

                            default:
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                                break;
                        }
                    }
                }
                break;
            }
            default:
                break;
        }

        // WM_CANCELMODE messages will unset the capture, in that
        // case I want to exit this loop
    }
    while (GetCapture() == hwnd);

    return FALSE;
}

void CDragDropVisualsApp::_BeginDrag(HWND hwndDragBegin)
{
    POINT pt;
    GetCursorPos(&pt);
    MapWindowPoints(HWND_DESKTOP, hwndDragBegin, &pt, 1);   // screen -> client

    if (CheckForDragBegin(hwndDragBegin, pt.x, pt.y))
    {
        IDataObject *pdtobj;
        HRESULT hr = _GetDataObject(hwndDragBegin, &pdtobj);
        if (SUCCEEDED(hr))
        {
            DWORD dwEffectResult;
            hr = SHDoDragDrop(_hdlg, pdtobj, NULL, _GetDropEffects(), &dwEffectResult);
            pdtobj->Release();
        }
    }
}


INT_PTR CDragDropVisualsApp::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDlg();
        break;

    case WM_COMMAND:
        {
            const int idCmd = LOWORD(wParam);
            switch (idCmd)
            {
            case IDOK:
            case IDCANCEL:
                return EndDialog(_hdlg, idCmd);

            case IDC_OPEN:
                _OnOpen();
                break;

            case IDC_CLEAR:
                SafeRelease(&_psiaDrop);
                _BindUI();
                break;

            case IDC_IMAGE:
                switch (HIWORD(wParam))
                {
                case STN_CLICKED:
                    _BeginDrag(GetDlgItem(_hdlg, idCmd));
                    break;

                case STN_DBLCLK:
                    _OnOpen();
                    break;
                }
                break;
            }
        }
        break;

    case WM_DESTROY:
        _OnDestroyDlg();
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CDragDropVisualsApp *pdlg = new (std::nothrow) CDragDropVisualsApp();
        if (pdlg)
        {
            pdlg->DoModal(NULL);
            pdlg->Release();
        }
        CoUninitialize();
    }
    return 0;
}
