// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <propsys.h>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

bool TestForPropertyKey(IPropertyStore *pps, REFPROPERTYKEY pk);
HRESULT LoadPropertyStoreFromStream(IStream *pstm, REFIID riid, void **ppv);
HRESULT SavePropertyStoreToStream(IPropertyStore *pps, IStream *pstm);
HRESULT CopyPropertyStores(IPropertyStore *ppsDest, IPropertyStore *ppsSource);
HRESULT ClonePropertyStoreToMemory(IPropertyStore *ppsSource, REFIID riid, void **ppv);
HRESULT SavePropertyStoreToString(IPropertyStore *pps, PWSTR *ppszBase64);

HRESULT SerializePropVariantAsString(REFPROPVARIANT propvar, PWSTR *ppszOut);
HRESULT DeserializePropVariantFromString(PCWSTR pszIn, PROPVARIANT *ppropvar);

HRESULT GetSafeSaveStream(IStream *pstmIn, IStream **ppstmTarget);

HRESULT GetBase64StringFromStream(IStream *pstm, PWSTR *ppszBase64);
HRESULT IStream_ReadToBuffer(IStream *pstm, UINT uMaxSize, BYTE **ppBytes, UINT *pcBytes);
