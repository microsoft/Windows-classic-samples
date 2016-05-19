//<SnippetMusicBundle_hUtilWholePage>
/*****************************************************************************
*
* File: Util.h
*
* Description: This file contains declarations for helper classes and functions.
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

#define countof(x) (sizeof(x)/sizeof(*(x)))

#define MAX_BUFFER_SIZE 1024

HRESULT
GetLastErrorAsHResult(
    VOID
    );

HRESULT
GetRelationshipTargetPart(
    IOpcPartSet         *partSet,
    IOpcRelationship    *rels,
    LPCWSTR              expectedContentType,
    IOpcPart           **targetedPart
    );

HRESULT
GetRelationshipByType(
    IOpcRelationshipSet   *relsSet,
    LPCWSTR                relationshipType,
    IOpcRelationship     **targetedRelationship
    );

HRESULT
GetFullFileName(
    LPCWSTR        filePath,
    LPCWSTR        fileName,
    LPWSTR        *fullFileName
    );

HRESULT
CreateDirectoryFromPartName(
    LPCWSTR        filePath,
    LPCWSTR        partName,
    LPWSTR        *fullFileName
    );

HRESULT
WriteFileContentToPart(
    IOpcFactory     *opcFactory,
    LPCWSTR          filePath,
    LPCWSTR          fileName,
    IOpcPart        *opcPart
    );

HRESULT
DisplayStreamContent(
    LPCWSTR           title,
    IStream          *stream
    );

HRESULT
WritePartContentToFile(
    IOpcFactory     *opcFactory,
    LPCWSTR          pathName,
    IOpcPart        *opcPart
    );
//</SnippetMusicBundle_hUtilWholePage>