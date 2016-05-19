// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

const WCHAR c_szProgID[] = L"Microsoft.Samples.CustomJumpListProgID";

PCWSTR const c_rgszExtsToRegister[] =
{
    L".txt",
    L".doc",
};

HRESULT _RegSetString(HKEY hkey, PCWSTR pszSubKey, PCWSTR pszValue, PCWSTR pszData)
{
    size_t lenData = lstrlen(pszData);
    return HRESULT_FROM_WIN32(SHSetValue(hkey, pszSubKey, pszValue, REG_SZ, pszData, static_cast<DWORD>((lenData + 1) * sizeof(*pszData))));
}

// Creates a basic ProgID to use for file type registrations.  For a document to appear in Jump Lists, the associated
// application must be registered to handle the document's file type (extension).
HRESULT _RegisterProgid(BOOL fRegister)
{
    HRESULT hr;
    if (fRegister)
    {
        HKEY hkeyProgid;
        hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CLASSES_ROOT, c_szProgID, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE | KEY_CREATE_SUB_KEY , NULL, &hkeyProgid, NULL));
        if (SUCCEEDED(hr))
        {
            hr = _RegSetString(hkeyProgid, NULL, L"FriendlyTypeName", L"Custom Jump List Document");
            if (SUCCEEDED(hr))
            {
                WCHAR szAppPath[MAX_PATH];
                hr = (GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath)) > 0) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
                if (SUCCEEDED(hr))
                {
                    WCHAR szIcon[MAX_PATH + 3];
                    hr = StringCchPrintf(szIcon, ARRAYSIZE(szIcon), L"%s,0", szAppPath);
                    if (SUCCEEDED(hr))
                    {
                        hr = _RegSetString(hkeyProgid, L"DefaultIcon", NULL, szAppPath);
                        if (SUCCEEDED(hr))
                        {
                            hr = _RegSetString(hkeyProgid, L"CurVer", NULL, c_szProgID);
                            if (SUCCEEDED(hr))
                            {
                                HKEY hkeyShell;
                                hr = HRESULT_FROM_WIN32(RegCreateKeyEx(hkeyProgid, L"shell", 0, NULL, REG_OPTION_NON_VOLATILE,
                                    KEY_SET_VALUE | KEY_CREATE_SUB_KEY, NULL, &hkeyShell, NULL));
                                if (SUCCEEDED(hr))
                                {
                                    // The list of verbs provided by the ProgID is located uner the "shell" key.  Here, only
                                    // the single "Open" verb is registered.
                                    WCHAR szCmdLine[MAX_PATH * 2];
                                    hr = StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine), L"%s /HandleDocument:%%1", szAppPath);
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = _RegSetString(hkeyShell, L"Open\\Command", NULL, szCmdLine);
                                        if (SUCCEEDED(hr))
                                        {
                                            // Set "Open" as the default verb for this ProgID.
                                            hr = _RegSetString(hkeyShell, NULL, NULL, L"Open");
                                        }
                                    }
                                    RegCloseKey(hkeyShell);
                                }
                            }
                        }
                    }
                }
            }
            RegCloseKey(hkeyProgid);
        }
    }
    else
    {
        long lRes = RegDeleteTree(HKEY_CLASSES_ROOT, c_szProgID);
        hr = (ERROR_SUCCESS == lRes || ERROR_FILE_NOT_FOUND == lRes) ? S_OK : HRESULT_FROM_WIN32(lRes);
    }
    return hr;
}

HRESULT _RegisterToHandleExt(PCWSTR pszExt, BOOL fRegister)
{
    WCHAR szKey[MAX_PATH];
    HRESULT hr = StringCchCopy(szKey, ARRAYSIZE(szKey), pszExt);
    if (SUCCEEDED(hr))
    {
        // All ProgIDs that can handle a given file type should be listed under OpenWithProgids, even if listed
        // as the default, so they can be enumerated in the Open With dialog, and so the Jump Lists can find
        // the correct ProgID to use when relaunching a document with the specific application the Jump List is
        // associated with.
        hr = PathAppend(szKey, L"OpenWithProgids") ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            HKEY hkeyProgidList;
            hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CLASSES_ROOT, szKey, 0, NULL, REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE, NULL, &hkeyProgidList, NULL));
            if (SUCCEEDED(hr))
            {
                if (fRegister)
                {
                    hr = HRESULT_FROM_WIN32(RegSetValueEx(hkeyProgidList, c_szProgID, 0, REG_NONE, NULL, 0));
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(RegDeleteKeyValue(hkeyProgidList, NULL, c_szProgID));
                }
                RegCloseKey(hkeyProgidList);
            }
        }
    }
    return hr;
}

HRESULT RegisterToHandleFileTypes()
{
    HRESULT hr = _RegisterProgid(TRUE);
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(c_rgszExtsToRegister); i++)
        {
            hr = _RegisterToHandleExt(c_rgszExtsToRegister[i], TRUE);
        }

        if (SUCCEEDED(hr))
        {
            // Notify that file associations have changed
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        }
    }
    return hr;
}

bool AreFileTypesRegistered()
{
    bool fRet = false;
    HKEY hkeyProgid;
    if (SUCCEEDED(HRESULT_FROM_WIN32(RegOpenKey(HKEY_CLASSES_ROOT, c_szProgID, &hkeyProgid))))
    {
        fRet = true;
        RegCloseKey(hkeyProgid);
    }
    return fRet;
}

HRESULT UnRegisterFileTypeHandlers()
{
    HRESULT hr = _RegisterProgid(FALSE);
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(c_rgszExtsToRegister); i++)
        {
            hr = _RegisterToHandleExt(c_rgszExtsToRegister[i], FALSE);
        }

        if (SUCCEEDED(hr))
        {
            // Notify that file associations have changed
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        }
    }
    return hr;
}