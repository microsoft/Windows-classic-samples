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

HRESULT LoadPropertyStoreFromStream(IStream *pstm, REFIID riid, void **ppv)
{
    *ppv = NULL;

    STATSTG stat;
    HRESULT hr = pstm->Stat(&stat, STATFLAG_NONAME);
    if (SUCCEEDED(hr))
    {
        IPersistStream *pps;
        hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&pps));
        if (SUCCEEDED(hr))
        {
            if (stat.cbSize.QuadPart == 0)
            {
                hr = S_OK; // empty stream => empty property store
            }
            else if (stat.cbSize.QuadPart > (128 * 1024))
            {
                hr = STG_E_MEDIUMFULL; // can't be that large
            }
            else
            {
                hr = pps->Load(pstm);
            }

            if (SUCCEEDED(hr))
            {
                hr = pps->QueryInterface(riid, ppv);
            }
            pps->Release();
        }
    }
    return hr;
}

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

HRESULT SerializePropVariantAsString(REFPROPVARIANT propvar, PWSTR *ppszOut)
{
    SERIALIZEDPROPERTYVALUE *pBlob;
    ULONG cbBlob;

    // serialize PROPVARIANT to binary form
    HRESULT hr = StgSerializePropVariant(&propvar, &pBlob, &cbBlob);
    if (SUCCEEDED(hr))
    {
        // determine the required buffer size
        hr = E_FAIL;
        DWORD cchString;
        if (CryptBinaryToStringW((BYTE *)pBlob, cbBlob, CRYPT_STRING_BASE64, NULL, &cchString))
        {
            // allocate a sufficient buffer
            hr = E_OUTOFMEMORY;
            *ppszOut = (PWSTR)CoTaskMemAlloc(sizeof(WCHAR) * cchString);
            if (*ppszOut)
            {
                // convert the serialized binary blob to a string representation
                hr = E_FAIL;
                if (CryptBinaryToStringW((BYTE *)pBlob, cbBlob, CRYPT_STRING_BASE64, *ppszOut, &cchString))
                {
                    hr = S_OK;
                }
                else
                {
                    CoTaskMemFree(*ppszOut);
                    *ppszOut = NULL;
                }
            }
        }

        CoTaskMemFree(pBlob);
    }

    return hr;
}

HRESULT DeserializePropVariantFromString(PCWSTR pszIn, PROPVARIANT *ppropvar)
{
    HRESULT hr = E_FAIL;
    DWORD dwFormatUsed, dwSkip, cbBlob;

    // compute and validate the required buffer size
    if (CryptStringToBinaryW(pszIn, 0, CRYPT_STRING_BASE64, NULL, &cbBlob, &dwSkip, &dwFormatUsed) &&
        dwSkip == 0 &&
        dwFormatUsed == CRYPT_STRING_BASE64)
    {
        // allocate a buffer to hold the serialized binary blob
        hr = E_OUTOFMEMORY;
        BYTE *pbSerialized = (BYTE *)CoTaskMemAlloc(cbBlob);
        if (pbSerialized)
        {
            // convert the string to a serialized binary blob
            hr = E_FAIL;
            if (CryptStringToBinaryW(pszIn, 0, CRYPT_STRING_BASE64, pbSerialized, &cbBlob, &dwSkip, &dwFormatUsed))
            {
                // deserialized the blob back into a PROPVARIANT value
                hr = StgDeserializePropVariant((SERIALIZEDPROPERTYVALUE *)pbSerialized, cbBlob, ppropvar);
            }

            CoTaskMemFree(pbSerialized);
        }
    }

    return hr;
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