// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             DataProtection.cpp
//
//  Contents:         This sample demonstrates the use of Data Protection API.
//    
//

#include <windows.h>
#include <winerror.h>
#include <stdio.h>
#include <ncrypt.h>
#include <ncryptprotect.h>
#include <sal.h>

//
// Default Protection Descriptor String (Sample)
//
static    LPCWSTR                 ProtectionDescriptorString = L"LOCAL=logon";

//
// Some sample data to protect
//
static    LPCWSTR                 SecretString = L"Some message to protect";

//----------------------------------------------------------------------------
//
//  ReportError
//  Prints error information to the console
//
//----------------------------------------------------------------------------
void 
ReportError( 
    _In_    SECURITY_STATUS       Status 
    )
{
    wprintf( L"Error: 0x%08x \n", Status );
}

//-----------------------------------------------------------------------------
//
//    Protect Secret
//
//-----------------------------------------------------------------------------

SECURITY_STATUS
ProtectSecret(
    _In_        LPCWSTR             ProtectionDescString,
    _In_reads_bytes_(PlainTextLength)
                PBYTE               PlainText,
    _In_        ULONG               PlainTextLength,
    _Outptr_result_bytebuffer_maybenull_(*ProtectedDataLengthPointer) 
                PBYTE               *ProtectedDataPointer,
    _Out_       ULONG               *ProtectedDataLengthPointer
    )
{
    SECURITY_STATUS Status;
    PBYTE       ProtectedData = NULL;
    ULONG       ProtectedDataLength = 0;
  
    NCRYPT_DESCRIPTOR_HANDLE    DescriptorHandle = NULL;

    *ProtectedDataPointer = NULL;
    *ProtectedDataLengthPointer = 0;

    //
    // Create Protection Descriptor Handle from the supplied 
    // protection string
    //

    Status = NCryptCreateProtectionDescriptor(
                                        ProtectionDescString,
                                        0,
                                        &DescriptorHandle
                                        );
    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }

    //
    // Protect using the Protection Descriptor Handle
    //

    Status = NCryptProtectSecret(
                            DescriptorHandle,
                            0,
                            PlainText,
                            PlainTextLength,
                            NULL, // Use default allocations by LocalAlloc/LocalFree
                            NULL, // Use default parent windows handle. 
                            &ProtectedData,  // out LocalFree
                            &ProtectedDataLength
                            );
    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }

    *ProtectedDataPointer = ProtectedData;
    ProtectedData = NULL;
    *ProtectedDataLengthPointer = ProtectedDataLength;

    Status = ERROR_SUCCESS;
cleanup:

    //
    // Release allocated resources
    //

    if( NULL != ProtectedData)
    {
        LocalFree( ProtectedData );
    }

    if( NULL != DescriptorHandle )
    {
        NCryptCloseProtectionDescriptor( DescriptorHandle );
    }
   
    return Status;

}

//-----------------------------------------------------------------------------
//
//    Unprotect Secret
//
//-----------------------------------------------------------------------------

SECURITY_STATUS
UnprotectSecret(
    _In_reads_bytes_(ProtectedDataLength)
                PBYTE               ProtectedData,
    _In_        ULONG               ProtectedDataLength,
    _Outptr_result_bytebuffer_maybenull_(*PlainTextLengthPointer)
                PBYTE               *PlainTextPointer,
    _Out_       ULONG               *PlainTextLengthPointer
    )
{
    SECURITY_STATUS    Status;
    PBYTE       PlainText = NULL;
    DWORD       PlainTextLength = 0;

    *PlainTextPointer = NULL;
    *PlainTextLengthPointer = 0;

    //
    // Unprotect the secret
    //

    Status = NCryptUnprotectSecret(
                                NULL,       // Optional
                                0,          // no flags
                                ProtectedData,
                                ProtectedDataLength,
                                NULL,        // Use default allocations by LocalAlloc/LocalFree
                                NULL,        // Use default parent windows handle. 
                                &PlainText,  // out LocalFree
                                &PlainTextLength
                                );

    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }

    *PlainTextPointer = PlainText;
    PlainText = NULL;
    *PlainTextLengthPointer = PlainTextLength;

    Status = ERROR_SUCCESS;

cleanup:

    //
    // Release allocated resources
    //

    if( NULL != PlainText )
    {
        LocalFree( PlainText );
    }
  
    return Status;
}

//-----------------------------------------------------------------------------
//
//    Get protection information from the encrypted data
//
//-----------------------------------------------------------------------------

SECURITY_STATUS
GetProtectionInfo( 
    _In_reads_bytes_(ProtectedDataLength)
                PBYTE               ProtectedData,
    _In_        ULONG               ProtectedDataLength
    )
{
    SECURITY_STATUS    Status;

    NCRYPT_DESCRIPTOR_HANDLE            DescriptorHandle = NULL;
    LPWSTR                              DescriptorString = NULL;
    PBYTE                               Data = NULL;
    DWORD                               DataLength = 0;

    //
    // Open the encrypted message without actually decrypting it.
    // This call will only reconstruct the Protectuion Descriptor
    //
    Status = NCryptUnprotectSecret(
                                &DescriptorHandle,
                                NCRYPT_UNPROTECT_NO_DECRYPT,
                                ProtectedData,
                                (ULONG)ProtectedDataLength,
                                NULL,
                                NULL,
                                &Data,
                                &DataLength
                                );
    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }
    
    Status = NCryptGetProtectionDescriptorInfo(
                                        DescriptorHandle,
                                        NULL,
                                        NCRYPT_PROTECTION_INFO_TYPE_DESCRIPTOR_STRING,
                                        (void**)&DescriptorString
                                        );
    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }

    wprintf( L"Descriptor String constructed from the blob:\r\n    %ls\r\n", DescriptorString );

    Status = ERROR_SUCCESS;

cleanup:

    if( NULL != Data )
    {
        LocalFree( Data );
    }

    if( NULL != DescriptorHandle )
    {
        NCryptCloseProtectionDescriptor( DescriptorHandle );
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
    SECURITY_STATUS         Status;

    PBYTE                   Secret = NULL;
    DWORD                   SecretLength = 0;
    PBYTE                   ProtectedSecret = NULL;
    DWORD                   ProtectedSecretLength = 0;
    PBYTE                   UnprotectedSecret = NULL;
    DWORD                   UnprotectedSecretLength = 0;

    //
    // Initialize secret to protect
    //

    Secret = (PBYTE)SecretString;
    SecretLength = (ULONG)( (wcslen(SecretString)+1)*sizeof(WCHAR) );

    //
    // Protect Secret
    //

    Status = ProtectSecret(
                        ProtectionDescriptorString,
                        Secret,
                        SecretLength,
                        &ProtectedSecret,
                        &ProtectedSecretLength
                        );

    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }

    if( NULL == ProtectedSecret )
    {
        Status = NTE_INTERNAL_ERROR;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Open encrypted data to get descriptor information
    //

    Status = GetProtectionInfo( 
                            ProtectedSecret,
                            ProtectedSecretLength
                            );

    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }


    //
    // Unprotect Secret
    //

    Status = UnprotectSecret(
                        ProtectedSecret,
                        ProtectedSecretLength,
                        &UnprotectedSecret,
                        &UnprotectedSecretLength
                        );

    if( ERROR_SUCCESS != Status )
    {
        ReportError(Status);
        goto cleanup;
    }

    if( NULL == UnprotectedSecret )
    {
        Status = NTE_INTERNAL_ERROR;
        ReportError(Status);
        goto cleanup;
    }

    //
    // Optional : Check if the original message and the message obtained after decrypt are the same 
    //

    if( SecretLength != UnprotectedSecretLength ||
        0 != (memcmp(Secret, UnprotectedSecret, SecretLength)) )
    {
        Status = NTE_FAIL;
        ReportError(Status);
        goto cleanup;
    }

    Status = ERROR_SUCCESS;
      
    wprintf(L"Success!\n");

cleanup:
    
    //
    // Release allocated resources
    //

    if( NULL != ProtectedSecret )
    {
        LocalFree( ProtectedSecret );
    }

    if( NULL != UnprotectedSecret )
    {
        LocalFree( UnprotectedSecret );
    }

    return (DWORD)Status;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
}
