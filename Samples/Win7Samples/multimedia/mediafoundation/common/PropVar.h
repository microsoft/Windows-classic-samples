#pragma once

#include <propsys.h>
#include <propvarutil.h>

///////////////////////////////////////////////////////////////////////////////
//
// PropVariant Class
//
// Wrapper class for PROPVARIANT types. 
//
// Notes: 
//    This class offers accessor functions for a subset of the PROPVARIANT
//    data types, designed to handle all of the types in actual use by 
//    Media Foundation at this time.
// 
///////////////////////////////////////////////////////////////////////////////

class PropVariant : public PROPVARIANT
{
public:
    PropVariant()
    {
        PropVariantInit(this);
    }
    ~PropVariant()
    {
        PropVariantClear(this);
    }

    void Clear()
    {
        PropVariantClear(this);
    }

    HRESULT SetBlob(DWORD cbSize, BYTE *pBuffer)    // VT_BLOB
    {
        Clear();
        BYTE *pb = (BYTE*)CoTaskMemAlloc(cbSize);
        if (pb == NULL)
        {
            return E_OUTOFMEMORY;
        }
        this->blob.cbSize = cbSize;
        CopyMemory(this->blob.pBlobData, pBuffer, cbSize);
        return S_OK;
    }
    HRESULT SetBOOL(BOOL val)   // VT_BOOL
    { 
        Clear();
        return InitPropVariantFromBoolean(val, this); 
    }             
    HRESULT SetGUID(const GUID guid)    // VT_CLSID
    {
        Clear();
        return InitPropVariantFromCLSID(guid, this);
    }
    HRESULT SetInt32(LONG val)  // VT_I4
    {    
        Clear();
        return InitPropVariantFromInt32(val, this); 
    }              
    HRESULT SetString(const WCHAR *str) // VT_LPWSTR    
    { 
        return InitPropVariantFromString(str, this); 
    }    
    HRESULT SetStringVector(const WCHAR **ppstr, ULONG cElems)  // VT_LPWSTR | VT_VECTOR
    {
        Clear();
        return InitPropVariantFromStringVector(ppstr, cElems, this);
    }
    HRESULT SetUInt32(ULONG val)    // VT_UI4
    {
        Clear();
        return InitPropVariantFromUInt32(val, this); 
    }           
    HRESULT SetUInt32Vector(const ULONG *pVals, ULONG cElems)   // VT_UI4 | VT_VECTOR
    {
        Clear();
        return InitPropVariantFromUInt32Vector(pVals, cElems, this);
    }
    HRESULT SetUnknown(IUnknown *pUnk)  // VT_UNKNOWN
    {
        if (pUnk == NULL) { return E_POINTER; }
        Clear();
        this->punkVal = pUnk;
        this->punkVal->AddRef();
        this->vt = VT_UNKNOWN;
        return S_OK;
    }
private:
    PropVariant& operator=(const PropVariant& var);
    PropVariant(const PropVariant &var);
};

