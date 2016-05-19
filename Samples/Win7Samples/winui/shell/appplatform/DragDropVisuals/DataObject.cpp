// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Sample data object implementation that demonstrates how to leverage the
// shell provided data object for the SetData() support

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <new>  // std::nothrow
#include "shellhelpers.h"

void DllAddRef() {} // implement these if this is used in a DLL
void DllRelease() {}

class CDataObject : public IDataObject
{
public:
    CDataObject() : _cRef(1), _pdtobjShell(NULL)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CDataObject, IDataObject),
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
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IDataObject
    IFACEMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);

    IFACEMETHODIMP GetDataHere(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP QueryGetData(FORMATETC *pformatetc);

    IFACEMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pFormatetcOut)
    {
        *pFormatetcOut = *pformatetcIn;
        pFormatetcOut->ptd = NULL;
        return DATA_S_SAMEFORMATETC;
    }
    IFACEMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
    IFACEMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);

    IFACEMETHODIMP DAdvise(FORMATETC* /* pformatetc */, DWORD /* advf */, IAdviseSink* /* pAdvSnk */, DWORD* /* pdwConnection */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP DUnadvise(DWORD /* dwConnection */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP EnumDAdvise(IEnumSTATDATA** /* ppenumAdvise */)
    {
        return E_NOTIMPL;
    }

private:
    ~CDataObject()
    {
        SafeRelease(&_pdtobjShell);
        DllRelease();
    }

    HRESULT _EnsureShellDataObject()
    {
        // the shell data object imptlements ::SetData() in a way that will store any format
        // this code delegates to that implementation to avoid having to implement ::SetData()
        return _pdtobjShell ? S_OK : SHCreateDataObject(NULL, 0, NULL, NULL, IID_PPV_ARGS(&_pdtobjShell));
    }

    long _cRef;

    IDataObject *_pdtobjShell;
};

WCHAR const c_szText[] = L"Clipboard Contents";

STDMETHODIMP CDataObject::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    ZeroMemory(pmedium, sizeof(*pmedium));

    HRESULT hr = DATA_E_FORMATETC;
    if (pformatetcIn->cfFormat == CF_UNICODETEXT)
    {
        if (pformatetcIn->tymed & TYMED_HGLOBAL)
        {
            UINT cb = sizeof(c_szText[0]) * (lstrlen(c_szText) + 1);
            HGLOBAL h = GlobalAlloc(GPTR, cb);
            hr = h ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                StringCbCopy((PWSTR)h, cb, c_szText);
                pmedium->hGlobal = h;
                pmedium->tymed = TYMED_HGLOBAL;
            }
        }
    }
    else if (SUCCEEDED(_EnsureShellDataObject()))
    {
        hr = _pdtobjShell->GetData(pformatetcIn, pmedium);
    }
    return hr;
}

IFACEMETHODIMP CDataObject::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    HRESULT hr = _EnsureShellDataObject();
    if (SUCCEEDED(hr))
    {
        hr = _pdtobjShell->SetData(pformatetc, pmedium, fRelease);
    }
    return hr;
}

STDMETHODIMP CDataObject::QueryGetData(FORMATETC *pformatetc)
{
    HRESULT hr = S_FALSE;
    if (pformatetc->cfFormat == CF_UNICODETEXT)
    {
        hr = S_OK;
    }
    else if (SUCCEEDED(_EnsureShellDataObject()))
    {
        hr = _pdtobjShell->QueryGetData(pformatetc);
    }
    return hr;
}

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
    *ppenumFormatEtc = NULL;
    HRESULT hr = E_NOTIMPL;
    if (dwDirection == DATADIR_GET)
    {
        FORMATETC rgfmtetc[] =
        {
            // the order here defines the accuarcy of rendering
            { CF_UNICODETEXT,          NULL, 0,  0, TYMED_HGLOBAL },
        };
        hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppenumFormatEtc);
    }
    return hr;
}

STDAPI CDataObject_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    CDataObject *p = new (std::nothrow) CDataObject();
    HRESULT hr = p ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = p->QueryInterface(riid, ppv);
        p->Release();
    }
    return hr;
}
