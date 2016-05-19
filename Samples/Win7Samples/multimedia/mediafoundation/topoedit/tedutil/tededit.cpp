// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tededit.h"
#include "tedvis.h"
#include "topoviewerwindow.h"
#include "tedmemo.h"
#include "dmoinfo.h"
#include "propertyview.h"

#include "mediaobj.h"
#include <dmo.h>
#include <evr.h>

#include <assert.h>

#include "Logger.h"

CLSID CLSID_AudioRenderActivate = { /* d23e6476-b104-4707-81cb-e1ca19a07016 */
    0xd23e6476,
    0xb104,
    0x4707,
    { 0x81, 0xcb, 0xe1, 0xca, 0x19, 0xa0, 0x70, 0x16 }
};

CLSID CLSID_VideoRenderActivate = { /* d23e6477-b104-4707-81cb-e1ca19a07016 */
    0xd23e6477,
    0xb104,
    0x4707,
    { 0x81, 0xcb, 0xe1, 0xca, 0x19, 0xa0, 0x70, 0x16 }
};

///////////////////////////////////////////////////////////////////////////////
// 
class CMoveComponentHandler : public CCommandHandler
{
public:
    CMoveComponentHandler(CVisualTree* pTree, ITedPropertyController* pController);
    
    BOOL OnLButtonDown(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnLButtonUp(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnMouseMove(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnLButtonDoubleClick(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnFocus(CVisualObject* pObj);

    void SetEditable(BOOL fEditable);

private:
    HRESULT ShowOTA(IMFTopologyNode* pNode);
    
    BOOL m_fCapture;
    BOOL m_fEditable;

    // offset from the left/top of component to mouse
    CVisualPoint m_Offset;
    
    CVisualTree * m_pTree;
    CComPtr<ITedPropertyController> m_spController;
};

///////////////////////////////////////////////////////////////////////////////
// 
class CConnectPinHandler : public CCommandHandler
{
public:
    CConnectPinHandler(CVisualTree* pTree, CTedTopologyEditor* pEditor, ITedPropertyController* pController);
    
    BOOL OnLButtonDown(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnLButtonUp(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnMouseMove(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnLButtonDoubleClick(CVisualObject* pObj, CVisualPoint& pt);
    BOOL OnFocus(CVisualObject* pObj);

    void SetEditable(BOOL fEditable);

protected:
    void RemoveOldConnectors(CVisualPin* pPin, CVisualPin* pOtherPin);
    void ShowMediaTypeProperties(IMFTopologyNode* pOutputNode, DWORD dwOutputPinIndex, IMFTopologyNode* pInputNode, DWORD dwInputPinIndex);
    CComPtr<IMFMediaType> GetNodeMFMediaType(IMFTopologyNode* pNode, DWORD dwPinIndex, bool fOutput);
    bool IsValidConnection(CVisualObject* pSourcePin, CVisualObject* pTargetPin);
    
private:
    BOOL m_fCapture;
    BOOL m_fEditable;

    CTedTopologyEditor * m_pEditor;
    CVisualTree * m_pTree;
    CComPtr<ITedPropertyController> m_spController;
    
    CVisualConnector * m_pNew;
    CVisualPin* m_pLastOverPin;
};

///////////////////////////////////////////////////////////////////////////////
//
CMoveComponentHandler::CMoveComponentHandler(CVisualTree* pTree, ITedPropertyController* pController)
    : m_fCapture(FALSE)
    , m_fEditable(TRUE)
    , m_pTree(pTree)
    , m_spController(pController)
{
}

BOOL CMoveComponentHandler::OnLButtonDown(CVisualObject* pObj, CVisualPoint& pt)
{
    m_Offset = pt;

    if(pObj->GetContainer())
    {
        m_Offset.Add(-pObj->GetContainer()->Rect().x(), -pObj->GetContainer()->Rect().y());
    }
    else
    {
        m_Offset.Add(-pObj->Rect().x(), -pObj->Rect().y());
    }
    
    m_fCapture = TRUE;
    
    return TRUE;
}

BOOL CMoveComponentHandler::OnLButtonUp(CVisualObject* pObj, CVisualPoint& pt)
{
    m_fCapture = FALSE;

    return FALSE;
}

BOOL CMoveComponentHandler::OnMouseMove(CVisualObject* pObj, CVisualPoint& pt)
{
    if(!m_fCapture)
    {
        return FALSE;
    }
    
    if(m_fEditable)
    {
        if(pObj->GetContainer())
        {
            pObj->GetContainer()->Move(pt.x() - m_Offset.x(), pt.y() - m_Offset.y());
        }
        else
        {
            pObj->Move(pt.x() - m_Offset.x(), pt.y() - m_Offset.y());
        }

        m_pTree->RouteAllConnectors();
    }

    return TRUE;
}

BOOL CMoveComponentHandler::OnLButtonDoubleClick(CVisualObject * pObj, CVisualPoint & pt)
{
    return TRUE;
}

BOOL CMoveComponentHandler::OnFocus(CVisualObject* pObj)
{
    HRESULT hr = S_OK;
    CTedTopologyNode* pNode = (CTedTopologyNode*) pObj->GetData();

    if(pNode->GetMFNodeCount() <= pObj->GetIndex()) return FALSE;
    
    if(m_spController.p) m_spController->ClearProperties();
        
    IMFTopologyNode* pMFNode = pNode->GetMFNode(pObj->GetIndex());

    MF_TOPOLOGY_TYPE nodeType;
    pMFNode->GetNodeType(&nodeType);
    
    if(m_spController.p)
    {
        CNodePropertyInfo* pNodePropertyInfo = new CNodePropertyInfo(pMFNode);
        m_spController->AddPropertyInfo(pNodePropertyInfo);

        // For source nodes, we also add presentation descriptor attributes
        if(nodeType == MF_TOPOLOGY_SOURCESTREAM_NODE)
        {
            CComPtr<IMFPresentationDescriptor> spPD;
            hr = pMFNode->GetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, IID_IMFPresentationDescriptor, (void**) &spPD);

            if(SUCCEEDED(hr))
            {
                CAttributesPropertyInfo* pPDInfo = new CAttributesPropertyInfo(spPD.p, LoadAtlString(IDS_PD_ATTRIBS), TED_ATTRIBUTE_CATEGORY_PRESENTATIONDESCRIPTOR);
                m_spController->AddPropertyInfo(pPDInfo);
            }
        }
    }

    // For transforms and sinks, we also add output trust authority information
    if(nodeType == MF_TOPOLOGY_TRANSFORM_NODE || nodeType == MF_TOPOLOGY_OUTPUT_NODE)
    {
        IFC( ShowOTA(pMFNode) );
    } 
Cleanup:
    return TRUE;
}

void CMoveComponentHandler::SetEditable(BOOL fEditable)
{
    m_fEditable = fEditable;
}

HRESULT CMoveComponentHandler::ShowOTA(IMFTopologyNode* pNode)
{
    HRESULT hr = S_OK;
    
    CComPtr<IUnknown> spUnkObj;
    CComPtr<IMFTrustedOutput> spTrustedOutput;
    DWORD dwOTACount;
    
    IFC( pNode->GetObject(&spUnkObj) );
    IFC( spUnkObj->QueryInterface(IID_IMFTrustedOutput, (void**) &spTrustedOutput) );
    IFC( spTrustedOutput->GetOutputTrustAuthorityCount(&dwOTACount) );

    CComPtr<IMFOutputTrustAuthority>* arrOTA = new CComPtr<IMFOutputTrustAuthority>[dwOTACount];
    CHECK_ALLOC(arrOTA);
    
    for(DWORD i =0; i < dwOTACount; i++)
    {
        CComPtr<IMFOutputTrustAuthority> spOTA;
        
        spTrustedOutput->GetOutputTrustAuthorityByIndex(i, &arrOTA[i]);
    }

    if(m_spController.p)
    {
        COTAPropertyInfo* pOTAPropertyInfo = new COTAPropertyInfo(arrOTA, dwOTACount);
        if(NULL == pOTAPropertyInfo) 
        {
            delete[] arrOTA;
            goto Cleanup;
        }

        m_spController->AddPropertyInfo(pOTAPropertyInfo);
    }

Cleanup:
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
CConnectPinHandler::CConnectPinHandler(CVisualTree * pTree, CTedTopologyEditor* pEditor, ITedPropertyController* pController)
    : m_pTree(pTree)
    , m_fEditable(TRUE)
    , m_pEditor(pEditor)
    , m_spController(pController)
    , m_pNew(NULL)
    , m_pLastOverPin(NULL)
{
    assert(m_pEditor != NULL);
    assert(m_pTree != NULL);
}

BOOL CConnectPinHandler::OnLButtonDown(CVisualObject * pObj, CVisualPoint & pt)
{
    if(m_fEditable)
    {
        // Begin a connection
        m_fCapture = TRUE;

        CVisualPin * pPin = (CVisualPin*)pObj;

        m_pNew = new CVisualConnector;

        m_pTree->AddVisual(m_pNew);
    
        m_pNew->Left() = pPin->GetConnectorPoint();
        m_pNew->Right() = pPin->GetConnectorPoint();
    }
    
    return TRUE;
}

BOOL CConnectPinHandler::OnLButtonUp(CVisualObject* pObj, CVisualPoint& pt)
{
    CVisualObject * pHitObj;
    
    assert(pObj != NULL);

    if(!m_fCapture || !m_fEditable)
    {
        return FALSE;
    }

    CVisualPin * pPin = (CVisualPin*)pObj;

    if(!m_pTree->HitTest(pt, &pHitObj))
    {
        goto Cleanup;
    }

    if(!IsValidConnection(pObj, pHitObj))
    {
        goto Cleanup;
    }

    CVisualPin * pOtherPin = (CVisualPin*)pHitObj;

    CVisualObject::CONNECTION_TYPE connType = pOtherPin->GetConnectionType();
    if(CVisualObject::INPUT == connType)
    {
        m_pNew->Right() = pOtherPin->GetConnectorPoint();
    }
    else
    {
        m_pNew->Left() = pOtherPin->GetConnectorPoint();

        // pPin needs to be the input pin
        CVisualPin* temp = pPin;
        pPin = pOtherPin;
        pOtherPin = temp;
    }
	
    assert(pPin != pOtherPin);
	
    CTedTopologyNode* outputterNode = (CTedTopologyNode*) (pPin->GetData());
    CTedTopologyNode* acceptorNode = (CTedTopologyNode*) (pOtherPin->GetData());

    m_pEditor->FullConnectNodes(outputterNode, pPin->GetPinId(), acceptorNode, pOtherPin->GetPinId());
    
Cleanup:

    if(NULL != m_pNew)
    {
        m_pTree->RemoveVisual(m_pNew);
        m_pNew = NULL;
    }

    if(m_pLastOverPin)
    {
        m_pLastOverPin->Highlight(false);
        m_pLastOverPin = NULL;
    }

    return FALSE;
}

BOOL CConnectPinHandler::OnMouseMove(CVisualObject * pObj, CVisualPoint & pt)
{
    if(!m_fCapture || !m_fEditable)
    {
        return FALSE;
    }

    assert(m_pNew);
    
    CVisualPin * pPin = (CVisualPin*)pObj;

    if(pPin->GetConnectionType() == CVisualObject::INPUT)
    {
        m_pNew->Left() = pt;
    }
    else
    {
        m_pNew->Right() = pt;
    }

    CVisualObject* pOverObject;

    if(m_pLastOverPin)
    {
        m_pLastOverPin->Highlight(false);
        m_pLastOverPin = NULL;
    }
    
    if(m_pTree->HitTest(pt, &pOverObject))
    {
        if(pOverObject->GetConnectionType() != CVisualObject::NONE && IsValidConnection(pPin, (CVisualPin*) pOverObject))
        {
            CVisualPin* pOverPin = (CVisualPin*) pOverObject;

            pOverPin->Highlight(true);

            m_pLastOverPin = pOverPin;
        }
    }
    
    return TRUE;
}

BOOL CConnectPinHandler::OnLButtonDoubleClick(CVisualObject* pObj, CVisualPoint& pt)
{
    return TRUE;
}

BOOL CConnectPinHandler::OnFocus(CVisualObject* pObj)
{
    CVisualObject::CONNECTION_TYPE connType;
    connType = pObj->GetConnectionType();

    if(m_spController.p) m_spController->ClearProperties();
    
    // If we have a connection, we want to display the media types for it
    if(connType != CVisualObject::NONE)
    {
        CVisualPin* pPin = (CVisualPin*) pObj;

        CTedTopologyNode* pNode = (CTedTopologyNode*) (pPin->GetData());

        // For source streams, we also want to display stream descriptor attributes
        if(pNode->GetType() == CTedTopologyNode::TED_SOURCE_NODE && m_spController.p)
        {
            IMFTopologyNode* pMFNode = pNode->GetMFNode(pPin->GetPinId());

            CComPtr<IMFStreamDescriptor> spSD;
            pMFNode->GetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, IID_IMFStreamDescriptor, (void**) &spSD);

            CComPtr<IMFAttributes> spAttr = spSD.p;
            
            CAttributesPropertyInfo* pAttrInfo = new CAttributesPropertyInfo(spAttr, LoadAtlString(IDS_SD_ATTRIBS), TED_ATTRIBUTE_CATEGORY_STREAMDESCRIPTOR);
            if(m_spController.p) m_spController->AddPropertyInfo(pAttrInfo);
        }
        
        if(pPin->GetConnector() == NULL)
        {
            return true;
        }
        
        CTedTopologyConnection* pConn = NULL;
        CTedTopologyNode* pOtherNode = NULL;

        if(connType == CVisualObject::INPUT)
        {
            pConn = m_pEditor->FindUpstreamConnection(pNode->GetID(), pPin->GetPinId());
            assert(pConn);
            pOtherNode = m_pEditor->FindNode(pConn->GetOutputNodeID());
            assert(pOtherNode);

            CTedTopologyPin outputPin;
            CTedTopologyPin inputPin;

            pNode->GetPin(pConn->GetInputPinID(), inputPin, true);
            pOtherNode->GetPin(pConn->GetOutputPinID(), outputPin, false);

            IMFTopologyNode* pMFInputNode = inputPin.PNode();
            IMFTopologyNode* pMFOutputNode = outputPin.PNode();

            ShowMediaTypeProperties(pMFOutputNode, outputPin.Index(), pMFInputNode, inputPin.Index());
        }
        else
        {
            pConn = m_pEditor->FindDownstreamConnection(pNode->GetID(), pPin->GetPinId());
            assert(pConn);
            pOtherNode = m_pEditor->FindNode(pConn->GetInputNodeID());
            assert(pOtherNode);

            CTedTopologyPin outputPin;
            CTedTopologyPin inputPin;

            pOtherNode->GetPin(pConn->GetInputPinID(), inputPin, true);
            pNode->GetPin(pConn->GetOutputPinID(), outputPin, false);

            IMFTopologyNode* pMFInputNode = inputPin.PNode();
            IMFTopologyNode* pMFOutputNode = outputPin.PNode();

            ShowMediaTypeProperties(pMFOutputNode, outputPin.Index(), pMFInputNode, inputPin.Index());
        }
    }

    return TRUE;
}

void CConnectPinHandler::ShowMediaTypeProperties(IMFTopologyNode* pOutputNode, DWORD dwOutputPinIndex, IMFTopologyNode* pInputNode, DWORD dwInputPinIndex)
{
    if(m_spController)
    {
        HRESULT hr;
        CComPtr<IMFMediaType> spOutputType = GetNodeMFMediaType(pOutputNode, dwOutputPinIndex, true);
        CComPtr<IMFMediaType> spInputType = GetNodeMFMediaType(pInputNode, dwInputPinIndex, false);
    
        if(spOutputType.p)
        {
            CComPtr<IMFAttributes> spOutAttr;
            hr = spOutputType->QueryInterface(IID_IMFAttributes, (void**) &spOutAttr);
            if(SUCCEEDED(hr))
            {
                CAttributesPropertyInfo* pOutInfo = new CAttributesPropertyInfo(spOutAttr, LoadAtlString(IDS_MT_UPSTREAM), TED_ATTRIBUTE_CATEGORY_MEDIATYPE);
                if(pOutInfo) m_spController->AddPropertyInfo(pOutInfo);
            }
        }
        
        if(spInputType.p)
        {
            CComPtr<IMFAttributes> spInAttr;
            hr = spInputType->QueryInterface(IID_IMFAttributes, (void**) &spInAttr);
            if(SUCCEEDED(hr))
            {
                CAttributesPropertyInfo* pInInfo = new CAttributesPropertyInfo(spInAttr, LoadAtlString(IDS_MT_DOWNSTREAM), TED_ATTRIBUTE_CATEGORY_MEDIATYPE);
                if(pInInfo) m_spController->AddPropertyInfo(pInInfo);
            }
        }
    }
}

CComPtr<IMFMediaType> CConnectPinHandler::GetNodeMFMediaType(IMFTopologyNode* pNode, DWORD dwPinIndex, bool fOutput)
{
    MF_TOPOLOGY_TYPE nodeType;
    CComPtr<IMFMediaType> spType;
        
    pNode->GetNodeType(&nodeType);

    if(nodeType == MF_TOPOLOGY_SOURCESTREAM_NODE)
    {
        CComPtr<IMFStreamDescriptor> spSD;
        CComPtr<IMFMediaTypeHandler> spMTH;
        
        pNode->GetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, IID_IMFStreamDescriptor, (void**) &spSD);
        spSD->GetMediaTypeHandler(&spMTH);

        spMTH->GetCurrentMediaType(&spType);
    }
    else if(nodeType == MF_TOPOLOGY_TRANSFORM_NODE)
    {
        CComPtr<IUnknown> spTransformUnk;
        CComPtr<IMFTransform> spTransform;

        pNode->GetObject(&spTransformUnk);
        spTransformUnk->QueryInterface(IID_IMFTransform, (void**) &spTransform);

        if(fOutput)
        {
            spTransform->GetOutputCurrentType(dwPinIndex, &spType);
        }
        else
        {
            spTransform->GetInputCurrentType(dwPinIndex, &spType);
        }
    }
    else if(nodeType == MF_TOPOLOGY_OUTPUT_NODE)
    {
        HRESULT hr;
        CComPtr<IUnknown> spUnknown;
        CComPtr<IMFStreamSink> spStreamSink;

        pNode->GetObject(&spUnknown);

        hr = spUnknown->QueryInterface(IID_IMFStreamSink, (void**) &spStreamSink);

        if(SUCCEEDED(hr))
        {
            CComPtr<IMFMediaTypeHandler> spMTH;

            spStreamSink->GetMediaTypeHandler(&spMTH);

            spMTH->GetCurrentMediaType(&spType);
        }
        
        if(NULL == spType.p)
        {
            pNode->GetInputPrefType(0, &spType);
        }
    }

    return spType;
}
 
bool CConnectPinHandler::IsValidConnection(CVisualObject* pSource, CVisualObject* pTarget)
{
    CVisualObject::CONNECTION_TYPE connType = pSource->GetConnectionType();
    CVisualObject::CONNECTION_TYPE targetConnType = pTarget->GetConnectionType();
    
    if( CVisualObject::NONE == connType /* don't process objects that cannot be connected */
        || CVisualObject::NONE == targetConnType /* ditto */
        ||targetConnType == connType /* Don't connect two inputs or two outputs together */
        || pSource == pTarget /* Don't connect an object to itself */
        )
        
    {
        return false;
    }

    return true;
}

void CConnectPinHandler::SetEditable(BOOL fEditable)
{
    m_fEditable = fEditable;
}

///////////////////////////////////////////////////////////////////////////////
//
CTedTopologyPin::CTedTopologyPin()
    : m_pNode(NULL)
    , m_nIndex(0)
{
}

CTedTopologyPin::CTedTopologyPin(IMFTopologyNode * pNode, DWORD nIndex)
    : m_pNode(pNode)
    , m_nIndex(nIndex)
{
}

CTedTopologyPin::~CTedTopologyPin()
{
}

///////////////////////////////////////////////////////////////////////////////
//
CTedTopologyConnection::CTedTopologyConnection(int nOutputNodeID, int nOutputPinID, int nInputNodeID, int nInputPinID)
    : m_nOutputNodeID(nOutputNodeID), m_nOutputPinID(nOutputPinID), m_nInputNodeID(nInputNodeID), m_nInputPinID(nInputPinID)
{
    assert(m_nOutputNodeID >= 0);
    assert(m_nOutputPinID >= 0);
    assert(m_nInputNodeID >= 0);
    assert(m_nInputPinID >= 0);
}

CTedTopologyConnection::CTedTopologyConnection(CTedConnectionMemo* pMemo)
    : m_nOutputNodeID(pMemo->m_nOutputNodeID), m_nOutputPinID(pMemo->m_nOutputPinID)
    , m_nInputNodeID(pMemo->m_nInputNodeID), m_nInputPinID(pMemo->m_nInputPinID)
{
}

int CTedTopologyConnection::GetOutputNodeID() const
{
    return m_nOutputNodeID;
}

int CTedTopologyConnection::GetInputNodeID() const
{
    return m_nInputNodeID;
}

int CTedTopologyConnection::GetOutputPinID() const
{
    return m_nOutputPinID;
}

int CTedTopologyConnection::GetInputPinID() const
{
    return m_nInputPinID;
}

CTedConnectionMemo* CTedTopologyConnection::CreateMemo() const
{
    return new CTedConnectionMemo(m_nOutputNodeID, m_nOutputPinID, m_nInputNodeID, m_nInputPinID);
}

///////////////////////////////////////////////////////////////////////////////
// CTedTopologyNode

// Initialization //

int CTedTopologyNode::ms_nNextID = 0;

CTedTopologyNode::CTedTopologyNode()
{
    m_nID = ms_nNextID++;
}

CTedTopologyNode::~CTedTopologyNode()
{
}

HRESULT CTedTopologyNode::Init(const CAtlStringW& label, bool fAutoInserted)
{
    HRESULT hr = S_OK;
    
    m_pVisual = new CVisualNode(label, fAutoInserted); 
    CHECK_ALLOC( m_pVisual );
    
    m_pVisual->Move(20, 20);
    m_pVisual->SetData((LONG_PTR) this);

    m_strLabel = label;
    
Cleanup:
    return hr;
}

HRESULT CTedTopologyNode::InitContainer(const CAtlStringW& label, bool fAutoinserted)
{
    HRESULT hr = S_OK;
    
    m_pVisual = new CVisualContainer(label);
    CHECK_ALLOC( m_pVisual );
    
    m_pVisual->Move(20, 20);
    m_pVisual->SetData((LONG_PTR) this);

    m_strLabel = label;
    
Cleanup:
    return hr;
}

void CTedTopologyNode::Init(CTedNodeMemo* pMemo)
{
    Init(pMemo->m_strLabel);

    m_nID = pMemo->m_nID;

    if(ms_nNextID <= m_nID) 
    {
        ms_nNextID = m_nID + 1;
    }

    m_pVisual->Move(pMemo->m_x, pMemo->m_y);
}

void CTedTopologyNode::InitContainer(CTedNodeMemo* pMemo)
{
    InitContainer(pMemo->m_strLabel);

    m_nID = pMemo->m_nID;

    if(ms_nNextID <= m_nID) 
    {
        ms_nNextID = m_nID + 1;
    }

    m_pVisual->Move(pMemo->m_x, pMemo->m_y);
}
    
// Accessors //

HWND CTedTopologyNode::GetVideoWindow() const
{
    return NULL;
}

CVisualComponent* CTedTopologyNode::GetVisual() const
{
    return m_pVisual;
}

CVisualPin* CTedTopologyNode::GetVisualInputPin(int pinID)
{
    return ((CVisualComponent*) m_pVisual)->GetInputPin(pinID);
}

CVisualPin* CTedTopologyNode::GetVisualOutputPin(int pinID)
{
    return ((CVisualComponent*) m_pVisual)->GetOutputPin(pinID);
}

int CTedTopologyNode::GetID() const
{
    return m_nID;
}

HRESULT CTedTopologyNode::GetNodeID(DWORD dwIndex, TOPOID& NodeID)
{
    HRESULT hr = S_OK;

    CComPtr<IMFTopologyNode> spNode = GetMFNode(dwIndex);
    
    IFC( spNode->GetTopoNodeID(&NodeID) );

Cleanup:
    return hr;
}

CAtlStringW CTedTopologyNode::GetLabel() const
{
    return m_strLabel;
}

bool CTedTopologyNode::IsOrphaned()
{
    bool fHasInputConnections = false;
    bool fHasOutputConnections = false;
    
    for(DWORD i = 0; i < GetMFNodeCount(); i++)
    {
        IMFTopologyNode* pNode = GetMFNode(i);
        
        DWORD cInputs = 0;
        pNode->GetInputCount(&cInputs);
        for(DWORD j = 0; j < cInputs; j++)
        {
            CComPtr<IMFTopologyNode> spUpNode;
            DWORD dwUpIndex;
            if(SUCCEEDED( pNode->GetInput(j, &spUpNode, &dwUpIndex) ))
            {
                fHasInputConnections = true;
                break;
            }
        }
        
        if(!fHasInputConnections) break;
        
        DWORD cOutputs = 0;
        pNode->GetOutputCount(&cOutputs);
        for(DWORD j = 0; j < cOutputs; j++)
        {
            CComPtr<IMFTopologyNode> spDownNode;
            DWORD dwDownIndex;
            if(SUCCEEDED( pNode->GetOutput(j, &spDownNode, &dwDownIndex) ))
            {
                fHasOutputConnections = true;
                break;
            }
        }
    }
    
    return (!fHasInputConnections || !fHasOutputConnections);
}

// Mutators //
HRESULT CTedTopologyNode::CopyAttributes(IMFTopologyNode* pNode, DWORD dwIndex)
{
    HRESULT hr = S_OK;
    IMFTopologyNode* pTargetNode = GetMFNode(dwIndex);
    CComPtr<IMFMediaType> spInputPrefType;

    IFC( pNode->CopyAllItems(pTargetNode) );

    m_fErrorNode = false;

    if( SUCCEEDED(pNode->GetItem(MF_TOPONODE_ERRORCODE, NULL)) )
    {
        m_fErrorNode = true;

        TOPOID tidNode;
        IFC( pNode->GetTopoNodeID(&tidNode) );

        for(DWORD j = 0; j < GetMFNodeCount(); ++j)
        {
            IMFTopologyNode* pTedNode = GetMFNode(j);
                 
            TOPOID tidTedNode;
            IFC( pTedNode->GetTopoNodeID(&tidTedNode) );

            if(tidNode == tidTedNode)
            {
                m_pVisual->FlagTopoLoadError(j, true);
                break;
            }
        } 
    }

    if(!m_fErrorNode)
    {
        for(DWORD i = 0; i < GetMFNodeCount(); i++)
        {
            m_pVisual->FlagTopoLoadError(i, false);
        }
    }
    
    DWORD cInputs;
    IFC( pNode->GetInputCount(&cInputs) );
    for(DWORD i = 0; i < cInputs; i++)
    {
        CComPtr<IMFMediaType> spPrefType;
        if(SUCCEEDED(pNode->GetInputPrefType(i, &spPrefType)))
        {
            pTargetNode->SetInputPrefType(i, spPrefType);
        }
    }
    
    DWORD cOutputs;
    IFC( pNode->GetOutputCount(&cOutputs) );
    for(DWORD i = 0; i < cOutputs; i++)
    {
        CComPtr<IMFMediaType> spPrefType;
        if(SUCCEEDED(pNode->GetOutputPrefType(i, &spPrefType)))
        {
            pTargetNode->SetOutputPrefType(i, spPrefType);
        }
    }
    
Cleanup:
    return hr;
}

// Non-public Helpers //
void CTedTopologyNode::AddPin(bool bIsInput, const CAtlStringW& strLabel, int nID)
{
    assert(m_pVisual->Type() == CVisualObject::NODE);
    ((CVisualNode*) m_pVisual)->AddPin(bIsInput, (LONG_PTR) const_cast<CTedTopologyNode*>(this), strLabel, nID);
}

HRESULT CTedTopologyNode::PostInitFromMemoCopyAttributes(CTedNodeMemo* pMemo)
{
    HRESULT hr = S_OK;
    for(DWORD i = 0; i < pMemo->GetNodeAttributeCount() && i < GetMFNodeCount(); i++)
    {
        IMFAttributes* pAttributes = pMemo->GetNodeAttributes(i);
        
        UINT32 cItems;
        IFC( pAttributes->GetCount(&cItems) );

        for(UINT32 j = 0; j < cItems; j++)
        {
            GUID gidKey;
            PROPVARIANT var;
            PropVariantInit(&var);

            IFC( pAttributes->GetItemByIndex(j, &gidKey, &var) );
            IFC( GetMFNode(i)->SetItem(gidKey, var) );

            PropVariantClear(&var);
        }
    }

Cleanup:
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// CTedSourceNode

// Initializers //

CComPtr<IMFSourceResolver> CTedSourceNode::ms_spResolver = NULL;
bool CTedSourceNode::m_bIsResolverCreated = false;

CTedSourceNode::CTedSourceNode()
    : m_fIsProtected(false)
    , m_fExternalShutdownRequired(false)
    , m_bInitializedFromMFSource(false)
{
}

CTedSourceNode::~CTedSourceNode()
{
    m_Nodes.RemoveAll();
}

HRESULT CTedSourceNode::Init(const CAtlStringW& strSourceURL, const CAtlStringW& label)
{
    CTedTopologyNode::InitContainer(label);

    HRESULT hr = S_OK;

    m_strSourceURL = strSourceURL;

    IFC( CreateMFSource() );
    IFC( CreateSourceNodes() );

Cleanup:
    return hr;
}

HRESULT CTedSourceNode::Init(IMFMediaSource* pSource, const CAtlStringW& label)
{
    CTedTopologyNode::InitContainer(label);

    HRESULT hr = S_OK;

    m_strSourceURL = label;

    IFC( CreateMFCaptureSource( pSource ) );
    IFC( CreateSourceNodes() );

    m_bInitializedFromMFSource = true;

Cleanup:
    return hr;
}

HRESULT CTedSourceNode::Init(CTedSourceMemo* pMemo)
{
    HRESULT hr = S_OK;

    CTedTopologyNode::InitContainer(pMemo);

    m_strSourceURL = pMemo->m_strSourceURL;

    IFC( CreateMFSource() );
    IFC( CreateSourceNodes() );

    IFC( PostInitFromMemoCopyAttributes(pMemo) );

Cleanup:
    return hr;
}

HRESULT CTedSourceNode::Init(CTedSourceNode* pNode)
{
    HRESULT hr = S_OK;

    CTedTopologyNode::InitContainer(pNode->GetLabel());
    
    m_spSource = pNode->m_spSource;
    m_spPD = pNode->m_spPD;
    m_strSourceURL = pNode->m_strSourceURL;
    m_fExternalShutdownRequired = pNode->m_fExternalShutdownRequired;

    for(DWORD i = 0; i < pNode->GetMFNodeCount(); i++)
    {
        CComPtr<IMFTopologyNode> spNode;
        
        IFC( MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &spNode) );
        IFC( spNode->CloneFrom(pNode->GetMFNode(i)) );
        IFC( InitMFSourceNode(spNode, i) );
    }
    
Cleanup:
    return hr;
}

HRESULT CTedSourceNode::Init(const CAtlStringW& strSourceURL, const CAtlStringW& label, IMFMediaSource* pSource)
{
    HRESULT hr = S_OK;

    CTedTopologyNode::InitContainer(label);

    m_strSourceURL = strSourceURL;
    m_spSource = pSource;
    IFC( m_spSource->CreatePresentationDescriptor(&m_spPD) );
    
    IFC( CreateSourceNodes() );

Cleanup:
    return hr;
}

HRESULT CTedSourceNode::InitIndirect(const CAtlStringW& strSourceURL, CAtlArray<IMFTopologyNode*>& sourceNodes)
{
    HRESULT hr = S_OK;
    
    m_strSourceURL = strSourceURL;

    CTedTopologyNode::InitContainer(m_strSourceURL);

    for(DWORD i = 0; i < sourceNodes.GetCount(); i++)
    {
        CComPtr<IMFTopologyNode> spNode;
        IFC( MFCreateTopologyNode( MF_TOPOLOGY_SOURCESTREAM_NODE,
                                            &spNode ));
        
        spNode->CloneFrom(sourceNodes.GetAt(i));

        IFC(InitMFSourceNode(spNode, i));
    }

Cleanup:
    return hr;
}

// Accessors //

HRESULT CTedSourceNode::GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin)
{
    HRESULT hr = S_OK;

    assert(!inputPin);
    assert(nInput < m_Nodes.GetCount());

    assert(m_Nodes.GetAt(nInput));
    Pin.SetPNode(m_Nodes.GetAt(nInput));
    Pin.SetIndex(0);
    
    return hr;
}

bool CTedSourceNode::IsProtected() const 
{
    return m_fIsProtected;
}

CTedNodeMemo* CTedSourceNode::CreateMemo() const
{
    CTedSourceMemo* pMemo = new CTedSourceMemo(GetVisual()->Rect().x(), GetVisual()->Rect().y(), GetLabel(), GetID(), m_strSourceURL);

    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        pMemo->AddNodeAttributes(m_Nodes.GetAt(i));
    }

    return pMemo;
}

DWORD CTedSourceNode::GetMFNodeCount() const
{
    return (DWORD) m_Nodes.GetCount();
}

IMFTopologyNode* CTedSourceNode::GetMFNode(DWORD nIndex) const
{
    return m_Nodes.GetAt(nIndex);
}

CTedTopologyNode::TED_NODE_TYPE CTedSourceNode::GetType()
{
    return TED_SOURCE_NODE;
}

DWORD CTedSourceNode::GetIndexOf(TOPOID nodeID)
{
    for(DWORD i = 0; i < m_Nodes.GetCount(); i++)
    {
        TOPOID curNodeID;
        m_Nodes.GetAt(i)->GetTopoNodeID(&curNodeID);

        if(nodeID == curNodeID) return i;
    }

    return (DWORD) -1;
}

CVisualPin* CTedSourceNode::GetVisualInputPin(int pinID)
{
    return GetVisual()->GetInputPin(pinID);
}

CVisualPin* CTedSourceNode::GetVisualOutputPin(int pinID)
{
    return GetVisual()->GetOutputPin(pinID);
}

// Mutators //

void CTedSourceNode::ReleaseResolver()
{
    ms_spResolver.Release();
    m_bIsResolverCreated = false;
}

void CTedSourceNode::ShutdownMFSource()
{
    if(!m_fExternalShutdownRequired)
    {
        m_spSource->Shutdown();
    }
}

HRESULT CTedSourceNode::CopyAttributes(IMFTopologyNode* pNode, DWORD dwIndex)
{
    HRESULT hr;
    BOOL fSelected;
    CComPtr<IMFStreamDescriptor> spSD;
    
    IFC( CTedTopologyNode::CopyAttributes(pNode, dwIndex) );
    
    IMFTopologyNode* pOurNode = GetMFNode(dwIndex);
    
    // Do not replace source, presentation descriptor, or stream descriptor
    IFC( pOurNode->SetUnknown(MF_TOPONODE_SOURCE, m_spSource) );
    IFC( pOurNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, m_spPD) );
    IFC( m_spPD->GetStreamDescriptorByIndex(dwIndex, &fSelected, &spSD) );
    IFC( pOurNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, spSD) );
    
Cleanup:
    return hr;
}

void CTedSourceNode::FlagExternalShutdownRequired()
{
    m_fExternalShutdownRequired = true;
}

HRESULT CTedSourceNode::SelectValidStreams()
{
    HRESULT hr = S_OK;
    
    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        IMFTopologyNode* pNode = m_Nodes.GetAt(i);
        CComPtr<IMFTopologyNode> spDownNode;
        DWORD dwDownIndex;
        
        HRESULT hrOutput = pNode->GetOutput(0, &spDownNode, &dwDownIndex);
        if(SUCCEEDED(hrOutput))
        {
            IFC( m_spPD->SelectStream((DWORD) i) );
        }
        else
        {
            IFC( m_spPD->DeselectStream((DWORD) i) );
        }
    }
    
Cleanup:
    return hr;
}

// Non-public Helpers //

HRESULT CTedSourceNode::CreateMFSource()
{
    HRESULT hr = S_OK;
    
    MF_OBJECT_TYPE ObjectType;

    if(!m_bIsResolverCreated)
    {
        IFC( MFCreateSourceResolver( &ms_spResolver ) );
        m_bIsResolverCreated = true;
    }
    
    IFC( ms_spResolver->CreateObjectFromURL(m_strSourceURL, 
                                           MF_RESOLUTION_MEDIASOURCE,
                                           NULL,
                                           &ObjectType,
                                           (IUnknown**)&m_spSource) );

    IFC( m_spSource->CreatePresentationDescriptor(&m_spPD) );

Cleanup:
    return hr;
}

HRESULT CTedSourceNode::CreateMFCaptureSource( IMFMediaSource* pSource )
{
    HRESULT hr = S_OK;

    m_spSource = pSource;
    IFC( m_spSource->CreatePresentationDescriptor(&m_spPD) );

Cleanup:
    return hr;
}

HRESULT CTedSourceNode::InitMFSourceNode(CComPtr<IMFTopologyNode> spNode, int nPinIndex)
{
    HRESULT hr = S_OK;

    CComPtr<IMFStreamDescriptor> spSD;
    CComPtr<IMFMediaTypeHandler> spMediaTypeHandler;
    BOOL fSelected = FALSE;
    GUID guidMajorType = GUID_NULL;
    CAtlString strPinLabel;

    IFC( spNode->GetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, IID_IMFStreamDescriptor, (void**) &spSD) );

    IFC( spSD->GetMediaTypeHandler( &spMediaTypeHandler ) );
    IFC( spMediaTypeHandler->GetMajorType( &guidMajorType ) );

    if(MFMediaType_Audio == guidMajorType) 
    {
        strPinLabel = LoadAtlString(IDS_AUDIO); 
    }
    else if(MFMediaType_Video == guidMajorType)
    {
        strPinLabel = LoadAtlString(IDS_VIDEO); 
    }

    InitIsProtected();

    CVisualNode* pVisualNode = new CVisualNode(strPinLabel, false);
    CHECK_ALLOC( pVisualNode );
    
    pVisualNode->SetData((LONG_PTR) this);
    pVisualNode->SetIndex(nPinIndex);

    pVisualNode->AddPin(false, (LONG_PTR) const_cast<CTedSourceNode*>(this), L"", nPinIndex);

    ((CVisualContainer*) GetVisual())->AddComponent(pVisualNode);

    
    assert(spNode != NULL);
    
    m_Nodes.Add(spNode);
        
Cleanup:
    return hr;
}

HRESULT CTedSourceNode::CreateSourceNodes() 
{
    HRESULT hr = S_OK;

    //
    // For each stream, create a source topology node 
    //
    DWORD cSourceStreams = 0;
    IFC( m_spPD->GetStreamDescriptorCount( &cSourceStreams ));

    DWORD i = 0;
    for ( i = 0; i < cSourceStreams; i++ )
    {
        CComPtr<IMFStreamDescriptor> spSD;
        CComPtr<IMFTopologyNode> spNode;
        BOOL fSelected = FALSE;

        IFC(m_spPD->GetStreamDescriptorByIndex(
                                    i,
                                    &fSelected,
                                    &spSD  ));

        IFC( MFCreateTopologyNode( MF_TOPOLOGY_SOURCESTREAM_NODE,
                                            &spNode ));

        IFC( spNode->SetUnknown(MF_TOPONODE_SOURCE, m_spSource) );
        IFC( spNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, m_spPD) );
        IFC( spNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, spSD) );
    
        IFC( InitMFSourceNode(spNode, i) );
    }

Cleanup:
    return hr;
}

HRESULT CTedSourceNode::InitIsProtected()
{
    HRESULT hr = S_OK;
    
    hr = MFRequireProtectedEnvironment(m_spPD);

    if(S_FALSE == hr)
    {
         m_fIsProtected = false;
    }
    else
    {
        m_fIsProtected = true;
    }
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// CTedOutputNode

// Initialization //

CTedOutputNode::CTedOutputNode()
    : m_fExternalShutdownRequired(false)
{
}

CTedOutputNode::~CTedOutputNode()
{
    m_arrStreamSinkNodes.RemoveAll();
}

HRESULT CTedOutputNode::Init(IMFActivate* pActivate, const CAtlStringW& label)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopologyNode> spNode;

    assert(pActivate != NULL);
    
    IFC( MFCreateTopologyNode( MF_TOPOLOGY_OUTPUT_NODE, &spNode ));
    IFC(spNode->SetObject(pActivate));
        
    m_arrStreamSinkNodes.Add(spNode);

    CTedTopologyNode::Init(label);
    AddPin(true, L"", 0);
Cleanup:

    return hr;
}

HRESULT CTedOutputNode::Init(IMFMediaSink* pSink, const CAtlStringW& label)
{
    assert(pSink != NULL);

    CTedTopologyNode::InitContainer(label);

    return InitFromSink(pSink);
}

HRESULT CTedOutputNode::Init(IMFActivate* pActivate, CTedOutputMemo* pMemo)
{
    HRESULT hr = S_OK;

    assert(pActivate != NULL);
    
    CTedTopologyNode::Init(pMemo);

    CComPtr<IMFTopologyNode> spNode;

    IFC( MFCreateTopologyNode( MF_TOPOLOGY_OUTPUT_NODE, &spNode ) );
    IFC( spNode->SetObject(pActivate) );

    m_arrStreamSinkNodes.Add(spNode);

    AddPin(true, L"", 0);

    IFC( PostInitFromMemoCopyAttributes(pMemo) );

Cleanup:
    return hr;
}

HRESULT CTedOutputNode::Init(IMFMediaSink* pSink, CTedOutputMemo* pMemo)
{
    assert (pSink != NULL);
    assert(pMemo != NULL);

    CTedTopologyNode::InitContainer(pMemo);

    HRESULT hr = InitFromSink(pSink);

    if(SUCCEEDED(hr))
    {
        hr = PostInitFromMemoCopyAttributes(pMemo);
    }

    return hr;
}

HRESULT CTedOutputNode::Init(IMFStreamSink* pStreamSink, const CAtlStringW& label)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopologyNode> spNode;

    assert(pStreamSink != NULL);
    
    IFC( MFCreateTopologyNode( MF_TOPOLOGY_OUTPUT_NODE, &spNode ) );
    IFC( spNode->SetObject(pStreamSink) );

    m_arrStreamSinkNodes.Add(spNode);

    CTedTopologyNode::Init(label);
    AddPin(true, L"", 0);
Cleanup:

    return hr;
}

// Accessors //

HRESULT CTedOutputNode::GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin)
{
    HRESULT hr = S_OK;

    assert(inputPin);
    assert(nInput < m_arrStreamSinkNodes.GetCount());
    
    Pin.SetPNode(m_arrStreamSinkNodes.GetAt(nInput));
    Pin.SetIndex(0);

    return hr;
}

DWORD CTedOutputNode::GetMFNodeCount() const
{
    return (DWORD) m_arrStreamSinkNodes.GetCount();
}

IMFTopologyNode* CTedOutputNode::GetMFNode(DWORD nIndex) const
{
    assert(nIndex < m_arrStreamSinkNodes.GetCount());

    return m_arrStreamSinkNodes.GetAt(nIndex);
}

CTedTopologyNode::TED_NODE_TYPE CTedOutputNode::GetType()
{
    return TED_OUTPUT_NODE;
}

CVisualPin* CTedOutputNode::GetVisualInputPin(int pinID)
{
    return GetVisual()->GetInputPin(pinID);
}

CVisualPin* CTedOutputNode::GetVisualOutputPin(int pinID)
{
    return GetVisual()->GetOutputPin(pinID);
}

// Mutators //

HRESULT CTedOutputNode::WrapStreamSink(CLogger* pLogger)
{
    HRESULT hr = S_OK;

    CComPtr<IUnknown> spActivateUnk;
    CComPtr<IMFActivate> spActivate;
    CComPtr<IMFMediaSink> spSink;
    CComPtr<IMFStreamSink> spStreamSink;
    CMFStreamSinkWrapper* pWrapper = NULL;

    IFC( m_arrStreamSinkNodes.GetAt(0)->GetObject(&spActivateUnk) );
    hr = spActivateUnk->QueryInterface(IID_IMFActivate, (void**) &spActivate);

    if(SUCCEEDED(hr))
    {
        IFC( spActivate->ActivateObject(IID_IMFMediaSink, (void**) &spSink) );
        IFC( spSink->GetStreamSinkById(0, &spStreamSink) );
    }
    else
    {
        IFC( spActivateUnk->QueryInterface(IID_IMFStreamSink, (void**) &spStreamSink) );
    }
    
    pWrapper = new CMFStreamSinkWrapper(spStreamSink, pLogger);
    CHECK_ALLOC( pWrapper );
    pWrapper->AddRef();

    IFC( m_arrStreamSinkNodes.GetAt(0)->SetObject(pWrapper) );

Cleanup:
    if(pWrapper) pWrapper->Release();
    return hr;
}

void CTedOutputNode::ShutdownMFSink()
{
    if(!m_fExternalShutdownRequired)
    {
        if(m_arrStreamSinkNodes.GetCount() > 0)
        {
            IMFTopologyNode* pNode = m_arrStreamSinkNodes.GetAt(0);
            CComPtr<IUnknown> spStreamSinkUnk;
            CComPtr<IMFStreamSink> spStreamSink;
            
            HRESULT hr = pNode->GetObject(&spStreamSinkUnk);
            
            if(SUCCEEDED(hr))
            {
                HRESULT hr = spStreamSinkUnk->QueryInterface(IID_IMFStreamSink, (void**) &spStreamSink);
                if(SUCCEEDED(hr))
                {
                    CComPtr<IMFMediaSink> spSink;
                    hr = spStreamSink->GetMediaSink(&spSink);
                    if(SUCCEEDED(hr))
                    {
                        spSink->Shutdown();
                    }
                }
            }
        }
    }
}

void CTedOutputNode::FlagExternalShutdownRequired()
{
    m_fExternalShutdownRequired = true;
}

// Non-public Helpers //

HRESULT CTedOutputNode::InitFromSink(IMFMediaSink* pSink)
{
    HRESULT hr = S_OK;

    CVisualContainer* pContainer = (CVisualContainer*) GetVisual();

    DWORD dwStreamSinkCount;
    IFC( pSink->GetStreamSinkCount(&dwStreamSinkCount) );

    for(DWORD i = 0; i < dwStreamSinkCount; ++i)
    {
        CComPtr<IMFStreamSink> spStreamSink;
        CComPtr<IMFTopologyNode> spStreamSinkNode;
        
        IFC( pSink->GetStreamSinkByIndex(i, &spStreamSink) );

        IFC( MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &spStreamSinkNode) );
        IFC( spStreamSinkNode->SetObject(spStreamSink) );
        m_arrStreamSinkNodes.Add(spStreamSinkNode);
        
        CVisualNode* pVisualNode = new CVisualNode(L"", false);
        CHECK_ALLOC( pVisualNode );
        
        pVisualNode->SetData((LONG_PTR) this);
        pVisualNode->SetIndex(i);

        pVisualNode->AddPin(true, (LONG_PTR) const_cast<CTedOutputNode*>(this), L"", i);

        pContainer->AddComponent(pVisualNode);
    }
    
Cleanup:
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// CTedAudioOutputNode

// Initialization //
HRESULT CTedAudioOutputNode::Init(const CAtlStringW& label)
{
    HRESULT hr = S_OK;

    CComPtr<IMFActivate> spSARAct;
    IFC(MFCreateAudioRendererActivate(&spSARAct));

    CTedOutputNode::Init(spSARAct, label);
Cleanup:
    return hr;
}

HRESULT CTedAudioOutputNode::Init(CTedAudioOutputMemo* pMemo)
{
    HRESULT hr = S_OK;

    CComPtr<IMFActivate> spSARAct;
    IFC(MFCreateAudioRendererActivate(&spSARAct));

    IFC(CTedOutputNode::Init(spSARAct, pMemo));

Cleanup:
    return hr;
}

HRESULT CTedAudioOutputNode::Init(const CAtlStringW& label, IMFStreamSink* pStreamSink)
{
    HRESULT hr = S_OK;

    assert(pStreamSink != NULL);
    
    IFC( CTedOutputNode::Init(pStreamSink, label) );
    
Cleanup:
    return hr;
}

// Accessors //

CTedNodeMemo* CTedAudioOutputNode::CreateMemo() const
{
    CTedAudioOutputMemo* pMemo = new CTedAudioOutputMemo(GetVisual()->Rect().x(), GetVisual()->Rect().y(), GetLabel(), GetID());
    pMemo->AddNodeAttributes(GetMFNode(0));
    GetMFNode(0)->AddRef();

    return pMemo;
}

///////////////////////////////////////////////////////////////////////////////
// CTedVideoOutputNode

// Initialization //

HRESULT CTedVideoOutputNode::Init(const CAtlStringW& label, HWND hVideoOutWindow) 
{
    HRESULT hr = S_OK;

    CComPtr<IMFActivate> spEVRAct;
    IFC(MFCreateVideoRendererActivate(hVideoOutWindow, &spEVRAct));

    IFC( CTedOutputNode::Init(spEVRAct, label) );

    m_hWnd = hVideoOutWindow;
Cleanup:
    return hr;
}

HRESULT CTedVideoOutputNode::Init(HWND hVideoOutWindow, CTedVideoOutputMemo* pMemo)
{
    HRESULT hr = S_OK;

    assert(pMemo != NULL);
    
    CComPtr<IMFActivate> spEVRAct;
    IFC(MFCreateVideoRendererActivate(hVideoOutWindow, &spEVRAct));

    IFC(CTedOutputNode::Init(spEVRAct, pMemo));

    m_hWnd = hVideoOutWindow;
Cleanup:
    return hr;
}

HRESULT CTedVideoOutputNode::Init(const CAtlStringW& label, HWND hVideoOutWindow, IMFStreamSink* pStreamSink) 
{
    HRESULT hr = S_OK;

    assert(pStreamSink != NULL);
    
    IFC( CTedOutputNode::Init(pStreamSink, label) );

    m_hWnd = hVideoOutWindow;
Cleanup:
    return hr;
}

// Accessors //

HWND CTedVideoOutputNode::GetVideoWindow() const
{
    return m_hWnd;
}

CTedNodeMemo* CTedVideoOutputNode::CreateMemo() const
{
    CTedVideoOutputMemo* pMemo = new CTedVideoOutputMemo(GetVisual()->Rect().x(), GetVisual()->Rect().y(), GetLabel(), GetID());
    pMemo->AddNodeAttributes(GetMFNode(0));

    return pMemo;
}

///////////////////////////////////////////////////////////////////////////////
// CTedCustomOutputNode

// Initialization //

HRESULT CTedCustomOutputNode::Init(GUID gidCustomSinkID, const CAtlStringW& label)
{
    HRESULT hr = S_OK;

    m_gidCustomSinkID = gidCustomSinkID;
    
    CComPtr<IMFMediaSink> spSink;
    IFC( CoCreateInstance(m_gidCustomSinkID, NULL, CLSCTX_INPROC_SERVER, IID_IMFMediaSink, (void**) &spSink) );
    
    IFC( CTedOutputNode::Init(spSink, label) );

Cleanup:
    return hr;
}

HRESULT CTedCustomOutputNode::Init(IMFMediaSink* pSink, const CAtlStringW& label)
{
    HRESULT hr = S_OK;

    m_gidCustomSinkID = GUID_NULL;

    IFC( CTedOutputNode::Init(pSink, label) );

Cleanup:
    return hr;
}

HRESULT CTedCustomOutputNode::Init(CTedCustomOutputMemo* pMemo)
{
    HRESULT hr = S_OK;

    assert(pMemo != NULL);

    m_gidCustomSinkID = pMemo->m_gidCustomSinkID;

    CComPtr<IMFMediaSink> spSink;
    IFC( CoCreateInstance(m_gidCustomSinkID, NULL, CLSCTX_INPROC_SERVER, IID_IMFMediaSink, (void**) &spSink) );
        
    IFC( CTedOutputNode::Init(spSink, pMemo) );

Cleanup:
    return hr;
}

CTedNodeMemo* CTedCustomOutputNode::CreateMemo() const
{
    CTedCustomOutputMemo* pMemo = new CTedCustomOutputMemo(GetVisual()->Rect().x(), GetVisual()->Rect().y(), GetLabel(), GetID(), m_gidCustomSinkID);

    for(DWORD i = 0; i < GetMFNodeCount(); i++)
    {
        pMemo->AddNodeAttributes(GetMFNode(0));
    }

    return pMemo;
}

///////////////////////////////////////////////////////////////////////////////
// CTedTransformNode

// Initialization //

HRESULT CTedTransformNode::Init(CLSID dmoCLSID, const CAtlStringW& label, bool fAutoInserted)
{
    HRESULT hr = S_OK;
    
    CTedTopologyNode::Init(label, fAutoInserted);

    IFC( InitTransform(dmoCLSID) );

Cleanup:
    return hr;
}

HRESULT CTedTransformNode::Init(IMFTransform* pTransform, CLSID dmoCLSID, const CAtlStringW& label, bool fAutoInserted)
{
    HRESULT hr = S_OK;

    CTedTopologyNode::Init(label, fAutoInserted);

    IFC( InitTransform(pTransform) );

    m_clsid = dmoCLSID;

Cleanup:
    return hr;
}

HRESULT CTedTransformNode::Init(IMFActivate* pTransformActivate, const CAtlStringW& label, bool fAutoInserted)
{
    HRESULT hr = S_OK;
    
    CTedTopologyNode::Init(label, fAutoInserted);
    
    IFC( InitTransform(pTransformActivate) );
    
Cleanup:
    return hr;
}

HRESULT CTedTransformNode::Init(CTedTransformMemo* pMemo)
{
    assert(pMemo != NULL);
    
    HRESULT hr = S_OK;
    
    CTedTopologyNode::Init(pMemo);

    IFC( InitTransform(pMemo->m_clsid) );
    
    IFC( PostInitFromMemoCopyAttributes(pMemo) );

Cleanup:
    return hr;
}

// Accessors //

HRESULT CTedTransformNode::GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin)
{
    HRESULT hr = S_OK;

    Pin.SetPNode(m_spTransformNode);
    Pin.SetIndex(nInput);
    
//Cleanup:

    return hr;
}

CTedNodeMemo* CTedTransformNode::CreateMemo() const
{
    CTedTransformMemo* pMemo = new CTedTransformMemo(GetVisual()->Rect().x(), GetVisual()->Rect().y(), GetLabel(), GetID(), m_clsid);
    pMemo->AddNodeAttributes(GetMFNode(0));

    return pMemo;
}

DWORD CTedTransformNode::GetMFNodeCount() const
{
    return 1;
}

IMFTopologyNode* CTedTransformNode::GetMFNode(DWORD nIndex) const
{
    assert(nIndex == 0);
        
    return m_spTransformNode;
}

bool CTedTransformNode::HasSameTransform(const CLSID& transformCLSID)
{
    if(transformCLSID == m_clsid)
    {
        return true;
    }

    return false;
}

CTedTopologyNode::TED_NODE_TYPE CTedTransformNode::GetType()
{
    return TED_TRANSFORM_NODE;
}

// Mutators //

HRESULT CTedTransformNode::CopyMediaTypes(IMFTopologyNode* pOldNode)
{
    HRESULT hr = S_OK;
    CComPtr<IUnknown> spOldTransformUnk, spNewTransformUnk;
    CComPtr<IMFTransform> spNewTransform, spOldTransform;
    DWORD dwNumInputs, dwNumOutputs;
    DWORD* pInputStreamIDs = NULL;
    DWORD* pOutputStreamIDs = NULL;

    CComPtr<IMFTopologyNode> spNewNode = m_spTransformNode;
    
    IFC( spNewNode->GetObject(&spNewTransformUnk) );
    IFC( spNewTransformUnk->QueryInterface(IID_IMFTransform, (void**) &spNewTransform) );

    IFC( pOldNode->GetObject(&spOldTransformUnk) );
    IFC( spOldTransformUnk->QueryInterface(IID_IMFTransform, (void**) &spOldTransform) );

    IFC( spNewTransform->GetStreamCount(&dwNumInputs, &dwNumOutputs) );

    pInputStreamIDs = new DWORD[dwNumInputs];
    CHECK_ALLOC( pInputStreamIDs );
    pOutputStreamIDs = new DWORD[dwNumOutputs];
    CHECK_ALLOC( pOutputStreamIDs );

    hr = spNewTransform->GetStreamIDs(dwNumInputs, pInputStreamIDs, dwNumOutputs, pOutputStreamIDs);

    if(FAILED(hr))
    {
        if(hr ==  E_NOTIMPL)
        {
            for(DWORD i = 0; i < dwNumInputs; ++i)
            {
                pInputStreamIDs[i] = i;
                pOutputStreamIDs[i] = i;
            }
        }
        else
        {
            IFC(hr);
        }
    }
        
    if ( spOldTransform != spNewTransform )
    {
        for(DWORD i = 0; i < dwNumInputs; ++i)
        {
            CComPtr<IMFMediaType> spType;
            
            IFC( spOldTransform->GetInputCurrentType(pInputStreamIDs[i], &spType) );
            IFC( spNewTransform->SetInputType(pInputStreamIDs[i], spType, 0) );
        }

        for(DWORD i = 0; i < dwNumOutputs; ++i)
        {
            CComPtr<IMFMediaType> spType;

            IFC( spOldTransform->GetOutputCurrentType(pOutputStreamIDs[i], &spType) );
            IFC( spNewTransform->SetOutputType(pOutputStreamIDs[i], spType, 0) );
        }
    }

Cleanup:
    delete[] pInputStreamIDs;
    delete[] pOutputStreamIDs;
    
    return hr;
}

HRESULT CTedTransformNode::WrapTransform(CLogger* pLogger)
{
    HRESULT hr = S_OK;
    CMFTransformWrapper* pTransformWrapper = NULL;
    CComPtr<IUnknown> spTransformUnk;
    CComPtr<IMFTransform> spTransform;
    
    IFC( m_spTransformNode->GetObject(&spTransformUnk) );
    IFC( spTransformUnk->QueryInterface(IID_IMFTransform, (void**) &spTransform) );
    
    pTransformWrapper = new CMFTransformWrapper(spTransform, pLogger);
    CHECK_ALLOC( pTransformWrapper );
    pTransformWrapper->AddRef();

    IFC( m_spTransformNode->SetObject(pTransformWrapper) );

Cleanup:
    if(pTransformWrapper) pTransformWrapper->Release();
    return hr;
}

HRESULT CTedTransformNode::ResetD3DManager()
{
    HRESULT hr;

    IFC( m_spTransform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, NULL) );
    IFC( m_spTransformNode->SetUINT32(MF_TOPONODE_D3DAWARE, 0) );

Cleanup:
    return hr;
}

// Non-public Helpers //

HRESULT CTedTransformNode::InitTransform(CLSID dmoCLSID)
{
    HRESULT hr = S_OK;
    CComPtr<IUnknown> spMFTUnk;
    CComPtr<IMFTransform> spTransform;
    DWORD dwNumInputs = 0, dwNumOutputs = 0;
    
    m_clsid = dmoCLSID;
    
    CComPtr<IMFTopologyNode> spNode;

    IFC( MFCreateTopologyNode( MF_TOPOLOGY_TRANSFORM_NODE, &spNode ));

    IFC( CoCreateInstance(m_clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **) &spMFTUnk) );

    IFC( spNode->SetObject(spMFTUnk) );
        
    IFC( spNode->SetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, m_clsid) );

    IFC( spMFTUnk->QueryInterface(IID_IMFTransform, (void**) &m_spTransform) );
    IFC( m_spTransform->GetStreamCount(&dwNumInputs, &dwNumOutputs) );
    
    m_spTransformNode = spNode;
        
    for(size_t i = 0; i < dwNumInputs; i++)
    {
        AddPin(true, L"", (DWORD) i);
    }

    for(size_t i = 0; i < dwNumOutputs; i++)
    {
        AddPin(false, L"", (DWORD) i);
    }

Cleanup:
    return hr;
}

HRESULT CTedTransformNode::InitTransform(IMFTransform* pTransform)
{
    HRESULT hr = S_OK;
    DWORD dwNumInputs = 0, dwNumOutputs = 0;

    m_spTransform = pTransform;
    
    CComPtr<IMFTopologyNode> spNode;
    IFC( MFCreateTopologyNode( MF_TOPOLOGY_TRANSFORM_NODE, &spNode ));
    IFC(spNode->SetObject(pTransform));
 
    m_spTransformNode = spNode;
       
    IFC(pTransform->GetStreamCount(&dwNumInputs, &dwNumOutputs));
    for(size_t i = 0; i < dwNumInputs; i++)
    {
        AddPin(true, L"", (DWORD) i);
    }

    for(size_t i = 0; i < dwNumOutputs; i++)
    {
        AddPin(false, L"", (DWORD) i);
    }

Cleanup:
    return hr;
}

HRESULT CTedTransformNode::InitTransform(IMFActivate* pTransformActivate)
{
    HRESULT hr;
    CComPtr<IMFTransform> spTransform;
    
    pTransformActivate->GetGUID(MFT_TRANSFORM_CLSID_Attribute, &m_clsid);
    
    IFC( pTransformActivate->ActivateObject(IID_IMFTransform, (void**) &spTransform) );
    IFC( InitTransform(spTransform) );
    
Cleanup:
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// CTedTeeNode

// Initialization //

HRESULT CTedTeeNode::Init()
{
    HRESULT hr = S_OK;
    
    CTedTopologyNode::Init(LoadAtlString(IDS_TEE));

    IFC( MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &m_spTeeNode ));

    AddPin(true, L"", 0);
    AddPin(false, L"", 0);

    m_nNextOutputIndex = 1;
Cleanup:
    return hr;
}

HRESULT CTedTeeNode::Init(CTedTeeMemo* pMemo)
{
    assert(pMemo != NULL);
    
    HRESULT hr = S_OK;

    CTedTopologyNode::Init(pMemo);

    IFC( MFCreateTopologyNode( MF_TOPOLOGY_TEE_NODE, &m_spTeeNode ));

    AddPin(true, L"", 0);

    m_nNextOutputIndex = pMemo->m_nNextOutputIndex;
    
    for(DWORD i = 0; i < m_nNextOutputIndex; i++)
    {
        AddPin(false, L"", i);
    }

    IFC( PostInitFromMemoCopyAttributes(pMemo) );

Cleanup:
    return hr;
};

// Accessors //

HRESULT CTedTeeNode::GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin)
{
    HRESULT hr = S_OK;

    Pin.SetPNode(m_spTeeNode);
    Pin.SetIndex(nInput);

    return hr;
}

CTedNodeMemo* CTedTeeNode::CreateMemo() const
{
    CTedTeeMemo* pMemo = new CTedTeeMemo(GetVisual()->Rect().x(), GetVisual()->Rect().y(), GetLabel(), GetID(), m_nNextOutputIndex);
    pMemo->AddNodeAttributes(GetMFNode(0));

    return pMemo;
}

DWORD CTedTeeNode::GetMFNodeCount() const
{
    return 1;
}

IMFTopologyNode* CTedTeeNode::GetMFNode(DWORD nIndex) const
{
    assert(nIndex == 0);

    return m_spTeeNode;
}

CTedTopologyNode::TED_NODE_TYPE CTedTeeNode::GetType()
{
    return TED_TEE_NODE;
}

// Mutators //

void CTedTeeNode::NotifyConnection()
{
    DWORD outputCount;

    m_spTeeNode->GetOutputCount(&outputCount);
    
    if(outputCount >= m_nNextOutputIndex)
    {
        AddPin(false, L"", m_nNextOutputIndex++);
    }
}

///////////////////////////////////////////////////////////////////////////////
// CTedEditorVisualObjectEventHandler

CTedEditorVisualObjectEventHandler::CTedEditorVisualObjectEventHandler(CTedTopologyEditor* pEditor)
    : m_pEditor(pEditor)
{
}

void CTedEditorVisualObjectEventHandler::NotifyObjectDeleted(CVisualObject* pVisualObj)
{
    if(pVisualObj->Type() == CVisualObject::CONNECTOR)
    {
        CTedTopologyConnection* pConn = (CTedTopologyConnection*) pVisualObj->GetData();
        if(pConn == NULL) return;
        
        CTedTopologyNode* pNode = m_pEditor->FindNode(pConn->GetOutputNodeID());

        HRESULT hr = m_pEditor->FullDisconnectNodes(pNode, pConn->GetOutputPinID());
        
        if(FAILED(hr))
        {
            CAtlString strErr;
            strErr.Format(L"%x", hr);
            MessageBox(NULL, LoadAtlString(IDS_E_DELETE_CONNECTION), strErr, MB_OK);
        }
    }
    else if(pVisualObj->Type() == CVisualObject::NODE || pVisualObj->Type() == CVisualObject::CONTAINER)
    {
        HRESULT hr = m_pEditor->RemoveNodeWithVisual(pVisualObj);
        
        if(FAILED(hr))
        {
            CAtlString strErr;
            strErr.Format(L"%x", hr);
            MessageBox(NULL, LoadAtlString(IDS_E_DELETE_NODE), strErr, MB_OK);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// CTedNodeCreator

CTedNodeCreator CTedNodeCreator::m_Singleton;

CTedNodeCreator::CTedNodeCreator()
{
}

CTedNodeCreator::~CTedNodeCreator()
{
}

CTedNodeCreator* CTedNodeCreator::GetSingleton()
{
    return &m_Singleton;
}

HRESULT CTedNodeCreator::CreateSource(const CAtlStringW& strSourceURL, IMFMediaSource* pFromSource, CTedSourceNode** ppSourceNode)
{
    if(NULL == ppSourceNode)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;

    CAtlStringW strLabel;
    int charLoc = strSourceURL.ReverseFind('\\');
    if(charLoc != -1)
    {
        strLabel = strSourceURL.Mid(charLoc + 1);
    }
    else
    {
        strLabel = strSourceURL;
    }

    CTedSourceNode* pSourceNode = new CTedSourceNode;
    CHECK_ALLOC( pSourceNode );

    if( pFromSource )
    {
        hr = pSourceNode->Init(strSourceURL, strLabel, pFromSource);
    }
    else
    {
        hr = pSourceNode->Init(strSourceURL, strLabel);
    }

    if(FAILED(hr))
    {
        delete pSourceNode;
    }
    else
    {
        *ppSourceNode = pSourceNode;
    }

Cleanup:
    return hr;
}

HRESULT CTedNodeCreator::CreateCaptureSource(IMFMediaSource* pSource, CTedSourceNode** ppSourceNode)
{
    if(NULL == ppSourceNode)
    {
        return E_POINTER;
    }

    if(NULL == pSource)
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = S_OK;

    CTedSourceNode* pSourceNode = new CTedSourceNode;
    CHECK_ALLOC( pSourceNode );

    //
    // Get the friendly name
    //
    {
        WCHAR* pwsz = NULL;
        HRESULT hrName = S_OK;
        CComPtr<IMFAttributes> spAttributes = NULL;
        UINT32 nLen = 0;

        hr = pSource->QueryInterface( __uuidof(IMFAttributes), (LPVOID*)&spAttributes );

        hrName = spAttributes->GetStringLength( MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, 
                                           &nLen );

        if ( SUCCEEDED( hrName ) )
        {
            pwsz = new WCHAR[ nLen + 1 ];
            if ( pwsz == NULL )
            {
                hrName = E_OUTOFMEMORY;
            }
        }

        if ( SUCCEEDED( hrName ) )
        {
            hrName = spAttributes->GetString( MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                         pwsz,
                                         nLen + 1,
                                         &nLen );
        }

        if ( SUCCEEDED( hrName ) )
        {
            hr = pSourceNode->Init(pSource, pwsz);
        }
        else
        {
            hr = pSourceNode->Init(pSource, LoadAtlString(IDS_CAPTURE_SOURCE));
        }

        delete[] pwsz;
    }

    if(FAILED(hr))
    {
        delete pSourceNode;
    }
    else
    {
        *ppSourceNode = pSourceNode;
    }

Cleanup:
    return hr;
}

HRESULT CTedNodeCreator::CreateSAR(CTedAudioOutputNode** ppSAR)
{
    if(NULL == ppSAR)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    CTedAudioOutputNode * pNode = NULL;

    pNode = new CTedAudioOutputNode();
    CHECK_ALLOC( pNode );
    hr = pNode->Init(LoadAtlString(IDS_AUDIO_RENDERER));

    if(FAILED(hr))
    {
        delete pNode;
    }
    else
    {
        *ppSAR = pNode;
    }

Cleanup:    
    return hr;
}

HRESULT CTedNodeCreator::CreateEVR(ITedVideoWindowHandler* pVideoHandler, CTedVideoOutputNode** ppEVR)
{
    if(NULL == ppEVR)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    CTedVideoOutputNode * pNode = NULL;
    HWND hWnd = NULL;
    
    if(pVideoHandler)
    {
        IFC( pVideoHandler->GetVideoWindow((LONG_PTR*) &hWnd) );
    }

    pNode = new CTedVideoOutputNode();
    CHECK_ALLOC( pNode );
    hr = pNode->Init(LoadAtlString(IDS_VIDEO_RENDERER), hWnd);

    if(FAILED(hr))
    {
        delete pNode;
        if(hWnd != NULL) pVideoHandler->ReleaseVideoWindow((LONG_PTR) hWnd);
    }
    else
    {
        *ppEVR = pNode;
    }

Cleanup:
    return hr;
}

HRESULT CTedNodeCreator::CreateCustomSink(IMFMediaSink* pSink, CTedCustomOutputNode** ppSinkNode)
{
    if(NULL == ppSinkNode)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    CTedCustomOutputNode* pNode = NULL;
    HWND hWnd = NULL;

    pNode = new CTedCustomOutputNode();
    CHECK_ALLOC( pNode );
    hr =  pNode->Init(pSink, LoadAtlString(IDS_CUSTOM_SINK));

    if(FAILED(hr))
    {
        delete pNode;
    }
    else
    {
        *ppSinkNode = pNode;
    }

Cleanup:
    return hr;
}

HRESULT CTedNodeCreator::CreateCustomSink(GUID gidCustomSinkID, CTedCustomOutputNode** ppSinkNode)
{
    if(NULL == ppSinkNode)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    
    CTedCustomOutputNode* pNode = new CTedCustomOutputNode();
    CHECK_ALLOC( pNode );
    hr = pNode->Init(gidCustomSinkID, LoadAtlString(IDS_CUSTOM_SINK));

    if(FAILED(hr))
    {
        delete pNode;
    }
    else
    {
        *ppSinkNode = pNode;
    }

Cleanup:
    return hr;
}

HRESULT CTedNodeCreator::CreateTransform(CLSID clsidDMO, const CAtlStringW& strName, CTedTransformNode** ppTransformNode)
{
    if(NULL == ppTransformNode)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    CTedTransformNode * pNode = NULL;
    
    pNode = new CTedTransformNode();
    CHECK_ALLOC( pNode );
    hr = pNode->Init(clsidDMO, strName);
    
    if(FAILED(hr))
    {
        delete pNode;
    }
    else
    {
        *ppTransformNode = pNode;
    }

Cleanup:        
    return hr;
}

HRESULT CTedNodeCreator::CreateTransform(IMFActivate* pTransformActivate, CTedTransformNode** ppTransformNode)
{
    if(NULL == ppTransformNode)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    CTedTransformNode * pNode = NULL;
    LPWSTR szName = NULL;
    
    pNode = new CTedTransformNode();
    CHECK_ALLOC( pNode );
    IFC( pTransformActivate->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &szName, NULL) );
    hr = pNode->Init(pTransformActivate, szName);
    
    if(FAILED(hr))
    {
        delete pNode;
    }
    else
    {
        *ppTransformNode = pNode;
    }

Cleanup:
    CoTaskMemFree(szName);
    
    return hr;
}

HRESULT CTedNodeCreator::CreateTee(CTedTeeNode** ppTeeNode)
{
    HRESULT hr = S_OK;

    CTedTeeNode * pNode = NULL;
    
    pNode = new CTedTeeNode;
    CHECK_ALLOC( pNode );
    hr = pNode->Init();

    if(FAILED(hr))
    {
        delete pNode;
    }
    else
    {
        *ppTeeNode = pNode;
    }

Cleanup:
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// CTedTopologyEditor

// Initialization //
const int CTedTopologyEditor::MARGIN_SIZE = 20;

CTedTopologyEditor::CTedTopologyEditor()
    : m_pMoveHandler(NULL)
    , m_pConnectHandler(NULL)
    , m_nProtectedSourceCount(0)
    , m_fSaved(true)
    , m_fShutdownSources(true)
    , m_VisualEventHandler(this)
    , m_LastSeqID(0)
{
    MFCreateSequencerSource(NULL, &m_spSequencer);
}

CTedTopologyEditor::~CTedTopologyEditor()
{
    for(size_t i = 0; i < m_Nodes.GetCount(); ++i)
    {
        if(m_fShutdownSources)
        {
            if(m_Nodes.GetAt(i)->GetType() == CTedTopologyNode::TED_SOURCE_NODE)
            {
                CTedSourceNode* pSource = (CTedSourceNode*) m_Nodes.GetAt(i);
                pSource->ShutdownMFSource();
            }
            else if(m_Nodes.GetAt(i)->GetType() == CTedTopologyNode::TED_OUTPUT_NODE)
            {
                CTedOutputNode* pOutput = (CTedOutputNode*) m_Nodes.GetAt(i);
                pOutput->ShutdownMFSink();
            }
        }
        
        delete m_Nodes.GetAt(i);
    }

    for(size_t i = 0; i < m_Connections.GetCount(); ++i)
    {
        delete m_Connections.GetAt(i);
    }

    delete m_pMoveHandler;
    delete m_pConnectHandler;

    for(size_t i = 0; i < m_arrLoggers.GetCount(); ++i)
    {
        m_arrLoggers.GetAt(i)->Release();
    }

    if(m_spSequencer.p)
    {
        CComPtr<IMFMediaSource> spSource;
        HRESULT hr = m_spSequencer->QueryInterface(IID_IMFMediaSource, (void**) &spSource);
        if(SUCCEEDED(hr))
        {
            spSource->Shutdown();
        }
    }
    
    CTedSourceNode::ReleaseResolver();
}

HRESULT CTedTopologyEditor::Init(ITedVideoWindowHandler* pVideoCallback, ITedPropertyController* pPropertyCallback, ITedTopoEventHandler* pEventCallback, CTopoViewerWindow* pView)
{
    assert(pView != NULL);
    
    HRESULT hr = S_OK;

    m_spVideoCallback = pVideoCallback;
    m_spPropertyCallback = pPropertyCallback;
    m_spEventCallback = pEventCallback;
    m_pView = pView;
    
    IFC( NewTopology() );
    
Cleanup:
    return hr;
}

HRESULT CTedTopologyEditor::NewTopology()
{
    HRESULT hr = S_OK;

    m_spTopology.Release();
    IFC( MFCreateTopology(&m_spTopology) );

    for(size_t i = 0; i < m_Connections.GetCount(); ++i)
    {
        delete m_Connections.GetAt(i);
    }

    for(size_t i = 0; i < m_Nodes.GetCount(); ++i)
    {
        CTedTopologyNode* pNode = m_Nodes.GetAt(i);

        if(pNode->GetType() == CTedTopologyNode::TED_SOURCE_NODE && m_fShutdownSources)
        {
            CTedSourceNode* pSource = (CTedSourceNode*) pNode;
            pSource->ShutdownMFSource();
        }
        else if(pNode->GetType() == CTedTopologyNode::TED_OUTPUT_NODE && m_fShutdownSources)
        {
            CTedOutputNode* pOutput = (CTedOutputNode*) pNode;
            pOutput->ShutdownMFSink();
            
            if(pNode->GetVideoWindow() != NULL)
            {
                m_spVideoCallback->ReleaseVideoWindow((LONG_PTR) pNode->GetVideoWindow());
            }
        }
        
        delete pNode;
    }

    m_Nodes.RemoveAll();
    m_Connections.RemoveAll();

    delete m_pMoveHandler;
    delete m_pConnectHandler;

    m_pMoveHandler = new CMoveComponentHandler(m_pView->PTree(), m_spPropertyCallback);
    CHECK_ALLOC( m_pMoveHandler );

    m_pConnectHandler = new CConnectPinHandler(m_pView->PTree(), this, m_spPropertyCallback);
    CHECK_ALLOC( m_pConnectHandler );

    CVisualNode::SetPinHandler(m_pConnectHandler);
    
    m_nProtectedSourceCount = 0;

    for(size_t i = 0; i < m_arrLoggers.GetCount(); ++i)
    {
        m_arrLoggers.GetAt(i)->Release();
    }

    m_arrLoggers.RemoveAll();

    m_fSaved = true;
    m_fShutdownSources = true;

    if(m_spPropertyCallback.p) m_spPropertyCallback->ClearProperties();

	m_pView->PTree()->SetEventHandler(&m_VisualEventHandler);

Cleanup:
    return hr;
}

// Accessors - General //

HRESULT CTedTopologyEditor::GetTopology(IMFTopology** ppTopo, BOOL* pfIsProtected)
{
    HRESULT hr = S_OK;
    CComPtr<IMFTopology> spNewTopo;
    
    if(NULL == ppTopo || NULL == pfIsProtected)
    {
        return E_POINTER;
    }

    bool fMultipleSrc = HasMultipleSources();
    
    IFC( MFCreateTopology(&spNewTopo) );
    IFC( spNewTopo->CloneFrom(m_spTopology) );

    *pfIsProtected = false;
    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        CTedTopologyNode::TED_NODE_TYPE NodeType = m_Nodes.GetAt(i)->GetType();
        if(NodeType == CTedTopologyNode::TED_SOURCE_NODE)
        {
            CTedSourceNode* pSourceNode = (CTedSourceNode*) m_Nodes.GetAt(i);

            if(pSourceNode->IsProtected())
            {
                *pfIsProtected = true;
            }
            
            IFC( pSourceNode->SelectValidStreams() );
            
            // Source nodes that get passed out cannot be shut down here as we are unsure
            // if the external app has finished using them
            pSourceNode->FlagExternalShutdownRequired();
        }
        else if(NodeType == CTedTopologyNode::TED_OUTPUT_NODE)
        {
            CTedOutputNode* pOutputNode = (CTedOutputNode*) m_Nodes.GetAt(i);
            
            pOutputNode->FlagExternalShutdownRequired();
        }
    }
    
    WORD cNodes;
    IFC( spNewTopo->GetNodeCount(&cNodes) );
    for(WORD i = 0; i < cNodes; i++)
    {
        CComPtr<IMFTopologyNode> spNode;
        IFC( spNewTopo->GetNode(i, &spNode) );
        
        spNode->DeleteItem(MF_TOPONODE_ERRORCODE);
        spNode->DeleteItem(MF_TOPONODE_ERROR_MAJORTYPE);
        spNode->DeleteItem(MF_TOPONODE_ERROR_SUBTYPE);

        BOOL fConnected = FALSE;
        
        DWORD cInputs;
        IFC( spNode->GetInputCount(&cInputs) )
        for(DWORD j = 0; j < cInputs; j++)
        {
            CComPtr<IMFTopologyNode> spUpNode;
            DWORD dwUpIndex;
            HRESULT hrPin = spNode->GetInput(j, &spUpNode, &dwUpIndex);
            if(SUCCEEDED(hrPin))
            {
                fConnected = TRUE;
                break;
            }
        }
        
        if(!fConnected)
        {
            DWORD cOutputs;
            IFC( spNode->GetOutputCount(&cOutputs) );
            for(DWORD j = 0; j < cOutputs; j++)
            {
                CComPtr<IMFTopologyNode> spDownNode;
                DWORD dwDownIndex;
                HRESULT hrPin = spNode->GetOutput(j, &spDownNode, &dwDownIndex);
                if(SUCCEEDED(hrPin))
                {
                    fConnected = TRUE;
                    break;
                }
            }
        }
        
        if(!fConnected)
        {
            IFC( spNewTopo->RemoveNode(spNode) );
            cNodes--;
            i--;
        }
    }
    
    if(fMultipleSrc)
    {
        MFSequencerElementId NewID;
        IFC( m_spSequencer->AppendTopology(spNewTopo, SequencerTopologyFlags_Last, &NewID) );
        
        if(m_LastSeqID != 0)
        {
            IFC( m_spSequencer->DeleteTopology(m_LastSeqID) );
        }
        
        m_LastSeqID = NewID;

        CComPtr<IMFMediaSource> spSrc;
        IFC( m_spSequencer->QueryInterface(IID_IMFMediaSource, (void**) &spSrc) );
        
        CComPtr<IMFPresentationDescriptor> spPD;
        IFC( spSrc->CreatePresentationDescriptor(&spPD) );
        
        CComPtr<IMFMediaSourceTopologyProvider> spSrcTopoProvider;
        IFC( m_spSequencer->QueryInterface(IID_IMFMediaSourceTopologyProvider, (void**) &spSrcTopoProvider) );

		spNewTopo.Release();
        IFC( spSrcTopoProvider->GetMediaSourceTopology(spPD, &spNewTopo) );
    }
    
    *ppTopo = spNewTopo;
    (*ppTopo)->AddRef();

Cleanup:
    return hr;
}

bool CTedTopologyEditor::HasSource()
{
    HRESULT hr = S_OK;
    WORD cNodes;
    WORD n;

    IFC(m_spTopology->GetNodeCount(&cNodes));

    for(n = 0; n < cNodes; n++)
    {
        CComPtr<IMFTopologyNode> spNode;
        MF_TOPOLOGY_TYPE Type;
        
        IFC( m_spTopology->GetNode(n, &spNode) );
        IFC( spNode->GetNodeType(&Type) );

        if(Type == MF_TOPOLOGY_SOURCESTREAM_NODE)
        {
            return true;
        }
    }

Cleanup:
    return false;
}

DWORD CTedTopologyEditor::GetNodeCount()
{
    return (DWORD) m_Nodes.GetCount();
}

CTedTopologyNode* CTedTopologyEditor::GetNode(DWORD dwIndex)
{
    return m_Nodes.GetAt(dwIndex);
}

// Accessors - Find //

CTedTopologyConnection* CTedTopologyEditor::FindDownstreamConnection(int nBeginNodeID, int nBeginPinID)
{
    for(size_t i = 0; i < m_Connections.GetCount(); ++i)
    {
        CTedTopologyConnection* pConn = m_Connections.GetAt(i);
        if(pConn->GetOutputNodeID() == nBeginNodeID && pConn->GetOutputPinID() == nBeginPinID)
        {
            return pConn;
        }
    }

    return NULL;
}

CTedTopologyConnection* CTedTopologyEditor::FindUpstreamConnection(int nBeginNodeID, int nBeginPinID)
{
    for(size_t i = 0; i < m_Connections.GetCount(); ++i)
    {
        CTedTopologyConnection* pConn = m_Connections.GetAt(i);
        if(pConn->GetInputNodeID() == nBeginNodeID && pConn->GetInputPinID() == nBeginPinID)
        {
            return pConn;
        }
    }

    return NULL;
}

CTedTopologyNode* CTedTopologyEditor::FindNode(int nodeID)
{
    for(size_t i = 0; i < m_Nodes.GetCount(); ++i)
    {
        CTedTopologyNode* pNode = m_Nodes.GetAt(i);

        if(pNode->GetID() == nodeID)
        {
            return pNode;
        }
    }

    return NULL;
}

// Mutators - General //

HRESULT CTedTopologyEditor::MergeTopology(IMFTopology* pTopo)
{
    assert(pTopo != NULL);
        
    HRESULT hr = S_OK;

    typedef struct _SourcePair
    {
        CTedSourceNode* m_pOldSource;
        CTedSourceNode* m_pNewSource;
    } SourcePair;
    
    // All connectors are invalidated by the new topology
    RemoveAllConnectors();
    
    int xPos, yPos;
    CAtlArray<SourcePair> arrSourcePairs;
    
    CComPtr<IMFCollection> spSourceNodeCollection;
    IFC(pTopo->GetSourceNodeCollection(&spSourceNodeCollection) );

    DWORD dwSourceNodeCount = 0;
    IFC( spSourceNodeCollection->GetElementCount(&dwSourceNodeCount) );

    for(DWORD i = 0; i < dwSourceNodeCount; i++)
    {
        CComPtr<IUnknown> spSourceNodeUnk;
        CComPtr<IMFTopologyNode> spSourceNode;
        CComPtr<IMFTopologyNode> spDownstreamNode;
        DWORD nNextIndex;

        // Get the next source stream node; find out if we have added this source already by looking up in our source table
        IFC( spSourceNodeCollection->GetElement(i, &spSourceNodeUnk) );
        IFC( spSourceNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**) &spSourceNode) );

        TOPOID idSourceNode;
        IFC( spSourceNode->GetTopoNodeID(&idSourceNode) );

        CTedSourceNode* pCurrentSource = NULL;
        CTedSourceNode* pNewSource = NULL;
        for(size_t j = 0; j < arrSourcePairs.GetCount(); j++)
        {
            if(arrSourcePairs.GetAt(j).m_pOldSource->GetIndexOf(idSourceNode) != (DWORD) -1)
            {
                pCurrentSource = arrSourcePairs.GetAt(j).m_pOldSource;
                pNewSource = arrSourcePairs.GetAt(j).m_pNewSource;
                break;
            }
        }

        // If there was not a source already in our source table, create a new one
        if(!pCurrentSource)
        {
            pCurrentSource = FindSourceNode(idSourceNode);

            if(!pCurrentSource) continue;

            pNewSource = new CTedSourceNode();
            CHECK_ALLOC( pNewSource );

            if( pCurrentSource->IsInitializedFromMFSource() )
            {
                pNewSource->Init(pCurrentSource->GetMFSource(), pCurrentSource->GetLabel());
            }
            else
            {
                pNewSource->Init(pCurrentSource->GetURL(), pCurrentSource->GetLabel());
            }
            pNewSource->GetVisual()->Move(pCurrentSource->GetVisual()->Rect().x(), pCurrentSource->GetVisual()->Rect().y());

            SourcePair sourcePair = {pCurrentSource, pNewSource};
            arrSourcePairs.Add(sourcePair);
        }

        DWORD realIndex = pCurrentSource->GetIndexOf(idSourceNode);
        
        xPos = int(pCurrentSource->GetVisual()->Rect().x() + pCurrentSource->GetVisual()->Rect().w() + MARGIN_SIZE);
        yPos = int(pCurrentSource->GetVisual()->Rect().y() + CVisualContainer::TOP_MARGIN * (realIndex + 1) + CVisualContainer::COMP_DEF_HEIGHT * realIndex);

        IFC( pNewSource->CopyAttributes(spSourceNode, realIndex) );
        
        hr = spSourceNode->GetOutput(0, &spDownstreamNode, &nNextIndex);
        if(FAILED(hr) || spDownstreamNode.p == NULL) continue;

        IFC( MergeBranch(pNewSource, realIndex, spDownstreamNode, nNextIndex, xPos, yPos, true) );
    }

    for(size_t i = 0; i < arrSourcePairs.GetCount(); i++)
    {
        RemoveNode(arrSourcePairs.GetAt(i).m_pOldSource);
        IFC( AddComponent(arrSourcePairs.GetAt(i).m_pNewSource) );
    }

    m_pView->PTree()->RouteAllConnectors();
    m_pView->NotifyNewVisuals();
    m_pView->InvalidateRect(NULL);

    RemoveOrphanTransforms();
    
    m_fShutdownSources = true;
    
Cleanup:
    return hr;
}


HRESULT CTedTopologyEditor::SpyNodeWithVisual(CVisualObject* pVisual)
 {
    assert(pVisual != NULL);
    
    CTedTopologyNode* pFoundNode = NULL;
    HRESULT hr = S_OK;

    for(size_t i = 0; i < m_Nodes.GetCount(); ++i)
    {
        if(m_Nodes.GetAt(i)->GetVisual() == pVisual)
        {
            pFoundNode = m_Nodes.GetAt(i);
            break;
        }
    }

    if(NULL != pFoundNode)
    {
        switch(pFoundNode->GetType())
        {
            case CTedTopologyNode::TED_TRANSFORM_NODE:
            {
                CTedTransformNode* pTedTransform = (CTedTransformNode*) pFoundNode;

                TOPOID nodeID;
                IFC( pTedTransform->GetNodeID(0, nodeID) );
                
                CAtlStringW fileName;
                fileName.Format(L"%s-%d.txt", pTedTransform->GetLabel(), nodeID);

                CLogger* pLogger = new CLogger(fileName);
                CHECK_ALLOC( pLogger );
                
                IFC( pTedTransform->WrapTransform(pLogger) );

                m_arrLoggers.Add(pLogger);
                pLogger->AddRef();

                break;
            }
            case CTedTopologyNode::TED_OUTPUT_NODE:
            {
                CTedOutputNode* pTedOutput = (CTedOutputNode*) pFoundNode;

                TOPOID nodeID;
                IFC( pTedOutput->GetNodeID(0, nodeID) );

                CAtlStringW fileName;
                fileName.Format(L"%s-%d.txt", pTedOutput->GetLabel(), nodeID);

                CLogger* pLogger = new CLogger(fileName);
                CHECK_ALLOC( pLogger );

                IFC( pTedOutput->WrapStreamSink(pLogger) );

                m_arrLoggers.Add(pLogger);
                pLogger->AddRef();

                break;
            }
            default:
            {
                break;  
            }
        }
    }
    
Cleanup:
    return hr;    
}

HRESULT CTedTopologyEditor::ShowTopology(IMFTopology* pTopology, LPCWSTR szSourceURL)
{
    HRESULT hr = S_OK;
    DWORD dwSourceNodeCount = 0;
    CComPtr<IUnknown> spSourceNodeUnk;
    CComPtr<IMFTopologyNode> spSourceNode;
    CComPtr<IMFMediaSource> spSource;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    
    NewTopology();

    CComPtr<IMFCollection> spSourceNodeCollection;
    IFC(pTopology->GetSourceNodeCollection(&spSourceNodeCollection) );

    IFC( spSourceNodeCollection->GetElementCount(&dwSourceNodeCount) );
    if(dwSourceNodeCount == 0) IFC(E_INVALIDARG);

    IFC( spSourceNodeCollection->GetElement(0, &spSourceNodeUnk) );
    IFC( spSourceNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**) &spSourceNode) );
    IFC( spSourceNode->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**) &spSource) );
    
    CTedSourceNode* pNewSource;
    IFC( pNodeCreator->CreateSource(szSourceURL, spSource, &pNewSource) );

    pNewSource->GetVisual()->Move(10, 10);

    int xPos = 10;
    int yPos = 10;
    
    xPos += int(pNewSource->GetVisual()->Rect().w() + MARGIN_SIZE);
    
    for(DWORD i = 0; i < dwSourceNodeCount; i++)
    {
        CComPtr<IUnknown> spSourceNodeUnk;
        CComPtr<IMFTopologyNode> spSourceNode;
        CComPtr<IMFTopologyNode> spDownstreamNode;
        DWORD nNextIndex;

        yPos = int(pNewSource->GetVisual()->Rect().y() + CVisualContainer::TOP_MARGIN * (i + 1) + CVisualContainer::COMP_DEF_HEIGHT * i);
        
        IFC( spSourceNodeCollection->GetElement(i, (IUnknown**) &spSourceNodeUnk) );
        IFC( spSourceNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**) &spSourceNode) );
        IFC( spSourceNode->GetOutput(0, &spDownstreamNode, &nNextIndex) );

        if(spDownstreamNode.p == NULL) continue;

        CComPtr<IMFPresentationDescriptor> spPD;
        IFC( spSourceNode->GetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, IID_IMFPresentationDescriptor, (LPVOID*) &spPD) );
        
        DWORD dwSDCount;
        IFC( spPD->GetStreamDescriptorCount(&dwSDCount) );

        CComPtr<IMFStreamDescriptor> spNodeSD;
        IFC( spSourceNode->GetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, IID_IMFStreamDescriptor, (LPVOID*) &spNodeSD) );

        DWORD dwOutputIndex = 0;
        for(DWORD j = 0; j < dwSDCount; j++)
        {
            CComPtr<IMFStreamDescriptor> spSD;
            BOOL fSelected;
            IFC( spPD->GetStreamDescriptorByIndex(j, &fSelected, &spSD) );

            if(spSD.p == spNodeSD.p)
            {
                dwOutputIndex = j;
                break;
            }
        }

        IFC( MergeBranch(pNewSource, dwOutputIndex, spDownstreamNode, nNextIndex, xPos, yPos, false) );

        yPos += int(pNewSource->GetVisual()->Rect().h() + MARGIN_SIZE);
    }

    IFC( AddComponent(pNewSource) );
    
    m_pView->PTree()->RouteAllConnectors();
    m_pView->NotifyNewVisuals();
    m_pView->InvalidateRect(NULL);

    m_fShutdownSources = true;
    
Cleanup:
    return hr;
}

void CTedTopologyEditor::SetEditable(BOOL fEditable)
{
    m_fEditable = fEditable;

    m_pMoveHandler->SetEditable(fEditable);
    m_pConnectHandler->SetEditable(fEditable);
}

// Mutators - Create New Node //

HRESULT CTedTopologyEditor::AddNode(CTedTopologyNode* pNode)
{
    HRESULT hr;
    
    IFC( AddComponent(pNode) );
    m_pView->InvalidateRect(NULL);
    m_fSaved = false;
    
    if(m_spEventCallback.p)
    {
        m_spEventCallback->NotifyAddedNode(pNode->GetID());
    }

Cleanup:
    return hr;
}

// Mutators - Connection //

// Connect the two nodes both in the topology and the visual representation
HRESULT CTedTopologyEditor::FullConnectNodes(CTedTopologyNode* pOutputNode, long nOutputPin,
                                                CTedTopologyNode* pInputNode, long nInputPin)
{
    assert(pOutputNode != NULL);
    assert(pInputNode != NULL);
    
    HRESULT hr = S_OK;
    
    CVisualPin* pPin = pOutputNode->GetVisualOutputPin(nOutputPin);
    CVisualPin* pOtherPin = pInputNode->GetVisualInputPin(nInputPin);

    assert(pPin != NULL);
    assert(pOtherPin != NULL);

    IFC( ConnectMFNodes(pOutputNode, nOutputPin, pInputNode, nInputPin) );
    
    RemoveOldConnectors(pPin, pOtherPin);

    CVisualConnector* pConn = new CVisualConnector();
    CHECK_ALLOC( pConn );
    m_pView->PTree()->AddVisual(pConn);
    
    pConn->Left() = pPin->GetConnectorPoint();
    pConn->Right() = pOtherPin->GetConnectorPoint();

    pPin->SetConnector(pConn);
    pOtherPin->SetConnector(pConn);

    CTedTopologyConnection* pTedConn = new CTedTopologyConnection(pOutputNode->GetID(), pPin->GetPinId(), 
                            pInputNode->GetID(), pOtherPin->GetPinId());
    CHECK_ALLOC( pTedConn );
    m_Connections.Add(pTedConn);

    pConn->SetData((LONG_PTR) pTedConn);
    
    CTedTopologyNode::TED_NODE_TYPE nodeType = pOutputNode->GetType();
    if(nodeType == CTedTopologyNode::TED_TEE_NODE)
    {
        CTedTeeNode* pTee = (CTedTeeNode*) pOutputNode;
        pTee->NotifyConnection();
        m_pView->PTree()->RouteAllConnectors();
    }

    m_fSaved = false;
    
    if(m_spEventCallback.p)
    {
        m_spEventCallback->NotifyConnection(pOutputNode->GetID(), pInputNode->GetID());
    }

Cleanup:
    return hr;
}

HRESULT CTedTopologyEditor::FullDisconnectNodes(CTedTopologyNode* pOutputNode, long nOutputPin)
{
    assert(pOutputNode != NULL);

    HRESULT hr = S_OK;
    IMFTopologyNode* pNode;
    
    if(pOutputNode->GetType() == CTedTopologyNode::TED_SOURCE_NODE || pOutputNode->GetType() == CTedTopologyNode::TED_OUTPUT_NODE)
    {
        pNode = pOutputNode->GetMFNode(nOutputPin);
        
        (void)pNode->DisconnectOutput(0);
    }
    else
    {
        pNode = pOutputNode->GetMFNode(0);
        (void)pNode->DisconnectOutput(nOutputPin);
    }

    int nEndNodeID;
    RemoveConnectionWithBegin(pOutputNode->GetID(), nOutputPin, &nEndNodeID);
    
    m_fSaved = false;
    
    if(m_spEventCallback.p)
    {
        m_spEventCallback->NotifyDisconnection(pOutputNode->GetID(), nEndNodeID);
    }
    
    return hr;
}

// Mutators - Removal //

HRESULT CTedTopologyEditor::RemoveNode(CTedTopologyNode* pNode, bool fRemoveVisual)
{
    assert(pNode != NULL);
    
    HRESULT hr = S_OK;

    for(size_t i = 0; i < m_Nodes.GetCount(); ++i)
    {
        if(m_Nodes.GetAt(i) == pNode)
        {
            m_Nodes.RemoveAt(i);
            break;
        }
    }

    DWORD nNodes = pNode->GetMFNodeCount();
    for(DWORD i = 0; i < nNodes; ++i)
    {
        IFC( m_spTopology->RemoveNode(pNode->GetMFNode(i)) );
    }

    if(pNode->GetType() == CTedTopologyNode::TED_SOURCE_NODE)
    {
        CTedSourceNode* pSourceNode = (CTedSourceNode*) pNode;
        pSourceNode->ShutdownMFSource();
    }
    else if(pNode->GetType() == CTedTopologyNode::TED_OUTPUT_NODE)
    {
        CTedOutputNode* pOutputNode = (CTedOutputNode*) pNode;
        pOutputNode->ShutdownMFSink();
    }
        
    HWND hWnd = pNode->GetVideoWindow();
    if(hWnd != NULL)
    {
        m_spVideoCallback->ReleaseVideoWindow((LONG_PTR) hWnd);

        // If a video renderer was removed, any D3D managers
        // associated with that video renderer are invalid
        (void)ResetD3DManagers();
    }

    m_fSaved = false;
    if(m_spEventCallback.p)
    {
        m_spEventCallback->NotifyRemovedNode(pNode->GetID());
    }
    
Cleanup:
    m_pView->Unselect();
    if(fRemoveVisual) m_pView->PTree()->RemoveVisual(pNode->GetVisual());
    delete pNode;

    return hr;
}

HRESULT CTedTopologyEditor::RemoveNodeWithVisual(CVisualObject* pVisual)
{
    assert(pVisual != NULL);

    CTedTopologyNode* pFoundNode = NULL;
    HRESULT hr = S_OK;

    // Iterate through the nodes and find the one that has the given visual
    for(size_t i = 0; i < m_Nodes.GetCount(); ++i)
    {
        if(m_Nodes.GetAt(i)->GetVisual()->ContainsVisual(pVisual))
        {
            pFoundNode = m_Nodes.GetAt(i);
        }
    }

    if(pFoundNode)
    {
        hr = RemoveNode(pFoundNode, false);
    }

    return hr;
}

void CTedTopologyEditor::RemoveConnectionWithBegin(int nBeginNodeID, int nBeginPinID, int* pEndNodeID) {
    for(size_t i = 0; i < m_Connections.GetCount(); ++i)
    {
        CTedTopologyConnection* pConn = m_Connections.GetAt(i);
        if(pConn->GetOutputNodeID() == nBeginNodeID && pConn->GetOutputPinID() == nBeginPinID)
        {
            if(pEndNodeID) *pEndNodeID = pConn->GetInputNodeID();
            m_Connections.RemoveAt(i);
            delete pConn;
            break;
        }
    }
}

void CTedTopologyEditor::RemoveConnectionWithEnd(int nEndNodeID, int nEndPinID, int* pBeginNodeID)
{
    for(size_t i = 0; i < m_Connections.GetCount(); ++i)
    {
        CTedTopologyConnection* pConn = m_Connections.GetAt(i);
        if(pConn->GetInputNodeID() == nEndNodeID && pConn->GetInputPinID() == nEndPinID)
        {
            if(pBeginNodeID) *pBeginNodeID = pConn->GetOutputNodeID();
            m_Connections.RemoveAt(i);
            delete pConn;
            break;
        }
    }
}


void CTedTopologyEditor::RemoveOldConnectors(CVisualPin* pPin, CVisualPin* pOtherPin)
{
    assert(pPin != NULL);
    assert(pOtherPin != NULL);
    
    CVisualConnector* pOriginConnector = pPin->GetConnector();
    CVisualConnector* pDestConnector = pOtherPin->GetConnector();

    CTedTopologyNode* pOutputterNode = (CTedTopologyNode*) (pPin->GetData());
    CTedTopologyNode* pAcceptorNode = (CTedTopologyNode*) (pOtherPin->GetData());
    
    if(pOriginConnector && pOriginConnector == pDestConnector) 
    {
        m_pView->PTree()->RemoveVisual(pOriginConnector);
        RemoveConnectionWithBegin(pOutputterNode->GetID(), pPin->GetPinId(), NULL);
    }
    else
    {
        if(pOriginConnector)
        {
            m_pView->PTree()->RemoveVisual(pOriginConnector);
            RemoveConnectionWithBegin(pOutputterNode->GetID(), pPin->GetPinId(), NULL);
        }

        if(pDestConnector)
        {
            m_pView->PTree()->RemoveVisual(pDestConnector);
            RemoveConnectionWithEnd(pAcceptorNode->GetID(), pOtherPin->GetPinId(), NULL);
        }
    }
}

void CTedTopologyEditor::RemoveAllConnectors()
{
    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        RemoveAllConnectorsForComponent(m_Nodes.GetAt(i)->GetVisual());
    }
    
    for(size_t i = 0; i < m_Connections.GetCount(); i++)
    {
        delete m_Connections.GetAt(i);
    }
    m_Connections.RemoveAll();
}

void CTedTopologyEditor::RemoveAllConnectorsForComponent(CVisualComponent* pComponent)
{
    for(size_t i = 0; i < pComponent->GetInputPinCount(); i++)
    {
        CVisualPin* pPin = pComponent->GetInputPinByIndex((DWORD) i);
        
        if(pPin->GetConnector())
        {
            m_pView->PTree()->RemoveVisual(pPin->GetConnector());
        }
    }
    
    for(size_t i = 0; i < pComponent->GetOutputPinCount(); i++)
    {
        CVisualPin* pPin = pComponent->GetOutputPinByIndex((DWORD) i);
        
        if(pPin->GetConnector())
        {
            m_pView->PTree()->RemoveVisual(pPin->GetConnector());
        }
    }    
}

// Mutators - Serialization //

HRESULT CTedTopologyEditor::SaveTopology(const CAtlStringW& fileName)
{
    HRESULT hr = S_OK;
        
    CComPtr<ITedDataSaver> spSaver;
    IFC( CoCreateInstance(CLSID_CXMLDataSaver, NULL, CLSCTX_INPROC_SERVER, IID_ITedDataSaver, (void**) &spSaver) );

    IFC(spSaver->Init(L"TedDocument"));

    for(size_t i = 0; i < m_Nodes.GetCount(); ++i) 
    {
        CAutoPtr<CTedNodeMemo> memo(m_Nodes.GetAt(i)->CreateMemo());
        IFC(memo->Serialize(spSaver));
    }

    for(size_t i = 0; i < m_Connections.GetCount(); ++i)
    {
        CAutoPtr<CTedConnectionMemo> memo(m_Connections.GetAt(i)->CreateMemo());
        IFC(memo->Serialize(spSaver));
    }

    IFC(spSaver->SaveToFile(fileName));

    m_fSaved = true;
    
Cleanup:
    return hr;
}

HRESULT CTedTopologyEditor::LoadTopology(const CAtlStringW& fileName)
{
    HRESULT hr = S_OK;
    LPWSTR strNextObject = NULL;

    CComPtr<ITedDataLoader> spLoader;
    IFC( CoCreateInstance(CLSID_CXMLDataLoader, NULL, CLSCTX_INPROC_SERVER, IID_ITedDataLoader, (void**) &spLoader) );

    NewTopology();

    IFC( spLoader->LoadFromFile(fileName, L"TedDocument") );

    BOOL fHasNextObject;
    IFC( spLoader->HasNextObject(&fHasNextObject) );
    while(fHasNextObject)
    {
        // Get the string that represents the serialized object
        hr = spLoader->GetNextObject(&strNextObject);

        if(hr == S_FALSE)
        {
            hr = S_OK;
            break;
        }
        else
        {
            IFC(hr);
        }

        IFC( LoadTopologyObject(spLoader, strNextObject) );

        CoTaskMemFree(strNextObject);
        strNextObject = NULL;
        
        IFC( spLoader->HasNextObject(&fHasNextObject) );
    }

    // We loaded a bunch of connections; now we need to actually connect the nodes in MF and the visual layer.
    ApplyLoadedConnections();

    m_pView->InvalidateRect(NULL);

    m_fSaved = true;
    
Cleanup:
    if(strNextObject)
    {
        CoTaskMemFree(strNextObject);
    }
    
    return hr;
}

// Non-public Helpers - General //

HRESULT CTedTopologyEditor::AddComponent(CTedTopologyNode* pNode)
{
    assert(pNode != NULL);
    HRESULT hr = S_OK;
        
    m_Nodes.Add(pNode);

    CVisualObject* pVisual = pNode->GetVisual();
    pVisual->SetHandler(m_pMoveHandler);

    m_pView->PTree()->AddVisual(pNode->GetVisual());
    m_pView->NotifyNewVisuals();

    DWORD cNodes = pNode->GetMFNodeCount();
    for(DWORD i = 0; i < cNodes; i++)
    {
        IFC( m_spTopology->AddNode(pNode->GetMFNode(i)) );
    }
    
Cleanup:
    return hr;
}


HRESULT CTedTopologyEditor::LoadTopologyObject(ITedDataLoader* pLoader, const CAtlStringW& strObjName)
{
    HRESULT hr = S_OK;

    if(strObjName == L"CTedSourceMemo")
    {
        CAutoPtr<CTedSourceMemo> memo(new CTedSourceMemo());
        IFC( memo->Deserialize(pLoader) );

        CTedSourceNode* node = new CTedSourceNode;
        CHECK_ALLOC( node );
        IFC( node->Init(memo) );

        IFC( AddComponent(node) );
    }
    else if(strObjName == L"CTedAudioOutputMemo")
    {
        CAutoPtr<CTedAudioOutputMemo> memo(new CTedAudioOutputMemo());
        IFC( memo->Deserialize(pLoader) );

        CTedAudioOutputNode* node = new CTedAudioOutputNode;
        CHECK_ALLOC( node );
        IFC( node->Init(memo) );

        IFC( AddComponent(node) );
    }
    else if(strObjName == L"CTedVideoOutputMemo")
    {
        CAutoPtr<CTedVideoOutputMemo> memo(new CTedVideoOutputMemo());
        HWND hVideoWindow = NULL;

        IFC( memo->Deserialize(pLoader) );
        
        if(m_spVideoCallback.p) IFC( m_spVideoCallback->GetVideoWindow((LONG_PTR*) &hVideoWindow) );
        
        CTedVideoOutputNode* node = new CTedVideoOutputNode;
        CHECK_ALLOC( node );
        IFC( node->Init(hVideoWindow, memo) );

        IFC( AddComponent(node) );
    }
    else if(strObjName == L"CTedTransformMemo")
    {
        CAutoPtr<CTedTransformMemo> memo(new CTedTransformMemo());

        IFC( memo->Deserialize(pLoader) );

        CTedTransformNode* node = new CTedTransformNode;
        CHECK_ALLOC( node );
        IFC( node->Init(memo) );

        IFC( AddComponent(node) );
    }
    else if(strObjName == L"CTedTeeMemo")
    {
        CAutoPtr<CTedTeeMemo> memo(new CTedTeeMemo());

        IFC(memo->Deserialize(pLoader));

        CTedTeeNode* node = new CTedTeeNode;
        CHECK_ALLOC( node );
        IFC( node->Init(memo) );
        
        IFC( AddComponent(node) );
    }
    else if(strObjName == L"CTedConnectionMemo") 
    {
        CAutoPtr<CTedConnectionMemo> memo(new CTedConnectionMemo());

        IFC( memo->Deserialize(pLoader) );
        
        CTedTopologyConnection* pConnection = new CTedTopologyConnection(memo);
        CHECK_ALLOC( pConnection );
        
        m_Connections.Add(pConnection);
    }
    else 
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

Cleanup:
    return hr;
}


HRESULT CTedTopologyEditor::ApplyLoadedConnections() 
{
    HRESULT hr = S_OK;

    for(size_t i = 0; i < m_Connections.GetCount(); i++)
    {
        CTedTopologyConnection* conn = m_Connections.GetAt(i);

        CTedTopologyNode* outputterNode = FindNodeWithID(conn->GetOutputNodeID());
        CTedTopologyNode* acceptorNode = FindNodeWithID(conn->GetInputNodeID());

        if(outputterNode == NULL || acceptorNode == NULL)
        {
            assert(false);
            continue;
        }

        IFC( ConnectMFNodes(outputterNode, conn->GetOutputPinID(), acceptorNode, conn->GetInputPinID()) );

        CVisualPin* outputterPin = outputterNode->GetVisualOutputPin(conn->GetOutputPinID());
        CVisualPin* acceptorPin = acceptorNode->GetVisualInputPin(conn->GetInputPinID());
        
        if(outputterPin == NULL || acceptorPin == NULL)
        {
            assert(false);
            continue;
        }

        m_pView->PTree()->MakeConnector(outputterPin, acceptorPin);
    }    

Cleanup:
    return hr;
}

HRESULT CTedTopologyEditor::ConnectMFNodes(CTedTopologyNode * pUpNode, long nUpPin, 
                            CTedTopologyNode * pDownNode, long nDownPin)
{
    assert(pUpNode != NULL);
    assert(pDownNode != NULL);
    
    HRESULT hr = S_OK;
    CTedTopologyPin UpPin;
    CTedTopologyPin DownPin;
    
    IFC(pUpNode->GetPin(nUpPin, UpPin, false));
    IFC(pDownNode->GetPin(nDownPin, DownPin, true));

    assert(UpPin.PNode() != NULL);
    assert(DownPin.PNode() != NULL);

     IFC(UpPin.PNode()->ConnectOutput(UpPin.Index(), DownPin.PNode(), DownPin.Index()));

Cleanup:
    return hr;
}

bool CTedTopologyEditor::HasMultipleSources()
{
    HRESULT hr = S_OK;
    bool fMultipleSrc = false;
    
    CComPtr<IMFCollection> spSrcNodeCollection;
    IFC( m_spTopology->GetSourceNodeCollection(&spSrcNodeCollection) );

    DWORD cElementCount;
    IFC( spSrcNodeCollection->GetElementCount(&cElementCount) );
    

    IMFMediaSource* pSrc = NULL;
    for(DWORD i = 0; i < cElementCount; i++)
    {
        CComPtr<IUnknown> spSrcNodeUnk;
        IFC( spSrcNodeCollection->GetElement(i, &spSrcNodeUnk) );

        CComPtr<IMFTopologyNode> spSrcNode;
        IFC( spSrcNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**) &spSrcNode) );

        CComPtr<IMFMediaSource> spSrc;
        IFC( spSrcNode->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**) &spSrc) );

        if(pSrc != NULL && spSrc != pSrc)
        {
            fMultipleSrc = true;
            break;
        }

        pSrc = spSrc;
    }

Cleanup:
    return fMultipleSrc;
}

HRESULT CTedTopologyEditor::ResetD3DManagers()
{
    HRESULT hr = S_OK;

    for(DWORD i = 0; i < m_Nodes.GetCount(); i++)
    {
        if(m_Nodes.GetAt(i)->GetType() == CTedTopologyNode::TED_TRANSFORM_NODE)
        {
            IFC( ((CTedTransformNode*) m_Nodes.GetAt(i))->ResetD3DManager() );
        }
    }

Cleanup:
    return hr;
}
    
// Non-public Helpers - Merge //

HRESULT CTedTopologyEditor::MergeBranch(CTedTopologyNode* pBeginTedNode, long nOutputPin,
                                            IMFTopologyNode* pNextNode, long nInputPin,
                                            int xPos, int yPos, bool fMarkNew)
{
    assert(pBeginTedNode != NULL);
    assert(pNextNode != NULL);

    HRESULT hr = S_OK;

    MF_TOPOLOGY_TYPE nodeType;
    CComPtr<IMFTopologyNode> spNode;
    DWORD nNewIndex;
    DWORD outputCount;
    
    IFC( pNextNode->GetNodeType(&nodeType) );
    IFC( pNextNode->GetOutputCount(&outputCount) );
    
    TOPOID idNode;
    IFC( pNextNode->GetTopoNodeID(&idNode) );
    
    CTedTopologyNode* pTedNode = FindNodeByTopoID(idNode);
    if(NULL == pTedNode)
    {    
   
        if(MF_TOPOLOGY_TRANSFORM_NODE == nodeType)
        {
            IFC( MergeTransformNode(pBeginTedNode, nOutputPin, pNextNode, nInputPin, xPos, yPos, fMarkNew, &pTedNode) );
        }
        else if(MF_TOPOLOGY_OUTPUT_NODE == nodeType)
        {
            IFC( MergeOutputNode(pBeginTedNode, nOutputPin, pNextNode, nInputPin, xPos, yPos, &pTedNode) );
        }
        else if(MF_TOPOLOGY_TEE_NODE == nodeType)
        {
            TOPOID nodeID;
            IFC( pNextNode->GetTopoNodeID(&nodeID) );
        
            CTedTeeNode* pTeeNode = (CTedTeeNode*) FindNodeByTopoID(nodeID);
            if(pTeeNode) RemoveNode(pTeeNode);
    
            pTeeNode = new CTedTeeNode();
            CHECK_ALLOC( pTeeNode );
            pTeeNode->Init();
            pTedNode = pTeeNode;
 
        }
    }

    // If we did not generate a new node, use this iter's upstream node as the next iter's upstream node
    if(!pTedNode)
    {
        pTedNode = pBeginTedNode;
    }
    else
    {
        pTedNode->CopyAttributes(pNextNode);
    }
    
    pTedNode->GetVisual()->Move(xPos, yPos);

    xPos += int(pTedNode->GetVisual()->Rect().w() + MARGIN_SIZE);
    yPos -= int((pTedNode->GetVisual()->Rect().h() / 2 + MARGIN_SIZE) * (outputCount - 1));

    if(yPos < 0) yPos = 0;

    for(DWORD i = 0; i < outputCount; i++)
    {
        spNode.Release();
        IFC(pNextNode->GetOutput(i, &spNode, &nNewIndex));

        // If a new node was not generated, use the upstream node's output pin
        if(pTedNode == pBeginTedNode)
        {
            IFC( MergeBranch(pTedNode, nOutputPin, spNode, nNewIndex, xPos, yPos, fMarkNew) );
        }
        else
        {
            IFC( MergeBranch(pTedNode, i, spNode, nNewIndex, xPos, yPos, fMarkNew) );
        }

        yPos += int(pTedNode->GetVisual()->Rect().h() + MARGIN_SIZE);
    }

    if(pBeginTedNode != pTedNode)
    {
        if(pTedNode->GetMFNodeCount() > 1)
        {
            TOPOID tidNodeID;
            IFC( pNextNode->GetTopoNodeID(&tidNodeID) );

            // Find which pin corresponds to the TopoID of the current node
            for(DWORD i = 0; i < pTedNode->GetMFNodeCount(); i++)
            {
                TOPOID tidNodeID2;
                IFC( pTedNode->GetMFNode(i)->GetTopoNodeID(&tidNodeID2) );

                if(tidNodeID == tidNodeID2)
                {
                    nInputPin = i;
                    break;
                }
            }
        }
        
        hr = FullConnectNodes(pBeginTedNode, nOutputPin, pTedNode, nInputPin);
    }

Cleanup:
    return hr;
}

HRESULT CTedTopologyEditor::MergeTransformNode(CTedTopologyNode* pBeginTedNode, long nOutputPin,
                                                    IMFTopologyNode* pNextNode, long nInputPin,
                                                    int xPos, int yPos, bool fMarkNew, CTedTopologyNode** ppNewNode)
{
    assert(pBeginTedNode != NULL);
    assert(pNextNode != NULL);
    assert(ppNewNode != NULL);
    
    HRESULT hr = S_OK;
    HRESULT hrTransform, hrActivate;
    CTedTransformNode* pTedNode = NULL;
    CComPtr<IUnknown> spTransformUnk;
    CComPtr<IMFTransform> spTransform;
    CComPtr<IMFActivate> spActivate;
  
    GUID gidTransform = GUID_NULL;
    pNextNode->GetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, &gidTransform);

    DMOInfo* pDMOInfo = DMOInfo::GetSingleton();
    CAtlStringW strLabel = pDMOInfo->GetCLSIDName(gidTransform);

    if(strLabel.IsEmpty()) 
    {
        // Cannot find a registered DMO with this CLSID, so just convert the CLSID
        // directly to a string and display it to the user
        LPOLESTR strClsid = NULL;
        StringFromCLSID(gidTransform, &strClsid);

        strLabel = OLE2W(strClsid);

        CoTaskMemFree(strClsid);
    }

    
    hr = pNextNode->GetObject(&spTransformUnk);
    if(FAILED(hr))
    {
        if(gidTransform == GUID_NULL)
        {
            // Probably a decrypter; just remove it
            *ppNewNode = NULL;
            return S_OK;
        }
        else
        {
            IFC( hr );
        }
    }

    hrTransform = spTransformUnk->QueryInterface(IID_IMFTransform, (void**) &spTransform);
    if(FAILED(hrTransform))
    {
        if(gidTransform == GUID_NULL)
        {
            *ppNewNode = NULL;
            return S_OK;
        }
        else
        {
            hrActivate = spTransformUnk->QueryInterface(IID_IMFActivate, (void**) &spActivate);
            if(FAILED(hrActivate))
            {
                IFC( hrActivate );
            }
        }
    }

    if(SUCCEEDED(hrTransform))
    {
        if(gidTransform == GUID_NULL)
        {
            CTedTransformNode* pTransformNode = new CTedTransformNode();
            CHECK_ALLOC( pTransformNode );
        pTransformNode->Init(spTransform, GUID_NULL, LoadAtlString(IDS_MFT_UNKNOWN), true);
            pTedNode = pTransformNode;
            IFC( AddComponent(pTedNode) );

	        pTedNode->CopyMediaTypes(pNextNode);
            *ppNewNode = pTedNode;
            return hr;
        }

        TOPOID tidNodeID;
        pNextNode->GetTopoNodeID(&tidNodeID);
        pTedNode = (CTedTransformNode*) FindNodeByTopoID(tidNodeID);

        if(NULL == pTedNode)
        {
            CTedTransformNode* pTransformNode = new CTedTransformNode();
            CHECK_ALLOC( pTransformNode );
            pTransformNode->Init(spTransform, gidTransform, strLabel, fMarkNew);
            pTedNode = pTransformNode;
            IFC( AddComponent(pTedNode) );
        }
    }
    else
    {
        LPWSTR szFriendlyName = NULL;
        if(SUCCEEDED( spActivate->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &szFriendlyName, NULL) ))
        {
            strLabel = szFriendlyName;
            CoTaskMemFree(szFriendlyName);
        }

        CTedTransformNode* pTransformNode = new CTedTransformNode();
        CHECK_ALLOC( pTransformNode );
        pTransformNode->Init(spActivate, strLabel, fMarkNew);
        pTedNode = pTransformNode;
        IFC( AddComponent(pTedNode) );
    }
    
    *ppNewNode = pTedNode;

Cleanup:
    return hr;
}

HRESULT CTedTopologyEditor::MergeOutputNode(CTedTopologyNode* pBeginTedNode, long nOutputPin,
                                                IMFTopologyNode* pNextNode, long nInputPin,
                                                int xPos, int yPos, CTedTopologyNode** ppNewNode)
{
    assert(pBeginTedNode != NULL);
    assert(pNextNode != NULL);
    assert(ppNewNode != NULL);
    
    HRESULT hr = S_OK;
    CTedTopologyNode* pTedNode = NULL;
    HWND hWndOld = NULL;

    TOPOID tidNodeID;    
    IFC( pNextNode->GetTopoNodeID(&tidNodeID) );        

    pTedNode = FindNodeByTopoID(tidNodeID);
    if(pTedNode)
    {
        RemoveNode(pTedNode);
    }
    
    {
        CComPtr<IUnknown> spStreamSinkUnk;
        CComPtr<IMFStreamSink> spStreamSink;
        CComPtr<IMFActivate> spActivate;
        CComPtr<IMFMediaType> spPrefType;
        bool fIsEVR, fIsSAR;
        
        IFC( IsEVR(pNextNode, &fIsEVR) );
        IFC( IsSAR(pNextNode, &fIsSAR) );

        IFC( pNextNode->GetObject(&spStreamSinkUnk) );
        HRESULT hrHasStreamSink = spStreamSinkUnk->QueryInterface(IID_IMFStreamSink, (void**) &spStreamSink);
        HRESULT hrHasActivate = spStreamSinkUnk->QueryInterface(IID_IMFActivate, (void**) &spActivate );

        if(fIsEVR)
        {
            CTedVideoOutputNode* pVideoNode = new CTedVideoOutputNode();
            CHECK_ALLOC( pVideoNode );
            CAtlString str = LoadAtlString(IDS_VIDEO_RENDERER);

            if(hrHasStreamSink == S_OK)
            {
                CComPtr<IMFGetService> spGetService;
                CComPtr<IMFVideoDisplayControl> spDisplayControl;
                HWND hCurrentWnd = NULL;
                
                IFC( spStreamSink->QueryInterface(IID_IMFGetService, (void**) &spGetService) );
                IFC( spGetService->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**) &spDisplayControl) );
                IFC( spDisplayControl->GetVideoWindow(&hCurrentWnd) );
                
                HWND hVideoWnd = hCurrentWnd;
                if(hCurrentWnd == NULL && m_spVideoCallback.p != NULL)
                {
                    IFC( m_spVideoCallback->GetVideoWindow((LONG_PTR*) &hVideoWnd) );
                    IFC( spDisplayControl->SetVideoWindow(hVideoWnd) );
                }
                
                pVideoNode->Init(str, hVideoWnd, spStreamSink);
            }
            else
            {
                HWND hVideoWnd = NULL;
                if(m_spVideoCallback.p != NULL)
                {
                    IFC( m_spVideoCallback->GetVideoWindow((LONG_PTR*) &hVideoWnd) );
                }
                
                pVideoNode->Init(str, hVideoWnd);
            }
            
            IFC( pVideoNode->CopyAttributes(pNextNode) );
            
            pTedNode = pVideoNode;
            IFC( AddComponent(pTedNode) );
        }
        else if(fIsSAR)
        {
            CTedAudioOutputNode* pAudioNode = new CTedAudioOutputNode();
            CHECK_ALLOC( pAudioNode );

            CAtlString str = LoadAtlString(IDS_AUDIO_RENDERER);
            if(hrHasStreamSink == S_OK)
            {
                pAudioNode->Init(str, spStreamSink);
            }
            else
            {
                pAudioNode->Init(str);
            }
            
            IFC( pAudioNode->CopyAttributes(pNextNode) );
            
            pTedNode = pAudioNode;
            IFC( AddComponent(pTedNode) );
        }
        else if(hrHasStreamSink == S_OK)
        {
            CComPtr<IMFMediaSink> spSink;

            IFC( spStreamSink->GetMediaSink(&spSink) );

            CAtlString str = LoadAtlString(IDS_CUSTOM_SINK);
    
            CTedCustomOutputNode* pCustomNode = new CTedCustomOutputNode();
            CHECK_ALLOC( pCustomNode );
            pCustomNode->Init(spSink, str);
            
            IFC( pCustomNode->CopyAttributes(pNextNode) );
            
            pTedNode = pCustomNode;
            IFC( AddComponent(pTedNode) );
            
        }
        else if(hrHasActivate == S_OK)
        {
            CTedOutputNode* pActivateNode = new CTedOutputNode();
            CHECK_ALLOC( pActivateNode );
            pActivateNode->Init(spActivate, L"Sink Activate");

            IFC( pActivateNode->CopyAttributes(pNextNode) );

            pTedNode = pActivateNode;
            IFC( AddComponent(pTedNode) );
        }
        else
        {
            // Ignore this node if it has no stream sink
        }
    }

    *ppNewNode = pTedNode;


Cleanup:
    return hr;
}

// Ugly hack to figure out if a given topology node is an EVR stream -- check for IMFVideoRenderer interface
HRESULT CTedTopologyEditor::IsEVR(IMFTopologyNode* pNode, bool* fIsEVR)
{
    if(NULL == fIsEVR)
    {
        return E_POINTER;
    }

    *fIsEVR = false;

    HRESULT hr = S_OK;
    CComPtr<IUnknown> spStreamSinkUnk;
    CComPtr<IMFStreamSink> spStreamSink;
    CComPtr<IMFMediaSink> spSink;
    CComPtr<IMFVideoRenderer> spVR;

    GUID gidActivateID;
    hr = pNode->GetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, &gidActivateID);

    if(SUCCEEDED(hr) && gidActivateID == CLSID_VideoRenderActivate)
    {
        *fIsEVR = true;
        return S_OK;
    }
    
    IFC( pNode->GetObject(&spStreamSinkUnk) );
    IFC( spStreamSinkUnk->QueryInterface(IID_IMFStreamSink, (void**) &spStreamSink) );
    IFC( spStreamSink->GetMediaSink(&spSink) );
    hr = spSink->QueryInterface(IID_IMFVideoRenderer, (void**) &spVR);

    *fIsEVR = SUCCEEDED(hr);
  
Cleanup:
    return S_OK;
}

// Ugly hack to figure out if a given topology node is a SAR stream -- check for IMFAudioStreamVolume interface
HRESULT CTedTopologyEditor::IsSAR(IMFTopologyNode* pNode, bool* fIsSAR)
{
    if(NULL == fIsSAR)
    {
        return E_POINTER;
    }

    *fIsSAR = false;
    
    HRESULT hr = S_OK;
    CComPtr<IUnknown> spStreamSinkUnk;
    CComPtr<IMFStreamSink> spStreamSink;
    CComPtr<IMFAudioStreamVolume> spASV;

    GUID gidActivateID;
    hr = pNode->GetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, &gidActivateID);

    if(SUCCEEDED(hr) && gidActivateID == CLSID_AudioRenderActivate)
    {
        *fIsSAR = true;
        return S_OK;
    }
    
    IFC( pNode->GetObject(&spStreamSinkUnk) );
    IFC( spStreamSinkUnk->QueryInterface(IID_IMFStreamSink, (void**) &spStreamSink) );
    hr = spStreamSink->QueryInterface(IID_IMFAudioStreamVolume, (void**) &spASV);

    *fIsSAR = SUCCEEDED(hr);

Cleanup:
    return S_OK;
}

HRESULT CTedTopologyEditor::RemoveOrphanTransforms()
{
    HRESULT hr = S_OK;
    
    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        CTedTopologyNode* pNode = m_Nodes.GetAt(i);
        
        if(pNode->GetType() == CTedTopologyNode::TED_TRANSFORM_NODE && pNode->IsOrphaned())
        {
            IFC( RemoveNode(pNode) );
            --i;
        }
    }
    
Cleanup:
    return hr;
}

HRESULT CTedTopologyEditor::RemoveNodesNotInTopology(IMFTopology* pTopology)
{
    HRESULT hr = S_OK;
    
    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        CTedTopologyNode* pTedNode = m_Nodes.GetAt(i);
        CComPtr<IMFTopologyNode> spNode;
        TOPOID tidTedNode;
        
        IFC( pTedNode->GetMFNode(0)->GetTopoNodeID(&tidTedNode) );
        if( FAILED( pTopology->GetNodeByID(tidTedNode, &spNode) ) )
        {
            IFC( RemoveNode(pTedNode) );
            i--;
        }
    }
    
Cleanup:
    return hr;
}

// Non-public Helpers - Find //

CTedTopologyNode* CTedTopologyEditor::FindNodeWithID(int id)
{
    for(size_t i = 0; i < m_Nodes.GetCount(); i++) 
    {
        if(m_Nodes.GetAt(i)->GetID() == id) return m_Nodes.GetAt(i);
    }

    return NULL;
}

CTedSourceNode* CTedTopologyEditor::FindSourceNode(TOPOID nodeID)
{
    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        if(m_Nodes.GetAt(i)->GetType() == CTedTopologyNode::TED_SOURCE_NODE)
        {
            CTedSourceNode* pSourceNode = (CTedSourceNode*) m_Nodes.GetAt(i);

            DWORD dwIndex = pSourceNode->GetIndexOf(nodeID);

            if(dwIndex != (DWORD) -1)
            {
                return pSourceNode;
            }
        }
    }

    return NULL;
}

CTedTopologyNode* CTedTopologyEditor::FindNodeByTopoID(TOPOID nodeID)
{
    for(size_t i = 0; i < m_Nodes.GetCount(); i++)
    {
        for(DWORD j = 0; j < m_Nodes.GetAt(i)->GetMFNodeCount(); j++)
        {
            TOPOID currNodeID;
            m_Nodes.GetAt(i)->GetNodeID(j, currNodeID);

            if(currNodeID == nodeID)
            {
                return m_Nodes.GetAt(i);
            }
        }
    }

    return NULL;
}
