// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "ShellHelpers.h"
#include <strsafe.h>
#include <new>

// file system bind data is a parameter passed to IShellFolder::ParseDisplayName() to
// provide the item information to the file system data source. this will enable
// parsing of items that do not exist and avoiding accessing the disk in the parse operation

// {fc0a77e6-9d70-4258-9783-6dab1d0fe31e}
static const CLSID CLSID_UnknownJunction = { 0xfc0a77e6, 0x9d70, 0x4258, {0x97, 0x83, 0x6d, 0xab, 0x1d, 0x0f, 0xe3, 0x1e} };

class CFileSysBindData : public IFileSystemBindData2
{
public:
    CFileSysBindData() : _cRef(1), _clsidJunction(CLSID_UnknownJunction)
    {
        ZeroMemory(&_fd, sizeof(_fd));
        ZeroMemory(&_liFileID, sizeof(_liFileID));
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CFileSysBindData, IFileSystemBindData),
            QITABENT(CFileSysBindData, IFileSystemBindData2),
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

    // IFileSystemBindData
    IFACEMETHODIMP SetFindData(const WIN32_FIND_DATAW *pfd)
    {
        _fd = *pfd;
        return S_OK;
    }

    IFACEMETHODIMP GetFindData(WIN32_FIND_DATAW *pfd)
    {
        *pfd = _fd;
        return S_OK;
    }

    // IFileSystemBindData2
    IFACEMETHODIMP SetFileID(LARGE_INTEGER liFileID)
    {
        _liFileID = liFileID;
        return S_OK;
    }

    IFACEMETHODIMP GetFileID(LARGE_INTEGER *pliFileID)
    {
        *pliFileID = _liFileID;
        return S_OK;
    }

    IFACEMETHODIMP SetJunctionCLSID(REFCLSID clsid)
    {
        _clsidJunction = clsid;
       return S_OK;
    }

    IFACEMETHODIMP GetJunctionCLSID(CLSID *pclsid)
    {
        HRESULT hr;
        if (CLSID_UnknownJunction == _clsidJunction)
        {
            *pclsid = CLSID_NULL;
            hr = E_FAIL;
        }
        else
        {
            *pclsid = _clsidJunction;   // may be CLSID_NULL (no junction handler case)
            hr = S_OK;
        }
        return hr;
    }

private:
    long _cRef;
    WIN32_FIND_DATAW _fd;
    LARGE_INTEGER _liFileID;
    CLSID _clsidJunction;
};

HRESULT CreateFileSystemBindData(IFileSystemBindData2 **ppfsbd)
{
    *ppfsbd = new (std::nothrow) CFileSysBindData();
    return *ppfsbd ? S_OK : E_OUTOFMEMORY;
}

// "simple parsing" allows you to pass the WIN32_FILE_DATA to the file system data source
// to avoid it having to access the file. this avoids the expense of getting the
// information from the file and allows you to parse items that may not necessarily exist
//
// the find data is passed to the data source via the bind context constructed here

HRESULT CreateFileSysBindCtx(const WIN32_FIND_DATAW *pfd, IBindCtx **ppbc)
{
    *ppbc = NULL;

    IFileSystemBindData2 *pfsbd;
    HRESULT hr = CreateFileSystemBindData(&pfsbd);
    if (SUCCEEDED(hr))
    {
        hr = pfsbd->SetFindData(pfd);
        if (SUCCEEDED(hr))
        {
            IBindCtx *pbc;
            hr = CreateBindCtx(0, &pbc);
            if (SUCCEEDED(hr))
            {
                BIND_OPTS bo = {sizeof(bo), 0, STGM_CREATE, 0};
                hr = pbc->SetBindOptions(&bo);
                if (SUCCEEDED(hr))
                {
                    hr = pbc->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, pfsbd);
                    if (SUCCEEDED(hr))
                    {
                        hr = pbc->QueryInterface(IID_PPV_ARGS(ppbc));
                    }
                }
                pbc->Release();
            }
        }
        pfsbd->Release();
    }
    return hr;
}

class CDummyUnknown : public IPersist
{
public:
    CDummyUnknown(REFCLSID clsid) : _cRef(1), _clsid(clsid)
    {
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CDummyUnknown, IPersist),
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

    // IPersist
    IFACEMETHODIMP GetClassID(CLSID *pclsid)
    {
        *pclsid = _clsid;
        return S_OK;
    }

private:
    long _cRef;
    CLSID _clsid;
};

// create a bind context with a named object

HRESULT CreateBindCtxWithParam(PCWSTR pszParam, IUnknown *punk, IBindCtx **ppbc)
{
    *ppbc = NULL;

    HRESULT hr = CreateBindCtx(0, ppbc);
    if (SUCCEEDED(hr))
    {
        hr = (*ppbc)->RegisterObjectParam(const_cast<PWSTR>(pszParam), punk);
        if (FAILED(hr))
        {
            (*ppbc)->Release();
            *ppbc = NULL;
        }
    }
    return hr;
}

// create a bind context with a dummy unknown parameter that is used to pass flag values
// to operations that accept bind contexts

HRESULT CreateBindCtxWithParam(PCWSTR pszParam, IBindCtx **ppbc)
{
    *ppbc = NULL;

    IUnknown *punk = new (std::nothrow) CDummyUnknown(CLSID_NULL);
    HRESULT hr = punk ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = CreateBindCtxWithParam(pszParam, punk, ppbc);
        punk->Release();
    }
    return hr;
}


// STR_FILE_SYS_BIND_DATA and IFileSystemBindData enable passing the file system information
// that the file system data source needs to perform a parse. this eliminates
// the IO that results when parsing an item and lets items that don't exist to be parsed.
// the helper CreateFileSysBindCtx() internally implements IFileSystemBindData and
// stores an object in the bind context with thh WIN32_FIND_DATA that it is provided

void DemonstrateFileSystemParsingParameters()
{
    WIN32_FIND_DATA fd = {};
    fd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;    // a file (not a folder)
    fd.nFileSizeLow  = (DWORD)-1; // file size is null or an unknown value
    fd.nFileSizeHigh = (DWORD)-1;

    IBindCtx *pbcParse;
    HRESULT hr = CreateFileSysBindCtx(&fd, &pbcParse);
    if (SUCCEEDED(hr))
    {
        // this item does not exist, but it can be parsed given the
        // parameter provided in the bind context

        IShellItem2 *psi;
        hr = SHCreateItemFromParsingName(L"c:\\a.txt", pbcParse, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            psi->Release();
        }

        // this is useful for resources on the network where the IO
        // in these cases is more costly
        hr = SHCreateItemFromParsingName(L"\\\\Server\\Share\\file.txt", pbcParse, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            psi->Release();
        }
        pbcParse->Release();
    }
}

// STR_ITEM_CACHE_CONTEXT provides a context that can be used for caching to a
// data source to speed up parsing of multiple items. the file system data source
// uses this so any clients that will be parsing multiple items should provide
// this to speed up the parsing function.
//
// the cache context object is itself a bind context stored in the bind context under
// the name STR_ITEM_CACHE_CONTEXT passed to the parse operation via the bind
// context that it is stored in

void DemonstrateParsingItemCacheContext()
{
    IBindCtx *pbcItemContext;   // this will be used by the data source for caching
    HRESULT hr = CreateBindCtx(0, &pbcItemContext);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbcParse; // passed to the parse
        hr = CreateBindCtxWithParam(STR_ITEM_CACHE_CONTEXT, pbcItemContext, &pbcParse);
        if (SUCCEEDED(hr))
        {
            IFileSystemBindData2 *pfsbd;
            hr = CreateFileSystemBindData(&pfsbd);
            if (SUCCEEDED(hr))
            {
                hr = pbcParse->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, pfsbd);
                if (SUCCEEDED(hr))
                {
                    // do lots of parsing here passing pbcParse each time

                    WIN32_FIND_DATA fd1 = {};   // a file
                    hr = pfsbd->SetFindData(&fd1);
                    if (SUCCEEDED(hr))
                    {
                        IShellItem2 *psi;
                        hr = SHCreateItemFromParsingName(L"C:\\folder\\file.txt", pbcParse, IID_PPV_ARGS(&psi));
                        if (SUCCEEDED(hr))
                        {
                            psi->Release();
                        }
                    }

                    WIN32_FIND_DATA fd2 = {};   // a file
                    hr = pfsbd->SetFindData(&fd2);
                    if (SUCCEEDED(hr))
                    {
                        IShellItem2 *psi;
                        hr = SHCreateItemFromParsingName(L"C:\\folder\\file.doc", pbcParse, IID_PPV_ARGS(&psi));
                        if (SUCCEEDED(hr))
                        {
                            psi->Release();
                        }
                    }
                }
                pfsbd->Release();
            }
            pbcParse->Release();
        }
        pbcItemContext->Release();
    }
}

// STR_PARSE_PREFER_FOLDER_BROWSING indicates that an item referenced via an http or https
// protocol should be parsed using the file system data source that supports such items
// via the WebDAV redirector. the default parsing these name forms is handled by
// the internet data source. this option lets you select the file system data source instead.
//
// note, unlike the internet data source the file system parsing operation verifies the
// resource is accessable (issuing IOs to the file system) so these will be slower
// than the default parsing behavior.
//
// providing this enables accessing these items as file system items using the file system
// data source getting the behavior you would if you provided a file system path (UNC in this case)

void DemonstratePreferFolderBrowsingParsing()
{
    IBindCtx *pbcParse;
    HRESULT hr = CreateBindCtxWithParam(STR_PARSE_PREFER_FOLDER_BROWSING, &pbcParse);
    if (SUCCEEDED(hr))
    {
        IShellItem2 *psi;
        hr = SHCreateItemFromParsingName(L"http://unknownserver/abc/file.extension", pbcParse, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            // the network path is valid
            SFGAOF sfgaof;
            hr = psi->GetAttributes(SFGAO_FILESYSTEM, &sfgaof); // will return SFGAO_FILESYSTEM
            psi->Release();
        }

        // in combination with the file system bind context data this avoids the IO
        // and still parses the item as a file system item

        IFileSystemBindData2 *pfsbd;
        hr = CreateFileSystemBindData(&pfsbd);
        if (SUCCEEDED(hr))
        {
            hr = pbcParse->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, pfsbd);
            if (SUCCEEDED(hr))
            {
                WIN32_FIND_DATA fd = {};
                fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY; // this is a folder

                hr = pfsbd->SetFindData(&fd);
                if (SUCCEEDED(hr))
                {
                    IShellItem2 *psi;
                    hr = SHCreateItemFromParsingName(L"http://unknownserver/dav/folder", pbcParse, IID_PPV_ARGS(&psi));
                    if (SUCCEEDED(hr))
                    {
                        SFGAOF sfgaof;
                        hr = psi->GetAttributes(SFGAO_FILESYSTEM, &sfgaof); // will return SFGAO_FILESYSTEM
                        psi->Release();
                    }
                }
            }
            pfsbd->Release();
        }
        pbcParse->Release();
    }
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        DemonstrateFileSystemParsingParameters();
        DemonstrateParsingItemCacheContext();
        DemonstratePreferFolderBrowsingParsing();
        CoUninitialize();
    }
    return 0;
}
