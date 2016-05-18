// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tedmemo.h"
#include <assert.h>

////////////////////////////////////////////////////
//

CTedAttributesSerializer::CTedAttributesSerializer(IMFAttributes* pAttributes)
    : m_pAttributes(pAttributes)
{
    m_pAttributes->AddRef();
}

CTedAttributesSerializer::~CTedAttributesSerializer()
{
    m_pAttributes->Release();
}

HRESULT CTedAttributesSerializer::Serialize(ITedDataSaver* pSaver)
{
    HRESULT hr;
    CAtlString converter;

    IFC( m_pAttributes->LockStore() );

    UINT32 cItems;
    IFC( m_pAttributes->GetCount(&cItems) );

    DWORD cUsedItems = 0;        
    for(UINT32 i = 0; i < cItems; i++)
    {
        GUID gidKey;
        PROPVARIANT var;
        PropVariantInit(&var);

        IFC( m_pAttributes->GetItemByIndex(i, &gidKey, &var) );

        if(VT_LPWSTR == var.vt || VT_UI4 == var.vt || VT_UI8 == var.vt || VT_R8 == var.vt || VT_CLSID == var.vt)
        {
            cUsedItems++;
        }

        PropVariantClear(&var);
    }

    converter.Format(L"%d", cUsedItems);
    IFC( pSaver->SaveData(L"ItemCount", converter) );

    for(UINT32 i = 0; i < cItems; i++)
    {
        GUID gidKey;
        PROPVARIANT var;
        PropVariantInit(&var);

        IFC( m_pAttributes->GetItemByIndex(i, &gidKey, &var) );

        if(VT_LPWSTR == var.vt || VT_UI4 == var.vt || VT_UI8 == var.vt || VT_R8 == var.vt || VT_CLSID == var.vt)
        {
            IFC( WriteGUID(pSaver, L"Key", gidKey) );
            IFC( WritePropVar(pSaver, L"Item", var) );
        }

        PropVariantClear(&var);
    }

    IFC( m_pAttributes->UnlockStore() );

Cleanup:
    return hr;
}

HRESULT CTedAttributesSerializer::Deserialize(ITedDataLoader* pLoader)
{
    HRESULT hr;
    UINT32 cItems;
    LPWSTR szData;

    IFC( pLoader->LoadData(L"ItemCount", &szData, 0) );
    cItems = wcstoul(szData, NULL, 10);
    CoTaskMemFree(szData);

    for(UINT32 i = 0; i < cItems; i++)
    {
        GUID gidKey;
        PROPVARIANT var;
        PropVariantInit(&var);

        IFC( ReadGUID(pLoader, L"Key", i, &gidKey) );
        IFC( ReadPropVar(pLoader, L"Item", i, var) );
        IFC( m_pAttributes->SetItem(gidKey, var) );

        PropVariantClear(&var);
    }

Cleanup:
    return hr;
}

HRESULT CTedAttributesSerializer::WriteGUID(ITedDataSaver* pSaver, CAtlString strTag, GUID gidToWrite)
{
    HRESULT hr;
    CAtlString converter;
    LPOLESTR strGuid = NULL;

    IFC( StringFromCLSID(gidToWrite, &strGuid) );

    converter.Format(L"%s", strGuid);
    IFC( pSaver->SaveData(strTag, converter) );

Cleanup:
    CoTaskMemFree(strGuid);

    return hr;
}

HRESULT CTedAttributesSerializer::WritePropVar(ITedDataSaver* pSaver, CAtlString strTag, PROPVARIANT& varToWrite)
{
    HRESULT hr;
    CAtlString converter;

    converter.Format(L"%d", varToWrite.vt);
    IFC( pSaver->SaveData(L"Type", converter) );

    switch(varToWrite.vt)
    {
    case VT_LPWSTR:
        IFC( pSaver->SaveData(strTag, varToWrite.pwszVal) );
        break;
    case VT_CLSID:
        IFC( WriteGUID(pSaver, strTag, *(varToWrite.puuid) ) );
        break;
    case VT_UI4:
        converter.Format(L"%d", varToWrite.ulVal);
        IFC( pSaver->SaveData(strTag, converter) );
        break;
    case VT_UI8:
        converter.Format(L"%I64d", varToWrite.uhVal.QuadPart);
        IFC( pSaver->SaveData(strTag, converter) );
        break;
    case VT_R8:
        converter.Format(L"%f", varToWrite.dblVal);
        IFC( pSaver->SaveData(strTag, converter) );
        break;
    case VT_VECTOR | VT_UI1:
    case VT_UNKNOWN:
        // Currently topoedit does not allow editing of these
        // types, so they will not be serialized here.
        break;
    }

Cleanup:
    return hr;
}

HRESULT CTedAttributesSerializer::ReadGUID(ITedDataLoader* pLoader, CAtlString strTag, long nIndex, GUID* pgidToRead)
{
    HRESULT hr;
    LPWSTR szData = NULL;

    IFC( pLoader->LoadData(strTag, &szData, nIndex) );
    IFC( CLSIDFromString(W2OLE(szData), pgidToRead) );

Cleanup:
    CoTaskMemFree(szData);

    return hr;
}

HRESULT CTedAttributesSerializer::ReadPropVar(ITedDataLoader* pLoader, CAtlString strTag, long nIndex, PROPVARIANT& varToRead)
{
    HRESULT hr;
    LPWSTR szData = NULL;

    IFC( pLoader->LoadData(L"Type", &szData, nIndex) );
    varToRead.vt = (VARTYPE) _wtoi(szData);
    CoTaskMemFree(szData);
    szData = NULL;

    switch(varToRead.vt)
    {
    case VT_LPWSTR:
        IFC( pLoader->LoadData(strTag, &szData, nIndex) );
        varToRead.pwszVal = szData;
        szData = NULL;
        // String should be freed by PropVariantClear
        break;
    case VT_CLSID:
        varToRead.puuid = (CLSID*) CoTaskMemAlloc(sizeof(CLSID));
        IFC( ReadGUID(pLoader, strTag, nIndex, varToRead.puuid) );
        // GUID should be freed by PropVariantClear
        break;
    case VT_UI4:
        IFC( pLoader->LoadData(strTag, &szData, nIndex) );
        varToRead.ulVal = wcstoul(szData, NULL, 10);
        CoTaskMemFree(szData);
        szData = NULL;
        break;
    case VT_UI8:
        IFC( pLoader->LoadData(strTag, &szData, nIndex) );
        varToRead.uhVal.QuadPart = _wtoi64(szData);
        CoTaskMemFree(szData);
        szData = NULL;
        break;
    case VT_R8:
        IFC( pLoader->LoadData(strTag, &szData, nIndex) );
        varToRead.dblVal = wcstod(szData, NULL);
        CoTaskMemFree(szData);
        szData = NULL;
        break;
    case VT_UNKNOWN:
    case VT_VECTOR | VT_UI1:
        // TopoEdit does not currently handle loading of
        // IUnknown or blob types.
        break;
    }

Cleanup:
    CoTaskMemFree(szData);

    return hr;
}

////////////////////////////////////////////////////
//

CTedNodeMemo::CTedNodeMemo()
{
}

CTedNodeMemo::CTedNodeMemo(double x, double y, const CAtlStringW& strLabel, int nID) 
    : m_x(x), m_y(y), m_strLabel(strLabel), m_nID(nID)
{

}

CTedNodeMemo::~CTedNodeMemo()
{
    for(size_t i = 0; i < m_arrNodeAttributes.GetCount(); i++)
    {
        m_arrNodeAttributes.GetAt(i)->Release();
    }
}

HRESULT CTedNodeMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;

    CAtlStringW converter;
    
    converter.Format(L"%f", m_x);
    IFC( pSaver->SaveData(L"X", converter) );
    
    converter.Format(L"%f", m_y);
    IFC( pSaver->SaveData(L"Y", converter) );

    converter.Format(L"%d", m_nID);
    IFC( pSaver->SaveData(L"ID", converter) );

    IFC( pSaver->SaveData(L"Label", m_strLabel) );

    converter.Format(L"%d", (DWORD) m_arrNodeAttributes.GetCount());
    IFC( pSaver->SaveData(L"NodeAttributesCount", converter) );

    IFC( pSaver->BeginSaveChildObjects() );
    for(size_t i = 0; i < m_arrNodeAttributes.GetCount(); i++)
    {
        IFC( pSaver->BeginSaveObject(L"NodeAttributes") );
        CTedAttributesSerializer Serializer(m_arrNodeAttributes.GetAt(i));
        IFC( Serializer.Serialize(pSaver) );

    }
    IFC( pSaver->EndSaveChildObjects() );

Cleanup:

    return hr;
}

HRESULT CTedNodeMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;
    DWORD cNodeAttributes;
    LPWSTR szData = NULL;

    IFC( pLoader->LoadData(L"X", &szData, 0) );
    m_x = _wtof(szData);
    CoTaskMemFree(szData);

    IFC( pLoader->LoadData(L"Y", &szData, 0) );
    m_y = _wtof(szData);
    CoTaskMemFree(szData);

    IFC( pLoader->LoadData(L"ID", &szData, 0) );
    m_nID = _wtoi(szData);
    CoTaskMemFree(szData);

    IFC( pLoader->LoadData(L"Label", &szData, 0) );
    m_strLabel = szData;
    CoTaskMemFree(szData);
    
    IFC( pLoader->LoadData(L"NodeAttributesCount", &szData, 0) );
    cNodeAttributes =  wcstoul(szData, NULL, 10);
    CoTaskMemFree(szData);

    BOOL fHasChildObjects;
    IFC( pLoader->HasChildObjects(&fHasChildObjects) );

    if(fHasChildObjects)
    {
        IFC( pLoader->BeginLoadChildObjects() );
        for(size_t i = 0; i < cNodeAttributes; i++)
        {
            LPWSTR szNextObject = NULL;

            IMFAttributes* pAttributes;
            IFC( MFCreateAttributes(&pAttributes, 4) );

            IFC( pLoader->GetNextObject(&szNextObject) );
            CoTaskMemFree(szNextObject);

            CTedAttributesSerializer Serializer(pAttributes);
            Serializer.Deserialize(pLoader);

            m_arrNodeAttributes.Add(pAttributes);
        }
        IFC( pLoader->EndLoadChildObjects() );
    }

Cleanup:
    return hr;
}

void CTedNodeMemo::AddNodeAttributes(IMFAttributes* pNodeAttributes)
{
    m_arrNodeAttributes.Add(pNodeAttributes);
    pNodeAttributes->AddRef();
}

DWORD CTedNodeMemo::GetNodeAttributeCount()
{
    return (DWORD) m_arrNodeAttributes.GetCount();
}

IMFAttributes* CTedNodeMemo::GetNodeAttributes(DWORD nIndex)
{
    return m_arrNodeAttributes.GetAt(nIndex);
}

////////////////////////////////////////////////////
//

CTedSourceMemo::CTedSourceMemo()
{
}

CTedSourceMemo::CTedSourceMemo(double x, double y, const CAtlStringW& strLabel, int nID, const CAtlStringW& strSourceURL) 
    : CTedNodeMemo(x, y, strLabel, nID), m_strSourceURL(strSourceURL)
{
}

HRESULT CTedSourceMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;

    IFC(pSaver->BeginSaveObject(L"CTedSourceMemo"));
    
    IFC(CTedNodeMemo::Serialize(pSaver));

    IFC(pSaver->SaveData(L"URL", m_strSourceURL));


Cleanup:
    return hr;
}

HRESULT CTedSourceMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;

    IFC(CTedNodeMemo::Deserialize(pLoader));

    LPWSTR strData = NULL;
    IFC(pLoader->LoadData(L"URL", &strData, 0));
    m_strSourceURL = strData;
    CoTaskMemFree(strData);

Cleanup:
    return hr;
}

////////////////////////////////////////////////////
//

CTedOutputMemo::CTedOutputMemo()
{
}

CTedOutputMemo::CTedOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID)
    : CTedNodeMemo(x, y, strLabel, nID)
{
}

HRESULT CTedOutputMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;

    IFC(CTedNodeMemo::Serialize(pSaver));

Cleanup:
    return hr;
}

HRESULT CTedOutputMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;

    IFC(CTedNodeMemo::Deserialize(pLoader));

Cleanup:
    return hr;
}

////////////////////////////////////////////////////
//

CTedAudioOutputMemo::CTedAudioOutputMemo()
{
}

CTedAudioOutputMemo::CTedAudioOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID)
    : CTedOutputMemo(x, y, strLabel, nID)
{
}

HRESULT CTedAudioOutputMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;

    IFC(pSaver->BeginSaveObject(L"CTedAudioOutputMemo"));

    IFC(CTedOutputMemo::Serialize(pSaver));

Cleanup:
    return hr;
}

HRESULT CTedAudioOutputMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;

    IFC(CTedOutputMemo::Deserialize(pLoader));

Cleanup:
    return hr;
}

////////////////////////////////////////////////////
//

CTedVideoOutputMemo::CTedVideoOutputMemo() 
{
}

CTedVideoOutputMemo::CTedVideoOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID)
    : CTedOutputMemo(x, y, strLabel, nID)
{
}

HRESULT CTedVideoOutputMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;

    IFC(pSaver->BeginSaveObject(L"CTedVideoOutputMemo"));

    IFC(CTedOutputMemo::Serialize(pSaver));

Cleanup:
    return hr;
}

HRESULT CTedVideoOutputMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;

    IFC(CTedOutputMemo::Deserialize(pLoader));

Cleanup:
    return hr;
}

///////////////////////////////////////////////////////
//

CTedCustomOutputMemo::CTedCustomOutputMemo() 
{
}

CTedCustomOutputMemo::CTedCustomOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID, GUID gidCustomSinkID)
    : CTedOutputMemo(x, y, strLabel, nID)
    , m_gidCustomSinkID(gidCustomSinkID)
{
}

HRESULT CTedCustomOutputMemo::Serialize(ITedDataSaver* pSaver)
{
    HRESULT hr = S_OK;
    CAtlStringW converter;
    LPOLESTR guidStr = NULL;

    IFC( pSaver->BeginSaveObject(L"CTedCustomOutputMemo") );
    IFC( CTedOutputMemo::Serialize(pSaver) );

    IFC( StringFromCLSID(m_gidCustomSinkID, &guidStr) );

    converter.Format(L"%s", guidStr);
    IFC( pSaver->SaveData(L"CustomSinkID", converter) );

    CoTaskMemFree(guidStr);

Cleanup:
    return hr;
}

HRESULT CTedCustomOutputMemo::Deserialize(ITedDataLoader* pLoader)
{
    HRESULT hr = S_OK;
    LPWSTR converter = NULL;
    
    IFC( CTedOutputMemo::Deserialize(pLoader) );

    IFC( pLoader->LoadData(L"CustomSinkID", &converter, 0) );
    IFC(CLSIDFromString(W2OLE(converter), &m_gidCustomSinkID));
    CoTaskMemFree(converter);

Cleanup:
    return hr;
}

///////////////////////////////////////////////////////
//

CTedTransformMemo::CTedTransformMemo() 
{
}

CTedTransformMemo::CTedTransformMemo(double x, double y, const CAtlStringW& strLabel, int nID, const CLSID& clsid)
    : CTedNodeMemo(x, y, strLabel, nID), m_clsid(clsid)
{
}

HRESULT CTedTransformMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;

    CAtlStringW converter;
    LPOLESTR clsidStr = NULL;
    
    IFC(pSaver->BeginSaveObject(L"CTedTransformMemo"));
    IFC(CTedNodeMemo::Serialize(pSaver));

    IFC(StringFromCLSID(m_clsid, &clsidStr));

    converter.Format(L"%s", clsidStr);
    IFC(pSaver->SaveData(L"CLSID", converter));

    CoTaskMemFree(clsidStr);

 Cleanup:
    return hr;
}

HRESULT CTedTransformMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;

    LPWSTR converter;
    
    IFC(CTedNodeMemo::Deserialize(pLoader));

    IFC(pLoader->LoadData(L"CLSID", &converter, 0));
    IFC(CLSIDFromString(W2OLE(converter), &m_clsid));
    CoTaskMemFree(converter);
    
 Cleanup:
    return hr;
}

///////////////////////////////////////////////////////
//

CTedTeeMemo::CTedTeeMemo() 
{
}

CTedTeeMemo::CTedTeeMemo(double x, double y, const CAtlStringW& strLabel, int nID, int nNextOutputIndex)
    : CTedNodeMemo(x, y, strLabel, nID), m_nNextOutputIndex(nNextOutputIndex)
{

}

HRESULT CTedTeeMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;

    CAtlStringW converter;
    
    IFC(pSaver->BeginSaveObject(L"CTedTeeMemo"));
    IFC(CTedNodeMemo::Serialize(pSaver));

    converter.Format(L"%d", m_nNextOutputIndex);
    IFC(pSaver->SaveData(L"NextOutputIndex", converter));

 Cleanup:
    return hr;
}

HRESULT CTedTeeMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;

    LPWSTR converter;
    
    IFC(CTedNodeMemo::Deserialize(pLoader));

    IFC(pLoader->LoadData(L"NextOutputIndex", &converter, 0));
    m_nNextOutputIndex = _wtoi(converter);
    CoTaskMemFree(converter);
    
 Cleanup:
    return hr;
}    
///////////////////////////////////////////////////////
//

CTedConnectionMemo::CTedConnectionMemo()
{
}

CTedConnectionMemo::CTedConnectionMemo(int nOutputNodeID, int nOutputPinID, int nInputNodeID, int nInputPinID) 
    : m_nOutputNodeID(nOutputNodeID), m_nOutputPinID(nOutputPinID)
    , m_nInputNodeID(nInputNodeID), m_nInputPinID(nInputPinID)
{

}

HRESULT CTedConnectionMemo::Serialize(ITedDataSaver* pSaver) 
{
    HRESULT hr = S_OK;
    
    CAtlStringW converter;

    IFC(pSaver->BeginSaveObject(L"CTedConnectionMemo"));

    converter.Format(L"%d", m_nOutputNodeID);
    IFC(pSaver->SaveData(L"OutputNodeID", converter));

    converter.Format(L"%d", m_nOutputPinID);
    IFC(pSaver->SaveData(L"OutputPinID", converter));

    converter.Format(L"%d", m_nInputNodeID);
    IFC(pSaver->SaveData(L"InputNodeID", converter));

    converter.Format(L"%d", m_nInputPinID);
    IFC(pSaver->SaveData(L"InputPinID", converter));

Cleanup:
    return hr;
}

HRESULT CTedConnectionMemo::Deserialize(ITedDataLoader* pLoader) 
{
    HRESULT hr = S_OK;

    LPWSTR converter;

    IFC(pLoader->LoadData(L"OutputNodeID", &converter, 0));
    m_nOutputNodeID = _wtoi(converter);
    CoTaskMemFree(converter);

    IFC(pLoader->LoadData(L"OutputPinID", &converter, 0));
    m_nOutputPinID = _wtoi(converter);
    CoTaskMemFree(converter);

    IFC(pLoader->LoadData(L"InputNodeID", &converter, 0));
    m_nInputNodeID = _wtoi(converter);
    CoTaskMemFree(converter);

    IFC(pLoader->LoadData(L"InputPinID", &converter, 0));
    m_nInputPinID = _wtoi(converter);
    CoTaskMemFree(converter);

Cleanup:
    return hr;
}
