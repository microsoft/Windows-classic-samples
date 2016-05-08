// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// debugging notes:
//      run the program once and it will register itself
//      run it again under the debugger with "-embedding", that will start up the app
//      so you can set breakpoints.

#include "ShellHelpers.h"
#include "COMHelpers.h"
#include "RegisterExtension.h"
#include "ApplicationVerb.h"
#include "PropertyStoreReader.h"
#include <propkey.h>
#include <new>  // std::nothrow

class CPlaylistCreator : public INamespaceWalkCB2
{
public:
    CPlaylistCreator() : _fCountingFiles(TRUE), _cFilesTotal(0), _cFileCur(0),
        _ppd(NULL), _pApp(NULL), _dwThreadID(GetCurrentThreadId())
    { }

    HRESULT CreatePlaylist(IShellItemArray *psia);

    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CPlaylistCreator, INamespaceWalkCB),
            QITABENT(CPlaylistCreator, INamespaceWalkCB2),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    // Delegate ref counting to Application Host
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return _pApp->AddRef();
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        return _pApp->Release();
    }

    // INamespaceWalkCB
    IFACEMETHODIMP FoundItem(IShellFolder *psf, PCUITEMID_CHILD pidl)
    {
        HRESULT hr = S_OK;
        if (_fCountingFiles)
        {
            _cFilesTotal++;
        }
        else
        {
            _cFileCur++;

            IShellItem2 *psi;
            hr = SHCreateItemWithParent(NULL, psf, pidl, IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr))
            {
                PWSTR pszName;
                hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName);
                if (SUCCEEDED(hr))
                {
                    _ppd->SetProgress64(_cFileCur, _cFilesTotal);
                    _ppd->SetLine(2, pszName, TRUE, NULL);

                    hr = _ProcessItem(psi);

                    CoTaskMemFree(pszName);
                }
                psi->Release();
            }
        }
        return _ppd->HasUserCancelled() ? HRESULT_FROM_WIN32(ERROR_CANCELLED) : hr;
    }

    IFACEMETHODIMP EnterFolder(IShellFolder * /* psf */, PCUITEMID_CHILD /* pidl */)
    {
        return S_OK;
    }

    IFACEMETHODIMP LeaveFolder(IShellFolder * /* psf */, PCUITEMID_CHILD /* pidl */)
    {
        return S_OK;
    }

    IFACEMETHODIMP InitializeProgressDialog(PWSTR *ppszTitle, PWSTR *ppszCancel)
    {
        *ppszTitle = NULL;
        *ppszCancel = NULL;
        return E_NOTIMPL;
    }

    IFACEMETHODIMP WalkComplete(HRESULT)
    {
        if (!_fCountingFiles)
        {
            _ExitMessageLoop();
        }
        return S_OK;
    }

    void SetApplication(IUnknown *punkApp)
    {
        _pApp = punkApp;
    }

private:
    HRESULT _GetSaveInFolder(REFIID riid, void **ppv);
    HRESULT _GetPlaylistStream(IStream **ppstm);
    HRESULT _GetPlaylistItem(REFIID riid, void **ppv);
    HRESULT _ProcessItem(IShellItem2 *psi);
    void _ExitMessageLoop()
    {
        PostThreadMessage(_dwThreadID, WM_QUIT, 0, 0);
    }

protected:
    UINT GetTotalFiles() { return _cFilesTotal; }

    virtual PCWSTR GetFileName() = 0;
    virtual HRESULT WriteHeader() { return S_OK; }
    virtual HRESULT WriteFooter() { return S_OK; }
    virtual HRESULT FormatItem(ULONG ulDuration, PCWSTR pszName, PCWSTR pszPath) = 0;

    DWORD _dwThreadID;          // post WM_QUIT here when done
    IStream *_pstm;             // held so the callbacks can use this
    IProgressDialog *_ppd;      // held so the callbacks can use this
    IUnknown *_pApp;            // reference to Application host for proper ref counting
    BOOL _fCountingFiles;       // call back used in the "counting files" mode
    UINT _cFilesTotal;          // total computed in the count
    UINT _cFileCur;             // current, for progress UI
};

// the properties we will be asking for (optimzation for the property store)
PROPERTYKEY const c_rgProps[] =
{
    PKEY_ParsingPath,   // use instead of ItemUrl
    PKEY_PerceivedType,
    PKEY_Media_Duration,
    PKEY_Title,
    PKEY_Music_TrackNumber,
    PKEY_Music_AlbumArtist,
    PKEY_Music_AlbumTitle
};

HRESULT CPlaylistCreator::_ProcessItem(IShellItem2 *psi)
{
    PWSTR pszPath;
    HRESULT hr = GetUNCPathFromItem(psi, &pszPath);
    if (FAILED(hr))
    {
        hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
    }

    if (SUCCEEDED(hr))
    {
        // the property store for this item. note that this binds to the file system
        // property store even for DBFolder items. we want to fix this in the future
        CPropertyStoreReader psr;

        hr = psr.InitFromItem(psi, GPS_DELAYCREATION, c_rgProps, ARRAYSIZE(c_rgProps));
        if (SUCCEEDED(hr))
        {
            long perceivedType; // PERCEIVED
            hr = psr.GetInt32(PKEY_PerceivedType, &perceivedType);
            if (SUCCEEDED(hr) && (PERCEIVED_TYPE_AUDIO == perceivedType))
            {
                ULONGLONG ullDuration;
                hr = psr.GetUInt64(PKEY_Media_Duration, &ullDuration);
                if (SUCCEEDED(hr))
                {
                    ullDuration /= 10000000; // scale ns to seconds

                    PWSTR pszTitle;
                    hr = psr.GetString(PKEY_Title, &pszTitle);
                    if (SUCCEEDED(hr))
                    {
                        ULONG ulTrackNumber;
                        psr.GetUInt32(PKEY_Music_TrackNumber, &ulTrackNumber);

                        PWSTR pszArtist;
                        psr.GetString(PKEY_Music_AlbumArtist, &pszArtist);

                        PWSTR pszAlbum;
                        psr.GetString(PKEY_Music_AlbumTitle, &pszAlbum);

                        FormatItem((ULONG)ullDuration, pszTitle, pszPath);

                        CoTaskMemFree(pszArtist);
                        CoTaskMemFree(pszAlbum);

                        CoTaskMemFree(pszTitle);
                    }
                }
            }
        }
        CoTaskMemFree(pszPath);
    }
    return S_OK;
}

HRESULT CPlaylistCreator::_GetSaveInFolder(REFIID riid, void **ppv)
{
    return SHCreateItemInKnownFolder(FOLDERID_Playlists, KF_FLAG_CREATE, NULL, riid, ppv);
}

HRESULT CPlaylistCreator::_GetPlaylistStream(IStream **ppstm)
{
    *ppstm = NULL;
    IShellItem *psiFolder;
    HRESULT hr = _GetSaveInFolder(IID_PPV_ARGS(&psiFolder));
    if (SUCCEEDED(hr))
    {
        IStorage *pstg;
        hr = psiFolder->BindToHandler(NULL, BHID_Storage, IID_PPV_ARGS(&pstg));
        if (SUCCEEDED(hr))
        {
            hr = pstg->CreateStream(GetFileName(), STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_NONE, 0, 0, ppstm);
            pstg->Release();
        }
        psiFolder->Release();
    }
    return hr;
}

// after successfully creating the playlist this will return the shell item (IShellItem) that was created
HRESULT CPlaylistCreator::_GetPlaylistItem(REFIID riid, void **ppv)
{
    *ppv = NULL;

    IShellItem *psiFolder;
    HRESULT hr = _GetSaveInFolder(IID_PPV_ARGS(&psiFolder));
    if (SUCCEEDED(hr))
    {
        hr = SHCreateItemFromRelativeName(psiFolder, GetFileName(), NULL, riid, ppv);
        psiFolder->Release();
    }
    return hr;
}

HRESULT CPlaylistCreator::CreatePlaylist(IShellItemArray *psia)
{
    INamespaceWalk *pnsw;
    HRESULT hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pnsw));
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_ppd));
        if (SUCCEEDED(hr))
        {
            _ppd->StartProgressDialog(NULL, NULL, PROGDLG_AUTOTIME, NULL);

            _ppd->SetTitle(L"Building Playlist");
            _ppd->SetLine(1, L"Finding music files...", FALSE, NULL);

            hr = pnsw->Walk(psia, NSWF_TRAVERSE_STREAM_JUNCTIONS | NSWF_DONT_ACCUMULATE_RESULT, 4, this);
            if (SUCCEEDED(hr))
            {
                _fCountingFiles = FALSE;
                _ppd->SetLine(1, L"Adding files...", FALSE, NULL);

                hr = _GetPlaylistStream(&_pstm);
                if (SUCCEEDED(hr))
                {
                    hr = WriteHeader();
                    if (SUCCEEDED(hr))
                    {
                        hr = pnsw->Walk(psia, NSWF_TRAVERSE_STREAM_JUNCTIONS | NSWF_DONT_ACCUMULATE_RESULT | NSWF_SHOW_PROGRESS, 4, this);
                        if (SUCCEEDED(hr))
                        {
                            hr = WriteFooter();
                        }
                    }

                    hr = _pstm->Commit(0);
                    SafeRelease(&_pstm);

                    if (SUCCEEDED(hr))
                    {
                        IShellItem *psiCreated;
                        hr = _GetPlaylistItem(IID_PPV_ARGS(&psiCreated));
                        if (SUCCEEDED(hr))
                        {
                            OpenFolderAndSelectItem(psiCreated);
                            psiCreated->Release();
                        }
                    }
                }
            }
            _ppd->StopProgressDialog();
            SafeRelease(&_ppd);
        }
        pnsw->Release();
    }
    _ExitMessageLoop();
    return 0;
}

// CPlaylistCreator derivitive which implements the goo necessary to create WPL playlists.
WCHAR const c_szWPLFileHeader[] =
    L"<?wpl version=\"1.0\"?>\r\n"
    L"<smil>\r\n"
    L"    <head>\r\n"
    L"       <meta name=\"Generator\" content=\"GuzTools -- 0.0.0.2\"/>\r\n"
    L"       <meta name=\"ItemCount\" content=\"%d\"/>\r\n"
    L"       <title>%s</title>\r\n"
    L"    </head>\r\n"
    L"    <body>\r\n"
    L"        <seq>\r\n";

char const c_szWPLFileFooter[] =
    "        </seq>\r\n"
    "    </body>\r\n"
    "</smil>\r\n";

class CWPLPlaylistCreator : public CPlaylistCreator
{
public:

protected:
    virtual PCWSTR GetFileName() { return L"New Playlist.wpl"; }

    HRESULT WriteHeader()
    {
        // _cFilesTotal is the # of items found, not the number of music files
        return IStream_CchPrintfAsUTF8(_pstm, c_szWPLFileHeader, GetTotalFiles(), GetFileName());
    }

    HRESULT WriteFooter()
    {
        return IStream_WriteString(_pstm, c_szWPLFileFooter);
    }

    HRESULT FormatItem(ULONG, PCWSTR, PCWSTR pszPath)
    {
        return IStream_CchPrintfAsUTF8(_pstm, L"            <media src=\"%s\"/>\r\n", pszPath);
    }
};

// Simple CPlaylistCreator derivitive which implements the goo necessary to create M3U playlists.
class CM3UPlaylistCreator : public CPlaylistCreator
{
protected:
    virtual PCWSTR GetFileName() { return L"New Playlist.m3u"; }

    HRESULT WriteHeader()
    {
        return IStream_WriteString(_pstm, "#EXTM3U\r\n");
    }

    HRESULT FormatItem(ULONG ulDuration, PCWSTR pszName, PCWSTR pszPath)
    {
        // #EXTINF:###,Artist  - Title
        return IStream_CchPrintfAsUTF8(_pstm, L"#EXTINF:%d,%s\r\n%s\r\n", (LONG)ulDuration, pszName, pszPath);
    }
};

class CPlaylistCreatorApp : public IUnknown
{
public:
    CPlaylistCreatorApp() : _cRef(1), _psia(NULL)
    {
        // set CPlaylistCreatorApp as the application host on the playlistcreator and verb classes
        _WPLCreator.SetApplication(this);
        _M3UCreator.SetApplication(this);

        _verbCreateWPL.SetApplication(this);
        _verbCreateM3U.SetApplication(this);
    }

    static UINT const WM_APP_CREATE_WPL = WM_APP + 100;
    static UINT const WM_APP_CREATE_M3U = WM_APP + 101;

    void DoMessageLoop()
    {
        _verbCreateWPL.Register();
        _verbCreateM3U.Register();

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_APP_CREATE_WPL)
            {
                _WPLCreator.CreatePlaylist(_psia);
            }
            else if (msg.message == WM_APP_CREATE_M3U)
            {
                _M3UCreator.CreatePlaylist(_psia);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        _verbCreateWPL.UnRegister();
        _verbCreateM3U.UnRegister();
    }

    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CPlaylistCreatorApp, IUnknown),
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

    // to make the verbs async capture the item array and post
    // a message to do the work later
    void CreatePlaylistAsync(UINT msg, IShellItemArray *psia)
    {
        // only allow one at a time
        if (_psia == NULL)
        {
            SetInterface(&_psia, psia);
            PostThreadMessage(GetCurrentThreadId(), msg, 0, 0);
        }
    }

    void SetSite(IUnknown *)
    { }

private:

    ~CPlaylistCreatorApp()
    {
        SafeRelease(&_psia);
    }

    class __declspec(uuid("352d62ad-3b26-4c1f-ad43-c2a4e6dfc916"))
        CCreateWPLPlaylistVerb : public CApplicationVerb<CPlaylistCreatorApp, CCreateWPLPlaylistVerb>
    {
        void DoVerb(IShellItemArray *psia) const
        {
            CPlaylistCreatorApp *pApp = GetApp();
            if (pApp)
            {
                pApp->CreatePlaylistAsync(WM_APP_CREATE_WPL, psia);
            }
        }
    };

    class __declspec(uuid("b011ce4c-1c1b-4a68-9240-d1d8866537e9"))
        CCreateM3UPlaylistVerb : public CApplicationVerb<CPlaylistCreatorApp, CCreateM3UPlaylistVerb>
    {
        void DoVerb(IShellItemArray *psia) const
        {
            CPlaylistCreatorApp *pApp = GetApp();
            if (pApp)
            {
                pApp->CreatePlaylistAsync(WM_APP_CREATE_M3U, psia);
            }
        }
    };

    long _cRef;

    IShellItemArray *_psia;

    CCreateWPLPlaylistVerb _verbCreateWPL;
    CCreateM3UPlaylistVerb _verbCreateM3U;

    CWPLPlaylistCreator _WPLCreator;
    CM3UPlaylistCreator _M3UCreator;
};


PCWSTR const rgAssociationElementsMusic[] =
{
    L"SystemFileAssociations\\Directory.Audio", // music folders
    L"Stack.System.Music",                      // music stacks anywhere
    L"Stack.Audio",                             // stacks in music library
    L"SystemFileAssociations\\Audio",           // music items
};

HRESULT RegisterApp()
{
    CRegisterExtension reCreateWPLPlaylist(__uuidof(CPlaylistCreatorApp::CCreateWPLPlaylistVerb));

    WCHAR const c_szCreateWPLPlaylistDescription[] = L"Create Playlist (.WPL)";
    WCHAR const c_szCreateWPLPlaylistVerb[] = L"CreateWPLPlaylist";

    HRESULT hr = reCreateWPLPlaylist.RegisterPlayerVerbs(rgAssociationElementsMusic, ARRAYSIZE(rgAssociationElementsMusic),
        c_szCreateWPLPlaylistVerb, c_szCreateWPLPlaylistDescription);

    if (SUCCEEDED(hr))
    {
        WCHAR const c_szCreateM3UPlaylistDescription[] = L"Create Playlist (.M3U)";
        WCHAR const c_szCreateM3UPlaylistVerb[] = L"CreateM3UPlaylist";

        CRegisterExtension reCreateM3ULPlaylist(__uuidof(CPlaylistCreatorApp::CCreateM3UPlaylistVerb));

        hr = reCreateM3ULPlaylist.RegisterPlayerVerbs(rgAssociationElementsMusic, ARRAYSIZE(rgAssociationElementsMusic),
            c_szCreateM3UPlaylistVerb, c_szCreateM3UPlaylistDescription);
    }
    return hr;
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR pszCmdLine, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        DisableComExceptionHandling();

        if (!StrStrI(pszCmdLine, L"-Embedding"))
        {
            hr = RegisterApp();
            if (SUCCEEDED(hr))
            {
                MessageBox(NULL, L"Installed 'Create Playlist' verbs for audio files and containers", L"SDK Sample - Playlist Creator", MB_OK);
            }
        }

        CPlaylistCreatorApp *pAppDrop = new (std::nothrow) CPlaylistCreatorApp();
        if (pAppDrop)
        {
            pAppDrop->DoMessageLoop();
            pAppDrop->Release();
        }
        CoUninitialize();
    }
    return 0;
}
