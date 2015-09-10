// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             RSACapiAndCngInterop.cpp
//
//  Contents:         Sample program for RSA PKCS#1 v1.5 signing and encryption using CNG and CAPI
//

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>
#include <stdio.h>
#include <wincrypt.h>
#include <bcrypt.h>
#include <ncrypt.h>
#include <sal.h>
#include <assert.h>

static
const
BYTE Data[] =
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
    wprintf( L"Error: 0x%08x (%u)\n", dwErrCode, dwErrCode );
}

//----------------------------------------------------------------------------
//
//  ReverseBytes
//  Reverses bytes for big-little endian conversion
//
//----------------------------------------------------------------------------
BOOL 
ReverseBytes (
    _Inout_updates_bytes_(ByteBufferLength)
                PBYTE   ByteBuffer,
    _In_        DWORD   ByteBufferLength)
{
    DWORD count = 0;
    BYTE  TmpByteBuffer = 0;

    assert( (ByteBufferLength % 2) == 0);

    for( count=0; count < ByteBufferLength/2; count++)
    {
      TmpByteBuffer = *(ByteBuffer + count);
      *(ByteBuffer + count) = *(ByteBuffer + ByteBufferLength - count -1);
      *(ByteBuffer + ByteBufferLength - count -1) = TmpByteBuffer;
    }

    return TRUE;

}

//----------------------------------------------------------------------------
//
//  SignWithCapiVerifyWithCng
//  Computes the hash of a message using SHA-256
//
//----------------------------------------------------------------------------
void
SignWithCapiVerifyWithCng(void)
{
    NTSTATUS                    Status;
    HCRYPTHASH                  CapiHashHandle = 0;
    HCRYPTKEY                   CapiKeyHandle = 0;
    HCRYPTPROV                  CapiProviderHandle = 0;
    PBYTE                       Signature = NULL;
    DWORD                       SignatureLength = 0;
    PBYTE                       Blob = NULL;
    DWORD                       BlobLength = 0;
    BCRYPT_ALG_HANDLE           CngAlgHandle = NULL;
    NCRYPT_KEY_HANDLE           CngTmpKeyHandle = NULL;
    BCRYPT_HASH_HANDLE          CngHashHandle = NULL;
    SECURITY_STATUS             secStatus = S_OK;
    DWORD                       ResultLength = 0,
                                HashLength = 0,
                                HashObjectLength = 0;
    PBYTE                       HashObject = NULL;
    PBYTE                       Hash = NULL;
    BCRYPT_PKCS1_PADDING_INFO   PKCS1PaddingInfo = {0};
    NCRYPT_PROV_HANDLE          CngProviderHandle = 0;
    
    //delete, ignore errors
    CryptAcquireContext(
                    &CapiProviderHandle,
                    TEXT("test"),
                    MS_STRONG_PROV,
                    PROV_RSA_FULL,
                    CRYPT_DELETEKEYSET);

    //acquire handle to a csp container
    if(!CryptAcquireContext(
                    &CapiProviderHandle,
                    TEXT("test"),
                    MS_STRONG_PROV,
                    PROV_RSA_FULL,
                    CRYPT_NEWKEYSET))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    //generate a key
    if(!CryptGenKey(
                CapiProviderHandle,
                AT_SIGNATURE,
                0,
                &CapiKeyHandle))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    //create and sign hash with CAPI API
    if(!CryptCreateHash(
                    CapiProviderHandle,
                    CALG_SHA1,
                    0,
                    0,
                    &CapiHashHandle))
    {
       secStatus = HRESULT_FROM_WIN32(GetLastError());
       ReportError(secStatus);
       goto cleanup;
    }

    if(!CryptHashData(
                    CapiHashHandle,
                    (PBYTE)Data,
                    sizeof(Data),
                    0))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if(!CryptSignHash(
                    CapiHashHandle,
                    AT_SIGNATURE, 
                    NULL,
                    0,
                    NULL,
                    &SignatureLength))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }
    
    Signature = (PBYTE)HeapAlloc (GetProcessHeap (), 0, SignatureLength);
    if( NULL == Signature )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    if(!CryptSignHash(
                    CapiHashHandle,
                    AT_SIGNATURE,
                    NULL,
                    0,
                    Signature,
                    &SignatureLength))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if(!CryptExportKey(
                    CapiKeyHandle,
                    0,
                    PUBLICKEYBLOB,
                    0,
                    NULL,
                    &BlobLength))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }
     
    Blob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, BlobLength);
    if( NULL == Blob )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    if(!CryptExportKey(
                    CapiKeyHandle,
                    0,
                    PUBLICKEYBLOB,
                    0,
                    Blob,
                    &BlobLength))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    //verify with cng
    Status = BCryptOpenAlgorithmProvider(
                                        &CngAlgHandle, 
                                        BCRYPT_SHA1_ALGORITHM, 
                                        NULL, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptGetProperty( 
                                        CngAlgHandle, 
                                        BCRYPT_OBJECT_LENGTH,
                                        (PBYTE)&HashObjectLength,
                                        sizeof(DWORD),
                                        &ResultLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    HashObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, HashObjectLength); 
    if( NULL == HashObject )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    Status = BCryptGetProperty( 
                                        CngAlgHandle, 
                                        BCRYPT_HASH_LENGTH,
                                        (PBYTE)&HashLength,
                                        sizeof(DWORD),
                                        &ResultLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }


    Hash = (PBYTE) HeapAlloc (GetProcessHeap (), 0, HashLength);
    if( NULL == Hash )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

   
    Status = BCryptCreateHash(
                                        CngAlgHandle, 
                                        &CngHashHandle, 
                                        HashObject, 
                                        HashObjectLength, 
                                        NULL, 
                                        0, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    Status = BCryptHashData(
                                        CngHashHandle,
                                        (PBYTE)Data,
                                        sizeof(Data),
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
       
    //close the hash
    Status = BCryptFinishHash(
                                        CngHashHandle, 
                                        Hash, 
                                        HashLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //reverse since CNG is big endian and CAPI is little endian
    ReverseBytes(Signature, SignatureLength);

    secStatus = NCryptOpenStorageProvider(
                                        &CngProviderHandle, 
                                        MS_KEY_STORAGE_PROVIDER, 
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = NCryptImportKey(
                                        CngProviderHandle,
                                        NULL,
                                        LEGACY_RSAPUBLIC_BLOB,
                                        NULL,
                                        &CngTmpKeyHandle,
                                        Blob,
                                        BlobLength,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //specify PKCS padding
    PKCS1PaddingInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;

    secStatus = NCryptVerifySignature(
                                        CngTmpKeyHandle,
                                        &PKCS1PaddingInfo,
                                        Hash,
                                        HashLength,
                                        Signature,
                                        SignatureLength,
                                        NCRYPT_PAD_PKCS1_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    wprintf(L"Success!\n");

cleanup:

    if(CapiKeyHandle)
    {
        CryptDestroyKey(CapiKeyHandle);
    }

    if(CapiHashHandle)   
    {
        CryptDestroyHash(CapiHashHandle);
    }

    if(CapiProviderHandle)   
    {
        CryptReleaseContext(CapiProviderHandle, 0);
    }

    if (CngHashHandle)   
    {
        BCryptDestroyHash(CngHashHandle);   
    }

    if(CngAlgHandle)  
    {
        BCryptCloseAlgorithmProvider(CngAlgHandle,0);
    }

    if(CngTmpKeyHandle)     
    {
        NCryptFreeObject(CngTmpKeyHandle);
    }

    if(CngProviderHandle)   
    {
        NCryptFreeObject(CngProviderHandle);
    }

    //attempt to delete container
    CryptAcquireContext(
                    &CapiProviderHandle,
                    TEXT("test"),
                    MS_STRONG_PROV,
                    PROV_RSA_FULL,
                    CRYPT_DELETEKEYSET);

    if(HashObject)
    {
        HeapFree(GetProcessHeap(), 0, HashObject);
    }

    if(Hash)
    {
        HeapFree(GetProcessHeap(), 0, Hash);
    }

    if(Signature)
    {
        HeapFree(GetProcessHeap(), 0, Signature);
    }

    if(Blob)
    {
        HeapFree(GetProcessHeap(), 0, Blob);
    }

}

//----------------------------------------------------------------------------
//
//  SignWithCngVerifyWithCapi
//  Signs the hash of a message
//  using BCryptSignHash(..) , DSA-1024
//
//----------------------------------------------------------------------------
void
SignWithCngVerifyWithCapi(void)
{
    NCRYPT_PROV_HANDLE          CngProviderHandle = 0;
    NCRYPT_KEY_HANDLE           CapiKeyHandle = 0;
    BCRYPT_ALG_HANDLE           CngAlgHandle = NULL;
    BCRYPT_HASH_HANDLE          CapiHashHandle = NULL;
    NTSTATUS                    Status;
    SECURITY_STATUS             secStatus = S_OK;
    DWORD                       HashLength = 0,
                                ResultLength = 0,
                                HashObjectLength = 0;
    PBYTE                       HashObject = NULL;
    PBYTE                       Hash = NULL,
                                Blob = NULL;
    PBYTE                       Signature = NULL;
    DWORD                       SignatureLength = 0,
                                BlobLength = 0;
    BCRYPT_PKCS1_PADDING_INFO   PKCS1PaddingInfo = {0};
    HCRYPTHASH                  HashHandle = 0;
    HCRYPTKEY                   CngTmpKeyHandle = 0;
    HCRYPTPROV                  CapiLocProvHandle = 0;


    secStatus = NCryptOpenStorageProvider(
                                        &CngProviderHandle,
                                        MS_KEY_STORAGE_PROVIDER,
                                        0);

    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = NCryptCreatePersistedKey(
                                        CngProviderHandle,
                                        &CapiKeyHandle,
                                        NCRYPT_RSA_ALGORITHM,
                                        L"test",
                                        0,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }


    secStatus = NCryptFinalizeKey(CapiKeyHandle, 0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //open alg provider handle
    Status = BCryptOpenAlgorithmProvider(
                                        &CngAlgHandle, 
                                        NCRYPT_SHA1_ALGORITHM, 
                                        NULL, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    Status = BCryptGetProperty( 
                                        CngAlgHandle, 
                                        BCRYPT_OBJECT_LENGTH,
                                        (PBYTE)&HashObjectLength,
                                        sizeof(DWORD),
                                        &ResultLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    HashObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0,HashObjectLength); 
    if( NULL == HashObject )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    //required size of hash?
    Status = BCryptGetProperty( 
                                        CngAlgHandle, 
                                        BCRYPT_HASH_LENGTH,
                                        (PBYTE)&HashLength,
                                        sizeof(DWORD),
                                        &ResultLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Hash = (PBYTE)HeapAlloc (GetProcessHeap (), 0, HashLength); 
    if( NULL == Hash )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    Status = BCryptCreateHash(
                                        CngAlgHandle, 
                                        &CapiHashHandle, 
                                        HashObject, 
                                        HashObjectLength, 
                                        NULL, 
                                        0, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = BCryptHashData(
                                        CapiHashHandle,
                                        (PBYTE)Data,
                                        sizeof(Data),
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
  
    Status = BCryptFinishHash(
                                        CapiHashHandle, 
                                        Hash, 
                                        HashLength, 
                                        0);
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    PKCS1PaddingInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;

    secStatus = NCryptSignHash(
                                        CapiKeyHandle,
                                        &PKCS1PaddingInfo,
                                        Hash,
                                        HashLength,
                                        NULL,
                                        0,
                                        &SignatureLength,
                                        NCRYPT_PAD_PKCS1_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    Signature = (PBYTE)HeapAlloc (GetProcessHeap (), 0, SignatureLength); 
    if( NULL == Signature )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    secStatus = NCryptSignHash(
                                        CapiKeyHandle,
                                        &PKCS1PaddingInfo,
                                        Hash,
                                        HashLength,
                                        Signature,
                                        SignatureLength,
                                        &SignatureLength,
                                        NCRYPT_PAD_PKCS1_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }
 
    secStatus = NCryptExportKey(
                                        CapiKeyHandle,
                                        NULL,
                                        LEGACY_RSAPUBLIC_BLOB,
                                        NULL,
                                        NULL,
                                        0,
                                        &BlobLength,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }
 
    Blob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, BlobLength); 
    if( NULL == Blob )
    {
       secStatus = NTE_NO_MEMORY;
       ReportError(secStatus);
       goto cleanup;

    }
  
    secStatus = NCryptExportKey(
                                        CapiKeyHandle,
                                        NULL,
                                        LEGACY_RSAPUBLIC_BLOB,
                                        NULL,
                                        Blob,
                                        BlobLength,
                                        &BlobLength,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    ReverseBytes(Signature, SignatureLength);

    //temporarily import the key into a verify context container and decrypt
    if(!CryptAcquireContext(
                            &CapiLocProvHandle,
                            NULL,
                            MS_ENH_RSA_AES_PROV,
                            PROV_RSA_AES,
                            CRYPT_VERIFYCONTEXT))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if(!CryptImportKey(
                    CapiLocProvHandle,
                    Blob,
                    BlobLength,
                    0,
                    0,
                    &CngTmpKeyHandle))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if(!CryptCreateHash(
                    CapiLocProvHandle,
                    CALG_SHA1,
                    0,
                    0,
                    &HashHandle))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

   
    if(!CryptHashData(
                    HashHandle,
                    (PBYTE)Data,
                    sizeof(Data),
                    0))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if(!CryptVerifySignature(
                    HashHandle, 
                    Signature, 
                    SignatureLength, 
                    CngTmpKeyHandle,
                    NULL, 
                    0))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    wprintf(L"Success!\n");

cleanup:

    if(CapiHashHandle)       
    {
        BCryptDestroyHash(CapiHashHandle);
    }

    if(CngAlgHandle)  
    {
        BCryptCloseAlgorithmProvider(CngAlgHandle,0);
    }

    if(HashHandle)   
    {
        CryptDestroyHash(HashHandle);
    }

    if(CngTmpKeyHandle)     
    {
        CryptDestroyKey(CngTmpKeyHandle);
    }

    if(CapiLocProvHandle)    
    {
        CryptReleaseContext(CapiLocProvHandle, 0);
    }

    if(CapiKeyHandle)        
    {
        NCryptDeleteKey(CapiKeyHandle, 0);
    }

    if(CngProviderHandle)   
    {
        NCryptFreeObject(CngProviderHandle);
    }

    if(HashObject)    
    {
        HeapFree(GetProcessHeap(), 0, HashObject);
    }

    if(Hash)          
    {
        HeapFree(GetProcessHeap(), 0, Hash);
    }

    if(Signature)     
    {
        HeapFree(GetProcessHeap(), 0, Signature);
    }

    if(Blob)          
    {
        HeapFree(GetProcessHeap(), 0, Blob);
    }

}

//-----------------------------------------------------------------------------------------
//
//  EncryptWithCapiDecryptWithCng
//  Verifies the signature given the signature Blob, key Blob, and hash of the message
//  using BCryptVerifySignature(..) , DSA-1024
//      
//----------------------------------------------------------------------------------------
void
EncryptWithCapiDecryptWithCng(void)
{
    HCRYPTKEY                   CapiKeyHandle = 0;
    HCRYPTPROV                  CapiProviderHandle = 0;
    PBYTE                       Blob = NULL,
                                EncryptDecryptData = NULL;
    DWORD                       BlobLength = 0;
    NCRYPT_KEY_HANDLE           CngTmpKeyHandle = NULL;
    SECURITY_STATUS             secStatus = S_OK;
    DWORD                       EncryptDecryptDataLength = 0,
                                MsgLength = 0,
                                ResultLength = 0;
    NCRYPT_PROV_HANDLE          CngProviderHandle = 0;
    NTSTATUS                    Status;

    if(!CryptAcquireContext(
                    &CapiProviderHandle,
                    TEXT("test"),
                    MS_STRONG_PROV,
                    PROV_RSA_FULL,
                    CRYPT_NEWKEYSET))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if(!CryptGenKey(
                    CapiProviderHandle,
                    AT_KEYEXCHANGE,
                    CRYPT_EXPORTABLE,
                    &CapiKeyHandle))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    MsgLength = sizeof(Data);

    if(!CryptEncrypt(
                        CapiKeyHandle, 
                        0, 
                        TRUE, 
                        0, 
                        NULL, 
                        &MsgLength,
                        sizeof(Data)))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    EncryptDecryptDataLength = MsgLength;
    EncryptDecryptData = (PBYTE)HeapAlloc (GetProcessHeap (), 0, EncryptDecryptDataLength);
    if( NULL == EncryptDecryptData )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    // Check to see if the allocated buffer is long enough
    if( EncryptDecryptDataLength < sizeof(Data) )
    {
        secStatus = NTE_FAIL;
        ReportError(secStatus);
        goto cleanup;
    } 

    //copy input data to buffer
    memcpy(EncryptDecryptData, (PBYTE) Data, sizeof(Data));
    MsgLength = sizeof(Data);

    // Call CAPI1 to encrypt
    if(!CryptEncrypt(
                CapiKeyHandle,
                0,
                TRUE,
                0,
                EncryptDecryptData,
                &MsgLength,//size of data to be encrypted
                EncryptDecryptDataLength))
    {
       secStatus = HRESULT_FROM_WIN32(GetLastError());
       ReportError(secStatus);
       goto cleanup;
    }

    // Export the key from CAPI1: buffer length probe
    if(!CryptExportKey(
                    CapiKeyHandle,
                    0,
                    PRIVATEKEYBLOB,
                    0,
                    NULL,
                    &BlobLength))
    {
       secStatus = HRESULT_FROM_WIN32(GetLastError());
       ReportError(secStatus);
       goto cleanup;
    }

    // Allocate memory to export the key to
    Blob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, BlobLength);
    if( NULL == Blob )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    // Export the key value to the allocated memory buffer
    if(!CryptExportKey(
                    CapiKeyHandle,
                    0,
                    PRIVATEKEYBLOB,
                    0,
                    Blob,
                    &BlobLength))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    // CAPI1 returns the key bytes reversed.
    ReverseBytes(EncryptDecryptData, EncryptDecryptDataLength);


    // Now it is time to import the exported CAPI1 key into CNG KSP ...
    // Open Microsoft KSP
    secStatus = NCryptOpenStorageProvider(
                                            &CngProviderHandle, 
                                            MS_KEY_STORAGE_PROVIDER, 
                                            0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    // ... and create a persistent key in the MS KSP
    secStatus = NCryptCreatePersistedKey(
                                        CngProviderHandle,
                                        &CngTmpKeyHandle,
                                        NCRYPT_RSA_ALGORITHM,
                                        L"cngtmpkey",
                                        0,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    // Set the property of this key: Legacy RSA private key Blob
    secStatus = NCryptSetProperty(
                                        CngTmpKeyHandle,
                                        LEGACY_RSAPRIVATE_BLOB,
                                        Blob,
                                        BlobLength,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }


    // And see that the key object is created.
    secStatus = NCryptFinalizeKey(
                                        CngTmpKeyHandle,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    // Do in place decryption by providing the same pointers
    // to the input and output buffers (Data, ResultLength) couple.
    secStatus = NCryptDecrypt(
                                    CngTmpKeyHandle,
                                    EncryptDecryptData,
                                    EncryptDecryptDataLength,
                                    NULL,
                                    EncryptDecryptData,
                                    EncryptDecryptDataLength,
                                    &ResultLength,
                                    NCRYPT_PAD_PKCS1_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Optional
    //

    if (0 != memcmp(EncryptDecryptData, (PBYTE)Data, sizeof(Data))) 
    {
        secStatus = NTE_FAIL;
        ReportError(secStatus);
        goto cleanup;
    }  

    Status = STATUS_SUCCESS;
    wprintf(L"Success!\n");

cleanup:

    if(CapiKeyHandle)    
    {
        CryptDestroyKey(CapiKeyHandle);
    }

    if(CapiProviderHandle)   
    {
        CryptReleaseContext(CapiProviderHandle, 0);
    }

    if(Blob)   
    {
        HeapFree(GetProcessHeap(), 0, Blob);
    }

    if(EncryptDecryptData)   
    {
        HeapFree(GetProcessHeap(), 0, EncryptDecryptData);
    }

    if(CngTmpKeyHandle)     
    {
        NCryptDeleteKey(CngTmpKeyHandle, 0);
    }

    if(CngProviderHandle)  
    {
        NCryptFreeObject(CngProviderHandle);
    }

    //attempt to delete container
    CryptAcquireContext(
                    &CapiProviderHandle,
                    TEXT("test"),
                    MS_STRONG_PROV,
                    PROV_RSA_FULL,
                    CRYPT_DELETEKEYSET);

}

//-----------------------------------------------------------------------------------------
//
//  EncryptWithCngDecryptWithCapi
//  Verifies the signature given the signature Blob, key Blob, and hash of the message
//  using BCryptVerifySignature(..) , DSA-1024
//      
//----------------------------------------------------------------------------------------
void
EncryptWithCngDecryptWithCapi(void)
{
    NCRYPT_PROV_HANDLE          CngProviderHandle = 0;
    NCRYPT_KEY_HANDLE           CapiKeyHandle = 0;
    SECURITY_STATUS             secStatus = S_OK;
    DWORD                       OutputLength = 0;
    PBYTE                       Blob = NULL,
                                Output = NULL;
    DWORD                       BlobLength = 0,
                                ResultLength = 0,
                                Policy = 0;
    HCRYPTKEY                   CngTmpKeyHandle = 0;
    HCRYPTPROV                  CapiLocProvHandle = 0;

    secStatus = NCryptOpenStorageProvider(
                                        &CngProviderHandle,
                                        MS_KEY_STORAGE_PROVIDER,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = NCryptCreatePersistedKey(
                                        CngProviderHandle,
                                        &CapiKeyHandle,
                                        NCRYPT_RSA_ALGORITHM,
                                        L"test",
                                        0,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    Policy = NCRYPT_ALLOW_PLAINTEXT_EXPORT_FLAG;

    secStatus = NCryptSetProperty(
                                        CapiKeyHandle, 
                                        NCRYPT_EXPORT_POLICY_PROPERTY,
                                        (PBYTE)&Policy,
                                        sizeof(DWORD),
                                        NCRYPT_PERSIST_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }      

    secStatus = NCryptFinalizeKey(CapiKeyHandle, 0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }


    secStatus = NCryptEncrypt(
                                        CapiKeyHandle,
                                        (PBYTE)Data,
                                        sizeof(Data),
                                        NULL,
                                        NULL,
                                        0,
                                        &OutputLength,
                                        NCRYPT_PAD_PKCS1_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    Output = (PBYTE)HeapAlloc (GetProcessHeap (), 0, OutputLength); 
    if( NULL == Output )
   {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }

    secStatus = NCryptEncrypt(
                                        CapiKeyHandle,
                                        (PBYTE)Data,
                                        sizeof(Data),
                                        NULL,
                                        Output,
                                        OutputLength,
                                        &ResultLength,
                                        NCRYPT_PAD_PKCS1_FLAG);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }
 

    secStatus = NCryptExportKey(
                                        CapiKeyHandle,
                                        NULL,
                                        LEGACY_RSAPRIVATE_BLOB,
                                        NULL,
                                        NULL,
                                        0,
                                        &BlobLength,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }
 

    Blob = (PBYTE)HeapAlloc (GetProcessHeap (), 0, BlobLength); 
    if( NULL == Blob )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;

    }
  
    secStatus = NCryptExportKey(
                                        CapiKeyHandle,
                                        NULL,
                                        LEGACY_RSAPRIVATE_BLOB,
                                        NULL,
                                        Blob,
                                        BlobLength,
                                        &BlobLength,
                                        0);
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    ReverseBytes(Output, OutputLength);

    //temporarily import the key into a verify context container and decrypt
    if(!CryptAcquireContext(
                        &CapiLocProvHandle,
                        NULL,
                        MS_STRONG_PROV,
                        PROV_RSA_FULL,
                        CRYPT_VERIFYCONTEXT))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if(!CryptImportKey(
                    CapiLocProvHandle,
                    Blob,
                    BlobLength,
                    0,
                    0,
                    &CngTmpKeyHandle))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }   

    if(!CryptDecrypt(
                CngTmpKeyHandle, 
                0, 
                TRUE, 
                0, 
                Output, 
                &OutputLength))
    {
        secStatus = HRESULT_FROM_WIN32(GetLastError());
        ReportError(secStatus);
        goto cleanup;
    }

    if (0 != memcmp(Output, (PBYTE)Data, sizeof(Data))) 
    {
        secStatus = NTE_FAIL;
        ReportError(secStatus);
        goto cleanup;
    } 

    wprintf(L"Success!\n");

cleanup:

    if(CngTmpKeyHandle)     
    {
        CryptDestroyKey(CngTmpKeyHandle);
    }

    if(CapiLocProvHandle)    
    {
        CryptReleaseContext(CapiLocProvHandle, 0);
    }

    if(CapiKeyHandle)        
    {
        NCryptDeleteKey(CapiKeyHandle, 0);
    }

    if(CngProviderHandle)   
    {
        NCryptFreeObject(CngProviderHandle);
    }

    if(Output)    
    {
        HeapFree(GetProcessHeap(), 0, Output);
    }

    if(Blob)      
    {
        HeapFree(GetProcessHeap(), 0, Blob);
    }

}
//-----------------------------------------------------------------------------
//
//    wmain
//
//-----------------------------------------------------------------------------
int
__cdecl
wmain(
    _In_               int     argc,
    _In_reads_(argc)   LPWSTR  argv[]
    )
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

     
    //
    // EncryptWithCngDecryptWithCapi
    //

    EncryptWithCngDecryptWithCapi();

    //
    // EncryptWithCapiDecryptWithCng
    //

    EncryptWithCapiDecryptWithCng();

    //
    // SignWithCngVerifyWithCapi
    //

    SignWithCngVerifyWithCapi();

    //
    // SignWithCapiVerifyWithCng
    //

    SignWithCapiVerifyWithCng();

    return 0;

}

