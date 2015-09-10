// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:              HmacComputation.cpp
//
//  Contents:          This sample shows how to compute SHA1-HMAC of a message(s) using CNG
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

static const 
BYTE HmacKey[] =
{
    0x1b, 0x20, 0x5a, 0x9e, 0x2b, 0xe3, 0xfe, 0x85, 
    0x9c, 0x37, 0xf1, 0xaf, 0xfe, 0x81, 0x88, 0x92, 
    0x63, 0x27, 0x38, 0x61,
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
    DWORD   IterationCount = 2; //Perform HMAC computation twice
    BOOL    IsReusable = TRUE;
    DWORD   LoopCounter = 0;

    //
    // Open an algorithm handle
    // 

    Status = BCryptOpenAlgorithmProvider(
                                        &AlgHandle,                 // Alg Handle pointer
                                        BCRYPT_SHA1_ALGORITHM,      // Cryptographic Algorithm name (null terminated unicode string) 
                                        NULL,                       // Provider name; if null, the default provider is loaded
                                        BCRYPT_ALG_HANDLE_HMAC_FLAG); // Flags, Peform HMAC 
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
    // Create a hash
    // BCRYPT_REUSABLE_HASH_FLAG ensures the reusable hash implementation is used for the specified algorithm.
    // If the provider does not support it, the entire sequence(BCryptCreateHash, BCryptHashData, BCryptFinishHash and BCryptDestroyHash) should be followed for every HMAC/Hash computation
    //

    Status = BCryptCreateHash(
                                        AlgHandle,                  // Handle to an algorithm provider                 
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer. 
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE)HmacKey,             // A pointer to a key to use for the hash or MAC
                                        sizeof(HmacKey),            // Size of the key in bytes
                                        BCRYPT_HASH_REUSABLE_FLAG); // Flags 

    if( !NT_SUCCESS(Status) )
    {
        //
        // The provider does not support reusable hash implementation
        //

        IsReusable = FALSE;
        Status = BCryptCreateHash(
                                        AlgHandle,                  // Handle to an algorithm provider                 
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer. 
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE)HmacKey,             // A pointer to a key to use for the hash or MAC
                                        sizeof(HmacKey),            // Size of the key in bytes
                                        0);                         // Flags
        if( !NT_SUCCESS(Status) )
        {
            ReportError(Status);
            goto cleanup;
        }   
    }  

    do
    { 
        //
        // Hash the message(s)
        // More than one message can be hashed by calling BCryptHashData 
        //

        Status = BCryptHashData(
                                            HashHandle,             // Handle to the hash or MAC object
                                            (PBYTE)Message,         // A pointer to a buffer that contains the data to hash
                                            sizeof (Message),       // Size of the buffer in bytes
                                            0);                     // Flags
        if( !NT_SUCCESS(Status) )
        {
            ReportError(Status);
            goto cleanup;
        }

    
        //
        // Close the hash
        //

        Status = BCryptFinishHash(
                                            HashHandle,             // Handle to the hash or MAC object
                                            Hash,                   // A pointer to a buffer that receives the hash or MAC value
                                            HashLength,             // Size of the buffer in bytes
                                            0);                     // Flags
        if( !NT_SUCCESS(Status) )
        {
            ReportError(Status);
            goto cleanup;
        }

        if( !IsReusable )
        {
            Status = BCryptDestroyHash(HashHandle);                 // Handle to hash/MAC object which needs to be destroyed
            if( !NT_SUCCESS(Status) )
            {
                ReportError(Status);
                goto cleanup;
            }
            HashHandle = NULL;

            Status = BCryptCreateHash(
                                        AlgHandle,                  // Handle to an algorithm provider                 
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer. 
                                        0,                          // Size of the buffer in bytes
                                        (PBYTE)HmacKey,             // A pointer to a key to use for the hash or MAC
                                        sizeof(HmacKey),            // Size of the key in bytes
                                        0);                         // Flags
            if( !NT_SUCCESS(Status) )
            {
                ReportError(Status);
                goto cleanup;
            } 
        }

        LoopCounter++;

    } while( LoopCounter < IterationCount );


    Status = STATUS_SUCCESS;

 cleanup:

    if( NULL != Hash )
    {
        HeapFree(GetProcessHeap(), 0, Hash);
    }
    
    if( NULL != HashHandle )    
    {
        BCryptDestroyHash(HashHandle);                              // Handle to hash/MAC object which needs to be destroyed
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

