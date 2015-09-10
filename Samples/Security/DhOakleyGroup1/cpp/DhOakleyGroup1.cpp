// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             DhOakleyGroup1.cpp
//
//  Contents:         Sample program for DH Oakley group1 Secret Agreement using CNG
//                    http://www.ietf.org/rfc/rfc2409.txt?number=2409
//                    Uses ephemeral keys (group1 = 768 bits key)
//

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>
#include <stdio.h>
#include <bcrypt.h>
#include <sal.h>

static 
const 
BYTE OakleyGroup1P[] = 
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f,
    0xda, 0xa2, 0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62, 0x8b,
    0x80, 0xdc, 0x1c, 0xd1, 0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67,
    0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6, 0x3b, 0x13, 0x9b, 0x22,
    0x51, 0x4a, 0x08, 0x79, 0x8e, 0x34, 0x04, 0xdd, 0xef, 0x95,
    0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b, 0x30, 0x2b, 0x0a, 0x6d,
    0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d, 0x6d, 0x51,
    0xc2, 0x45, 0xe4, 0x85, 0xb5, 0x76, 0x62, 0x5e, 0x7e, 0xc6,
    0xf4, 0x4c, 0x42, 0xe9, 0xa6, 0x3a, 0x36, 0x20, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static 
const 
BYTE OakleyGroup1G[] = 
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02
};

static 
const 
BYTE rgbrgbTlsSeed[] = 
{
    0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65, 0x63, 0x64, 
    0x65, 0x66, 0x64, 0x65, 0x66, 0x67, 0x65, 0x66, 0x67, 0x68, 
    0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6a, 0x68, 0x69, 
    0x6a, 0x6b, 0x69, 0x6a, 0x6b, 0x6c, 0x6a, 0x6b, 0x6c, 0x6d, 
    0x6b, 0x6c, 0x6d, 0x6e, 0x6c, 0x6d, 0x6e, 0x6f, 0x6d, 0x6e, 
    0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6a, 0x68, 0x69, 
    0x6f, 0x70, 0x6e, 0x6f
};

LPCWSTR  Label      = L"MyTlsLabel";


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
    BCRYPT_ALG_HANDLE       ExchAlgHandleA  = NULL;
    BCRYPT_ALG_HANDLE       ExchAlgHandleB  = NULL;
    BCRYPT_KEY_HANDLE       PrivKeyHandleA  = NULL;
    BCRYPT_KEY_HANDLE       PubKeyHandleA   = NULL;
    BCRYPT_KEY_HANDLE       PrivKeyHandleB  = NULL;
    BCRYPT_KEY_HANDLE       PubKeyHandleB   = NULL;
    
    NTSTATUS                Status;
    
    PBYTE                   PubBlobA      = NULL,
                            PubBlobB      = NULL,
                            AgreedSecretA = NULL,
                            AgreedSecretB = NULL,
                            DhParamBlob = NULL;
    DWORD                   PubBlobLengthA = 0,
                            PubBlobLengthB = 0,
                            AgreedSecretLengthA = 0,
                            AgreedSecretLengthB = 0,
                            DhParamBlobLength = 0,
                            KeyLength = 0;
    BCRYPT_SECRET_HANDLE    AgreedSecretHandleA = NULL,
                            AgreedSecretHandleB = NULL;
    BCryptBufferDesc        ParameterList = {0};
    
    const DWORD             BufferLength = 2;
    BCryptBuffer            BufferArray[BufferLength] = {0};

    BCRYPT_DH_PARAMETER_HEADER  *DhParamHdrPointer = NULL;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);


    KeyLength = 768;//bits

    //
    // Construct the DH parameter blob. this is the only supported
    // method for DH in CNG.
    //
    // Calculate size of param blob and allocate memory

    DhParamBlobLength = sizeof(BCRYPT_DH_PARAMETER_HEADER) + 
                    sizeof(OakleyGroup1G) + 
                    sizeof(OakleyGroup1P);

    DhParamBlob = (PBYTE)HeapAlloc (
                                        GetProcessHeap (), 
                                        0, 
                                        DhParamBlobLength);
    if( NULL == DhParamBlob )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    DhParamHdrPointer  = (BCRYPT_DH_PARAMETER_HEADER *)DhParamBlob;

    //
    // Set header properties on param blob
    //

    DhParamHdrPointer->cbLength      = DhParamBlobLength;
    DhParamHdrPointer->cbKeyLength   = KeyLength/8;//bytes
    DhParamHdrPointer->dwMagic       = BCRYPT_DH_PARAMETERS_MAGIC;

    //
    // Set prime
    //

    memcpy(DhParamBlob + sizeof(BCRYPT_DH_PARAMETER_HEADER),
            OakleyGroup1P,
            sizeof(OakleyGroup1P));

    //
    // Set generator
    //

    memcpy(DhParamBlob + sizeof(BCRYPT_DH_PARAMETER_HEADER) + sizeof(OakleyGroup1P),
           OakleyGroup1G,
           sizeof(OakleyGroup1G));


    //
    // Open alg provider handle
    //

    Status = BCryptOpenAlgorithmProvider(
                                        &ExchAlgHandleA, 
                                        BCRYPT_DH_ALGORITHM, 
                                        NULL, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptOpenAlgorithmProvider(
                                        &ExchAlgHandleB, 
                                        BCRYPT_DH_ALGORITHM, 
                                        NULL, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // A generates a private key
    // 

    Status = BCryptGenerateKeyPair(
                                        ExchAlgHandleA,             // Algorithm handle
                                        &PrivKeyHandleA,            // Key handle - will be created
                                        KeyLength,                  // Length of the key - in bits
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptSetProperty(
                                        PrivKeyHandleA,
                                        BCRYPT_DH_PARAMETERS,
                                        DhParamBlob,
                                        DhParamBlobLength,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptFinalizeKeyPair(
                                        PrivKeyHandleA,             // Key handle
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }


    //
    // A exports DH public key
    // 

    Status = BCryptExportKey(
                                        PrivKeyHandleA,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_DH_PUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        NULL,                       // Buffer that recieves the key blob
                                        0,                          // Buffer length (in bytes)
                                        &PubBlobLengthA,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    PubBlobA = (PBYTE)HeapAlloc (
                                        GetProcessHeap (), 
                                        0, 
                                        PubBlobLengthA);
    if( NULL == PubBlobA )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }


    Status = BCryptExportKey(
                                        PrivKeyHandleA,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_DH_PUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        PubBlobA,                   // Buffer that recieves the key blob
                                        PubBlobLengthA,             // Buffer length (in bytes)
                                        &PubBlobLengthA,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // B generates a private key
    // 

    Status = BCryptGenerateKeyPair(
                                        ExchAlgHandleB,             // Algorithm handle
                                        &PrivKeyHandleB,            // Key handle - will be created
                                        KeyLength,                  // Length of the key - in bits
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptSetProperty(
                                        PrivKeyHandleB,
                                        BCRYPT_DH_PARAMETERS,
                                        DhParamBlob,
                                        DhParamBlobLength,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptFinalizeKeyPair(
                                        PrivKeyHandleB,             // Key handle
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // B exports DH public key
    //

    Status = BCryptExportKey(
                                        PrivKeyHandleB,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_DH_PUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        NULL,                       // Buffer that recieves the key blob
                                        0,                          // Buffer length (in bytes)
                                        &PubBlobLengthB,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    PubBlobB = (PBYTE)HeapAlloc (
                                        GetProcessHeap (), 
                                        0, 
                                        PubBlobLengthB);
    if( NULL == PubBlobB )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }


    Status = BCryptExportKey(
                                        PrivKeyHandleB,             // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_DH_PUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        PubBlobB,                   // Buffer that recieves the key blob
                                        PubBlobLengthB,             // Buffer length (in bytes)
                                        &PubBlobLengthB,            // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // A imports B's public key
    //

    Status = BCryptImportKeyPair(
                                        ExchAlgHandleA,             // Alg handle
                                        NULL,                       // Parameter not used
                                        BCRYPT_DH_PUBLIC_BLOB,      // Blob type (Null terminated unicode string)
                                        &PubKeyHandleA,             // Key handle that will be recieved
                                        PubBlobB,                   // Buffer than points to the key blob
                                        PubBlobLengthB,             // Buffer length in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptCloseAlgorithmProvider(
                                        ExchAlgHandleA,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    ExchAlgHandleA = 0;
    
    //
    // Build KDF parameter list
    //

    //specify hash algorithm, SHA1 if null

    //specify secret to append
    BufferArray[0].BufferType = KDF_TLS_PRF_SEED;
    BufferArray[0].cbBuffer = sizeof(rgbrgbTlsSeed);
    BufferArray[0].pvBuffer = (PVOID)rgbrgbTlsSeed;

    //specify secret to prepend
    BufferArray[1].BufferType = KDF_TLS_PRF_LABEL;
    BufferArray[1].cbBuffer = (DWORD)((wcslen(Label) + 1) * sizeof(WCHAR));
    BufferArray[1].pvBuffer = (PVOID)Label;

    ParameterList.cBuffers  = 2;
    ParameterList.pBuffers  = BufferArray;
    ParameterList.ulVersion = BCRYPTBUFFER_VERSION;

    //
    // A generates the agreed secret
    //

    Status = BCryptSecretAgreement(
                                        PrivKeyHandleA,             // Private key handle
                                        PubKeyHandleA,              // Public key handle
                                        &AgreedSecretHandleA,       // Handle that represents the secret agreement value
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptDeriveKey(
                                       AgreedSecretHandleA,         // Secret agreement handle
                                       BCRYPT_KDF_TLS_PRF,          // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       NULL,                        // Buffer that recieves the derived key 
                                       0,                           // Length of the buffer
                                       &AgreedSecretLengthA,        // Number of bytes copied to the buffer
                                       0);                          // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    AgreedSecretA = (PBYTE)HeapAlloc(
                                        GetProcessHeap (), 
                                        0, 
                                        AgreedSecretLengthA);
    if( NULL == AgreedSecretA )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptDeriveKey(
                                       AgreedSecretHandleA,         // Secret agreement handle
                                       BCRYPT_KDF_TLS_PRF,          // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       AgreedSecretA,               // Buffer that recieves the derived key 
                                       AgreedSecretLengthA,         // Length of the buffer
                                       &AgreedSecretLengthA,        // Number of bytes copied to the buffer
                                       0);                          // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // B imports A's public key
    //

    Status = BCryptImportKeyPair(
                                        ExchAlgHandleB,             // Alg handle
                                        NULL,                       // Parameter not used
                                        BCRYPT_DH_PUBLIC_BLOB,      // Blob type (Null terminated unicode string)
                                        &PubKeyHandleB,             // Key handle that will be recieved
                                        PubBlobA,                   // Buffer than points to the key blob
                                        PubBlobLengthA,             // Buffer length in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptCloseAlgorithmProvider(
                                        ExchAlgHandleB,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    ExchAlgHandleB = 0;
    
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
        goto cleanup;
    }

    Status = BCryptDeriveKey(
                                       AgreedSecretHandleB,         // Secret agreement handle
                                       BCRYPT_KDF_TLS_PRF,          // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       NULL,                        // Buffer that recieves the derived key 
                                       0,                           // Length of the buffer
                                       &AgreedSecretLengthB,        // Number of bytes copied to the buffer
                                       0);                          // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    AgreedSecretB = (PBYTE)HeapAlloc(
                                        GetProcessHeap (), 
                                        0, 
                                        AgreedSecretLengthB);
    if( NULL == AgreedSecretB  )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptDeriveKey(
                                       AgreedSecretHandleB,         // Secret agreement handle   
                                       BCRYPT_KDF_TLS_PRF,          // Key derivation function (null terminated unicode string)
                                       &ParameterList,              // KDF parameters
                                       AgreedSecretB,               // Buffer that recieves the derived key 
                                       AgreedSecretLengthB,         // Length of the buffer
                                       &AgreedSecretLengthB,        // Number of bytes copied to the buffer
                                       0);                          // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
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
        Status = STATUS_UNSUCCESSFUL;
        ReportError(Status);
        goto cleanup;

    }

    Status = STATUS_SUCCESS;

	wprintf(L"Success!\n");

cleanup:

    if( PubKeyHandleA )    
    {
        BCryptDestroyKey(PubKeyHandleA);
    }

    if( PubKeyHandleB )    
    {
        BCryptDestroyKey(PubKeyHandleB);
    }

    if( PrivKeyHandleA )    
    {
        BCryptDestroyKey(PrivKeyHandleA);
    }

    if( PrivKeyHandleB )    
    {
        BCryptDestroyKey(PrivKeyHandleB);
    }

    if( ExchAlgHandleA )
    {
        BCryptCloseAlgorithmProvider(ExchAlgHandleA,0);
    }

    if( ExchAlgHandleB )
    {
        BCryptCloseAlgorithmProvider(ExchAlgHandleB,0);
    }

    if( AgreedSecretHandleA )
    {
        BCryptDestroySecret(AgreedSecretHandleA);
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

    if( DhParamBlob )
    {
        HeapFree(GetProcessHeap(), 0, DhParamBlob);
    }

    return Status;
}

