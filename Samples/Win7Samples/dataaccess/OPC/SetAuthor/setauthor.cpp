//<SnippetSetAuthor_cppSetAuthorWholePage>
/*****************************************************************************
*
* File: setauthor.cpp
*
* Description:
* This sample is a simple console application that might be used
* as a starting-point for an application that uses the Packaging API. This
* sample demonstrates the modification of an existing package, a word
* document.
*
* For the sake of simplicity, the sample does not update the last
* modified date, though this would be appropriate, nor does it modify history
* or other document parts and properties that may contain personally
* identifiable information. This sample is not sufficient to remove all
* authorship information from a document.
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
****************************************************************************/

#include "stdio.h"
#include "windows.h"
#include "shlobj.h"
#include "msopc.h"
#include "msxml6.h"

#include "opclib.h"
#include "wordlib.h"

//////////////////////////////////////////////////////////////////////////////
// Description:
// Show the first few paragraphs & author information from the specified
// Office Open XML word processing document and, if requested, update the
// author information & save to the specified output file.
//////////////////////////////////////////////////////////////////////////////
HRESULT
SetAuthorMain(
      LPCWSTR filename, // Name of file to read.
      LPCWSTR newAuthor, // Optional. New author name. If specified, targetFile must also be specified.
      LPCWSTR targetFile // Optional. Name of output file. If specified, newAuthor must also be
                         // specified.
      )
{
    IOpcPackage * package = NULL;

    // A representation of the part containing word processing document markup.
    IOpcPart * documentPart = NULL; 

//<SnippetCreateFactory_cppCreateFactory>
    IOpcFactory * factory = NULL;

    // Create a new factory.
    HRESULT hr = CoCreateInstance(
                    __uuidof(OpcFactory),
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof(IOpcFactory),
                    (LPVOID*)&factory
                    );

//</SnippetCreateFactory_cppCreateFactory>

    if (SUCCEEDED(hr))
    {
        // Load the package file.
        hr = opclib::LoadPackage(factory, filename, &package);
    }

    if (SUCCEEDED(hr)) 
    {
        // Locate the Main Document part that contains wordprocessing markup.
        hr = wordlib::FindDocumentInPackage(package, &documentPart);
    }

    // Print a preview of document contents and properties.
    if (SUCCEEDED(hr))
    {
        DWORD maxParagraphCount = 3;
        wprintf(L"\n----------------- preview of contents ----------------\n");
        hr = wordlib::PrintBeginningParagraphs(documentPart, maxParagraphCount);
    }
    if (SUCCEEDED(hr))
    {
        wprintf(L"------------------- core properties ------------------\n");
        hr = opclib::PrintCoreProperties(package);
        wprintf(L"------------------------------------------------------\n\n");
    }

    // If newAuthor and targetFile are specified, modify the "author" &
    // "last modified by" properties to the new author and save the altered
    // package by writing the package to disk.
    if (SUCCEEDED(hr) && newAuthor && targetFile)
    {
        // New IOpcPackage interface pointer, for the package to create.
        IOpcPackage * newPackage = NULL;

        // Populate new package with original package data except "author" and
        // "last modified by" properties will reflect the new author.
        hr = opclib::SetAuthorAndModifiedBy(package, newAuthor);

        if (SUCCEEDED(hr))
        {
            // Save the package as a new file. This is done because data from the
            // original package may need to be read while the changes are saved.
            hr = opclib::SavePackage(factory, package, targetFile);
        }

        // The following code loads and displays the new package file to demonstrate
        // that the property changes have been applied.
        if (SUCCEEDED(hr))
        {
            wprintf(L"\n--- New author information saved to '%s' ---\n\n", targetFile);

            hr = opclib::LoadPackage(factory, targetFile, &newPackage);
        }
        if (SUCCEEDED(hr))
        {
            hr = opclib::PrintCoreProperties(newPackage);
        }

        // Release resources
        if (newPackage)
        {
            newPackage->Release();
            newPackage = NULL;
        }
    }

    // Release resources
    if (package)
    {
        package->Release();
        package = NULL;
    }

    if (documentPart)
    {
        documentPart->Release();
        documentPart = NULL;
    }

    if (factory)
    {
        factory->Release();
        factory = NULL;
    }

    return hr;
}

// Entry point for the sample
int
wmain(
    int argc, 
    wchar_t* argv[]
    )
{
    if (argc != 2 && argc != 4)
    {
        wprintf(L"Usage: Setauthor <filename> [<new author> <targetfile>]\n");   
        exit(0);
    }

    // Original file name.
    LPWSTR pFileName = argv[1];

    // New author name.
    LPWSTR pNewAuthor = NULL;

    // New author and other, original file data saved to new package file.
    LPWSTR pTargetFile = NULL;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        if (argc == 4)
        {
            pNewAuthor = argv[2];
            pTargetFile = argv[3];
        }

        // Display document content and properties. If requested, change
        // properties and save changes to document as a new file.
        hr = SetAuthorMain(
                pFileName,
                pNewAuthor,
                pTargetFile
                );

        CoUninitialize();
    }

    if (FAILED(hr))
    {
        wprintf(L"Error: 0x%.8X\n", hr);
    }

    return SUCCEEDED(hr) ? 0 : 1;
}
//</SnippetSetAuthor_cppSetAuthorWholePage>
