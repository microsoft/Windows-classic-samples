// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "../stdafx.h"
#include "../MainFrm.h"

#include <uiribbon.h>
#include <uiribbonpropertyhelpers.h>

#include "font.h"
#include "ribbon.h"
#include "../HTMLEdDoc.h"
#include "../HTMLEdView.h"


HRESULT SetFont(CMainFrame* pFrame, IPropertyStore* pStore)
{
    HRESULT hr = S_OK;

    // Get the HTML editor.
    CHTMLEdView *pView = (CHTMLEdView*)pFrame->GetActiveView();
    if (pView == NULL)
    {
        return E_FAIL;
    }
    
    // Set Font Face.
    PROPVARIANT face;
    hr = pStore->GetValue(UI_PKEY_FontProperties_Family, &face);
    if (FAILED(hr))
    {
        return hr;
    }

    CString strFontFace(face.pwszVal);
    hr = pView->SetFontFace(strFontFace.GetBuffer());
    if (FAILED(hr))
    {
        return hr;
    }

    PropVariantClear(&face);
        
    // Set Font Size.
    PROPVARIANT size;
    hr = pStore->GetValue(UI_PKEY_FontProperties_Size, &size);
    if (FAILED(hr))
    {
        return hr;
    }

    USHORT uSize;
    VarUI2FromDec(&size.decVal, &uSize);
    hr = pView->SetFontSize(uSize - 7);
    if (FAILED(hr))
    {
        return hr;
    }

    PropVariantClear(&size);
    return hr;
}

HRESULT UpdateFont(CMainFrame* pFrame, IPropertyStore* pStore)
{
    HRESULT hr = S_OK;

    // Get the HTML editor.
    CHTMLEdView *pView = (CHTMLEdView*)pFrame->GetActiveView();
    if (pView == NULL)
    {
        return E_FAIL;
    }

    // Update Font Face control.
    CString strFace;
    hr = pView->GetFontFace(strFace);
    if (FAILED(hr))
    {
        return hr;
    }

    int iLength = strFace.GetLength();
    LPWSTR pStr = (LPWSTR)CoTaskMemAlloc((iLength+1) * sizeof(WCHAR));

    if (pStr)
    {
        PROPVARIANT face;
        face.vt = VT_LPWSTR;
        face.pwszVal = pStr;

        MultiByteToWideChar(CP_ACP, 0, strFace.GetBuffer(), -1, face.pwszVal, iLength);
        face.pwszVal[iLength] = L'\0';
        hr = pStore->SetValue(UI_PKEY_FontProperties_Family, face);

        PropVariantClear(&face);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Update Font Size control.
    PROPVARIANT size;
    short s;
    hr = pView->GetFontSize(s);
    if (FAILED(hr))
    {
        return hr;
    }

    s += 7;
    
    hr = VarDecFromI2(s, (DECIMAL*)&size);
    if (FAILED(hr))
    {
        return hr;
    }

    size.vt = VT_DECIMAL;
    
    hr = pStore->SetValue(UI_PKEY_FontProperties_Size, size);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = PropVariantClear(&size);
    if (FAILED(hr))
    {
        return hr;
    }

    return pStore->Commit();    
}
