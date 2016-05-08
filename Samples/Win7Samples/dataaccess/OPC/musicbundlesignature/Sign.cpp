//<SnippetMusicBundleSig_cppSignWholePage>
/*****************************************************************************
*
* File: Sign.cpp
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
#include "urlmon.h" // for Uri stuff


// Input/Ouput file names
static const WCHAR g_unsignedFilePath[] = L"SampleMusicBundle.zip";
const WCHAR g_signedFilePath[] = L"SampleMusicBundle_signed.zip";

static const WCHAR g_signatureMethod[] = L"http://www.w3.org/2000/09/xmldsig#rsa-sha1";
static const WCHAR g_defaultDigestMethod[] = L"http://www.w3.org/2000/09/xmldsig#sha1";

static const WCHAR g_signatureOriginPartName[] = L"/package/services/digital-signature/origin.psdsor";

// Content types for parts in music bundle
static const WCHAR g_trackContentType[] = L"audio/x-ms-wma";
static const WCHAR g_lyricContentType[] = L"text/plain";
static const WCHAR g_albumArtContentType[] = L"image/jpeg";
static const WCHAR g_tracklistPartContentType[] = L"application/vnd.ms-wpl";

// Relationship types for relationships in music bundle
static const WCHAR g_tracklistRelationshipType[] = 
    L"http://schemas.example.com/package/2008/relationships/media-bundle/tracklist";

// We reuse the thumbnail relationship defined in Office Open XML spec for relationship 
// to the album art image part.
static const WCHAR g_albumArtRelationshipType[] = 
    L"http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail";

static const WCHAR g_albumWebsiteRelationshipType[] = 
    L"http://schemas.example.com/package/2008/relationships/media-bundle/album-website";

static LPCWSTR g_trackRelationshipType = 
    L"http://schemas.example.com/package/2008/relationships/media-bundle/playlist-song";

static LPCWSTR g_lyricRelationshipType = 
    L"http://schemas.example.com/package/2008/relationships/media-bundle/song-lryic";

// Signature relationship type as defined in OPC spec
static LPCWSTR g_signatureRelationshipType = 
    L"http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/signature";


// Get the parts that need to be signed according to the signing policy.
// Method may return valid values for some of the out parameters even on failure,
// and the caller must always free resources if a non-NULL value is returned.
//
// Parameters:
//  opcPackage - the package of the parts
//  trackParts - when the function returns successfully, it holds the arry of track parts.
//  lyricParts - when the function returns successfully, it holds the arry of lyric parts.
//  tracklistPart - when the function returns successfully, it holds the tracklist part.
//  albumArtPart - when the function returns successfully, it holds the album art part.
HRESULT
GetPartsToBeSigned(
    IOpcPackage* opcPackage,
    OpcPartArray* trackParts,
    OpcPartArray* lyricParts,
    IOpcPart** tracklistPart,
    IOpcPart** albumArtPart
    )
{
    IOpcPartSet * partSet = NULL;
    IOpcRelationshipSet * relsSet = NULL;
    IOpcRelationshipSet * partRelsSet = NULL;
    IOpcRelationshipEnumerator * partRelsEnumerator = NULL;
    UINT32 count = 0;
    BOOL hasNext = FALSE;
    BOOL hasPrevious = FALSE;

    HRESULT hr = opcPackage->GetRelationshipSet(&relsSet);

    if (SUCCEEDED(hr))
    {
        hr = opcPackage->GetPartSet(&partSet);
    }

    // Get album art part. As defined by our sample signing policy, it needs to be signed.
    if (SUCCEEDED(hr))
    {
        hr = GetPartByRelationshipType(
                partSet, 
                relsSet, 
                g_albumArtRelationshipType, 
                g_albumArtContentType,
                albumArtPart
                );
    }

    // Get tracklist part. As defined by our sample signing policy, it needs to be signed.
    if (SUCCEEDED(hr))
    {
        hr = GetPartByRelationshipType(
                partSet, 
                relsSet, 
                g_tracklistRelationshipType, 
                g_tracklistPartContentType, 
                tracklistPart
                );
    }

    // Get tracklist part's relationships.
    if (SUCCEEDED(hr))
    {
        hr = (*tracklistPart)->GetRelationshipSet(&partRelsSet);
    }


    // Get tracklist part's relationships with track relationship type.
    if (SUCCEEDED(hr))
    {
        hr = partRelsSet->GetEnumeratorForType(g_trackRelationshipType, &partRelsEnumerator);
    }

    // Get the number of relationships to track parts. Note that we assume the 
    // number of track parts is the same as the number of lyric parts.
    
    

    while (
        SUCCEEDED(hr) &&
        SUCCEEDED(hr = partRelsEnumerator->MoveNext(&hasNext)) &&
        hasNext
        )
    {
        count++;
    }
    
    // Initialize the OpcPartArrays with desired capacity.
    if (SUCCEEDED(hr))
    {
        hr = trackParts->Init(count);
    }
    if (SUCCEEDED(hr))
    {
        hr = lyricParts->Init(count);
    }

        
    while (
        SUCCEEDED(hr) &&
        // There is no method to rewind the enumerator to go to the beginning.
        // So we will have to go backwards to revisit the elements if we reuse the existing
        // enumerator. If you don't want to go backwards, you can call Clone() on the
        // enumerator BEFORE you started to iterate through it. The cloned enumerator
        // will have the position pointing to the beginning so you can iterate forwards.
        SUCCEEDED(hr = partRelsEnumerator->MovePrevious(&hasPrevious)) &&
        hasPrevious
        )
    {
        IOpcRelationship * rels = NULL;
        IOpcPart * trackPart = NULL;
        IOpcRelationshipSet * trackPartRelationshipSet = NULL;
        IOpcPart * lyricPart = NULL;

        hr = partRelsEnumerator->GetCurrent(&rels);

        // Get track part. As defined by our sample signing policy, it needs to be signed.
        if (SUCCEEDED(hr))
        {
            hr = GetRelationshipTargetPart(partSet, rels, g_trackContentType, &trackPart);
        }

        if (SUCCEEDED(hr))
        {
            hr = trackParts->Append(trackPart);
        }

        if (SUCCEEDED(hr))
        {
            hr = trackPart->GetRelationshipSet(&trackPartRelationshipSet);
        }

        // Get lyric part. As defined by our sample signing policy, it needs to be signed.
        if (SUCCEEDED(hr))
        {
            hr = GetPartByRelationshipType(
                    partSet, 
                    trackPartRelationshipSet, 
                    g_lyricRelationshipType, 
                    g_lyricContentType, 
                    &lyricPart
                    );
        }
        
        if (SUCCEEDED(hr))
        {
            hr = lyricParts->Append(lyricPart);
        }

        // Release resources
        if (rels)
        {
            rels->Release();
            rels = NULL;
        }

        if (trackPart)
        {
            trackPart->Release();
            trackPart = NULL;
        }

        if (trackPartRelationshipSet)
        {
            trackPartRelationshipSet->Release();
            trackPartRelationshipSet = NULL;
        }

        if (lyricPart)
        {
            lyricPart->Release();
            lyricPart = NULL;
        }
    }
    
    // Release resources
    if (partSet)
    {
        partSet->Release();
        partSet = NULL;
    }

    if (relsSet)
    {
        relsSet->Release();
        relsSet = NULL;
    }

    if (partRelsSet)
    {
        partRelsSet->Release();
        partRelsSet = NULL;
    }

    if (partRelsEnumerator)
    {
        partRelsEnumerator->Release();
        partRelsEnumerator = NULL;
    }

    // Depending on file format specification, we may also want to check whether the parts 
    // we want to sign have valid content formats. For example, if the file format says
    // a specific part cannot be empty, then we should also check whether the part size 
    // is zero. Validating the file format before signing makes sure you sign a package 
    // that is valid according to your file format specification. 
    return hr;
}

// Adds relationship reference by relationship type. Multiple relationship types
// can be added in one call to this method. 
//
// Parameters:
//  relsReferenceSet - the relationship reference set that the new reference will be added to
//  sourceUri - source uri of the relationships
//  count - number of relationship type strings in the relationshipTypes array
//  relationshipTypes - an array of relationship types
HRESULT
AddReferenceByRelationshipType(
    IOpcSignatureRelationshipReferenceSet* relsReferenceSet,
    IOpcUri* sourceUri,
    UINT32 count,
    LPCWSTR relationshipTypes[]
    )
{
    // Create the relationship selector set to add relationship types as selection criterion.
    IOpcRelationshipSelectorSet * relsSelectorSet = NULL;

    HRESULT hr = relsReferenceSet->CreateRelationshipSelectorSet(&relsSelectorSet);

    if (SUCCEEDED(hr))
    {
        for (UINT32 i = 0; i < count && SUCCEEDED(hr); i++)
        {
            // Add relationship type as as selection criterion to the selector set.
            // Since we sign the relationships by type, it guards against changes to the 
            // relationships we care about. However, relationships with different types
            // can be changed, added or removed later and not breaking the signature. If 
            // you want to be more strict, you can sign the entire relationships part
            // (see OPC_RELATIONSHIP_SIGN_PART) so that any change to the relationships
            // part will break the signature.
            hr = relsSelectorSet->Create(
                    OPC_RELATIONSHIP_SELECT_BY_TYPE,
                    relationshipTypes[i],
                    NULL
                    );
        }

        // Create and add the relationship reference to the set.
        if (SUCCEEDED(hr))
        {
            relsReferenceSet->Create(
                sourceUri, 
                NULL,       // use default digest method, which will be set by IOpcSigningOptions::SetDefaultDigestMethod()
                OPC_RELATIONSHIP_SIGN_USING_SELECTORS, 
                relsSelectorSet, 
                OPC_CANONICALIZATION_C14N, 
                NULL        // no need to get the output reference object
                );
        }
    }

    // Release resources
    if (relsSelectorSet)
    {
        relsSelectorSet->Release();
        relsSelectorSet = NULL;
    }

    return hr;
}

// This method demonstrates how you can add custom objects to the signature,
// and also sign the custom objects.
HRESULT
AddCustomObjectsToSign(
    IOpcSigningOptions* signingOptions
    )
{
    IOpcSignatureCustomObjectSet * customObjectSet = NULL;
    IOpcSignatureReferenceSet * customReferenceSet = NULL;
    IUri * referenceUri1 = NULL;
    IUri * referenceUri2 = NULL;

    UINT32 count = 0;

    // Custom objects allow applications to add extra information to the signature.
    // One possible usage may be adding trustable timestamp to the signature.
    // Note that the signing time provided by OPC digital signature spec is from the
    // clock of local machine where the signature is created. So it is not a trustable
    // time since the clock may be inaccurate or the user may have changed the time locally.  
    //
    // Here we use a fake timestamp as an example to show how you can use custom object. 
    // In real world you need to get a trustable timestamp from a timestamp server.
    //
    // To better understand this sample, you should open and view the signature XML stored in
    // the signature part in the signed package. The default file extension of the signature
    // part is .psdsxs.

    // Example of constructing a fully signed custom object. The entire
    // object XML markup will be signed because we will reference the Id of the
    // object element. 
    //
    // Note that it is important to include namespace declaration in the XML markup. 
    // Otherwise it is not a valid XML snippet that can be interpreted.
    UINT8 customObject1[] = 
        "<Object Id=\"idMyCustomObject_Timestamp1\" xmlns=\"http://www.w3.org/2000/09/xmldsig#\" xmlns:x=\"http://www.example.com/MusicBundle\">"
            "<x:Comment>This is a fake timestamp.</x:Comment>"
            "<x:EncodedTime>ABCDEFGHIJK</x:EncodedTime>"
        "</Object>";

    // Example of custructing a partially signed custom object. 
    UINT8 customObject2[] = 
        "<Object xmlns=\"http://www.w3.org/2000/09/xmldsig#\">"
            // The <TimeStamp> element XML markup will be signed, because we will 
            // reference its Id.
            "<TimeStamp Id=\"idMyCustomObject_Timestamp2\" xmlns=\"http://www.example.com/MusicBundle\">"
                "<Comment>This is a fake timestamp.</Comment>"
                "<EncodedTime>ABCDEFGHIJK</EncodedTime>"
            "</TimeStamp>"
            // Outside of the <TimeStamp> element is the unsigned portion of this custom
            // object. Updates (changes, addition or deletion) can be made in that portion
            // without breaking the signature.
            "<Extension xmlns=\"http://www.example.com/MusicBundle\">"
                "<Comment>Unsigned portion of XML. Can be updated without breaking the signature.</Comment>"
            "</Extension>"
        "</Object>";


    // Get the custom object set from signing options. We will add custom objects to
    // it so those custom objects will be created in the signature XML after Sign() is called.
    HRESULT hr = signingOptions->GetCustomObjectSet(&customObjectSet);
    
    // Create and add the custom object into the custom object set.
    if (SUCCEEDED(hr))
    {
        // Number of bytes of the custom object. Note that we need to minus 1 because the 
        // buffer has the last byte as string terminator ('\0').
        count = sizeof(customObject1)/sizeof(customObject1[0]) - 1;

        hr = customObjectSet->Create(customObject1, count, NULL);
    }
   
    // Create and add the custom object into the custom object set.
    if (SUCCEEDED(hr))
    {
        // Number of bytes of the custom object. Note that we need to minus 1 because the 
        // buffer has the last byte as string terminator ('\0').
        count = sizeof(customObject2)/sizeof(customObject2[0]) - 1;
     
        hr = customObjectSet->Create(customObject2, count, NULL);
    }

    // Adding custom objects into the custom object set will make them appear
    // in the signature xml. However, unless you also add references to them,
    // they will not be signed.
    //
    // Add reference to the custom objects so they will be signed.
    if (SUCCEEDED(hr))
    {
        hr = signingOptions->GetCustomReferenceSet(&customReferenceSet);
    }

    // Create reference to the first custom object. Note that the reference Uri is
    // constructed as "#" + the Id value of the element you want to sign. Here
    // "#idMyCustomObject_Timestamp1" refers to the first custom object. Its entire
    // XML markup will be signed.
    if (SUCCEEDED(hr))
    {
        hr = CreateUri(L"#idMyCustomObject_Timestamp1", Uri_CREATE_ALLOW_RELATIVE, 0, &referenceUri1);
    }

    if (SUCCEEDED(hr))
    {
        hr = customReferenceSet->Create(
                referenceUri1, 
                NULL,   // Id. No Id for the reference element
                L"http://www.w3.org/2000/09/xmldsig#Object", // Type
                NULL,   // Digest method. Use default digest method, which will be set by IOpcSigningOptions::SetDefaultDigestMethod()
                OPC_CANONICALIZATION_C14N,   // Use C14N to canonicalize the referenced XML markup before signing
                NULL    // no need to get the output signature reference object
                );
    }

    // Create reference to the element we want to sign in the second custom object. 
    // Note that the reference Uri is constructed as "#" + the Id value of the element 
    // you want to sign. Here "#idMyCustomObject_Timestamp2" refers to the <TimeStamp>
    // element inside of the second custom object. Only the <TimeStamp> XML markup, including
    // its nested elements will be signed.
    if (SUCCEEDED(hr))
    {
        hr = CreateUri(
                L"#idMyCustomObject_Timestamp2",
                Uri_CREATE_ALLOW_RELATIVE,
                0,
                &referenceUri2
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = customReferenceSet->Create(
                referenceUri2, 
                NULL,   // Id. No Id for the reference element
                L"http://www.example.com/MusicBundle#TimestampObject", // Type, user defined type
                NULL,   // Digest method. Use default digest method, which will be set by IOpcSigningOptions::SetDefaultDigestMethod()
                OPC_CANONICALIZATION_C14N,   // Use C14N to canonicalize the referenced XML markup before signing
                NULL    // no need to get the output signature reference object
                );
    }

    // Release resources
    if (customObjectSet)
    {
        customObjectSet->Release();
        customObjectSet = NULL;
    }

    if (customReferenceSet)
    {
        customReferenceSet->Release();
        customReferenceSet = NULL;
    }

    if (referenceUri1)
    {
        referenceUri1->Release();
        referenceUri1 = NULL;
    }

    if (referenceUri2)
    {
        referenceUri2->Release();
        referenceUri2 = NULL;
    }

    return hr;
}

// Pop up an certificate selection dialog and let user select a certificate for signing.
HRESULT
SelectCert(
    PCCERT_CONTEXT* certContext
    )
{
    typedef BOOL (WINAPI *fnCertSelectCertificate)(PCERT_SELECT_STRUCT_W);

    HMODULE hCryptDlgInst = LoadLibrary(L"CryptDlg.dll");

    if (hCryptDlgInst == NULL)
    {
        fwprintf(stderr, L"Cannot load CryptDlg.dll.\n");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HRESULT hr = S_OK;

    fnCertSelectCertificate  pfnCertSelectCertificate = 
        (fnCertSelectCertificate)GetProcAddress(hCryptDlgInst, "CertSelectCertificateW");

    if (pfnCertSelectCertificate)
    {
        HCERTSTORE hMySysStore = CertOpenSystemStore(NULL, L"MY");
        if (hMySysStore)
        {
            CERT_SELECT_STRUCT certSelect = {0};
            certSelect.dwSize = sizeof(CERT_SELECT_STRUCT);
            certSelect.hInstance = hCryptDlgInst;

            certSelect.szTitle = L"Select signing certificate.";
            certSelect.cCertStore = 1;
            certSelect.arrayCertStore = &hMySysStore;

            certSelect.cCertContext = 1;
            certSelect.arrayCertContext = certContext;

            BOOL result = pfnCertSelectCertificate(&certSelect);

            if (!result) 
            {
                hr = HRESULT_FROM_WIN32(GetLastError());

                if (SUCCEEDED(hr))
                {
                    // The user just clicked "Cancel".
                    fwprintf(stderr, L"You must select a certificate for signing to proceed.\n");
                    hr = E_FAIL;
                }
                else
                {
                    fwprintf(stderr, L"CertSelectCertificate() failed.\n");
                }
            }

            CertCloseStore(hMySysStore, 0);
        }
        else 
        {
            fwprintf(stderr, L"CertOpenSystemStore() failed.\n");
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else 
    {
        fwprintf(stderr, L"Cannot find CertSelectCertificateW function in CryptDlg.dll.\n");
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    FreeLibrary(hCryptDlgInst);
    return hr;
}

// Adding signing information to signing options to prepare for signing.
//
// Parameters:
//  opcFactory - the IOpcFactory instance
//  opcPackage - the package to sign
//  signingOptions - the signing options that will be filled in with signing information in this function
//  opcDigSigManager - the digital signature manager that will control the signing process
HRESULT
PrepareForSigning(
    IOpcFactory* opcFactory,
    IOpcPackage* opcPackage,
    IOpcSigningOptions* signingOptions,
    IOpcDigitalSignatureManager* opcDigSigManager
    )
{
    IOpcPart * albumArtPart = NULL;
    IOpcPart * tracklistPart = NULL;
    IOpcSignaturePartReferenceSet * partReferenceSet = NULL;
    IOpcSignatureRelationshipReferenceSet * relsReferenceSet = NULL;
    IOpcUri * rootUri = NULL;
    IOpcPartUri * albumArtPartUri = NULL;
    IOpcPartUri * tracklistPartUri = NULL;
    IOpcPartUri * signatureOriginPartUri = NULL;

    OpcPartArray trackParts;
    OpcPartArray lyricParts;

    // As defined by our sample signing policy, any root relationship with tracklist, 
    // album art or album website relationship type needs to be signed. 
    LPCWSTR rootRelationshipTypesToSign[] = 
    {
        g_tracklistRelationshipType, 
        g_albumArtRelationshipType, 
        g_albumWebsiteRelationshipType
    };
    
    // Get the parts that should be signed according to the custom signing policy.
    HRESULT hr = GetPartsToBeSigned(
                    opcPackage,
                    &trackParts,
                    &lyricParts,
                    &tracklistPart,
                    &albumArtPart
                    );

    // Get the part reference set so we can add references to parts we want to sign.
    if (SUCCEEDED(hr))
    {
        hr = signingOptions->GetSignaturePartReferenceSet(&partReferenceSet);
    }

    // Get the relationship reference set so we can add references to relationships we want to sign.
    if (SUCCEEDED(hr))
    {
        hr = signingOptions->GetSignatureRelationshipReferenceSet(&relsReferenceSet);
    }

    //
    // Create relationship references to root relationships that need to be signed.
    //
    if (SUCCEEDED(hr))
    {
        hr = opcFactory->CreatePackageRootUri(&rootUri);
    }

    // Add references to root relationships need-to-sign to the relationship reference set. 
    if (SUCCEEDED(hr))
    {
        hr = AddReferenceByRelationshipType(relsReferenceSet, rootUri, 3, rootRelationshipTypesToSign);
    }

    //
    // Create part reference to the album art part so it will be signed.
    //
    if (SUCCEEDED(hr))
    {
        hr = albumArtPart->GetName(&albumArtPartUri);
    }

    if (SUCCEEDED(hr))
    {
        hr = partReferenceSet->Create(
                albumArtPartUri, 
                NULL,   // use default digest method, which will be set by IOpcSigningOptions::SetDefaultDigestMethod()
                OPC_CANONICALIZATION_NONE,
                NULL    // no need to get the output part reference
                );
    }

    //
    // Create part reference to the tracklist part so it will be signed.
    //
    if (SUCCEEDED(hr))
    {
        hr = tracklistPart->GetName(&tracklistPartUri);
    }

    if (SUCCEEDED(hr))
    {
        hr = partReferenceSet->Create(
                tracklistPartUri, 
                NULL,   // use default digest method, which will be set by IOpcSigningOptions::SetDefaultDigestMethod()
                OPC_CANONICALIZATION_NONE,
                NULL    // no need to get the output part reference
                );
    }

    //
    // As defined by our sample signing policy, any tracklist part relationship with 
    // track relationship type needs to be signed. 
    // Add a reference to such a relationship to the relationship reference set. 
    //
    if (SUCCEEDED(hr))
    {
        hr = AddReferenceByRelationshipType(
                relsReferenceSet,
                tracklistPartUri,
                1,
                &g_trackRelationshipType
                );
    }

    if (SUCCEEDED(hr))
    {
        //
        // Create part reference to the track parts so they will be signed. 
        //
        UINT32 numberOfTracks = trackParts.GetCount();
    
        for (UINT32 i=0; i<numberOfTracks && SUCCEEDED(hr); i++)
        {
            IOpcPart * trackPart = NULL;
            IOpcPartUri * trackPartUri = NULL;
    
            hr = trackParts.GetAt(i, &trackPart);
            
            if (SUCCEEDED(hr))
            {
                hr = trackPart->GetName(&trackPartUri);
            }
    
            if (SUCCEEDED(hr))
            {
                hr = partReferenceSet->Create(
                        trackPartUri, 
                        NULL,   // use default digest method, which will be set by IOpcSigningOptions::SetDefaultDigestMethod()
                        OPC_CANONICALIZATION_NONE,
                        NULL    // no need to get the output part reference
                        );
            }
    
            //
            // As defined by our sample signing policy, any track part relationship with 
            // lyric relationship type needs to be signed. 
            // Add a reference to such a relationship to the relationship reference set. 
            //
            if (SUCCEEDED(hr))
            {
                hr = AddReferenceByRelationshipType(
                        relsReferenceSet,
                        trackPartUri,
                        1,
                        &g_lyricRelationshipType
                        );
            }
    
            if (trackPart)
            {
                trackPart->Release();
                trackPart = NULL;
            }
    
            if (trackPartUri)
            {
                trackPartUri->Release();
                trackPartUri = NULL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        //
        // Create part reference to the lyric parts so they will be signed. 
        //
        UINT32 numberOfLyrics = lyricParts.GetCount();
    
        for (UINT32 i=0; i<numberOfLyrics && SUCCEEDED(hr); i++)
        {
            IOpcPart * lyricPart = NULL;
            IOpcPartUri * lyricPartUri = NULL;
    
            hr = lyricParts.GetAt(i, &lyricPart);
    
            if (SUCCEEDED(hr))
            {
                hr = lyricPart->GetName(&lyricPartUri);
            }
    
            if (SUCCEEDED(hr))
            {
                hr = partReferenceSet->Create(
                        lyricPartUri, 
                        NULL,   // use default digest method, which will be set by IOpcSigningOptions::SetDefaultDigestMethod()
                        OPC_CANONICALIZATION_NONE,
                        NULL    // no need to get the output part reference
                        );
            }
    
            if (lyricPart)
            {
                lyricPart->Release();
                lyricPart = NULL;
            }
    
            if (lyricPartUri)
            {
                lyricPartUri->Release();
                lyricPartUri = NULL;
            }
        }
    }

    //
    // As defined by our sample signing policy, any signature origin part's relationship
    // with signature relationship type needs to be signed. 
    // In order to do that, we must define the signature origin part's name before calling Sign().
    //
    // Note that signing the signature relationships from the signature origin part is not required
    // by OPC Digital Signature spec. Doing so will prevent any new signatures from being added to 
    // the signed package in the future. It may be desirable or not depending on user's scenario
    // and should be called out in the signing policy of the custom file format. In general, if you
    // want to allow somebody else to sign the package after you sign it (countersign scenario),
    // then you shouldn't sign the signature origin part's signature relationships.
    //
    if (SUCCEEDED(hr))
    {
        hr = opcFactory->CreatePartUri(g_signatureOriginPartName, &signatureOriginPartUri);
    }
    
    // We assume the signature origin part uri is unique in the package. If not, Sign() will fail.
    if (SUCCEEDED(hr))
    {
        hr = opcDigSigManager->SetSignatureOriginPartName(signatureOriginPartUri);
    }

    // Add a reference to signature relationship to the relationship reference set so it will be signed.
    if (SUCCEEDED(hr))
    {
        hr = AddReferenceByRelationshipType(
                relsReferenceSet,
                signatureOriginPartUri,
                1,
                &g_signatureRelationshipType
                );
    }

    // The default certificate embedding option is OPC_CERTIFICATE_IN_CERTIFICATE_PART,
    // so if you are fine with the default you don't really need to make this call. 
    // Making it explicitly here to let you know you can change the embedding option by calling
    // SetCertificateEmbeddingOption(). Using OPC_CERTIFICATE_IN_CERTIFICATE_PART is recommended
    // because multiple signatures may share the same certificate if the certificate is stored in
    // a part. 
    if (SUCCEEDED(hr))
    {
        hr = signingOptions->SetCertificateEmbeddingOption(OPC_CERTIFICATE_IN_CERTIFICATE_PART);
    }

    // Advanced scenario: adding custom objects to the signature. 
    // Custom objects can add more information (like XAdES timestamp) to the signature
    // if needed. For basic scenarios, you can skip this step.
    if (SUCCEEDED(hr))
    {
        hr = AddCustomObjectsToSign(signingOptions);
    }

    // User must provide default digest method before calling Sign(). 
    if (SUCCEEDED(hr))
    {
        hr = signingOptions->SetDefaultDigestMethod(g_defaultDigestMethod);
    }

    // User must provide signature method before calling Sign(). 
    // User also needs to make sure the certificate he/she selected for signing supports
    // the signature method. Otherwise Sign() will fail. For more information on how to
    // find the supported signature methods in a certificate, check the
    // CryptXmlEnumAlgorithmInfo API.
    if (SUCCEEDED(hr))
    {
        hr = signingOptions->SetSignatureMethod(g_signatureMethod);
    }

    // Release resources
    if (albumArtPart)
    {
        albumArtPart->Release();
        albumArtPart = NULL;
    }

    if (tracklistPart)
    {
        tracklistPart->Release();
        tracklistPart = NULL;
    }

    if (partReferenceSet)
    {
        partReferenceSet->Release();
        partReferenceSet = NULL;
    }

    if (relsReferenceSet)
    {
        relsReferenceSet->Release();
        relsReferenceSet = NULL;
    }

    if (rootUri)
    {
        rootUri->Release();
        rootUri = NULL;
    }

    if (albumArtPartUri)
    {
        albumArtPartUri->Release();
        albumArtPartUri = NULL;
    }

    if (tracklistPartUri)
    {
        tracklistPartUri->Release();
        tracklistPartUri = NULL;
    }

    if (signatureOriginPartUri)
    {
        signatureOriginPartUri->Release();
        signatureOriginPartUri = NULL;
    }

    return hr;
}

HRESULT
SignMusicBundle(
    IOpcFactory* opcFactory
    )
{
    IOpcPackage * opcPackage = NULL;
    IOpcDigitalSignatureManager * opcDigSigManager = NULL;
    IOpcDigitalSignatureEnumerator * signatureEnumerator = NULL;
    IOpcSigningOptions * signingOptions = NULL;
    IOpcDigitalSignature * signature = NULL;

    // Load the input music bundle package.
    HRESULT hr = ReadPackageFromFile(g_unsignedFilePath, opcFactory, &opcPackage);

    // Create a digital signature manager for the package.
    if (SUCCEEDED(hr))
    {
        hr = opcFactory->CreateDigitalSignatureManager(opcPackage, &opcDigSigManager);
    }

    // First detect whether the package has been signed previously.
    // Our signing policy in this sample disallows countersignature (adding more
    // signatures to an already signed music bundle) so we signed the signature origin part’s 
    // signature relationships by relationship type. Adding new signature will
    // break the existing signature. 
    //
    // Note that it is not required or recommended for a signing policy to disallow
    // countersignature. If your signing policy allows countersignature, you shouldn't
    // sign the signature origin part’s signature relationships by relationship type. 
    if (SUCCEEDED(hr))
    {
        hr = opcDigSigManager->GetSignatureEnumerator(&signatureEnumerator);
    }

    if (SUCCEEDED(hr))
    {
        BOOL hasNext = FALSE;
        hr = signatureEnumerator->MoveNext(&hasNext);

        if (SUCCEEDED(hr) && hasNext)
        {
            fwprintf(
                stderr, 
                L"This music bundle has been signed. To sign a music bundle, it must have not been signed before.\n"
                );

            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = opcDigSigManager->CreateSigningOptions(&signingOptions);
    }

    // Configure signing options, adding references to parts and relationships that need to be signed.
    if (SUCCEEDED(hr))
    {
        hr = PrepareForSigning(opcFactory, opcPackage, signingOptions, opcDigSigManager);
    }

    // Let user select a certificate from the certificate store on the local machine.
    // We will use the certificate to sign the music bundle and the signer will be identified
    // by the certificate. It is not always necessary to pop up a UI dialog for user to select
    // a signer certificate. In a sever signing scenario (no human interaction), the sever process 
    // may have pre-defined certificate to use or pick a certificate from certificate store using
    // CertFindCertificateInStore API.
    //
    // Note that Sign() does not verify the certificate so user should verify the certificate like
    // making sure it is not expired, not in revocation lists, etc. User should also verify the 
    // certification path of the certificate to see whether it leads to a trusted root certificate.
    // The above can be done programmatically in production code. For more information, check
    // CertGetCertificateChain API.
    //
    // Also note that the certificate selected must support the signature method we use in this sample,
    // which is RSA-SHA1. RSA is the encryption algorithm. A user can check whether a certificate 
    // supports RSA by checking its public key type. SHA1 is the hash algorithm and is not related to
    // a certificate. Because of these, the signature algorithm can be determined based on
    // what certificate is used for signing, instead of using an arbitrary one like in this sample.
    // For more information on how to find supported signature algorithms in a certificate, check the
    // CryptXmlEnumAlgorithmInfo API.
    PCCERT_CONTEXT cert = NULL;
    if (SUCCEEDED(hr))
    {
        hr = SelectCert(&cert);
    }

    // Sign() will add signature related parts and relationships to the package object. 
    // If Sign() fails, it may leave unwanted parts and relationships in the package. 
    // However, the changes are not persisted until we call IOpcFactory::WritePackageToStream(). 
    // So you can simply discard the package object if Sign() fails and your original package
    // file on disk is intact. 
    //
    // However, if you just created the package from scratch or edited an existing package,
    // you should save the package before calling Sign() to prevent loss of data in case Sign()
    // fails. 
    //
    // For this sample, since the package has no changes that need to be saved at this point,
    // we will call Sign() immediately.
    if (SUCCEEDED(hr))
    {
        hr = opcDigSigManager->Sign(cert, signingOptions, &signature);
    }

    // Release the CERT_CONTEXT we got from SelectCert().
    if (cert)
    {
        CertFreeCertificateContext(cert);
    }

    // Save the signed music bundle.
    if (SUCCEEDED(hr))
    {
        hr = WritePackageToFile(g_signedFilePath, opcPackage, opcFactory);
    }

    if (SUCCEEDED(hr))
    {
        fwprintf(stdout, L"Music bundle has been signed successfully and saved to %s\n", g_signedFilePath);
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

    if (signingOptions)
    {
        signingOptions->Release();
        signingOptions = NULL;
    }

    if (signature)
    {
        signature->Release();
        signature = NULL;
    }

    return hr;
}
//</SnippetMusicBundleSig_cppSignWholePage>