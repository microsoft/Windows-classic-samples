// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * utility functions to read and write values to the profile,
 * using win.ini for Win16 or HKEY_CURRENT_USER\software\microsoft\sdkdiff\...
 * in the registry for Win32
 */

#ifndef _PROFILE_REG_H
#define _PROFILE_REG_H

#define MMPROFILECACHE 0  // Set to 1 to cache keys, 0 otherwise
#define USESTRINGSALSO 1

#ifndef _WIN32

#define mmGetProfileIntA(app, value, default) \
          GetProfileInt(app, value, default)

#define mmWriteProfileString(appname, valuename, pData) \
          WriteProfileString(appname, valuename, pData)

#define mmGetProfileString(appname, valuename, pDefault, pResult, cbResult) \
          GetProfileString(appname, valuename, pDefault, pResult, cbResult)

#define CloseKeys()

#else


/*
 * read a UINT from the profile, or return default if
 * not found.
 */
UINT mmGetProfileIntA(LPCSTR appname, LPCSTR valuename, INT uDefault);

/*
 * read a string from the profile into pResult.
 * result is number of characters written into pResult
 */
DWORD mmGetProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pDefault,
                    LPTSTR pResult, int cbResult
);

/*
 * write a string/integer to the profile
 */
BOOL mmWriteProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pData);

BOOL mmWriteProfileInt(LPCTSTR appname, LPCTSTR valuename, INT value);

UINT mmGetProfileInt(LPCTSTR appname, LPCTSTR valuename, INT value);

#undef WriteProfileString
#undef GetProfileString
#undef GetProfileInt

#define WriteProfileString  mmWriteProfileString
#define WriteProfileInt     mmWriteProfileInt
#define GetProfileString    mmGetProfileString
#define GetProfileInt       mmGetProfileInt


#if MMPROFILECACHE
VOID CloseKeys(VOID);
#else
#define CloseKeys()
#endif

/*
 * convert an Ansi string to Wide characters
 */
LPWSTR mmAnsiToWide (
   LPWSTR lpwsz,   // out: wide char buffer to convert into
   LPCSTR  lpsz,   // in: ansi string to convert from
   UINT   nChars); // in: count of characters in each buffer

/*
 * convert a Wide char string to Ansi
 */
LPSTR mmWideToAnsi (
   LPSTR  lpsz,    // out: ansi buffer to convert into
   LPCWSTR lpwsz,  // in: wide char buffer to convert from
   UINT   nChars); // in: count of characters (not bytes!)

#if !defined NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#endif
#endif // _PROFILE_REG_H
