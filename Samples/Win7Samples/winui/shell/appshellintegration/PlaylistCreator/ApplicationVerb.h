// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// CApplicationVerb is a template class that is used to implement shell verbs
// that integrate with an application using the DelegateExecute verb invoke method.
// the DelegateExecute method has a number of advantages over CommandLine and
// ContextMenuHandler methods discussed in the shell verb implementers guide.
// this could be adapted to use the DropTarget method to enable operation on Windows XP
//
// this design assumes that the application is expressed as a C++ class with
// methods that the verb implementation can call to do its work
//
// to use this class
// 1) run uuidgen.exe to generate a new CLSID for each verb you will implement
//
// 2) for each verb define a class using CApplicationVerb<>, provide the CLSID
//    of the verb using __declspec(uuid()), declare the DoVerb() method
//    this can be a nested class in the application class.
//
//        class __declspec(uuid("4a4f70f8-0f4d-46dc-a4ee-3611308d885f"))
//            CPlayVerb : public CApplicationVerb<CPlayerApplication, CPlayVerb>
//        {
//            CPlayVerb() : CApplicationVerb(AVF_DEFAULT) {}
//            void StartVerb() const;
//            void OnItem(IShellItem *psi) const;
//        };
//
// 3) provide the implementation of the verb by implementing DoVerb(), this can
//    be inline in the class declaration
//
//        class __declspec(uuid("4a4f70f8-0f4d-46dc-a4ee-3611308d885f"))
//            CPlayVerb : public CApplicationVerb<CPlayerApplication, CPlayVerb>
//        {
//            CPlayVerb() : CApplicationVerb(AVF_DEFAULT) {}
//            void StartVerb() const
//            {
//                CPlayerApplication *pApp = GetApp();
//                if (pApp)
//                {
//                    // per verb invocation setup step
//                }
//            }
//            void OnItem(IShellItem *psi) const
//            {
//                CPlayerApplication *pApp = GetApp();
//                if (pApp)
//                {
//                    // process item
//                }
//            }
//        };
//
// 4) delcare an instance of each verb in your application class
//
//    class CPlayerApplication
//    {
//    private:
//        CPlayVerb _verbPlay;
//    };
//
// 5) set the host application on the CPlayVerb members in the contructor of the class that hosts it
//
//    _verbPlay.SetApplication(this);
//
// 6) at application startup time call CApplicationVerb<>.Register()
//
//    void CPlayerApplication::_OnInitDlg()
//    {
//        _verbPlay.Register();
//    }
//
// 7) at app shutdown time call CApplicationVerb<>.UnRegister() to unregister the verb
//
//    void CPlayerApplication::_OnDestroyDlg()
//    {
//        _verbPlay.UnRegister();
//    }
//
// 7) register the verbs in the registry as COM local servers that launches your
//    application. note your app will get passed the "-Embedding" when COM
//    launches you. you can use CRegisterExtension.RegisterAppAsLocalServer() to do this

#include <objbase.h>

// template parameters:
//
// TApplication - the application class that provides methods for the verb implementation
// to call to do its work
//
// TVerb - the verb class itself, used to determine the CLSID value of the COM object
// that implements the verb
//
// this class combines the drop target and the class factory into a single object
// and it expects to be declared as a member variable of the application calls
// giving it a lifetime that matches the application

typedef enum
{
    AVF_DEFAULT             = 0x0000,
    AVF_ASYNC               = 0x0001,   // invoke the verb using the async methods provided by the app
    AVF_ONE_IMPLIES_ALL     = 0x0002,   // implement one implies all behavior
} APPLICATION_VERB_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS(APPLICATION_VERB_FLAGS);

template <class TApplication, class TVerb> class CApplicationVerb :
    public IExecuteCommand,
    public IObjectWithSelection,
    public IObjectWithSite,
    public IClassFactory,
    public INamespaceWalkCB2
{
public:

    CApplicationVerb(APPLICATION_VERB_FLAGS flags = AVF_DEFAULT) : _flags(flags),
        _pApp(NULL), _dwRegisterClass(0), _psia(NULL), _punkSite(NULL), _grfKeyState(0)
    { }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CApplicationVerb, IExecuteCommand),        // required
            QITABENT(CApplicationVerb, IObjectWithSelection),   // required
            QITABENT(CApplicationVerb, IObjectWithSite),        // used to get to the view
            QITABENT(CApplicationVerb, IClassFactory),          // object is its own class factory
            QITABENT(CApplicationVerb, INamespaceWalkCB),
            QITABENT(CApplicationVerb, INamespaceWalkCB2),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return _pApp->AddRef();
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        return _pApp->Release();
    }

    // IExecuteCommand
    IFACEMETHODIMP SetKeyState(DWORD grfKeyState)
    {
        _grfKeyState = grfKeyState;
        return S_OK;
    }

    IFACEMETHODIMP SetParameters(PCWSTR /* pszParameters */)
    { return S_OK; }

    IFACEMETHODIMP SetPosition(POINT /* pt */)
    {
        // to respect the monitor launched from capture this value
        return S_OK;
    }

    IFACEMETHODIMP SetShowWindow(int /* nShow */)
    { return S_OK; }

    IFACEMETHODIMP SetNoShowUI(BOOL /* fNoShowUI */)
    { return S_OK; }

    IFACEMETHODIMP SetDirectory(PCWSTR /* pszDirectory */)
    { return S_OK; }

    IFACEMETHODIMP Execute()
    {
        HRESULT hr;
        if (_flags & AVF_ASYNC)
        {
            INamespaceWalk *pnsw;
            hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pnsw));
            if (SUCCEEDED(hr))
            {
                StartVerb();

                // try to get the items from the view if they are available
                IShellView *psv;
                hr = IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARGS(&psv));

                DWORD const walkFlags = (_flags & AVF_ONE_IMPLIES_ALL) ?
                    NSWF_ONE_IMPLIES_ALL | NSWF_DONT_ACCUMULATE_RESULT | NSWF_ASYNC | NSWF_FLAG_VIEWORDER :
                                           NSWF_DONT_ACCUMULATE_RESULT | NSWF_ASYNC | NSWF_FLAG_VIEWORDER;
                UINT const walkDepth = /* psv ? 0 : */ 8;

                hr = pnsw->Walk(psv ? static_cast<IUnknown*>(psv) : _psia,
                    walkFlags, walkDepth, this);

                SafeRelease(&psv);  // optional

                pnsw->Release();
            }
        }
        else if (_psia)
        {
            DoVerb(_psia);      // provided by the class using this template
        }
        SafeRelease(&_psia);    // don't need this any more
        return S_OK;
    }

    // IObjectWithSelection
    IFACEMETHODIMP SetSelection(IShellItemArray *psia)
    {
        SetInterface(&_psia, psia);
        return S_OK;
    }

    IFACEMETHODIMP GetSelection(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        return _psia ? _psia->QueryInterface(riid, ppv) : E_FAIL;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        if (punkOuter)
        {
            *ppv = NULL;
            return CLASS_E_NOAGGREGATION;
        }
        return QueryInterface(riid, ppv);   // return this object
    }

    IFACEMETHODIMP LockServer(BOOL)
    {
        return S_OK;
    }

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown *punkSite)
    {
        SetInterface(&_punkSite, punkSite);
        __if_exists(TAppliation::SetSite)
        {
            _pApp->SetSite(punkSite);
        }
        return S_OK;
    }

    IFACEMETHODIMP GetSite(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        return _punkSite ? _punkSite->QueryInterface(riid, ppv) : E_FAIL;
    }

    // INamespaceWalkCB
    IFACEMETHODIMP FoundItem(IShellFolder *psf, PCUITEMID_CHILD pidl)
    {
        IShellItem2 *psi;
        HRESULT hr = SHCreateItemWithParent(NULL, psf, pidl, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            OnItem(psi);
            psi->Release();
        }
        return ShouldContinue() ? hr : HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    IFACEMETHODIMP EnterFolder(IShellFolder * /* psf */, PCUITEMID_CHILD /* pidl */)
    {
        return ShouldContinue() ? S_OK : HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    IFACEMETHODIMP LeaveFolder(IShellFolder * /* psf */, PCUITEMID_CHILD /* pidl */)
    {
        return ShouldContinue() ? S_OK : HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    IFACEMETHODIMP InitializeProgressDialog(PWSTR *ppszTitle, PWSTR *ppszCancel)
    {
        *ppszTitle = *ppszCancel = NULL;
        return E_NOTIMPL;
    }

    // INamespaceWalkCB
    IFACEMETHODIMP WalkComplete(HRESULT /* hr */)
    {
        EndVerb();
        return S_OK;
    }

    // application calls this when it is being created to enable the verbs
    void Register()
    {
        CoRegisterClassObject(__uuidof(TVerb), static_cast<IClassFactory *>(this),
            CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &_dwRegisterClass);
    }

    // calls this on shutdown to revoke the verb registration
    void UnRegister()
    {
        if (_dwRegisterClass)
        {
            CoRevokeClassObject(_dwRegisterClass);
            _dwRegisterClass = 0;
        }
    }

    // used in the verb implementations to get a ref to the app
    // the verb calls methods on the app to do its work
    TApplication *GetApp() const { return _pApp; }

    // call this in the constructor of the application to enable the verb to
    // access the application
    void SetApplication(TApplication *ptApp)
    {
        _pApp = ptApp; // weak reference to application
    }

    DWORD GetKeyState() const { return _grfKeyState; }

    virtual ~CApplicationVerb()
    {
        SafeRelease(&_psia);
        SafeRelease(&_punkSite);
    }

private:
    // This method is disabled.
    // User-defined assignment operator is necessary for /W4 /WX since a default
    // one cannot be created by the compiler because this class contains const members.
    CApplicationVerb & operator=(const CApplicationVerb &);

    // verb implementation provides these methods to get the verb behavior
    // some are required and some are optional

    // this is called when a verb is being invoked synchronously with the items
    // implement this for sync verbs
    virtual void DoVerb(IShellItemArray * /* psia */) const { };

    // indicates that the verb has been invoked using the async method
    // and items will be returned to the verb via OnItem()
    virtual void StartVerb() { };

    // the discovery if items in the async verb has is complete
    virtual void EndVerb() { };

    // as the items are being discovered this method is called to see
    // if the discovery should continue
    virtual bool ShouldContinue() const { return _dwRegisterClass != 0; }

    // this method is called with each item when async discovery of the
    // items is enabled
    virtual void OnItem(IShellItem * /* psi */) { };

    TApplication *_pApp;
    DWORD _dwRegisterClass;

    APPLICATION_VERB_FLAGS const _flags;    // controls behavior of verb
    IShellItemArray *_psia;
    IUnknown *_punkSite;
    DWORD _grfKeyState;
};
