#pragma once

#include <windows.h>
#include <strsafe.h>

// Return the name of the COM DLL associated with a given CLSID string.
// The CLSID string must be in canonical form.
inline HRESULT GetFilenameByCLSIDString(const WCHAR *szGUID, WCHAR *szFile, size_t cch)
{
    HRESULT hr;

	const DWORD STR_LEN = 512;

	WCHAR szKey[STR_LEN];


	int rc=0;

	// Create key name for reading filename from registry
	hr = StringCchPrintf(
		szKey, 
		STR_LEN, 
		L"Software\\Classes\\CLSID\\%s\\InprocServer32\0",
		szGUID);

	if (SUCCEEDED(hr))
	{

        HKEY hkeyFilter=0;
        DWORD dwSize=MAX_PATH;
        BYTE szFilename[MAX_PATH];

        // Open the CLSID key that contains information about the filter
        rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
        if (rc == ERROR_SUCCESS)
        {
            rc = RegQueryValueEx(hkeyFilter, NULL,  // Read (Default) value
                                 NULL, NULL, szFilename, &dwSize);
           
            if (rc == ERROR_SUCCESS)
			{
                hr = StringCchPrintf(szFile, cch, L"%s\0", szFilename);
			}
			else
			{
				hr = E_FAIL;
			}
            RegCloseKey(hkeyFilter);
        }
		else
		{
			hr = E_FAIL;
		}

    }
	return hr;
}


// Return the name of the COM DLL associated with a given CLSID.
// (Same as above, but takes a CLSID instead of a string.)
inline HRESULT GetFilenameByCLSID(const GUID *pGUID, WCHAR *szFile, size_t cch)
{
	const DWORD GUID_STR_LEN = 40;

	WCHAR szGUID[GUID_STR_LEN];


	if (0 == StringFromGUID2(*pGUID, szGUID, GUID_STR_LEN))
	{
		return E_FAIL;
	}
	else
	{
		return GetFilenameByCLSIDString(szGUID, szFile, cch);
	}
}


// The following templates are also defined in streams.h
#ifndef __STREAMS__

#pragma warning(push)
#pragma warning(disable: 4312 4244)
// _GetWindowLongPtr
// Templated version of GetWindowLongPtr, to suppress spurious compiler warning.
template <class T>
T _GetWindowLongPtr(HWND hwnd, int nIndex)
{
    return (T)GetWindowLongPtr(hwnd, nIndex);
}

// _SetWindowLongPtr
// Templated version of SetWindowLongPtr, to suppress spurious compiler warning.
template <class T>
LONG_PTR _SetWindowLongPtr(HWND hwnd, int nIndex, T p)
{
    return SetWindowLongPtr(hwnd, nIndex, (LONG_PTR)p);
}
#pragma warning(pop)

#endif