// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

Title: Encrypting Data and creating an enveloped message

This example shows how to encrypt and decrypt a PKCS7 / CMS message.                
It illustrates use of the CryptEncryptMessage and CryptDecryptMessage API.

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
  HrLoadFile

    Load file into allocated (*ppbData). 
    The caller must free the memory by LocalFree().
*****************************************************************************/
HRESULT
HrLoadFile(
	LPCWSTR  wszFileName,
    PBYTE   *ppbData,
    DWORD   *pcbData
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       cbRead = 0;
    HRESULT     hr = S_OK;

    *ppbData = NULL;
    *pcbData = 0;

    hFile = CreateFileW( wszFileName, 
                        GENERIC_READ,
                        0,
                        NULL, 
                        OPEN_EXISTING, 
                        0, 
                        NULL );

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    *pcbData = GetFileSize( hFile, NULL );
    if( *pcbData == 0 ) 
    {
        hr = S_FALSE;
        goto CleanUp;
    }

    *ppbData = (PBYTE)LocalAlloc( LPTR, *pcbData );
    if( NULL == *ppbData )
    {
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
        
        goto CleanUp;
    }

    if( !ReadFile( hFile, *ppbData, *pcbData, &cbRead, NULL ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

CleanUp:

    if (hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle(hFile);
    }

    if( FAILED(hr) )
    {
        if( NULL != *ppbData )
        {
            LocalFree( *ppbData );
        }

        *ppbData = NULL;
        *pcbData = 0;
    }

    return hr;
}

/*****************************************************************************
  HrSaveFile

*****************************************************************************/
HRESULT
HrSaveFile(
    LPCWSTR             wszFileName,
	PBYTE               pbData,
    DWORD               cbData
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HRESULT     hr = S_OK;
    DWORD       cbWritten = 0;

    hFile = CreateFileW( wszFileName, 
                        GENERIC_WRITE,
                        0,
                        NULL, 
                        CREATE_ALWAYS, 
                        0, 
                        NULL );

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    if( !WriteFile( hFile, pbData, cbData, &cbWritten, NULL ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

CleanUp:

    if (hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle(hFile);
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
    wprintf( L"%s [Options] {COMMAND}\n", wsName );
    wprintf( L"    Options:\n" );
    wprintf( L"      -s {STORENAME}   : store name, (by default MY)\n" );
    wprintf( L"      -n {SubjectName} : Recepient certificate's CN to search for.\n" );
    wprintf( L"                        (by default \"Test\")\n" );
    wprintf( L"      -a {CNGAlgName}  : Encryption algorithm, (by default AES128)\n" );
    wprintf( L"      -k {KeySize}     : Encryption key size in bits, (by default 128)\n" );
    wprintf( L"    COMMANDS:\n" );
    wprintf( L"      ENCRYPT {inputfile} {outputfile}\n" );
    wprintf( L"                       | Encrypt message\n" );
    wprintf( L"      DECRYPT {inputfile} {outputfile}\n" );
    wprintf( L"                       | Decrypt message\n" );
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

    BOOL                        fEncrypt = TRUE;        

    LPCWSTR                     pwszInputFile = NULL;
    LPCWSTR                     pwszOutputFile = NULL;

    BYTE                        *pbInput = NULL;
    DWORD                       cbInput = 0;    
    BYTE                        *pbOutput = NULL;
    DWORD                       cbOutput = 0;

    PCCERT_CONTEXT              pRecipientCert = NULL;
    HCERTSTORE                  hStoreHandle = NULL;

    LPCWSTR                     pwszStoreName = L"MY";  // by default
    LPCWSTR                     pwszCName = L"Test";    // by default

    LPCWSTR                     pwszAlgName = L"AES128";
    ULONG                       cKeySize = 128;

    int                         i;

    //
    // options
    //

    for( i=1; i<argc; i++ )
    {
        if ( lstrcmpW (argv[i], L"/?") == 0 ||
             lstrcmpW (argv[i], L"-?") == 0 ) 
        {
            Usage( L"cms_encrypt.exe" );
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
        if ( lstrcmpW (argv[i], L"-n") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            pwszCName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-a") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            pwszAlgName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-k") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            cKeySize = (ULONG)wcstoul( argv[++i], NULL, 0);
        }
    }

    if( 0 == lstrcmpW (argv[i], L"ENCRYPT"))
    {
        if( i+2 >= argc )
        {
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        fEncrypt = TRUE;
        pwszInputFile = argv[++i];
        pwszOutputFile = argv[++i];
    }
    else
    if( 0 == lstrcmpW (argv[i], L"DECRYPT"))
    {
        if( i+2 >= argc )
        {
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        fEncrypt = FALSE;
        pwszInputFile = argv[++i];
        pwszOutputFile = argv[++i];
    }
    else
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if( i != argc-1 )
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    //-------------------------------------------------------------------
    // Open the certificate store to be searched.

    hStoreHandle = CertOpenStore(
                           CERT_STORE_PROV_SYSTEM,          // the store provider type
                           0,                               // the encoding type is not needed
                           NULL,                            // use the default HCRYPTPROV
                           CERT_SYSTEM_STORE_CURRENT_USER,  // set the store location in a 
                                                            //  registry location
                           pwszStoreName
                           );                               // the store name 

    if( NULL == hStoreHandle )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    //
    // Load file
    // 

    hr = HrLoadFile(
                                            pwszInputFile,
                                            &pbInput,
                                            &cbInput
                                            );
    if( FAILED(hr) )
    {
        wprintf( L"Unable to read file: %s\n", pwszInputFile );
        goto CleanUp;
    }


    if( fEncrypt )
    {
        CRYPT_ENCRYPT_MESSAGE_PARA  EncryptParams = {0};

        //-------------------------------------------------------------------
        // Get a certificate that has the specified Subject Name

        pRecipientCert = CertFindCertificateInStore(
                               hStoreHandle,
                               X509_ASN_ENCODING ,        // Use X509_ASN_ENCODING
                               0,                         // No dwFlags needed
                               CERT_FIND_SUBJECT_STR,     // Find a certificate with a
                                                          //  subject that matches the 
                                                          //  string in the next parameter
                               pwszCName,                 // The Unicode string to be found
                                                          //  in a certificate's subject
                               NULL);                     // NULL for the first call to the
                                                          //  function; In all subsequent
                                                          //  calls, it is the last pointer
                                                          //  returned by the function
        if( NULL == pRecipientCert )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanUp;
        }

        //-------------------------------------------------------------------
        // Initialize the CRYPT_ENCRYPT_MESSAGE_PARA structure. 

        EncryptParams.cbSize =  sizeof(EncryptParams);
        EncryptParams.dwMsgEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
        EncryptParams.hCryptProv = NULL;

        //-------------------------------------------------------------------
        // Find Content Encryption Algorithm OID

        EncryptParams.ContentEncryptionAlgorithm.pszObjId = NULL;

        PCCRYPT_OID_INFO pOidInfo = CryptFindOIDInfo(
                                        CRYPT_OID_INFO_NAME_KEY,
                                        (void*)pwszAlgName,
                                        CRYPT_ENCRYPT_ALG_OID_GROUP_ID | 
                                            (cKeySize << CRYPT_OID_INFO_OID_GROUP_BIT_LEN_SHIFT)
                                        );
        if( NULL == pOidInfo )
        {
            hr = CRYPT_E_UNKNOWN_ALGO;
            wprintf( L"FAILED: Unknown algorithm: '%s'.\n", pwszAlgName );
            goto CleanUp;
        }

        EncryptParams.ContentEncryptionAlgorithm.pszObjId = (LPSTR) pOidInfo->pszOID;

        //-------------------------------------------------------------------
        // Call CryptEncryptMessage.

        if( !CryptEncryptMessage(
                                  &EncryptParams,        // Pointer to encryption parameters
                                  1,                     // Number of elements in the CertArray
                                  &pRecipientCert,       // Array of pointers to recipient certificates
                                  pbInput,               // Pointer to message to be encrypted
                                  cbInput,               // Size of message to be encrypted
                                  NULL,                  // Pointer to encrypted content
                                  &cbOutput))            // Size of encrypted content
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            wprintf( L"FAILED: CryptEncryptMessage\n" );
            goto CleanUp;
        }

        //-------------------------------------------------------------------
        // Allocate memory for the returned BLOB.
        pbOutput = (BYTE*)LocalAlloc( LPTR, cbOutput );
        if( NULL == pbOutput )
        {
            hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
            goto CleanUp;
        }

        //-------------------------------------------------------------------
        // Call CryptEncryptMessage again to encrypt the content.

        if( !CryptEncryptMessage(
                                  &EncryptParams,        // Pointer to encryption parameters
                                  1,                    // Number of elements in the CertArray
                                  &pRecipientCert,        // Array of pointers to recipient certificates
                                  pbInput,                // Pointer to message to be encrypted
                                  cbInput,                // Size of message to be encrypted
                                  pbOutput,                // Pointer to encrypted content
                                  &cbOutput))            // Size of encrypted content
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanUp;
        }

        wprintf( L"Successfully encrypted message using CryptEncryptMessage.\n");
    }
    else
    {
        //-------------------------------------------------------------------
        //   Initialize the CRYPT_DECRYPT_MESSAGE_PARA structure.

        CRYPT_DECRYPT_MESSAGE_PARA  DecryptParams = {0};

        DecryptParams.cbSize = sizeof(DecryptParams);
        DecryptParams.dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
        DecryptParams.cCertStore = 1;
        DecryptParams.rghCertStore = &hStoreHandle;

        //-------------------------------------------------------------------
        //  Decrypt the message data.
        //  Call CryptDecryptMessage to get the returned data size.

        if( !CryptDecryptMessage(
                                              &DecryptParams,        // Pointer to decryption parameters
                                              pbInput,                // Pointer to encrypted content
                                              cbInput,                // Size of encrypted content
                                              NULL,                    // Pointer to decrypted message
                                              &cbOutput,            // Size of decrypted message
                                              NULL))                // Pointer to certificate that corresponds
                                                                    // to private exchange key for decryption
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            wprintf( L"FAILED: CryptDecryptMessage\n" );
            goto CleanUp;
        }

        //-------------------------------------------------------------------
        // Allocate memory for the returned decrypted data.

        pbOutput = (BYTE*)LocalAlloc( LPTR, cbOutput );
        if( NULL == pbOutput )
        {
            hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
            goto CleanUp;
        }

        //-------------------------------------------------------------------
        // Call CryptDecryptMessage to decrypt the data.

        if( !CryptDecryptMessage(
                                              &DecryptParams,        // Pointer to decryption parameters
                                              pbInput,                // Pointer to encrypted content
                                              cbInput,                // Size of encrypted content
                                              pbOutput,                // Pointer to decrypted message
                                              &cbOutput,            // Size of decrypted message
                                              NULL))                // Pointer to certificate that corresponds
                                                                    // to private exchange key for decryption
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanUp;
        }

        wprintf( L"Successfully decrypted message using CryptDecryptMessage.\n");
    }

    hr = HrSaveFile(
                                pwszOutputFile,
                                pbOutput,
                                cbOutput
                                );
    if( FAILED(hr) )
    {
        wprintf( L"Unable to save file: %s\n", pwszOutputFile );
        
        goto CleanUp;
    }


    hr = S_OK;

    //-------------------------------------------------------------------
    // Clean up memory.

CleanUp:

    if( NULL != pbInput )
        LocalFree(pbInput);

    if( NULL != pbOutput )
        LocalFree(pbOutput);

    if( NULL != pRecipientCert )
        CertFreeCertificateContext(pRecipientCert);

    if( NULL != hStoreHandle)
        CertCloseStore( hStoreHandle, 0 );

    if( FAILED( hr ))
    {
        ReportError( NULL, hr  );
    }

    return (DWORD)hr;
} // End of main
