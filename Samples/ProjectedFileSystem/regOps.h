/*++

Copyright (c) Microsoft Corporation

Module Name:

    regOps.h

Abstract:

   Helper class to encapsulate registry operations.

--*/

#include <psapi.h>

namespace regfs {

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Represents a single entry in the registry, capturing its name and size.
struct RegEntry {
    std::wstring Name;
    ULONG Size;
};

// Stores RegEntry items for the entries within a registry key, separated into lists of subkeys
// and values.
struct RegEntries {
    std::vector<RegEntry> SubKeys;
    std::vector<RegEntry> Values;
};

class RegOps {
public:

    RegOps()
    {
        _regRootKeyMap[L"HKEY_CLASSES_ROOT"] = HKEY_CLASSES_ROOT;
        _regRootKeyMap[L"HKEY_CURRENT_USER"] = HKEY_CURRENT_USER;
        _regRootKeyMap[L"HKEY_LOCAL_MACHINE"] = HKEY_LOCAL_MACHINE;
        _regRootKeyMap[L"HKEY_USERS"] = HKEY_USERS;
        _regRootKeyMap[L"HKEY_CURRENT_CONFIG"] = HKEY_CURRENT_CONFIG;
    }

    // Returns a RegEntries struct populated with the subkeys and values in the registry key whose
    // path is specified.
    HRESULT EnumerateKey(const std::wstring& path, RegEntries& entries)
    {
        HRESULT hr = S_OK;

        if (PathUtils::IsVirtualizationRoot(path.c_str()))
        {
            // The path is the "root" of the registry, so return the names of the predefined keys
            // in _regRootKeyMap.
            for (auto it = _regRootKeyMap.begin(); it != _regRootKeyMap.end(); it++)
            {
                RegEntry entry;
                entry.Name = it->first;
                entries.SubKeys.emplace_back(entry);
            }
        }
        else
        {
            // The path is somewhere below the root, so try opening the key.
            HKEY subKey = nullptr;
            hr = OpenKeyByPath(path, subKey);

            // If the path corresponds to a registry key, enumerate it.
            if (subKey != nullptr)
            {
                hr = EnumerateKey(subKey, entries);
                RegCloseKey(subKey);
            }
        }

        return hr;
    }

    // Reads a value from the registry.
    bool ReadValue(const std::wstring& path, PBYTE data, UINT32& len)
    {
        auto lastPos = path.find_last_of(L"\\");
        if (lastPos == std::wstring::npos)
        {
            // There are no '\' characters in the path. The only paths with no '\' are the predefined
            // keys, so this can't be a value.
            return false;
        }

        // Split the path into <key>\<value>
        auto keyPath = path.substr(0, lastPos);
        auto valName = path.substr(lastPos + 1);

        // Open the key path to get a HKEY handle to it.
        HKEY subkey = nullptr;
        OpenKeyByPath(keyPath, subkey);

        // Read the value's content from the registry.
        DWORD ret = RegQueryValueEx(subkey,
                                    valName.c_str(),
                                    0,
                                    nullptr,
                                    reinterpret_cast<LPBYTE>(data),
                                    reinterpret_cast<LPDWORD>(&len));

        if (ret != ERROR_SUCCESS)
        {
            wprintf(L"%hs: RegQueryValueEx [%s]: %d\n",
                    __FUNCTION__, valName.c_str(), ret);
            RegCloseKey(subkey);
            return false;
        }

        RegCloseKey(subkey);

        return true;
    }

    // Returns true if the given path corresponds to a key that exists in the registry.
    bool DoesKeyExist(const std::wstring& path)
    {
        HKEY subkey = nullptr;
        OpenKeyByPath(path, subkey);
        if (subkey == nullptr)
        {
            wprintf(L"%hs: key [%s] doesn't exist \n",
                    __FUNCTION__, path.c_str());
            return false;
        }

        RegCloseKey(subkey);
        return true;
    }

    // Returns true if the given path corresponds to a value that exists in the registry, and tells
    // you how big it is.
    bool DoesValueExist(const std::wstring& path, INT64& valSize)
    {
        auto pos = path.find_last_of(L"\\");
        if (pos == std::wstring::npos)
        {
            // There are no '\' characters in the path. The only paths with no '\' are the predefined
            // keys, so this can't be a value.
            return false;
        }
        else
        {
            HKEY subkey = nullptr;
            OpenKeyByPath(path.substr(0, pos), subkey);
            if (subkey == nullptr)
            {
                wprintf(L"%hs: value [%s] doesn't exist \n",
                        __FUNCTION__, path.substr(0, pos).c_str());
                return false;
            }

            auto valPathStr = path.substr(pos + 1);
            auto res = RegGetValue(subkey,
                                   nullptr,
                                   valPathStr.c_str(),
                                   RRF_RT_ANY,
                                   nullptr,
                                   nullptr,
                                   reinterpret_cast<PDWORD>(&valSize));

            if (res != ERROR_SUCCESS)
            {
                wprintf(L"%hs: Could not get value [%s] at key [%s]: %d\n",
                        __FUNCTION__, valPathStr.c_str(), path.substr(0, pos).c_str(), res);
                return false;
            }

            RegCloseKey(subkey);
        }

        return true;
    }

private:

    // Gets the HKEY for a registry key given the path, if it exists.
    HRESULT OpenKeyByPath(const std::wstring& path, HKEY& hKey)
    {
        HRESULT hr = S_OK;

        hKey = nullptr;

        auto pos = path.find(L"\\");
        if (pos == std::wstring::npos)
        {
            // There are no '\' characters in the path. The only paths with no '\' are the predefined
            // keys, so try to find the correct HKEY in the predefined keys map.
            if (_regRootKeyMap.find(path) != _regRootKeyMap.end())
            {
                hKey = _regRootKeyMap[path];
            }
            else
            {
                wprintf(L"%hs: root key [%s] doesn't exist\n",
                        __FUNCTION__, path.c_str());
                hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
            }
        }
        else
        {
            // The first component of the path should be a predefined key, so get its HKEY value and
            // try opening the rest of the key relative to it.
            auto rootKeyStr = path.substr(0, pos);
            auto rootKey = _regRootKeyMap[rootKeyStr];
            DWORD res = RegOpenKeyEx(rootKey,
                                     path.substr(pos + 1).c_str(),
                                     0,
                                     KEY_READ,
                                     &hKey);

            if (res != ERROR_SUCCESS)
            {
                wprintf(L"%hs: failed to open key [%s]: %d\n",
                        __FUNCTION__, path.substr(pos + 1).c_str(), res);
                hr = HRESULT_FROM_WIN32(res);
            }
        }

        return hr;
    }

    // Returns a RegEntries struct populated with the subkeys and values in the specified registry key.
    HRESULT EnumerateKey(HKEY hKey, RegEntries& entries)
    {
        HRESULT  hr = S_OK;
        DWORD    subKeyCount = 0;
        DWORD    valueCount = 0;

        DWORD i, retCode;

        // Find out how many subkeys and values there are under this key.
        retCode = RegQueryInfoKey(hKey,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  &subKeyCount,
                                  nullptr,
                                  nullptr,
                                  &valueCount,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr);

        if (retCode != ERROR_SUCCESS)
        {
            wprintf(L"%hs: RegQueryInfoKey: %d\n",
                    __FUNCTION__, retCode);
            return HRESULT_FROM_WIN32(retCode);
        }

        TCHAR    keyName[MAX_KEY_LENGTH];
        DWORD    keyNameLength;

        // If there are subkeys, enumerate them until RegEnumKeyEx fails.
        if (subKeyCount)
        {
            wprintf(L"\n%hs: Subkeys:\n",
                    __FUNCTION__);

            for (i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++)
            {
                keyNameLength = MAX_KEY_LENGTH;
                keyName[0] = '\0';

                retCode = RegEnumKeyEx(hKey,
                                       i,
                                       keyName,
                                       &keyNameLength,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr);

                if (retCode == ERROR_SUCCESS)
                {
                    wprintf(TEXT("(%d) %s\n"), i + 1, keyName);

                    RegEntry entry;
                    entry.Name = std::wstring(keyName);
                    entry.Size = 0;

                    entries.SubKeys.push_back(entry);
                }
                else if (retCode != ERROR_NO_MORE_ITEMS)
                {
                    wprintf(L"%hs: RegEnumKeyEx: %d\n",
                            __FUNCTION__, retCode);
                    hr = HRESULT_FROM_WIN32(retCode);

                    return hr;
                }
            }
        }

        TCHAR  valueName[MAX_VALUE_NAME];
        DWORD  valueNameLength = MAX_VALUE_NAME;

        // If there are values, enumerate them until RegEnumValue fails.
        if (valueCount)
        {
            wprintf(L"\n%hs: Values: \n",
                    __FUNCTION__);

            retCode = ERROR_SUCCESS;

            for (i = 0, retCode = ERROR_SUCCESS; retCode == ERROR_SUCCESS; i++)
            {
                valueNameLength = MAX_VALUE_NAME;
                valueName[0] = '\0';

                DWORD size = 0;
                retCode = RegEnumValue(hKey,
                                       i,
                                       valueName,
                                       &valueNameLength,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       &size);

                if (retCode == ERROR_SUCCESS)
                {
                    wprintf(TEXT("(%d) %s (%d bytes)\n"), i + 1, valueName, size);

                    RegEntry entry;
                    entry.Name = std::wstring(valueName);
                    entry.Size = size;

                    entries.Values.push_back(entry);
                }
                else if (retCode != ERROR_NO_MORE_ITEMS)
                {
                    wprintf(L"%hs: RegEnumValue: %d\n",
                            __FUNCTION__, retCode);
                    hr = HRESULT_FROM_WIN32(retCode);

                    return hr;
                }
            }
        }

        return hr;
    }

private:

    // Maps string names of the predefined registry keys (HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, etc.)
    // to their HKEY values.
    std::map<std::wstring, HKEY> _regRootKeyMap;
};

}