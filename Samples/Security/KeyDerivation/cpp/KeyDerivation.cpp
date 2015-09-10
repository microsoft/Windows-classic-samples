// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             KeyDerivation.cpp
//
//  Contents:         This sample shows how to derive bytes from a secret 
//                    using BCryptKeyDerivation API.
//    
//

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>
#include <winerror.h>
#include <stdio.h>
#include <sal.h>

#include "KeyDerivation.h"

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
// PerformKeyDerivation
//
//----------------------------------------------------------------------------
NTSTATUS
PeformKeyDerivation(
    _In_  DWORD    ArrayIndex
    )
{
    NTSTATUS            Status;
    BCRYPT_ALG_HANDLE   KdfAlgHandle = NULL;
    BCRYPT_KEY_HANDLE   SecretKeyHandle = NULL;
    
    DWORD   ResultLength = 0;
    PBYTE   DerivedKey = NULL;
    DWORD   DerivedKeyLength = 0;
  
    
    Status = BCryptOpenAlgorithmProvider(
                                        &KdfAlgHandle,              // Alg Handle pointer
                                        KdfAlgorithmNameArray[ArrayIndex],   
                                                                    // Cryptographic Algorithm name (null terminated unicode string)
                                        NULL,                       // Provider name; if null, the default provider is loaded
                                        0);                         // Flags
        if( !NT_SUCCESS(Status) )
        {
            ReportError(Status);
            goto cleanup;
        }
        
        Status = BCryptGenerateSymmetricKey(
                                        KdfAlgHandle,               // Algorithm Handle 
                                        &SecretKeyHandle,           // A pointer to a key handle
                                        NULL,                       // Buffer that recieves the key object;NULL implies memory is allocated and freed by the function
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE) Secret,             // Buffer that contains the key material
                                        sizeof(Secret),             // Size of the buffer in bytes
                                        0);                         // Flags
        if( !NT_SUCCESS(Status) )
        {
            ReportError(Status);
            goto cleanup;
        }
        
        //
        // Derive the key
        //
        
        DerivedKeyLength = DERIVED_KEY_LEN;
        
        DerivedKey = (PBYTE)HeapAlloc(GetProcessHeap(), 0, DerivedKeyLength);
        if( NULL == DerivedKey )
        {
            Status = STATUS_NO_MEMORY;
            ReportError(Status);
            goto cleanup;
        }

        // Generic parameters: 
        // KDF_GENERIC_PARAMETER and KDF_HASH_ALGORITHM are the generic parameters that can be passed for the following KDF algorithms:
        // BCRYPT_SP800108_CTR_HMAC_ALGORITHM 
        //      KDF_GENERIC_PARAMETER = KDF_LABEL||0x00||KDF_CONTEXT 
        // BCRYPT_SP80056A_CONCAT_ALGORITHM
        //      KDF_GENERIC_PARAMETER = KDF_ALGORITHMID || KDF_PARTYUINFO || KDF_PARTYVINFO {|| KDF_SUPPPUBINFO } {|| KDF_SUPPPRIVINFO }
        // BCRYPT_PBKDF2_ALGORITHM
        //      KDF_GENERIC_PARAMETER = KDF_SALT
        // BCRYPT_CAPI_KDF_ALGORITHM
        //      KDF_GENERIC_PARAMETER = Not used
        //
        // Alternatively, KDF specific parameters can be passed.
        // For BCRYPT_SP800108_CTR_HMAC_ALGORITHM: 
        //      KDF_HASH_ALGORITHM, KDF_LABEL and KDF_CONTEXT are required
        // For BCRYPT_SP80056A_CONCAT_ALGORITHM:
        //      KDF_HASH_ALGORITHM, KDF_ALGORITHMID, KDF_PARTYUINFO, KDF_PARTYVINFO are required
        //      KDF_SUPPPUBINFO, KDF_SUPPPRIVINFO are optional
        // For BCRYPT_PBKDF2_ALGORITHM
        //      KDF_HASH_ALGORITHM is required
        //      KDF_ITERATION_COUNT, KDF_SALT are optional
        //      Iteration count, (if not specified) will default to 10,000
        // For BCRYPT_CAPI_KDF_ALGORITHM
        //      KDF_HASH_ALGORITHM is required
        //
       
        // 
        // This sample uses KDF specific parameters defined in KeyDerivation.h
        //

        Status = BCryptKeyDerivation(
                                        SecretKeyHandle,            // Handle to the password key
                                        &ParamList[ArrayIndex],     // Parameters to the KDF algorithm
                                        DerivedKey,                 // Address of the buffer which recieves the derived bytes
                                        DerivedKeyLength,           // Size of the buffer in bytes
                                        &ResultLength,              // Variable that recieves number of bytes copied to above buffer 
                                        0);                         // Flags
        if( !NT_SUCCESS(Status) )
        {
            ReportError(Status);
            goto cleanup;
        }

        //
        // DerivedKeyLength bytes have been derived
        //

cleanup:    
   
        if( NULL != DerivedKey )
        {
            HeapFree( GetProcessHeap(), 0, DerivedKey);
            DerivedKey = NULL;
        }

        if( NULL != SecretKeyHandle )
        {
            Status = BCryptDestroyKey(SecretKeyHandle);
            if( !NT_SUCCESS(Status) )
            {
                ReportError(Status);
            }
            SecretKeyHandle = NULL;
        }

        if( NULL != KdfAlgHandle )
        {
            Status = BCryptCloseAlgorithmProvider(KdfAlgHandle,0);
            if( !NT_SUCCESS(Status) )
            {
                ReportError(Status);
            }
            KdfAlgHandle = NULL;
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
    NTSTATUS    Status = STATUS_SUCCESS;
    DWORD   i = 0 ;

    for( i=0; i<ARRAYSIZE(KdfAlgorithmNameArray); i++ )
    {
        Status = PeformKeyDerivation(i);

        if( !NT_SUCCESS(Status) )
        {
            ReportError(Status);
            goto exit;
        }

    } // for loop
   
exit:

  return (DWORD)Status;
    
  UNREFERENCED_PARAMETER( argc );
  UNREFERENCED_PARAMETER( argv );
}
