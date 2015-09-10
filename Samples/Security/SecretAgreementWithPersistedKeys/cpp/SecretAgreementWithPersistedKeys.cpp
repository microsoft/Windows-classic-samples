// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             SecretAgreementWithPersistedKeys.cpp
//
//  Contents:         Sample program for ECDH 256 Secret Agreement using CNG.
//                    The sample also shows how to derive key from an agreed secret using BCRYPT_KDF_HMAC. 
//                    BCRYPT_KDF_HASH can be used with the same parameters.
//                    Persisted and ephemeral keys are used.
//                   
//

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>
#include <stdio.h>
#include <bcrypt.h>
#include <ncrypt.h>
#include <sal.h>

static 
const 
BYTE SecretPrependArray[] = 
{
    0x12, 0x34, 0x56
};

static
const 
BYTE SecretAppendArray[] = 
{
    0xab, 0xcd, 0xef
};

//
// Utilities and helper functions
//

//----------------------------------------------------------------------------
//
//  ReportError
//  Prints error information to the console
//
//----------------------------------------------------------------------------
void 
ReportError( 
    _In_    DWORD       dwErrCode 
    )
{
    wprintf( L"Error: 0x%08x (%d)\n", dwErrCode, dwErrCode );
}

//-----------------------------------------------------------------------------
//
//    wmain
//
//-----------------------------------------------------------------------------
DWORD
__cdecl
wmain(
    _In_               int     argc,
    _In_reads_(argc)   LPWSTR  argv[]
    )
{
    NCRYPT_PROV_HANDLE      ProviderHandleA = 0;
    NCRYPT_KEY_HANDLE       PrivKeyHandleA  = 0;
    NCRYPT_KEY_HANDLE       PubKeyHandleA   = 0;
    
    BCRYPT_ALG_HANDLE       ExchAlgHandleB  = NULL;
    BCRYPT_KEY_HANDLE       PrivKeyHandleB  = NULL;
    BCRYPT_KEY_HANDLE       PubKeyHandleB   = NULL;
    
    NTSTATUS                Status;
    SECURITY_STATUS         secStatus;
    
    PBYTE                   PubBlobA      = NULL,
                            PubBlobB      = NULL,
                            AgreedSecretA = NULL,
                            AgreedSecretB = NULL;
                            
    DWORD                   PubBlobLengthA = 0,
                            PubBlobLengthB = 0,
                            AgreedSecretLengthA = 0,
                            AgreedSecretLengthB = 0,
                            KeyPolicy = 0;
    NCRYPT_SECRET_HANDLE    AgreedSecretHandleA = NULL;
    BCRYPT_SECRET_HANDLE    AgreedSecretHandleB = NULL;
    BCryptBufferDesc        ParameterList = {0};
    
    const DWORD             BufferLength = 3;
    BCryptBuffer            BufferArray[BufferLength] = {0};

    
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);


    //
    // Get a handle to MS KSP
    //

    secStatus = NCryptOpenStorageProvider(
                                        &ProviderHandleA, 
                                        MS_KEY_STORAGE_PROVIDER, 
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Delete existing keys
    //

    secStatus = NCryptOpenKey(
                                        ProviderHandleA, 
                                        &PrivKeyHandleA,
                                        L"Sample ECDH Key",
                                        0, 
                                        0);
    if( SUCCEEDED(secStatus) )
    {
        secStatus = NCryptDeleteKey(PrivKeyHandleA,0);
        if( FAILED(secStatus) )
        {
            ReportError(secStatus);
            goto cleanup;
        }
        PrivKeyHandleA = 0;
    }

    //
    // A generates a private key
    // 

    secStatus = NCryptCreatePersistedKey(
                                        ProviderHandleA,            // Provider handle
                                        &PrivKeyHandleA,            // Key handle - will be created
                                        NCRYPT_ECDH_P256_ALGORITHM, // Alg name 
                                        L"Sample ECDH Key",         // Key name (null terminated unicode string)
                                        0,                          // legacy spec
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Make the key exportable
    //

    KeyPolicy = NCRYPT_ALLOW_EXPORT_FLAG;

    secStatus = NCryptSetProperty(
                                        PrivKeyHandleA,
                                        NCRYPT_EXPORT_POLICY_PROPERTY,
                                        (PBYTE)&KeyPolicy,
                                        sizeof(KeyPolicy),
                                        NCRYPT_PERSIST_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = NCryptFinalizeKey(
                                        PrivKeyHandleA,             // Key handle
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }


    //
    // A exports public key
    // 

    secStatus = NCryptExportKey(
                                        PrivKeyHandleA,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        NULL,                       // Parameter list
                                        NULL,                       // Buffer that recieves the key blob
                                        0,                          // Buffer length (in bytes)
                                        &PubBlobLengthA,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    PubBlobA = (PBYTE)HeapAlloc (
                                        GetProcessHeap (), 
                                        0, 
                                        PubBlobLengthA);
    if( NULL == PubBlobA )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }


    secStatus = NCryptExportKey(
                                        PrivKeyHandleA,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        NULL,                       // Parameter list
                                        PubBlobA,                   // Buffer that recieves the key blob
                                        PubBlobLengthA,             // Buffer length (in bytes)
                                        &PubBlobLengthA,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // B generates a private key
    // 

    Status = BCryptOpenAlgorithmProvider(
                                        &ExchAlgHandleB, 
                                        BCRYPT_ECDH_P256_ALGORITHM, 
                                        MS_PRIMITIVE_PROVIDER, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    Status = BCryptGenerateKeyPair(
                                        ExchAlgHandleB,             // Algorithm handle
                                        &PrivKeyHandleB,            // Key handle - will be created
                                        256,                        // Length of the key - in bits
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    
    Status = BCryptFinalizeKeyPair(
                                        PrivKeyHandleB,             // Key handle
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    //
    // B exports public key
    //

    Status = BCryptExportKey(
                                        PrivKeyHandleB,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        NULL,                       // Buffer that recieves the key blob
                                        0,                          // Buffer length (in bytes)
                                        &PubBlobLengthB,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    PubBlobB = (PBYTE)HeapAlloc (
                                        GetProcessHeap (), 
                                        0, 
                                        PubBlobLengthB);
    if( NULL == PubBlobB )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }


    Status = BCryptExportKey(
                                        PrivKeyHandleB,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        PubBlobB,                   // Buffer that recieves the key blob
                                        PubBlobLengthB,             // Buffer length (in bytes)
                                        &PubBlobLengthB,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    //
    // A imports B's public key
    //

    secStatus = NCryptImportKey(
                                        ProviderHandleA,            // Provider handle
                                        NULL,                       // Parameter not used
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (Null terminated unicode string)
                                        NULL,                       // Parameter list
                                        &PubKeyHandleA,             // Key handle that will be recieved
                                        PubBlobB,                   // Buffer than points to the key blob
                                        PubBlobLengthB,             // Buffer length in bytes
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // A generates the agreed secret
    //

    secStatus = NCryptSecretAgreement(
                                        PrivKeyHandleA,             // Private key handle
                                        PubKeyHandleA,              // Public key handle
                                        &AgreedSecretHandleA,       // Handle that represents the secret agreement value
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Build KDF parameter list
    //

    //specify hash algorithm
    BufferArray[0].BufferType = KDF_HASH_ALGORITHM;
    BufferArray[0].cbBuffer = (DWORD)((wcslen(BCRYPT_SHA256_ALGORITHM) + 1) * sizeof(WCHAR));
    BufferArray[0].pvBuffer = (PVOID)BCRYPT_SHA256_ALGORITHM;

    //specify secret to append
    BufferArray[1].BufferType = KDF_SECRET_APPEND;
    BufferArray[1].cbBuffer = sizeof(SecretAppendArray);
    BufferArray[1].pvBuffer = (PVOID)SecretAppendArray;

    //specify secret to prepend
    BufferArray[2].BufferType = KDF_SECRET_PREPEND;
    BufferArray[2].cbBuffer = sizeof(SecretPrependArray);
    BufferArray[2].pvBuffer = (PVOID)SecretPrependArray;

    ParameterList.cBuffers  = 3;
    ParameterList.pBuffers  = BufferArray;
    ParameterList.ulVersion = BCRYPTBUFFER_VERSION;

    secStatus = NCryptDeriveKey(
                                       AgreedSecretHandleA,         // Secret agreement handle
                                       BCRYPT_KDF_HMAC,             // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       NULL,                        // Buffer that recieves the derived key 
                                       0,                           // Length of the buffer
                                       &AgreedSecretLengthA,        // Number of bytes copied to the buffer
                                       KDF_USE_SECRET_AS_HMAC_KEY_FLAG);   // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    AgreedSecretA = (PBYTE)HeapAlloc(
                                        GetProcessHeap (), 
                                        0, 
                                        AgreedSecretLengthA);
    if( NULL == AgreedSecretA )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = NCryptDeriveKey(
                                       AgreedSecretHandleA,         // Secret agreement handle
                                       BCRYPT_KDF_HMAC,             // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       AgreedSecretA,               // Buffer that recieves the derived key 
                                       AgreedSecretLengthA,         // Length of the buffer
                                       &AgreedSecretLengthA,        // Number of bytes copied to the buffer
                                       KDF_USE_SECRET_AS_HMAC_KEY_FLAG);   // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // B imports A's public key
    //

    Status = BCryptImportKeyPair(
                                        ExchAlgHandleB,             // Alg handle
                                        NULL,                       // Parameter not used
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (Null terminated unicode string)
                                        &PubKeyHandleB,             // Key handle that will be recieved
                                        PubBlobA,                   // Buffer than points to the key blob
                                        PubBlobLengthA,             // Buffer length in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }
   
    //
    // B generates the agreed secret
    //

    Status = BCryptSecretAgreement(
                                        PrivKeyHandleB,             // Private key handle
                                        PubKeyHandleB,              // Public key handle
                                        &AgreedSecretHandleB,       // Handle that represents the secret agreement value
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    Status = BCryptDeriveKey(
                                       AgreedSecretHandleB,         // Secret agreement handle
                                       BCRYPT_KDF_HMAC,             // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       NULL,                        // Buffer that recieves the derived key 
                                       0,                           // Length of the buffer
                                       &AgreedSecretLengthB,        // Number of bytes copied to the buffer
                                       KDF_USE_SECRET_AS_HMAC_KEY_FLAG);    // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    AgreedSecretB = (PBYTE)HeapAlloc(
                                        GetProcessHeap (), 
                                        0, 
                                        AgreedSecretLengthB);
    if( NULL == AgreedSecretB )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }

    
    Status = BCryptDeriveKey(
                                       AgreedSecretHandleB,         // Secret agreement handle   
                                       BCRYPT_KDF_HMAC,             // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       AgreedSecretB,               // Buffer that recieves the derived key 
                                       AgreedSecretLengthB,         // Length of the buffer
                                       &AgreedSecretLengthB,        // Number of bytes copied to the buffer
                                       KDF_USE_SECRET_AS_HMAC_KEY_FLAG);    // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    //
    // At this point the AgreedSecretA should be the same as AgreedSecretB.
    // In a real scenario, the agreed secrets on both sides will probably 
    // be input to a BCryptGenerateSymmetricKey function. 
    // Optional : Compare them
    //

    if( (AgreedSecretLengthA != AgreedSecretLengthB) ||
        (memcmp(AgreedSecretA, AgreedSecretB, AgreedSecretLengthA))
        )
    {
        secStatus = NTE_FAIL;
        ReportError(secStatus);
        goto cleanup;

    }

    secStatus = S_OK;

    wprintf(L"Success!\n");

cleanup:

    if( PubKeyHandleA )    
    {
        NCryptFreeObject(PubKeyHandleA);
    }

    if( PubKeyHandleB )    
    {
        BCryptDestroyKey(PubKeyHandleB);
    }

    if( PrivKeyHandleA )    
    {
        NCryptDeleteKey(PrivKeyHandleA, 0);
    }

    if( PrivKeyHandleB )    
    {
        BCryptDestroyKey(PrivKeyHandleB);
    }
    
    if( ProviderHandleA )    
    {
        NCryptFreeObject(ProviderHandleA);
    } 

    if( ExchAlgHandleB )
    {
        BCryptCloseAlgorithmProvider(ExchAlgHandleB,0);
    }

    if( AgreedSecretHandleA )
    {
        NCryptFreeObject(AgreedSecretHandleA);
    }

    if( AgreedSecretHandleB )
    {
        BCryptDestroySecret(AgreedSecretHandleB);
    }

    if( PubBlobA )
    {
        HeapFree(GetProcessHeap(), 0, PubBlobA);
    }

    if( PubBlobB )
    {
        HeapFree(GetProcessHeap(), 0, PubBlobB);
    }

    if( AgreedSecretA )
    {
        HeapFree(GetProcessHeap(), 0, AgreedSecretA);
    }

    if( AgreedSecretB )
    {
        HeapFree(GetProcessHeap(), 0, AgreedSecretB);
    }

    return secStatus;
}

