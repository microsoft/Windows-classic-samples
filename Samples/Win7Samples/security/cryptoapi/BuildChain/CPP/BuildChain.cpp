// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

This sample code shows how to validate a certificate by building 
and verifying the certificate chain.

****************************************************************/

#include <windows.h>
#include <winerror.h>
#include <strsafe.h>
#include <wincrypt.h>
#include <stdio.h>

/*****************************************************************************
 ReportError

	Prints error information to the console
*****************************************************************************/
void 
ReportError( 
    LPCWSTR     wszMessage, 
    DWORD       dwErrCode 
    )
{
	LPWSTR pwszMsgBuf = NULL;

	if( NULL!=wszMessage && 0!=*wszMessage )
    {
        wprintf( L"%s\n", wszMessage );
    }

	FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,                                       // Location of message
                                                    //  definition ignored
        dwErrCode,                                  // Message identifier for
                                                    //  the requested message    
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Language identifier for
                                                    //  the requested message
        (LPWSTR) &pwszMsgBuf,                       // Buffer that receives
                                                    //  the formatted message
        0,                                          // Size of output buffer
                                                    //  not needed as allocate
                                                    //  buffer flag is set
        NULL                                        // Array of insert values
		);
	
	if( NULL != pwszMsgBuf )
	{
	    wprintf( L"Error: 0x%08x (%d) %s\n", dwErrCode, dwErrCode, pwszMsgBuf );
		LocalFree(pwszMsgBuf);
	}
	else
	{
	    wprintf( L"Error: 0x%08x (%d)\n", dwErrCode, dwErrCode );
	}
}

//----------------------------------------------------------------------------
// HrFindCertificateBySubjectName
//
//----------------------------------------------------------------------------
HRESULT
HrFindCertificateBySubjectName(
    LPCWSTR			wszStore,
    LPCWSTR			wszSubject,
    PCCERT_CONTEXT	*ppcCert
    )
{
    HRESULT hr = S_OK;
    HCERTSTORE  hStoreHandle = NULL;  // The system store handle.

    *ppcCert = NULL;

    //-------------------------------------------------------------------
    // Open the certificate store to be searched.

    hStoreHandle = CertOpenStore(
                           CERT_STORE_PROV_SYSTEM,          // the store provider type
                           0,                               // the encoding type is not needed
                           NULL,                            // use the default HCRYPTPROV
                           CERT_SYSTEM_STORE_CURRENT_USER,  // set the store location in a 
                                                            //  registry location
                           wszStore
                           );                               // the store name 

    if( NULL == hStoreHandle )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    //-------------------------------------------------------------------
    // Get a certificate that has the specified Subject Name

    *ppcCert = CertFindCertificateInStore(
                           hStoreHandle,
                           X509_ASN_ENCODING ,        // Use X509_ASN_ENCODING
                           0,                         // No dwFlags needed
                           CERT_FIND_SUBJECT_STR,     // Find a certificate with a
                                                      //  subject that matches the 
                                                      //  string in the next parameter
                           wszSubject,                // The Unicode string to be found
                                                      //  in a certificate's subject
                           NULL);                     // NULL for the first call to the
                                                      //  function; In all subsequent
                                                      //  calls, it is the last pointer
                                                      //  returned by the function
    if( NULL == *ppcCert )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

CleanUp:

    if(NULL != hStoreHandle)
    {
        CertCloseStore( hStoreHandle, 0);
    }   

    return hr;
}


/*****************************************************************************
 Usage

*****************************************************************************/
void 
Usage( 
    LPCWSTR wsName 
    )
{
    wprintf( L"%s [Options] SubjectName\n", wsName );
    wprintf( L"\tOptions:\n" );
    wprintf( L"\t        -f 0xHHHHHHHH    : CertGetCertificateChain flags\n" );
    wprintf( L"\t        -s STORENAME     : store name, (by default MY)\n" );
}

/*****************************************************************************
 wmain

*****************************************************************************/
DWORD
__cdecl
wmain(
    int     argc,
    LPWSTR  argv[]
    )
{
    int                         i;

    LPCWSTR                     pwszStoreName = L"MY"; // by default, MY
    LPCWSTR                     pwszCName = NULL;

    PCCERT_CONTEXT              pcTestCertContext = NULL;
    PCCERT_CHAIN_CONTEXT        pChainContext     = NULL;

    CERT_CHAIN_PARA             ChainPara         = {0};
    CERT_CHAIN_POLICY_PARA      ChainPolicy       = {0};
    CERT_CHAIN_POLICY_STATUS    PolicyStatus      = {0};

    DWORD                       dwFlags           = CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;

    HRESULT                     hr = S_OK;

    ChainPara.cbSize = sizeof(ChainPara);
    ChainPolicy.cbSize = sizeof(ChainPolicy);
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    //
    // options
    //

    for( i=1; i<argc; i++ )
    {
        if ( lstrcmpW (argv[i], L"/?") == 0 ||
             lstrcmpW (argv[i], L"-?") == 0 ) 
        {
            Usage( L"BuildChain.exe" );
            goto CleanUp;
        }

        if( *argv[i] != L'-' )
            break;

        if ( lstrcmpW (argv[i], L"-s") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                goto CleanUp;
            }

            pwszStoreName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-f") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                goto CleanUp;
            }

            dwFlags = (DWORD)wcstoul( argv[++i], NULL, 0 );
        }
        
    }

    if( i >= argc )
    {
        hr = E_INVALIDARG;
        
        goto CleanUp;
    }

    pwszCName = argv[i];

    //-------------------------------------------------------------------
    // Find the test certificate to be validated and obtain a pointer to it

    hr = HrFindCertificateBySubjectName(
                                        pwszStoreName,
                                        pwszCName,
                                        &pcTestCertContext
                                        );
    if( FAILED(hr) )
    {    
        goto CleanUp;
    }
    
    //-------------------------------------------------------------------
    // Build a chain using CertGetCertificateChain
    
    if( !CertGetCertificateChain(
                                NULL,                  // use the default chain engine
                                pcTestCertContext,     // pointer to the end certificate
                                NULL,                  // use the default time
                                NULL,                  // search no additional stores
                                &ChainPara,            // use AND logic and enhanced key usage 
                                                       //  as indicated in the ChainPara 
                                                       //  data structure
                                dwFlags,
                                NULL,                  // currently reserved
                                &pChainContext ))      // return a pointer to the chain created
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

	wprintf( L"Chain built with %d certificates.\n", pChainContext->rgpChain[0]->cElement );

    //---------------------------------------------------------------
    // Verify that the chain complies with policy

    ChainPolicy.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
    ChainPolicy.dwFlags = CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;

    PolicyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

    //
    // Base policy  
    //

    ChainPolicy.pvExtraPolicyPara = NULL;
    if (!CertVerifyCertificateChainPolicy(
                                    CERT_CHAIN_POLICY_BASE,
                                    pChainContext,
                                    &ChainPolicy,
                                    &PolicyStatus))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    if( PolicyStatus.dwError != S_OK ) 
    {
        ReportError( L"Base Policy Chain Status Failure:", PolicyStatus.dwError  );
        hr = PolicyStatus.dwError;

        // Instruction: If the PolicyStatus.dwError is CRYPT_E_NO_REVOCATION_CHECK or CRYPT_E_REVOCATION_OFFLINE, 
		// it indicates errors in obtaining revocation information. 
		// These can be ignored since the retrieval of revocation information 
		// depends on network availability

        goto CleanUp;
    }
     else
    {
        wprintf( L"Base Policy CertVerifyCertificateChainPolicy succeeded.\n" );
    }


CleanUp:

    if( NULL != pChainContext )
    {
        CertFreeCertificateChain(pChainContext);
    }
   
    if( NULL != pcTestCertContext ) 
    {
        CertFreeCertificateContext(pcTestCertContext);
    }

    if( FAILED( hr ))
    {
        ReportError( NULL, hr  );
    }

    return (DWORD)hr;
} // end main

