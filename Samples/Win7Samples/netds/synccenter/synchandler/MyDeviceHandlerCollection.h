//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      MyDeviceHandlerCollection.h
//
//  Abstract:
//      Include file for Sync Center handler collection classes for a device.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////
//
// Structure encapsulating info about a device.
//
//////////////////////////////////////////////////////////////////////////////
struct SYNCDEVICEINFO
{
    DWORD   nPartnerID;
    WCHAR   szHandlerID[MAX_SYNCMGR_ID];
    WCHAR   szName[MAX_SYNCMGR_NAME];

}; //*** struct SYNCDEVICEINFO

//----------------------------------------------------------------------------
// Forward Class Declarations
//----------------------------------------------------------------------------

class CMyDeviceHandlerCollection;
class CEnumDeviceHandlerIDs;

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CMyDeviceHandlerCollection_CreateInstance(__in_opt IUnknown *punkOuter, __deref_out IUnknown **ppunk);
HRESULT CEnumDeviceHandlerIDs_CreateInstance(__inout CMyDeviceHandlerCollection *pHandlerCollection, __in REFIID riid, __deref_out void **ppv);

//////////////////////////////////////////////////////////////////////////////
//
// Class which allows devices to be enumerated and exposed at the top level
// in Sync Center.
//
//////////////////////////////////////////////////////////////////////////////
class CMyDeviceHandlerCollection : public ISyncMgrHandlerCollection
{
public:
    CMyDeviceHandlerCollection();

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef()     { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // ISyncMgrHandlerCollection
    IFACEMETHODIMP GetHandlerEnumerator(__deref_out IEnumString **ppenum);
    IFACEMETHODIMP BindToHandler(__in LPCWSTR pszHandlerID, __in REFIID riid, __deref_out void **ppv);

private:
    ~CMyDeviceHandlerCollection();

    HRESULT _LoadHandlerIDs();

    /////////////////////
    // Member Variables
    /////////////////////
    LONG             _cRef;
    DWORD            _cDevices;
    SYNCDEVICEINFO  *_pDevices;

    // Friend Declarations
    friend class CEnumDeviceHandlerIDs;

}; //*** class CMyDeviceHandlerCollection

//////////////////////////////////////////////////////////////////////////////
//
// Enumeration of all the devices to expose in Sync Center.
//
//////////////////////////////////////////////////////////////////////////////
class CEnumDeviceHandlerIDs : public IEnumString
{
public:
    CEnumDeviceHandlerIDs(__inout CMyDeviceHandlerCollection *pHandlerCollection) :
        _cRef(1),
        _iCur(0),
        _pHandlerCollection(pHandlerCollection)
    {
        DllAddRef();
        _pHandlerCollection->AddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef()     { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // IEnumString
    IFACEMETHODIMP Next(__in ULONG celt, __deref_out_ecount(celt) LPWSTR *rgelt, __out_opt ULONG *pceltFetched);
    IFACEMETHODIMP Skip(__in ULONG cSkip)
    {
        UNREFERENCED_PARAMETER(cSkip);
        return E_NOTIMPL;
    }
    IFACEMETHODIMP Reset()
    {
        _iCur = 0;
        return S_OK;
    }
    IFACEMETHODIMP Clone(__deref_out IEnumString **ppenum)
    {
        *ppenum = NULL;
        return E_NOTIMPL;
    }

private:
    ~CEnumDeviceHandlerIDs()
    {
        if (_pHandlerCollection!= NULL)
        {
            _pHandlerCollection->Release();
        }
        DllRelease();
    }

    // Member Variables
    LONG                         _cRef;
    ULONG                        _iCur;
    CMyDeviceHandlerCollection  *_pHandlerCollection;

}; //*** class CEnumDeviceHandlerIDs
