//////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  DisplayString.h
//
//          CDisplayString declaration.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "StringRange.h"
#include "PointerArray.h"

class CDisplayString
{
public:
    CDisplayString() { }
    ~CDisplayString() { }

    int Count() { }
    // VOID SetDisplayString(int iIndex, WCHAR* pchText, USHORT cchMax, TF_DISPLAYATTRIBUTE tfDisplayAttribute) { }
    // VOID GetDisplayString(int iIndex, WCHAR* pchText, USHORT cchMax, USHORT* pch, TF_DISPLAYATTRIBUTE tfDisplayAttribute) { }
    VOID SetLogicalFont(LOGFONTW LogFont) { }
    VOID GetLogicalFont(LOGFONTW* pLogFont) { }

private:
    //typedef struct _DISPLAY_STRING {
    //    CStringRange         _StringRange;                   // Unicode string.
    //                                                         // Length and MaximumLength is in character count.
    //                                                         // Buffer doesn't add zero terminate.
    //    TF_DISPLAYATTRIBUTE  _tfDisplayAttribute;            // Display attribute for each array.
    //} DISPLAY_STRING;

    //
    // Array of DISPLAY_STRING
    //
    //CPointerArray<DISPLAY_STRING>  _pDisplayString;

    // Logical font
    LOGFONTW                       _logfont;
};
