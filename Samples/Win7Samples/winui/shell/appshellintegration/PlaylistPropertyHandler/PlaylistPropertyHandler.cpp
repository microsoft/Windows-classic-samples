// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "dll.h"
#include "RegisterExtension.h"
#include "PropertyStoreHelpers.h"
#include <shobjidl.h>
#include <shlwapi.h>
#include <propvarutil.h>
#include <propkey.h>
#include <msxml6.h>      // DOM interfaces, consider using XMLLite instead

class DECLSPEC_UUID("9DBD2C50-62AD-11D0-B806-00C04FD706EC") PropertyThumbnailHandler;

// MSXML is a dispatch based OM and requires BSTRs as input
// we will cheat and cast C strings to BSTRs as MSXML does not
// utalize the BSTR features of its inputs
// check with the MSXML guys to see if this is OK

__inline BSTR MAKEBSTR(PCWSTR psz) { return (BSTR)psz; }

//<smil>
//  <head>
//    <meta name="generator" content="Zune -- 2.5.447.0" />
//    <meta name="itemCount" content="93" />
//    <meta name="totalDuration" content="20657951" />
//    <meta name="creatorId" content="{2B86B47C-5302-0119-2982-A1676DAB3DF4}" />
//    <meta name="averageRating" content="0" />
//    <title>Spring 2008</title>
//    <guid>{C34F5F53-E91F-4BAA-AF47-516A8F742E63}</guid>
//    <properties>base64_property_store</properties>
//  </head>
//</smil>

struct PROPERTYMAP
{
    const PROPERTYKEY *pkey;
    BOOL fOverrideMeta;         // normally properties in <meta> are read only and
                                // prefered over values found in <properties>
                                // this flag lets them be written by letting the
                                // a value in <properties> be prefered
    PCWSTR pszValueNodeName;
    PCWSTR pszMetaNameValue;
};

// all elements are in smil/head
const PROPERTYMAP c_rgPROPERTYMAP[] =
{
    { &PKEY_Title,               false, L"title",   NULL },
    { &PKEY_Media_ContentID,     false, L"guid",    NULL },
    { &PKEY_ApplicationName,     false, L"meta",    L"generator" },
    { &PKEY_FileCount,           false, L"meta",    L"itemCount" },
    { &PKEY_Media_Duration,      false, L"meta",    L"totalDuration" },
    { &PKEY_Rating,              true,  L"meta",    L"averageRating" },
};

// TODO:
// the target items display name and file name should be inlcuded in PKEY_Search_Contents to enable searches
// for those names to return the playlist file

class CPlaylistPropertyHandler :
    public IPropertyStore,
    public IInitializeWithStream,
    public IPropertyStoreCapabilities,
    public IObjectProvider
{
public:
    CPlaylistPropertyHandler() : _cRef(1), _pStream(NULL), _grfMode(0), _pDomDoc(NULL), _pCache(NULL)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CPlaylistPropertyHandler, IPropertyStore),             // required
            QITABENT(CPlaylistPropertyHandler, IInitializeWithStream),      // required
            QITABENT(CPlaylistPropertyHandler, IPropertyStoreCapabilities), // optional
            QITABENT(CPlaylistPropertyHandler, IObjectProvider),            // optional
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

    // IPropertyStoreCapabilities
    IFACEMETHODIMP IsPropertyWritable(REFPROPERTYKEY key)
    {
        HRESULT hr = S_OK;  // by default yes, we support openmetadata
        for (int i = 0; i < ARRAYSIZE(c_rgPROPERTYMAP); i++)
        {
            if (IsEqualPropertyKey(*c_rgPROPERTYMAP[i].pkey, key) &&
                !c_rgPROPERTYMAP[i].fOverrideMeta)
            {
                hr = S_FALSE;   // no, not writeable
                break;
            }
        }
        return hr;
    }

    // by implementing IObjectProvider clients of this property handler can
    // get to this object and access a handler specific programming model.
    // those clients use IObjectProvider::QueryObject(<object ID defined by the handler>, ...)
    // this would be useful for clients that need access to special features of
    // a particular file format but want to mostly use the IPropertyStore abstraction
    IFACEMETHODIMP QueryObject(REFGUID guidObject, REFIID riid, void **ppv)
    {
        *ppv = NULL;
        HRESULT hr = E_NOTIMPL;
        // provide access to this object directly, but in this case only
        // on IUnknown. provide access to handler specific features
        // by creating a handler specific interface that can be retrieved here
        if (guidObject == __uuidof(CPlaylistPropertyHandler))
        {
            hr = QueryInterface(riid, ppv);
        }
        return hr;
    }

private:

    HRESULT _LoadCacheFromDom();
    HRESULT _LoadMetaPropertyFromNode(IXMLDOMNode *pNodeParent, const PROPERTYMAP &map, PROPVARIANT *ppropvar);
    HRESULT _LoadProperty(const PROPERTYMAP &map);

    HRESULT _SaveCacheToDom();
    HRESULT _SaveToStream();
    HRESULT _SavePropertyValue(IXMLDOMNode *pNodeParent, const PROPERTYMAP &map, REFPROPVARIANT propvar);
    HRESULT _SaveProperty(REFPROPVARIANT propvar, const PROPERTYMAP &map);

    void _ReleaseResources()
    {
        SafeRelease(&_pStream);
        SafeRelease(&_pCache);
        SafeRelease(&_pDomDoc);
    }

    ~CPlaylistPropertyHandler()
    {
        _ReleaseResources();
        DllRelease();
    }

    long _cRef;
    IStream *_pStream;
    DWORD _grfMode;
    IXMLDOMDocument*     _pDomDoc; // DOM object representing the file
    IPropertyStoreCache *_pCache;  // internal value cache to abstract IPropertyStore operations from the DOM back-end
};

HRESULT CPlaylistPropertyHandler_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    CPlaylistPropertyHandler *pirm = new (std::nothrow) CPlaylistPropertyHandler();
    HRESULT hr = pirm ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pirm->QueryInterface(riid, ppv);
        pirm->Release();
    }
    return hr;
}

HRESULT CPlaylistPropertyHandler::GetCount(DWORD *pcProps)
{
    *pcProps = 0;
    return _pCache ? _pCache->GetCount(pcProps) : E_UNEXPECTED;
}

HRESULT CPlaylistPropertyHandler::GetAt(DWORD iProp, PROPERTYKEY *pkey)
{
    *pkey = PKEY_Null;
    return _pCache ? _pCache->GetAt(iProp, pkey) : E_UNEXPECTED;
}

HRESULT CPlaylistPropertyHandler::GetValue(REFPROPERTYKEY key, PROPVARIANT *pPropVar)
{
    PropVariantInit(pPropVar);
    return _pCache ? _pCache->GetValue(key, pPropVar) : E_UNEXPECTED;
}

// SetValue just updates the internal value cache
HRESULT CPlaylistPropertyHandler::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propVar)
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

HRESULT CPlaylistPropertyHandler::_SavePropertyValue(
    IXMLDOMNode *pNodeParent, const PROPERTYMAP &map,
    REFPROPVARIANT propvar)
{
    BSTR bstrValue;
    HRESULT hr = PropVariantToBSTR(propvar, &bstrValue);
    if (SUCCEEDED(hr))
    {
        // create an element and set its text to the value
        IXMLDOMElement *pValue;
        hr = _pDomDoc->createElement(MAKEBSTR(map.pszValueNodeName), &pValue);
        if (SUCCEEDED(hr))
        {
            hr = pValue->put_text(bstrValue);
            if (SUCCEEDED(hr))
            {
                // append the value to its parent node
                hr = pNodeParent->appendChild(pValue, NULL);
            }
            pValue->Release();
        }

        SysFreeString(bstrValue);
    }
    return hr;
}

HRESULT CPlaylistPropertyHandler::_SaveProperty(REFPROPVARIANT propvar, const PROPERTYMAP &map)
{
    // obtain the parent node of the value
    IXMLDOMNode *pNodeParent;
    HRESULT hr = _pDomDoc->selectSingleNode(MAKEBSTR(L"smil/head"), &pNodeParent);
    if (hr == S_OK)
    {
        // remove existing value nodes
        IXMLDOMNodeList *pNodeListValues;
        hr = pNodeParent->selectNodes(MAKEBSTR(map.pszValueNodeName), &pNodeListValues);
        if (hr == S_OK)
        {
            IXMLDOMSelection *pSelectionValues;
            hr = pNodeListValues->QueryInterface(&pSelectionValues);
            if (SUCCEEDED(hr))
            {
                hr = pSelectionValues->removeAll();
                pSelectionValues->Release();
            }

            pNodeListValues->Release();
        }

        if (SUCCEEDED(hr))
        {
            // save the new values to the parent node
            hr = _SavePropertyValue(pNodeParent, map, propvar);
        }

        pNodeParent->Release();
    }

    return hr;
}

HRESULT CPlaylistPropertyHandler::_SaveCacheToDom()
{
    HRESULT hr = S_OK;

    for (UINT i = 0; SUCCEEDED(hr) && (i < ARRAYSIZE(c_rgPROPERTYMAP)); i++)
    {
        PROPVARIANT propvar;
        if (SUCCEEDED(_pCache->GetValue(*c_rgPROPERTYMAP[i].pkey, &propvar)) &&
            (propvar.vt != VT_EMPTY))
        {
            if (!c_rgPROPERTYMAP[i].pszMetaNameValue)
            {
                // only non meta values can be saved
                hr = _SaveProperty(propvar, c_rgPROPERTYMAP[i]);
            }

            if (!c_rgPROPERTYMAP[i].fOverrideMeta)
            {
                // clear the value in the cache so we don't save this redundantly
                const PROPVARIANT vEmpty = {};
                _pCache->SetValue(*c_rgPROPERTYMAP[i].pkey, vEmpty);
            }

            PropVariantClear(&propvar);
        }
    }

    if (SUCCEEDED(hr))
    {
        IXMLDOMNode *pPropertiesNode = NULL;

        IXMLDOMNode *pNode;
        hr = _pDomDoc->selectSingleNode(MAKEBSTR(L"smil/head/properties"), &pNode);
        if (S_OK == hr)
        {
            hr = pNode->QueryInterface(&pPropertiesNode);
            pNode->Release();
        }
        else
        {
            hr = _pDomDoc->selectSingleNode(MAKEBSTR(L"smil/head"), &pNode);
            if (hr == S_OK)
            {
                IXMLDOMElement *pChildElem;
                hr = _pDomDoc->createElement(MAKEBSTR(L"properties"), &pChildElem);
                if (SUCCEEDED(hr))
                {
                    hr = pNode->appendChild(pChildElem, &pPropertiesNode);
                    pChildElem->Release();
                }
                pNode->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            PWSTR pszBase64;
            hr = SavePropertyStoreToString(_pCache, &pszBase64);
            if (SUCCEEDED(hr))
            {
                hr = pPropertiesNode->put_text(MAKEBSTR(pszBase64));
                CoTaskMemFree(pszBase64);
            }

            pPropertiesNode->Release();
        }
    }
    return hr;
}

HRESULT CPlaylistPropertyHandler::_SaveToStream()
{
    IStream *pStreamSaveTo;
    HRESULT hr = GetSafeSaveStream(_pStream, &pStreamSaveTo);
    if (SUCCEEDED(hr))
    {
        VARIANT varStream = {};
        varStream.vt = VT_UNKNOWN;
        varStream.punkVal = pStreamSaveTo;
        hr = _pDomDoc->save(varStream);
        if (SUCCEEDED(hr))
        {
            hr = _pStream->Commit(STGC_DEFAULT);
        }
        pStreamSaveTo->Release();
    }
    return hr;
}

// Commit writes the internal value cache back out to the stream passed to Initialize
HRESULT CPlaylistPropertyHandler::Commit()
{
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        hr = STG_E_ACCESSDENIED;
        if (_grfMode & STGM_READWRITE)  // must be opened for writing
        {
            hr = _SaveCacheToDom();
            if (SUCCEEDED(hr))
            {
                hr = _SaveToStream();
            }
        }
    }
    return hr;
}

HRESULT LoadDomDocFromStream(IStream *pStream, IXMLDOMDocument **ppDomDoc)
{
    *ppDomDoc = NULL;

    IXMLDOMDocument *pDomDoc;
    HRESULT hr = CoCreateInstance(CLSID_DOMDocument60, NULL, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pDomDoc));
    if (SUCCEEDED(hr))
    {
        pDomDoc->put_validateOnParse(VARIANT_FALSE);
        pDomDoc->put_async(VARIANT_FALSE);
        pDomDoc->put_resolveExternals(VARIANT_FALSE);
        pDomDoc->put_preserveWhiteSpace(VARIANT_TRUE);

        // load the DOM object's contents from the stream
        VARIANT varStream = {};
        varStream.vt = VT_UNKNOWN;
        varStream.punkVal = pStream;

        VARIANT_BOOL vfSuccess = VARIANT_FALSE;
        hr = pDomDoc->load(varStream, &vfSuccess);
        if ((hr == S_OK) && (vfSuccess == VARIANT_TRUE))
        {
            hr = pDomDoc->QueryInterface(ppDomDoc);
        }
        else
        {
            hr = E_FAIL;
        }
        pDomDoc->Release();
    }
    return hr;
}

HRESULT CPlaylistPropertyHandler::_LoadMetaPropertyFromNode(
    IXMLDOMNode *pNodeParent, const PROPERTYMAP &map, PROPVARIANT *ppropvar)
{
    // the <smil/head/meta> elements need special attention
    IXMLDOMNodeList *pElementsToSearch;
    HRESULT hr = pNodeParent->selectNodes(MAKEBSTR(map.pszValueNodeName), &pElementsToSearch);
    if (hr == S_OK)
    {
        long cElements;
        hr = pElementsToSearch->get_length(&cElements);

        for (long iElements = 0; (ppropvar->vt == VT_EMPTY) && SUCCEEDED(hr) && (iElements < cElements); iElements++)
        {
            IXMLDOMNode *pElement;
            hr = pElementsToSearch->get_item(iElements, &pElement);
            if (S_OK == hr)
            {
                IXMLDOMNamedNodeMap *pAttributes;
                hr = pElement->get_attributes(&pAttributes);
                if (S_OK == hr)
                {
                    IXMLDOMNode *pNameAttribute;
                    hr = pAttributes->getNamedItem(MAKEBSTR(L"name"), &pNameAttribute);
                    if (S_OK == hr)
                    {
                        BSTR bstrNameValue;
                        hr = pNameAttribute->get_text(&bstrNameValue);
                        if (S_OK == hr)
                        {
                            if (StrCmpIC(bstrNameValue, map.pszMetaNameValue) == 0)
                            {
                                IXMLDOMNode *pContentAttribute;
                                hr = pAttributes->getNamedItem(MAKEBSTR(L"content"), &pContentAttribute);
                                if (S_OK == hr)
                                {
                                    hr = pContentAttribute->get_text(&ppropvar->bstrVal);
                                    if (S_OK == hr)
                                    {
                                        ppropvar->vt = VT_BSTR;
                                        hr = PSCoerceToCanonicalValue(*map.pkey, ppropvar);
                                        if (SUCCEEDED(hr))
                                        {
                                            if (IsEqualPropertyKey(*map.pkey, PKEY_Media_Duration) && (ppropvar->vt == VT_UI8))
                                            {
                                                ppropvar->uhVal.QuadPart *= 1000;
                                            }
                                        }
                                    }
                                    pContentAttribute->Release();
                                }
                            }

                            SysFreeString(bstrNameValue);
                        }
                        pNameAttribute->Release();
                    }
                    pAttributes->Release();
                }
                pElement->Release();
            }
        }
        pElementsToSearch->Release();
    }
    return hr;
}

HRESULT CPlaylistPropertyHandler::_LoadProperty(const PROPERTYMAP &map)
{
    // select the property's parent node and load the property's value(s)
    IXMLDOMNode *pNodeParent = NULL;
    HRESULT hr = _pDomDoc->selectSingleNode(MAKEBSTR(L"smil/head"), &pNodeParent);
    if (hr == S_OK)
    {
        PROPVARIANT propvar = {};

        if (map.pszMetaNameValue)
        {
            // enable some properties (System.Rating) to be writeable
            // even though it is found in a <meta> element
            if (map.fOverrideMeta && TestForPropertyKey(_pCache, *map.pkey))
            {
                hr = S_FALSE;
            }
            else
            {
                hr = _LoadMetaPropertyFromNode(pNodeParent, map, &propvar);
            }
        }
        else
        {
            IXMLDOMNode *pNodeValue;
            hr = pNodeParent->selectSingleNode(MAKEBSTR(map.pszValueNodeName), &pNodeValue);
            if (hr == S_OK)
            {
                hr = pNodeValue->get_text(&propvar.bstrVal);
                if (S_OK == hr)
                {
                    propvar.vt = VT_BSTR;
                    // coerce the value(s) to the appropriate type for the property key
                    hr = PSCoerceToCanonicalValue(*map.pkey, &propvar);
                }
                pNodeValue->Release();
            }
        }
        if (hr == S_OK)
        {
            hr = _pCache->SetValueAndState(*map.pkey, &propvar, PSC_NORMAL);
            PropVariantClear(&propvar);
        }
        pNodeParent->Release();
    }
    return hr;
}

HRESULT CPlaylistPropertyHandler::_LoadCacheFromDom()
{
    HRESULT hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&_pCache));
    if (SUCCEEDED(hr))
    {
        // first load the open open metadata properties (if there are any)

        IXMLDOMNode *pNode;
        hr = _pDomDoc->selectSingleNode(MAKEBSTR(L"smil/head/properties"), &pNode);
        if (S_OK == hr)
        {
            BSTR bstrProperties;
            hr = pNode->get_text(&bstrProperties);
            if (S_OK == hr)
            {
                IStream *pstm;
                hr = GetStreamFromBase64String(bstrProperties, &pstm);
                if (SUCCEEDED(hr))
                {
                    IPersistStream *pps;
                    hr = _pCache->QueryInterface(&pps);
                    if (SUCCEEDED(hr))
                    {
                        hr = pps->Load(pstm);
                        pps->Release();
                    }
                    pstm->Release();
                }
                SysFreeString(bstrProperties);
            }
            pNode->Release();
        }

        // load the properties from the smil/head WPL schema
        // these are loaded last to override any values loaded above

        for (UINT i = 0; SUCCEEDED(hr) && (i < ARRAYSIZE(c_rgPROPERTYMAP)); ++i)
        {
            hr = _LoadProperty(c_rgPROPERTYMAP[i]);
        }
    }

    return hr;
}

HRESULT CPlaylistPropertyHandler::Initialize(IStream *pStream, DWORD grfMode)
{
    HRESULT hr = E_UNEXPECTED;
    if (!_pStream)
    {
        hr = LoadDomDocFromStream(pStream, &_pDomDoc);
        if (SUCCEEDED(hr))
        {
            hr = _LoadCacheFromDom();
            if (SUCCEEDED(hr))
            {
                // save a reference to the stream as well as the grfMode
                hr = pStream->QueryInterface(&_pStream);
                if (SUCCEEDED(hr))
                {
                    _grfMode = grfMode;
                }
            }

            if (FAILED(hr))
            {
                _ReleaseResources();
            }
        }
    }
    return hr;
}

HRESULT RegisterPropLists(CRegisterExtension &re, PCWSTR pszFileAssoc)
{
    const WCHAR c_szPlaylistTileInfo[] = L"prop:System.ItemType;*System.FileCount;*System.Media.Diration;System.DateModified";
    const WCHAR c_szPlaylistPreviewDetails[] = L"prop:*System.Media.Duration;System.Rating;*System.DateModified;System.Keywords;System.ParentalRating;*System.OfflineAvailability;*System.OfflineStatus;*System.DateCreated;*System.SharedWith";
    const WCHAR c_szPlaylistInfoTip[] = L"prop:System.ItemType;*System.FileCount;*System.Media.Diration;System.DateModified;System.Size";
    const WCHAR c_szPlaylistFullDetails[] = L"prop:System.PropGroup.Description;System.Title;System.Media.SubTitle;System.Rating;System.Keywords;System.Comment;System.PropGroup.Media;System.Music.Artist;System.Music.AlbumArtist;System.Music.AlbumTitle;System.Media.Year;System.Music.TrackNumber;System.Music.Genre;System.Media.Duration;System.PropGroup.Audio;System.Audio.EncodingBitrate;System.PropGroup.Origin;System.Media.Producer;System.Media.Publisher;System.Media.ContentDistributor;System.Media.DateEncoded;System.Media.EncodedBy;System.Media.AuthorUrl;System.Media.PromotionUrl;System.Copyright;System.PropGroup.Content;System.ParentalRating;System.ParentalRatingReason;System.Music.Composer;System.Music.Conductor;System.Music.ContentGroupDescription;System.Music.Period;System.Music.Mood;System.Music.PartOfSet;System.Music.InitialKey;System.Music.BeatsPerMinute;System.DRM.IsProtected;System.PropGroup.FileSystem;System.ItemNameDisplay;System.ItemType;System.ItemFolderPathDisplay;System.DateCreated;System.DateModified;System.Size;System.FileAttributes;System.OfflineAvailability;System.OfflineStatus;System.SharedWith;System.FileOwner;System.ComputerName";
    const WCHAR c_szPlaylistExtendedTileInfo[] = L"prop:System.ItemType;System.Size;System.Music.Artist;System.Media.Duration;System.OfflineAvailability";

    HRESULT hr = re.RegisterProgIDValue(pszFileAssoc, L"TileInfo", c_szPlaylistTileInfo);
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterProgIDValue(pszFileAssoc, L"PreviewDetails", c_szPlaylistPreviewDetails);
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterProgIDValue(pszFileAssoc, L"InfoTip", c_szPlaylistInfoTip);
            if (SUCCEEDED(hr))
            {
                hr = re.RegisterProgIDValue(pszFileAssoc, L"FullDetails", c_szPlaylistFullDetails);
                if (SUCCEEDED(hr))
                {
                    hr = re.RegisterProgIDValue(pszFileAssoc, L"ExtendedTileInfo", c_szPlaylistExtendedTileInfo);
                }
            }
        }
    }
    return hr;
}

const WCHAR c_szWplFileExtension[] = L".wpl";
const WCHAR c_szWplProgID[] = L"Windows.WPL";
const WCHAR c_szZplFileExtension[] = L".zpl";
const WCHAR c_szZplProgID[] = L"Windows.ZPL";

HRESULT RegisterHandler()
{
    // register the property handler COM object, and set the options it uses
    const WCHAR c_szPropertyHandlerDescription[] = L"Playlist (.wpl, .zpl) Property Handler";
    CRegisterExtension re(__uuidof(CPlaylistPropertyHandler), HKEY_LOCAL_MACHINE);
    HRESULT hr = re.RegisterInProcServer(c_szPropertyHandlerDescription, L"Both");
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterInProcServerAttribute(L"ManualSafeSave", TRUE);
    }

    // Property Handler registrations use a different mechanism than the rest of the filetype association system, and do not use ProgIDs
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterPropertyHandler(c_szWplFileExtension);
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterPropertyHandler(c_szZplFileExtension);
        }
    }

    // Associate our ProgIDs with the file extensions, and write the proplists to the ProgID to minimize conflicts with other applications and facilitate easy unregistration
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterExtensionWithProgID(c_szWplFileExtension, c_szWplProgID);
        if (SUCCEEDED(hr))
        {
            hr = RegisterPropLists(re, c_szWplProgID);
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterExtensionWithProgID(c_szZplFileExtension, c_szZplProgID);
        if (SUCCEEDED(hr))
        {
            hr = RegisterPropLists(re, c_szZplProgID);
        }
    }

    // also register the property-driven thumbnail handler on the ProgIDs
    if (SUCCEEDED(hr))
    {
        re.SetHandlerCLSID(__uuidof(PropertyThumbnailHandler));
        hr = re.RegisterThumbnailHandler(c_szWplProgID);
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterThumbnailHandler(c_szZplProgID);
        }
    }
    return hr;
}

HRESULT UnregisterHandler()
{
    // Unregister the property handler COM object.
    CRegisterExtension re(__uuidof(CPlaylistPropertyHandler), HKEY_LOCAL_MACHINE);
    HRESULT hr = re.UnRegisterObject();
    if (SUCCEEDED(hr))
    {
        // Unregister the property handler the file extensions.
        hr = re.UnRegisterPropertyHandler(c_szWplFileExtension);
        if (SUCCEEDED(hr))
        {
            hr = re.UnRegisterPropertyHandler(c_szZplFileExtension);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Remove the whole ProgIDs since we own all of those settings.
        // Don't try to remove the file extension association since some other application may have overridden it with their own ProgID in the meantime.
        // Leaving the association to a non-existing ProgID is handled gracefully by the Shell.
        // NOTE: If the file extension is unambiguously owned by this application, the association to the ProgID could be safely removed as well,
        //       along with any other association data stored on the file extension itself.
        hr = re.UnRegisterProgID(c_szWplProgID, c_szWplFileExtension);
        if (SUCCEEDED(hr))
        {
            hr = re.UnRegisterProgID(c_szZplProgID, c_szZplFileExtension);
        }
    }
    return hr;
}
