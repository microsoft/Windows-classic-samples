//<SnippetMusicBundleSig_cppUtilWholePage>
/*****************************************************************************
*
* File: Util.h
*
* Description: 
* Utility functions for Packaging APIs.
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

HRESULT
GetRelationshipTargetPart(
    IOpcPartSet* partSet,
    IOpcRelationship* relationship,
    LPCWSTR expectedContentType,
    IOpcPart** targetedPart
    )
{
    // Check target mode of the relationship. It must be internal for the target to be a part.
    OPC_URI_TARGET_MODE targetMode;
    IOpcUri * sourceUri = NULL;
    IUri * targetUri = NULL;
    IOpcPartUri * tracklistPartUri = NULL;
    LPWSTR contentType = NULL;
    BOOL partExists = FALSE;

    HRESULT hr = relationship->GetTargetMode(&targetMode);

    if (SUCCEEDED(hr) && targetMode != OPC_URI_TARGET_MODE_INTERNAL)
    {
        // Check whether the TargetMode of the relationship is internal. If it is not
        // internal, its target cannot be a part inside of the package.
    
        LPWSTR relationshipType = NULL;
        relationship->GetRelationshipType(&relationshipType);

        fwprintf(
            stderr, 
            L"Invalid music bundle package: relationship with type %s must have Internal target mode.\n",
            relationshipType
            );

        hr = E_FAIL;

        CoTaskMemFree(static_cast<LPVOID>(relationshipType));
    }

    // In order to get the target part uri, we need to resolve the target uri 
    // based on the source uri, because the target uri may be a relative uri.
    if (SUCCEEDED(hr))
    {
        hr = relationship->GetSourceUri(&sourceUri);
    }

    if (SUCCEEDED(hr))
    {
        hr = relationship->GetTargetUri(&targetUri);
    }

    if (SUCCEEDED(hr))
    {
        hr = sourceUri->CombinePartUri(targetUri, &tracklistPartUri);
    }

    if (SUCCEEDED(hr))
    {
        hr = partSet->PartExists(tracklistPartUri, &partExists);
    }

    if (SUCCEEDED(hr) && !partExists)
    {
        fwprintf(
            stderr, 
            L"Invalid music bundle package: the target part of relationship does not exist.\n"
            );

        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        hr = partSet->GetPart(tracklistPartUri, targetedPart);
    }

    // Check content type of the part. Make sure it matches the expected part content type.
    if (SUCCEEDED(hr))
    {
        hr = (*targetedPart)->GetContentType(&contentType);
    }

    if (SUCCEEDED(hr))
    {
        if (wcscmp(contentType, expectedContentType) != 0)
        {
            fwprintf(
                stderr, 
                L"Invalid music bundle package: the target part does not have correct content type.\n"
                );

            hr = E_FAIL;
        }
    }

    // Release resources
    if (sourceUri)
    {
        sourceUri->Release();
        sourceUri = NULL;
    }

    if (targetUri)
    {
        targetUri->Release();
        targetUri = NULL;
    }

    if (tracklistPartUri)
    {
        tracklistPartUri->Release();
        tracklistPartUri = NULL;
    }

    CoTaskMemFree(static_cast<LPVOID>(contentType));

    return hr;
}

HRESULT
GetPartByRelationshipType(
    IOpcPartSet* partSet,
    IOpcRelationshipSet* relsSet,
    LPCWSTR relationshipType,
    LPCWSTR expectedContentType,
    IOpcPart** targetedPart
    )
{
    IOpcRelationshipEnumerator * relsEnumerator = NULL;
    BOOL hasNext = FALSE;
    int count = 0;

    HRESULT hr = relsSet->GetEnumeratorForType(relationshipType, &relsEnumerator);

    while (
        SUCCEEDED(hr)
        &&
        SUCCEEDED(hr = relsEnumerator->MoveNext(&hasNext))
        &&
        hasNext
        )
    {
        IOpcRelationship * relationship = NULL;

        count++;

        if (count > 1)
        {
            // Our custom format for music bundle expects only one relationship with specific
            // type exists in a relationships part. Note that it is not an OPC rule.
            fwprintf(
                stderr, 
                L"Invalid music bundle package: cannot have more than 1 relationship with type: %s.\n",
                relationshipType
                );

            hr = E_FAIL;
            break;
        }

        if (SUCCEEDED(hr))
        {
            hr = relsEnumerator->GetCurrent(&relationship);
        }

        if (SUCCEEDED(hr))
        {
            hr = GetRelationshipTargetPart(partSet, relationship, expectedContentType, targetedPart);
        }

        // Release resources
        if (relationship)
        {
            relationship->Release();
            relationship = NULL;
        }
    }
    
    if (SUCCEEDED(hr))
    {
         if (count == 0)
         {
             fwprintf(
                 stderr, 
                 L"Invalid music bundle package: relationship with type %s does not exist.\n",
                 relationshipType
                 );

             hr = E_FAIL;
         }
    }

    // Release resources
    if (relsEnumerator)
    {
        relsEnumerator->Release();
        relsEnumerator = NULL;
    }

    return hr;
}

HRESULT
ReadPackageFromFile(
    LPCWSTR filename,
    IOpcFactory* opcFactory,
    IOpcPackage** opcPackage
    )
{
    IStream * fileStream = NULL;

    // First we create a stream over the file.
    HRESULT hr = opcFactory->CreateStreamOnFile(
                    filename,
                    OPC_STREAM_IO_READ, 
                    NULL,         // use default security attribute
                    FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAttributes
                    &fileStream             // the created stream object over the file
                    );

    // Then we read the package from the stream. 
    if (SUCCEEDED(hr))
    {
        hr = opcFactory->ReadPackageFromStream(
                fileStream, 
                OPC_READ_DEFAULT,
                opcPackage
                );
    }

    if (FAILED(hr))
    {
        fwprintf(stderr, L"Failed to load package file: %s.\n", filename);
    }

    // Release resources
    if (fileStream)
    {
        fileStream->Release();
        fileStream = NULL;
    }

    return hr;
}

HRESULT
WritePackageToFile(
    LPCWSTR filename,
    IOpcPackage* opcPackage,
    IOpcFactory* opcFactory
    )
{
    IStream * fileStream = NULL;

    HRESULT hr = opcFactory->CreateStreamOnFile(
                    filename,     // output file
                    OPC_STREAM_IO_WRITE,
                    NULL,         // use default security attribute
                    FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAttributes
                    &fileStream             // the created stream object over the file
                    );

    // Then we write the package to the stream. 
    if (SUCCEEDED(hr))
    {
        hr = opcFactory->WritePackageToStream(
                opcPackage,
                OPC_WRITE_DEFAULT,
                fileStream
                );
    }

    // Release resources
    if (fileStream)
    {
        fileStream->Release();
        fileStream = NULL;
    }

    return hr;
}


HRESULT
SaveCertificateToFile(
    PCCERT_CONTEXT cert,
    LPCWSTR filename
    )
{
    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

    DWORD written = 0;
    HRESULT hr = S_OK;

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        fwprintf(stderr, L"CreateFile failed with hr=0x%x\n", hr);
    }

    if (SUCCEEDED(hr))
    {
        if (!WriteFile(hFile, cert->pbCertEncoded, cert->cbCertEncoded, &written, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            fwprintf(stderr, L"WriteFile failed with hr=0x%x\n", hr);
        }
        else if (written != cert->cbCertEncoded)
        {
            hr = E_UNEXPECTED;
            fwprintf(stderr, L"WriteFile did not write all the bytes.\n");
        }
    }

    // Release resources
    CloseHandle(hFile);
    return hr;
}
//</SnippetMusicBundleSig_cppUtilWholePage>