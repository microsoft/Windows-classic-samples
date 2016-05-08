// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "priv.h"
#include <strsafe.h>

class CNonDefaultDropMenuVerb : public IContextMenu,
                                public IShellExtInit
{
public:

    CNonDefaultDropMenuVerb() : _cRef(1), _pdtobj(NULL)
    {
        _szTargetFolder[0] = 0;
        DllAddRef();
    }

    // IUnknown methods

    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CNonDefaultDropMenuVerb, IContextMenu),
            QITABENT(CNonDefaultDropMenuVerb, IShellExtInit),
            { 0 }
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
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IContextMenu
    IFACEMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    IFACEMETHODIMP GetCommandString(UINT_PTR /*idCmd*/, UINT /*uType*/, UINT * /*pRes*/, LPSTR /*pszName*/, UINT /*cchMax*/) { return E_NOTIMPL; }

    // IShellExtInit
    IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

private:
    ~CNonDefaultDropMenuVerb()
    {
        SafeRelease(&_pdtobj);
        DllRelease();
    }

    UINT _GetNetResource(HGLOBAL hnres, UINT iItem, NETRESOURCEW *pnresOut, UINT cbMax);
    HRESULT _CheckForHNRES(HWND hwnd);
    HRESULT _CheckForHDROP(HWND hwnd);

    long        _cRef;
    WCHAR       _szTargetFolder[MAX_PATH];
    IDataObject *_pdtobj;       // data object
};

// This function is called back from within IClassFactory::CreateInstance()
// of the default class factory object, which is created by SHCreateClassObject.

HRESULT CNonDefaultDropMenuVerb_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    CNonDefaultDropMenuVerb *pnddmv = new (std::nothrow) CNonDefaultDropMenuVerb();
    HRESULT hr = pnddmv ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pnddmv->QueryInterface(riid, ppv);
        pnddmv->Release();
    }
    return hr;
}

// IShellExtInit
IFACEMETHODIMP CNonDefaultDropMenuVerb::Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY /*hkeyProgID*/)
{
    // Initialize can be called more than once.
    SafeRelease(&_pdtobj);
    _szTargetFolder[0] = 0;

    HRESULT hr = pdtobj->QueryInterface(&_pdtobj);
    // Get the path to the drop target folder
    if (SUCCEEDED(hr) && pidlFolder)
    {
        hr = SHGetPathFromIDList(pidlFolder, _szTargetFolder);
    }
    return hr;
}

// IContextMenu
IFACEMETHODIMP CNonDefaultDropMenuVerb::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/)
{
    WCHAR szMenuItem[80];

    LoadString(g_hinst, IDS_CHECKDROP, szMenuItem, ARRAYSIZE(szMenuItem));
    InsertMenu(hmenu, indexMenu++, MF_BYPOSITION, idCmdFirst + IDM_CHECKDROP, szMenuItem);

    LoadString(g_hinst, IDS_CHECKNETRESOURCES, szMenuItem, ARRAYSIZE(szMenuItem));
    InsertMenu(hmenu, indexMenu++, MF_BYPOSITION, idCmdFirst + IDM_CHECKNETRESOURCES, szMenuItem);

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(2));  // indicate that we added 2 verbs.
}

IFACEMETHODIMP CNonDefaultDropMenuVerb::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    HRESULT hr = E_INVALIDARG;  // assume error
    // No need to support string based command.
    if (!HIWORD(lpici->lpVerb))
    {
        UINT const idCmd = LOWORD(lpici->lpVerb);
        switch (idCmd)
        {
        case IDM_CHECKDROP:
            hr = _CheckForHDROP(lpici->hwnd);
            break;

        case IDM_CHECKNETRESOURCES:
            hr = _CheckForHNRES(lpici->hwnd);
            break;
        }
    }
    return hr;
}

HRESULT CNonDefaultDropMenuVerb::_CheckForHDROP(HWND hwnd)
{
    WCHAR szMessageTitle[128];
    LoadString(g_hinst, IDS_MESSAGEBOXTITLE, szMessageTitle, ARRAYSIZE(szMessageTitle));

    FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium;
    HRESULT hr = _pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        HDROP hdrop = (HDROP)medium.hGlobal;
        WCHAR szFile[MAX_PATH];
        DragQueryFile(hdrop, 0, szFile, ARRAYSIZE(szFile));

        UINT cFiles = DragQueryFile(hdrop, (UINT)-1, NULL, 0);

        WCHAR szMessageBase[128];
        LoadString(g_hinst, IDS_MESSAGETEMPLATEFS, szMessageBase, ARRAYSIZE(szMessageBase));

        WCHAR szMessage[INTERNET_MAX_URL_LENGTH];
        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szMessageBase, _szTargetFolder, cFiles, szFile);

        MessageBox(hwnd, szMessage, szMessageTitle, MB_OK);
        ReleaseStgMedium(&medium);
    }
    else
    {
        WCHAR szErrorMessage[128];
        LoadString(g_hinst, IDS_ERRORMESSAGEFS, szErrorMessage, ARRAYSIZE(szErrorMessage));
        MessageBox(hwnd, szErrorMessage, szMessageTitle, MB_OK);
    }
    return hr;
}

PCWSTR _Offset2Ptr(PCWSTR pszBase, size_t cbBase, size_t uOffset)
{
    PWSTR pszRet = NULL;
    if (uOffset && uOffset < cbBase)
    {
        pszRet = (PWSTR)((BYTE *)pszBase + uOffset);
    }
    return pszRet;
}

// fill in pmedium with a NRESARRAY structure. this is a flat clipboard format on an HGLOBAL
// the name of this format is CFSTR_NETRESOURCES
//
// typedef struct
// {
//     UINT cItems;
//     NETRESOURCE nr[1];    // variable # of these, with string pointers converted to offsets
//     <strings stored here>
// } NRESARRAY;

// This is a helper routine which extracts a specified NETRESOURCE from hnres.
UINT CNonDefaultDropMenuVerb::_GetNetResource(HGLOBAL hnres, UINT iItem, NETRESOURCEW *pnresOut, UINT cbMax)
{
    UINT iRet = 0;        // assume error
    NRESARRAY *panr = (NRESARRAY *)GlobalLock(hnres);
    if (panr)
    {
        if (iItem == (UINT)-1)
        {
            iRet = panr->cItems;
        }
        else if (iItem < panr->cItems)
        {
            HRESULT hr = ERROR_INSUFFICIENT_BUFFER;

            size_t const cbResArraySize = GlobalSize(hnres);
            PCWSTR pszProvider   = _Offset2Ptr(reinterpret_cast<PCWSTR>(panr), cbResArraySize, reinterpret_cast<size_t>(panr->nr[iItem].lpProvider));
            PCWSTR pszRemoteName = _Offset2Ptr(reinterpret_cast<PCWSTR>(panr), cbResArraySize, reinterpret_cast<size_t>(panr->nr[iItem].lpRemoteName));
            if (cbMax >= sizeof(*pnresOut))
            {
                *pnresOut = panr->nr[iItem];
                hr = S_OK;

                PWSTR psz = (PWSTR)(pnresOut + 1);
                size_t cch = (cbMax - sizeof(*pnresOut)) / sizeof(*psz);
                if (pnresOut->lpProvider)
                {
                    pnresOut->lpProvider = psz;
                    if (pszProvider)
                    {
                        hr = StringCchCopyExW(psz, cch, pszProvider, &psz, &cch, 0);
                        if (SUCCEEDED(hr))
                        {
                            if (cch) // point after NULL for potential next string, if we have room
                            {
                                psz++;
                                cch--;
                            }
                        }
                    }
                    else
                    {
                        // If this string is Null there was an issue with the NRESARRAY's data
                        hr = E_FAIL;
                    }
                }
                if (pnresOut->lpRemoteName && SUCCEEDED(hr))
                {
                    pnresOut->lpRemoteName = psz;
                    if (pszRemoteName)
                    {
                        hr = StringCchCopy(psz, cch, pszRemoteName);
                    }
                    else
                    {
                        // If this string is Null there was an issue with the NRESARRAY's data
                        hr = E_FAIL;
                    }
                }
            }
            iRet = SUCCEEDED(hr) ? 1 : 0;
        }
        GlobalUnlock(hnres);
    }
    return iRet;
}

HRESULT CNonDefaultDropMenuVerb::_CheckForHNRES(HWND hwnd)
{
    WCHAR szMessageTitle[128];
    LoadString(g_hinst, IDS_MESSAGEBOXTITLE, szMessageTitle, ARRAYSIZE(szMessageTitle));

    static CLIPFORMAT g_cfNetResource = 0;  // Clipboard format
    if (g_cfNetResource == 0)
    {
        g_cfNetResource = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_NETRESOURCES);
    }

    FORMATETC fmte = { g_cfNetResource, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium;
    HRESULT hr = _pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        size_t const cbNetResource = 2048;
        NETRESOURCEW *pnr = (NETRESOURCEW *)LocalAlloc(LPTR, cbNetResource);
        hr = pnr ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            // Get the NETRESOURCE of the first item
            _GetNetResource(medium.hGlobal, 0, pnr, cbNetResource);

            UINT const cItems = _GetNetResource(medium.hGlobal, (UINT)-1, NULL, 0);

            WCHAR szMessageBase[128];
            LoadString(g_hinst, IDS_MESSAGETEMPLATENET, szMessageBase, ARRAYSIZE(szMessageBase));

            WCHAR szNA[128];
            LoadString(g_hinst, IDS_ERRORMESSAGENA, szNA, ARRAYSIZE(szNA));

            WCHAR szMessage[MAX_PATH];
            StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szMessageBase, cItems,
                            pnr->lpProvider, pnr->lpRemoteName ? pnr->lpRemoteName : szNA,
                            pnr->dwDisplayType, pnr->dwType, pnr->dwUsage);

            MessageBox(hwnd, szMessage, szMessageTitle, MB_OK);
            LocalFree(pnr);
        }
        ReleaseStgMedium(&medium);
    }
    else
    {
        WCHAR szErrorMessage[128];
        LoadString(g_hinst, IDS_ERRORMESSAGENET, szErrorMessage, ARRAYSIZE(szErrorMessage));
        MessageBox(hwnd, szErrorMessage, szMessageTitle, MB_OK);
    }
    return hr;
}
