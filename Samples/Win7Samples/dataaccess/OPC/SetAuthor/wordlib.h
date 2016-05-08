//<SnippetSetAuthor_hWordlibWholePage>
/*****************************************************************************
*
* File: wordlib.h
*
* Description:
* This file contains declarations of utility functions that wrap some common
* operations performed with wordprocessing documents. Wordprocessing documents
* conform to WordprocessingML standards found in Office Open XML Part 1:
* Fundamentals.
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

#pragma once

namespace wordlib
{

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
    );

//////////////////////////////////////////////////////////////////////////////
// Description
// Prints text from the beginning paragraphs of the Main Document to console
// and accepts a maximum number of paragraphs to print.
//////////////////////////////////////////////////////////////////////////////
HRESULT
PrintBeginningParagraphs(
    IOpcPart *documentPart,
    DWORD maxParagraphCount
    );

}
//</SnippetSetAuthor_hWordlibWholePage>