// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <shlwapi.h>
#include <propkey.h>
#include "dll.h"
#include "RegisterExtension.h"
#include "PropertyStoreHelpers.h"

class COpenMetadataHandler : public IPropertyStore, public IInitializeWithStream
{
public:
    COpenMetadataHandler() : _cRef(1), _grfMode(0), _pStream(NULL), _pCache(NULL)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(COpenMetadataHandler, IPropertyStore),
            QITABENT(COpenMetadataHandler, IInitializeWithStream),
            {0, 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IPropertyStore
    IFACEMETHODIMP GetCount(DWORD *pcProps);
    IFACEMETHODIMP GetAt(DWORD iProp, PROPERTYKEY *pkey);
    IFACEMETHODIMP GetValue(REFPROPERTYKEY key, PROPVARIANT *pPropVar);
    IFACEMETHODIMP SetValue(REFPROPERTYKEY key, REFPROPVARIANT propVar);
    IFACEMETHODIMP Commit();

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

private:
    HRESULT _SaveToStream();
    ~COpenMetadataHandler()
    {
        SafeRelease(&_pStream);
        SafeRelease(&_pCache);
        DllRelease();
    }

    long _cRef;
    DWORD _grfMode;
    IStream *_pStream;
    IPropertyStoreCache *_pCache;  // internal value cache to abstract IPropertyStore operations from the DOM back-end
};

HRESULT COpenMetadataHandler_CreateInstance(REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    COpenMetadataHandler *pirm = new (std::nothrow) COpenMetadataHandler();
    if (pirm)
    {
        hr = pirm->QueryInterface(riid, ppv);
        pirm->Release();
    }
    return hr;
}

HRESULT COpenMetadataHandler::GetCount(DWORD *pcProps)
{
    *pcProps = 0;
    return _pCache ? _pCache->GetCount(pcProps) : E_UNEXPECTED;
}

HRESULT COpenMetadataHandler::GetAt(DWORD iProp, PROPERTYKEY *pkey)
{
    *pkey = PKEY_Null;
    return _pCache ? _pCache->GetAt(iProp, pkey) : E_UNEXPECTED;
}

HRESULT COpenMetadataHandler::GetValue(REFPROPERTYKEY key, PROPVARIANT *pPropVar)
{
    PropVariantInit(pPropVar);
    return _pCache ? _pCache->GetValue(key, pPropVar) : E_UNEXPECTED;
}

// SetValue just updates the internal value cache
HRESULT COpenMetadataHandler::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propVar)
{
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        // check grfMode to ensure writes are allowed
        hr = STG_E_ACCESSDENIED;
        if (_grfMode & STGM_READWRITE)
        {
            hr = _pCache->SetValueAndState(key, &propVar, PSC_DIRTY);
        }
    }
    return hr;
}

HRESULT COpenMetadataHandler::_SaveToStream()
{
    IStream *pStreamSaveTo;
    HRESULT hr = GetSafeSaveStream(_pStream, &pStreamSaveTo);
    if (SUCCEEDED(hr))
    {
        // write the XML out to the temprorary stream and commit it
        hr = SavePropertyStoreToStream(_pCache, pStreamSaveTo);
        if (SUCCEEDED(hr))
        {
            hr = _pStream->Commit(STGC_DEFAULT);    // also commits pStreamSaveTo
        }
        pStreamSaveTo->Release();
    }
    return hr;
}

// Commit writes the internal value cache back out to the stream passed to Initialize
HRESULT COpenMetadataHandler::Commit()
{
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        hr = STG_E_ACCESSDENIED;
        if (_grfMode & STGM_READWRITE)  // must be opened for writing
        {
            hr = _SaveToStream();
        }
    }
    return hr;
}

HRESULT COpenMetadataHandler::Initialize(IStream *pStream, DWORD grfMode)
{
    HRESULT hr = E_UNEXPECTED;
    if (!_pStream)
    {
        hr = LoadPropertyStoreFromStream(pStream, IID_PPV_ARGS(&_pCache));
        if (SUCCEEDED(hr))
        {
            // save a reference to the stream as well as the grfMode
            hr = pStream->QueryInterface(&_pStream);
            if (SUCCEEDED(hr))
            {
                _grfMode = grfMode;
            }
        }
    }
    return hr;
}

const WCHAR c_szOpenMetadataFileExtension[] = L".openmetadata-ms";
const WCHAR c_szOpenMetadataProgID[] = L"Windows.OpenMetadata";
const WCHAR c_szOpenMetadataDescription[] = L"Open Metadata File";

HRESULT RegisterOpenMetadata()
{
    // register the property handler COM object, and set the options it uses
    CRegisterExtension re(__uuidof(COpenMetadataHandler), HKEY_LOCAL_MACHINE);
    HRESULT hr = re.RegisterInProcServer(c_szOpenMetadataDescription, L"Both");
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterInProcServerAttribute(L"ManualSafeSave", TRUE);
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterInProcServerAttribute(L"EnableShareDenyWrite", TRUE);
            if (SUCCEEDED(hr))
            {
                hr = re.RegisterInProcServerAttribute(L"EnableShareDenyNone", TRUE);
            }
        }
    }

    // Property Handler and Kind registrations use a different mechanism than the rest of the filetype association system, and do not use ProgIDs
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterPropertyHandler(c_szOpenMetadataFileExtension);
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterPropertyHandlerOverride(L"System.Kind");
        }
    }

    // Associate our ProgID with the file extension, and write the remainder of the registration data to the ProgID to minimize conflicts with other applications and facilitate easy unregistration
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterExtensionWithProgID(c_szOpenMetadataFileExtension, c_szOpenMetadataProgID);
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterProgID(c_szOpenMetadataProgID, c_szOpenMetadataDescription, IDI_ICON_OPENMETADATA);
            if (SUCCEEDED(hr))
            {
                hr = re.RegisterProgIDValue(c_szOpenMetadataProgID, L"NoOpen", L"This is a sample file type and does not have any apps installed to handle it");
                if (SUCCEEDED(hr))
                {
                    hr = re.RegisterNewMenuNullFile(c_szOpenMetadataFileExtension, c_szOpenMetadataProgID);
                    if (SUCCEEDED(hr))
                    {
                        hr = re.RegisterProgIDValue(c_szOpenMetadataProgID, L"FullDetails", c_szDocFullDetails);
                        if (SUCCEEDED(hr))
                        {
                            hr = re.RegisterProgIDValue(c_szOpenMetadataProgID, L"InfoTip", c_szDocInfoTip);
                            if (SUCCEEDED(hr))
                            {
                                hr = re.RegisterProgIDValue(c_szOpenMetadataProgID, L"PreviewDetails", c_szDocPreviewDetails);
                            }
                        }
                    }
                }
            }
        }
    }

    // also register the property-driven thumbnail handler on the ProgID
    if (SUCCEEDED(hr))
    {
        re.SetHandlerCLSID(__uuidof(PropertyThumbnailHandler));
        hr = re.RegisterThumbnailHandler(c_szOpenMetadataProgID);
    }
    return hr;
}

HRESULT UnregisterOpenMetadata()
{
    // Unregister the property handler COM object.
    CRegisterExtension re(__uuidof(COpenMetadataHandler), HKEY_LOCAL_MACHINE);
    HRESULT hr = re.UnRegisterObject();
    if (SUCCEEDED(hr))
    {
        // Unregister the property handler and kind for the file extension.
        hr = re.UnRegisterPropertyHandler(c_szOpenMetadataFileExtension);
        if (SUCCEEDED(hr))
        {
            hr = re.UnRegisterKind(c_szOpenMetadataFileExtension);
            if (SUCCEEDED(hr))
            {
                // Remove the whole ProgID since we own all of those settings.
                // Don't try to remove the file extension association since some other application may have overridden it with their own ProgID in the meantime.
                // Leaving the association to a non-existing ProgID is handled gracefully by the Shell.
                // NOTE: If the file extension is unambiguously owned by this application, the association to the ProgID could be safely removed as well,
                //       along with any other association data stored on the file extension itself.
                hr = re.UnRegisterProgID(c_szOpenMetadataProgID, c_szOpenMetadataFileExtension);
            }
        }
    }
    return hr;
}
