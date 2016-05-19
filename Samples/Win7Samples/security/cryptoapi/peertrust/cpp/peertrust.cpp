// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

Peer Trust 

This sample code shows chain building for a certificate 
in Trusted People store. 

****************************************************************/

//Following functions will use their UNICODE equivalents
//CertStrToName : CertStrToNameW

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

/*****************************************************************************
 Usage

*****************************************************************************/
void 
Usage( 
	LPCWSTR wsName 
    )
{
    wprintf( L"%s [Options]\n", wsName );
    wprintf( L"\tOptions:\n" );
    wprintf( L"\t  -s {STORENAME}     : store name (by default \"TrustedPeople\")\n" );
    wprintf( L"\t  -fe 0xHHHHHHHH     : CertCreateCertificateChainEngine flags\n");
    wprintf( L"\t  -fc 0xHHHHHHHH     : CertGetCertificateChain flags\n");
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
    HRESULT                     hr = S_OK;
    int                         i = 0;

    DWORD                       dwChainFlags    = 0;
    LPCWSTR                     wsStore			= L"TrustedPeople";
    PCCERT_CONTEXT              pCert           = NULL;
    HCERTSTORE                  hStore          = NULL;
    HCERTCHAINENGINE            hChainEngine    = NULL;
    PCCERT_CHAIN_CONTEXT        pChainContext   = NULL;

	CERT_ENHKEY_USAGE           EnhkeyUsage     = {0};
	CERT_USAGE_MATCH            CertUsage       = {0};  
	CERT_CHAIN_PARA             ChainPara       = {0};
	CERT_CHAIN_POLICY_PARA      ChainPolicy     = {0};
	CERT_CHAIN_POLICY_STATUS    PolicyStatus    = {0};
    CERT_CHAIN_ENGINE_CONFIG    EngineConfig    = {0};

	//---------------------------------------------------------
    // Initialize data structures for chain building.

    EnhkeyUsage.cUsageIdentifier = 0;
    EnhkeyUsage.rgpszUsageIdentifier=NULL;
    
	CertUsage.dwType = USAGE_MATCH_TYPE_AND;
    CertUsage.Usage  = EnhkeyUsage;

    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage=CertUsage;

    ChainPolicy.cbSize = sizeof(ChainPolicy);

    PolicyStatus.cbSize = sizeof(PolicyStatus);

    EngineConfig.cbSize = sizeof(EngineConfig);
    EngineConfig.dwUrlRetrievalTimeout = 0;

    //
    // options
    //

    for( i=1; i<argc; i++ )
    {
        if ( lstrcmpW (argv[i], L"/?") == 0 ||
             lstrcmpW (argv[i], L"-?") == 0 ) 
        {
            Usage(argv[0]);
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

            wsStore = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-fc") == 0 )
        {
            if( i+1 >= argc )
            {
                goto InvalidCommandLine;
            }
            
            dwChainFlags = (DWORD)wcstoul( argv[++i], NULL, 0 );
        }
        else
        if ( lstrcmpW (argv[i], L"-fe") == 0 )
        {
            if( i+1 >= argc )
            {
                goto InvalidCommandLine;
            }
            
            EngineConfig.dwFlags = (DWORD)wcstoul( argv[++i], NULL, 0 );
        }
        else
        {
            goto InvalidCommandLine;
        }
    }

	hStore = CertOpenStore(
                   CERT_STORE_PROV_SYSTEM,         // the store provider type
                   0,                              // encoding type not needed
                   NULL,                           // use default HCRYPTPROV
                   CERT_SYSTEM_STORE_CURRENT_USER, // set the store location  
                                                   //  in a registry location
                   wsStore
				   );

    if( NULL == hStore )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    pCert = CertFindCertificateInStore(
                        hStore,
                        X509_ASN_ENCODING,
                        0,
                        CERT_FIND_ANY,
                        NULL,
                        NULL
                        );
    if( NULL == pCert )
    {
        wprintf( L"FAILED: Certificate not found in 'Trusted People' store.\n" );
        hr = CRYPT_E_NOT_FOUND;
        goto CleanUp;
    }

    dwChainFlags |= CERT_CHAIN_ENABLE_PEER_TRUST; 

    // When this flag is set, end entity certificates in the
    // Trusted People store are trusted without doing any chain building
    // This optimizes the chain building process.

	//---------------------------------------------------------
    // Create chain engine.
    
    if( !CertCreateCertificateChainEngine(
                                &EngineConfig,
                                &hChainEngine
                                ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    //-------------------------------------------------------------------
    // Build a chain using CertGetCertificateChain
    
    if( !CertGetCertificateChain(
                                hChainEngine,           // use the default chain engine
                                pCert,                 // pointer to the end certificate
                                NULL,                  // use the default time
                                NULL,                  // search no additional stores
                                &ChainPara,            // use AND logic and enhanced key usage 
                                                       //  as indicated in the ChainPara 
                                                       //  data structure
                                dwChainFlags,
                                NULL,                  // currently reserved
                                &pChainContext ))      // return a pointer to the chain created
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    //---------------------------------------------------------------
    // Verify that the chain complies with policy

    if( !CertVerifyCertificateChainPolicy(
                                CERT_CHAIN_POLICY_BASE, // use the base policy
                                pChainContext,          // pointer to the chain    
                                &ChainPolicy,             
                                &PolicyStatus ))        // return a pointer to the policy status
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    if( PolicyStatus.dwError != S_OK ) 
    {
        ReportError( L"CertVerifyCertificateChainPolicy: Chain Status", PolicyStatus.dwError );
        hr = PolicyStatus.dwError;

		// Instruction: If the PolicyStatus.dwError is CRYPT_E_NO_REVOCATION_CHECK or CRYPT_E_REVOCATION_OFFLINE, it indicates errors in obtaining
		//				revocation information. These can be ignored since the retrieval of revocation information depends on network availability

        goto CleanUp;
	}

    wprintf( L"CertVerifyCertificateChainPolicy succeeded.\n" );

    hr = S_OK;

    //
    // END
    //

    goto CleanUp;

    //
    // Invalid Command Line
    //

InvalidCommandLine:

    if( i < argc )
    {
        wprintf( L"Invalid command line '%s'\n", argv[i] );
    }
    else
        Usage(argv[0]);

    hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );

CleanUp:

    if( FAILED(hr) )
    {
        ReportError( NULL, hr );
    }

    if( NULL != pChainContext )
    {
        CertFreeCertificateChain( pChainContext );
    }

    if( NULL != hChainEngine )
    {
        CertFreeCertificateChainEngine( hChainEngine );
    }

    if( NULL != pCert )
    {
        CertFreeCertificateContext( pCert );
    }

    if( NULL != hStore )
    {
        CertCloseStore( hStore, 0 );
    }

    return (DWORD)hr;
} // end main

