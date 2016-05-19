//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  --------------------------------------------------------------------------
//  This sample shows how to use the new certificate selection API in Windows 7
//  CertSelectCertificateChains to select certificates.  It also shows how to 
//  display the selected certificates to the user for selection using
//  CredUIPromptForWindowsCredentials API.
//  
//  Requirements:
//  
//  Scenario: Selecting a certificate for signing email
//  Select a certificate that meets the following criteria
//  a.    Is from the user MY store
//  b.    Has the SMIME EKU 
//  c.    Has the Digital Signature KU
//  d.    Is not expired
//  
//  NOTE: THIS SAMPLE WILL NOT COMPILE ON WINDOWS VISTA AS THIS USES NEW APIS AVAILABLE IN WINDOWS 7.

#include <windows.h>
#include <winerror.h>
#include <strsafe.h>
#include <wincrypt.h>
#include <stdio.h>
#include <cryptuiapi.h>
#include <wincred.h>

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
 wmain

*****************************************************************************/
DWORD
__cdecl
wmain(
    int     argc,
    LPWSTR  argv[]
    )
{
    HRESULT                 hr = S_OK;
    HCERTSTORE              hStore = NULL;
    PCCERT_CHAIN_CONTEXT    *prgpSelection = 0;
    DWORD                   cSelection = 0;
    BYTE                    bDigSig = CERT_DIGITAL_SIGNATURE_KEY_USAGE; // in byte 0
    
	ULONG                 ulAuthPackage;
    BOOL                  bSave           = FALSE;
    DWORD                 cbAuthBuffer    = 0;
    DWORD                 dwError         = ERROR_SUCCESS;
    DWORD                 dwAuthError     = 0;
    void * pBuffer = NULL;
    ULONG ulSize = 0;  
    LPVOID                pbAuthBuffer;
    PCCERT_CONTEXT	  pCertContext = NULL;
    CREDUI_INFO           CredUiInfo;
    
    CredUiInfo.pszCaptionText = L"Select your credentials";
    CredUiInfo.pszMessageText = L"Please select a certificate";
    CredUiInfo.cbSize = sizeof(CredUiInfo);
    CredUiInfo.hbmBanner = NULL;
    CredUiInfo.hwndParent = NULL;

    ulAuthPackage = (ULONG) CERT_CREDENTIAL_PROVIDER_ID; 

    CERT_SELECTUI_INPUT certInput = {0};
       
    void* rgParaEKU[] =
    {
        szOID_PKIX_KP_EMAIL_PROTECTION
    };

    CERT_SELECT_CRITERIA EKUCriteria = 
    {
        CERT_SELECT_BY_ENHKEY_USAGE,
        ARRAYSIZE(rgParaEKU),
        rgParaEKU
    };

    CERT_EXTENSION extDigSig =
    {
        NULL, // pszObjId
        FALSE, // fCritical
        {1, &bDigSig} // Value
    };

    void* rgParaKU[] =
    {
        &extDigSig // digital signature key usage
    };

    CERT_SELECT_CRITERIA KUCriteria = 
    {
        CERT_SELECT_BY_KEY_USAGE,
        ARRAYSIZE(rgParaKU),
        rgParaKU
    };

   CERT_SELECT_CRITERIA rgCriteriaFilter[] =
    {
        EKUCriteria,
        KUCriteria
    };

    hStore = CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            NULL,
                            CERT_SYSTEM_STORE_CURRENT_USER,
                            L"MY"
                            );
    
    if( NULL == hStore )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    wprintf( L"Looking for certificates in MY store ...\n" );

    if( !CertSelectCertificateChains(
                            NULL,
                            CERT_SELECT_TRUSTED_ROOT | CERT_SELECT_HAS_PRIVATE_KEY,
                            NULL,
                            ARRAYSIZE(rgCriteriaFilter),
                            rgCriteriaFilter,
                            hStore,
                            &cSelection,
                            &prgpSelection))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    if( cSelection < 1 )
    {
        wprintf( L"No certificates found matching the selection criteria.\n" );
        goto CleanUp;
    }
    
    wprintf( L"%u certificates found matching the selection criteria.\n", cSelection );

	//
	// show the selected cert in UI
	//

	certInput.prgpChain = prgpSelection;
    certInput.cChain = cSelection;
        
    hr = CertSelectionGetSerializedBlob(
                        &certInput,
                        &pBuffer,
                        &ulSize);

    if(S_OK != hr)
        goto CleanUp;
    
    dwError = CredUIPromptForWindowsCredentials(
                        &CredUiInfo, 
                        dwAuthError, 
                        (PULONG)&ulAuthPackage, 
                        pBuffer,
                        ulSize,
                        &pbAuthBuffer, 
                        &cbAuthBuffer, 
                        &bSave, 
                        CREDUIWIN_AUTHPACKAGE_ONLY);
    if(ERROR_SUCCESS != dwError)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
		//get the selected cert context 
        if( !CertAddSerializedElementToStore( NULL,
                                          (BYTE *)pbAuthBuffer,
                                          cbAuthBuffer,
                                          CERT_STORE_ADD_ALWAYS,
                                          0, // flags
                                          CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
                                          NULL,
                                          reinterpret_cast<const void **>(&pCertContext)
                                          ))
        {
			 goto CleanUp;
		}

		//
		// pCertContext now is ready to use
		//
    }

CleanUp:

    if( FAILED( hr ))
    {
        ReportError( NULL, hr  );
    }

    if( NULL != pBuffer )
	{
        LocalFree(pBuffer);
	}

    if( NULL != pCertContext )
	{
        CertFreeCertificateContext(pCertContext);
	}
    
    return (DWORD)hr;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );
}
