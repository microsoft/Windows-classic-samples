//+-------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       setup.h
//
//--------------------------------------------------------------------------

#ifndef __SETUP_H_58FA8147_50A0_4FDC_BD83_17C3A2525E0A_
#define __SETUP_H_58FA8147_50A0_4FDC_BD83_17C3A2525E0A_

#include "setupui.h"

#include <windows.h>
#include <wincrypt.h>

/*--------------------------------------------------------------------------
 *
 * Constants
 *
 --------------------------------------------------------------------------*/
#define MAX_STR_LENGTH 1024
#define MINIMUM_SUPPORTED_MSI_VERSION 150
#define MAX_LENGTH_GUID 40

const char szUrlPathSep[] = "/";
const char szPathSep[] = "\\";

const char szDefaultOperation[] = "DEFAULT";
const char szInstallOperation[] = "INSTALL";
const char szMinPatchOperation[] = "MINPATCH";
const char szMajPatchOperation[] = "MAJPATCH";
const char szInstallUpdOperation[] = "INSTALLUPD";

const char szDefaultMinPatchCommandLine[] = "REINSTALL=ALL REINSTALLMODE=omus";
const char szDefaultInstallUpdCommandLine[] = "REINSTALL=ALL REINSTALLMODE=vomus";
const char szAdminInstallProperty[] = " ACTION=ADMIN";

const char sqlProductCode[] = "SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'";

/*--------------------------------------------------------------------------
 *
 * Enums
 *
 --------------------------------------------------------------------------*/
enum itvEnum
{
    itvWintrustNotOnMachine = 0,
    itvTrusted = 1,
    itvUnTrusted = 2
};

// Execution modes.
enum emEnum
{
    emPreset = 0,
    emHelp = 1,
    emVerify = 2,
    emAdminInstall = 3
};

/*--------------------------------------------------------------------------
 *
 * Prototypes
 *
 --------------------------------------------------------------------------*/

DWORD VerifyFileSignature (LPCSTR lpszModule, __in_opt LPSTR lpszCmdLine);
emEnum GetExecutionMode (LPCSTR lpszCmdLine);
DWORD GetNextArgument (LPCSTR pszCmdLine, LPCSTR *ppszArgStart, LPCSTR *ppszArgEnd, bool * pfQuoted);
DWORD GetAdminInstallInfo (bool fPatch, __in_opt LPSTR lpszCmdLine, LPCSTR * ppszAdminImagePath);
bool AlreadyInProgress(HANDLE& hMutex);
void DisplayUsage (HINSTANCE hInst, HWND hwndOwner, LPCSTR szCaption);
DWORD GetFileVersionNumber(__in LPSTR szFilename, DWORD *pdwMSVer, DWORD *pdwLSVer);
bool IsAdmin();
bool IsOSSupported();
bool AcquireShutdownPrivilege();

/////////////////////////////////////////////////////////////////////////////
//
// WinVerifyTrust functions
//
/////////////////////////////////////////////////////////////////////////////
itvEnum IsPackageTrusted(LPCSTR szSetupEXE, LPCSTR szPackage, HWND hwndParent);
itvEnum IsFileTrusted(LPCWSTR szwFile, HWND hwndParent, DWORD dwUIChoice, bool *pfIsSigned, PCCERT_CONTEXT *ppcSigner);

/////////////////////////////////////////////////////////////////////////////
//
// Upgrade functions
//
/////////////////////////////////////////////////////////////////////////////
bool IsMsiUpgradeNecessary(ULONG ulReqMsiMinVer);
DWORD ExecuteUpgradeMsi(__in LPSTR szUpgradeMsi);
DWORD ExecuteVerifyUpdate(LPCSTR szModuleFile, LPCSTR szUpdateCachePath);
DWORD WaitForProcess(HANDLE handle);
bool IsUpdateRequiredVersion(__in LPSTR szFilename, ULONG ulMinVer);
UINT UpgradeMsi(HINSTANCE hInst, CDownloadUI *piDownloadUI, LPCSTR szAppTitle, LPCSTR szUpgdLocation, LPCSTR szUpgrade, ULONG ulMinVer);
UINT DownloadAndUpgradeMsi(HINSTANCE hInst, CDownloadUI *piDownloadUI, LPCSTR szAppTitle, LPCSTR szBase, LPCSTR szUpdate, LPCSTR szModuleFile, ULONG ulMinVer);
UINT ValidateUpdate(HINSTANCE hInst, CDownloadUI *piDownloadUI, LPCSTR szAppTitle, __in LPSTR szUpdatePath, LPCSTR szModuleFile, ULONG ulMinVer);

/////////////////////////////////////////////////////////////////////////////
//
// Error handling functions
//
/////////////////////////////////////////////////////////////////////////////
void ReportErrorOutOfMemory(HINSTANCE hInst, HWND hwndOwner, LPCSTR szCaption);
void PostResourceNotFoundError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, LPCSTR szName);
void ReportUserCancelled(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle);
void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId);
void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, int iValue);
void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue);
void PostError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue, int iValue);
void PostMsiError(HINSTANCE hInst, HINSTANCE hMsi, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId);
void PostFormattedError(HINSTANCE hInst, HWND hwndOwner, LPCSTR szTitle, UINT uiErrorId, LPCSTR szValue);

/////////////////////////////////////////////////////////////////////////////
//
// Update command line options
//
//
/////////////////////////////////////////////////////////////////////////////
const char szDelayReboot[] = " /norestart";
const char szDelayRebootQuiet[] = " /quiet /norestart";

/////////////////////////////////////////////////////////////////////////////
//
// Debugging Functions
//
//
/////////////////////////////////////////////////////////////////////////////
void DebugMsg(LPCSTR szFormat, int iArg1);
void DebugMsg(LPCSTR szFormat, int iArg1, int iArg2);
void DebugMsg(LPCSTR szFormat, LPCSTR szArg1 = 0, LPCSTR szArg2 = 0);
const char szDebugEnvVar[] = "_MSI_WEB_BOOTSTRAP_DEBUG";


#endif //__SETUP_H_58FA8147_50A0_4FDC_BD83_17C3A2525E0A_
