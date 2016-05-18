//<SnippetSetAuthor_hUtilWholePage>
/*****************************************************************************
*
* File: util.h
*
* Description: This file contains declarations and definitions for simple COM
* helper classes.
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

// Non-throwing, simplified alternative to the ATL Variant class
class AutoVariant : 
    public VARIANT
{

public:

    AutoVariant()
    {
        VariantInit(this);
    }

    ~AutoVariant()
    {
        VariantClear(this);
    }

    HRESULT
    SetBSTRValue(
        LPCWSTR sourceString
        )
    {
        VariantClear(this);

        V_VT(this) = VT_BSTR;
        V_BSTR(this) = SysAllocString(sourceString);

        if (!V_BSTR(this))
        {
            return E_OUTOFMEMORY;
        }

        return S_OK;
    }

    void
    SetObjectValue(
        IUnknown *sourceObject
        )
    {
        VariantClear(this);

        V_VT(this) = VT_UNKNOWN;
        V_UNKNOWN(this) = sourceObject;

        if (V_UNKNOWN(this))
        {
            V_UNKNOWN(this)->AddRef();
        }
    }

};
//</SnippetSetAuthor_hUtilWholePage>