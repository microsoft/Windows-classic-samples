// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


// Miscellaneous helper functions.

#include "Capture.h"
#include <wincodec.h>

HRESULT CopyAttribute(IMFAttributes *pSrc, IMFAttributes *pDest, const GUID& key)
{
    PROPVARIANT var;
    PropVariantInit( &var );
    HRESULT hr = pSrc->GetItem(key, &var);
    if (SUCCEEDED(hr))
    {
        hr = pDest->SetItem(key, var);
        PropVariantClear(&var);
    }
    return hr;
}


// Creates a compatible video format with a different subtype.

HRESULT CloneVideoMediaType(IMFMediaType *pSrcMediaType, REFGUID guidSubType, IMFMediaType **ppNewMediaType)
{
    IMFMediaType *pNewMediaType = NULL;

    HRESULT hr = MFCreateMediaType(&pNewMediaType);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pNewMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);     
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pNewMediaType->SetGUID(MF_MT_SUBTYPE, guidSubType);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CopyAttribute(pSrcMediaType, pNewMediaType, MF_MT_FRAME_SIZE);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CopyAttribute(pSrcMediaType, pNewMediaType, MF_MT_FRAME_RATE);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CopyAttribute(pSrcMediaType, pNewMediaType, MF_MT_PIXEL_ASPECT_RATIO);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CopyAttribute(pSrcMediaType, pNewMediaType, MF_MT_INTERLACE_MODE);
    if (FAILED(hr))
    {
        goto done;
    }

    *ppNewMediaType = pNewMediaType;
    (*ppNewMediaType)->AddRef();

done:
    SafeRelease(&pNewMediaType);
    return hr;
}

// Creates a JPEG image type that is compatible with a specified video media type.

HRESULT CreatePhotoMediaType(IMFMediaType *pSrcMediaType, IMFMediaType **ppPhotoMediaType)
{
    *ppPhotoMediaType = NULL;

    const UINT32 uiFrameRateNumerator = 30;
    const UINT32 uiFrameRateDenominator = 1;

    IMFMediaType *pPhotoMediaType = NULL;

    HRESULT hr = MFCreateMediaType(&pPhotoMediaType);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pPhotoMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Image);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pPhotoMediaType->SetGUID(MF_MT_SUBTYPE, GUID_ContainerFormatJpeg);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CopyAttribute(pSrcMediaType, pPhotoMediaType, MF_MT_FRAME_SIZE);
    if (FAILED(hr))
    {
        goto done;
    }

    *ppPhotoMediaType = pPhotoMediaType;
    (*ppPhotoMediaType)->AddRef();

done:
    SafeRelease(&pPhotoMediaType);
    return hr;
}

void ShowError(HWND hwnd, PCWSTR szMessage, HRESULT hr)
{
    wchar_t msg[256];

    if (SUCCEEDED(StringCchPrintfW(msg, ARRAYSIZE(msg),  L"%s (hr = 0x%X)", szMessage, hr)))
    {
        MessageBox(hwnd, msg, NULL, MB_OK | MB_ICONERROR);
    }
}


void ShowError(HWND hwnd, UINT id, HRESULT hr)
{
    wchar_t msg[256];

    if (0 != LoadString(GetModuleHandle(NULL), id, msg, ARRAYSIZE(msg)))
    {
        ShowError(hwnd, msg, hr);
    }
}



void SetMenuItemText(HMENU hMenu, UINT uItem, _In_ PWSTR pszText)
{
    MENUITEMINFO mii = {};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = pszText;

    SetMenuItemInfo(hMenu, uItem, FALSE, &mii);
}


VOID DbgPrint(PCTSTR format, ...)
{
    va_list args;
    va_start(args, format);

    TCHAR string[MAX_PATH];

    if (SUCCEEDED(StringCbVPrintf(string, sizeof(string), format, args)))
    {
        OutputDebugString(string);
    }
    else
    {
        DebugBreak();
    }
}
