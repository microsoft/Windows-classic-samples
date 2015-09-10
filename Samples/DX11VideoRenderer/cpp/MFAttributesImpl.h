#pragma once

template <class TBase=IMFAttributes>
class CMFAttributesImpl : public TBase
{
protected:

    // This version of the constructor does not initialize the
    // attribute store. The derived class must call Initialize() in
    // its own constructor.
    CMFAttributesImpl(void) :
        _pAttributes(NULL)
    {
    }

    // This version of the constructor initializes the attribute
    // store, but the derived class must pass an HRESULT parameter
    // to the constructor.

    CMFAttributesImpl(HRESULT& hr, UINT32 cInitialSize = 0) :
        _pAttributes(NULL)
    {
        hr = Initialize(cInitialSize);
    }

    // The next version of the constructor uses a caller-provided
    // implementation of IMFAttributes.

    // (Sometimes you want to delegate IMFAttributes calls to some
    // other object that implements IMFAttributes, rather than using
    // MFCreateAttributes.)

    CMFAttributesImpl(HRESULT& hr, IUnknown* pUnk) :
        _pAttributes(NULL)
    {
        hr = Initialize(pUnk);
    }

    virtual ~CMFAttributesImpl(void)
    {
        if (_pAttributes)
        {
            _pAttributes->Release();
        }
    }

    // Initializes the object by creating the standard Media Foundation attribute store.
    HRESULT Initialize(UINT32 cInitialSize = 0)
    {
        if (_pAttributes == NULL)
        {
            return MFCreateAttributes(&_pAttributes, cInitialSize);
        }
        else
        {
            return S_OK;
        }
    }

    // Initializes this object from a caller-provided attribute store.
    // pUnk: Pointer to an object that exposes IMFAttributes.
    HRESULT Initialize(IUnknown* pUnk)
    {
        if (_pAttributes)
        {
            _pAttributes->Release();
            _pAttributes = NULL;
        }

        return pUnk->QueryInterface(IID_PPV_ARGS(&_pAttributes));
    }

public:

    // IMFAttributes methods

    STDMETHODIMP GetItem(__RPC__in REFGUID guidKey, __RPC__inout_opt PROPVARIANT* pValue)
    {
        assert(_pAttributes);
        return _pAttributes->GetItem(guidKey, pValue);
    }

    STDMETHODIMP GetItemType(__RPC__in REFGUID guidKey, __RPC__out MF_ATTRIBUTE_TYPE* pType)
    {
        assert(_pAttributes);
        return _pAttributes->GetItemType(guidKey, pType);
    }

    STDMETHODIMP CompareItem(__RPC__in REFGUID guidKey, __RPC__in REFPROPVARIANT Value, __RPC__out BOOL* pbResult)
    {
        assert(_pAttributes);
        return _pAttributes->CompareItem(guidKey, Value, pbResult);
    }

    STDMETHODIMP Compare(__RPC__in_opt IMFAttributes* pTheirs, MF_ATTRIBUTES_MATCH_TYPE MatchType, __RPC__out BOOL* pbResult)
    {
        assert(_pAttributes);
        return _pAttributes->Compare(pTheirs, MatchType, pbResult);
    }

    STDMETHODIMP GetUINT32(__RPC__in REFGUID guidKey, __RPC__out UINT32* punValue)
    {
        assert(_pAttributes);
        return _pAttributes->GetUINT32(guidKey, punValue);
    }

    STDMETHODIMP GetUINT64(__RPC__in REFGUID guidKey, __RPC__out UINT64* punValue)
    {
        assert(_pAttributes);
        return _pAttributes->GetUINT64(guidKey, punValue);
    }

    STDMETHODIMP GetDouble(__RPC__in REFGUID guidKey, __RPC__out double* pfValue)
    {
        assert(_pAttributes);
        return _pAttributes->GetDouble(guidKey, pfValue);
    }

    STDMETHODIMP GetGUID(__RPC__in REFGUID guidKey, __RPC__out GUID* pguidValue)
    {
        assert(_pAttributes);
        return _pAttributes->GetGUID(guidKey, pguidValue);
    }

    STDMETHODIMP GetStringLength(__RPC__in REFGUID guidKey, __RPC__out UINT32* pcchLength)
    {
        assert(_pAttributes);
        return _pAttributes->GetStringLength(guidKey, pcchLength);
    }

    STDMETHODIMP GetString(__RPC__in REFGUID guidKey, __RPC__out_ecount_full(cchBufSize) LPWSTR pwszValue, UINT32 cchBufSize, __RPC__inout_opt UINT32* pcchLength)
    {
        assert(_pAttributes);
        return _pAttributes->GetString(guidKey, pwszValue, cchBufSize, pcchLength);
    }

    STDMETHODIMP GetAllocatedString(__RPC__in REFGUID guidKey, __RPC__deref_out_ecount_full_opt(( *pcchLength + 1 ) ) LPWSTR* ppwszValue, __RPC__out UINT32* pcchLength)
    {
        assert(_pAttributes);
        return _pAttributes->GetAllocatedString(guidKey, ppwszValue, pcchLength);
    }

    STDMETHODIMP GetBlobSize(__RPC__in REFGUID guidKey, __RPC__out UINT32* pcbBlobSize)
    {
        assert(_pAttributes);
        return _pAttributes->GetBlobSize(guidKey, pcbBlobSize);
    }

    STDMETHODIMP GetBlob(__RPC__in REFGUID guidKey, __RPC__out_ecount_full(cbBufSize) UINT8* pBuf, UINT32 cbBufSize, __RPC__inout_opt UINT32* pcbBlobSize)
    {
        assert(_pAttributes);
        return _pAttributes->GetBlob(guidKey, pBuf, cbBufSize, pcbBlobSize);
    }

    STDMETHODIMP GetAllocatedBlob(__RPC__in REFGUID guidKey, __RPC__deref_out_ecount_full_opt(*pcbSize) UINT8** ppBuf, __RPC__out UINT32* pcbSize)
    {
        assert(_pAttributes);
        return _pAttributes->GetAllocatedBlob(guidKey, ppBuf, pcbSize);
    }

    STDMETHODIMP GetUnknown(__RPC__in REFGUID guidKey, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppv)
    {
        assert(_pAttributes);
        return _pAttributes->GetUnknown(guidKey, riid, ppv);
    }

    STDMETHODIMP SetItem(__RPC__in REFGUID guidKey, __RPC__in REFPROPVARIANT Value)
    {
        assert(_pAttributes);
        return _pAttributes->SetItem(guidKey, Value);
    }

    STDMETHODIMP DeleteItem(__RPC__in REFGUID guidKey)
    {
        assert(_pAttributes);
        return _pAttributes->DeleteItem(guidKey);
    }

    STDMETHODIMP DeleteAllItems(void)
    {
        assert(_pAttributes);
        return _pAttributes->DeleteAllItems();
    }

    STDMETHODIMP SetUINT32(__RPC__in REFGUID guidKey, UINT32 unValue)
    {
        assert(_pAttributes);
        return _pAttributes->SetUINT32(guidKey, unValue);
    }

    STDMETHODIMP SetUINT64(__RPC__in REFGUID guidKey, UINT64 unValue)
    {
        assert(_pAttributes);
        return _pAttributes->SetUINT64(guidKey, unValue);
    }

    STDMETHODIMP SetDouble(__RPC__in REFGUID guidKey, double fValue)
    {
        assert(_pAttributes);
        return _pAttributes->SetDouble(guidKey, fValue);
    }

    STDMETHODIMP SetGUID(__RPC__in REFGUID guidKey, __RPC__in REFGUID guidValue)
    {
        assert(_pAttributes);
        return _pAttributes->SetGUID(guidKey, guidValue);
    }

    STDMETHODIMP SetString(__RPC__in REFGUID guidKey, __RPC__in_string LPCWSTR wszValue)
    {
        assert(_pAttributes);
        return _pAttributes->SetString(guidKey, wszValue);
    }

    STDMETHODIMP SetBlob(__RPC__in REFGUID guidKey, __RPC__in_ecount_full(cbBufSize) const UINT8* pBuf, UINT32 cbBufSize)
    {
        assert(_pAttributes);
        return _pAttributes->SetBlob(guidKey, pBuf, cbBufSize);
    }

    STDMETHODIMP SetUnknown(__RPC__in REFGUID guidKey, __RPC__in_opt IUnknown* pUnknown)
    {
        assert(_pAttributes);
        return _pAttributes->SetUnknown(guidKey, pUnknown);
    }

    STDMETHODIMP LockStore(void)
    {
        assert(_pAttributes);
        return _pAttributes->LockStore();
    }

    STDMETHODIMP UnlockStore(void)
    {
        assert(_pAttributes);
        return _pAttributes->UnlockStore();
    }

    STDMETHODIMP GetCount(__RPC__out UINT32* pcItems)
    {
        assert(_pAttributes);
        return _pAttributes->GetCount(pcItems);
    }

    STDMETHODIMP GetItemByIndex(UINT32 unIndex, __RPC__out GUID* pguidKey, __RPC__inout_opt PROPVARIANT* pValue)
    {
        assert(_pAttributes);
        return _pAttributes->GetItemByIndex(unIndex, pguidKey, pValue);
    }

    STDMETHODIMP CopyAllItems(__RPC__in_opt IMFAttributes* pDest)
    {
        assert(_pAttributes);
        return _pAttributes->CopyAllItems(pDest);
    }

    // Helper functions

    HRESULT SerializeToStream(DWORD dwOptions, IStream* pStm)
        // dwOptions: Flags from MF_ATTRIBUTE_SERIALIZE_OPTIONS
    {
        assert(_pAttributes);
        return MFSerializeAttributesToStream(_pAttributes, dwOptions, pStm);
    }

    HRESULT DeserializeFromStream(DWORD dwOptions, IStream* pStm)
    {
        assert(_pAttributes);
        return MFDeserializeAttributesFromStream(_pAttributes, dwOptions, pStm);
    }

    // SerializeToBlob: Stores the attributes in a byte array.
    //
    // ppBuf: Receives a pointer to the byte array.
    // pcbSize: Receives the size of the byte array.
    //
    // The caller must free the array using CoTaskMemFree.
    HRESULT SerializeToBlob(UINT8** ppBuffer, UINT* pcbSize)
    {
        assert(_pAttributes);

        if (ppBuffer == NULL)
        {
            return E_POINTER;
        }
        if (pcbSize == NULL)
        {
            return E_POINTER;
        }

        HRESULT hr = S_OK;
        UINT32 cbSize = 0;
        BYTE* pBuffer = NULL;

        CHECK_HR(hr = MFGetAttributesAsBlobSize(_pAttributes, &cbSize));

        pBuffer = (BYTE*)CoTaskMemAlloc(cbSize);
        if (pBuffer == NULL)
        {
            CHECK_HR(hr = E_OUTOFMEMORY);
        }

        CHECK_HR(hr = MFGetAttributesAsBlob(_pAttributes, pBuffer, cbSize));

        *ppBuffer = pBuffer;
        *pcbSize = cbSize;

        if (FAILED(hr))
        {
            *ppBuffer = NULL;
            *pcbSize = 0;
            CoTaskMemFree(pBuffer);
        }
        return hr;
    }

    HRESULT DeserializeFromBlob(const UINT8* pBuffer, UINT cbSize)
    {
        assert(_pAttributes);
        return MFInitAttributesFromBlob(_pAttributes, pBuffer, cbSize);
    }

    HRESULT GetRatio(REFGUID guidKey, UINT32* pnNumerator, UINT32* punDenominator)
    {
        assert(_pAttributes);
        return MFGetAttributeRatio(_pAttributes, guidKey, pnNumerator, punDenominator);
    }

    HRESULT SetRatio(REFGUID guidKey, UINT32 unNumerator, UINT32 unDenominator)
    {
        assert(_pAttributes);
        return MFSetAttributeRatio(_pAttributes, guidKey, unNumerator, unDenominator);
    }

    // Gets an attribute whose value represents the size of something (eg a video frame).
    HRESULT GetSize(REFGUID guidKey, UINT32* punWidth, UINT32* punHeight)
    {
        assert(_pAttributes);
        return MFGetAttributeSize(_pAttributes, guidKey, punWidth, punHeight);
    }

    // Sets an attribute whose value represents the size of something (eg a video frame).
    HRESULT SetSize(REFGUID guidKey, UINT32 unWidth, UINT32 unHeight)
    {
        assert(_pAttributes);
        return MFSetAttributeSize (_pAttributes, guidKey, unWidth, unHeight);
    }

protected:

    IMFAttributes* _pAttributes;
};