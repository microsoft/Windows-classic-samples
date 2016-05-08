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
//      MyDeviceHandlerCollection.cpp
//
//  Abstract:
//      Implementation of the Sync Center handler collection for a device.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "MyDeviceHandlerCollection.h"
#include "MyDeviceSyncHandler.h"
#include "Guids.h"          // Declare the GUIDs used by this component.

//////////////////////////////////////////////////////////////////////////////
// class CMyDeviceHandlerCollection
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the handler collection class.
//
//  Parameters:
//      punkOuter       - Outer IUnknown for aggregation.
//      ppunk           - IUnknown interface pointer returned to caller.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceHandlerCollection_CreateInstance(
    __in_opt        IUnknown     *punkOuter,
    __deref_out     IUnknown    **ppunk
    )
{
    *ppunk = NULL;

    UNREFERENCED_PARAMETER(punkOuter);

    CMyDeviceHandlerCollection *phc = new CMyDeviceHandlerCollection();
    HRESULT hr = (phc != NULL) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = phc->QueryInterface(IID_PPV_ARGS(ppunk));
        phc->Release();
    }

    return hr;

} //*** CMyDeviceHandlerCollection_CreateInstance

//----------------------------------------------------------------------------
//
//  Description:
//      Default constructor.
//
//----------------------------------------------------------------------------
CMyDeviceHandlerCollection::CMyDeviceHandlerCollection()
    :
    _cRef(1),
    _cDevices(0),
    _pDevices(NULL)
{
    DllAddRef();

} //*** default constructor CMyDeviceHandlerCollection::CMyDeviceHandlerCollection

//----------------------------------------------------------------------------
//
//  Description:
//      Destructor.
//
//----------------------------------------------------------------------------
CMyDeviceHandlerCollection::~CMyDeviceHandlerCollection()
{
    if (_pDevices != NULL)
    {
        delete [] _pDevices;
    }
    DllRelease();

} //*** destructor CMyDeviceHandlerCollection::~CMyDeviceHandlerCollection

//----------------------------------------------------------------------------
// IUnknown (CMyDeviceHandlerCollection)
//----------------------------------------------------------------------------

STDMETHODIMP CMyDeviceHandlerCollection::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CMyDeviceHandlerCollection, ISyncMgrHandlerCollection),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CMyDeviceHandlerCollection::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CMyDeviceHandlerCollection::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;

} //*** CMyDeviceHandlerCollection::Release

//----------------------------------------------------------------------------
// ISyncMgrHandlerCollection (CMyDeviceHandlerCollection)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get an enumerator of sync handlers.
//
//  Implements: ISyncMgrHandlerCollection
//
//  Parameters:
//      ppenum          - IEnumString interface pointer returned by handler.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceHandlerCollection::GetHandlerEnumerator(__deref_out IEnumString **ppenum)
{
    return CEnumDeviceHandlerIDs_CreateInstance(this, IID_PPV_ARGS(ppenum));

} //*** CMyDeviceHandlerCollection::GetHandlerEnumerator

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to bind to (load) a particular handler.
//      Note that there are times when this method will be called before the
//      EnumHandlers() method is called.
//
//  Implements: ISyncMgrHandlerCollection
//
//  Parameters:
//      pszHandlerID    - ID representing the handler to bind to.
//      riid            - ID for the interface pointer to return.
//      ppv             - Interface pointer.
//
//  Return Values:
//      S_OK                        - Operation completed successfully.
//      CLASS_E_CLASSNOTAVAILABLE   - Unrecognized handler ID.
//      Other HRESULTs              - Error loading handlers or creating the handler instance.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceHandlerCollection::BindToHandler(
    __in            LPCWSTR      pszHandlerID,
    __in            REFIID       riid,
    __deref_out     void       **ppv)
{
    HRESULT   hr = CLASS_E_CLASSNOTAVAILABLE;
    HRESULT   hrLoadHandlersIDs = S_OK;

    *ppv = NULL;

    // The BindToHandler() method can be called even if EnumHandlers() hasn't
    // been called.  Therefore it is possible that the handler ID array hasn't
    // been loaded yet.  If it hasn't, load it here so that we can bind to the
    // specified handler.
    if (_pDevices == NULL)
    {
        hrLoadHandlersIDs = _LoadHandlerIDs();
    }

    // Find the device (represented by pszHandlerID) in the table collected by
    // _LoadHandlerIDs().
    if (SUCCEEDED(hrLoadHandlersIDs))
    {
        for (DWORD iDevice = 0; iDevice < _cDevices; iDevice++)
        {
            if (CompareSyncMgrID(pszHandlerID, _pDevices[iDevice].szHandlerID) == 0)
            {
                hr = CMyDeviceSyncHandler_CreateInstance(NULL, riid, &_pDevices[iDevice], ppv);
                break;
            }
        } // for: each device in the table
    } // if: handlers loaded successfully

    return hr;

} //*** CMyDeviceHandlerCollection::BindToHandler

//----------------------------------------------------------------------------
// CMyDeviceHandlerCollection Private Methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Load handler IDs (i.e. devices) to sync from the registry.
//      This works by enumerating partners in the registry for a sample device
//      and loading the partner ID, Sync Center ID, and device name so that
//      it can be used when handlers are enumerated.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating memory for the device info array.
//      Other HRESULTs  - Operation failed.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceHandlerCollection::_LoadHandlerIDs()
{
    HRESULT hr = S_OK;
    HKEY    hkeyPartners = NULL;
    DWORD   cPartners = 0;

    // Open the root of the sample device key to find partnerships.
    DWORD nStatus = RegOpenKeyExW(
                        HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Sample Device\\Partnerships",
                        0,
                        KEY_READ,
                        &hkeyPartners);
    if (nStatus != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(nStatus);
    }

    // Get the number of subkeys to determine the number of partnerships to sync.
    // Then allocate the item array.
    if (SUCCEEDED(hr))
    {
        nStatus = RegQueryInfoKey(hkeyPartners, NULL, NULL, NULL, &cPartners,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        if (nStatus == ERROR_SUCCESS)
        {
            delete [] _pDevices;
            _pDevices = new SYNCDEVICEINFO[cPartners];
            if (_pDevices == NULL)
            {
                hr = E_OUTOFMEMORY;
                _cDevices = 0;
            }
            else
            {
                _cDevices = cPartners;
            }
        } // if: regkey queried successfully
        else
        {
            hr = HRESULT_FROM_WIN32(nStatus);
        }
    } // if: regkey opened successfully

    // Enumerate each subkey and initialize its corresponding device entry in the array.
    if (SUCCEEDED(hr) && (cPartners > 0))
    {
        HKEY  hkeyPartnership = NULL;
        WCHAR szRegString[MAX_PATH];
        for (DWORD iPartner = 0; iPartner < cPartners; iPartner++)
        {
            // If we fail trying to retrieve information for a single handler,
            // we should continue to retrieve the rest of our handlers that
            // belong to this collection.
            HRESULT hrLocal = S_OK;
            // Get the next partner key name.
            nStatus = RegEnumKeyW(hkeyPartners, iPartner, szRegString, ARRAYSIZE(szRegString));
            if (nStatus != ERROR_SUCCESS)
            {
                hrLocal = HRESULT_FROM_WIN32(nStatus);
            }
            if (SUCCEEDED(hrLocal))
            {
                // Save the partner ID.
                _pDevices[iPartner].nPartnerID = wcstoul(szRegString, NULL, 10);

                // Open the partner key.
                nStatus = RegOpenKeyExW(hkeyPartners, szRegString, 0, (KEY_READ | KEY_WRITE), &hkeyPartnership);
                if (nStatus != ERROR_SUCCESS)
                {
                    hrLocal = HRESULT_FROM_WIN32(nStatus);
                }

                if (SUCCEEDED(hrLocal))
                {
                    // Read the sync Center handler ID.
                    DWORD cbData = sizeof(_pDevices[iPartner].szHandlerID);
                    nStatus = RegQueryValueExW(hkeyPartnership, L"SyncCenterGUID", 0, NULL, (PBYTE) _pDevices[iPartner].szHandlerID, &cbData);
                    if (nStatus != ERROR_SUCCESS)
                    {
                        hrLocal = HRESULT_FROM_WIN32(nStatus);
                    }
                    if (FAILED(hrLocal))
                    {
                        // TODO For Developers: Remove demo code
                        // Generate a GUID and write it back to the registry.
                        // This causes every device partnership to show up as a sync partnership.
                        GUID guidHandlerID;
                        hrLocal = CoCreateGuid(&guidHandlerID);
                        if (SUCCEEDED(hrLocal))
                        {
                            StringFromGUID2(guidHandlerID, _pDevices[iPartner].szHandlerID, ARRAYSIZE(_pDevices[iPartner].szHandlerID));
                            nStatus = RegSetValueExW(
                                hkeyPartnership,
                                L"SyncCenterGUID",
                                0,
                                REG_SZ,
                                (const BYTE *) _pDevices[iPartner].szHandlerID,
                                (lstrlenW(_pDevices[iPartner].szHandlerID) + 1) * sizeof(_pDevices[iPartner].szHandlerID[0]));
                            if (nStatus != ERROR_SUCCESS)
                            {
                                hrLocal = HRESULT_FROM_WIN32(nStatus);
                            }
                        } // if: successfully created GUID
                        if (FAILED(hrLocal))
                        {
                            ZeroMemory(_pDevices[iPartner].szHandlerID, sizeof(_pDevices[iPartner].szHandlerID));
                            hrLocal = S_OK;
                        }
                    } // if: failed to read or convert handler ID

                    // Read the partnership name.
                    cbData = sizeof(_pDevices[iPartner].szName);
                    nStatus = RegQueryValueExW(hkeyPartnership, L"DisplayName", 0, NULL, (PBYTE) _pDevices[iPartner].szName, &cbData);
                    if (nStatus != ERROR_SUCCESS)
                    {
                        hrLocal = HRESULT_FROM_WIN32(nStatus);
                    }

                    RegCloseKey(hkeyPartnership);
                    hkeyPartnership = NULL;
                } // if: successfully open the partner key
            } // if: successfully retrieve next partner key name

        } // for: each partner subkey
    } // if: regkey queried successfully

    if (FAILED(hr))
    {
        if (_pDevices != NULL)
        {
            delete [] _pDevices;
        }
        _pDevices = NULL;
    }

    if (hkeyPartners)
    {
        RegCloseKey(hkeyPartners);
    }
    return hr;

} //*** CMyDeviceHandlerCollection::_LoadHandlerIDs

//////////////////////////////////////////////////////////////////////////////
// class CEnumDeviceHandlerIDs
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the enumerator class.
//
//  Parameters:
//      pHandlerCollection  - Handler collection to associate this enumerator with.
//      riid                - Interface ID to get.
//      ppv                 - Interface pointer returned to caller.
//
//  Return Values:
//      S_OK                - Operation completed successfully.
//      E_OUTOFMEMORY       - Error allocating the object.
//      Other HRESULTs      - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CEnumDeviceHandlerIDs_CreateInstance(
    __inout         CMyDeviceHandlerCollection   *pHandlerCollection,
    __in            REFIID                        riid,
    __deref_out     void                        **ppv)
{
    *ppv = NULL;

    // Create an enumerator for the handler IDs.
    CEnumDeviceHandlerIDs *penum = new CEnumDeviceHandlerIDs(pHandlerCollection);
    HRESULT hr = (penum != NULL) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = penum->QueryInterface(riid, ppv);
        penum->Release();
    }

    return hr;

} //*** CEnumDeviceHandlerIDs_CreateInstance

//////////////////////////////////////////////////////////////////////////////
// IUnknown (CEnumDeviceHandlerIDs)
//////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CEnumDeviceHandlerIDs::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CEnumDeviceHandlerIDs, IEnumString),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CEnumDeviceHandlerIDs::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CEnumDeviceHandlerIDs::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;

} //*** CEnumDeviceHandlerIDs::Release

//----------------------------------------------------------------------------
// IEnumString (CEnumDeviceHandlerIDs)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Get the next entry from the enumeration.
//
//  Implements: IEnumString
//
//  Parameters:
//      celt
//          Number of items requested.
//
//      rgelt
//          Array of items returned.  Each item must be deallocated using
//          CoTaskMemFree().
//
//      pceltFetched
//          Number of items returned.
//
//  Return Values:
//      S_OK            - Number of items requested was returned.
//      S_FALSE         - Fewer items than requested was returned.
//      Other HRESULTs
//
//----------------------------------------------------------------------------
STDMETHODIMP CEnumDeviceHandlerIDs::Next(
    __in                        ULONG        celt,
    __deref_out_ecount(celt)    LPWSTR      *rgelt,
    __out_opt                   ULONG       *pceltFetched)
{
    HRESULT hr = S_OK;
    ULONG   cFetched = 0;

    // Load handler IDs if they haven't been loaded already.
    if (_pHandlerCollection->_pDevices == NULL)
    {
        hr = _pHandlerCollection->_LoadHandlerIDs();
    }
    if (SUCCEEDED(hr))
    {
        SYNCDEVICEINFO *pDevices = _pHandlerCollection->_pDevices;
        while ((cFetched < celt) && (_iCur < _pHandlerCollection->_cDevices) && SUCCEEDED(hr))
        {
            // Skip devices without a handler ID.
            // These devices haven't been set up for sync yet.
            if (pDevices[_iCur].szHandlerID[0] != L'\0')
            {
                hr = SHStrDupW(pDevices[_iCur].szHandlerID, &rgelt[cFetched]);
                cFetched++;
            }
            _iCur++;
        } // while: more handlers

        if (FAILED(hr))
        {
            // Deallocate memory if we encountered an error while building
            // the return values..
            while (cFetched > 0)
            {
                --cFetched;
                CoTaskMemFree(rgelt[cFetched]);
                rgelt[cFetched] = NULL;
            }
        } // if: error allocating memory for the handler ID
        else
        {
            hr = (cFetched == celt) ? S_OK : S_FALSE;
        }
    } // if: handler IDs loaded successfully

    if (pceltFetched != NULL)
    {
        *pceltFetched = cFetched;
    }

    return hr;

} //*** CEnumDeviceHandlerIDs::Next

