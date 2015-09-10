// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <wintrust.h>
#include <softpub.h>
#include <mscat.h>

void PrintUsage(_In_ PCWSTR fileName)
{
    wprintf(L"%s [-p] <-c | -e> file\n", fileName);
    wprintf(L"Flags:\n");
    wprintf(L"  -p: Use signature policy of the current os (szOID_CERT_STRONG_SIGN_OS_CURRENT)\n");
    wprintf(L"  -c: Search for the file in system catalogs\n");
    wprintf(L"  -e: Verify embedded file signature\n");
}

//----------------------------------------------------------------------------
//
//  PrintError
//  Prints error information to the console
//
//----------------------------------------------------------------------------
void PrintError(_In_ DWORD Status)
{
    wprintf(L"Error: 0x%08x (%d)\n", Status, Status);
}

//----------------------------------------------------------------------------
//
//  VerifyEmbeddedSignatures
//  Verifies all embedded signatures of a file
//
//----------------------------------------------------------------------------
DWORD VerifyEmbeddedSignatures(_In_ PCWSTR FileName,
                               _In_ HANDLE FileHandle,
                               _In_ bool UseStrongSigPolicy)
{
    DWORD Error = ERROR_SUCCESS;    
    bool WintrustCalled = false;
    GUID GenericActionId = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WintrustData = {};
    WINTRUST_FILE_INFO FileInfo = {};    
    WINTRUST_SIGNATURE_SETTINGS SignatureSettings = {};
    CERT_STRONG_SIGN_PARA StrongSigPolicy = {};

    // Setup data structures for calling WinVerifyTrust
    WintrustData.cbStruct = sizeof(WINTRUST_DATA);
    WintrustData.dwStateAction  = WTD_STATEACTION_VERIFY;
    WintrustData.dwUIChoice = WTD_UI_NONE;
    WintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WintrustData.dwUnionChoice = WTD_CHOICE_FILE;
    
    FileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO_);
    FileInfo.hFile = FileHandle;
    FileInfo.pcwszFilePath = FileName;
    WintrustData.pFile = &FileInfo;
    
    //
    // First verify the primary signature (index 0) to determine how many secondary signatures
    // are present. We use WSS_VERIFY_SPECIFIC and dwIndex to do this, also setting 
    // WSS_GET_SECONDARY_SIG_COUNT to have the number of secondary signatures returned.
    //
    SignatureSettings.cbStruct = sizeof(WINTRUST_SIGNATURE_SETTINGS);
    SignatureSettings.dwFlags = WSS_GET_SECONDARY_SIG_COUNT | WSS_VERIFY_SPECIFIC;
    SignatureSettings.dwIndex = 0;
    WintrustData.pSignatureSettings = &SignatureSettings;
    
    if (UseStrongSigPolicy != false)
    {
        StrongSigPolicy.cbSize = sizeof(CERT_STRONG_SIGN_PARA);
        StrongSigPolicy.dwInfoChoice = CERT_STRONG_SIGN_OID_INFO_CHOICE;
        StrongSigPolicy.pszOID = szOID_CERT_STRONG_SIGN_OS_CURRENT;
        WintrustData.pSignatureSettings->pCryptoPolicy = &StrongSigPolicy;
    }
    
    wprintf(L"Verifying primary signature... ");
    Error = WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
    WintrustCalled = true;
    if (Error != ERROR_SUCCESS) 
    {
        PrintError(Error);        
        goto Cleanup;
    }

    wprintf(L"Success!\n");

    wprintf(L"Found %d secondary signatures\n", WintrustData.pSignatureSettings->cSecondarySigs);

    // Now attempt to verify all secondary signatures that were found
    for(DWORD x = 1; x <= WintrustData.pSignatureSettings->cSecondarySigs; x++)
    {
        wprintf(L"Verify secondary signature at index %d... ", x);

        // Need to clear the previous state data from the last call to WinVerifyTrust
        WintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        Error = WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
        if (Error != ERROR_SUCCESS) 
        {
            //No need to call WinVerifyTrust again
            WintrustCalled = false;
            PrintError(Error);            
            goto Cleanup;
        }

        WintrustData.hWVTStateData = NULL;

        // Caller must reset dwStateAction as it may have been changed during the last call
        WintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
        WintrustData.pSignatureSettings->dwIndex = x;
        Error = WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
        if (Error != ERROR_SUCCESS) 
        {
            PrintError(Error);            
            goto Cleanup;
        }

        wprintf(L"Success!\n");
    }

Cleanup:

    //
    // Caller must call WinVerifyTrust with WTD_STATEACTION_CLOSE to free memory
    // allocate by WinVerifyTrust
    //
    if (WintrustCalled != false)
    {
        WintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(NULL, &GenericActionId, &WintrustData);
    }

    return Error;     
}


//----------------------------------------------------------------------------
//
//  VerifyCatalogSignature
//  Looks up a file by hash in the system catalogs. 
//
//----------------------------------------------------------------------------
DWORD VerifyCatalogSignature(_In_ HANDLE FileHandle,
                             _In_ bool UseStrongSigPolicy)
{
    DWORD Error = ERROR_SUCCESS;
    bool Found = false;
    HCATADMIN CatAdminHandle = NULL;
    HCATINFO CatInfoHandle = NULL;
    DWORD HashLength = 0;
    PBYTE HashData = NULL;
    CERT_STRONG_SIGN_PARA SigningPolicy = {};

    if (UseStrongSigPolicy != false)
    {
        SigningPolicy.cbSize = sizeof(CERT_STRONG_SIGN_PARA);
        SigningPolicy.dwInfoChoice = CERT_STRONG_SIGN_OID_INFO_CHOICE;
        SigningPolicy.pszOID = szOID_CERT_STRONG_SIGN_OS_CURRENT;
        if (!CryptCATAdminAcquireContext2(
                    &CatAdminHandle,
                    NULL,
                    BCRYPT_SHA256_ALGORITHM,
                    &SigningPolicy,
                    0))
        {
            Error = GetLastError();
            goto Cleanup;
        }
    }
    else
    {
        if (!CryptCATAdminAcquireContext2(
                    &CatAdminHandle,
                    NULL,
                    BCRYPT_SHA256_ALGORITHM,
                    NULL,
                    0))
        {
            Error = GetLastError();
            goto Cleanup;
        }
    }

    // Get size of hash to be used
    if (!CryptCATAdminCalcHashFromFileHandle2(
                CatAdminHandle,
                FileHandle,
                &HashLength,
                NULL,
                NULL))
    {
        Error = GetLastError();
        goto Cleanup;
    }

    HashData = (PBYTE) HeapAlloc(GetProcessHeap(), 0, HashLength);
    if (HashData == NULL)
    {
        Error = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    // Generate hash for a give file
    if (!CryptCATAdminCalcHashFromFileHandle2(
                CatAdminHandle,
                FileHandle,
                &HashLength,
                HashData,
                NULL))
    {
        Error = GetLastError();
        goto Cleanup;
    }

    // Find the first catalog containing this hash
    CatInfoHandle = NULL;
    CatInfoHandle = CryptCATAdminEnumCatalogFromHash(
                CatAdminHandle,
                HashData,
                HashLength,
                0,
                &CatInfoHandle);

    while (CatInfoHandle != NULL)
    {
        CATALOG_INFO catalogInfo = {};
        catalogInfo.cbStruct = sizeof(catalogInfo);
        Found = true;

        if (!CryptCATCatalogInfoFromContext(
                    CatInfoHandle,
                    &catalogInfo,
                    0))
        {
            Error = GetLastError();
            break;
        }

        wprintf(L"Hash was found in catalog %s\n\n", catalogInfo.wszCatalogFile);

        // Look for the next catalog containing the file's hash
        CatInfoHandle = CryptCATAdminEnumCatalogFromHash(
                    CatAdminHandle,
                    HashData,
                    HashLength,
                    0,
                    &CatInfoHandle);
    }

    if (Found != true)
    {
        wprintf(L"Hash was not found in any catalogs.\n");
    }

Cleanup:
    if (CatAdminHandle != NULL)
    {
        if (CatInfoHandle != NULL)
        {
            CryptCATAdminReleaseCatalogContext(CatAdminHandle, CatInfoHandle, 0);
        }

        CryptCATAdminReleaseContext(CatAdminHandle, 0);
    }

    if (HashData != NULL)
    {
        HeapFree(GetProcessHeap(), 0, HashData);
    }

     return Error;
}

int __cdecl wmain(_In_ unsigned int argc, _In_reads_(argc) PCWSTR wargv[])
{
    DWORD Error = ERROR_SUCCESS;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    DWORD ArgStart = 1;
    bool UseStrongSigPolicy = false;

    if (argc < 3 || argc > 4)
    {
        PrintUsage(wargv[0]);
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (_wcsicmp(wargv[ArgStart], L"-p") == 0)
    {
        UseStrongSigPolicy = true;
        ArgStart++;
    }

    if (ArgStart + 1 >= argc)
    {
        PrintUsage(wargv[0]);
        Error = ERROR_INVALID_PARAMETER;        
        goto Cleanup;
    }
    
    if ((wcslen(wargv[ArgStart]) != 2) ||
        ((_wcsicmp(wargv[ArgStart], L"-c") != 0) &&
         (_wcsicmp(wargv[ArgStart], L"-e") != 0)))
    {   
        PrintUsage(wargv[0]);
        Error = ERROR_INVALID_PARAMETER;        
        goto Cleanup;
    }

    FileHandle = CreateFileW(wargv[ArgStart+1],
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();
        PrintError(Error);        
        goto Cleanup;
    }

    if (_wcsicmp(wargv[ArgStart], L"-c") == 0)
    {
        Error = VerifyCatalogSignature(FileHandle, UseStrongSigPolicy);
    }else if (_wcsicmp(wargv[ArgStart], L"-e") == 0)
    {
        Error = VerifyEmbeddedSignatures(wargv[ArgStart+1], FileHandle, UseStrongSigPolicy);
    }else
    {
        PrintUsage(wargv[0]);
        Error = ERROR_INVALID_PARAMETER;
    }

Cleanup:
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(FileHandle);
    }

    return Error;
}

