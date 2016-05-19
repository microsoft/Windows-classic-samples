// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * profile.cpp
 *
 * win32/win16 utility functions to read and write profile items
 * for VFW
 *
 * When a profile call
 * is made the code looks for an atom to correspond to the appname supplied.
 * If an atom is found, then the table of cached registry keys is searched
 * for the matching registry handle.  If the handle exists, it is used.
 * No handle would mean that someone else registered an atom using this
 * name, so we proceed to the next step.
 *
 * No atom found, or no matching registry handle, means that we have to
 * open a registry key.  If successful, and there is space in the cache,
 * we AddAtom the appname, and cache the registry key before returning to
 * the caller.
 */

#include "precomp.h"

#ifdef _WIN32
#include "profile.key"

#include "mmsystem.h"

#include "profile.h"

#ifdef USESTRINGSALSO
#include <stdlib.h>/* for atoi */
#endif

#if MMPROFILECACHE

#ifdef DEBUG
#define KEYSCACHED 3 
#else
#define KEYSCACHED 2 
#endif

HKEY   ahkey[KEYSCACHED];
ATOM   akeyatoms[KEYSCACHED];
UINT   keyscached = 0;

#else
#define KEYSCACHED 0
#endif


static HKEY GetKeyA(LPCSTR appname, BOOL * closekey, BOOL fCreate)
{
    HKEY key = 0;
    char achName[MAX_PATH];
	HRESULT hr;
#if !MMPROFILECACHE
    *closekey = TRUE;
#else
    UINT n;
    ATOM atm;

    *closekey = FALSE;
    //
    // See if we have already used this key
    //
    atm = FindAtomA(appname);

    if (atm != 0) {
	// Atom exists... search the table for it.
        for (n=0; n<keyscached; ++n) {
            if (akeyatoms[n] == atm) {
                DPF2(("Found existing key for %s\n", appname));
                return ahkey[n];
            }
        }
    }
    DPF2(("No key found for %s", appname));
#endif

    hr = StringCchCopyA(achName, MAX_PATH, KEYNAMEA);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_COPY);

    if ((!fCreate && RegOpenKeyA(ROOTKEY, achName, &key) == ERROR_SUCCESS)
        || (fCreate && RegCreateKeyA(ROOTKEY, achName, &key) == ERROR_SUCCESS)) {
#if MMPROFILECACHE
        if ((keyscached < KEYSCACHED)
	  && (atm = AddAtomA(appname))) {
            // Add this key to the cache array
            akeyatoms[keyscached] = atm;
            ahkey[keyscached] = key;
            DPF1(("Adding key %s to cache array in position %d\n", appname, keyscached));
            ++keyscached;
        } else {
            DPF2(("Not adding key %s to cache array\n", appname));
            *closekey = TRUE;
        }
#endif
    }

    return(key);
}

#ifdef UNICODE
static HKEY GetKeyW(LPCWSTR appname, BOOL * closekey, BOOL fCreate) {

    HKEY key = 0;
    WCHAR achName[MAX_PATH];
#if !MMPROFILECACHE
    *closekey = TRUE;
#else
    UINT n;
    ATOM atm;

    *closekey = FALSE;
    //
    // See if we have already used this key
    //
    atm = FindAtomW(appname);

    if (atm != 0) {
	// Atom exists... search the table for it.
        for (n=0; n<keyscached; ++n) {
            if (akeyatoms[n] == atm) {
                DPF2(("(W)Found existing key for %ls\n", appname));
                return ahkey[n];
            }
        }
    }
    DPF2(("(W)No key found for %ls\n", appname));
#endif

    StringCchCopyW(achName, MAX_PATH, KEYNAME );
    StrCchCatW(achName, MAX_PATH, appname);

    if ((!fCreate && RegOpenKeyW(ROOTKEY, achName, &key) == ERROR_SUCCESS)
        || (fCreate && RegCreateKeyW(ROOTKEY, achName, &key) == ERROR_SUCCESS)) {
#if MMPROFILECACHE
        if (keyscached < KEYSCACHED
	  && (atm = AddAtomW(appname))) {
            // Add this key to the cache array
            akeyatoms[keyscached] = atm;
            ahkey[keyscached] = key;
            DPF1(("Adding key %ls to cache array in position %d\n", appname, keyscached));
            ++keyscached;
        } else {
            DPF2(("Not adding key to cache array\n"));
            *closekey = TRUE;
        }
#endif
    }

    return(key);
}
#define GetKey GetKeyW
#else
#define GetKey GetKeyA
#endif // UNICODE

/*
 * read a UINT from the profile, or return default if
 * not found.
 */
#ifdef _WIN32
UINT
mmGetProfileInt(LPCSTR appname, LPCSTR valuename, INT uDefault)
{
    DWORD dwType;
    INT value = uDefault;
    DWORD dwData;
    int cbData;
    BOOL fCloseKey;

    HKEY key = GetKeyA(appname, &fCloseKey, FALSE);

    if (key) {

        cbData = sizeof(dwData);
        if (RegQueryValueExA(
            key,
            (LPSTR)valuename,
            NULL,
            &dwType,
            (PBYTE) &dwData,
            (LPDWORD)&cbData) == ERROR_SUCCESS) {
            if (dwType == REG_DWORD || dwType == REG_BINARY) {
                value = (INT)dwData;
#ifdef USESTRINGSALSO
            } else if (dwType == REG_SZ) {
		value = atoi((LPSTR) &dwData);
#endif
	    }
	}

        // close open key open if we did not cache it
        if (fCloseKey) {
            RegCloseKey(key);
        }
    }

    return((UINT)value);
}
#endif

/*
 * read a string from the profile into pResult.
 * result is number of bytes written into pResult
 */
#ifdef _WIN32
DWORD
mmGetProfileString(
    LPCTSTR appname,
    LPCTSTR valuename,
    LPCTSTR pDefault,
    LPTSTR pResult,
    int cbResult
)
{
    DWORD dwType;
    BOOL fCloseKey;
	HRESULT hr;

    HKEY key = GetKey(appname, &fCloseKey, FALSE);

    if (key) {

        cbResult = cbResult * sizeof(TCHAR);
        if (RegQueryValueEx(
            key,
            (LPTSTR)valuename,
            NULL,
            &dwType,
            (LPBYTE)pResult,
            (LPDWORD)&cbResult) == ERROR_SUCCESS) {

                if (dwType == REG_SZ) {
                    // cbResult is set to the size including null
                    // we return the number of characters

                    // close key if we did not cache it
                    if (fCloseKey) {
                        RegCloseKey(key);
                    }
                    return(cbResult/sizeof(TCHAR) - 1);
                }
        }

        // close open key if we did not cache it
        if (fCloseKey) {
            RegCloseKey(key);
        }
    }

    // if we got here, we didn't find it, or it was the wrong type - return
    // the default string
    hr = StringCchCopy(pResult, cbResult, pDefault);
	if (FAILED(hr)) {
		OutputError(hr, IDS_SAFE_COPY);
		return NULL;
	}
    return(lstrlen(pDefault));
}
#endif


/*
 * write a string to the profile
 */
#ifdef _WIN32
BOOL
mmWriteProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pData)
{
    BOOL fCloseKey;
    HKEY key = GetKey(appname, &fCloseKey, TRUE);
    BOOL fResult = !(ERROR_SUCCESS);

    if (key) {
        if (pData) {
            fResult = RegSetValueEx(
                key,
                (LPTSTR)valuename,
                0,
                REG_SZ,
                (LPBYTE)pData,
                (lstrlen(pData) + 1) * sizeof(TCHAR)
            );
        } else {
            fResult = RegDeleteValue(
                key,
                (LPTSTR)valuename
            );
        }

        if (fCloseKey) {
            RegCloseKey(key);
        }
    }

    if (ERROR_SUCCESS == fResult) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*****************************************************************************

 functions to help convert wide characters to multibyte & vv. (using
 functions to control code size...)

 ****************************************************************************/

/*
 * convert an Ansi string to Unicode
 */
LPWSTR mmAnsiToWide (
   LPWSTR lpwsz,  // out: wide char buffer to convert into
   LPCSTR lpsz,   // in: ansi string to convert from
   UINT   nChars) // in: count of characters in each buffer
{
   MultiByteToWideChar(GetACP(), 0, lpsz, nChars, lpwsz, nChars);
   return lpwsz;
}

/*
 * convert a unicode string to ansi
 */
LPSTR mmWideToAnsi (
   LPSTR   lpsz,   // out: ansi buffer to convert into
   LPCWSTR lpwsz,  // in: wide char buffer to convert from
   UINT    nChars) // in: count of characters (not bytes!)
{
   WideCharToMultiByte(GetACP(), 0, lpwsz, nChars, lpsz, nChars, NULL, NULL);
   return lpsz;
}


/*
 * Close all open registry keys
 */
#if MMPROFILECACHE
VOID CloseKeys()
{
    for (; keyscached--;) {

#ifdef DEBUG
        if (!ahkey[keyscached]) {           
            DPF0(("Closing a null key\n"));
        }
#endif
        RegCloseKey(ahkey[keyscached]);
        DeleteAtom(akeyatoms[keyscached]);
    }
}

#endif //MMPROFILECACHE


/*
 * write a UINT to the profile, if it is not the
 * same as the default or the value already there
 */
#ifdef _WIN32
BOOL
mmWriteProfileInt(LPCTSTR appname, LPCTSTR valuename, INT Value)
{
    // If we would write the same as already there... return.
    if (mmGetProfileInt(appname, valuename, !Value) == ((UINT)Value)) {
        return TRUE;
    }

    {
        TCHAR achName[MAX_PATH];
        HKEY hkey;

        HRESULT hr = StringCchCopy(achName, MAX_PATH, KEYNAME);
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_COPY);
        
        if (RegCreateKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {
            RegSetValueEx(
                hkey,
                valuename,
                0,
                REG_DWORD,
                (PBYTE) &Value,
                sizeof(Value)
            );

            RegCloseKey(hkey);
        }
    }
    return TRUE;
}

#endif // _WIN32

#endif
#endif
