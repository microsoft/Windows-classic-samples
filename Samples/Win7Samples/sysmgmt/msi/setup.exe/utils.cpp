//+-------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       utils.cpp
//
//--------------------------------------------------------------------------

#include "setup.h"
#include "resource.h"
#include "common.h"

#include "msi.h"

#include <assert.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <strsafe.h>

#define WIN // scope W32 API

/////////////////////////////////////////////////////////////////////////////
// VerifyFileSignature
//
DWORD VerifyFileSignature (LPCSTR lpszModule, __in_opt LPSTR lpszCmdLine)
{
    LPCSTR  pszFirstArgEnd;
    LPCSTR  pszFileName;
    LPCSTR  pszEnd;
    DWORD   Status;
    
    //
    // When this function is called, the first argument has already
    // been verified. So skip the first argument.
    //
    GetNextArgument (lpszCmdLine, NULL, &pszFirstArgEnd, NULL);
    
    // Now get the name of the file whose signature needs to be verified.
    Status = GetNextArgument (CharNextA(pszFirstArgEnd), &pszFileName, &pszEnd, NULL);
    
    // Must supply a filename
    if (ERROR_NO_MORE_ITEMS == Status)
        return ERROR_BAD_ARGUMENTS;
    
    // Should not have any more arguments
    if ('\0' != *(CharNextA(pszEnd)) &&
        ERROR_NO_MORE_ITEMS != GetNextArgument (CharNextA(CharNextA(pszEnd)), NULL, NULL, NULL))
    {
        return ERROR_BAD_ARGUMENTS;
    }
    
    // We have the right arguments. Null terminate the filename.
    *(CharNextA(pszEnd)) = '\0';
    
    switch (IsPackageTrusted(lpszModule, pszFileName, NULL))
    {
    case itvWintrustNotOnMachine:
        return TRUST_E_PROVIDER_UNKNOWN;
    case itvTrusted:
        return ERROR_SUCCESS;
    case itvUnTrusted:
    default:
        return TRUST_E_SUBJECT_NOT_TRUSTED;
    }
}

/////////////////////////////////////////////////////////////////////////////
// GetExecutionMode
//
emEnum GetExecutionMode (LPCSTR lpszCmdLine)
{
    LPCSTR  pszStart = NULL;
    LPCSTR  pszEnd = NULL;
    DWORD   dwStatus = ERROR_SUCCESS;
    bool    fQuoted = false;
    //
    // Check the first argument and set the execution mode accordingly.
    // When run without arguments, it is assumed that the default install
    // preset by the package publisher needs to be performed.
    //
    // In case an invalid option is provided, the help dialog describing the
    // usage must be displayed.
    //
    dwStatus = GetNextArgument (lpszCmdLine, &pszStart, &pszEnd, &fQuoted);
    
    if (ERROR_NO_MORE_ITEMS == dwStatus)
        return emPreset;
    
    // The only allowed values in the first argument are /a, /v and /?
    if (pszEnd != CharNextA(pszStart) || fQuoted)
        return emHelp;
    
    if ('/' != (*pszStart) && '-' != (*pszStart))
        return emHelp;
    
    switch (*pszEnd)
    {
    case 'a':
    case 'A':
        return emAdminInstall;
    case 'v':
    case 'V':
        return emVerify;
    default:
        return emHelp;
    }
}

/////////////////////////////////////////////////////////////////////////////
// GetNextArgument
//
DWORD GetNextArgument (LPCSTR pszCmdLine, LPCSTR *ppszArgStart, LPCSTR *ppszArgEnd, bool * pfQuoted)
{
    bool    fInQuotes = false;
    bool    fFoundArgEnd = false;
    LPCSTR  pszChar = pszCmdLine;
    LPCSTR  pszFirst = NULL;
    LPCSTR  pszLast = NULL;
    
    if (NULL == pszChar)
        return ERROR_NO_MORE_ITEMS;
    
    // Skip leading spaces.
    while (' ' == *pszChar || '\t' == *pszChar)
        pszChar = CharNextA(pszChar);
    
    // Check if we have run out of arguments.
    if ('\0' == (*pszChar))
        return ERROR_NO_MORE_ITEMS;
    
    // Check if we this argument has been enclosed in quotes
    if ('\"' == (*pszChar))
    {
        fInQuotes = true;
        pszChar = CharNextA (pszChar);
    }
        
    pszFirst = pszChar;
    
    // Now look for the end of the argument
    while (! fFoundArgEnd)
    {
        pszChar = CharNextA(pszChar);
        
        if ('\0' == (*pszChar))
            fFoundArgEnd = true;
        
        if (fInQuotes && '\"' == (*pszChar))
            fFoundArgEnd = true;
        
        if (!fInQuotes && ' ' == (*pszChar))
            fFoundArgEnd = true;
        
        if (!fInQuotes && '\t' == (*pszChar))
            fFoundArgEnd = true;
    }
    
    pszLast = CharPrevA (pszFirst, pszChar);
    
    if (ppszArgStart)
        *ppszArgStart = pszFirst;
    
    if (ppszArgEnd)
        *ppszArgEnd = pszLast;
    
    if (pfQuoted)
        *pfQuoted = fInQuotes;
    
    return ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//
//
DWORD GetAdminInstallInfo (bool fPatch, __in_opt LPSTR lpszCmdLine, LPCSTR * ppszAdminImagePath)
{
    LPCSTR  pszFirstArgEnd;
    LPCSTR  pszFileName;
    LPCSTR  pszEnd;
    DWORD   Status;
    
    //
    // When this function is called, the first argument has already been
    // verified. So skip the first argument.
    //
    GetNextArgument (lpszCmdLine, NULL, &pszFirstArgEnd, NULL);
    
    // See if there is another argument
    Status = GetNextArgument (CharNextA(pszFirstArgEnd), &pszFileName, &pszEnd, NULL);
    
    // If it is not a patch, there should not be any more arguments.
    if (!fPatch)
    {
        if (ERROR_NO_MORE_ITEMS != Status)
            return ERROR_BAD_ARGUMENTS;
        
        // If we are here, then we are done, because we have all the information we need.
        if (ppszAdminImagePath)
            *ppszAdminImagePath = NULL;
        return ERROR_SUCCESS;
    }
    
    // If we are here, this is a patch. Get the path to the admin. install.
    if (ERROR_NO_MORE_ITEMS == Status)
        return ERROR_BAD_ARGUMENTS;     // No path was supplied.
    
    // Should not have any more arguments.
    if ('\0' != *(CharNextA(pszEnd)) &&
        ERROR_NO_MORE_ITEMS != GetNextArgument (CharNextA(CharNextA(pszEnd)), NULL, NULL, NULL))
    {
        return ERROR_BAD_ARGUMENTS;
    }
    
    // We have the right arguments. Null terminate the pathname.
    *(CharNextA(pszEnd)) = '\0';
    
    if (ppszAdminImagePath)
        *ppszAdminImagePath = pszFileName;
    
    return ERROR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// LoadResourceString
//
UINT LoadResourceString(HINSTANCE hInst, LPCSTR lpType, LPCSTR lpName, __out_ecount(*pdwBufSize) LPSTR lpBuf, DWORD *pdwBufSize)
{
    HRSRC   hRsrc   = 0;
    HGLOBAL hGlobal = 0;
    WCHAR   *pch    = 0;

    if ((hRsrc = WIN::FindResource(hInst, lpName, lpType)) != 0
        && (hGlobal = WIN::LoadResource(hInst, hRsrc)) != 0)
    {
        // resource exists
        if ((pch = (WCHAR*)LockResource(hGlobal)) != 0)
        {
            unsigned int cch = WideCharToMultiByte(CP_ACP, 0, pch, -1, NULL, 0, NULL, NULL);
            if (cch > *pdwBufSize)
            {
                *pdwBufSize = cch;
                return ERROR_MORE_DATA;
            }

            if (0 == WideCharToMultiByte(CP_ACP, 0, pch, -1, lpBuf, *pdwBufSize, NULL, NULL))
                return ERROR_FUNCTION_FAILED;
            *pdwBufSize = cch;

        }
        else
        {
            if (1 > *pdwBufSize)
            {
                *pdwBufSize = 1;
                return ERROR_MORE_DATA;
            }

            *pdwBufSize = 1;
            *lpBuf = 0;
        }
        
        DebugMsg("[Resource] lpName = %s, lpBuf = %s\n", lpName, lpBuf);

        return ERROR_SUCCESS;
    }

    // resource does not exist
    DebugMsg("[Resource] lpName = %s NOT FOUND\n", lpName);

    return ERROR_RESOURCE_NAME_NOT_FOUND;
}

/////////////////////////////////////////////////////////////////////////////
// SetupLoadResourceString
//

UINT SetupLoadResourceString(HINSTANCE hInst, LPCSTR lpName, __deref_out LPSTR *lppBuf, DWORD dwBufSize)
{
    UINT uiStat = 0;
    if (!*lppBuf)
    {
        dwBufSize = (dwBufSize > 0) ? dwBufSize : 256;
        *lppBuf = new char[dwBufSize];
        if (!*lppBuf)
            return ERROR_OUTOFMEMORY;
    }

    if (ERROR_SUCCESS != (uiStat = LoadResourceString(hInst, RT_INSTALL_PROPERTY, lpName, *lppBuf, &dwBufSize)))
    {
        if (uiStat != ERROR_MORE_DATA)
            return uiStat;

        // resize and try again
        delete [] *lppBuf;
        *lppBuf = new char[dwBufSize];
        if (!*lppBuf)
            return ERROR_OUTOFMEMORY;

        uiStat = LoadResourceString(hInst, RT_INSTALL_PROPERTY, lpName, *lppBuf, &dwBufSize);
    }

    return uiStat;
}

/////////////////////////////////////////////////////////////////////////////
// PostResourceNotFoundError
//

void PostResourceNotFoundError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, LPCSTR szName)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, IDS_MISSING_RESOURCE, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, szName);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONEXCLAMATION);
}

/////////////////////////////////////////////////////////////////////////////
// ReportUserCancelled
//

void ReportUserCancelled(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle)
{
    char szError[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, IDS_USER_CANCELLED, szError, sizeof(szError)/sizeof(char));
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONEXCLAMATION);
}

/////////////////////////////////////////////////////////////////////////////
// PostError
//

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId)
{
    char szError[MAX_STR_LENGTH]  = {0};

    WIN::LoadString(hInst, uiErrorId, szError, sizeof(szError)/sizeof(char));
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

/////////////////////////////////////////////////////////////////////////////
// PostError
//

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, szValue);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

/////////////////////////////////////////////////////////////////////////////
// PostError
//

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue, int iValue)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, szValue, iValue);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

/////////////////////////////////////////////////////////////////////////////
// PostError
//

void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, int iValue)
{
    char szError[MAX_STR_LENGTH]  = {0};
    char szFormat[MAX_STR_LENGTH] = {0};

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    StringCchPrintf(szError, sizeof(szError), szFormat, iValue);
    MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
}

/////////////////////////////////////////////////////////////////////////////
// PostFormattedError
//

void PostFormattedError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue)
{
    char szFormat[MAX_STR_LENGTH] = {0};
    const char* szArgs[1] = {szValue};
    LPVOID lpMessage = 0;;

    WIN::LoadString(hInst, uiErrorId, szFormat, sizeof(szFormat)/sizeof(char));
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, (LPVOID)szFormat, 0, 0, (LPSTR)&lpMessage, 0, (va_list*)szArgs);
    if (!lpMessage)
    {
        ReportErrorOutOfMemory(hInst, hwndOwner, szTitle);
        return;
    }
    MessageBox(hwndOwner, (LPCSTR)lpMessage, szTitle, MB_OK | MB_ICONERROR);
    LocalFree(lpMessage);
}

/////////////////////////////////////////////////////////////////////////////
// PostMsiError
//

void PostMsiError(HINSTANCE hInst, HINSTANCE hMsi, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId)
{
    switch (uiErrorId)
    {
    case ERROR_INSTALL_SUSPEND:
    case ERROR_INSTALL_USEREXIT:
    case ERROR_INSTALL_FAILURE:
    case ERROR_SUCCESS_REBOOT_REQUIRED:
    case ERROR_SUCCESS_REBOOT_INITIATED:
    case ERROR_APPHELP_BLOCK:
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_INVALID_NAME:
    case ERROR_PATH_NOT_FOUND:
        uiErrorId = ERROR_INSTALL_PACKAGE_OPEN_FAILED;
    default:
        {
            char szError[MAX_STR_LENGTH] = {0};
            if (0 == WIN::LoadString(hMsi, uiErrorId, szError, sizeof(szError)/sizeof(char)))
            {
                // error string does not exist, use default
                PostError(hInst, hwndOwner, szTitle, IDS_INSTALL_ERROR, uiErrorId);
            }
            else
            {
                MessageBox(hwndOwner, szError, szTitle, MB_OK | MB_ICONERROR);
            }
            return;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// AlreadyInProgress
//
//  Attempts to create the MSISETUP mutex. Returns TRUE
//  if mutex already exists or failed to create mutex
//

bool AlreadyInProgress(HANDLE& hMutex)
{
    const char *szMutexName = "Global\\_MSISETUP_{2956EBA1-9B5A-4679-8618-357136DA66CA}";
    hMutex = WIN::CreateMutex(NULL /*default security descriptor*/, FALSE, szMutexName);
    if (!hMutex || ERROR_ALREADY_EXISTS == GetLastError())
        return true;

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// DisplayUsage
//
void DisplayUsage (HINSTANCE hInst, HWND hwndOwner, LPCSTR szCaption)
{
    char szMessage[MAX_STR_LENGTH];

    WIN::LoadString(hInst, IDS_USAGE, szMessage, sizeof(szMessage)/sizeof(char));
    WIN::MessageBox(hwndOwner, szMessage, szCaption, MB_OK | MB_ICONINFORMATION);
}

/////////////////////////////////////////////////////////////////////////////
// ReportErrorOutOfMemory
//

void ReportErrorOutOfMemory(HINSTANCE hInst, HWND hwndOwner, LPCSTR szCaption)
{
    char szError[MAX_STR_LENGTH];

    WIN::LoadString(hInst, IDS_OUTOFMEM, szError, sizeof(szError)/sizeof(char));
    WIN::MessageBox(hwndOwner, szError, szCaption, MB_OK | MB_ICONERROR);
}


/////////////////////////////////////////////////////////////////////////////
// GetFileVersionNumber
//

DWORD GetFileVersionNumber(__in LPSTR szFilename, DWORD * pdwMSVer, DWORD * pdwLSVer)
{
    DWORD             dwResult = NOERROR;
    unsigned          uiSize;
    DWORD             dwVerInfoSize;
    DWORD             dwHandle;
    BYTE              *prgbVersionInfo = NULL;
    VS_FIXEDFILEINFO  *lpVSFixedFileInfo = NULL;

    DWORD dwMSVer = 0xffffffff;
    DWORD dwLSVer = 0xffffffff;

    dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwHandle);
    if (0 != dwVerInfoSize)
    {
        prgbVersionInfo = (LPBYTE) WIN::GlobalAlloc(GPTR, dwVerInfoSize);
        if (NULL == prgbVersionInfo)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
            goto Finish;
        }

        // Read version stamping info
        if (GetFileVersionInfo(szFilename, dwHandle, dwVerInfoSize, prgbVersionInfo))
        {
            // get the value for Translation
            if (VerQueryValue(prgbVersionInfo, "\\", (LPVOID*)&lpVSFixedFileInfo, &uiSize) && (uiSize != 0))
            {
                dwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                dwLSVer = lpVSFixedFileInfo->dwFileVersionLS;
            }
        }
        else
        {
            dwResult = GetLastError();
            goto Finish;
        }
    }
    else
    {
        dwResult = GetLastError();
    }

#ifdef DEBUG
    char szVersion[255];
    StringCchPrintf(szVersion, sizeof(szVersion), "%s is version %d.%d.%d.%d\n", szFilename, HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer));
    DebugMsg("[INFO] %s", szVersion);
#endif // DEBUG

Finish:
    if (NULL != prgbVersionInfo)
        WIN::GlobalFree(prgbVersionInfo);
    if (pdwMSVer)
        *pdwMSVer = dwMSVer;
    if (pdwLSVer)
        *pdwLSVer = dwLSVer;

    return dwResult;
}

/////////////////////////////////////////////////////////////////////////////
// MimimumWindowsPlatform
//
//  Returns true if running on a platform whose major version, minor version
//  and service pack major are greater than or equal to the ones specifed
//  while making this function call
//
bool MimimumWindowsPlatform(DWORD dwMajorVersion, DWORD dwMinorVersion, WORD wServicePackMajor)
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;

   // Initialize the OSVERSIONINFOEX structure.
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = dwMajorVersion;
   osvi.dwMinorVersion = dwMinorVersion;
   osvi.wServicePackMajor = wServicePackMajor;
   
   // Initialize the condition mask.
   VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
   VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
   VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
 
   // Perform the test.
   return VerifyVersionInfo(&osvi, 
                            VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR,
                            dwlConditionMask) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
// IsOSSupported
//
//  Returns true if running on Windows 2003, Windows XP or 
//  Windows 2000 SP3 and above. Else returns false
//
bool IsOSSupported()
{
    OSVERSIONINFO sInfoOS;
    memset((void*)&sInfoOS, 0x00, sizeof(OSVERSIONINFO));

    sInfoOS.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    WIN::GetVersionEx(&sInfoOS);

    // We do no support any platform prior to Windows 2000
    if (5 > sInfoOS.dwMajorVersion)
        return false;

    // We support:
    if(MimimumWindowsPlatform(5, 2, 0) ||   // Windows 2003 and above
       MimimumWindowsPlatform(5, 1, 0) ||   // Windows XP and above
       MimimumWindowsPlatform(5, 0, 3))     // Windows 2000 SP3 and above
        return true;
    else
        return false;
}

//--------------------------------------------------------------------------------------
// ADVAPI32 API -- delay load
//--------------------------------------------------------------------------------------

#define ADVAPI32_DLL "advapi32.dll"

#define ADVAPI32API_CheckTokenMembership "CheckTokenMembership"
typedef BOOL (WINAPI* PFnCheckTokenMembership)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

#define ADVAPI32API_AdjustTokenPrivileges "AdjustTokenPrivileges"
typedef BOOL (WINAPI* PFnAdjustTokenPrivileges)(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);

#define ADVAPI32API_OpenProcessToken "OpenProcessToken"
typedef BOOL (WINAPI* PFnOpenProcessToken)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);

#define ADVAPI32API_LookupPrivilegeValue "LookupPrivilegeValueA"
typedef BOOL (WINAPI* PFnLookupPrivilegeValue)(LPCSTR lpSystemName, LPCSTR lpName, PLUID lpLuid);

/////////////////////////////////////////////////////////////////////////////
// IsAdmin
//
//  Returns true if current user is an administrator (or if on Win9X)
//  Returns false if current user is not an adminstrator
//
//  implemented as per KB Q118626
//

bool IsAdmin()
{
    // get the administrator sid
    PSID psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    if(!AllocateAndInitializeSid(&siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdministrators))
        return false;

    // on NT5, use the CheckTokenMembershipAPI to correctly handle cases where
    // the Adiminstrators group might be disabled. bIsAdmin is BOOL for 
    BOOL bIsAdmin = FALSE;
    // CheckTokenMembership checks if the SID is enabled in the token. NULL for
    // the token means the token of the current thread. Disabled groups, restricted
    // SIDS, and SE_GROUP_USE_FOR_DENY_ONLY are all considered. If the function
    // returns false, ignore the result.

    HMODULE hAdvapi32 = LoadLibrary(ADVAPI32_DLL);
    if (!hAdvapi32)
        bIsAdmin = FALSE;
    else
    {
        PFnCheckTokenMembership pfnCheckTokenMembership = (PFnCheckTokenMembership)GetProcAddress(hAdvapi32, ADVAPI32API_CheckTokenMembership);
        if (!pfnCheckTokenMembership || !pfnCheckTokenMembership(NULL, psidAdministrators, &bIsAdmin))
            bIsAdmin = FALSE;
    }
    FreeLibrary(hAdvapi32);
    hAdvapi32 = 0;
    
    WIN::FreeSid(psidAdministrators);
    return bIsAdmin ? true : false;

}

/////////////////////////////////////////////////////////////////////////////
// AcquireShutdownPrivilege
//
//  Attempts to enable the SE_SHUTDOWN_NAME privilege in the process token
//
bool AcquireShutdownPrivilege()
{
    HANDLE hToken = 0;
    TOKEN_PRIVILEGES tkp;

    HMODULE hAdvapi32 = LoadLibrary(ADVAPI32_DLL);
    if (!hAdvapi32)
        return false;

    PFnOpenProcessToken pfnOpenProcessToken = (PFnOpenProcessToken)GetProcAddress(hAdvapi32, ADVAPI32API_OpenProcessToken);
    PFnLookupPrivilegeValue pfnLookupPrivilegeValue = (PFnLookupPrivilegeValue)GetProcAddress(hAdvapi32, ADVAPI32API_LookupPrivilegeValue);
    PFnAdjustTokenPrivileges pfnAdjustTokenPrivileges = (PFnAdjustTokenPrivileges)GetProcAddress(hAdvapi32, ADVAPI32API_AdjustTokenPrivileges);
    if (!pfnOpenProcessToken || !pfnLookupPrivilegeValue || !pfnAdjustTokenPrivileges)
    {
        FreeLibrary(hAdvapi32);
        return false;
    }

    // grab this process's token
    if (!pfnOpenProcessToken(WIN::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        FreeLibrary(hAdvapi32);
        return false;
    }

    // get the LUID for the shutdown privilege
    pfnLookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1; // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // get the shutdown privilege for this process
    pfnAdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    // cannot test return value of AdjustTokenPrivileges
    if (ERROR_SUCCESS != WIN::GetLastError())
    {
        FreeLibrary(hAdvapi32);
        return false;
    }

    FreeLibrary(hAdvapi32);

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// SetDiagnosticMode
//
//  Turns on debug output if first char of szDebugEnvVar is set to 1
//

int g_dmDiagnosticMode = -1; // -1 until set, then DebugMsg skips fn call if 0

void SetDiagnosticMode()
{
    g_dmDiagnosticMode = 0; // disable DebugMsg to start

    char rgchBuf[64] = {0};
    if (0 != WIN::GetEnvironmentVariable(szDebugEnvVar, rgchBuf, sizeof(rgchBuf)/sizeof(char))
        && rgchBuf[0] == '1')
    {
        g_dmDiagnosticMode = 1; // enable DebugMsg output
    }
}

/////////////////////////////////////////////////////////////////////////////
// DebugMsg
//
//  Outputs debugging string to debugger if debug output is enabled
//

void DebugMsg(LPCSTR szFormat, int iArg1)
{
    if (-1 == g_dmDiagnosticMode)
    {
        SetDiagnosticMode();
    }

    if (0 == g_dmDiagnosticMode || !szFormat)
        return; // debug output is not enabled or nothing to output

    const int INT_AS_STRING_SIZE = 12;
    size_t cchFormat = lstrlen(szFormat);
    size_t cchDebug = cchFormat + INT_AS_STRING_SIZE + 1;
    char *szDebug = new char[cchDebug];
    if (!szDebug)
        return ; // out of memory

    if (FAILED(StringCchPrintf(szDebug, cchDebug, szFormat, iArg1)))
    {
        delete[] szDebug;
        return;
    }
    OutputDebugString(szDebug);
    return;
}

void DebugMsg(LPCSTR szFormat, int iArg1, int iArg2)
{
    if (-1 == g_dmDiagnosticMode)
    {
        SetDiagnosticMode();
    }

    if (0 == g_dmDiagnosticMode || !szFormat)
        return; // debug output is not enabled or nothing to output

    const int INT_AS_STRING_SIZE = 12;
    size_t cchFormat = lstrlen(szFormat);
    size_t cchDebug = cchFormat + 2 * INT_AS_STRING_SIZE + 1;
    char *szDebug = new char[cchDebug];
    if (!szDebug)
        return ; // out of memory

    if (FAILED(StringCchPrintf(szDebug, cchDebug, szFormat, iArg1, iArg2)))
    {
        delete[] szDebug;
        return;
    }
    OutputDebugString(szDebug);
    return;
}

void DebugMsg(LPCSTR szFormat, LPCSTR szArg1, LPCSTR szArg2)
{
    if (-1 == g_dmDiagnosticMode)
    {
        SetDiagnosticMode();
    }

    if (0 == g_dmDiagnosticMode || !szFormat)
        return; // debug output is not enabled or nothing to output

    size_t cchFormat = lstrlen(szFormat);
    size_t cchArg1 = (szArg1 != 0) ? lstrlen(szArg1) : 0;
    size_t cchArg2 = (szArg2 != 0) ? lstrlen(szArg2) : 0;

    if (0 == cchArg1)
    {
        OutputDebugString(szFormat);
    }
    else
    {
        size_t cchDebug = cchFormat + cchArg1 + cchArg2 + 1;
        char *szDebug = new char[cchDebug];
        if (!szDebug)
            return ; // out of memory
        if (0 == cchArg2)
        {            
            if (FAILED(StringCchPrintf(szDebug, cchDebug, szFormat, szArg1)))
            {
                delete[] szDebug;
                return;
            }
            OutputDebugString(szDebug);
        }
        else
        {
            if (FAILED(StringCchPrintf(szDebug, cchDebug, szFormat, szArg1, szArg2)))
            {
                delete[] szDebug;
                return;
            }
            OutputDebugString(szDebug);
        }
    }

    return;
}
