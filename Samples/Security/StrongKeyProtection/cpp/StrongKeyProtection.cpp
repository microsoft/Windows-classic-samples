// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
//
//  File:             StrongKeyProtection.cpp
//
//  Contents:         This sample shows how to set the UX properties for a strong key in CNG.
//                    The key is used to sign a hash value.
//
//    
//

#include <windows.h>
#include <winerror.h>
#include <stdio.h>
#include <bcrypt.h>
#include <ncrypt.h>
#include <wincrypt.h>
#include <sal.h>

static const 
BYTE Hash[20] = 
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
};

//
// Pin value to be entered while creating the key
// Instead of entering the pin again while using the key, the pin value will be set as a property on the key handle
//

static 
LPCWSTR Pin = L"Password123";

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
    SECURITY_STATUS         secStatus;
    
    NCRYPT_PROV_HANDLE      ProviderHandle = 0;
    NCRYPT_KEY_HANDLE       KeyHandle = 0;
    NCRYPT_UI_POLICY        UIPolicy = {0};
    
    LPCWSTR                 KeyName = L"SampleStrongKey";
    PBYTE                   Signature = NULL;
    DWORD                   SignatureLength = 0;
    DWORD                   ResultLength = 0;
    
    BCRYPT_PKCS1_PADDING_INFO PKCS1PaddingInfo;
    VOID                    *pPaddingInfo;
    HWND                    hwndConsole = NULL;

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
    // Create an RSA key signature key in MS KSP.
    // NCRYPT_OVERWRITE_KEY_FLAG - is used to overwrite an existing key with the provided name.
    //

    secStatus = NCryptCreatePersistedKey(
                                        ProviderHandle,             // Handle of the key storage provider
                                        &KeyHandle,                 // Address of the variable that recieves the key handle
                                        NCRYPT_RSA_ALGORITHM,       // Algorithm name (null terminated unicode string)
                                        KeyName,                    // Key name (null terminated unicode string)
                                        AT_SIGNATURE,               // Legacy identifier (AT_KEYEXCHANGE, AT_SIGNATURE or 0 )
                                        NCRYPT_OVERWRITE_KEY_FLAG); // Flags; If a key already exists in the container with the specified name, the existing key will be overwritten.
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Set the UI policy on this key, before finalizing the key.
    // Once the key pair generation is finalized, the level of protection
    // (in UIPolicy.dwFlags) cannot be changed.
    //

    ZeroMemory(
        &UIPolicy, 
        sizeof(UIPolicy));

    UIPolicy.dwVersion = 1;
    UIPolicy.dwFlags = NCRYPT_UI_FORCE_HIGH_PROTECTION_FLAG;
    
    UIPolicy.pszCreationTitle   = L"Strong Key UX Sample";
    UIPolicy.pszFriendlyName    = L"Sample Friendly Name";
    UIPolicy.pszDescription = L"This is a sample strong key";

    secStatus = NCryptSetProperty(
                                        KeyHandle,                  // Handle of the key storage object 
                                        NCRYPT_UI_POLICY_PROPERTY,  // Property name (null terminated unicode string)
                                        (PBYTE)&UIPolicy,           // Address of the buffer that contains the property value
                                        sizeof(UIPolicy),           // Size of the buffer in bytes
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Get a handle to the console window
    //

    hwndConsole = GetDesktopWindow();
    if( NULL == hwndConsole )
    {
        secStatus = NTE_INVALID_HANDLE;
        ReportError(secStatus);
        goto cleanup;
    }

    
    //
    // Attach the window handle to the key
    // 

    secStatus = NCryptSetProperty(
                                        KeyHandle,                  // Handle of the key storage object 
                                        NCRYPT_WINDOW_HANDLE_PROPERTY,  // Property name (null terminated unicode string)
                                        (PBYTE)&hwndConsole,        // Address of the buffer that contains the property value
                                        sizeof(hwndConsole),        // Size of the buffer in bytes
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
                                        KeyHandle,                  // Handle of the key - that has to be finalized
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Delete this object
    //

    secStatus = NCryptFreeObject(KeyHandle);                           // The handle of the object to free
	if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Using the strong key for signing a hash
    // Intent from here on is: to show the strong key UX as per its policy set above.
    //

    //
    // Get a handle to key in the provider (KSP).
    //

    secStatus = NCryptOpenKey(
                                        ProviderHandle,             // Handle of the key storage provider
                                        &KeyHandle,                 // Key handle
                                        KeyName,                    // Key name (null terminated unicode string)
                                        AT_SIGNATURE,               // Legacy identifier
                                        0);                         // Flags

    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Set the pin property on the key - programmatically
    //

    secStatus = NCryptSetProperty(
                                        KeyHandle,                  // Handle of the key storage object 
                                        NCRYPT_PIN_PROPERTY,        // Property name (null terminated unicode string)
                                        (PBYTE)Pin,                 // Address of the buffer that contains the property value
                                        (ULONG)wcslen(Pin)*sizeof(WCHAR),   // Size of the buffer in bytes
                                        0);                         // Flags
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    PKCS1PaddingInfo.pszAlgId = NCRYPT_SHA1_ALGORITHM;
    pPaddingInfo = &PKCS1PaddingInfo;

    //
    // Determing the required output length
    //

    secStatus = NCryptSignHash(
                                        KeyHandle,                  // Handle of the key
                                        pPaddingInfo,               // Padding information
                                        (PBYTE)Hash,                // Buffer that contains the hash value to be signed
                                        sizeof(Hash),               // Size of the buffer in bytes
                                        NULL,                       // Buffer that recieves the signature 
                                        0,                          // Size of the buffer in bytes
                                        &SignatureLength,           // Number of bytes copied into the Signatue buffer
                                        NCRYPT_PAD_PKCS1_FLAG);     // Flags;Use the PKCS1 padding scheme
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Allocate memory on the heap
    //

    Signature = (PBYTE)HeapAlloc (GetProcessHeap(), 0, SignatureLength);
    if( NULL == Signature )
    {
        secStatus = NTE_NO_MEMORY;
        ReportError(secStatus);
        goto cleanup;
    }

    //
    // Call the signature function again to sign the hash data
    // and to get the signature blob
    //

    secStatus = NCryptSignHash(
                                        KeyHandle,                  // Handle of the key
                                        pPaddingInfo,               // Padding information
                                        (PBYTE)Hash,                // Buffer that contains the hash value to be signed
                                        sizeof(Hash),               // Size of the buffer in bytes
                                        Signature,                  // Buffer that recieves the signature 
                                        SignatureLength,            // Size of the buffer in bytes
                                        &ResultLength,              // Number of bytes copied into the Signatue buffer
                                        NCRYPT_PAD_PKCS1_FLAG);     // Flags;Use the PKCS1 padding scheme
    if( FAILED(secStatus) )
    {
        ReportError(secStatus);
        goto cleanup;
    }

    wprintf(L"Success!\n");

cleanup:

    if( NULL != Signature )
    {
        HeapFree(GetProcessHeap(), 0, Signature);
        Signature = NULL;
    }

    if( NULL != KeyHandle )
    {
        NCryptDeleteKey(KeyHandle, 0);
        KeyHandle = 0;
    }

     if( NULL != ProviderHandle )
    {
        NCryptFreeObject(ProviderHandle);
        ProviderHandle = 0;
    } 
       
    return (DWORD)secStatus;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
}



