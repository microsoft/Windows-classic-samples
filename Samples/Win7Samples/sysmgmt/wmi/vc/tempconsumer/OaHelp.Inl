// **************************************************************************
// Copyright (c)  Microsoft Corporation
//
// File:  OAHELP.INL
//
// Description:  OLE Automation Helpers for client code
//
// History:
//
// **************************************************************************

#ifndef _OAHELP_INL_
#define _OAHELP_INL_

class CBSTR
{
    BSTR m_pStr;
public:
    CBSTR() { m_pStr = 0; }
    CBSTR(LPWSTR pSrc) { m_pStr = SysAllocString(pSrc); }
   ~CBSTR() { if (m_pStr) SysFreeString(m_pStr); }
    operator BSTR() { return m_pStr; }

    void Unbind() { m_pStr = 0; }
};

class CVARIANT
{
    VARIANT v;
public:
    CVARIANT() { VariantInit(&v); }
   ~CVARIANT() { VariantClear(&v); }
    void Clear()  { VariantClear(&v); }

    operator VARIANT *() { return &v; }
    VARIANT *operator &() { return &v; }

    CVARIANT(LPWSTR pSrc)   { VariantInit(&v); SetStr(pSrc); }
    CVARIANT(LONG lSrc)     { VariantInit(&v); SetLONG(lSrc); }
    CVARIANT(BOOL b)        { VariantInit(&v); SetBool(b); }
    CVARIANT(short i)       { VariantInit(&v); SetShort(i); }
    CVARIANT(double d)      { VariantInit(&v); SetDouble(d); }
    CVARIANT(BYTE b)        { VariantInit(&v); SetByte(b); }
    CVARIANT(IDispatch * pDisp) { VariantInit(&v); pDisp->AddRef(); SetDispatch(pDisp); }

    void   SetStr(LPWSTR pSrc)
    { Clear(); V_VT(&v) = pSrc ? VT_BSTR : VT_NULL; 
      V_BSTR(&v) = pSrc ? SysAllocString(pSrc) : 0; 
    }

    LPWSTR GetStr() { return V_VT(&v) == VT_BSTR ? V_BSTR(&v) : 0; }
    operator LPWSTR() { return V_VT(&v) == VT_BSTR ? V_BSTR(&v) : 0; }

    void SetLONG(LONG lSrc) { Clear(); V_VT(&v) = VT_I4; V_I4(&v) = lSrc; }
    LONG GetLONG() { return V_I4(&v); }
    operator LONG() { return V_I4(&v);  }

    void SetDouble(double dSrc) { Clear(); V_VT(&v) = VT_R8; V_R8(&v) = dSrc; }
    double GetDouble() { return V_R8(&v); }
    operator double() { return V_R8(&v);  }

    void SetByte(BYTE bySrc) { Clear(); V_VT(&v) = VT_UI1; V_UI1(&v) = bySrc; }
    BYTE GetByte() { return V_UI1(&v); }
    operator BYTE() { return V_UI1(&v);  }

    void SetBool(BOOL b) { V_VT(&v) = VT_BOOL; V_BOOL(&v) = b ? VARIANT_TRUE : VARIANT_FALSE; }
    BOOL GetBool() { return V_BOOL(&v) == VARIANT_TRUE; }
    operator BOOL() { return V_BOOL(&v); }

    void SetDispatch(IDispatch* pDisp) { V_VT(&v) = VT_DISPATCH; V_DISPATCH(&v) = pDisp; }
    IDispatch * GetDispatch() { return V_DISPATCH(&v); }

    void SetUnknown(IUnknown* pUnk) { V_VT(&v) = VT_UNKNOWN; V_UNKNOWN(&v) = pUnk; }
    IUnknown * GetUnknown() { return V_UNKNOWN(&v); }

    void SetShort(short i) { V_VT(&v) = VT_I2; V_I2(&v) = i; }
    short GetShort() { return V_I2(&v); }
    operator short() { return V_I2(&v); }

    VARTYPE GetType() { return V_VT(&v); }

    void SetArray(SAFEARRAY *p, VARTYPE vt) { Clear(); V_VT(&v) = vt; V_ARRAY(&v) = p; }
        // This function acquires the SAFEARRAY pointer and it is no longer owned
        // by the caller.

    operator SAFEARRAY *() { return (V_VT(&v) & VT_ARRAY ? V_ARRAY(&v) : 0); }

    void Unbind() { VariantInit(&v); }
};

class CSAFEARRAY
{
    SAFEARRAY *p;
public:
    CSAFEARRAY(VARTYPE v, int nSize) 
    {
        SAFEARRAYBOUND rgsabound[1];
    	rgsabound[0].lLbound = 0;
    	rgsabound[0].cElements = nSize;
        p = SafeArrayCreate(v, 1, rgsabound);
    }
    
    ~CSAFEARRAY() { if (p) SafeArrayDestroy(p); }
    CSAFEARRAY(SAFEARRAY *pSrc) { p = pSrc; }

    HRESULT Put(long nLocation, LPVOID pData) {
        return SafeArrayPutElement(p, &nLocation, pData);                
    }

    HRESULT Get(long nLocation, LPVOID pData) { 
        return SafeArrayGetElement(p, &nLocation, pData);                
    }

    HRESULT Access(void **pData) { return SafeArrayAccessData(p, pData); }
    HRESULT Unaccess() { return SafeArrayUnaccessData(p); }

    operator SAFEARRAY *() { return p; }
    long GetNumElements() { long upper; SafeArrayGetUBound(p, 1, &upper); return upper + 1; }

    void Unbind() { p = 0; }
};


#endif
