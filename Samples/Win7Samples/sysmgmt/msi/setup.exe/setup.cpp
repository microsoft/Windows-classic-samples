//+-------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File:       setup.cpp
//
//--------------------------------------------------------------------------

#define WIN // scope W32 API
#define MSI // scope MSI API

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <strsafe.h>

// internet download
#include "wininet.h"  // DeleteUrlCacheEntry, InternetCanonicalizeUrl
#include "urlmon.h"   // URLDownloadToCacheFile

// package trust
#include "wintrust.h"
#include "softpub.h"

// msi installation
#include "msidefs.h"
#include "msiquery.h"
#include "msi.h"

// setup.exe
#include "common.h"
#include "setup.h"
#include "setupui.h"
#include "resource.h"

//--------------------------------------------------------------------------------------
// MSI API -- delay load
//--------------------------------------------------------------------------------------

#define MSI_DLL "msi.dll"

#define MSIAPI_MsiSetInternalUI "MsiSetInternalUI"
typedef INSTALLUILEVEL (WINAPI* PFnMsiSetInternalUI)(INSTALLUILEVEL dwUILevel, HWND *phWnd);

#define MSIAPI_MsiInstallProduct "MsiInstallProductA"
typedef UINT (WINAPI* PFnMsiInstallProduct)(LPCSTR szPackagePath, LPCSTR szCommandLine);

#define MSIAPI_MsiApplyPatch "MsiApplyPatchA"
typedef UINT (WINAPI* PFnMsiApplyPatch)(LPCSTR szPatchPackage, LPCSTR szInstallPackage, INSTALLTYPE eInstallType, LPCSTR szCommandLine);

#define MSIAPI_MsiReinstallProduct "MsiReinstallProductA"
typedef UINT (WINAPI* PFnMsiReinstallProduct)(LPCSTR szProduct, DWORD dwReinstallMode);

#define MSIAPI_MsiQueryProductState "MsiQueryProductStateA"
typedef INSTALLSTATE (WINAPI* PFnMsiQueryProductState)(LPCSTR szProduct);

#define MSIAPI_MsiOpenDatabase "MsiOpenDatabaseA"
typedef UINT (WINAPI* PFnMsiOpenDatabase)(LPCSTR szDatabasePath, LPCSTR szPersist, MSIHANDLE *phDatabase);

#define MSIAPI_MsiDatabaseOpenView "MsiDatabaseOpenViewA"
typedef UINT (WINAPI* PFnMsiDatabaseOpenView)(MSIHANDLE hDatabase, LPCSTR szQuery, MSIHANDLE *phView);

#define MSIAPI_MsiViewExecute "MsiViewExecute"
typedef UINT (WINAPI* PFnMsiViewExecute)(MSIHANDLE hView, MSIHANDLE hRecord);

#define MSIAPI_MsiViewFetch "MsiViewFetch"
typedef UINT (WINAPI* PFnMsiViewFetch)(MSIHANDLE hView, MSIHANDLE *phRecord);

#define MSIAPI_MsiRecordGetString "MsiRecordGetStringA"
typedef UINT (WINAPI* PFnMsiRecordGetString)(MSIHANDLE hRecord, unsigned int uiField, LPSTR szValue, DWORD *pcchValueBuf);

#define MSIAPI_MsiCloseHandle "MsiCloseHandle"
typedef UINT (WINAPI* PFnMsiCloseHandle)(MSIHANDLE h);

const DWORD lcidLOCALE_INVARIANT = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

/////////////////////////////////////////////////////////////////////////////
// WinMain -- Application Entry Point
//

extern "C" int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst , __in LPSTR lpszCmdLine, int nCmdShow)
{

//-----------------------------------------------------------------------------------------------------------------
//  VARIABLES
//
//-----------------------------------------------------------------------------------------------------------------
    UINT    uiRet = ERROR_SUCCESS;
    HRESULT hr    = S_OK;

    char *szMsiFile          = 0;
    char *szBaseURL          = 0;
    char *szInstallPath      = 0;
    char *szMsiCacheFile     = 0;
    char *szOperation        = 0;
    char *szProductName      = 0;
    char *szMinimumMsi       = 0;
    char *szProperties       = 0;
    char *szInstProperties   = 0;
    char *szTempPath         = 0;
    char *szFilePart         = 0;
    char *szBase             = 0;
    char *szUpdate           = 0;

    char *szRegisteredMsiFolder = 0;
    char *szMsiDllLocation      = 0;

    char szAppTitle[MAX_STR_CAPTION]    = {0};
    char szError[MAX_STR_LENGTH]        = {0};
    char szText[MAX_STR_CAPTION]        = {0};
    char szBanner[MAX_STR_LENGTH]       = {0};
    char szAction[MAX_STR_LENGTH]       = {0};
    char szUserPrompt[MAX_STR_LENGTH]   = {0};
    char szProductCode[MAX_LENGTH_GUID] = {0};

    char szModuleFile[MAX_PATH]         = {0};
    DWORD dwModuleFileSize       = MAX_PATH;
    
    DWORD dwMsiFileSize          = 0;
    DWORD dwBaseURLSize          = 0;
    DWORD cchInstallPath         = 0;
    DWORD dwMsiCacheFileSize     = 0;
    DWORD dwOperationSize        = 0;
    DWORD dwProductNameSize      = 0;
    DWORD dwMinimumMsiSize       = 0;
    DWORD dwPropertiesSize       = 0;
    DWORD cchInstProperties      = 0;
    DWORD cchTempPath            = 0;
    DWORD dwLastError            = 0;
    DWORD cchReturn              = 0;
    DWORD dwBaseUpdateSize      = 0;
    DWORD dwUpdateSize          = 0;
    DWORD dwResult               = 0;
    DWORD dwType                 = 0;
    DWORD dwProductCodeSize      = MAX_LENGTH_GUID;

    DWORD dwRegisteredMsiFolderSize  = 0;
    DWORD dwMsiDllLocationSize       = 0;

    ULONG ulMsiMinVer        = 0;
    char *szStopScan         = NULL;

    bool        fDelayRebootReq    = false;
    bool        fPatch             = false;
    bool        fQFE               = false;
    bool        fOSSupported       = false;
    emEnum      emExecMode         = emPreset;

    HKEY hInstallerKey = 0;

    HMODULE hMsi = 0;
    PFnMsiSetInternalUI pfnMsiSetInternalUI = 0;
    PFnMsiInstallProduct pfnMsiInstallProduct = 0;
    PFnMsiApplyPatch pfnMsiApplyPatch = 0;
    PFnMsiReinstallProduct pfnMsiReinstallProduct = 0;
    PFnMsiQueryProductState pfnMsiQueryProductState = 0;
    PFnMsiOpenDatabase pfnMsiOpenDatabase = 0;
    PFnMsiDatabaseOpenView pfnMsiDatabaseOpenView = 0;
    PFnMsiViewExecute pfnMsiViewExecute = 0;
    PFnMsiViewFetch pfnMsiViewFetch = 0;
    PFnMsiRecordGetString pfnMsiRecordGetString = 0;
    PFnMsiCloseHandle pfnMsiCloseHandle = 0;

    MSIHANDLE hDatabase = 0;
    MSIHANDLE hView = 0;
    MSIHANDLE hRec = 0;

    INSTALLSTATE isProduct = INSTALLSTATE_UNKNOWN;
    
    const char * szAdminImagePath = 0;



//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
    // create our UI object
    CDownloadUI DownloadUI;

    // Load our AppTitle (caption)
    WIN::LoadString(hInst, IDS_APP_TITLE, szAppTitle, sizeof(szAppTitle)/sizeof(char));

    // Obtain path we are running from
    if (0 == WIN::GetModuleFileName(hInst, szModuleFile, dwModuleFileSize))
    {
        // No UI displayed. Silent failure.
        uiRet = GetLastError();
        goto CleanUp;
    }
    DebugMsg("[Info] we are running from --> %s\n", szModuleFile);

    // Figure out what we want to do
    emExecMode = GetExecutionMode (lpszCmdLine);
    
    if (emVerify == emExecMode)
    {
        //
        // We don't want any UI to be displayed in this case. The return value
        // from the exe is the result of the verification. Therefore, this
        // should be done before initializing the UI.
        //
        uiRet = VerifyFileSignature (szModuleFile, lpszCmdLine);
        if (ERROR_BAD_ARGUMENTS != uiRet)
            goto CleanUp;
    }
    
    if (ERROR_BAD_ARGUMENTS == uiRet || emHelp == emExecMode)
    {
        DisplayUsage(hInst, NULL, szAppTitle);
        goto CleanUp;
    }
    
    //
    // NOTE:
    // Delay handling admin. installs until we have determined if we are
    // patching an existing install or if we are doing a default install.
    //
 
    // initialize our UI object with desktop as parent
    DownloadUI.Initialize(hInst, /* hwndParent = */ 0, szAppTitle);

    // Check if we are installing on an OS that supports Windows Installer 3.0
    fOSSupported = IsOSSupported();
    if(!fOSSupported)
    {
        PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_OS_NOT_SUPPORTED);
        uiRet = ERROR_INSTALL_FAILURE;
        goto CleanUp;
    }
    HANDLE hMutex = 0;

    // only run one instance at a time
    if (AlreadyInProgress(hMutex))
    {
        // silently return - correct return code ?
		uiRet = ERROR_INSTALL_ALREADY_RUNNING;
		goto CleanUp;
    }
    
    // determine operation, default (if not present) is INSTALL
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_OPERATION, &szOperation, dwOperationSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    if (ERROR_SUCCESS != uiRet)
    {
        // set operation to default which is install
        if (szOperation)
            delete [] szOperation;
        szOperation = new char[lstrlen(szDefaultOperation) + 1];
        if (!szOperation)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        if (FAILED(StringCchCopy(szOperation, lstrlen(szDefaultOperation) + 1, szDefaultOperation)))
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
            uiRet = ERROR_INSTALL_FAILURE;
            goto CleanUp;
        }
    }

    // obtain name of product
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_PRODUCTNAME, &szProductName, dwProductNameSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    if (ERROR_SUCCESS != uiRet)
    {
        // use default
        if (szProductName)
            delete [] szProductName;
        szProductName = new char[MAX_STR_CAPTION];
        if (!szProductName)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        WIN::LoadString(hInst, IDS_DEFAULT_PRODUCT, szProductName, MAX_STR_CAPTION);
    }

    // set banner text
    WIN::LoadString(hInst, IDS_BANNER_TEXT, szText, MAX_STR_CAPTION);
    StringCchPrintf(szBanner, sizeof(szBanner), szText, szProductName);
    if (irmCancel == DownloadUI.SetBannerText(szBanner))
    {
        ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_INSTALL_USEREXIT;
        goto CleanUp;
    }

    // Determine if this is a patch or a normal install.
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_DATABASE, &szMsiFile, dwMsiFileSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    else if (ERROR_SUCCESS != uiRet)
    {
        // look for patch
        if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_PATCH, &szMsiFile, dwMsiFileSize)))
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        else if (ERROR_SUCCESS != uiRet)
        {
            PostResourceNotFoundError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, ISETUPPROPNAME_DATABASE);
            goto CleanUp;
        }

        fPatch = true;
    }
    
    //
    // If we are here, this is either an admin. install or a default install.
    // File signature verification, help and other invalid parameters have
    // already been taken care of above.
    //
    if (emAdminInstall == emExecMode)
    {
        uiRet = GetAdminInstallInfo (fPatch, lpszCmdLine, &szAdminImagePath);
        if (ERROR_BAD_ARGUMENTS == uiRet)
        {
            DisplayUsage(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
    }
    
    //
    // At this point, the validation of the commandline arguments is complete
    // and we have all the information we need.
    //

    // obtain minimum required MSI version
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_MINIMUM_MSI, &szMinimumMsi, dwMinimumMsiSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    else if (ERROR_SUCCESS != uiRet)
    {
        PostResourceNotFoundError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, ISETUPPROPNAME_MINIMUM_MSI);
        goto CleanUp;
    }

    // make sure required Msi version is a valid value -- must be >= 150
    ulMsiMinVer = strtoul(szMinimumMsi, &szStopScan, 10);
    if (!szStopScan || (szStopScan == szMinimumMsi) || (*szStopScan != 0) || ulMsiMinVer < MINIMUM_SUPPORTED_MSI_VERSION)
    {
        // invalid minimum version string
        PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_VER_STR, szMinimumMsi, MINIMUM_SUPPORTED_MSI_VERSION);
        uiRet = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

    DebugMsg("[Resource] Minimum Msi Value = %d\n", ulMsiMinVer);

    // compare minimum required MSI version to that which is on the machine
    if (IsMsiUpgradeNecessary(ulMsiMinVer))
    {
        DebugMsg("[Info] Upgrade of Windows Installer is requested\n");

        // make sure this is admin -- must have admin priviledges to upgrade Windows Installer
        if (!IsAdmin())
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_REQUIRES_ADMIN_PRIV);
            uiRet = ERROR_INSTALL_FAILURE;
            goto CleanUp;
        }

        // Ask the user if they want to upgrade the installer
        WIN::LoadString(hInst, IDS_ALLOW_MSI_UPDATE, szUserPrompt, MAX_STR_LENGTH);
        if (IDYES != WIN::MessageBox(DownloadUI.GetCurrentWindow(), szUserPrompt, szAppTitle, MB_YESNO|MB_ICONQUESTION))
        {
            // user decided to cancel
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }

        if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_UPDATE, &szUpdate, dwUpdateSize)))
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        else if (ERROR_SUCCESS != uiRet)
        {
            PostResourceNotFoundError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, ISETUPPROPNAME_UPDATE);
            goto CleanUp;
        }            
        
        // determine if we need to download the Windows Installer update package from the web -- based on presence of UPDATELOCATION property
        if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_UPDATELOCATION, &szBase, dwBaseUpdateSize)))
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        else if (ERROR_SUCCESS == uiRet)
        {
            // presence of UPDATELOCATION property indicates assumption of URL source
            if (ERROR_SUCCESS != (uiRet = DownloadAndUpgradeMsi(hInst, &DownloadUI, szAppTitle, szBase, szUpdate, szModuleFile, ulMsiMinVer)))
            {
                if (ERROR_SUCCESS_REBOOT_REQUIRED == uiRet)
                {
                    // successful, but must reboot at end
                    fDelayRebootReq = true;
                }
                else
                    goto CleanUp;
            }
        }
        else
        {
            // lack of UPDATELOCATION property indicates assumption of Media source
            if (ERROR_SUCCESS != (uiRet = UpgradeMsi(hInst, &DownloadUI, szAppTitle, szModuleFile, szUpdate, ulMsiMinVer)))
            {
                if (ERROR_SUCCESS_REBOOT_REQUIRED == uiRet)
                {
                    // successful, but must reboot at end
                    fDelayRebootReq = true;
                }
                else
                    goto CleanUp;
            }
        }
    }

    DebugMsg("[Info] Windows Installer has been upgraded, or was already correct version\n");

    // perform some extra authoring validation
    if (fPatch
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szMinPatchOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szMajPatchOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szDefaultOperation, -1))
    {
        // wrong operation
        DebugMsg("[Error] Operation %s is not valid for a patch\n", szOperation);
        PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_OPERATION, szOperation);
        uiRet = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }
    else if (!fPatch
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szInstallOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szInstallUpdOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szDefaultOperation, -1))
    {
        // wrong operation
        DebugMsg("[Error] Operation %s is not valid for a package\n", szOperation);
        PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_OPERATION, szOperation);
        uiRet = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

    // by now we either have a MSI or a MSP
    if (CSTR_EQUAL == CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szMinPatchOperation, -1)
        || CSTR_EQUAL == CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szInstallUpdOperation, -1)
        || (fPatch && CSTR_EQUAL == CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szDefaultOperation, -1)))
        fQFE = true;

    // obtain base URL
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_BASEURL, &szBaseURL, dwBaseURLSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    else if (ERROR_SUCCESS == uiRet)
    {
        // presence of BASEURL property indicates assumption of URL source . . .

        // generate the path to the installation package == baseURL + msiFile
        //   note: msiFile is a relative path
        cchTempPath = lstrlen(szBaseURL) + lstrlen(szMsiFile) + 2; // 1 for slash, 1 for null
        szTempPath = new char[cchTempPath ];
        if (!szTempPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }
        if (FAILED(StringCchCopy(szTempPath, cchTempPath, szBaseURL)))
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
            uiRet = ERROR_INSTALL_FAILURE;
            goto CleanUp;
        }
        // check for trailing slash on szBaseURL
        char *pch = szBaseURL + lstrlen(szBaseURL) + 1; // put at null terminator
        pch = CharPrev(szBaseURL, pch);
        if (*pch != '/')
        {
            if (FAILED(StringCchCat(szTempPath, cchTempPath, szUrlPathSep)))
            {
                PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
                uiRet = ERROR_INSTALL_FAILURE;
                goto CleanUp;
            }
        }
        if (FAILED(StringCchCat(szTempPath, cchTempPath, szMsiFile)))
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
            uiRet = ERROR_INSTALL_FAILURE;
            goto CleanUp;
        }        

        // canocialize the URL path
        cchInstallPath = cchTempPath*2;
        szInstallPath = new char[cchInstallPath];
        if (!szInstallPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

        dwLastError = 0; // success
        if (!InternetCanonicalizeUrl(szTempPath, szInstallPath, &cchInstallPath, 0))
        {
            dwLastError = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
            {
                // try again
                delete [] szInstallPath;
                szInstallPath = new char[cchInstallPath];
                if (!szInstallPath)
                {
                    ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                    uiRet = ERROR_OUTOFMEMORY;
                    goto CleanUp;
                }
                dwLastError = 0; // reset to success for 2nd attempt
                if (!InternetCanonicalizeUrl(szTempPath, szInstallPath, &cchInstallPath, 0))
                    dwLastError = GetLastError();
            }
        }
        if (0 != dwLastError)
        {
            // error -- invalid path/Url
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
            uiRet = dwLastError;
            goto CleanUp;
        }

        // set action text for download
        WIN::LoadString(hInst, IDS_DOWNLOADING_PACKAGE, szText, MAX_STR_CAPTION);
        StringCchPrintf(szAction, sizeof(szAction), szText, szMsiFile);
        if (irmCancel == DownloadUI.SetActionText(szAction))
        {
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }

        // download the msi file so we can attempt a trust check -- must be local for WinVerifyTrust
        DebugMsg("[Info] Downloading msi file %s for WinVerifyTrust check\n", szInstallPath);

        szMsiCacheFile = new char[MAX_PATH];
        dwMsiCacheFileSize = MAX_PATH;
        if (!szMsiCacheFile)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

        hr = WIN::URLDownloadToCacheFile(NULL, szInstallPath, szMsiCacheFile, dwMsiCacheFileSize, 0, /* IBindStatusCallback = */ &CDownloadBindStatusCallback(&DownloadUI));
        if (DownloadUI.HasUserCanceled())
        {
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }
        if (FAILED(hr))
        {
            // error during download -- probably because file not found (or lost connection)
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_NOMSI, szInstallPath);
            uiRet = ERROR_FILE_NOT_FOUND;
            goto CleanUp;
        }

        DebugMsg("[Info] Msi file was cached to %s\n", szMsiCacheFile);

        // set action text for trust verification
        WIN::LoadString(hInst, IDS_VALIDATING_SIGNATURE, szText, MAX_STR_CAPTION);
        StringCchPrintf(szAction, sizeof(szAction), szText, szMsiFile);
        if (irmCancel == DownloadUI.SetActionText(szAction))
        {
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }

        // perform trust check 
        itvEnum itv = IsPackageTrusted(szModuleFile, szMsiCacheFile, DownloadUI.GetCurrentWindow());
        if (itvWintrustNotOnMachine == itv)
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_NO_WINTRUST);
            uiRet = ERROR_CALL_NOT_IMPLEMENTED;
            goto CleanUp;
        }
        else if (itvUnTrusted == itv)
        {
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_UNTRUSTED, szInstallPath);
            uiRet = HRESULT_CODE(TRUST_E_SUBJECT_NOT_TRUSTED);
            goto CleanUp;
        }
    }
    else
    {
        // lack of BASEURL property indicates assumption of Media source

        // generate the path to the Msi file =  szModuleFile + msiFile
        //   note: msiFile is a relative path
        cchTempPath = lstrlen(szModuleFile) + lstrlen(szMsiFile) + 2; // 1 for null terminator, 1 for back slash
        szTempPath = new char[cchTempPath];
        if (!szTempPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

        // find 'setup.exe' in the path so we can remove it
        if (0 == GetFullPathName(szModuleFile, cchTempPath, szTempPath, &szFilePart))
        {
            uiRet = GetLastError();
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
            goto CleanUp;
        }
        if (szFilePart)
            *szFilePart = '\0';

        if (FAILED(StringCchCat(szTempPath, cchTempPath, szMsiFile)))
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
            uiRet = ERROR_INSTALL_FAILURE;
            goto CleanUp;
        }
        
        cchInstallPath = 2*cchTempPath;
        szInstallPath = new char[cchInstallPath];
        if (!szInstallPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

        // normalize the path
        cchReturn = GetFullPathName(szTempPath, cchInstallPath, szInstallPath, &szFilePart);
        if (cchReturn > cchInstallPath)
        {
            // try again, with larger buffer
            delete [] szInstallPath;
            cchInstallPath = cchReturn;
            szInstallPath = new char[cchInstallPath];
            if (!szInstallPath)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                uiRet = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            cchReturn = GetFullPathName(szTempPath, cchInstallPath, szInstallPath, &szFilePart);
        }
        if (0 == cchReturn)
        {
            // error -- invalid path
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
            uiRet = dwLastError;
            goto CleanUp;
        }

        // no download is necessary -- but we can check for the file's existence
        DWORD dwFileAttrib = GetFileAttributes(szInstallPath);
        if (0xFFFFFFFF == dwFileAttrib)
        {
            // package is missing
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_NOMSI, szInstallPath);
            uiRet = ERROR_FILE_NOT_FOUND;
            goto CleanUp;
        }
    }

    //
    // good to go -- terminate our UI and let the Windows Installer take over
    //

    // retrieve the optional command line PROPERTY = VALUE strings if available
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_PROPERTIES, &szProperties, dwPropertiesSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_OUTOFMEMORY;
        goto CleanUp;
    }
    else if (ERROR_SUCCESS != uiRet)
    {
        // PROPERTY=VALUE pairs not specified
        if (szProperties)
            delete [] szProperties;
        szProperties = NULL;
    }

    DownloadUI.Terminate();

    //
    // perform install 
    //

    hMsi = LoadLibrary(MSI_DLL);
    
    if (hMsi)
    {
        pfnMsiSetInternalUI = (PFnMsiSetInternalUI)GetProcAddress(hMsi, MSIAPI_MsiSetInternalUI);
        pfnMsiInstallProduct = (PFnMsiInstallProduct)GetProcAddress(hMsi, MSIAPI_MsiInstallProduct);
        pfnMsiApplyPatch = (PFnMsiApplyPatch)GetProcAddress(hMsi, MSIAPI_MsiApplyPatch);
        pfnMsiReinstallProduct = (PFnMsiReinstallProduct)GetProcAddress(hMsi, MSIAPI_MsiReinstallProduct);
        pfnMsiQueryProductState = (PFnMsiQueryProductState)GetProcAddress(hMsi, MSIAPI_MsiQueryProductState);
        pfnMsiOpenDatabase = (PFnMsiOpenDatabase)GetProcAddress(hMsi, MSIAPI_MsiOpenDatabase);
        pfnMsiDatabaseOpenView = (PFnMsiDatabaseOpenView)GetProcAddress(hMsi, MSIAPI_MsiDatabaseOpenView);
        pfnMsiViewExecute = (PFnMsiViewExecute)GetProcAddress(hMsi, MSIAPI_MsiViewExecute);
        pfnMsiViewFetch = (PFnMsiViewFetch)GetProcAddress(hMsi, MSIAPI_MsiViewFetch);
        pfnMsiRecordGetString = (PFnMsiRecordGetString)GetProcAddress(hMsi, MSIAPI_MsiRecordGetString);
        pfnMsiCloseHandle = (PFnMsiCloseHandle)GetProcAddress(hMsi, MSIAPI_MsiCloseHandle);
    }
    if (!hMsi || !pfnMsiSetInternalUI || !pfnMsiInstallProduct || !pfnMsiApplyPatch || !pfnMsiReinstallProduct || !pfnMsiQueryProductState
        || !pfnMsiDatabaseOpenView || !pfnMsiViewExecute || !pfnMsiViewFetch || !pfnMsiRecordGetString || !pfnMsiCloseHandle)
    {
        PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_FAILED_TO_UPGRADE_MSI);
        uiRet = ERROR_INSTALL_FAILURE;
        goto CleanUp;
    }

    DebugMsg("[Info] Setting Internal UI level to FULL...\n");
    pfnMsiSetInternalUI(INSTALLUILEVEL_FULL, 0);

    if (!fPatch)
    {
        // performing install or reinstall/recache
        DebugMsg("[Info] Calling MsiInstallProduct with szInstallPath = %s", szInstallPath); 
        DebugMsg(" and szCommandLine = %s\n", szProperties ? szProperties : "{null}");

        // default operation for a package is INSTALL

        if (fQFE)
        {
            // check to see if this product is already installed
            if (ERROR_SUCCESS == pfnMsiOpenDatabase(szMsiCacheFile ? szMsiCacheFile : szInstallPath, MSIDBOPEN_READONLY, &hDatabase)
                && ERROR_SUCCESS == pfnMsiDatabaseOpenView(hDatabase, sqlProductCode, &hView)
                && ERROR_SUCCESS == pfnMsiViewExecute(hView, 0)
                && ERROR_SUCCESS == pfnMsiViewFetch(hView, &hRec)
                && ERROR_SUCCESS == pfnMsiRecordGetString(hRec, 1, szProductCode, &dwProductCodeSize))
            {
                isProduct = pfnMsiQueryProductState(szProductCode);
                DebugMsg("[Info] MsiQueryProductState returned %d\n", isProduct);
                if (INSTALLSTATE_ADVERTISED != isProduct && INSTALLSTATE_DEFAULT != isProduct)
                {
                    // product is unknown, so this will be a first time install
                    DebugMsg("[Info] The product code '%s' is unknown. Will use first time install logic...\n", szProductCode);
                    fQFE = false;
                }
                else
                {
                    // product is known, use QFE syntax
                    DebugMsg("[Info] The product code '%s' is known. Will use QFE recache and reinstall upgrade logic...\n", szProductCode);
                }
            }
            else
            {
                // some failure occurred when processing the product code, so treat as non-QFE
                DebugMsg("[Info] Unable to process product code. Will treat as first time install...\n");
                fQFE = false;
            }
            if (hDatabase)
                pfnMsiCloseHandle(hDatabase);
            if (hView)
                pfnMsiCloseHandle(hView);
            if (hRec)
                pfnMsiCloseHandle(hRec);
        }
        
        //
        // Set up the properties to be passed into MSIInstallProduct
        //
        if (fQFE && !szProperties)
            cchInstProperties = lstrlen (szDefaultInstallUpdCommandLine);
        else if (szProperties)
            cchInstProperties = lstrlen (szProperties);
        if (emAdminInstall == emExecMode)
            cchInstProperties += lstrlen (szAdminInstallProperty);
        
        szInstProperties = new char[cchInstProperties + 1];
        if (! szInstProperties)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }
        
        if (fQFE && !szProperties)
        {
            if (FAILED(StringCchCopy(szInstProperties, cchInstProperties + 1, szDefaultInstallUpdCommandLine)))
            {
                PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
                uiRet = ERROR_INSTALL_FAILURE;
                goto CleanUp;
            }
        }
        else if (szProperties)
        {
            if (FAILED(StringCchCopy(szInstProperties, cchInstProperties + 1, szProperties)))
            {
                PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
                uiRet = ERROR_INSTALL_FAILURE;
                goto CleanUp;
            }
        }
        else
            szInstProperties[0] = '\0';
        if (emAdminInstall == emExecMode)
        {
            if (FAILED(StringCchCat(szInstProperties, cchInstProperties + 1, szAdminInstallProperty)))
            {
                PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
                uiRet = ERROR_INSTALL_FAILURE;
                goto CleanUp;
            }
        }

        uiRet = pfnMsiInstallProduct(szInstallPath, szInstProperties);
        if (ERROR_SUCCESS != uiRet)
        {
            // attempt to display an error message stored in msi.dll
            PostMsiError(hInst, hMsi, DownloadUI.GetCurrentWindow(), szAppTitle, uiRet);
        }

        DebugMsg("[Info] MsiInstallProduct returned %d\n", uiRet);
    }
    else
    {
        // default Operation for a patch is MINPATCH

        // if szProperties is NULL, use our default value for QFE patches
        if (!szProperties && fQFE)
        {
            DebugMsg("[Info] Patch is a MINPATCH (small or minor update patch) so using default command line '%s'\n", szDefaultMinPatchCommandLine);

            szProperties = new char[lstrlen(szDefaultMinPatchCommandLine) + 1];
            if (!szProperties)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                uiRet = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            if (FAILED(StringCchCopy(szProperties, lstrlen(szDefaultMinPatchCommandLine) + 1, szDefaultMinPatchCommandLine)))
            {
                PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INTERNAL_ERROR);
                uiRet = ERROR_INSTALL_FAILURE;
                goto CleanUp;
            }
        }

        if (emAdminInstall == emExecMode)
        {
            // performing a patch
            DebugMsg("[Info] Calling MsiApplyPatch with szPatchPackage = %s", szMsiCacheFile);
            DebugMsg(" and szInstallPackage = %s and eInstallType = INSTALLTYPE_NETWORK_IMAGE", szAdminImagePath);
            DebugMsg(" and szCommandLine = %s\n", szProperties ? szProperties : "{null}");

            uiRet = pfnMsiApplyPatch(szMsiCacheFile, szAdminImagePath, INSTALLTYPE_NETWORK_IMAGE, szProperties);
        }
        else
        {
            // performing a patch
            DebugMsg("[Info] Calling MsiApplyPatch with szPatchPackage = %s", szInstallPath);
            DebugMsg(" and szInstallPackage = {null} and eInstallType = INSTALLTYPE_DEFAULT");
            DebugMsg(" and szCommandLine = %s\n", szProperties ? szProperties : "{null}");

            uiRet = pfnMsiApplyPatch(szInstallPath, NULL, INSTALLTYPE_DEFAULT, szProperties);
        }
        if (ERROR_SUCCESS != uiRet)
        {
            // attempt to display an error message stored in msi.dll
            PostMsiError(hInst, hMsi, DownloadUI.GetCurrentWindow(), szAppTitle, uiRet);
        }

        DebugMsg("[Info] MsiApplyPatch returned %d\n", uiRet);
    }

    
CleanUp:

    if (szMsiFile)
        delete [] szMsiFile;
    if (szBaseURL)
        delete [] szBaseURL;
    if (szInstallPath)
        delete [] szInstallPath;
    if (szMsiCacheFile)
    {
        WIN::DeleteUrlCacheEntry(szMsiCacheFile);
        delete [] szMsiCacheFile;
    }
    if (szProductName)
        delete [] szProductName;
    if (szMinimumMsi)
        delete [] szMinimumMsi;
    if (szProperties)
        delete [] szProperties;
    if (szTempPath)
        delete [] szTempPath;
    if (szBase)
        delete [] szBase;
    if (szUpdate)
        delete [] szUpdate;
    if (szRegisteredMsiFolder)
        delete [] szRegisteredMsiFolder;
    if (szMsiDllLocation)
        delete [] szMsiDllLocation;
    if (szOperation)
        delete [] szOperation;

    if(hMutex)
        CloseHandle(hMutex);

    if (hMsi)
        FreeLibrary(hMsi);

    DebugMsg("[Info] Setup exit code is %d\n", uiRet);

    if (fDelayRebootReq)
    {
        // need to reboot machine for updating Windows Installer
        WIN::LoadString(hInst, IDS_REBOOT_REQUIRED, szAction, MAX_STR_LENGTH);
        if (IDYES == MessageBox(NULL, szAction, szAppTitle, MB_YESNO|MB_ICONQUESTION))
        {
            // must first aquire system shutdown privileges on NT/Win2K
            AcquireShutdownPrivilege();
            // initiate system shutdown for reboot
            WIN::ExitWindowsEx(EWX_REBOOT, PCLEANUI | SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_INSTALLATION);
        }
    }

    return uiRet;
}

