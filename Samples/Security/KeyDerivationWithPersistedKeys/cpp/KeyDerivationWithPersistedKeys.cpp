// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             KeyDerivationWithPersistedKeys.cpp
//
//  Contents:         This sample shows how to create a persisted KDF key, and then derive 
//                    ephemeral KDF/symmetric keys from the persisted KDF key using the NCryptKeyDerivation API.
//                    The derived ephemeral AES key is used to encrypt and decrypt a message in CBC mode( non authentication mode )
//    
//

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>
#include <winerror.h>
#include <stdio.h>
#include <ncrypt.h>
#include <bcrypt.h>
#include <sal.h>

static const 
BYTE Secret[20] = 
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
};

static const 
BYTE GenericParameter[20] = 
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 
};


static const 
BYTE Message[20] = 
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
};

//
// Alternatively, these values can be found using NCryptGetProperty(NCRYPT_LENGTHS_PROPERTY)
// on an AES key
//

static const
DWORD  AesKeyLength = 16;  // in bytes

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

NTSTATUS
EncryptData(
    _In_        BCRYPT_KEY_HANDLE   KeyHandle,
    _In_        DWORD               KeyObjectLength,
    _In_reads_bytes_(InitVectorLength)
                PBYTE               InitVector,
    _In_        DWORD               InitVectorLength,
    _In_reads_bytes_(ChainingModeLength)
                PBYTE               ChainingMode,
    _In_        DWORD               ChainingModeLength,
    _In_reads_bytes_(PlainTextLength)
				PBYTE               PlainText,
    _In_        DWORD               PlainTextLength,
    _Outptr_result_bytebuffer_(*CipherTextLengthPointer) 
                PBYTE               *CipherTextPointer,
    _Out_       DWORD               *CipherTextLengthPointer
    )
{
    NTSTATUS    Status;

    BCRYPT_KEY_HANDLE   EncryptKeyHandle = NULL;

    DWORD   ResultLength = 0;
    PBYTE   TempInitVector = NULL;
    DWORD   TempInitVectorLength = 0;
    PBYTE   CipherText = NULL;
    DWORD   CipherTextLength = 0;
    PBYTE   KeyObject = NULL;
    
    //
    // Allocate KeyObject on the heap
    //

    KeyObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, KeyObjectLength);
    if( NULL == KeyObject )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Generate a duplicate AES key 
    //

    Status = BCryptDuplicateKey(
                                        KeyHandle,                  // A pointer to key handle
                                        &EncryptKeyHandle,          // A pointer to BCRYPT_KEY_HANDLE that recieves a handle to the duplicate key 
                                        KeyObject,                  // A pointer to the buffer that recieves the duplicate key object
                                        KeyObjectLength,            // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Set the chaining mode on the key handle
    //
    
    Status = BCryptSetProperty( 
                                        EncryptKeyHandle,           // Handle to a CNG object          
                                        BCRYPT_CHAINING_MODE,       // Property name(null terminated unicode string)
                                        ChainingMode,               // Address of the buffer that contains the new property value 
                                        ChainingModeLength,         // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Copy initialization vector into a temporary initialization vector buffer
    // Because after an encrypt/decrypt operation, the IV buffer is overwritten.
    //

    TempInitVector = (PBYTE)HeapAlloc (GetProcessHeap(), 0, InitVectorLength);
    if( NULL == TempInitVector )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }
    
    TempInitVectorLength = InitVectorLength;
    memcpy(TempInitVector, InitVector, TempInitVectorLength);

    //
    // Get CipherText's length
    // If the program can compute the length of cipher text(based on algorihtm and chaining mode info.), this call can be avoided.
    //
    
    Status = BCryptEncrypt(
                                        EncryptKeyHandle,           // Handle to a key which is used to encrypt 
                                        PlainText,                  // Address of the buffer that contains the plaintext
                                        PlainTextLength,            // Size of the buffer in bytes
                                        NULL,                       // A pointer to padding info, used with asymmetric and authenticated encryption; else set to NULL
                                        TempInitVector,             // Address of the buffer that contains the IV. 
                                        TempInitVectorLength,       // Size of the IV buffer in bytes
                                        NULL,                       // Address of the buffer the recieves the ciphertext
                                        0,                          // Size of the buffer in bytes
                                        &CipherTextLength,          // Variable that recieves number of bytes copied to ciphertext buffer 
                                        BCRYPT_BLOCK_PADDING);      // Flags; Block padding allows to pad data to the next block size
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Allocate Cipher Text on the heap
    //

    CipherText = (PBYTE)HeapAlloc (GetProcessHeap (), 0, CipherTextLength);
    if( NULL == CipherText )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Peform encyption
    // For block length messages, block padding will add an extra block
    //
    
    Status = BCryptEncrypt(
                                        EncryptKeyHandle,           // Handle to a key which is used to encrypt 
                                        PlainText,                  // Address of the buffer that contains the plaintext
                                        PlainTextLength,            // Size of the buffer in bytes
                                        NULL,                       // A pointer to padding info, used with asymmetric and authenticated encryption; else set to NULL
                                        TempInitVector,             // Address of the buffer that contains the IV. 
                                        TempInitVectorLength,       // Size of the IV buffer in bytes
                                        CipherText,                 // Address of the buffer the recieves the ciphertext
                                        CipherTextLength,           // Size of the buffer in bytes
                                        &ResultLength,              // Variable that recieves number of bytes copied to ciphertext buffer 
                                        BCRYPT_BLOCK_PADDING);      // Flags; Block padding allows to pad data to the next block size

    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    *CipherTextPointer = CipherText;
    CipherText = NULL;
    *CipherTextLengthPointer = CipherTextLength;

cleanup:

    if( NULL != CipherText)
    {
        SecureZeroMemory(CipherText, CipherTextLength);
        HeapFree(GetProcessHeap(), 0, CipherText);
    }

    if( NULL != KeyObject)
    {
        SecureZeroMemory(KeyObject, KeyObjectLength);
        HeapFree(GetProcessHeap(), 0, KeyObject);
    }

    if( NULL != TempInitVector )
    {
        HeapFree(GetProcessHeap(), 0, TempInitVector);
    }

    if( NULL != EncryptKeyHandle )
    {
        BCryptDestroyKey(EncryptKeyHandle);
    }
     
    return Status;

}

//-----------------------------------------------------------------------------
//
//    Decrypt Data
//
//-----------------------------------------------------------------------------

NTSTATUS
DecryptData(
    _In_        BCRYPT_KEY_HANDLE   KeyHandle,
    _In_        DWORD               KeyObjectLength,
    _In_reads_bytes_(InitVectorLength)
                PBYTE               InitVector,
    _In_        DWORD               InitVectorLength,
    _In_reads_bytes_(ChainingModeLength)
                PBYTE               ChainingMode,
    _In_        DWORD               ChainingModeLength,
    _In_reads_bytes_(CipherTextLength)
                PBYTE               CipherText,
    _In_        DWORD               CipherTextLength,
    _Outptr_result_bytebuffer_(*PlainTextLengthPointer)
                PBYTE               *PlainTextPointer,
    _Out_       DWORD               *PlainTextLengthPointer
    )
{
    NTSTATUS    Status;
    PBYTE   TempInitVector = NULL;
    DWORD   TempInitVectorLength = 0;
    PBYTE   PlainText = NULL;
    DWORD   PlainTextLength = 0;
    DWORD   ResultLength = 0;
    BCRYPT_KEY_HANDLE DecryptKeyHandle = NULL;
    PBYTE   KeyObject = NULL;
    
    //
    // Allocate KeyObject on the heap
    //

    KeyObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, KeyObjectLength);
    if( NULL == KeyObject )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Generate a duplicate AES key 
    //

    Status = BCryptDuplicateKey(
                                        KeyHandle,                  // A pointer to key handle
                                        &DecryptKeyHandle,          // A pointer to BCRYPT_KEY_HANDLE that recieves a handle to the duplicate key 
                                        KeyObject,                  // A pointer to the buffer that recieves the duplicate key object
                                        KeyObjectLength,            // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    //
    // Set the chaining mode on the key handle
    //
    
    Status = BCryptSetProperty( 
                                        DecryptKeyHandle,           // Handle to a CNG object          
                                        BCRYPT_CHAINING_MODE,       // Property name(null terminated unicode string)
                                        ChainingMode,               // Address of the buffer that contains the new property value 
                                        ChainingModeLength,         // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Copy initialization vector into a temporary initialization vector buffer
    // Because after an encrypt/decrypt operation, the IV buffer is overwritten.
    //


    TempInitVector = (PBYTE)HeapAlloc (GetProcessHeap(), 0, InitVectorLength);
    if( NULL == TempInitVector )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }
    
    TempInitVectorLength = InitVectorLength;
    memcpy(TempInitVector, InitVector, TempInitVectorLength);
    
    //
    // Get CipherText's length
    // If the program can compute the length of cipher text(based on algorihtm and chaining mode info.), this call can be avoided.
    //
    
    Status = BCryptDecrypt(
                                        DecryptKeyHandle,           // Handle to a key which is used to encrypt 
                                        CipherText,                 // Address of the buffer that contains the ciphertext
                                        CipherTextLength,           // Size of the buffer in bytes
                                        NULL,                       // A pointer to padding info, used with asymmetric and authenticated encryption; else set to NULL
                                        TempInitVector,             // Address of the buffer that contains the IV. 
                                        TempInitVectorLength,       // Size of the IV buffer in bytes
                                        NULL,                       // Address of the buffer the recieves the plaintext
                                        0,                          // Size of the buffer in bytes
                                        &PlainTextLength,           // Variable that recieves number of bytes copied to plaintext buffer 
                                        BCRYPT_BLOCK_PADDING);      // Flags; Block padding allows to pad data to the next block size
                                        
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }


    PlainText = (PBYTE)HeapAlloc (GetProcessHeap (), 0, PlainTextLength);
    if(NULL == PlainText)
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Decrypt CipherText
    //

    Status = BCryptDecrypt(
                                        DecryptKeyHandle,           // Handle to a key which is used to encrypt 
                                        CipherText,                 // Address of the buffer that contains the ciphertext
                                        CipherTextLength,           // Size of the buffer in bytes
                                        NULL,                       // A pointer to padding info, used with asymmetric and authenticated encryption; else set to NULL
                                        TempInitVector,             // Address of the buffer that contains the IV. 
                                        TempInitVectorLength,       // Size of the IV buffer in bytes
                                        PlainText,                  // Address of the buffer the recieves the plaintext
                                        PlainTextLength,            // Size of the buffer in bytes
                                        &ResultLength,              // Variable that recieves number of bytes copied to plaintext buffer 
                                        BCRYPT_BLOCK_PADDING);      // Flags; Block padding allows to pad data to the next block size
                                       
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    *PlainTextPointer = PlainText;
    PlainText = NULL;
    *PlainTextLengthPointer = PlainTextLength;

cleanup:

    if( NULL != PlainText )
    {
        HeapFree(GetProcessHeap(), 0, PlainText);
    }

    if( NULL != KeyObject)
    {
        SecureZeroMemory(KeyObject, KeyObjectLength);
        HeapFree(GetProcessHeap(), 0, KeyObject);
    }

    if( NULL != TempInitVector )
    {
        HeapFree(GetProcessHeap(), 0, TempInitVector);
    }

    if( NULL != DecryptKeyHandle )
    {
        BCryptDestroyKey(DecryptKeyHandle);
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
    NTSTATUS    Status;
    SECURITY_STATUS     secStatus = S_OK;
    
    NCRYPT_PROV_HANDLE      ProviderHandle = 0;
    NCRYPT_KEY_HANDLE       KdfKeyHandle = 0;
    BCRYPT_KEY_HANDLE       AesKeyHandle = NULL;
    BCRYPT_ALG_HANDLE       AesAlgHandle = NULL;
    
    LPCWSTR                 KdfKeyName = L"SampleKDFKey";
    NCryptBuffer            GenericKdfParameters[2] = {0};
    NCryptBufferDesc        GenericKdfParamList;

    PBYTE                   IVBuffer = NULL;
    DWORD                   IVBufferLength = 0;
    PBYTE                   EncryptedMessage = NULL;
    DWORD                   EncryptedMessageLength = 0;
    PBYTE                   DecryptedMessage = NULL;
    DWORD                   DecryptedMessageLength = 0;
    DWORD                   ResultLength = 0;
    DWORD                   AesKeyMaterialLength = AesKeyLength;
    PBYTE                   AesKeyMaterial = NULL;
    DWORD                   KeyObjectLength = 0;
    DWORD                   BlockLength = 0;
    
    //
    // Open Microsoft KSP (Key Storage Provider) 
    //

    secStatus = NCryptOpenStorageProvider(
                                        &ProviderHandle,            // Pointer to a variable that recieves the provider handle
                                        MS_KEY_STORAGE_PROVIDER,    // Storage provider identifier(null terminated unicode string); If NULL, default provider is loaded
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Creating a persisted KDF key
    // KDF Algorithm used: SP800-108 Hmac counter mode
    //
    // NCRYPT_OVERWRITE_KEY_FLAG - If a key already exists in the container with the specified name, the existing key will be overwritten.
    //

    secStatus = NCryptCreatePersistedKey(
                                        ProviderHandle,             // Handle of the key storage provider
                                        &KdfKeyHandle,              // Address of the variable that recieves the key handle
                                        NCRYPT_SP800108_CTR_HMAC_ALGORITHM, // Algorithm name (null terminated unicode string)
                                        KdfKeyName,                 // Key name (null terminated unicode string)
                                        0,                          // Legacy identifier (AT_KEYEXCHANGE, AT_SIGNATURE or 0 )
                                        NCRYPT_OVERWRITE_KEY_FLAG); // Flags; If a key already exists in the container with the specified name, the existing key will be overwritten.
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Set the secret on the KDF key handle
    //

    secStatus = NCryptSetProperty(
                                        KdfKeyHandle,               // Handle of the key storage object 
                                        NCRYPT_KDF_SECRET_VALUE,    // Property name (null terminated unicode string)
                                        (PBYTE)Secret,              // Address of the buffer that contains the property value
                                        sizeof(Secret),             // Size of the buffer in bytes
                                        0);                         // Flags
                        
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Finalize the key generation process
    // The key is usable here onwards
    //

    secStatus = NCryptFinalizeKey(
                                        KdfKeyHandle,               // Handle of the key - that has to be finalized
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }  

    //
    // Construct parameter list
    //

    //
    // Generic parameters: 
    // KDF_GENERIC_PARAMETER and KDF_HASH_ALGORITHM are the generic parameters that can be passed for the following KDF algorithms:
    // BCRYPT/NCRYPT_SP800108_CTR_HMAC_ALGORITHM 
    //      KDF_GENERIC_PARAMETER = KDF_LABEL||0x00||KDF_CONTEXT 
    // BCRYPT/NCRYPT_SP80056A_CONCAT_ALGORITHM
    //      KDF_GENERIC_PARAMETER = KDF_ALGORITHMID || KDF_PARTYUINFO || KDF_PARTYVINFO {|| KDF_SUPPPUBINFO } {|| KDF_SUPPPRIVINFO }
    // BCRYPT/NCRYPT_PBKDF2_ALGORITHM
    //      KDF_GENERIC_PARAMETER = KDF_SALT
    // BCRYPT/NCRYPT_CAPI_KDF_ALGORITHM
    //      KDF_GENERIC_PARAMETER = Not used
    //
    // Alternatively, KDF specific parameters can be passed as well.
    // For NCRYPT_SP800108_CTR_HMAC_ALGORITHM: 
    //      KDF_HASH_ALGORITHM, KDF_LABEL and KDF_CONTEXT are required
    // For NCRYPT_SP80056A_CONCAT_ALGORITHM:
    //      KDF_HASH_ALGORITHM, KDF_ALGORITHMID, KDF_PARTYUINFO, KDF_PARTYVINFO are required
    //      KDF_SUPPPUBINFO, KDF_SUPPPRIVINFO are optional
    // For NCRYPT_PBKDF2_ALGORITHM
    //      KDF_HASH_ALGORITHM is required
    //      KDF_ITERATION_COUNT, KDF_SALT are optional
    //      Iteration count, (if not specified) will default to 10,000
    // For NCRYPT_CAPI_KDF_ALGORITHM
    //      KDF_HASH_ALGORITHM is required
    //

    //
    // Generic parameters are used in the sample
    // KDF_HASH_ALGORITHM
    //
    GenericKdfParameters[0].cbBuffer = sizeof(BCRYPT_SHA256_ALGORITHM);
    GenericKdfParameters[0].BufferType = KDF_HASH_ALGORITHM;
    GenericKdfParameters[0].pvBuffer = (void*)BCRYPT_SHA256_ALGORITHM;
   
    //
    // KDF_GENERIC_PARAMETER
    //
    GenericKdfParameters[1].cbBuffer = sizeof(GenericParameter);
    GenericKdfParameters[1].BufferType = KDF_GENERIC_PARAMETER;
    GenericKdfParameters[1].pvBuffer = (void*)GenericParameter;
   
    GenericKdfParamList.ulVersion = NCRYPTBUFFER_VERSION;
    GenericKdfParamList.cBuffers = ARRAYSIZE(GenericKdfParameters);
    GenericKdfParamList.pBuffers = GenericKdfParameters;

    //
    // Allocate the AesKeyMaterial on the heap
    //

    AesKeyMaterial = (PBYTE) HeapAlloc( GetProcessHeap(), 0, AesKeyMaterialLength);
    if( NULL == AesKeyMaterial )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Derive an ephemeral key
    //

    secStatus = NCryptKeyDerivation(
                                        KdfKeyHandle,               // Handel of the key
                                        &GenericKdfParamList,       // Address of the NCryptBufferDesc structure that contains the KDF parameters
                                        AesKeyMaterial,             // Variable that recieves the derived key material
                                        AesKeyMaterialLength,       // Size of the buffer in bytes
                                        &ResultLength,
                                        0);                         // Flags 
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Open an algorithm handle
    //
    
    Status = BCryptOpenAlgorithmProvider(
                                        &AesAlgHandle,              // Alg Handle pointer
                                        BCRYPT_AES_ALGORITHM,       // Cryptographic Algorithm name (null terminated unicode string)
                                        NULL,                       // Provider name; if null, the default provider is loaded
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        secStatus = HRESULT_FROM_NT(Status);
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Generate an AES key from the key bytes
    //

    Status = BCryptGenerateSymmetricKey(
                                        AesAlgHandle,               // Algorithm provider handle
                                        &AesKeyHandle,              // A pointer to key handle
                                        NULL,                       // A pointer to the buffer that recieves the key object;NULL implies memory is allocated and freed by the function
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE)AesKeyMaterial,      // A pointer to a buffer that contains the key material
                                        AesKeyMaterialLength,       // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        secStatus = HRESULT_FROM_NT(Status);
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Obtain key object size
    //

    Status = BCryptGetProperty(
                                        AesAlgHandle,               // Handle to a CNG object
                                        BCRYPT_OBJECT_LENGTH,       // Property name (null terminated unicode string)
                                        (PBYTE)&KeyObjectLength,    // Addr of the output buffer which recieves the property value
                                        sizeof (KeyObjectLength),   // Size of the buffer in the bytes
                                        &ResultLength,              // Number of bytes that were copied into the buffer
                                        0);                         // Flags
                                        
    if( !NT_SUCCESS(Status) )
    {
        secStatus = HRESULT_FROM_NT(Status);
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Obtain block size
    //
    
    Status = BCryptGetProperty(
                                        AesAlgHandle,               // Handle to a CNG object
                                        BCRYPT_BLOCK_LENGTH,        // Property name (null terminated unicode string)
                                        (PBYTE)&BlockLength,        // Addr of the output buffer which recieves the property value
                                        sizeof (BlockLength),       // Size of the buffer in the bytes
                                        &ResultLength,              // Number of bytes that were copied into the buffer
                                        0);                         // Flags
                                        
    if( !NT_SUCCESS(Status) )
    {
        secStatus = HRESULT_FROM_NT(Status);
        ReportError(secStatus);
        goto cleanup;
    }

    IVBufferLength = BlockLength; 

    //
    // Allocate the InitVector on the heap
    //

    IVBuffer = (PBYTE) HeapAlloc( GetProcessHeap(), 0, IVBufferLength);
    if( NULL == IVBuffer )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }
    
    //
    // Generate IV randomly
    //

    Status = BCryptGenRandom (
                                        NULL,                       // Alg Handle pointer; If NULL, the default provider is chosen
                                        (PBYTE)IVBuffer,            // Address of the buffer that recieves the random number(s)
                                        IVBufferLength,             // Size of the buffer in bytes
                                        BCRYPT_USE_SYSTEM_PREFERRED_RNG); // Flags 
    if( !NT_SUCCESS(Status) )
    {
        secStatus = HRESULT_FROM_NT(Status);
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Encrypt plain text
    //
        
    Status = EncryptData(
                                        AesKeyHandle,
                                        KeyObjectLength,
                                        IVBuffer,
                                        IVBufferLength,
                                        (PBYTE)BCRYPT_CHAIN_MODE_CBC,
                                        sizeof (BCRYPT_CHAIN_MODE_CBC),
                                        (PBYTE)Message,
                                        sizeof (Message),
                                        &EncryptedMessage,
                                        &EncryptedMessageLength);

    if( !NT_SUCCESS(Status) )
    {
        secStatus = HRESULT_FROM_NT(Status);
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Decrypt Cipher text
    //

    Status = DecryptData(
                                        AesKeyHandle,
                                        KeyObjectLength,
                                        IVBuffer,
                                        IVBufferLength,
                                        (PBYTE)BCRYPT_CHAIN_MODE_CBC,
                                        sizeof (BCRYPT_CHAIN_MODE_CBC),
                                        EncryptedMessage,
                                        EncryptedMessageLength,
                                        &DecryptedMessage,
                                        &DecryptedMessageLength);

    if( !NT_SUCCESS(Status) )
    {
        secStatus = HRESULT_FROM_NT(Status);
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Optional : Check if the original message and the message obtained after decrypt are the same 
    //

    if( 0 != (memcmp(Message, DecryptedMessage, sizeof (Message))) )
    {
        secStatus = NTE_FAIL;
        ReportError(secStatus);
        goto cleanup;
    }

    secStatus = S_OK;
      
    wprintf(L"Success!\n");

cleanup:
    
    if( NULL != EncryptedMessage )
    {
        HeapFree( GetProcessHeap(), 0, EncryptedMessage);
        EncryptedMessage = NULL;
    }

    if( NULL != DecryptedMessage )
    {
        HeapFree( GetProcessHeap(), 0, DecryptedMessage);
        DecryptedMessage = NULL;
    }

    if( NULL != AesKeyMaterial )
    {
        SecureZeroMemory(AesKeyMaterial, AesKeyMaterialLength);
        HeapFree( GetProcessHeap(), 0, AesKeyMaterial);
        AesKeyMaterial = NULL;
    }

    if( NULL != IVBuffer )
    {
        HeapFree( GetProcessHeap(), 0, IVBuffer);
        IVBuffer = NULL;
    }

    if( NULL != AesKeyHandle )
    {
        BCryptDestroyKey(AesKeyHandle);                           
        AesKeyHandle = NULL;
    }

    if( NULL != AesAlgHandle )
    {
        BCryptCloseAlgorithmProvider(AesAlgHandle, 0);
        AesAlgHandle = NULL;
    }

    if( NULL != KdfKeyHandle )
    {
        NCryptDeleteKey(KdfKeyHandle, 0);
        KdfKeyHandle = 0;
    }

    if( NULL != ProviderHandle )
    {
        NCryptFreeObject(ProviderHandle);                           // The handle of the object to free
        ProviderHandle = 0;
    }
       
    return (DWORD)secStatus;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
}
