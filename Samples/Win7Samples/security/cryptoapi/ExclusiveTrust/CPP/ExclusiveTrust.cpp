// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

Title: Providing exclusive trust anchors for certificate validation

This sample shows how to pass in exclusive trust anchors for certificate validation. 
By default, certificate validation on Windows uses the trusted self-signed certificates 
from the Trusted Root Certification Authorities store as trust anchors. Windows 7 
provides a way for calling applications to pass in exclusive trust anchors which are independent 
of the system trust anchors. This can be done by creating a non-default certificate chain engine 
as shown in the sample.

****************************************************************/

#define CRYPT_OID_INFO_HAS_EXTRA_FIELDS

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
    wprintf( L"%s [Options] {StoreFile} [EndCertFile]\n", wsName );
    wprintf( L"\tOptions:\n" );
    wprintf( L"\t  -fe 0xHHHHHHHH     : CertCreateCertificateChainEngine flags\n");
    wprintf( L"\t  -fc 0xHHHHHHHH     : CertGetCertificateChain flags\n");
    wprintf( L"\t  -p                 : Peer Trust\n");
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

    BOOL                        fPeerTrust      = FALSE;
    DWORD                       dwChainFlags    = CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
    LPCWSTR                     wsCertFile      = NULL;
    LPCWSTR                     wsStoreFile     = NULL;
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
        if ( lstrcmpW (argv[i], L"-p") == 0 )
        {
            fPeerTrust = TRUE;
        }
        else
        {
            goto InvalidCommandLine;
        }
    }

    if( i >= argc )
    {
        goto InvalidCommandLine;
    }

    wsStoreFile = argv[i++];

    if( i < argc )
    {
        wsCertFile = argv[i];
    }

    hStore = CertOpenStore(
                            CERT_STORE_PROV_FILENAME_W,
                            X509_ASN_ENCODING,
                            NULL,
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                            wsStoreFile
                            );
    if( NULL == hStore )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    if( NULL != wsCertFile && 0 != *wsCertFile )
    {
        if( !CryptQueryObject(
                            CERT_QUERY_OBJECT_FILE,
                            wsCertFile,
                            CERT_QUERY_CONTENT_FLAG_CERT |
                            CERT_QUERY_CONTENT_SERIALIZED_CERT
                            ,
                            CERT_QUERY_FORMAT_FLAG_ALL,
                            0,      // dwFlags,
                            0,      // pdwMsgAndCertEncodingType,
                            0,      // pdwContentType,
                            0,      // pdwFormatType,
                            0,      // phCertStore,
                            0,      // phMsg,
                            (const void**)&pCert
                            ))
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanUp;
        }
    }
    else
    {
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
            hr = CRYPT_E_NOT_FOUND;
            goto CleanUp;
        }
    }

    if( fPeerTrust )
    {
        EngineConfig.hExclusiveTrustedPeople = hStore;      // Exclusive peer trust
        dwChainFlags |= CERT_CHAIN_ENABLE_PEER_TRUST;
    }
    else
    {
        EngineConfig.hExclusiveRoot = hStore;               // Exclusive root
    }

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
                                hChainEngine,           
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
