//***************************************************************************
//    THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
//    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//    PARTICULAR PURPOSE.
//
//    Copyright 2003 Microsoft Corporation. All Rights Reserved.
//***************************************************************************

//***************************************************************************
//
//    File:          DataObj.cpp
//
//    Description:   CPropSheetHost implementation.
//
//***************************************************************************

//***************************************************************************
//    #include statements
//***************************************************************************

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

   CPropSheetHost::QueryInterface

**************************************************************************/

STDMETHODIMP CPropSheetHost::QueryInterface(REFIID riid, 
                                            LPVOID *ppReturn)
{
    if(!ppReturn)
    {
        return E_INVALIDARG;
    }
    
    *ppReturn = NULL;

    if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDataObject))
    {
        *ppReturn = (IDataObject*)this;
    }

    if(*ppReturn)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}                                             

/**************************************************************************

   CPropSheetHost::AddRef

**************************************************************************/

STDMETHODIMP_(DWORD) CPropSheetHost::AddRef()
{
    return ++m_ObjRefCount;
}

/**************************************************************************

   CPropSheetHost::Release

**************************************************************************/

STDMETHODIMP_(DWORD) CPropSheetHost::Release()
{
    if(--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }
   
    return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IDataObject Implementation
//

/**************************************************************************

   CPropSheetHost::GetData()

    Retrieves the data and places it in memory that the implementation 
    allocates.

**************************************************************************/

STDMETHODIMP CPropSheetHost::GetData(FORMATETC *pFormatEtc, 
                                    STGMEDIUM *pStgMedium)
{
    if(!pFormatEtc || ! pStgMedium)
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = DV_E_FORMATETC;

    if(m_cfDSDispSpecOptions == pFormatEtc->cfFormat)
    {
        hr = _GetDSDispSpecOption(pFormatEtc, pStgMedium);
    }
    else if(m_cfDSObjectNames == pFormatEtc->cfFormat)
    {
        hr = _GetDSObjectNames(pFormatEtc, pStgMedium);
    }
    else if(m_cfDSPropSheetConfig == pFormatEtc->cfFormat)
    {
        hr = _GetDSPropSheetConfig(pFormatEtc, pStgMedium);
    }

    return hr;
}

/**************************************************************************

   CPropSheetHost::GetDataHere()

    Retrieves the data and places it in memory that the caller 
    allocates.

**************************************************************************/

STDMETHODIMP CPropSheetHost::GetDataHere(FORMATETC *pFormatEtc, 
                                        STGMEDIUM *pStgMedium)
{
    return E_NOTIMPL;
}

/**************************************************************************

    CPropSheetHost::QueryGetData()

    Determines if the data object can supply the data in the specified 
    format. Returns S_OK if it can or one of the DV_E_ values if not. It is 
    not necessary to call this before GetData() or GetDataHere().

**************************************************************************/

STDMETHODIMP CPropSheetHost::QueryGetData(FORMATETC *pFormatEtc)
{
    if(this->m_cfDSDispSpecOptions == pFormatEtc->cfFormat)
    {
        return S_OK;
    }
    else if(this->m_cfDSObjectNames == pFormatEtc->cfFormat)
    {
        return S_OK;
    }
    else if(this->m_cfDSPropSheetConfig == pFormatEtc->cfFormat)
    {
        return S_OK;
    }
    
    return DV_E_FORMATETC;
}

/**************************************************************************

   CPropSheetHost::GetCanonicalFormatEtc()

**************************************************************************/

STDMETHODIMP CPropSheetHost::GetCanonicalFormatEtc(LPFORMATETC pFEIn, 
                                                  LPFORMATETC pFEOut)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CPropSheetHost::EnumFormatEtc()

**************************************************************************/

STDMETHODIMP CPropSheetHost::EnumFormatEtc(DWORD dwDirection, 
                                          IEnumFORMATETC** ppEFE)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CPropSheetHost::SetData()

**************************************************************************/

STDMETHODIMP CPropSheetHost::SetData(LPFORMATETC pFormatEtc, 
                                    LPSTGMEDIUM pStgMedium, 
                                    BOOL fRelease)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CPropSheetHost::DAdvise()

**************************************************************************/

STDMETHODIMP CPropSheetHost::DAdvise(  LPFORMATETC pFE, 
                                    DWORD advf, 
                                    IAdviseSink *ppAdviseSink, 
                                    LPDWORD pdwConnection)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CPropSheetHost::DUnadvise()

**************************************************************************/

STDMETHODIMP CPropSheetHost::DUnadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

/**************************************************************************

   CPropSheetHost::EnumDAdvise()

**************************************************************************/

STDMETHODIMP CPropSheetHost::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
    return E_NOTIMPL;
}

/***************************************************************************

    CPropSheetHost::_GetDSDispSpecOption()

    Get the CFSTR_DSDISPLAYSPECOPTIONS data.

***************************************************************************/

HRESULT CPropSheetHost::_GetDSDispSpecOption(FORMATETC *pFormatEtc, 
                                             STGMEDIUM *pStgMedium)
{
    if((m_cfDSDispSpecOptions != pFormatEtc->cfFormat) ||
        !(pFormatEtc->tymed & TYMED_HGLOBAL))
    {
        return DV_E_FORMATETC;
    }
    
    HRESULT hr = E_OUTOFMEMORY;
    LPWSTR pwszPrefix = m_pwszPrefix;
    DWORD dwPrefixOffset;

    // Size of the DSDISPLAYSPECOPTIONS structure.
    DWORD dwBytes = sizeof(DSDISPLAYSPECOPTIONS);
    
    // Store the offset to the prefix.
    dwPrefixOffset = dwBytes;
    
    // Length of the prefix Unicode string, including the null terminator.
	DWORD strBufferLenInChar = lstrlenW(pwszPrefix) + 1;
    dwBytes += strBufferLenInChar * sizeof(WCHAR);

    pStgMedium->pUnkForRelease = NULL;
    pStgMedium->tymed = TYMED_HGLOBAL;
    pStgMedium->hGlobal = GlobalAlloc(GPTR, dwBytes);
    if(pStgMedium->hGlobal)
    {
        DSDISPLAYSPECOPTIONS *pDispSpecOptions = (DSDISPLAYSPECOPTIONS*)GlobalLock(pStgMedium->hGlobal);
        if(pDispSpecOptions)
        {
            LPWSTR pwszTemp;

            pDispSpecOptions->dwSize = sizeof(DSDISPLAYSPECOPTIONS);
            pDispSpecOptions->dwFlags = 0;
            pDispSpecOptions->offsetAttribPrefix = dwPrefixOffset;
            pDispSpecOptions->offsetUserName = 0;
            pDispSpecOptions->offsetPassword = 0;
            pDispSpecOptions->offsetServer = 0;
            pDispSpecOptions->offsetServerConfigPath = 0;
            
            // Copy the prefix string.
            pwszTemp = (LPWSTR)((LPBYTE)pDispSpecOptions + dwPrefixOffset);
            wcscpy_s(pwszTemp, strBufferLenInChar, pwszPrefix);

            GlobalUnlock(pStgMedium->hGlobal);

            hr = S_OK;
        }
    }

    return hr;
}

/***************************************************************************

    CPropSheetHost::_GetDSObjectNames()

    Get the CFSTR_DSOBJECTNAMES data.

***************************************************************************/

HRESULT CPropSheetHost::_GetDSObjectNames(FORMATETC *pFormatEtc, 
                                          STGMEDIUM *pStgMedium)
{
    if((m_cfDSObjectNames != pFormatEtc->cfFormat) ||
        !(pFormatEtc->tymed & TYMED_HGLOBAL))
    {
        return DV_E_FORMATETC;
    }
    
    HRESULT hr;
    CComBSTR sbstrADsPath;

    hr = m_spADObject->get_ADsPath(&sbstrADsPath);
    if(FAILED(hr))
    {
        return hr;
    }

    CComBSTR sbstrClass;
    hr = m_spADObject->get_Class(&sbstrClass);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = E_OUTOFMEMORY;

    // Size of the DSOBJECTNAMES structure.
    size_t dwBytes = sizeof(DSOBJECTNAMES);
    
    // Store the offset to the name.
    size_t dwNameOffset = dwBytes;
    
    // Length of the ADsPath Unicode string, including the null terminator.
	size_t pathBuffLenInChar = sbstrADsPath.Length() + 1;
    dwBytes += pathBuffLenInChar * sizeof(WCHAR);
    
    // Store the offset to the class.
    size_t dwClassOffset = dwBytes;

    // Length of the class Unicode string, including the null terminator.
	size_t classBuffLenInChar = sbstrClass.Length() + 1;
    dwBytes += classBuffLenInChar * sizeof(WCHAR);
    
    pStgMedium->pUnkForRelease = NULL;
    pStgMedium->tymed = TYMED_HGLOBAL;
    pStgMedium->hGlobal = GlobalAlloc(GPTR, dwBytes);
    if(pStgMedium->hGlobal)
    {
        DSOBJECTNAMES *pObjectNames = (DSOBJECTNAMES*)GlobalLock(pStgMedium->hGlobal);
        if(pObjectNames)
        {
            LPWSTR pwszTemp;

            pObjectNames->clsidNamespace = GUID_NULL;
            pObjectNames->cItems = 1;
            pObjectNames->aObjects[0].dwFlags = 0;
            pObjectNames->aObjects[0].dwProviderFlags = DSPROVIDER_ADVANCED;
            pObjectNames->aObjects[0].offsetName = (DWORD)dwNameOffset;
            pObjectNames->aObjects[0].offsetClass = (DWORD)dwClassOffset;

            // Copy the ADsPath string.
            pwszTemp = (LPWSTR)((LPBYTE)pObjectNames + dwNameOffset);
            wcscpy_s(pwszTemp, pathBuffLenInChar, sbstrADsPath);

            // Copy the class string.
            pwszTemp = (LPWSTR)((LPBYTE)pObjectNames + dwClassOffset);
            wcscpy_s(pwszTemp, classBuffLenInChar, sbstrClass);

            GlobalUnlock(pStgMedium->hGlobal);

            hr = S_OK;
        }
    }

    return hr;
}

/***************************************************************************

    CPropSheetHost::_GetDSPropSheetConfig()

    Get the CFSTR_DS_PROPSHEETCONFIG data.

***************************************************************************/

HRESULT CPropSheetHost::_GetDSPropSheetConfig(FORMATETC *pFormatEtc, 
                                              STGMEDIUM *pStgMedium)
{
    if((m_cfDSPropSheetConfig != pFormatEtc->cfFormat) ||
        !(pFormatEtc->tymed & TYMED_HGLOBAL))
    {
        return DV_E_FORMATETC;
    }
    
    HRESULT hr = E_OUTOFMEMORY;

    pStgMedium->pUnkForRelease = NULL;
    pStgMedium->tymed = TYMED_HGLOBAL;
    pStgMedium->hGlobal = GlobalAlloc(GPTR, sizeof(PROPSHEETCFG));
    if(pStgMedium->hGlobal)
    {
        PROPSHEETCFG *pPropSheetCfg = (PROPSHEETCFG*)GlobalLock(pStgMedium->hGlobal);
        if(pPropSheetCfg)
        {
            // hwndParentSheet recevies the handle of the window to receive WM_ADSPROP_NOTIFY_CHANGE messages.
            pPropSheetCfg->hwndParentSheet = m_hwndHidden;

            // hwndParentSheet receives the handle of the window that will receive WM_DSA_SHEET_CREATE_NOTIFY and WM_DSA_SHEET_CLOSE_NOTIFY messages.
            pPropSheetCfg->hwndHidden = m_hwndHidden;

            // lNotifyHandle is not used.
            pPropSheetCfg->lNotifyHandle = 0;

            /*
            wParamSheetClose is an identifier that is passed as the wparam in 
            the WM_DSA_SHEET_CLOSE_NOTIFY message. If this member is zero, the 
            WM_DSA_SHEET_CLOSE_NOTIFY is not posted. 
            */
            pPropSheetCfg->wParamSheetClose = PROP_SHEET_HOST_ID;
            
            GlobalUnlock(pStgMedium->hGlobal);

            hr = S_OK;
        }
    }

    return hr;
}

