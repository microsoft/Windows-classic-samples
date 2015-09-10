// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             SignHashWithPersistedKeys.cpp
//
//  Contents:         Sample program for ECDSA 256 signing using CNG. 
//                    A persisted key is used for signing and an ephemeral key is used for verification
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
BYTE Message[] =
{
    0x04, 0x87, 0xec, 0x66, 0xa8, 0xbf, 0x17, 0xa6,
    0xe3, 0x62, 0x6f, 0x1a, 0x55, 0xe2, 0xaf, 0x5e,
    0xbc, 0x54, 0xa4, 0xdc, 0x68, 0x19, 0x3e, 0x94,
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

//----------------------------------------------------------------------------
//
//  ComputeHash
//  Computes the hash of a message using SHA-256
//
//----------------------------------------------------------------------------

NTSTATUS
ComputeHash(
    _In_reads_bytes_(DataLength)
                PBYTE           Data,
    _In_        DWORD           DataLength,
    _Outptr_result_bytebuffer_maybenull_(*DataDigestLengthPointer)
                PBYTE           *DataDigestPointer,
    _Out_       DWORD           *DataDigestLengthPointer
    )
{
    NTSTATUS                Status;
    
    BCRYPT_ALG_HANDLE       HashAlgHandle   = NULL;
    BCRYPT_HASH_HANDLE      HashHandle      = NULL;
    
    PBYTE                   HashDigest       = NULL;
    DWORD                   HashDigestLength = 0;

    DWORD                   ResultLength     = 0;
    
    *DataDigestPointer = NULL;
    *DataDigestLengthPointer = 0;

    //
    // Open a Hash algorithm handle
    //

    Status = BCryptOpenAlgorithmProvider(
                                        &HashAlgHandle,
                                        BCRYPT_SHA1_ALGORITHM,
                                        NULL,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    
    //
    // Calculate the length of the Hash
    //
    
    Status= BCryptGetProperty(
                                        HashAlgHandle, 
                                        BCRYPT_HASH_LENGTH, 
                                        (PBYTE)&HashDigestLength, 
                                        sizeof(HashDigestLength), 
                                        &ResultLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //allocate the Hash buffer on the heap
    HashDigest = (PBYTE)HeapAlloc (GetProcessHeap (), 0, HashDigestLength);
    if( NULL == HashDigest )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }
    
    //
    // Create a Hash
    //

    Status = BCryptCreateHash(
                                        HashAlgHandle, 
                                        &HashHandle, 
                                        NULL, 
                                        0, 
                                        NULL, 
                                        0, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    //
    // Hash Data(s)
    //
    Status = BCryptHashData(
                                        HashHandle,
                                        (PBYTE)Data,
                                        DataLength,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    //
    // Close the Hash
    //
    
    Status = BCryptFinishHash(
                                        HashHandle, 
                                        HashDigest, 
                                        HashDigestLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    *DataDigestPointer = HashDigest;
    HashDigest = NULL;
    *DataDigestLengthPointer = HashDigestLength; 

    Status = STATUS_SUCCESS;

cleanup:

    if( NULL != HashDigest )
    {
        HeapFree( GetProcessHeap(), 0, HashDigest );
        HashDigest = NULL;
    }

    if( NULL != HashHandle )
    {
        Status = BCryptDestroyHash(HashHandle);
        HashHandle = NULL;
    }

    if( NULL != HashAlgHandle )
    {
        BCryptCloseAlgorithmProvider(HashAlgHandle,0);
    }

    return Status;
}

//----------------------------------------------------------------------------
//
//  SignHash
//  Signs the hash of a message
//  using NCryptSignHash(..) , ECDSA_P256
//
//----------------------------------------------------------------------------
SECURITY_STATUS
SignHash(
    _In_reads_bytes_(MessageLength)
                PBYTE           MessageToSign,
    _In_        DWORD           MessageLength,
    _Outptr_result_bytebuffer_maybenull_(*SignatureBlobLengthPointer) 
                PBYTE               *SignatureBlobPointer,
    _Out_       DWORD               *SignatureBlobLengthPointer,
    _Outptr_result_bytebuffer_maybenull_(*KeyBlobLengthPointer) 
                PBYTE               *KeyBlobPointer,
    _Out_       DWORD               *KeyBlobLengthPointer
    )
{
    NTSTATUS                Status;
    SECURITY_STATUS         secStatus = S_OK;
    NCRYPT_PROV_HANDLE      ProviderHandle = 0;
    NCRYPT_KEY_HANDLE       KeyHandle = 0;
    
    PBYTE                   MessageDigest   = NULL;
    DWORD                   MessageDigestLength = 0;
    PBYTE                   KeyBlob         = NULL;
    DWORD                   KeyBlobLength   = 0;
    PBYTE                   SignatureBlob   = NULL;
    DWORD                   SignatureBlobLength = 0;
    DWORD                   ResultLength    = 0;
    
    *SignatureBlobPointer = NULL;
    *SignatureBlobLengthPointer = 0;
    *KeyBlobPointer = NULL;
    *KeyBlobLengthPointer = 0;

    //
    // Compute hash of the message
    //

    Status = ComputeHash(
                                        MessageToSign,
                                        MessageLength,
                                        &MessageDigest,
                                        &MessageDigestLength);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    
    if( MessageDigest == NULL )
    {
        Status = NTE_INTERNAL_ERROR;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Open handle to KSP
    //

    secStatus = NCryptOpenStorageProvider(
                                        &ProviderHandle, 
                                        MS_KEY_STORAGE_PROVIDER, 
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Create a persisted key
    //

    secStatus = NCryptCreatePersistedKey(
                                        ProviderHandle,
                                        &KeyHandle,
                                        NCRYPT_ECDSA_P256_ALGORITHM,
                                        L"Sample ECC key",
                                        0,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Finalize the key - create it on the disk
    //
    
    secStatus = NCryptFinalizeKey(
                                        KeyHandle,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }   

    //
    // Sign the Hash
    //

    secStatus = NCryptSignHash(
                                        KeyHandle,                  // Key handle used to sign the hash
                                        NULL,                       // Padding information
                                        MessageDigest,              // Hash of the message
                                        MessageDigestLength,        // Length of the hash
                                        NULL,                       // Signed hash buffer
                                        0,                          // Length of the signature(signed hash value)
                                        &SignatureBlobLength,       // Number of bytes copied to the signature buffer
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }
                
    //allocate the signature buffer
    SignatureBlob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, SignatureBlobLength);
    if( NULL == SignatureBlob )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }


    secStatus = NCryptSignHash(
                                        KeyHandle,                  // Key handle used to sign the hash      
                                        NULL,                       // Padding information
                                        MessageDigest,              // Hash of the message
                                        MessageDigestLength,        // Length of the hash
                                        SignatureBlob,              // Signed hash buffer
                                        SignatureBlobLength,        // Length of the signature(signed hash value)
                                        &ResultLength,              // Number of bytes copied to the signature buffer
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    } 
    
    *SignatureBlobPointer = SignatureBlob;
    SignatureBlob = NULL;
    *SignatureBlobLengthPointer = SignatureBlobLength;

    //
    // Export the public key
    //

    secStatus = NCryptExportKey(
                                        KeyHandle,                  // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        NULL,                       // Parameter list
                                        NULL,                       // Buffer that recieves the key blob
                                        0,                          // Buffer length (in bytes)
                                        &KeyBlobLength,             // Number of bytes copied to the buffer 
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    KeyBlob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, KeyBlobLength);
    if( NULL == KeyBlob )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = NCryptExportKey(
                                        KeyHandle,                  // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (null terminated unicode string)
                                        NULL,                       // Parameter list
                                        KeyBlob,                    // Buffer that recieves the key blob
                                        KeyBlobLength,              // Buffer length (in bytes)
                                        &ResultLength,              // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    *KeyBlobPointer = KeyBlob;
    KeyBlob = NULL;
    *KeyBlobLengthPointer = KeyBlobLength;
    
    secStatus = S_OK;

cleanup:

    if( NULL != KeyBlob )
    {
        HeapFree( GetProcessHeap(), 0, KeyBlob );
        KeyBlob = NULL;
    }

    if( NULL != SignatureBlob )
    {
        HeapFree( GetProcessHeap(), 0, SignatureBlob );
        SignatureBlob = NULL;
    }

    if( NULL != MessageDigest )
    {
        HeapFree( GetProcessHeap(), 0, MessageDigest );
        MessageDigest = NULL;
    }

    if( 0 != KeyHandle )
    {
        NCryptDeleteKey(KeyHandle, 0);
        KeyHandle = 0;
    }

    if( 0 != ProviderHandle )
    {
        NCryptFreeObject(ProviderHandle);
    }

    return secStatus;
}

//-----------------------------------------------------------------------------
//
//  VerifySignature
//  Verifies the signature given the signature blob, key blob, and hash of the message
//  using BCryptVerifySignature(..) , ECDSA_P256
//      
//-----------------------------------------------------------------------------
SECURITY_STATUS
VerifySignature(
    _In_reads_bytes_(MessageLength)
               PBYTE           MessageToVerify,
    _In_       DWORD           MessageLength,
    _In_reads_bytes_(SignatureBlobLength) 
               PBYTE           SignatureBlob,
    _In_       DWORD           SignatureBlobLength,
    _In_reads_bytes_(KeyBlobLength) 
               PBYTE           KeyBlob,
    _In_       DWORD           KeyBlobLength
    )
{
    NTSTATUS                Status;
    SECURITY_STATUS         secStatus = S_OK;
    BCRYPT_KEY_HANDLE       KeyHandle       = NULL;
    BCRYPT_ALG_HANDLE       DsaAlgHandle    = NULL;
    
    PBYTE                   MessageDigest   = NULL;
    DWORD                   MessageDigestLength = 0;
    
    //
    // Compute hash of the message
    //

    Status = ComputeHash(
                                        MessageToVerify,
                                        MessageLength,
                                        &MessageDigest,
                                        &MessageDigestLength);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    if( MessageDigest == NULL )
    {
        Status = NTE_INTERNAL_ERROR;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Open a DSA algorithm handle
    //

    Status = BCryptOpenAlgorithmProvider(
                                        &DsaAlgHandle,
                                        BCRYPT_ECDSA_P256_ALGORITHM,
                                        NULL,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    //
    // Import the public key
    //
    
    Status = BCryptImportKeyPair(
                                        DsaAlgHandle,               // Alg handle
                                        NULL,                       // Parameter not used
                                        BCRYPT_ECCPUBLIC_BLOB,      // Blob type (Null terminated unicode string)
                                        &KeyHandle,                 // Key handle that will be recieved
                                        KeyBlob,                    // Buffer than points to the key blob
                                        KeyBlobLength,              // Buffer length in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    //
    // Verify the signature
    //
    
    Status = BCryptVerifySignature(
                                        KeyHandle,                  // Handle of the key used to decrypt the signature
                                        NULL,                       // Padding information
                                        MessageDigest,              // Hash of the message
                                        MessageDigestLength,        // Hash's length
                                        SignatureBlob,              // Signature - signed hash data
                                        SignatureBlobLength,        // Signature's length
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        secStatus = HRESULT_FROM_NT(Status);
        goto cleanup;
    }

    secStatus = S_OK;

cleanup:

    if( NULL != MessageDigest )
    {
        HeapFree( GetProcessHeap(), 0, MessageDigest );
        MessageDigest = NULL;
    }

    if( NULL != KeyHandle )
    {
        BCryptDestroyKey(KeyHandle);
        KeyHandle = NULL;
    }

    if( NULL != DsaAlgHandle )
    {
        BCryptCloseAlgorithmProvider(DsaAlgHandle,0);
    }

    return secStatus;

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
    SECURITY_STATUS         secStatus;
    
    DWORD                   KeyBlobLength   = 0;
    DWORD                   SignatureBlobLength = 0;

    PBYTE                   KeyBlob          = NULL;
    PBYTE                   SignatureBlob    = NULL;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

     
    //
    // Sign the message
    //

    secStatus = SignHash(
                                        (PBYTE)Message,
                                        sizeof(Message),
                                        &SignatureBlob,
                                        &SignatureBlobLength,
                                        &KeyBlob,
                                        &KeyBlobLength);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    if( NULL == SignatureBlob ||
        NULL == KeyBlob)
    {
        secStatus = NTE_INTERNAL_ERROR;
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Verify the signature
    //

    secStatus = VerifySignature(
                                        (PBYTE)Message,
                                        sizeof(Message),
                                        SignatureBlob,
                                        SignatureBlobLength,
                                        KeyBlob,
                                        KeyBlobLength);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = S_OK;

    wprintf(L"Success!\n");

cleanup:

    if( NULL != KeyBlob )
    {
        HeapFree( GetProcessHeap(), 0, KeyBlob );
        KeyBlob = NULL;
    }

    if( NULL != SignatureBlob )
    {
        HeapFree( GetProcessHeap(), 0, SignatureBlob );
        SignatureBlob = NULL;
    }

    return (DWORD)secStatus;
}

