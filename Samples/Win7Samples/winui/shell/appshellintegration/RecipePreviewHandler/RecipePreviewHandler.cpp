// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma comment(lib, "shlwapi.lib")

#include <shlwapi.h>
#include <shobjidl.h>   // IPreviewHandler, IShellItem, IInitializeWithItem, IParentAndItem
#include <richedit.h>   // MSFTEDIT_CLASS
#include <msxml6.h>     // for xml-related interfaces (to parse the custom .recipe file format)
#include <assert.h>     // for assert
#include <new>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#define MAX_RICHEDIT_READ (64 * 1024) // 64k

inline int RECTWIDTH(const RECT &rc)
{
    return (rc.right - rc.left);
}

inline int RECTHEIGHT(const RECT &rc )
{
    return (rc.bottom - rc.top);
}

class CRecipePreviewHandler : public IObjectWithSite,
                              public IPreviewHandler,
                              public IOleWindow,
                              public IInitializeWithStream
{
public:
    CRecipePreviewHandler() : _cRef(1), _hwndParent(NULL), _hwndPreview(NULL),  _hinstEditLibrary(NULL),
        _pStyleSheetNode(NULL), _pStream(NULL), _punkSite(NULL)
    {
    }

    virtual ~CRecipePreviewHandler()
    {
        if (_hwndPreview)
        {
            DestroyWindow(_hwndPreview);
        }
        if (_hinstEditLibrary)
        {
            FreeLibrary(_hinstEditLibrary);
        }
        SafeRelease(&_punkSite);
        SafeRelease(&_pStream);
        SafeRelease(&_pStyleSheetNode);
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        static const QITAB qit[] =
        {
            QITABENT(CRecipePreviewHandler, IObjectWithSite),
            QITABENT(CRecipePreviewHandler, IOleWindow),
            QITABENT(CRecipePreviewHandler, IInitializeWithStream),
            QITABENT(CRecipePreviewHandler, IPreviewHandler),
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
        ULONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown *punkSite);
    IFACEMETHODIMP GetSite(REFIID riid, void **ppv);

    // IPreviewHandler
    IFACEMETHODIMP SetWindow(HWND hwnd, const RECT *prc);
    IFACEMETHODIMP SetFocus();
    IFACEMETHODIMP QueryFocus(HWND *phwnd);
    IFACEMETHODIMP TranslateAccelerator(MSG *pmsg);
    IFACEMETHODIMP SetRect(const RECT *prc);
    IFACEMETHODIMP DoPreview();
    IFACEMETHODIMP Unload();

    // IOleWindow
    IFACEMETHODIMP GetWindow(HWND *phwnd);
    IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

private:
    HRESULT _CreatePreviewWindow(PCWSTR pszRtf);
    HRESULT _CreateStyleSheetNode();

    long _cRef;
    HWND                    _hwndParent;        // parent window that hosts the previewer window; do NOT DestroyWindow this
    RECT                    _rcParent;          // bounding rect of the parent window
    HWND                    _hwndPreview;       // the actual previewer window
    HINSTANCE               _hinstEditLibrary;  // the library that lets us create a richedit control
    IUnknown                *_punkSite;         // site pointer from host, used to get _pFrame
    IXMLDOMNode             *_pStyleSheetNode;  // an xml node representing the style-sheet to use for formatting
    IStream                 *_pStream;          // the stream for the file we are previewing
};

// IPreviewHandler
// This method gets called when the previewer gets created
HRESULT CRecipePreviewHandler::SetWindow(HWND hwnd, const RECT *prc)
{
    if (hwnd && prc)
    {
        _hwndParent = hwnd; // cache the HWND for later use
        _rcParent   = *prc; // cache the RECT for later use

        if (_hwndPreview)
        {
            // Update preview window parent and rect information
            SetParent(_hwndPreview, _hwndParent);
            SetWindowPos(_hwndPreview, NULL, _rcParent.left, _rcParent.top,
                RECTWIDTH(_rcParent), RECTHEIGHT(_rcParent), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
    return S_OK;
}

HRESULT CRecipePreviewHandler::SetFocus()
{
    HRESULT hr = S_FALSE;
    if (_hwndPreview)
    {
        ::SetFocus(_hwndPreview);
        hr = S_OK;
    }
    return hr;
}

HRESULT CRecipePreviewHandler::QueryFocus(HWND *phwnd)
{
    HRESULT hr = E_INVALIDARG;
    if (phwnd)
    {
        *phwnd = ::GetFocus();
        if (*phwnd)
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

HRESULT CRecipePreviewHandler::TranslateAccelerator(MSG *pmsg)
{
    HRESULT hr = S_FALSE;
    IPreviewHandlerFrame *pFrame = NULL;
    if (_punkSite && SUCCEEDED(_punkSite->QueryInterface(&pFrame)))
    {
        // If your previewer has multiple tab stops, you will need to do appropriate first/last child checking.
        // This particular sample previewer has no tabstops, so we want to just forward this message out
        hr = pFrame->TranslateAccelerator(pmsg);
        SafeRelease(&pFrame);
    }
    return hr;
}

// This method gets called when the size of the previewer window changes (user resizes the Reading Pane)
HRESULT CRecipePreviewHandler::SetRect(const RECT *prc)
{
    HRESULT hr = E_INVALIDARG;
    if (prc)
    {
        _rcParent = *prc;
        if (_hwndPreview)
        {
            // Preview window is already created, so set its size and position
            SetWindowPos(_hwndPreview, NULL, _rcParent.left, _rcParent.top,
                RECTWIDTH(_rcParent), RECTHEIGHT(_rcParent), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        hr = S_OK;
    }

    return hr;
}

// The main method that extracts relevant information from the file stream and
// draws content to the previewer window
HRESULT CRecipePreviewHandler::DoPreview()
{
    HRESULT hr = E_FAIL;
    if (_hwndPreview == NULL && _pStream) // cannot call more than once (Unload should be called before another DoPreview)
    {
        IXMLDOMDocument *pDomDoc;
        // We will use the DOM interface from MSXML to load up our file (which is xml format)
        // and convert it to rich text format using an xml style-sheet
        hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDomDoc));
        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL vfSuccess = VARIANT_FALSE;
            VARIANT vtXmlSource = {0};
            V_VT(&vtXmlSource) = VT_UNKNOWN;
            V_UNKNOWN(&vtXmlSource) = static_cast<IUnknown *>(_pStream);
            hr = pDomDoc->load(vtXmlSource, &vfSuccess);

            if (vfSuccess != VARIANT_TRUE)
            {
                hr = FAILED(hr) ? hr : E_FAIL; // keep failed hr
            }

            if (SUCCEEDED(hr))
            {
                if (_pStyleSheetNode == NULL)
                {
                    hr = _CreateStyleSheetNode();
                }

                if (SUCCEEDED(hr))
                {
                    BSTR bstrRtf;
                    hr  = pDomDoc->transformNode(_pStyleSheetNode, &bstrRtf);
                    if (SUCCEEDED(hr))
                    {
                        hr = _CreatePreviewWindow(bstrRtf);
                        SysFreeString(bstrRtf);
                    }
                }
            }
            pDomDoc->Release();
        }
    }
    return hr;
}

// This method gets called when a shell item is de-selected in the listview
HRESULT CRecipePreviewHandler::Unload()
{
    SafeRelease(&_pStream);
    if (_hwndPreview)
    {
        DestroyWindow(_hwndPreview);
        _hwndPreview = NULL;
    }
    return S_OK;
}

// IObjectWithSite methods
HRESULT CRecipePreviewHandler::SetSite(IUnknown *punkSite)
{
    SafeRelease(&_punkSite);
    return punkSite ? punkSite->QueryInterface(&_punkSite) : S_OK;
}

HRESULT CRecipePreviewHandler::GetSite(REFIID riid, void **ppv)
{
    *ppv = NULL;
    return _punkSite ? _punkSite->QueryInterface(riid, ppv) : E_FAIL;
}

// IOleWindow methods
HRESULT CRecipePreviewHandler::GetWindow(HWND* phwnd)
{
    HRESULT hr = E_INVALIDARG;
    if (phwnd)
    {
        *phwnd = _hwndParent;
        hr = S_OK;
    }
    return hr;
}

HRESULT CRecipePreviewHandler::ContextSensitiveHelp(BOOL)
{
    return E_NOTIMPL;
}

// IInitializeWithStream methods
// This method gets called when an item gets selected in listview
HRESULT CRecipePreviewHandler::Initialize(IStream *pStream, DWORD)
{
    HRESULT hr = E_INVALIDARG;
    if (pStream)
    {
        // Initialize can be called more than once, so release existing valid _pStream
        SafeRelease(&_pStream);

        _pStream = pStream;
        _pStream->AddRef();
        hr = S_OK;
    }

    return hr;
}

// Helper method to post text content to the previewer window
HRESULT CRecipePreviewHandler::_CreatePreviewWindow(PCWSTR pszRtfWide)
{
    assert(_hwndPreview == NULL);

    HRESULT hr = E_FAIL;
    if (_hinstEditLibrary == NULL)
    {
        // MSFTEDIT_CLASS used below comes from this binary
        _hinstEditLibrary = LoadLibraryW(L"msftedit.dll");
    }

    if (_hinstEditLibrary == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        // Create the preview window
        _hwndPreview = CreateWindowExW(0, MSFTEDIT_CLASS, NULL,
                                       WS_CHILD | WS_VSCROLL | WS_VISIBLE | ES_MULTILINE | ES_READONLY,  // Always create read-only
                                       _rcParent.left, _rcParent.top, RECTWIDTH(_rcParent), RECTHEIGHT(_rcParent),
                                       _hwndParent, NULL, NULL,NULL);
        if (_hwndPreview == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            // Let's limit ourselves to MAX_RICHEDIT_READ (64k)
            SendMessage(_hwndPreview, EM_EXLIMITTEXT, 0, MAX_RICHEDIT_READ);

            size_t const cchRtf = 1 + wcslen(pszRtfWide);
            PSTR pszRtf = (PSTR)CoTaskMemAlloc(cchRtf);
            if (pszRtf)
            {
                WideCharToMultiByte(CP_ACP, 0, pszRtfWide, cchRtf, pszRtf, cchRtf, NULL, NULL);
                SETTEXTEX st = { ST_DEFAULT, CP_ACP };

                SendMessage(_hwndPreview, EM_SETTEXTEX, (WPARAM) &st, (LPARAM) pszRtf);
                ShowWindow(_hwndPreview, SW_SHOW);

                CoTaskMemFree(pszRtf);
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

// Helper method to create an xml node from the an xml string that represents the style-sheet
HRESULT CRecipePreviewHandler::_CreateStyleSheetNode()
{
    IXMLDOMDocument *pDomDoc;
    HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDomDoc));
    if (SUCCEEDED(hr))
    {
        // The style-sheet to use for formatting xml as rich text
        // We do this only once
        BSTR bstrStyleSheet = SysAllocString(L"<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n"
                                             L"    <xsl:output method=\"text\" version=\"1.0\" encoding=\"UTF-8\" indent=\"no\"/>\n"
                                             L"    <xsl:template match=\"Recipe\">{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Segoe UI;}{\\f1\\fnil\\fprq2\\fcharset0 Segoe Print;}}\\viewkind4\\uc1\\pard\\f0\\fs36 <xsl:value-of select=\"Title\"/>\\par\n"
                                             L"\\fs20 <xsl:value-of select=\"Background/Author\"/>\\par\n"
                                             L"<xsl:value-of select=\"Comments\"/>\\f1\\par\n"
                                             L"\\par\n"
                                             L"\\b\\f0 Ingredients:\\par\n"
                                             L"\\pard\\tx270\\tx720\\b0\n"
                                             L"<xsl:for-each select=\"Ingredients/Item\">\n"
                                             L"<xsl:value-of select=\"@Quantity\"/>\\tab <xsl:value-of select=\"@Unit\"/>\\tab <xsl:value-of select=\".\"/>\\par\n"
                                             L"</xsl:for-each>\n"
                                             L"\\pard\\par\n"
                                             L"\\b Directions:\\par\n"
                                             L"\\b0\n"
                                             L"<xsl:for-each select=\"Directions/Step\">\n"
                                             L"<xsl:value-of select=\".\"/>\\par\n"
                                             L"\\par\n"
                                             L"</xsl:for-each>\n"
                                             L"Yield: <xsl:value-of select=\"RecipeInfo/Yield\"/>\\par\n"
                                             L"Preparation Time: <xsl:value-of select=\"RecipeInfo/PreparationTime\"/>\\par\n"
                                             L"Cook Time: <xsl:value-of select=\"RecipeInfo/CookTime\"/>\\par\n"
                                             L"}\n"
                                             L"    </xsl:template>\n"
                                             L"</xsl:stylesheet>\n");

        if (bstrStyleSheet == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            VARIANT_BOOL vfSuccess = VARIANT_FALSE;
            hr = pDomDoc->loadXML(bstrStyleSheet, &vfSuccess);
            if (vfSuccess != VARIANT_TRUE)
            {
                hr = FAILED(hr) ? hr : E_FAIL; // keep failed hr
            }

            if (SUCCEEDED(hr))
            {
                hr = pDomDoc->QueryInterface(&_pStyleSheetNode);
            }
            SysFreeString(bstrStyleSheet);
        }
        pDomDoc->Release();
    }
    return hr;
}

HRESULT CRecipePreviewHandler_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;

    CRecipePreviewHandler *pNew = new (std::nothrow) CRecipePreviewHandler();
    HRESULT hr = pNew ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pNew->QueryInterface(riid, ppv);
        pNew->Release();
    }
    return hr;
}
