//<SnippetSetAuthor_cppHeadWordlibWholePage>
/*****************************************************************************
*
* File: wordlib.cpp
*
* Description:
* This file contains definitions of utility functions that wrap some common
* operations performed with wordprocessing documents. Wordprocessing documents
* conform to WordprocessingML standards found in Office Open XML Part 1:
* Fundamentals (ECMA-376 Part 1).
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

#include "stdio.h"
#include "windows.h"
#include "shlobj.h"
#include "msopc.h"
#include "msxml6.h"

#include "opclib.h"
#include "wordlib.h"

namespace wordlib
{

// The relationship type (office document relationship type) of the
// relationship that targets the Main Document part, as defined by the Office
// Open XML specification.
// Note: The relationship type of a relationship targeting the Main
// Document part is the same for all Office Open XML packages.
static const WCHAR g_officeDocumentRelationshipType[] =
    L"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument";

// The expected content type of the Main Document part, which specifies the
// wordprocessing document content type, as defined by the Office
// Open XML specification.
// Note:  It is necessary to look at the content type of the Main Document
// part to determine whether the package is a word processing document because
// the Main Document relationship type does not provide this information.
static const WCHAR g_wordProcessingContentType[] =
    L"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml";

// Namespaces used in XPath selection queries for a Main Document part of the
// wordprocessing document content type, as defined by the Office Open XML
// specification.
static const WCHAR g_wordSelectionNamespaces[] =
    L"xmlns:w='http://schemas.openxmlformats.org/wordprocessingml/2006/main\'";


//---------------------------------
// Helper methods

//////////////////////////////////////////////////////////////////////////////
// Description:
// Prints the text in a paragraph to the console. If the funtion cannot
// find text, nothing will be printed. This function is not intended to
// display complex wordprocessing documents.
//////////////////////////////////////////////////////////////////////////////
HRESULT 
ShowParagraph(
    IXMLDOMNode *paragraph
    )
{
    IXMLDOMNodeList * textNodeList = NULL;

    HRESULT hr = paragraph->selectNodes(L".//w:t", &textNodeList);

    while (SUCCEEDED(hr))
    {
        IXMLDOMNode * currentNode = NULL;
        BSTR text = NULL;
        hr = textNodeList->nextNode(&currentNode);

        if (hr == S_OK)
        {
            hr = currentNode->get_text(&text);

            if (hr == S_OK)
            {
                // There is text to print, so print it to the console.
                wprintf(L"%s ", text);
            }
            // An error code, or S_FALSE was returned.
            // S_FALSE indicates no text for current text node.
        }
        else
        {
            // An error code, or S_FALSE was returned.
            // S_FALSE indicates no more text nodes.
            break;
        }

        // Release resources
        if (currentNode)
        {
            currentNode->Release();
            currentNode = NULL;
        }
        SysFreeString(text);
    }

    wprintf(L"\n");

    // Release resources
    if (textNodeList)
    {
        textNodeList->Release();
        textNodeList = NULL;
    }

    return hr;
}

//---------------------------------
// Library methods.

//////////////////////////////////////////////////////////////////////////////
// Description:
// Find a Main Document part in a package.
//
// Note: This function selects the first, arbitrary relationship of the
// office document relationship type and returns a pointer to target part of
// that relationship.
//////////////////////////////////////////////////////////////////////////////
HRESULT 
FindDocumentInPackage(
    IOpcPackage *package,
    IOpcPart   **documentPart
    )
{
    // Find the Main Document part of the word processing document.
    // Note: Check the content type of the found part to ensure 
    // that this is a word processing document.
    return opclib::FindPartByRelationshipType(
                package,
                g_officeDocumentRelationshipType,
                g_wordProcessingContentType,
                documentPart
                );
}

//////////////////////////////////////////////////////////////////////////////
// Description
// Prints text from the beginning paragraphs of the Main Document to console
// and accepts a maximum number of paragraphs to print.
//////////////////////////////////////////////////////////////////////////////
HRESULT
PrintBeginningParagraphs(
    IOpcPart *documentPart,
    DWORD maxParagraphCount
    )
{
    IXMLDOMDocument2 * documentDom = NULL;
    IXMLDOMNodeList * paragraphNodeList = NULL;

    HRESULT hr = opclib::DOMFromPart(
                    documentPart,
                    g_wordSelectionNamespaces,
                    &documentDom
                    );

    if (SUCCEEDED(hr))
    {
        // Select paragraph nodes to print.
        hr = documentDom->selectNodes(L"//w:p", &paragraphNodeList);
    }

    // Print paragraphs.
    // Stop printing when max number is reached, when an error code is
    // returned, or when S_FALSE is returned indicating end of list.
    while (hr == S_OK && maxParagraphCount > 0)
    {
        IXMLDOMNode * currentParagraph = NULL;

        hr = paragraphNodeList->nextNode(&currentParagraph);

        if (hr == S_OK)
        {
            // There is a paragraph to print, so print it.
            hr = ShowParagraph(currentParagraph);
        }
        // An error code, or S_FALSE was returned.
        maxParagraphCount--;

        // Release resources
        if (currentParagraph)
        {
            currentParagraph->Release();
            currentParagraph = NULL;
        }
    }

    // Release resources
    if (documentDom)
    {
        documentDom->Release();
        documentDom = NULL;
    }

    if (paragraphNodeList)
    {
        paragraphNodeList->Release();
        paragraphNodeList = NULL;
    }

    return hr;
}

} // namespace wordlib
//</SnippetSetAuthor_cppWordlibWholePage>