// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


//
//
//  File:              CipherEncryptionDecryption.cpp
//
//  Contents:          This sample shows how to encrypt and decrypt a message given a password using
//                     128 bit AES in CBC mode. 
//                     An AES 128 bit key is derived from the password using PBKDF2
//                     IV for the encrypt and decrypt operations is generated randomly.
//    
//

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>
#include <winerror.h>
#include <stdio.h>
#include <bcrypt.h>
#include <sal.h>

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
	_In_	DWORD       dwErrCode 
    )
{
    wprintf( L"Error: 0x%08x (%d)\n", dwErrCode, dwErrCode );
}

static const 
BYTE PlainTextArray[] = 
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

static const 
BYTE Aes128Password[] = {'P', 'A', 'S', 'S', 'W', 'O', 'R', 'D'};

static const
BYTE Salt [] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

static const
ULONGLONG IterationCount = 1024;


//-----------------------------------------------------------------------------
//
//    Encrypt Data
//
//-----------------------------------------------------------------------------

NTSTATUS
EncryptData(
    _In_        BCRYPT_ALG_HANDLE   AlgHandle,
    _In_reads_bytes_(KeyLength)
                PBYTE               Key,
    _In_        DWORD               KeyLength,
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

    BCRYPT_KEY_HANDLE   KeyHandle = NULL;

    DWORD   ResultLength = 0;
    PBYTE   TempInitVector = NULL;
    DWORD   TempInitVectorLength = 0;
    PBYTE   CipherText = NULL;
    DWORD   CipherTextLength = 0;

    //
    // Generate an AES key from the key bytes
    //

    Status = BCryptGenerateSymmetricKey(
                                        AlgHandle,                  // Algorithm provider handle
                                        &KeyHandle,                 // A pointer to key handle
                                        NULL,                       // A pointer to the buffer that recieves the key object;NULL implies memory is allocated and freed by the function
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE)Key,                 // A pointer to a buffer that contains the key material
                                        KeyLength,                  // Size of the buffer in bytes
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
                                        KeyHandle,                  // Handle to a CNG object          
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
                                        KeyHandle,                  // Handle to a key which is used to encrypt 
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
                                        KeyHandle,                  // Handle to a key which is used to encrypt 
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

    if( NULL != TempInitVector )
    {
        HeapFree(GetProcessHeap(), 0, TempInitVector);
    }

    if( NULL != KeyHandle )
    {
        BCryptDestroyKey(KeyHandle);
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
    _In_        BCRYPT_ALG_HANDLE   AlgHandle,
    _In_reads_bytes_(KeyLength)
                PBYTE               Key,
    _In_        DWORD               KeyLength,
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

    BCRYPT_KEY_HANDLE   KeyHandle = NULL;

    PBYTE   TempInitVector = NULL;
    DWORD   TempInitVectorLength = 0;
    PBYTE   PlainText = NULL;
    DWORD   PlainTextLength = 0;
    DWORD   ResultLength = 0;

    //
    // Generate an AES key from the key bytes
    //

    Status = BCryptGenerateSymmetricKey(
                                        AlgHandle,                  // Algorithm provider handle
                                        &KeyHandle,                 // A pointer to key handle
                                        NULL,                       // A pointer to the buffer that recieves the key object;NULL implies memory is allocated and freed by the function
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE)Key,                 // A pointer to a buffer that contains the key material
                                        KeyLength,                  // Size of the buffer in bytes
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
                                        KeyHandle,                  // Handle to a CNG object          
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
                                        KeyHandle,                  // Handle to a key which is used to encrypt 
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
                                        KeyHandle,                  // Handle to a key which is used to encrypt 
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

    if( NULL != TempInitVector )
    {
        HeapFree(GetProcessHeap(), 0, TempInitVector);
    }

    if( NULL != KeyHandle )
    {
        BCryptDestroyKey(KeyHandle);
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
    _In_                int     argc,
    _In_reads_(argc)   LPWSTR  argv[]
    )
{
    NTSTATUS    Status;

    BCRYPT_ALG_HANDLE   AesAlgHandle = NULL;
    BCRYPT_ALG_HANDLE   KdfAlgHandle = NULL;

    BCRYPT_KEY_HANDLE   Aes128PasswordKeyHandle = NULL;
    
    PBYTE InitVector = NULL;
    DWORD InitVectorLength = 0;
    PBYTE Aes128Key = NULL;
    DWORD Aes128KeyLength = 0;

    PBYTE CipherText = NULL;
    DWORD CipherTextLength = 0;
    PBYTE PlainText = NULL;
    DWORD PlainTextLength = 0;

    DWORD ResultLength = 0;
    DWORD BlockLength = 0;

    BCryptBuffer PBKDF2ParameterBuffers[] = {
                                        {
                                            sizeof (BCRYPT_SHA256_ALGORITHM),
                                            KDF_HASH_ALGORITHM,
                                            BCRYPT_SHA256_ALGORITHM,
                                        },
                                        {
                                            sizeof (Salt),
                                            KDF_SALT,
                                            (PBYTE)Salt,
                                        },
                                        {
                                            sizeof (IterationCount),
                                            KDF_ITERATION_COUNT,
                                            (PBYTE)&IterationCount,
                                        }
    };

    BCryptBufferDesc PBKDF2Parameters = {
                                        BCRYPTBUFFER_VERSION,
                                        3,
                                        PBKDF2ParameterBuffers
    };

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
        ReportError(Status);
        goto cleanup;
    }

    
    //
    // Convert bits to bytes
    //
    
    Aes128KeyLength = 128/8;

    //
    // Allocate Key buffer
    //

    Aes128Key = (PBYTE) HeapAlloc( GetProcessHeap(), 0, Aes128KeyLength);
    if( NULL == Aes128Key )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Derive the AES 128 key from the password 
    // Using PBKDF2
    //

    //
    // Open an algorithm handle
    //
    
    Status = BCryptOpenAlgorithmProvider(
                                        &KdfAlgHandle,              // Alg Handle pointer
                                        BCRYPT_PBKDF2_ALGORITHM,    // Cryptographic Algorithm name (null terminated unicode string)
                                        NULL,                       // Provider name; if null, the default provider is loaded
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    
    //
    // Create a key handle to the password
    //

    Status = BCryptGenerateSymmetricKey(
                                        KdfAlgHandle,               // Algorithm Handle 
                                        &Aes128PasswordKeyHandle,   // A pointer to a key handle
                                        NULL,                       // Buffer that recieves the key object;NULL implies memory is allocated and freed by the function
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE)Aes128Password,      // Buffer that contains the key material
                                        sizeof (Aes128Password),    // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }


    //
    // Derive AES key from the password
    //

    Status = BCryptKeyDerivation(
                                        Aes128PasswordKeyHandle,    // Handle to the password key
                                        &PBKDF2Parameters,          // Parameters to the KDF algorithm
                                        Aes128Key,                  // Address of the buffer which recieves the derived bytes
                                        Aes128KeyLength,            // Size of the buffer in bytes
                                        &ResultLength,              // Variable that recieves number of bytes copied to above buffer  
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
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
        ReportError(Status);
        goto cleanup;
    }


    InitVectorLength = BlockLength; 

    //
    // Allocate the InitVector on the heap
    //

    InitVector = (PBYTE) HeapAlloc( GetProcessHeap(), 0, InitVectorLength);
    if( NULL == InitVector )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }
    
    //
    // Generate IV randomly
    //

    Status = BCryptGenRandom (
                                        NULL,                       // Alg Handle pointer; If NULL, the default provider is chosen
                                        (PBYTE)InitVector,          // Address of the buffer that recieves the random number(s)
                                        InitVectorLength,           // Size of the buffer in bytes
                                        BCRYPT_USE_SYSTEM_PREFERRED_RNG); // Flags 
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }


    //
    // Encrypt plain text
    //
    
    Status = EncryptData(
                                        AesAlgHandle,
                                        Aes128Key,
                                        Aes128KeyLength,
                                        InitVector,
                                        InitVectorLength,
                                        (PBYTE)BCRYPT_CHAIN_MODE_CBC,
                                        sizeof (BCRYPT_CHAIN_MODE_CBC),
                                        (PBYTE)PlainTextArray,
                                        sizeof (PlainTextArray),
                                        &CipherText,
                                        &CipherTextLength);

    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    
    //
    // Decrypt Cipher text
    //

    Status = DecryptData(
                                        AesAlgHandle,
                                        Aes128Key,
                                        Aes128KeyLength,
                                        InitVector,
                                        InitVectorLength,
                                        (PBYTE)BCRYPT_CHAIN_MODE_CBC,
                                        sizeof (BCRYPT_CHAIN_MODE_CBC),
                                        CipherText,
                                        CipherTextLength,
                                        &PlainText,
                                        &PlainTextLength);

    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }


    //
    // Optional : Check if the original plaintext and the plaintext obtained after decrypt are the same 
    //

    if( 0 != (memcmp(PlainText, PlainTextArray, sizeof (PlainTextArray))) )
    {
        Status = STATUS_UNSUCCESSFUL;
        ReportError(Status);
        goto cleanup;
    }

    Status = STATUS_SUCCESS;
    wprintf(L"Success : Plaintext has been encrypted, ciphertext has been decrypted with AES-128 bit key\n");

cleanup:

    if( NULL != CipherText)
    {
        SecureZeroMemory(CipherText, CipherTextLength);
        HeapFree(GetProcessHeap(), 0, CipherText);
    }

    if( NULL != PlainText)
    {
        HeapFree(GetProcessHeap(), 0, PlainText);
    }

    if( NULL != InitVector )
    {
        HeapFree(GetProcessHeap(), 0, InitVector);
    }

    if( NULL != Aes128Key )
    {
        SecureZeroMemory(Aes128Key, Aes128KeyLength);
        HeapFree(GetProcessHeap(), 0, Aes128Key);
    }

    if( NULL != AesAlgHandle)
    {
        BCryptCloseAlgorithmProvider(AesAlgHandle,0);
    }
    
    if( NULL != KdfAlgHandle)
    {
        BCryptCloseAlgorithmProvider(KdfAlgHandle,0);
    }


    return (DWORD)Status;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
}
    
    

  
