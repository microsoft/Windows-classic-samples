//+-------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       vertrust.cpp
//
//--------------------------------------------------------------------------

#define WIN // scope W32 API
#define MSI // scope MSI API

#include <windows.h>
#include <tchar.h>

#include "setup.h" // for itvEnum
#include "common.h"

// package trust
#include "wintrust.h"
#include "softpub.h"

//--------------------------------------------------------------------------------------
// CRYPTO API -- delay load
//--------------------------------------------------------------------------------------

#define CRYPT32_DLL "crypt32.dll"

#define CRYPTOAPI_CertDuplicateCertificateContext "CertDuplicateCertificateContext"
typedef PCCERT_CONTEXT (WINAPI* PFnCertDuplicateCertificateContext)(PCCERT_CONTEXT pCertContext);

#define CRYPTOAPI_CertCompareCertificate "CertCompareCertificate"
typedef BOOL (WINAPI* PFnCertCompareCertificate)(DWORD dwCertEncodingType, PCERT_INFO pCertId1, PCERT_INFO pCertId2);

#define CRYPTOAPI_CertFreeCertificateContext "CertFreeCertificateContext"
typedef BOOL (WINAPI* PFnCertFreeCertificateContext)(PCCERT_CONTEXT pCertContext);

//--------------------------------------------------------------------------------------
// WINTRUST API -- delay load
//--------------------------------------------------------------------------------------

#define WINTRUST_DLL "wintrust.dll"

#define WINTRUSTAPI_WinVerifyTrust "WinVerifyTrust"
typedef HRESULT (WINAPI *PFnWinVerifyTrust)(HWND hwnd, GUID *pgActionID, WINTRUST_DATA *pWinTrustData);

#define WINTRUSTAPI_WTHelperProvDataFromStateData "WTHelperProvDataFromStateData"
typedef PCRYPT_PROVIDER_DATA (WINAPI *PFnWTHelperProvDataFromStateData)(HANDLE hStateData);

#define WINTRUSTAPI_WTHelperGetProvSignerFromChain "WTHelperGetProvSignerFromChain"
typedef PCRYPT_PROVIDER_SGNR (WINAPI *PFnWTHelperGetProvSignerFromChain)(PCRYPT_PROVIDER_DATA pProvData, DWORD idxSigner, BOOL fCounterSigner, DWORD idxCounterSigner);

#define WINTRUSTAPI_WTHelperGetProvCertFromChain "WTHelperGetProvCertFromChain"
typedef PCRYPT_PROVIDER_CERT (WINAPI* PFnWTHelperGetProvCertFromChain)(PCRYPT_PROVIDER_SGNR pSgnr, DWORD idxCert);

//--------------------------------------------------------------------------------------
// download provider
//--------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// IsFileTrusted
//
itvEnum IsFileTrusted(LPCWSTR lpwFile, HWND hwndParent, DWORD dwUIChoice, bool *pfIsSigned, PCCERT_CONTEXT *ppcSigner)
{
    char szDebugOutput[MAX_STR_LENGTH] = {0};

    itvEnum itv = itvUnTrusted;

    if (pfIsSigned)
        *pfIsSigned = false;
    if (ppcSigner)
        *ppcSigner  = 0;

    GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    WINTRUST_FILE_INFO sWintrustFileInfo;
    WINTRUST_DATA      sWintrustData;
    HRESULT            hr;

    memset((void*)&sWintrustFileInfo, 0x00, sizeof(WINTRUST_FILE_INFO)); // zero out
    memset((void*)&sWintrustData, 0x00, sizeof(WINTRUST_DATA)); // zero out

    sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    sWintrustFileInfo.pcwszFilePath = lpwFile;
    sWintrustFileInfo.hFile = NULL;

    sWintrustData.cbStruct            = sizeof(WINTRUST_DATA);
    sWintrustData.dwUIChoice          = dwUIChoice;
    sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    sWintrustData.dwUnionChoice       = WTD_CHOICE_FILE;
    sWintrustData.pFile               = &sWintrustFileInfo;
    sWintrustData.dwStateAction       = (ppcSigner) ? WTD_STATEACTION_VERIFY : 0;

    HMODULE hWinTrust = LoadLibrary(WINTRUST_DLL);
    if (!hWinTrust)
    {
        // WinTrust is unavailable on the machine
        return itvWintrustNotOnMachine;
    }
    PFnWinVerifyTrust pfnWinVerifyTrust = (PFnWinVerifyTrust)GetProcAddress(hWinTrust, WINTRUSTAPI_WinVerifyTrust);
    PFnWTHelperProvDataFromStateData pfnWTHelperProvDataFromStateData= (PFnWTHelperProvDataFromStateData)GetProcAddress(hWinTrust, WINTRUSTAPI_WTHelperProvDataFromStateData);
    PFnWTHelperGetProvSignerFromChain pfnWTHelperGetProvSignerFromChain = (PFnWTHelperGetProvSignerFromChain)GetProcAddress(hWinTrust, WINTRUSTAPI_WTHelperGetProvSignerFromChain);
    PFnWTHelperGetProvCertFromChain pfnWTHelperGetProvCertFromChain = (PFnWTHelperGetProvCertFromChain)GetProcAddress(hWinTrust, WINTRUSTAPI_WTHelperGetProvCertFromChain);
    if (!pfnWinVerifyTrust || !pfnWTHelperProvDataFromStateData || !pfnWTHelperGetProvSignerFromChain || !pfnWTHelperGetProvCertFromChain)
    {
        // WinTrust is unavailable on the machine
        FreeLibrary(hWinTrust);
        return itvWintrustNotOnMachine;
    }

    hr = pfnWinVerifyTrust(/* UI Window Handle */ (dwUIChoice == WTD_UI_NONE) ? (HWND)INVALID_HANDLE_VALUE : hwndParent, &guidAction, &sWintrustData);
    DebugMsg("[WVT] WVT returned 0x%X\n", hr);

    itv = (TRUST_E_PROVIDER_UNKNOWN == hr) ? itvWintrustNotOnMachine : ((S_OK == hr) ? itvTrusted : itvUnTrusted); 

    if (itvWintrustNotOnMachine == itv)
    {
        // release state data
        sWintrustData.dwUIChoice = WTD_UI_NONE;
        sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        pfnWinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);

        FreeLibrary(hWinTrust);
        return itv; // return immediately
    }

    if (pfIsSigned)
        *pfIsSigned = (TRUST_E_NOSIGNATURE == hr) ? false : true;

    if (TRUST_E_NOSIGNATURE == hr)
    {
        // release state data
        sWintrustData.dwUIChoice = WTD_UI_NONE;
        sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        pfnWinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);

        FreeLibrary(hWinTrust);
        return itv;
    }

    if (ppcSigner)
    {
        CRYPT_PROVIDER_DATA const *psProvData     = NULL;
        CRYPT_PROVIDER_SGNR       *psProvSigner   = NULL;
        CRYPT_PROVIDER_CERT       *psProvCert     = NULL;

        // grab the provider data
        psProvData = pfnWTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
        if (psProvData)
        {
            // grab the signer data from the CRYPT_PROV_DATA
            psProvSigner = pfnWTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0 /*first signer*/, FALSE /* not a counter signer */, 0);
            if (psProvSigner)
            {
                // grab the signer cert from CRYPT_PROV_SGNR (pos 0 = signer cert; pos csCertChain-1 = root cert)
                psProvCert = pfnWTHelperGetProvCertFromChain(psProvSigner, 0);
            }
        }
    
        if (!psProvCert)
        {
            // some failure in obtaining the signer cert data
            *ppcSigner = 0;
        }
        else
        {
            // duplicate the cert
            HMODULE hCrypt32 = LoadLibrary(CRYPT32_DLL);
            if (hCrypt32)
            {
                PFnCertDuplicateCertificateContext pfnCertDuplicateCertificateContext = (PFnCertDuplicateCertificateContext)GetProcAddress(hCrypt32, CRYPTOAPI_CertDuplicateCertificateContext);
                if (pfnCertDuplicateCertificateContext)
                    *ppcSigner = pfnCertDuplicateCertificateContext(psProvCert->pCert);
                FreeLibrary(hCrypt32);
            }
        }

        // release state data
        sWintrustData.dwUIChoice = WTD_UI_NONE;
        sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        pfnWinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
    }

    FreeLibrary(hWinTrust);
    return itv;
}

/////////////////////////////////////////////////////////////////////////////
// IsPackageTrusted
//

itvEnum IsPackageTrusted(LPCSTR szSetupExe, LPCSTR szPackage, HWND hwndParent)
{
    WCHAR *szwSetup   = 0;
    WCHAR *szwPackage = 0;
    int   cchWide     = 0;

    bool    fPackageIsTrusted = false;
    bool    fSetupExeIsSigned = false;
    bool    fPackageIsSigned  = false;
    itvEnum itv               = itvUnTrusted;

    DWORD dwUILevel = 0;

    char szDebugOutput[MAX_STR_LENGTH] = {0};

    PCCERT_CONTEXT pcExeSigner = 0;
    PCCERT_CONTEXT pcMsiSigner = 0;

    HMODULE hCrypt32 = LoadLibrary(CRYPT32_DLL);
    if (!hCrypt32)
    {
        // no crypto on the machine
        return itvWintrustNotOnMachine;
    }
    PFnCertCompareCertificate pfnCertCompareCertificate = (PFnCertCompareCertificate)GetProcAddress(hCrypt32, CRYPTOAPI_CertCompareCertificate);
    PFnCertFreeCertificateContext pfnCertFreeCertificateContext = (PFnCertFreeCertificateContext)GetProcAddress(hCrypt32, CRYPTOAPI_CertFreeCertificateContext);
    if (!pfnCertCompareCertificate || !pfnCertFreeCertificateContext)
    {
        // no crypt on the machine
        FreeLibrary(hCrypt32);
        return itvWintrustNotOnMachine;
    }

    // convert szSetupExe to WIDE
    cchWide = MultiByteToWideChar(CP_ACP, 0, szSetupExe, -1, 0, 0);
    szwSetup = new WCHAR[cchWide];
    if (!szwSetup)
    {
        // out of memory
        FreeLibrary(hCrypt32);
        return itvUnTrusted;
    }
    if (0 == MultiByteToWideChar(CP_ACP, 0, szSetupExe, -1, szwSetup, cchWide))
    {
        // failed to convert string
        FreeLibrary(hCrypt32);
        delete [] szwSetup;
        return itvUnTrusted;
    }

    //
    // step 1: silently call WinVerifyTrust on szSetupExe, ignore return value - except for ivtWintrustNotOnMachine
    //

    DebugMsg("[WVT] step 1: silently call WinVerifyTrust on szSetupExe, ignoring return value\n");

    if (itvWintrustNotOnMachine == (itv = IsFileTrusted(szwSetup, hwndParent, WTD_UI_NONE, &fSetupExeIsSigned, &pcExeSigner)))
    {
        goto CleanUp;
    }

    DebugMsg("[WVT] fSetupExeIsSigned = %s\n", fSetupExeIsSigned ? "TRUE" : "FALSE");

    // convert szPackage to WIDE
    cchWide = MultiByteToWideChar(CP_ACP, 0, szPackage, -1, 0, 0);
    szwPackage = new WCHAR[cchWide];
    if (!szwPackage)
    {
        // out of memory
        FreeLibrary(hCrypt32);
        return itvUnTrusted;
    }
    if (0 == MultiByteToWideChar(CP_ACP, 0, szPackage, -1, szwPackage, cchWide))
    {
        // failed to convert string
        FreeLibrary(hCrypt32);
        return itvUnTrusted;
    }

    //
    // step 2: silently call WinVerifyTrust on szPackage, ignore return value - except for ivtWintrustNotOnMachine
    //

    if (fSetupExeIsSigned)
    {
        DebugMsg("[WVT] step2: silently call WinVerifyTrust on szPackage, ignoring return value\n");
        if (itvWintrustNotOnMachine == (itv = IsFileTrusted(szwPackage, hwndParent, WTD_UI_NONE, &fPackageIsSigned, &pcMsiSigner)))
        {
            goto CleanUp;
        }

        DebugMsg("[WVT] fPackageIsSigned = %s\n", fPackageIsSigned ? "TRUE" : "FALSE");
    }

    //
    // step 3: call WinVerifyTrust on szPackage, return value matters; use proper UI-level
    //

    if ( !fSetupExeIsSigned  // exe is not signed
        || !fPackageIsSigned // package is not signed
        || !pcExeSigner      // exe signer cert is missing
        || !pcMsiSigner      // package signer cert is missing
        || !pfnCertCompareCertificate(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pcExeSigner->pCertInfo, pcMsiSigner->pCertInfo)) // signed by different certs
    {
        // always show UI
        DebugMsg("[WVT] step3: last call to WinVerifyTrust using full UI\n");
        dwUILevel = WTD_UI_ALL;
    }
    else
    {
        // show UI only if bad
        DebugMsg("[WVT] step3: last call to WinVerifyTrust showing UI only if something is wrong\n");
        dwUILevel = WTD_UI_NOGOOD;
    }

    itv = IsFileTrusted(szwPackage, hwndParent, dwUILevel, NULL, NULL);

    //
    // cleanup
    //

CleanUp:
    if (szwPackage)
        delete [] szwPackage;
    if (szwSetup)
        delete [] szwSetup;

    if (pcExeSigner)
        pfnCertFreeCertificateContext(pcExeSigner);
    if (pcMsiSigner)
        pfnCertFreeCertificateContext(pcMsiSigner);

    FreeLibrary(hCrypt32);

    return itv;
}