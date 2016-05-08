// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// CPropertyStoreReader is a helper class that holds onto the PropertyStore of a ShellItem.
// Property Stores consist of <PROPERTYKEY, PROPVARIANT> pairs.
// Given a PROPERTYKEY we get the PROPVARIANT value from the Property Store and convert it to
// the appropriate value (string, int, uint, bool, etc.) before returning it to the caller.

class CPropertyStoreReader
{
public:
    CPropertyStoreReader(IPropertyStore *pps = NULL) : _pps(pps)
    {
        if (_pps)
        {
            pps->AddRef();
        }
    }

    ~CPropertyStoreReader()
    {
        _ClearPropertyStore();
    }

    HRESULT InitFromItem(IShellItem *psi, GETPROPERTYSTOREFLAGS flags, const PROPERTYKEY *rgKeys = NULL, UINT cKeys = 0)
    {
        _ClearPropertyStore();

        IShellItem2 *psi2;
        HRESULT hr = psi->QueryInterface(&psi2);
        if (SUCCEEDED(hr))
        {
            hr = psi2->GetPropertyStoreForKeys(rgKeys, cKeys, flags, IID_PPV_ARGS(&_pps));
            psi2->Release();
        }
        return hr;
    }

    HRESULT GetString(REFPROPERTYKEY key, PWSTR *ppsz)
    {
        *ppsz = NULL;

        PROPVARIANT propvar;
        HRESULT hr = _pps->GetValue(key, &propvar);
        if (SUCCEEDED(hr))
        {
            if (VT_EMPTY != propvar.vt)
            {
                hr = PropVariantToStringAlloc(propvar, ppsz);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            PropVariantClear(&propvar);
        }
        return hr;
    }

    HRESULT GetInt32(REFPROPERTYKEY key, LONG *pl)
    {
        *pl = 0;

        PROPVARIANT propvar;
        HRESULT hr = _pps->GetValue(key, &propvar);
        if (SUCCEEDED(hr))
        {
            if (VT_EMPTY != propvar.vt)
            {
                hr = PropVariantToInt32(propvar, pl);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            PropVariantClear(&propvar);
        }
        return hr;
    }

    HRESULT GetUInt32(REFPROPERTYKEY key, ULONG *pul)
    {
        *pul = 0;

        PROPVARIANT propvar;
        HRESULT hr = _pps->GetValue(key, &propvar);
        if (SUCCEEDED(hr))
        {
            if (VT_EMPTY != propvar.vt)
            {
                hr = PropVariantToUInt32(propvar, pul);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            PropVariantClear(&propvar);
        }
        return hr;
    }

    HRESULT GetUInt64(REFPROPERTYKEY key, ULONGLONG *pull)
    {
        *pull = 0;

        PROPVARIANT propvar;
        HRESULT hr = _pps->GetValue(key, &propvar);
        if (SUCCEEDED(hr))
        {
            if (VT_EMPTY != propvar.vt)
            {
                hr = PropVariantToUInt64(propvar, pull);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            PropVariantClear(&propvar);
        }
        return hr;
    }

    HRESULT GetBool(REFPROPERTYKEY key, BOOL *pf)
    {
        *pf = 0;

        PROPVARIANT propvar;
        HRESULT hr = _pps->GetValue(key, &propvar);
        if (SUCCEEDED(hr))
        {
            if (VT_EMPTY != propvar.vt)
            {
                hr = PropVariantToBoolean(propvar, pf);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
            PropVariantClear(&propvar);
        }
        return hr;
    }

    HRESULT GetBytes(REFPROPERTYKEY key, void *pv, UINT cb)
    {
        PROPVARIANT propvar;
        HRESULT hr = _pps->GetValue(key, &propvar);
        if (SUCCEEDED(hr))
        {
            hr = PropVariantToBuffer(propvar, pv, cb);
            PropVariantClear(&propvar);
        }
        else
        {
            ZeroMemory(pv, cb);
        }
        return hr;
    }

    // add more type accessor methods here as needed

private:
    void _ClearPropertyStore()
    {
        if (_pps)
        {
            _pps->Release();
        }
    }

    IPropertyStore *_pps;
};
