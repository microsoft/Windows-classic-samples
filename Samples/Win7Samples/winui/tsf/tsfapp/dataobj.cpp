/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          DataObj.cpp
   
   Description:   CTSFDataObject implementation.

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include "DataObj.h"

/**************************************************************************
   global variables
**************************************************************************/

/**************************************************************************

   CTSFDataObject::CTSFDataObject()

**************************************************************************/

CTSFDataObject::CTSFDataObject()
{
    m_ObjRefCount = 1;

    m_FormatEtc.cfFormat = CF_UNICODETEXT;
    m_FormatEtc.dwAspect = DVASPECT_CONTENT;
    m_FormatEtc.ptd = NULL;
    m_FormatEtc.tymed = TYMED_HGLOBAL;
    m_FormatEtc.lindex = -1;

    m_pwszText = NULL;
}

/**************************************************************************

   CTSFDataObject::~CTSFDataObject()

**************************************************************************/

CTSFDataObject::~CTSFDataObject()
{
    _SetText(NULL);
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

   CTSFDataObject::QueryInterface

**************************************************************************/

STDMETHODIMP CTSFDataObject::QueryInterface(   REFIID riid, 
                                            LPVOID *ppReturn)
{
    *ppReturn = NULL;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        *ppReturn = (LPUNKNOWN)(LPDATAOBJECT)this;
    }
    else if(IsEqualIID(riid, IID_IDataObject))
    {
        *ppReturn = (LPDATAOBJECT)this;
    }   

    if(*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}                                             

/**************************************************************************

   CTSFDataObject::AddRef

**************************************************************************/

STDMETHODIMP_(DWORD) CTSFDataObject::AddRef()
{
    return ++m_ObjRefCount;
}

/**************************************************************************

   CTSFDataObject::Release

**************************************************************************/

STDMETHODIMP_(DWORD) CTSFDataObject::Release()
{
    if(--m_ObjRefCount == 0)
    {
        delete this;
    }
   
    return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IDataObject Implementation
//

/**************************************************************************

   CTSFDataObject::GetData()

**************************************************************************/

STDMETHODIMP CTSFDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    if(pFE->cfFormat == m_FormatEtc.cfFormat)
    {
        pStgMedium->pUnkForRelease = NULL;
        pStgMedium->hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (lstrlenW(m_pwszText) + 1) * sizeof(WCHAR));

        if(pStgMedium->hGlobal)
        {
            pStgMedium->tymed = TYMED_HGLOBAL;
            return GetDataHere(pFE, pStgMedium);
        }

        return E_OUTOFMEMORY;
    }

    return E_INVALIDARG;
}

/**************************************************************************

   CTSFDataObject::GetDataHere()

**************************************************************************/

STDMETHODIMP CTSFDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    if(pFE->cfFormat == m_FormatEtc.cfFormat)
    {
        if(pFE->tymed & TYMED_HGLOBAL)
        {
            //copy the text into the buffer
            LPWSTR  pwsz = (LPWSTR)GlobalLock(pStgMedium->hGlobal);

            if(pwsz)
            {
                SIZE_T  sizeDestBytes = GlobalSize(pStgMedium->hGlobal);
                SIZE_T  sizeSrcBytes = (lstrlenW(m_pwszText) + 1) * sizeof(WCHAR);

                if(sizeDestBytes < sizeSrcBytes)
                {
                    return STG_E_MEDIUMFULL;
                }
                
                CopyMemory(pwsz, m_pwszText, sizeSrcBytes);

                GlobalUnlock(pStgMedium->hGlobal);

                return S_OK;
            }
        }
        else
        {
            return DV_E_TYMED;
        }
    }
    else
    {
        return DV_E_CLIPFORMAT;
    }

    return E_FAIL;
}

/**************************************************************************

   CTSFDataObject::QueryGetData()

**************************************************************************/

STDMETHODIMP CTSFDataObject::QueryGetData(LPFORMATETC pFE)
{
BOOL fReturn = FALSE;

    /*
    Check the aspects we support. Implementations of this object will only
    support DVASPECT_CONTENT.
    */
    if(!(DVASPECT_CONTENT & pFE->dwAspect))
    {
        return DV_E_DVASPECT;
    }

    if(pFE->cfFormat == m_FormatEtc.cfFormat)
    {
        fReturn |= m_FormatEtc.tymed & pFE->tymed;
    }

    return (fReturn ? S_OK : DV_E_TYMED);
}

/**************************************************************************

   CTSFDataObject::GetCanonicalFormatEtc()

**************************************************************************/

STDMETHODIMP CTSFDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut)
{
    if(NULL == pFEOut)
    {
        return E_INVALIDARG;
    }

    pFEOut->ptd = NULL;

    return DATA_S_SAMEFORMATETC;
}

/**************************************************************************

   CTSFDataObject::EnumFormatEtc()

**************************************************************************/

STDMETHODIMP CTSFDataObject::EnumFormatEtc(  DWORD dwDirection, 
                                          IEnumFORMATETC** ppEFE)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CTSFDataObject::SetData()

**************************************************************************/

STDMETHODIMP CTSFDataObject::SetData(  LPFORMATETC pFE, 
                                    LPSTGMEDIUM pStgMedium, 
                                    BOOL fRelease)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CTSFDataObject::DAdvise()

**************************************************************************/

STDMETHODIMP CTSFDataObject::DAdvise(  LPFORMATETC pFE, 
                                    DWORD advf, 
                                    IAdviseSink *ppAdviseSink, 
                                    LPDWORD pdwConnection)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CTSFDataObject::DUnadvise()

**************************************************************************/

STDMETHODIMP CTSFDataObject::DUnadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CTSFDataObject::EnumDAdvise()

**************************************************************************/

STDMETHODIMP CTSFDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////
//
// private and utility methods
//

/**************************************************************************

   CTSFDataObject::_SetText()

**************************************************************************/

HRESULT CTSFDataObject::_SetText(LPWSTR pwszText)
{
    if(m_pwszText)
    {
        GlobalFree(m_pwszText);
        m_pwszText = NULL;
    }

    if(pwszText && *pwszText)
    {
        size_t cch = lstrlenW(pwszText) + 1; 
        m_pwszText = (LPWSTR)GlobalAlloc(GMEM_FIXED, cch * sizeof(WCHAR));
        if(NULL == m_pwszText)
        {
            return E_OUTOFMEMORY;
        }

        wcscpy_s(m_pwszText, cch, pwszText);
    }
    
    return S_OK;
}

