// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             HashComputation.cpp
//
//  Contents:         This sample shows how to compute SHA 256 hash of a message(s) using CNG.
//
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

static const 
BYTE Message[] = 
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
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
    NTSTATUS    Status;
    
    BCRYPT_ALG_HANDLE   AlgHandle = NULL;
    BCRYPT_HASH_HANDLE  HashHandle = NULL;
    
    PBYTE   Hash = NULL;
    DWORD   HashLength = 0;
    DWORD   ResultLength = 0;

    //
    // Open an algorithm handle
    // This sample passes BCRYPT_HASH_REUSABLE_FLAG with BCryptAlgorithmProvider(...) to load a provider which supports reusable hash
    //
    
    Status = BCryptOpenAlgorithmProvider(
                                        &AlgHandle,                 // Alg Handle pointer
                                        BCRYPT_SHA256_ALGORITHM,    // Cryptographic Algorithm name (null terminated unicode string)
                                        NULL,                       // Provider name; if null, the default provider is loaded
                                        BCRYPT_HASH_REUSABLE_FLAG); // Flags; Loads a provider which supports reusable hash
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Obtain the length of the hash
    //

     Status = BCryptGetProperty(
                                        AlgHandle,                  // Handle to a CNG object
                                        BCRYPT_HASH_LENGTH,         // Property name (null terminated unicode string)
                                        (PBYTE)&HashLength,         // Address of the output buffer which recieves the property value
                                        sizeof(HashLength),         // Size of the buffer in bytes
                                        &ResultLength,              // Number of bytes that were copied into the buffer
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Allocate the hash buffer on the heap
    //

    Hash = (PBYTE)HeapAlloc (GetProcessHeap (), 0, HashLength);
    if( NULL == Hash )
    {
        Status = STATUS_NO_MEMORY;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Create a hash handle
    //

    Status = BCryptCreateHash(
                                        AlgHandle,                  // Handle to an algorithm provider                 
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object
                                        0,                          // Size of the buffer in bytes
                                        NULL,                       // A pointer to a key to use for the hash or MAC
                                        0,                          // Size of the key in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    //
    // Hash the message(s)
    // More than one message can be hashed by calling BCryptHashData 
    //
   
    Status = BCryptHashData(
                                        HashHandle,                 // Handle to the hash or MAC object
                                        (PBYTE)Message,             // A pointer to a buffer that contains the data to hash
                                        sizeof (Message),           // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    //
    // Obtain the hash of the message(s) into the hash buffer
    //
    
    Status = BCryptFinishHash(
                                        HashHandle,                 // Handle to the hash or MAC object
                                        Hash,                       // A pointer to a buffer that receives the hash or MAC value
                                        HashLength,                 // Size of the buffer in bytes
                                        0);                         // Flags
    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }

    Status = STATUS_SUCCESS;
       
cleanup:
    
    if( NULL != Hash)
    {
        HeapFree(GetProcessHeap(), 0, Hash);
    }

    if (NULL != HashHandle)    
    {
         BCryptDestroyHash(HashHandle);                             // Handle to hash/MAC object which needs to be destroyed
    }

    if( NULL != AlgHandle )
    {
        BCryptCloseAlgorithmProvider(
                                        AlgHandle,                  // Handle to the algorithm provider which needs to be closed
                                        0);                         // Flags
    }
    
    return (DWORD)Status;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
}



