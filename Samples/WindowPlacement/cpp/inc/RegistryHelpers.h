#pragma once
// Note: Intended to be included by User32Utils.h.

//
// Helpers for reading and writing values (strings and DWORDs) to app-specific
// registry keys, used to persist data like window positions.
//

inline HKEY GetAppRegKey(PCWSTR appName)
{
    HKEY hKey = nullptr;

    LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, appName,
        0, KEY_ALL_ACCESS , &hKey);

    if (openRes != ERROR_SUCCESS)
    {
        RegCreateKeyEx(HKEY_CURRENT_USER, appName,
            0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    }

    return hKey;
}

inline std::wstring ReadStringRegKey(PCWSTR appName, PCWSTR keyName)
{
    HKEY hKey = GetAppRegKey(appName);

    WCHAR textBuffer[500];
    DWORD dwBufferSize = sizeof(textBuffer);

    // Read the registry key.
    ULONG nError = RegQueryValueExW(hKey, keyName,
        0, NULL, (LPBYTE)textBuffer, &dwBufferSize);

    // Return empty string if registry read failed.
    if (nError != ERROR_SUCCESS)
    {
        return L"";
    }

    RegCloseKey(hKey);

    return (PWSTR)&textBuffer;
}

inline void WriteStringRegKey(PCWSTR appName, PCWSTR keyName, std::wstring keyValue)
{
    HKEY hKey = GetAppRegKey(appName);

    RegSetValueEx(hKey, keyName, 0, REG_SZ,
        (LPBYTE)keyValue.c_str(), (DWORD)((wcslen(keyValue.c_str()) * 2) + 1));

    RegCloseKey(hKey);
}

inline void DeleteRegValue(PCWSTR appName, PCWSTR keyName)
{
    HKEY hKey = GetAppRegKey(appName);

    RegDeleteValue(hKey, keyName);

    RegCloseKey(hKey);
}

inline DWORD ReadDwordRegKey(PCWSTR appName, PCWSTR keyName, DWORD dwDefault)
{
    HKEY hKey = GetAppRegKey(appName);

    unsigned long type = REG_DWORD, size = 1024;
    RegQueryValueEx(hKey, keyName, nullptr, &type, (PBYTE)&dwDefault, &size);

    RegCloseKey(hKey);

    return dwDefault;
}

inline void WriteDwordRegKey(PCWSTR appName, PCWSTR keyName, DWORD dwValue)
{
    HKEY hKey = GetAppRegKey(appName);

    RegSetValueEx(hKey, keyName, 0, REG_DWORD, (PBYTE)&dwValue, sizeof(dwValue));

    RegCloseKey(hKey);
}
