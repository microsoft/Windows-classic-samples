// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:              RandomNumGeneration.cpp
//
//  Contents:          This sample shows random number generation in CNG. 
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
    
    BYTE    Buffer[128];
    DWORD   BufferSize;
    
    BufferSize = sizeof (Buffer);
    memset (Buffer, 0, BufferSize);
    
    //
    // Fill the buffer with random bytes
    //

    Status = BCryptGenRandom (
                                        NULL,                       // Alg Handle pointer; NUll is passed as BCRYPT_USE_SYSTEM_PREFERRED_RNG flag is used
                                        Buffer,                     // Address of the buffer that recieves the random number(s)
                                        BufferSize,                 // Size of the buffer in bytes
                                        BCRYPT_USE_SYSTEM_PREFERRED_RNG); // Flags                  

    if( !NT_SUCCESS(Status) )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    Status = STATUS_SUCCESS;
        
cleanup:
    
    return (DWORD)Status;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
}