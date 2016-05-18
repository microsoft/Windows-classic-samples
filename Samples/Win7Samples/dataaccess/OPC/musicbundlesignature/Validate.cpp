//<SnippetMusicBundleSig_cppValidateWholePage>
/*****************************************************************************
*
* File: Validate.cpp
*
* Description: 
* This sample is a simple application that might be used as a starting-point
* for an application that uses the Packaging API. This sample demonstrates
* signature generation and validation using a sample signing policy described
* in Sign.h
*
* ------------------------------------
*
*  This file is part of the Microsoft Windows SDK Code Samples.
* 
*  Copyright (C) Microsoft Corporation.  All rights reserved.
* 
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
* 
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
* 
****************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>

#include <msopc.h>  // For OPC APIs

#include <WinCrypt.h>   // For certificate stuff
#include <CryptDlg.h>   // For certificate selection dialog

#include <strsafe.h>
#include <new>

#include "Util.h"
#include "Sign.h"
#include "Validate.h"

// Validates a signature and finds the certificate of the signer.
//
// Parameters:
//      opcDigSigManager - the digital signature manager from which we get the signature.
//      signature - the signature we want to validate.
//      isValid   - output. When the function returns successfully, it tells whether the
//                  signature is valid.
//      signerCert - output. When the function returns successfully and isValid is true, 
//                  it holds the certificate of the signer.
HRESULT
ValidateSignature(
    IOpcDigitalSignatureManager* opcDigSigManager,
    IOpcDigitalSignature* signature,
    BOOL* isValid,
    PCCERT_CONTEXT* signerCert
    )
{
    *isValid = FALSE;
    *signerCert = NULL;

    IOpcCertificateEnumerator * certEnumerator = NULL;
    HRESULT hr = signature->GetCertificateEnumerator(&certEnumerator);

    UINT32 certCount = 0;
    BOOL hasNext = TRUE;

    // There may be multiple certificates related to the signature. Only one is the signer's
    // certificate. Others should be in a chain to a trusted root certificate. 
    // We need to iterate through the certificate enumerator and try using each certificate
    // to validate the signature until one is successful (the signature is valid against the 
    // certificate). Then the certificate must be the signer's certificate. 
    //
    // In production code, the application may build a certificate chain starting from the 
    // signer's certificate to a trusted root certificate (see CertGetCertificateChain for more 
    // details). If such a chain cannot be built, then the signer's certificate is not trustable.
    // The other certificates in the certificate enumerator are used to facilitate chain building.
    // The application can create a memory certificate store (see CertOpenStore) and put all these 
    // certificates to the store, then provide the store as an additional store to the 
    // CertGetCertificateChain function to search for the certificate chain. Without providing the 
    // additional store, if the user's machine does not have all the needed certificates, it will
    // need to download them from Internet, if possible, or fail to build the chain.
    //
    // In this sample code we are not doing certificate chain building and validation for
    // simplicity.
    while (
        SUCCEEDED(hr)
        &&
        SUCCEEDED(hr = certEnumerator->MoveNext(&hasNext))
        &&
        hasNext
        )
    {
        OPC_SIGNATURE_VALIDATION_RESULT result;
        PCCERT_CONTEXT cert = NULL;

        certCount++;

        // The contract of GetCurrent ensures that if the method fails,
        // it will not return a valid CERT_CONTEXT. If it succeeds,
        // CertFreeCertificateContext is called to free the CERT_CONTEXT;
        // unless it is being returned to the caller in signerCert.
        hr = certEnumerator->GetCurrent(&cert);

        if (FAILED(hr))
        {
            // Filter with known possible errors that we can still continue safely.
            UINT32 facilityCode = HRESULT_FACILITY(hr);

            if (facilityCode == FACILITY_SECURITY || facilityCode == FACILITY_OPC)
            {
                // The error may be due to a corrupted certificate, bad certificate relationship or
                // missing certificate part. We should give a warning to the end user and continue to 
                // search for the next certificate available.
                fwprintf(
                    stdout, 
                    L"Warning: Found corrupted certificate in the package. "
                    L"IOpcCertificateEnumerator::GetCurrent() returns hr=0x%x\n",
                    hr
                    );

                hr = S_OK;  // reset the hr so we can continue with the loop.
                continue;
            }
            else
            {
                // For other types of errors, we will not continue. This way, we will not try to
                // continue if the error is critical like E_OUTOFMEMORY.
                break;
            }
        }

        // Verify that the digital signature is valid. It means that all the content 
        // that was signed has not been modified.
        hr = opcDigSigManager->Validate(signature, cert, &result);
        
        if (SUCCEEDED(hr))
        {
            if (result == OPC_SIGNATURE_VALID)
            {
                // Found the signer certificate and the signature is valid.
                *isValid = TRUE;
                *signerCert = cert;
                break;
            }
        }
        else
        {
            // The failure HRESULT from IOpcDigitalSignatureManager::Validate() has information on
            // why the validation failed. 
            UINT32 facilityCode = HRESULT_FACILITY(hr);

            if (facilityCode == FACILITY_SECURITY)
            {
                // The error may be due to an incorrect certificate provided to the validation. 
                // We should try the next certificate available.
                fwprintf(
                    stdout, 
                    L"Warning: Tried to validate signature with certificate %u and it failed with hr=0x%x. \n",
                    certCount,
                    hr
                    );

                hr = S_OK;  // reset the hr so we can continue with the loop.
            }
            // For other types of errors, we will not continue. This way, we will not try to
            // continue if the error is critical like E_OUTOFMEMORY.
        }

        // Free the CERT_CONTEXT we got from the certificate enumerator.
        CertFreeCertificateContext(cert);
    }

    if (SUCCEEDED(hr) && certCount == 0)
    {
        // OPC spec does not require certificate of a signature to be stored in the package.
        // If it is not stored in the package, the application validating the signature should have
        // the knowledge of what certificate to use. For example, a person A signs a package without
        // storing his/her certificate in the package. Person B gets the package and is told the
        // package was signed by A. B has A's certificate (with public key only). So B can explicitly
        // use the certificate to verify whether the package is really signed by A. In this sample we
        // don't have such a scenario and require signer's certificate must store with the signature
        // in the package.
        fwprintf(
            stderr, 
            L"There is no certificate associated with the signature. Cannot validate the signature.\n"
            );
    }

    // Release resources
    if (certEnumerator)
    {
        certEnumerator->Release();
        certEnumerator = NULL;
    }

    return hr;
}

HRESULT
PrintRelationshipReferenceInfo(
    IOpcSignatureRelationshipReference* relsReference
    )
{
    IOpcUri * sourceUri = NULL;
    BSTR uriString = NULL;

    OPC_RELATIONSHIPS_SIGNING_OPTION relsSigningOption = OPC_RELATIONSHIP_SIGN_USING_SELECTORS;

    HRESULT hr = relsReference->GetSourceUri(&sourceUri);

    if (SUCCEEDED(hr))
    {
        hr = sourceUri->GetDisplayUri(&uriString);
    }

    if (SUCCEEDED(hr))
    {
        fwprintf(stdout, L"Source Uri: %s\n", uriString);
    }

    if (SUCCEEDED(hr))
    {
        hr = relsReference->GetRelationshipSigningOption(&relsSigningOption);
    }

    if (SUCCEEDED(hr))
    {
        if (relsSigningOption == OPC_RELATIONSHIP_SIGN_PART)
        {
            // Note that in this sample we don't sign relationships part in whole
            // so we should not hit this line of code.
            fwprintf(stdout, L"The whole relationships part is signed.\n");
        }
        else // relsSigningOption == OPC_RELATIONSHIP_SIGN_USING_SELECTORS
        {
            IOpcRelationshipSelectorEnumerator * selectorEnumerator = NULL;
            BOOL hasNext = FALSE;

            hr = relsReference->GetRelationshipSelectorEnumerator(&selectorEnumerator);

            while(
                SUCCEEDED(hr)
                &&
                SUCCEEDED(hr = selectorEnumerator->MoveNext(&hasNext))
                &&
                hasNext
                )
            {
                IOpcRelationshipSelector * selector = NULL;
                LPCWSTR token = NULL;
                LPWSTR criterion = NULL;

                hr = selectorEnumerator->GetCurrent(&selector);

                OPC_RELATIONSHIP_SELECTOR selectorType = OPC_RELATIONSHIP_SELECT_BY_ID;
                if (SUCCEEDED(hr))
                {
                    hr = selector->GetSelectorType(&selectorType);
                }

                switch (selectorType)
                {
                case OPC_RELATIONSHIP_SELECT_BY_ID:
                    token = L"Id";
                    fwprintf(stdout, L"Select by relationship Id.\n");
                    break;

                case OPC_RELATIONSHIP_SELECT_BY_TYPE:
                    token = L"Type";
                    fwprintf(stdout, L"Select by relationship Type.\n");
                    break;

                default:
                    fwprintf(stderr, L"Invalid OPC_RELATIONSHIP_SELECTOR value.\n");
                    hr = E_UNEXPECTED;
                }

                if (SUCCEEDED(hr))
                {
                    hr = selector->GetSelectionCriterion(&criterion);
                }

                if (SUCCEEDED(hr))
                {
                    fwprintf(stdout, L"%s = %s\n", token, criterion);
                }

                if (selector)
                {
                    selector->Release();
                    selector = NULL;
                }

                CoTaskMemFree(static_cast<LPVOID>(criterion));
            }

            if (selectorEnumerator)
            {
                selectorEnumerator->Release();
                selectorEnumerator = NULL;
            }
        }
    }

    // Release resources
    if (sourceUri)
    {
        sourceUri->Release();
        sourceUri = NULL;
    }

    SysFreeString(uriString);

    return hr;
}

HRESULT
PrintSigningInfo(
    IOpcDigitalSignature* signature
    )
{
    IOpcSignaturePartReferenceEnumerator * signedPartsEnumerator = NULL;
    IOpcSignatureRelationshipReferenceEnumerator * signedRelationshipsEnumerator = NULL;
    BOOL hasNext = FALSE;

    HRESULT hr = signature->GetSignaturePartReferenceEnumerator(&signedPartsEnumerator);

    if (SUCCEEDED(hr))
    {
        fwprintf(stdout, L"\nThe signature signed the following parts:\n");
    }

    while(
        SUCCEEDED(hr)
        &&
        SUCCEEDED(hr = signedPartsEnumerator->MoveNext(&hasNext))
        &&
        hasNext
        )
    {
        IOpcSignaturePartReference * partReference = NULL;
        IOpcPartUri * partName = NULL;
        BSTR nameString = NULL;

        hr = signedPartsEnumerator->GetCurrent(&partReference);

        if (SUCCEEDED(hr))
        {
            hr = partReference->GetPartName(&partName);
        }

        if (SUCCEEDED(hr))
        {
            hr = partName->GetDisplayUri(&nameString);
        }

        if (SUCCEEDED(hr))
        {
            fwprintf(stdout, nameString);
            fwprintf(stdout, L"\n");
        }

        // Release resources
        if (partReference)
        {
            partReference->Release();
            partReference = NULL;
        }

        if (partName)
        {
            partName->Release();
            partName = NULL;
        }

        SysFreeString(nameString);
    }

    if (SUCCEEDED(hr))
    {
        hr = signature->GetSignatureRelationshipReferenceEnumerator(&signedRelationshipsEnumerator);

        fwprintf(stdout, L"\nThe signature signed the following relationships:\n");
    }

    while(
        SUCCEEDED(hr)
        &&
        SUCCEEDED(hr = signedRelationshipsEnumerator->MoveNext(&hasNext))
        &&
        hasNext
        )
    {
        IOpcSignatureRelationshipReference * relsReference = NULL;

        hr = signedRelationshipsEnumerator->GetCurrent(&relsReference);

        if (SUCCEEDED(hr))
        {
            hr = PrintRelationshipReferenceInfo(relsReference);
        }

        fwprintf(stdout, L"\n");

        // Release resources
        if (relsReference)
        {
            relsReference->Release();
            relsReference = NULL;
        }
    }

    // Release resources
    if (signedPartsEnumerator)
    {
        signedPartsEnumerator->Release();
        signedPartsEnumerator = NULL;
    }

    if (signedRelationshipsEnumerator)
    {
        signedRelationshipsEnumerator->Release();
        signedRelationshipsEnumerator = NULL;
    }

    return hr;
}

HRESULT
ValidateMusicBundleSignature(
    IOpcFactory* opcFactory
    )
{
    IOpcPackage * opcPackage = NULL;
    IOpcDigitalSignatureManager * opcDigSigManager = NULL;
    IOpcDigitalSignatureEnumerator * signatureEnumerator = NULL;
    BOOL hasNext = FALSE;
    UINT32 count = 0;

    // Load the signed music bundle package.
    HRESULT hr = ReadPackageFromFile(g_signedFilePath, opcFactory, &opcPackage);

    // Create a digital signature manager for the package.
    if (SUCCEEDED(hr))
    {
        hr = opcFactory->CreateDigitalSignatureManager(opcPackage, &opcDigSigManager);
    }

    if (SUCCEEDED(hr))
    {
        hr = opcDigSigManager->GetSignatureEnumerator(&signatureEnumerator);
    }

    // We report all the signatures (if there are more than one) in the music bundle and validate
    // all of them. Note that in the signing part of this sample we only created 1 signature but 
    // the validation sample is written for generic handling of a signed package. 
    while (
        SUCCEEDED(hr)
        &&
        SUCCEEDED(hr = signatureEnumerator->MoveNext(&hasNext))
        &&
        hasNext
        )
    {
        count++;

        IOpcDigitalSignature * signature = NULL;
        BOOL isValid = FALSE;
        PCCERT_CONTEXT signerCert = NULL;

        hr = signatureEnumerator->GetCurrent(&signature);

        if (SUCCEEDED(hr))
        {
            fwprintf(stdout, L"Found Signature %u:\n", count);

            hr = ValidateSignature(opcDigSigManager, signature, &isValid, &signerCert);

            if (SUCCEEDED(hr))
            {
                if (isValid)
                {
                    WCHAR certFileName[MAX_PATH];

                    fwprintf(stdout, L"Signature %u is valid.\n", count);
    
                    // In production code, the application should check whether the signature is compliant to
                    // the signing policy: all the required parts and relationships are signed properly (please
                    // refer to the sample signing policy in Sign.h to see the details).
                    // In this sample, for simplicity, we just print out such information for user to verify.
                    PrintSigningInfo(signature);

                    hr = StringCchPrintf(certFileName, MAX_PATH, L"Signer%u.cer", count);
    
                    if (SUCCEEDED(hr))
                    {
                        hr = SaveCertificateToFile(signerCert, certFileName);

                        if (SUCCEEDED(hr))
                        {
                            // In production code, the application may verify the certificate by building a certificate 
                            // chain to a trusted root certificate (see CertGetCertificateChain for more details). 
                            // In this sample, for simplicity, we store the signer certificate to a file so the user
                            // can double-click it and open it in the certificate UI and verify it.
                            fwprintf(
                                stdout, 
                                L"The signer of signature %u is identified by the certificate stored in %s. "
                                L"Please inspect the certificate to see whether you trust the signer "
                                L"(you can double click on the certificate file to open it).\n", 
                                count,
                                certFileName
                                );
                        }
                        else
                        {
                            fwprintf(
                                stderr,
                                L"Failed to save the certificate to file. \n"
                                );
                        }
                    }
    
                    CertFreeCertificateContext(signerCert);
                }
                else
                {
                    fwprintf(stdout, L"Signature %u is invalid.\n", count);
                }
            }
        }
        else
        {
            // Filter with known possible errors that are continuable, and we should be
            // able to continue to check the next signature if there is any.
            UINT32 facilityCode = HRESULT_FACILITY(hr);

            if (facilityCode == FACILITY_OPC
                ||
                facilityCode == FACILITY_SECURITY
                ||
                facilityCode == FACILITY_WEBSERVICES)
            {
                // These types of error indicate that the signature has problems such as 
                // the signature part is missing, the signature XML is mal-formatted or corrupted, etc.
                // End user may be able to get a clue of what went wrong in the signature by
                // looking up the error code. 
                fwprintf(
                    stdout, 
                    L"Warning: Found Signature %u but it is corrupted. "
                    L"IOpcDigitalSignatureEnumerator::GetCurrent() returns hr=0x%x\n", 
                    count,
                    hr
                    );

                hr = S_OK;  // reset the hr so we can continue with the loop.
            }
        }

        if (signature)
        {
            signature->Release();
            signature = NULL;
        }
    }

    if (SUCCEEDED(hr) && count == 0)
    {
        fwprintf(stderr, L"Music bundle [%s] has no signature.\n", g_signedFilePath);
        hr = E_UNEXPECTED;
    }

    // Release resources
    if (opcPackage)
    {
        opcPackage->Release();
        opcPackage = NULL;
    }

    if (opcDigSigManager)
    {
        opcDigSigManager->Release();
        opcDigSigManager = NULL;
    }

    if (signatureEnumerator)
    {
        signatureEnumerator->Release();
        signatureEnumerator = NULL;
    }

    return hr;
}
//</SnippetMusicBundleSig_cppValidateWholePage>