// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <shlobj.h>
#include <propsys.h>
#include <propvarutil.h>
#include "PropertyStoreHelpers.h"

#define MAP_ENTRY(x) {L#x, x}

HRESULT CopyPropertyStores(IPropertyStore *ppsDest, IPropertyStore *ppsSource)
{
    HRESULT hr = S_OK; // ok if no properties to copy

    int iIndex = 0;
    PROPERTYKEY key;
    while (SUCCEEDED(hr) && (S_OK == ppsSource->GetAt(iIndex++, &key)))
    {
        PROPVARIANT propvar;
        hr = ppsSource->GetValue(key, &propvar);
        if (SUCCEEDED(hr))
        {
            hr = ppsDest->SetValue(key, propvar);
            PropVariantClear(&propvar);
        }
    }
    return hr;
}

// returns an instance of a memory property store (IPropertyStore) or related interface in the output
HRESULT ClonePropertyStoreToMemory(IPropertyStore *ppsSource, REFIID riid, void **ppv)
{
    *ppv = NULL;
    IPropertyStore *ppsMemory;
    HRESULT hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&ppsMemory));
    if (SUCCEEDED(hr))
    {
        hr = CopyPropertyStores(ppsMemory, ppsSource);
        if (SUCCEEDED(hr))
        {
            hr = ppsMemory->QueryInterface(riid, ppv);
        }
        ppsMemory->Release();
    }
    return hr;
}

// IPersistSerializedPropStorage implies the standard property store
// seralization format supported by the memory property store. this function
// saves the property store into a stream using that format
//
// on win7 use IPersistSerializedPropStorage2 methods here

HRESULT SaveSerializedPropStorageToStream(IPersistSerializedPropStorage *psps, IStream *pstm)
{
    IPersistStream *pPersistStream;
    HRESULT hr = psps->QueryInterface(IID_PPV_ARGS(&pPersistStream));
    if (SUCCEEDED(hr))
    {
        // implement size limit here
        hr = pPersistStream->Save(pstm, TRUE);
        pPersistStream->Release();
    }
    return hr;
}

// seralizes a property store either directly or by creating a copy and using the
// memory property store and IPersistSerializedPropStorage

HRESULT SavePropertyStoreToStream(IPropertyStore *pps, IStream *pstm)
{
    IPersistSerializedPropStorage *psps;
    HRESULT hr = pps->QueryInterface(IID_PPV_ARGS(&psps));
    if (SUCCEEDED(hr))
    {
        // implement size limit here
        hr = SaveSerializedPropStorageToStream(psps, pstm);
        psps->Release();
    }
    else
    {
        // this store does not suppot serialization directly, copy it to
        // one that does and save it
        hr = ClonePropertyStoreToMemory(pps, IID_PPV_ARGS(&psps));
        if (SUCCEEDED(hr))
        {
            hr = SaveSerializedPropStorageToStream(psps, pstm);
            psps->Release();
        }
    }
    return hr;
}


HRESULT SavePropertyStoreToString(IPropertyStore *pps, PWSTR *ppszBase64)
{
    *ppszBase64 = NULL;

    IStream *pstm;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pstm);
    if (SUCCEEDED(hr))
    {
        hr = SavePropertyStoreToStream(pps, pstm);
        if (SUCCEEDED(hr))
        {
            IStream_Reset(pstm);
            hr = GetBase64StringFromStream(pstm, ppszBase64);
        }
        pstm->Release();
    }
    return hr;
}

bool TestForPropertyKey(IPropertyStore *pps, REFPROPERTYKEY pk)
{
    PROPVARIANT pv;
    bool fHasPropertyKey = SUCCEEDED(pps->GetValue(pk, &pv)) && (pv.vt != VT_EMPTY);
    PropVariantClear(&pv);
    return fHasPropertyKey;
}

// property handlers should indicate that they support safe safe by providing
// the "ManualSafeSave" attribute on their CLSID value in the registry.
// other code that saves files should use this to get safe save functionality
// when avaiable.
//
// uses IDestinationStreamFactory to discover the stream to save in.
// after a successful return from this function save to *ppstmTarget
// and then call Commit(STGC_DEFAULT) on pstmIn.

HRESULT GetSafeSaveStream(IStream *pstmIn, IStream **ppstmTarget)
{
    *ppstmTarget = NULL;

    IDestinationStreamFactory *pdsf;
    HRESULT hr = pstmIn->QueryInterface(&pdsf);
    if (SUCCEEDED(hr))
    {
        hr = pdsf->GetDestinationStream(ppstmTarget);
        pdsf->Release();
    }

    if (FAILED(hr))
    {
        // fall back to original stream
        hr = pstmIn->QueryInterface(ppstmTarget);
        if (SUCCEEDED(hr))
        {
            // we need to reset and truncate it, otherwise we're left
            // with stray bits.
            hr = IStream_Reset(*ppstmTarget);
            if (SUCCEEDED(hr))
            {
                ULARGE_INTEGER uliNull = {0};
                hr = (*ppstmTarget)->SetSize(uliNull);
            }

            if (FAILED(hr))
            {
                SafeRelease(ppstmTarget);
            }
        }
    }

    return hr;
}

HRESULT GetStreamFromBase64String(PCWSTR pszBase64, IStream **ppStream)
{
    *ppStream = NULL;

    DWORD dwDecodedImageSize, dwSkipChars, dwActualFormat;

    // Base64-decode the string
    HRESULT hr = CryptStringToBinary(pszBase64, NULL, CRYPT_STRING_BASE64, NULL,
        &dwDecodedImageSize, &dwSkipChars, &dwActualFormat) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        BYTE *pbDecodedImage = (BYTE*)LocalAlloc(LPTR, dwDecodedImageSize);
        hr = pbDecodedImage ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = CryptStringToBinary(pszBase64, lstrlen(pszBase64), CRYPT_STRING_BASE64,
                pbDecodedImage, &dwDecodedImageSize, &dwSkipChars, &dwActualFormat) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                *ppStream = SHCreateMemStream(pbDecodedImage, dwDecodedImageSize);
                hr = *ppStream ? S_OK : E_OUTOFMEMORY;
            }
            LocalFree(pbDecodedImage);
        }
    }

    return hr;
}

// use LocalFree() to free the result
HRESULT GetBase64StringFromStream(IStream *pstm, PWSTR *ppszBase64)
{
    *ppszBase64 = NULL;

    BYTE *pdata;
    UINT cBytes;
    HRESULT hr = IStream_ReadToBuffer(pstm, 256 * 1024, &pdata, &cBytes);
    if (SUCCEEDED(hr))
    {
        DWORD dwStringSize = 0; // requested size here
        hr = CryptBinaryToString(pdata, cBytes, CRYPT_STRING_BASE64, NULL, &dwStringSize) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            *ppszBase64 = (PWSTR)LocalAlloc(LPTR, dwStringSize * sizeof(**ppszBase64));
            hr = *ppszBase64 ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                hr = CryptBinaryToString(pdata, cBytes, CRYPT_STRING_BASE64, *ppszBase64, &dwStringSize) ? S_OK : E_FAIL;
                if (FAILED(hr))
                {
                    LocalFree(*ppszBase64);
                    *ppszBase64 = NULL;
                }
            }
        }
        LocalFree(pdata);
    }
    return hr;
}

HRESULT IStream_ReadToBuffer(IStream *pstm, UINT uMaxSize,
                             BYTE **ppBytes, UINT *pcBytes)
{
    *ppBytes = NULL;
    *pcBytes = 0;

    ULARGE_INTEGER uli;
    HRESULT hr = IStream_Size(pstm, &uli);
    if (SUCCEEDED(hr))
    {
        const ULARGE_INTEGER c_uliMaxSize = { uMaxSize };

        hr = (uli.QuadPart < c_uliMaxSize.QuadPart) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            BYTE *pdata = (BYTE*)LocalAlloc(LPTR, uli.LowPart);
            hr = pdata ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                hr = IStream_Read(pstm, pdata, uli.LowPart);
                if (SUCCEEDED(hr))
                {
                    *ppBytes = pdata;
                    *pcBytes = uli.LowPart;
                }
                else
                {
                    LocalFree(pdata);
                }
            }
        }
    }
    return hr;
}