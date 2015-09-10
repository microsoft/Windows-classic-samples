// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             SignHashAndVerifySignature.cpp
//
//  Contents:         Sample program for DSA 1024 that shows how to sign a Hash value and verify the signature using CNG
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
    // Hash message(s)
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
//  using BCryptSignHash(..) , DSA-1024
//
//----------------------------------------------------------------------------
NTSTATUS
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
    BCRYPT_KEY_HANDLE       KeyHandle       = NULL;
    BCRYPT_ALG_HANDLE       DsaAlgHandle    = NULL;
    
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
        goto cleanup;
    }

    if( MessageDigest == NULL )
    {
        Status = STATUS_UNSUCCESSFUL;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Open a DSA algorithm handle
    //

    Status = BCryptOpenAlgorithmProvider(
                                        &DsaAlgHandle,
                                        BCRYPT_DSA_ALGORITHM,
                                        NULL,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Generate a 1024 bit DSA key
    //
    
    Status = BCryptGenerateKeyPair(
                                        DsaAlgHandle,
                                        &KeyHandle,
                                        1024,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }   

    //
    // Finalize the key
    //

    Status = BCryptFinalizeKeyPair(KeyHandle, 0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Sign the Hash
    //

    Status = BCryptSignHash(
                                        KeyHandle,                  // Key handle used to sign the hash
                                        NULL,                       // Padding information
                                        MessageDigest,              // Hash of the message
                                        MessageDigestLength,        // Length of the hash
                                        NULL,                       // Signed hash buffer
                                        0,                          // Length of the signature(signed hash value)
                                        &SignatureBlobLength,       // Number of bytes copied to the signature buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
                
    //allocate the signature buffer
    SignatureBlob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, SignatureBlobLength);
    if( NULL == SignatureBlob )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptSignHash(
                                        KeyHandle,                  // Key handle used to sign the hash      
                                        NULL,                       // Padding information
                                        MessageDigest,              // Hash of the message
                                        MessageDigestLength,        // Length of the hash
                                        SignatureBlob,              // Signed hash buffer
                                        SignatureBlobLength,        // Length of the signature(signed hash value)
                                        &ResultLength,              // Number of bytes copied to the signature buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    } 
    
    *SignatureBlobPointer = SignatureBlob;
    SignatureBlob = NULL;
    *SignatureBlobLengthPointer = SignatureBlobLength;

    //
    // Export the public key
    //

    Status = BCryptExportKey(
                                        KeyHandle,                  // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_DSA_PUBLIC_BLOB,     // Blob type (null terminated unicode string)
                                        NULL,                       // Buffer that recieves the key blob
                                        0,                          // Buffer length (in bytes)
                                        &KeyBlobLength,             // Number of bytes copied to the buffer 
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    KeyBlob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, KeyBlobLength);
    if( NULL == KeyBlob )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptExportKey(
                                        KeyHandle,                  // Handle of the key to export
                                        NULL,                       // Handle of the key used to wrap the exported key
                                        BCRYPT_DSA_PUBLIC_BLOB,     // Blob type (null terminated unicode string)
                                        KeyBlob,                    // Buffer that recieves the key blob
                                        KeyBlobLength,              // Buffer length (in bytes)
                                        &ResultLength,              // Number of bytes copied to the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    *KeyBlobPointer = KeyBlob;
    KeyBlob = NULL;
    *KeyBlobLengthPointer = KeyBlobLength;
    
    Status = STATUS_SUCCESS;

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

    if( NULL != KeyHandle )
    {
        BCryptDestroyKey(KeyHandle);
        KeyHandle = NULL;
    }

    if( NULL != DsaAlgHandle )
    {
        BCryptCloseAlgorithmProvider(DsaAlgHandle,0);
    }

    return Status;
}

//-----------------------------------------------------------------------------
//
//  VerifySignature
//  Verifies the signature given the signature blob, key blob, and hash of the message
//  using BCryptVerifySignature(..) , DSA-1024
//      
//-----------------------------------------------------------------------------
NTSTATUS
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
        goto cleanup;
    }

    if( MessageDigest == NULL )
    {
        Status = STATUS_UNSUCCESSFUL;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Open a DSA algorithm handle
    //

    Status = BCryptOpenAlgorithmProvider(
                                        &DsaAlgHandle,
                                        BCRYPT_DSA_ALGORITHM,
                                        NULL,
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Import the public key
    //
    
    Status = BCryptImportKeyPair(
                                        DsaAlgHandle,               // Alg handle
                                        NULL,                       // Parameter not used
                                        BCRYPT_DSA_PUBLIC_BLOB,     // Blob type (Null terminated unicode string)
                                        &KeyHandle,                 // Key handle that will be recieved
                                        KeyBlob,                    // Buffer than points to the key blob
                                        KeyBlobLength,              // Buffer length in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
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
        goto cleanup;
    }

    Status = STATUS_SUCCESS;

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

    return Status;

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
    NTSTATUS                Status;

    DWORD                   KeyBlobLength   = 0;
    DWORD                   SignatureBlobLength = 0;

    PBYTE                   KeyBlob          = NULL;
    PBYTE                   SignatureBlob    = NULL;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

     
    //
    // Sign the message
    //

    Status = SignHash(
                                        (PBYTE)Message,
                                        sizeof(Message),
                                        &SignatureBlob,
                                        &SignatureBlobLength,
                                        &KeyBlob,
                                        &KeyBlobLength);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    if( NULL == SignatureBlob ||
        NULL == KeyBlob)
    {
        Status = STATUS_UNSUCCESSFUL;
        ReportError(Status);
        goto cleanup;
    }


    //
    // Verify the signature
    //

    Status = VerifySignature(
                                        (PBYTE)Message,
                                        sizeof(Message),
                                        SignatureBlob,
                                        SignatureBlobLength,
                                        KeyBlob,
                                        KeyBlobLength);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = STATUS_SUCCESS;

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

    return (DWORD)Status;
}

