// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "tedutil.h"

class CCommandHandler;
class CTedNodeMemo;
class CTedSourceMemo;
class CTedOutputMemo;
class CTedAudioOutputMemo;
class CTedVideoOutputMemo;
class CTedCustomOutputMemo;
class CTedTransformMemo;
class CTedConnectionMemo;
class CTedTeeMemo;
class CMoveComponentHandler;
class CConnectPinHandler;
class CVisualPin;

class CTedSerializableObject
{
    virtual HRESULT Serialize(ITedDataSaver* pSaver) = 0;
    virtual HRESULT Deserialize(ITedDataLoader* pLoader) = 0;
};

class CTedAttributesSerializer : public CTedSerializableObject
{
public:
    CTedAttributesSerializer(IMFAttributes* pAttributes);
    ~CTedAttributesSerializer();

    virtual HRESULT Serialize(ITedDataSaver* pSaver);
    virtual HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    HRESULT WriteGUID(ITedDataSaver* pSaver, CAtlString strTag, GUID gidToWrite);
    HRESULT WritePropVar(ITedDataSaver* pSaver, CAtlString strTag, PROPVARIANT& varToWrite);

    HRESULT ReadGUID(ITedDataLoader* pLoader, CAtlString strTag, long nIndex, GUID* pgidToRead);
    HRESULT ReadPropVar(ITedDataLoader* pLoader, CAtlString strTag, long nIndex, PROPVARIANT& varToRead);
private:
    IMFAttributes* m_pAttributes;
};

class CTedNodeMemo : public CTedSerializableObject
{
    friend class CTedTopologyNode;
public:
    CTedNodeMemo();
    virtual ~CTedNodeMemo();

    virtual HRESULT Serialize(ITedDataSaver* pSaver);
    virtual HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    CTedNodeMemo(double x, double y, const CAtlStringW& strLabel, int nID);
    void AddNodeAttributes(IMFAttributes* pNodeAttributes);
    DWORD GetNodeAttributeCount();
    IMFAttributes* GetNodeAttributes(DWORD nIndex);

private:
    double m_x;
    double m_y;
    CAtlStringW m_strLabel;
    int m_nID;
    CAtlArray<IMFAttributes*> m_arrNodeAttributes;
};

class CTedSourceMemo : public CTedNodeMemo 
{
    friend class CTedSourceNode;
public:
    CTedSourceMemo();
    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
     CTedSourceMemo(double x, double y, const CAtlStringW& strLabel, int nID, const CAtlStringW& strSourceURL);
private:
    CAtlStringW m_strSourceURL;
};

class CTedOutputMemo : public CTedNodeMemo 
{
    friend class CTedOutputNode;
public:
    CTedOutputMemo();

    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    CTedOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID);
};

class CTedAudioOutputMemo : public CTedOutputMemo 
{
    friend class CTedAudioOutputNode;
public:
    CTedAudioOutputMemo();

    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    CTedAudioOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID);
private:
    
};

class CTedVideoOutputMemo : public CTedOutputMemo 
{
    friend class CTedVideoOutputNode;
public:
    CTedVideoOutputMemo();

    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    CTedVideoOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID);
private:
    
};

class CTedCustomOutputMemo : public CTedOutputMemo 
{
    friend class CTedCustomOutputNode;
public:
    CTedCustomOutputMemo();

    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    CTedCustomOutputMemo(double x, double y, const CAtlStringW& strLabel, int nID, GUID gidCustomSinkID);
private:
        GUID m_gidCustomSinkID;
};

class CTedTransformMemo : public CTedNodeMemo 
{
    friend class CTedTransformNode;
 public:
    CTedTransformMemo();

    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    CTedTransformMemo(double x, double y, const CAtlStringW& strLabel, int nID, const CLSID& clsid);
private:
    CLSID m_clsid;
};

class CTedTeeMemo : public CTedNodeMemo 
{
    friend class CTedTeeNode;
public:
    CTedTeeMemo();

    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);

protected:
    CTedTeeMemo(double x, double y, const CAtlStringW& strLabel, int nID, int nNextOutputIndex);
private:
    int m_nNextOutputIndex;
};

class CTedConnectionMemo 
{
    friend class CTedTopologyConnection;
public:
    CTedConnectionMemo();

    HRESULT Serialize(ITedDataSaver* pSaver);
    HRESULT Deserialize(ITedDataLoader* pLoader);
    
protected:
    CTedConnectionMemo(int nOutputNodeID, int nOutputPinID, int nInputNodeID, int nInputPinID);
private:
    int m_nOutputNodeID;
    int m_nOutputPinID;
    int m_nInputNodeID;
    int m_nInputPinID;
};