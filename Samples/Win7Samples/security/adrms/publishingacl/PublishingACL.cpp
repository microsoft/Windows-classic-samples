/*=====================================================================
File:      PublishingACL.cpp

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
=====================================================================*/

//
// This sample shows how to create an issuance license. See the 
// comments at the beginning of wmain() for a more detailed description.
// Note: To retrieve an object's DACL, you must be the object's owner or 
// have READ_CONTROL access to the object. To run this sample, you must 
// have READ_CONTROL on the path you provide on the command line.
//

#define _WIN32_DCOM
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <wtypes.h>
#include <strsafe.h>
#include <stdio.h>
#include <windows.h>
#include <sddl.h>
#include <ctype.h>
#include <Wbemidl.h>
#include <comdef.h>
#include <objbase.h>
#include <oleauto.h>
#include <comutil.h>

#include "msdrm.h"
#include "msdrmdefs.h"
#include "msdrmerror.h"

#pragma comment( lib, "wbemuuid.lib" )

//
// Time to wait for "downloads" to complete
//
static const DWORD DW_WAIT_TIME_SECONDS = 60 * 1000;  

//
// Length of a GUID string
//
static const UINT GUID_STRING_LENGTH = 128;

//
// Define the two rights that are used in this sample
//
#define EDIT_RIGHT L"EDIT"
#define VIEW_RIGHT L"VIEW"

//
// Define the file to save the template into
//
#define TEMPLATE_FILE L".\\ILTemplate.xml"

//
// Define the known domains to exclude from the issuance license
//
#define NT_AUTHORITY L"NT AUTHORITY"
#define BUILTIN      L"BUILTIN"

//
// Print the correct usage of this application
//
void 
PrintUsage()
{
    wprintf( L"Usage:\n" );
    wprintf( L"\n  PublishingACL -P Path\n" );
    wprintf( L"    -P: Path to the directory to use for the "\
        L"issuance license's rights.\n" );
    wprintf( L"        example: c:\\myDir\n" );
}

//
// Parse the values passed in through the command line
//
HRESULT 
ParseCommandLine( 
                 int argc, 
                 __in_ecount( argc )WCHAR **argv, 
                 __deref_out PWCHAR *pwszPath
                 )
{
    HRESULT hr = S_OK;
    size_t uiPathLength = 0;

    //
    // Check input parameters.
    //
    if ( ( 3 != argc ) )
    {
        return E_INVALIDARG;
    }

    for( int i = 1; SUCCEEDED( hr ) && i < argc - 1; i ++ )
    {
        if ( ( '-' != argv[ i ][ 0 ] ) && ( '/' != argv[ i ][ 0 ] ) )
        {
            hr = E_INVALIDARG;
            break;
        }
        else if ( ( '-' == argv[ i + 1 ][ 0 ] ) || 
            ( '/' == argv[ i + 1 ][ 0 ] ) )
        {
            hr = E_INVALIDARG;
            break;
        }

        //
        // Path to directory for publishing license rights
        //
        if ( 'P' == toupper( argv[ i ][ 1 ] ) )
        {
            //
            // Retrieve the length of the path
            //
            hr = StringCchLengthW( argv[ i + 1 ], 
                STRSAFE_MAX_CCH, 
                &uiPathLength 
                );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the path
            //
            *pwszPath = new WCHAR[ uiPathLength + 1 ];
            if ( NULL == *pwszPath ) 
            {
                wprintf( L"Failed to allocate memory for pwszPath\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the URL into the pwszPath buffer
            //
            hr = StringCchCopyW( ( wchar_t* )*pwszPath, 
                uiPathLength + 1, 
                argv[ i + 1 ] 
                );
                if ( FAILED( hr ) )
                {
                    wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
                i++;
        }
        else
        {
            hr = E_INVALIDARG;
            break;
        }
    }
    if ( NULL == pwszPath )
    {
        wprintf( L"A path is required.\n" );
        hr = E_INVALIDARG;
    }
    return hr;
}

//
// Write the template string to a unicode file
//
HRESULT
WriteTemplateToUnicodeFile(
                           __in LPCWSTR szFileName,
                           __in PWSTR wszTemplateString
                           )
{
    HRESULT hr              = E_FAIL;
    HANDLE  hFile           = INVALID_HANDLE_VALUE;
    size_t  cchTemplate     = 0;
    DWORD   dwBytesWritten  = 0;

    //
    // Validate the parameters
    //
    if ( ( NULL == szFileName ) || 
        ( FAILED( StringCchLengthW( ( LPCWSTR )szFileName, MAX_PATH, NULL ) ) ) ||
        ( NULL == wszTemplateString ) )
    {
        hr = E_INVALIDARG;
        goto e_Exit;
    }

    hr = StringCchLengthW( wszTemplateString, STRSAFE_MAX_CCH, &cchTemplate );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nStringCchLengthW failed.  hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // Create a handle to the file to write to
    //
    hFile = CreateFileW( szFileName, 
        GENERIC_WRITE, 
        0, 
        NULL, 
        CREATE_ALWAYS, 
        0, 
        NULL 
        );
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        wprintf( L"\nCreateFileW failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // Write the template string to the file
    //
    if ( !WriteFile( hFile, 
        wszTemplateString, 
        ( DWORD )( sizeof( wchar_t ) * cchTemplate), 
        &dwBytesWritten,
        NULL
        ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        wprintf( L"\nWriteFile failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }


e_Exit:
    if ( ( NULL != hFile ) && ( INVALID_HANDLE_VALUE != hFile ) )
    {
        CloseHandle( hFile );
    }
    return hr;
}

//
// Determine if the machine the sample is running on is a domain controller
//
bool
IsDomainController()
{
    HRESULT               hr;
    bool                  result      = false;
    IWbemLocator         *pLocator    = NULL;
    IWbemServices        *pSvc        = NULL;
    IEnumWbemClassObject *pEnumerator = NULL;
    IWbemClassObject     *pclsObj     = NULL;
    ULONG                 uReturn     = 0;
    VARIANT               vtProp;

    //
    // Initialize COM
    //
    hr = CoInitializeEx( 0, COINIT_MULTITHREADED );
    if ( FAILED( hr ) )
    {
        wprintf( L"CoInitializeEx failed. hr = 0x%x\n", hr );
        result = false;
        goto e_Exit;
    }

    //
    // Set general COM security levels.
    // NOTE: If you are using Windows 2000, you need to specify the default
    // authentication credentials for a user by using a 
    // SOLE_AUTHENTICATION_LIST structure in the pAuthList parameter
    //
    hr = CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );
    if ( FAILED( hr ) )
    {
        wprintf( L"CoInitializeSecurity failed. hr = 0x%x\n", hr );
        result = false;
        goto e_Exit;
    }

    //
    // Obtain the initial locator to WMI
    //
    hr = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, ( LPVOID* )&pLocator );
    if ( FAILED( hr ) )
    {
        wprintf( L"CoCreateInstance failed. hr = 0x%x\n", hr );
        result = false;
        goto e_Exit;
    }

    //
    // Connect to WMI through the IWbemLocator::ConnectServer method
    // 
    // Connect to the root\cimv2 namespace with the current user and obtain
    // pointer pSvc to make IWbemServices calls.
    //
    if ( NULL == pLocator )
    {
        wprintf( L"pLocator was NULL\n" );
        goto e_Exit;
    }

    hr = pLocator->ConnectServer( _bstr_t( L"ROOT\\CIMV2" ), NULL, NULL, 0,
        NULL, 0, 0, &pSvc );
    if ( FAILED( hr ) )
    {
        wprintf( L"ConnectServer failed. hr = 0x%x\n", hr );
        result = false;
        goto e_Exit;
    }

    //
    // Set security levels on the proxy
    //
    hr = CoSetProxyBlanket( pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE );
    if ( FAILED( hr ) )
    {
        wprintf( L"CoSetProxyBlanket failed. hr = 0x%x\n", hr );
        result = false;
        goto e_Exit;
    }

    //
    // Use the IWbemServices pointer to make requests of WMI
    //
    hr = pSvc->ExecQuery( _bstr_t( L"WQL" ), 
        _bstr_t( L"SELECT * FROM Win32_ComputerSystem" ),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"ExecQuery failed. hr = 0x%x\n", hr );
        result = false;
        goto e_Exit;
    }

    //
    // Use the data from the query to determine if the machine
    // is a domain controller
    //
    while ( pEnumerator )
    {
        hr = pEnumerator->Next( WBEM_INFINITE, 1, &pclsObj, &uReturn );
        if ( FAILED( hr ) )
        {
            wprintf( L"Next failed. hr = 0x%x\n", hr );
            result = false;
            goto e_Exit;
        }

        if ( 0 == uReturn )
        {
            break;
        }

        VariantInit( &vtProp );

        //
        // Get the value of the DomainRole property
        //
        hr = pclsObj->Get( L"DomainRole", 0, &vtProp, 0, 0);
        if ( FAILED( hr ) )
        {
            wprintf( L"Get failed. hr = 0x%x\n", hr );
            result = false;
            goto e_Exit;
        }

        if ( ( vtProp.lVal == 4 ) || ( vtProp.lVal == 5 ) )
        {
            result = true;
        }
    }

e_Exit:
    VariantClear( &vtProp );
    if ( pSvc )
    {
        pSvc->Release();
    }
    if ( pLocator )
    {
        pLocator->Release();
    }
    CoUninitialize();
    return result;
}

//
// This sample will perform the following actions:
//    1.  Create an unsigned issuance license from scratch
//    2.  Determine what the ACLs are on the directory
//        passed in as the command line argument
//    3.  For each ACL:
//        (a) Get the user name for the ACL
//        (b) Determine if the user should be added to the issuance
//            license
//        (c) Get the SID in the form of a string
//        (d) Get the right name for the ACL
//        (e) Create a user based on the ACL
//        (f) Create a right based on the ACL
//        (g) Add the user and right to the unsigned issuance license
//    4. Set the metadata in the issuance license
//    5. Generate a template from the issuance license
//    6.  Clean up and free memory
//
int __cdecl 
wmain( 
      int argc, 
      __in_ecount( argc )WCHAR **argv 
      )
{
    HRESULT                   hr                 = E_FAIL;
    int                       iDaclPresent, iDaclDefault, iMask;
    int                       count              = 0;
    size_t                    uiRightLength      = 0;
    DWORD                     dwSize;
    DWORD                     pdwNameLength      = 0;
    DWORD                     pdwDomainLength    = 0;
    DWORD                     dwMachNameLength   = 0;
    PWCHAR                    wszTemplate        = NULL;
    PWCHAR                    wszRight           = NULL;
    PWCHAR                    wszPath            = NULL;
    PWCHAR                    wszSid             = NULL;
    PWCHAR                    wszName            = NULL;
    PWCHAR                    wszDomain          = NULL;
    PWCHAR                    wszMachineName     = NULL;
    ACL                      *pDacl              = NULL;
    ACL_SIZE_INFORMATION      asi;
    SID_NAME_USE              sidNameUse;
    SECURITY_DESCRIPTOR      *pSD                = NULL;
    ACCESS_ALLOWED_ACE       *pAccessAllowed     = NULL;
    SID                      *pSid               = NULL;
    void                     *pAce               = NULL;
    SYSTEMTIME                stTimeFrom, stTimeUntil;
    DRMPUBHANDLE              hIssuanceLicense   = NULL;
    DRMPUBHANDLE              hUser              = NULL;
    DRMPUBHANDLE              hRight             = NULL;
    UINT                      uiTemplateLength   = 0;
    GUID                      guid;
    PWSTR                     wszGUID            = NULL;
    UINT                      uiGUIDLength;
    WCHAR                    *p                  = NULL;

    if ( FAILED ( ParseCommandLine( argc, argv, &wszPath ) ) )
    {
        PrintUsage();
        goto e_Exit;
    }

    wprintf( L"\nRunning sample PublishingACL...\n" );

    //
    // Get the system time for the starting and ending validity times
    // in the unsigned issuance license
    //
    GetSystemTime( &stTimeFrom );
    GetSystemTime( &stTimeUntil );
    
    FILETIME ft = { 0 };
    SystemTimeToFileTime(&stTimeUntil, &ft);

    ULARGE_INTEGER u = { 0 };
    memcpy(&u, &ft, sizeof(u));
    u.QuadPart += 90 * 24 * 60 * 60 * 10000000LLU;  // 90 days
    memcpy(&ft, &u, sizeof(ft));

    FileTimeToSystemTime(&ft, &stTimeUntil);

    //
    // 1. Create an unsigned issuance license from scratch
    //
    hr = DRMCreateIssuanceLicense( &stTimeFrom,
        &stTimeUntil,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &hIssuanceLicense 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    //  2.  Determine what the ACLs are on the directory passed in as 
    //      the command line argument

    //
    // Get the DACL information from the path from the command line argument.
    //
    if ( GetFileSecurityW( wszPath, 
        DACL_SECURITY_INFORMATION, 
        NULL, 
        NULL, 
        &dwSize 
        ) )
    {
        wprintf( L"GetFileSecurityW should not have returned true\n" );
        goto e_Exit;
    }
    if ( ERROR_INSUFFICIENT_BUFFER != GetLastError() )
    {
        wprintf( L"GetFileSecurityW failed. Error: %d\n", GetLastError() );
        goto e_Exit;
    }

    pSD = ( SECURITY_DESCRIPTOR* )new BYTE[ dwSize ];
    if ( NULL == pSD )
    {
        hr = E_OUTOFMEMORY;
        wprintf( L"Failed to allocate memory for the "\
            L"SECURITY_DESCRIPTOR: hr = 0x%x\n", hr );
        goto e_Exit;
    }

    if ( !GetFileSecurityW( wszPath, 
        DACL_SECURITY_INFORMATION, 
        pSD, 
        dwSize, 
        &dwSize ) )
    {
        wprintf( L"GetFileSecurityW failed.  Error: %d\n", GetLastError() );
        goto e_Exit;
    }

    //
    // Get the Dacl from the SECURITY_DESCRIPTOR
    //
    if ( !GetSecurityDescriptorDacl( pSD, 
        &iDaclPresent, 
        &pDacl, 
        &iDaclDefault ) )
    {
        wprintf( L"GetSecurityDescriptorDacl failed.  "\
            L"Error: %d\n", GetLastError() );
        goto e_Exit;
    }

    // 
    // If there was no Dacl present, then fail since no user/right 
    // information will be put into the issuance license
    //
    if ( !iDaclPresent )
    {
        wprintf( L"There was no DACL present in the security descriptor\n" );
        hr = E_FAIL;
        goto e_Exit;
    }

    // 
    // Get Acl information from the Dacl
    //
    if ( !GetAclInformation( pDacl, 
        ( void* ) &asi, 
        sizeof( asi ), 
        AclSizeInformation ) )
    {
        wprintf( L"GetAclInformation failed.  Error: %d\n", GetLastError() );
        goto e_Exit;
    }

    //
    // Get the machine name
    //
    if ( !GetComputerNameExW( ComputerNamePhysicalDnsHostname,
        NULL,
        &dwMachNameLength
        ) )
    {
        if ( ERROR_MORE_DATA != GetLastError() )
        {
            wprintf( L"GetComputerNameExW failed. "\
                L"Error = %d\n", GetLastError() );
            goto e_Exit;
        }
        wszMachineName = new WCHAR[ dwMachNameLength ];
        if ( NULL == *wszMachineName )
        {
            wprintf( L"Failed to allocate memory for the machine name.\n" );
            goto e_Exit;
        }
    }

    if ( !GetComputerNameExW( ComputerNamePhysicalDnsHostname,
        wszMachineName,
        &dwMachNameLength
        ) )
    {
        wprintf( L"GetComputerNameExW failed. "\
            L"Error = %d\n", GetLastError() );
        goto e_Exit;
    }

    //
    // Make the machine name all upper case
    //
    for ( p = wszMachineName; p < wszMachineName + dwMachNameLength; p++ )
    {
        if ( iswlower( *p ) )
        {
            *p = towupper( *p );
        }
    }

    //
    //    3.  For each ACL:
    //

    //
    // Parse each Acl and create the appropriate user and 
    // right for the issuance license
    //
    for ( int i = 0; i < ( int )asi.AceCount; i++ )
    {
        pdwDomainLength = pdwNameLength = 0;
        pAccessAllowed = NULL;
        pSid = NULL;
        pAce = NULL;
        uiRightLength = 0;
        iMask = 0;

        if ( !GetAce( pDacl, i, &pAce ) )
        {
            wprintf( L"GetAce failed.  Error: %d\n", GetLastError() );
            goto e_Exit;
        }

        if ( !pAce )
        {
            wprintf( L"Invalid ACE.  Continuing...\n" );
            continue;
        }
        //
        // If the Ace was an ACCESS_ALLOWED_ACE_TYPE
        //
        if ( ( ( ACCESS_ALLOWED_ACE* )pAce )->Header.AceType == 
            ACCESS_ALLOWED_ACE_TYPE )
        {
            pAccessAllowed = ( ACCESS_ALLOWED_ACE* )pAce;
            pSid = ( SID* ) &pAccessAllowed->SidStart;
            iMask = pAccessAllowed->Mask;

            //
            // Verify that we have a valid SID
            //
            if ( IsValidSid( pSid ) )
            {
                count++;

                //
                // 3(a). Get the user name for the ACL
                //

                //
                // Lookup the account name and domain associated with the SID
                //
                if ( LookupAccountSidW( 0, 
                    pSid, 
                    ( wchar_t* )wszName, 
                    &pdwNameLength, 
                    ( wchar_t* )wszDomain, 
                    &pdwDomainLength, 
                    &sidNameUse ) )
                {
                    wprintf( L"LookupAccountSidW passed when it "\
                        L"should have failed." );
                    goto e_Exit;
                }
                if ( ERROR_INSUFFICIENT_BUFFER != GetLastError() )
                {
                    if ( ERROR_NONE_MAPPED == GetLastError() )
                    {
                        //
                        // Skip this SID since it cannot be resolved
                        //
                        continue;
                    }
                    wprintf( L"LookUpAccountSidW failed. "\
                        L"Error: %d\n", GetLastError() );
                    goto e_Exit;
                }
                wszName = new WCHAR[ pdwNameLength ];
                wszDomain = new WCHAR[ pdwDomainLength ];

                if ( NULL == wszName || NULL == wszDomain )
                {
                    wprintf( L"Memory allocation failed for wszName "\
                        L"or wszDomain\n" );
                    goto e_Exit;
                }

                if ( !LookupAccountSidW( 0, 
                    pSid, 
                    ( wchar_t* )wszName, 
                    &pdwNameLength, 
                    ( wchar_t* )wszDomain,
                    &pdwDomainLength, 
                    &sidNameUse ) )
                {
                    wprintf( L"LookupAccountSidW failed.  "\
                        L"Error: %d\n", GetLastError() );
                    goto e_Exit;
                }

                //
                // 3(b) Determine if the user should be added to the issuance
                //     license. If the SID is not a SID we want to put into 
                //     the issuance license, then clean up memory and 
                //     continue through the for loop
                //
                if ( ( wcscmp( ( wchar_t* )wszDomain, NT_AUTHORITY ) == 0 ) ||
                    ( wcscmp( ( wchar_t* )wszDomain, BUILTIN ) == 0 ) ||
                    ( wcscmp( ( wchar_t* )wszDomain, L"" ) == 0 ) ||
					( wcscmp( ( wchar_t* )wszDomain, L"NT SERVICE" ) == 0 ) ||
                    ( ( !IsDomainController() && 
                    wcscmp( ( wchar_t* )wszDomain, ( wchar_t* )wszMachineName ) == 0 ) ) )
                {
                    if ( NULL != wszName )
                    {
                        delete [] wszName;
                        wszName = NULL;
                    }
                    if ( NULL != wszDomain )
                    {
                        delete [] wszDomain;
                        wszDomain = NULL;
                    }
                    continue;
                }

                //
                // 3(c). Get the SID in the form of a string
                //
                if ( !ConvertSidToStringSidW( pSid, &wszSid ) )
                {
                    wprintf( L"ConvertSidToStringSidW failed.  "\
                        L"Error: %d\n", GetLastError() );
                    goto e_Exit;
                }

                //
                // 3(d). Get the right name for the ACL
                //

                //
                // If the access allowed is a "write" access, then that will 
                // be considered an EDIT_RIGHT for the purposes of this sample
                //
                if ( iMask & FILE_SHARE_WRITE )
                {
                    hr = StringCchLengthW( EDIT_RIGHT, 
                        STRSAFE_MAX_CCH, 
                        &uiRightLength );
                    if ( FAILED( hr ) )
                    {
                        wprintf( L"StringCchLengthW failed.  "\
                            L"hr = 0x%x\n", hr );
                        goto e_Exit;
                    }

                    //
                    // Allocate memory for the right
                    //
                    wszRight = new WCHAR[ uiRightLength + 1 ];
                    if ( NULL == wszRight ) 
                    {
                        wprintf( L"Failed to allocate memory for "\
                            L"wszRight.\n" );
                        hr = E_OUTOFMEMORY;
                        goto e_Exit;
                    }

                    hr = StringCchCopyW( ( wchar_t* )wszRight, 
                        uiRightLength + 1, 
                        EDIT_RIGHT );
                    if ( FAILED( hr ) )
                    {
                        wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                        break;
                    }
                }
                //
                // If the access allowed is a "read" access, then that will be
                // considered a VIEW_RIGHT for the purposes of this sample
                //
                else if ( iMask & FILE_SHARE_READ )
                {
                    hr = StringCchLengthW( VIEW_RIGHT, 
                        STRSAFE_MAX_CCH, 
                        &uiRightLength );
                    if ( FAILED( hr ) )
                    {
                        wprintf( L"StringCchLengthW failed.  "\
                            L"hr = 0x%x\n", hr );
                        goto e_Exit;
                    }

                    //
                    // Allocate memory for the right
                    //
                    wszRight = new WCHAR[ uiRightLength + 1 ];
                    if ( NULL == wszRight ) 
                    {
                        wprintf( L"Failed to allocate memory "\
                            L"for wszRight.\n" );
                        hr = E_OUTOFMEMORY;
                        goto e_Exit;
                    }

                    hr = StringCchCopyW( ( wchar_t* )wszRight, 
                        uiRightLength + 1, 
                        VIEW_RIGHT );
                    if ( FAILED( hr ) )
                    {
                        wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                        break;
                    }
                }
                else
                {
                    wprintf( L"An unknown right was encountered. "\
                        L"Continuing...\n" );
                    if ( NULL != wszName )
                    {
                        delete [] wszName;
                        wszName = NULL;
                    }
                    if ( NULL != wszDomain )
                    {
                        delete [] wszDomain;
                        wszDomain = NULL;
                    }
                    continue;
                }
            }
        }

        //
        // 3(e). Create a user based on the ACL
        //
        hr = DRMCreateUser( wszName, wszSid, L"Windows", &hUser );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMCreateUser failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }

        //
        // 3(f). Create a right based on the ACL
        //
        hr = DRMCreateRight( wszRight, 
            &stTimeFrom, 
            &stTimeUntil, 
            NULL, 
            NULL, 
            NULL, 
            &hRight );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMCreateRight failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }

        //
        // 3(g). Add the user and right to the unsigned issuance license
        //
        hr = DRMAddRightWithUser( hIssuanceLicense, hRight, hUser );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMAddRightWithUser failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }

        //
        // Clean up the memory and variables before 
        // continuing through the loop
        //
        if ( NULL != wszName )
        {
            delete [] wszName;
            wszName = NULL;
        }
        if ( NULL != wszDomain )
        {
            delete [] wszDomain;
            wszDomain = NULL;
        }
        if ( NULL != wszRight )
        {
            delete [] wszRight;
            wszRight = NULL;
        }
        if ( NULL != wszSid )
        {
            LocalFree( wszSid );
            wszSid = NULL;
        }
        if ( NULL != hUser )
        {
            hr = DRMClosePubHandle( hUser );
            if ( FAILED( hr ) )
            {
                wprintf( L"DRMClosePubHandle failed while closing hUser.  "\
                    L"hr = 0x%x\n", hr );
                hr = E_FAIL;
            }
            hUser = NULL;
        }
        if ( NULL != hRight )
        {
            hr = DRMClosePubHandle( hRight );
            if ( FAILED( hr ) )
            {
                wprintf( L"DRMClosePubHandle failed while closing hRight.  "\
                    L"hr = 0x%x\n", hr );
                hr = E_FAIL;
            }
            hRight = NULL;
        }
    }

    //
    // Create a GUID to use as a unique content ID
    // in the issuance license
    //
    hr = CoCreateGuid( &guid );
    if ( FAILED( hr ) )
    {
        wprintf( L"CoCreateGuid failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wszGUID = new WCHAR[ GUID_STRING_LENGTH ];
    if ( NULL == wszGUID )
    {
        wprintf( L"Failed to allocate memory for wszGUID\n" );
        goto e_Exit;
    }

    uiGUIDLength = StringFromGUID2( guid, wszGUID, GUID_STRING_LENGTH );
    if ( 0 == uiGUIDLength )
    {
        wprintf( L"StringFromGUID2 failed.\n" );
        hr = E_FAIL;
        goto e_Exit;
    }

    //
    // 4. Set the metadata in the issuance license
    //
    hr = DRMSetMetaData( hIssuanceLicense, 
        wszGUID, 
        L"MS-GUID", 
        NULL, 
        NULL, 
        NULL, 
        NULL 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"DRMSetMetaData failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    // 
    // If there were no valid SIDs, then fail since no users/rights will be 
    // added to the issuance license
    //
    if ( 0 == count )
    {
        wprintf( L"No valid SIDs were found for the provided directory.\n" );
        hr = E_FAIL;
        goto e_Exit;
    }

    //
    // 5. Generate a template from the issuance license
    //
    hr = DRMGetIssuanceLicenseTemplate( hIssuanceLicense, 
        &uiTemplateLength, 
        NULL 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMGetIssuanceLicenseTemplate failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wszTemplate = new WCHAR[ uiTemplateLength ];
    if ( NULL == wszTemplate )
    {
        wprintf( L"\nMemory allocation failed for the template.\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }


    hr = DRMGetIssuanceLicenseTemplate( hIssuanceLicense, 
        &uiTemplateLength, 
        wszTemplate 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMGetIssuanceLicenseTemplate failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // Write the template to a file
    //
    hr = WriteTemplateToUnicodeFile( TEMPLATE_FILE, wszTemplate );
    if ( FAILED( hr ) )
    {
        wprintf( L"WriteTemplateToUnicodeFile failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wprintf( L"PublishingACL succeeded.\n" );


e_Exit:
    //
    // 6. Clean up and free memory
    //
    if ( NULL != pSD )
    {
        delete [] pSD;
    }
    if ( NULL != wszName )
    {
        delete [] wszName;
    }
    if ( NULL != wszDomain )
    {
        delete [] wszDomain;
    }
    if ( NULL != wszRight )
    {
        delete [] wszRight;
    }
    if ( NULL != wszSid )
    {
        LocalFree( wszSid );
        wszSid = NULL;
    }
    if ( NULL != wszPath )
    {
        delete [] wszPath;
    }
    if ( NULL != wszMachineName )
    {
        delete [] wszMachineName;
    }
    if ( NULL != wszTemplate )
    {
        delete [] wszTemplate;
    }
    if ( NULL != hIssuanceLicense )
    {
        hr = DRMClosePubHandle( hIssuanceLicense );
        if ( FAILED( hr ) )
        {
            wprintf( L"DRMClosePubHandle failed while closing "\
                L"hIssuanceLicense.  hr = 0x%x\n", hr );
            hr = E_FAIL;
        }
    }
    if ( NULL != hUser )
    {
        hr = DRMClosePubHandle( hUser );
        if ( FAILED( hr ) )
        {
            wprintf( L"DRMClosePubHandle failed while closing hUser.  "\
                L"hr = 0x%x\n", hr );
            hr = E_FAIL;
        }
    }
    if ( NULL != hRight )
    {
        hr = DRMClosePubHandle( hRight );
        if ( FAILED( hr ) )
        {
            wprintf( L"DRMClosePubHandle failed while closing hRight.  "\
                L"hr = 0x%x\n", hr );
            hr = E_FAIL;
        }
    }
    if ( FAILED( hr ) )
    {
        wprintf( L"\nPublishingACL failed. hr = 0x%x\n", hr );
    }
    return hr;
}