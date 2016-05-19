// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// This demonstrates how implement a shell verb using the DropTarget method
// this method is prefered for verb implementations that need to work on Windows XP
// as it provides the most flexibility, it is simple, and supports out of process activation.
//
// This sample implements a stand alone local server COM object but
// it is expected that the verb implementation will be integreated
// into existing applications. To do that have your main application object
// register a class factory for itself and have that object implement IDropTarget
// for the verbs of your application. Note that COM will launch your application
// if it is not already running and will connect to an already running instance
// of your application if it is already running. These are features of the COM
// based verb implementation methods
//
// It is also possible (but not recomended) to create in process implementations
// of this object. To do that follow this sample but replace the local
// server COM object with an inproc server.

// version note. the DropTarget method works on Windows XP and above
// this sample demonstrates how to use the APIs on XP as well as those o
// Vista or above depending ont he version you compile with...

#define _WIN32_WINNT _WIN32_WINNT_WINXP     // specify _WIN32_WINNT_VISTA if you app requires Vista and above

#include "ShellHelpers.h"
#include "VerbHelpers.h"
#include "RegisterExtension.h"
#include <strsafe.h>
#include <new>  // std::nothrow

// Each verb has to have a unique COM object, run UUIDGEN to
// create new CLSID values for your new verbs

WCHAR const c_szVerbDisplayName[] = L"DropTarget Verb Sample";

// accessor helpers for the HIDA (aka CFSTR_SHELLIDLIST) clipboard format

// get the "folder" part of the IDList, this is the IDList that the children
// are rooted from
__inline PCUIDLIST_ABSOLUTE IDA_GetFolderIDList(CIDA const *pida)
{
    return (PCUIDLIST_ABSOLUTE)(((char *)pida) + pida->aoffset[0]);
}

// get the "child" item part of the IDList, these can be either child items or
// relative, hence PCUIDLIST_RELATIVE return type
__inline PCUIDLIST_RELATIVE IDA_GetItemIDList(CIDA const *pida, int i)
{
    return (PCUIDLIST_RELATIVE)(((char *)pida) + pida->aoffset[i + 1]);
}

// a CIDA structure represents a set of shell items, create the Nth item
// from that set in the form of an IShellItem
//
// this uses XP SP1 APIs so it works downlevel

HRESULT CreateShellItemFromHIDA(CIDA *pida, UINT iItem, IShellItem **ppsi)
{
    *ppsi = NULL;

    HRESULT hr = E_FAIL;
    if (iItem < pida->cidl)
    {
        PIDLIST_ABSOLUTE pidlFolder; // needed for alignment
        hr = SHILCloneFull(IDA_GetFolderIDList(pida), &pidlFolder);
        if (SUCCEEDED(hr))
        {
            PIDLIST_ABSOLUTE pidl;
            hr = SHILCombine(pidlFolder, IDA_GetItemIDList(pida, iItem), &pidl);
            if (SUCCEEDED(hr))
            {
                // cast needed due to overload of the type of the 3rd param, when the first
                // 2 params are NULL this is an absolute IDList
                hr = SHCreateShellItem(NULL, NULL, reinterpret_cast<PCUITEMID_CHILD>(pidl), ppsi);
                CoTaskMemFree(pidl);
            }
            CoTaskMemFree(pidlFolder);
        }
    }
    return hr;
}

// declare a static CLIPFORMAT and pass that that by ref as the first param

__inline CLIPFORMAT GetClipboardFormat(CLIPFORMAT *pcf, PCWSTR pszForamt)
{
    if (*pcf == 0)
    {
        *pcf = (CLIPFORMAT)RegisterClipboardFormat(pszForamt);
    }
    return *pcf;
}

static CLIPFORMAT g_cfHIDA = 0;

// Helper to enable access to IShellItem programming model on XP. this requires the XP SP1 SDK
// return a GlobalAlloc()ed copy of the HIDA clipboard format (CFSTR_SHELLIDLIST)
// use this to create shell items from the data object
// on Vista and above other APIs should be used as these are less flexable and performant

HRESULT DataObj_CopyHIDA(IDataObject *pdtobj, CIDA **ppida)
{
    *ppida = NULL;

    FORMATETC fmte = {GetClipboardFormat(&g_cfHIDA, CFSTR_SHELLIDLIST), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        void const *pSrc = GlobalLock(medium.hGlobal);
        size_t const cbSize = GlobalSize(medium.hGlobal);
        *ppida = (CIDA *)GlobalAlloc(GPTR, cbSize);
        hr = *ppida ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            memcpy(*ppida, pSrc, cbSize);
        }
        ReleaseStgMedium(&medium);
    }
    return hr;
}

class __declspec(uuid("4f0ecd66-5b4d-4821-a73b-aaa64023e19c"))
    CDropTargetVerb : public IDropTarget,
                      public IObjectWithSite,
                      CAppMessageLoop
{
public:
    CDropTargetVerb() : _cRef(1), _pdtobj(NULL), _punkSite(NULL)
    {
    }

    HRESULT Run();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CDropTargetVerb, IDropTarget),         // required
            QITABENT(CDropTargetVerb, IObjectWithSite),     // optional
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
        {
            delete this;
        }
        return cRef;
    }

    // IDropTarget
    // this is the required interface for a verb implemeting the DropTarget method
    // DragEnter is called to enable the implementaiton to zero the output dwEffect
    // value, indicating that the verb does not accept the input data object. this is
    // rarely used.
    IFACEMETHODIMP DragEnter(IDataObject * /* pdtobj */, DWORD /* grfKeyState */, POINTL /* pt */, DWORD * /* pdwEffect */)
    {
        // leave unchanged *pdwEffect to indicate that we can do whatever effect is requested
        return S_OK;
    }

    IFACEMETHODIMP DragOver(DWORD /* grfKeyState */, POINTL /* pt */, DWORD * /* pdwEffect */)
    {
        // leave unchanged *pdwEffect to indicate that we can do whatever effect is requested
        return S_OK;
    }

    IFACEMETHODIMP DragLeave()
    {
        return S_OK;
    }

    // this method is what is called to invoke the verb and is typically the only method
    // that needs to be implemented
    IFACEMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    // IObjectWithSite
    // the IObjectWithSite implementation for a verb is optional, it provides access
    // to the invoking code, in the case of verbs activated in the context of the
    // explorer the programming model of the explorer can be accessed via the site
    IFACEMETHODIMP SetSite(IUnknown *punkSite)
    {
        SetInterface(&_punkSite, punkSite);
        return S_OK;
    }

    IFACEMETHODIMP GetSite(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        return _punkSite ? _punkSite->QueryInterface(riid, ppv) : E_FAIL;
    }

private:
    ~CDropTargetVerb()
    {
        SafeRelease(&_pdtobj);
        SafeRelease(&_punkSite);
    }

    void OnAppCallback()
    {
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA

        // on Vista SHCreateShellItemArrayFromDataObject is supported and will
        // convert the data object into the set of shell items directly, use that if you
        // work on Vista and above

        IShellItemArray *psia;
        HRESULT hr = SHCreateShellItemArrayFromDataObject(_pdtobj, IID_PPV_ARGS(&psia));
        if (SUCCEEDED(hr))
        {
            DWORD count;
            psia->GetCount(&count);

            IShellItem2 *psi;
            hr = GetItemAt(psia, 0, IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr))
            {
                PWSTR pszName;
                hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &pszName);
                if (SUCCEEDED(hr))
                {
                    WCHAR szMsg[128];
                    StringCchPrintf(szMsg, ARRAYSIZE(szMsg), L"%d item(s), first item is named %s", count, pszName);
                    MessageBox(NULL, szMsg, c_szVerbDisplayName, MB_OK | MB_SETFOREGROUND);
                    CoTaskMemFree(pszName);
                }
                psi->Release();
            }
            psia->Release();
        }
#else

#ifdef USE_HDROP
        // this clipboard format contains a set of file sysetm paths, but it is lacking information
        // that is often needed that is present in the IDList form (code below). That incldues
        // if the item is a file or a folder, properties like size, date, attributes, etc and
        // access to the display name without doing the IO to convert the path into a shell item
        //
        // hence the code below is preferred as it is more efficient and can address any type of
        // shell item including non file system forms

        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium;

        HRESULT hr = _pdtobj->GetData(&fmte, &medium);
        if (SUCCEEDED(hr))
        {
            UINT const count = DragQueryFile((HDROP)medium.hGlobal, -1, NULL, 0);
            WCHAR szPath[MAX_PATH];
            DragQueryFile((HDROP)medium.hGlobal, 0, szPath, ARRAYSIZE(szPath));
            WCHAR szMsg[128];
            StringCchPrintf(szMsg, ARRAYSIZE(szMsg), L"%d item(s), first item is named %s", count, szPath);
            MessageBox(NULL, szMsg, c_szVerbDisplayName, MB_OK | MB_SETFOREGROUND);
            ReleaseStgMedium(&medium);
        }
#else
        CIDA *pida;
        HRESULT hr = DataObj_CopyHIDA(_pdtobj, &pida);
        if (SUCCEEDED(hr))
        {
            IShellItem *psi;
            hr = CreateShellItemFromHIDA(pida, 0, &psi);
            if (SUCCEEDED(hr))
            {
                PWSTR pszName;
                hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &pszName);
                if (SUCCEEDED(hr))
                {
                    WCHAR szMsg[128];
                    StringCchPrintf(szMsg, ARRAYSIZE(szMsg), L"%d item(s), first item is named %s", pida->cidl, pszName);
                    MessageBox(NULL, szMsg, c_szVerbDisplayName, MB_OK | MB_SETFOREGROUND);
                    CoTaskMemFree(pszName);
                }
                psi->Release();
            }
            GlobalFree(pida);
        }
#endif

#endif
    }

    long _cRef;
    IDataObject *_pdtobj;   // selected items on XP
    IUnknown *_punkSite;    // optional site, supports IObjectWithSite impl
    HWND _hwnd;
};

// this is the method that is called to invoke the verb
// the data object represents the selection and is converted into a shell item
// array to address the items being acted on.

IFACEMETHODIMP CDropTargetVerb::Drop(IDataObject *pdtobj, DWORD /* grfKeyState */, POINTL /* pt */, DWORD *pdwEffect)
{
    SetInterface(&_pdtobj, pdtobj);

    // capture state from the site, the HWND of the parent (that should not be used for a modal
    // window) but might be useful for positioning the UI of this verb
    IUnknown_GetWindow(_punkSite, &_hwnd);

    QueueAppCallback();

    *pdwEffect = DROPEFFECT_NONE;   // didn't move or copy the file
    return S_OK;
}

// this sample is a local server drop target so it must enter a message loop
// and wait for COM to make calls on the registered object

HRESULT CDropTargetVerb::Run()
{
    CStaticClassFactory<CDropTargetVerb> classFactory(static_cast<IDropTarget*>(this));

    HRESULT hr = classFactory.Register(CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE);
    if (SUCCEEDED(hr))
    {
        MessageLoop();
    }
    return S_OK;
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR pszCmdLine, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        if (StrStrI(pszCmdLine, L"-Embedding"))
        {
            CDropTargetVerb *pAppDrop = new (std::nothrow) CDropTargetVerb();
            if (pAppDrop)
            {
                pAppDrop->Run();
                pAppDrop->Release();
            }
        }
        else
        {
            CRegisterExtension re(__uuidof(CDropTargetVerb));

            WCHAR const c_szProgID[] = L"txtfile";
            WCHAR const c_szVerbName[] = L"DropTargetVerb";

            hr = re.RegisterAppAsLocalServer(c_szVerbDisplayName);
            if (SUCCEEDED(hr))
            {
                // register this verb on .txt files ProgID
                hr = re.RegisterDropTargetVerb(c_szProgID, c_szVerbName, c_szVerbDisplayName);
                if (SUCCEEDED(hr))
                {
                    hr = re.RegisterVerbAttribute(c_szProgID, c_szVerbName, L"NeverDefault");
                    if (SUCCEEDED(hr))
                    {
                        MessageBox(NULL,
                            L"Installed DropTarget Verb Sample for .txt files\n\n"
                            L"right click on a .txt file and choose 'DropTarget Verb Sample' to see this in action",
                            c_szVerbDisplayName, MB_OK);
                    }
                }
            }
        }

        CoUninitialize();
    }

    return 0;
}
