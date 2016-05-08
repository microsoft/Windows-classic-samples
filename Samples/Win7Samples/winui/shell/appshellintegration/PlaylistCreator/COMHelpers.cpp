// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "COMHelpers.h"
#include "ShellHelpers.h"

#pragma comment(lib, "crypt32.lib")     // these helpers have these dependancies
#pragma comment(lib, "shlwapi.lib")     // link to this
#pragma comment(lib, "mpr.lib")

HRESULT IStream_WriteString(IStream *pstm, PCWSTR pszBuf)
{
    return IStream_Write(pstm, pszBuf, sizeof(*pszBuf) * lstrlenW(pszBuf));
}

HRESULT IStream_WriteString(IStream *pstm, PCSTR pszBuf)
{
    return IStream_Write(pstm, pszBuf, sizeof(*pszBuf) * lstrlenA(pszBuf));
}

HRESULT IStream_WriteStringAsUTF8(IStream *pstm, PCWSTR pszBuf)
{
    char szBufA[2048];
    WideCharToMultiByte(CP_UTF8, 0, pszBuf, -1, szBufA, ARRAYSIZE(szBufA), NULL, NULL);
    return IStream_WriteString(pstm, szBufA);
}

// write to a UTF8 stream

HRESULT IStream_CchPrintfAsUTF8(IStream *pstm, PCWSTR pszKeyFormatString, ...)
{
    va_list argList;
    va_start(argList, pszKeyFormatString);

    WCHAR szBuffer[2048];
    HRESULT hr = StringCchVPrintf(szBuffer, ARRAYSIZE(szBuffer), pszKeyFormatString, argList);
    if (SUCCEEDED(hr))
    {
        hr = IStream_WriteStringAsUTF8(pstm, szBuffer);
    }

    va_end(argList);

    return hr;
}

HRESULT IStream_ReadToBuffer(IStream *pstm, UINT uMaxSize,
                             BYTE **ppBytes, UINT *pcBytes)
{
    *ppBytes = NULL;
    *pcBytes = 0;

    ULARGE_INTEGER uli;
    HRESULT hr = IStream_Size(pstm, &uli);
    if (SUCCEEDED(hr))
    {
        const ULARGE_INTEGER c_uliMaxSize = { uMaxSize };

        hr = (uli.QuadPart < c_uliMaxSize.QuadPart) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            BYTE *pdata = (BYTE*)LocalAlloc(LPTR, uli.LowPart);
            hr = pdata ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                hr = IStream_Read(pstm, pdata, uli.LowPart);
                if (SUCCEEDED(hr))
                {
                    *ppBytes = pdata;
                    *pcBytes = uli.LowPart;
                }
                else
                {
                    LocalFree(pdata);
                }
            }
        }
    }
    return hr;
}

HRESULT OpenFolderAndSelectItem(IShellItem *psi)
{
    PIDLIST_ABSOLUTE pidl;
    HRESULT hr = SHGetIDListFromObject(psi, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
        CoTaskMemFree(pidl);
    }
    return hr;
}

typedef BOOL (WINAPI* PFNGETNETRESOURCEFROMLOCALPATH)(PCWSTR pcszPath, PWSTR pszNameBuf, DWORD cchNameBufLen, DWORD *pdwNetType);

STDAPI_(BOOL) GetNetResourceFromLocalPath(PCWSTR pcszPath, PWSTR pszNameBuf, DWORD cchNameBufLen, DWORD *pdwNetType)
{
    BOOL bRet = FALSE;
    HINSTANCE hinst = LoadLibrary(L"ntshrui.dll");
    if (hinst)
    {
        PFNGETNETRESOURCEFROMLOCALPATH pfn = (PFNGETNETRESOURCEFROMLOCALPATH)GetProcAddress(hinst, "GetNetResourceFromLocalPathW");
        if (pfn)
        {
            bRet = pfn(pcszPath, pszNameBuf, cchNameBufLen, pdwNetType);
        }
        FreeLibrary(hinst);
    }
    return bRet;
}

//  Return UNC version of a path if it is a mapped net drive
//
//  pszPath - initial path (drive letter or UNC style)
//  ppszUNC- UNC path returned here
//
//  Return:
//      S_OK - UNC path returned
//
//  The function fails is the path is not a valid network path.  If the path is already UNC,
//  a copy is made without validating the path. *ppszUNC must be CoTaskMemFree()'d by the caller.

HRESULT PathConvertMappedDriveToUNC(PCWSTR pszPath, PWSTR *ppszUNC)
{
    *ppszUNC = NULL;

    HRESULT hr = E_FAIL;
#if 0
    // build simple root "C:" (with no slash) since WNetGetConnection does not deal with these
    WCHAR szDrive[3] = {pszPath[0], pszPath[1], 0};
    if (DRIVE_REMOTE == GetDriveType(szDrive))
    {
        WCHAR szRemoteName[MAX_PATH];
        DWORD dwLen = ARRAYSIZE(szRemoteName);
        if (NO_ERROR == WNetGetConnection(szDrive, szRemoteName, &dwLen))
        {
            // +3 to skip to "\foo" in "C:\foo"
            WCHAR szUNC[MAX_PATH];
            PathCombine(szUNC, szRemoteName, pszPath + 3);
            hr = SHStrDup(szUNC, ppszUNC);
        }
    }
#else
    // alternate implementation
    struct
    {
        UNIVERSAL_NAME_INFO uni;    // a pointer
        WCHAR szBuf[MAX_PATH];      // buffer for path, make bigger in the future
    } uniBuffer;
    DWORD cbBuffer = sizeof(uniBuffer);

    // test this with disconnected drive letters, might fail for them

    hr = HRESULT_FROM_WIN32(WNetGetUniversalNameW(pszPath, UNIVERSAL_NAME_INFO_LEVEL, &uniBuffer, &cbBuffer));
    if (SUCCEEDED(hr))
    {
        PathRemoveBackslash(uniBuffer.uni.lpUniversalName); // some cases return trailing slash
        hr = SHStrDup(uniBuffer.uni.lpUniversalName, ppszUNC);
    }
#endif
    return hr;
}

// search the shares on this machine to find if the pszLocalPath is scoped by
// one of those shares, if so convert that path to the UNC version
//
// since there can be many shares that scope a path this function searches for the one that
// produces the shortest UNC path (the deepest share)
//
// C:\Documents and Settings\All Users\Shared Docs\foo.txt -> \\machine\SharedDocs\foo.txt

BOOL PathConvertLocalToUNC(PCWSTR pcszLocalPath, PWSTR *ppszUNC)
{
    *ppszUNC = NULL;

    WCHAR szPath[MAX_PATH];
    StringCchCopy(szPath, ARRAYSIZE(szPath), pcszLocalPath);

    WCHAR szResult[MAX_PATH];
    szResult[0] = 0;

    do
    {
        DWORD dwNetType;
        WCHAR szCanidate[MAX_PATH]; // UNC path
        if (GetNetResourceFromLocalPath(szPath, szCanidate, ARRAYSIZE(szCanidate), &dwNetType) &&
            (L'$' != szCanidate[lstrlen(szCanidate) - 1]))
        {
            PathAppend(szCanidate, pcszLocalPath + lstrlen(szPath));
            if ((0 == szResult[0]) || (lstrlen(szCanidate) < lstrlen(szResult)))
            {
                StringCchCopy(szResult, ARRAYSIZE(szResult), szCanidate);
            }
        }
    }
    while (PathRemoveFileSpec(szPath));

    return szResult[0] ? SHStrDup(szResult, ppszUNC) : E_FAIL;
}

// make best effort to convert a path, either local or mapped net drive, to a UNC path
// also deals with the case where the path is already a UNC path

// convert a path that might be local, mapped drive letters or already UNC into a
// UNC path
//
// example:
//      "\\Unc\Path"    -> "\\Unc\Path" (return the input)
//      "X:\folder"     -> "\\mappedserver\share\folder"
//      "C:\Users"      -> "\\Machine\Users"
//      "D:\unshared"   -> FAIL
//
// free result with CoTaskMemFree()

HRESULT PathConvertToUNC(PCWSTR pszPath, PWSTR *ppszUNC)
{
    *ppszUNC = NULL;

    HRESULT hr;
    if (PathIsUNC(pszPath))
    {
        hr = SHStrDup(pszPath, ppszUNC);
    }
    else
    {
        hr = PathConvertMappedDriveToUNC(pszPath, ppszUNC);
        if (FAILED(hr))
        {
            hr = PathConvertLocalToUNC(pszPath, ppszUNC);
        }
    }
    return hr;
}

HRESULT GetUNCPathFromItem(IShellItem *psi, PWSTR *ppszUNC)
{
    *ppszUNC = NULL;

    PWSTR pszPath;
    HRESULT hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
    if (SUCCEEDED(hr))
    {
        hr = PathConvertToUNC(pszPath, ppszUNC);
        CoTaskMemFree(pszPath);
    }
    return hr;
}
