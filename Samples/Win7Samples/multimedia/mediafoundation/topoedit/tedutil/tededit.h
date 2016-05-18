// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __TEDEDIT__
#define __TEDEDIT__

#include "tedutil.h"
#include "mftransformwrapper.h"
#include "mfstreamsinkwrapper.h"
#include "tedvis.h"

class CVisualObject;
class CVisualConnector;
class CVisualComponent;
class CVisualTree;
class CTopoViewerWindow;
class CCommandHandler;
class CTedTopologyEditor;
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

///////////////////////////////////////////////////////////////////////////////
// points to one input / output of the MF topology
class CTedTopologyPin
{
public:
    CTedTopologyPin();
    CTedTopologyPin(IMFTopologyNode * pNode, DWORD nIndex);
    ~CTedTopologyPin();
    
    void SetPNode(IMFTopologyNode* pNode) { m_pNode = pNode; }
    void SetIndex(DWORD nIndex) { m_nIndex = nIndex; }

    IMFTopologyNode * PNode() { return m_pNode; }
    DWORD Index() { return m_nIndex; }
    
private:    
    CComPtr<IMFTopologyNode> m_pNode;
    DWORD m_nIndex;
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedTopologyConnection {
public:
    CTedTopologyConnection(int nOutputNodeID, int nOutputPinID, int nInputNodeID, int nInputPinID);
    CTedTopologyConnection(CTedConnectionMemo* pMemo);

    int GetOutputNodeID() const;
    int GetOutputPinID() const;
    int GetInputNodeID() const;
    int GetInputPinID() const;

    CTedConnectionMemo* CreateMemo() const;
    
private:
    int m_nOutputNodeID;
    int m_nOutputPinID;
    int m_nInputNodeID;
    int m_nInputPinID;
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedTopologyNode
{
public:
    CTedTopologyNode();
    virtual ~CTedTopologyNode();
    
    enum TED_NODE_TYPE
    {
        TED_SEQUENCER_NODE,
        TED_SOURCE_NODE,
        TED_TRANSFORM_NODE,
        TED_OUTPUT_NODE,
        TED_TEE_NODE
    };
    
    // Accessors //
    virtual HWND GetVideoWindow() const;
    CVisualComponent* GetVisual() const;
    virtual CVisualPin* GetVisualInputPin(int pinID);
    virtual CVisualPin* GetVisualOutputPin(int pinID);
    int GetID() const;
    HRESULT GetNodeID(DWORD dwIndex, TOPOID& NodeID);    
    CAtlStringW GetLabel() const;
    bool IsOrphaned();

    // Abstract accessors //
    virtual DWORD GetMFNodeCount() const = 0 ;
    virtual IMFTopologyNode* GetMFNode(DWORD nIndex) const = 0;
    virtual HRESULT GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin) = 0;
    virtual TED_NODE_TYPE GetType() = 0;
    virtual CTedNodeMemo* CreateMemo() const = 0;
    
    // Mutators //
    HRESULT CopyAttributes(IMFTopologyNode* pNode, DWORD dwIndex = 0);
   
protected:
    HRESULT Init(const CAtlStringW& label, bool fAutoInserted = false);
    HRESULT InitContainer(const CAtlStringW& label, bool fAutoInserted = false);
    void Init(CTedNodeMemo* pMemo);
    void InitContainer(CTedNodeMemo* pMemo);

    // Non-public helpers //
    void AddPin(bool bIsInput, const CAtlStringW& strLabel, int nID);
    HRESULT PostInitFromMemoCopyAttributes(CTedNodeMemo* pMemo);

private:
    CVisualComponent* m_pVisual;
    CAtlStringW m_strLabel;
    int m_nID;
    bool m_fErrorNode;

    static int ms_nNextID;
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedSourceNode : public CTedTopologyNode
{
public:
    // Initialization //
    CTedSourceNode();
    virtual ~CTedSourceNode();

    HRESULT Init(const CAtlStringW& strSourceURL, const CAtlStringW& label);
    HRESULT Init(IMFMediaSource* pSource, const CAtlStringW& label);
    HRESULT Init(CTedSourceMemo* pMemo);
    HRESULT Init(CTedSourceNode* pNode);
    HRESULT Init(const CAtlStringW& strSourceURL, const CAtlStringW& label, IMFMediaSource* pSource);
    HRESULT InitIndirect(const CAtlStringW& strSourceURL, CAtlArray<IMFTopologyNode*>& sourceNodes);

    // Accessors //
    HRESULT GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin);
    bool IsProtected() const;
    CTedNodeMemo* CreateMemo() const;
    virtual DWORD GetMFNodeCount() const;
    virtual IMFTopologyNode* GetMFNode(DWORD nIndex) const;
    CAtlStringW GetURL() { return m_strSourceURL; }
    TED_NODE_TYPE GetType();
    DWORD GetIndexOf(TOPOID nodeID);
    virtual CVisualPin* GetVisualInputPin(int pinID);
    virtual CVisualPin* GetVisualOutputPin(int pinID);
    
    IMFMediaSource* GetMFSource() 
    { 
        return m_spSource; 
    }
    bool IsInitializedFromMFSource() 
    { 
        return m_bInitializedFromMFSource; 
    }

    // Mutators //
    static void ReleaseResolver();
    void ShutdownMFSource();
    HRESULT CopyAttributes(IMFTopologyNode* pNode, DWORD dwIndex = 0);
    void FlagExternalShutdownRequired();
    HRESULT SelectValidStreams();
    
protected:
    // Non-public Helpers //
    HRESULT CreateMFSource();
    HRESULT CreateMFCaptureSource(IMFMediaSource* pSource);
    HRESULT InitMFSourceNode(CComPtr<IMFTopologyNode> spNode, int nPinIndex);
    HRESULT CreateSourceNodes();
    HRESULT InitIsProtected();

private:
    CComPtr<IMFMediaSource> m_spSource;
    CComPtr<IMFPresentationDescriptor> m_spPD;
    
    CAtlStringW m_strSourceURL;
    bool m_fIsProtected;

    // array of source nodes
    CAtlArray<CComPtr<IMFTopologyNode> > m_Nodes;

    bool m_fExternalShutdownRequired;
    
    static CComPtr<IMFSourceResolver> ms_spResolver;
    static bool m_bIsResolverCreated;
    bool m_bInitializedFromMFSource;
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedOutputNode : public CTedTopologyNode
{
public:
    // Initialization //
    CTedOutputNode();
    virtual ~CTedOutputNode();
    
    HRESULT Init(IMFActivate * pActivate, const CAtlStringW& label);
    HRESULT Init(IMFMediaSink* pSink, const CAtlStringW& label);
    HRESULT Init(IMFActivate* pActivate, CTedOutputMemo* pMemo);
    HRESULT Init(IMFMediaSink* pSink, CTedOutputMemo* pMemo);
    HRESULT Init(IMFStreamSink* pStreamSink, const CAtlStringW& label);
    
    // Accessors //
    HRESULT GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin);
    virtual DWORD GetMFNodeCount() const;
    virtual IMFTopologyNode* GetMFNode(DWORD nIndex) const;
    TED_NODE_TYPE GetType();
    CVisualPin* GetVisualInputPin(int pinID);
    CVisualPin* GetVisualOutputPin(int pinID);
    virtual CTedNodeMemo* CreateMemo() const { return NULL; }
    
    // Mutators //
    HRESULT WrapStreamSink(CLogger* pLogger);
    void ShutdownMFSink();
    void FlagExternalShutdownRequired();
    
protected:
    // Non-public Helepers //
    HRESULT InitFromSink(IMFMediaSink* pSink);

private:
    // array of output nodes
    CAtlArray<CComPtr<IMFTopologyNode> > m_arrStreamSinkNodes;
    bool m_fExternalShutdownRequired;
};


///////////////////////////////////////////////////////////////////////////////
//
class CTedAudioOutputNode : public CTedOutputNode
{
public:
    // Initialization //
    HRESULT Init(const CAtlStringW& label);
    HRESULT Init(CTedAudioOutputMemo* pMemo);
    HRESULT Init(const CAtlStringW& label, IMFStreamSink* pStreamSink);

    // Accessors //
    CTedNodeMemo* CreateMemo() const;
private:
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedVideoOutputNode : public CTedOutputNode
{
public:
    // Initialization //
    HRESULT Init(const CAtlStringW& label, HWND hVideoOutWindow);
    HRESULT Init(HWND hVideoOutWindow, CTedVideoOutputMemo* pMemo);
    HRESULT Init(const CAtlStringW& label, HWND hVideoOutWindow, IMFStreamSink* pStreamSink);

    // Accessors //
    virtual HWND GetVideoWindow() const;
    CTedNodeMemo* CreateMemo() const;

private:
    HWND m_hWnd;
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedCustomOutputNode : public CTedOutputNode
{
public:
    // Initialization //
    HRESULT Init(GUID gidCustomSinkID, const CAtlStringW& label);
    HRESULT Init(IMFMediaSink* pSink, const CAtlStringW& label);
    HRESULT Init(CTedCustomOutputMemo* pMemo);
    
    // Accessors //
    CTedNodeMemo* CreateMemo() const;
private:
    GUID m_gidCustomSinkID;
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedTransformNode : public CTedTopologyNode
{
public:
    // Initialization //
    HRESULT Init(CLSID dmoCLSID, const CAtlStringW& label, bool fAutoInserted = false);
    HRESULT Init(IMFTransform* pTransform, CLSID dmoCLSID, const CAtlStringW& label, bool fAutoInserted = false);
    HRESULT Init(IMFActivate* pTransformActivate, const CAtlStringW& label, bool fAutoInserted = false);
    HRESULT Init(CTedTransformMemo* pMemo);

    // Accessors //
    HRESULT GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin);
    CTedNodeMemo* CreateMemo() const;
    virtual DWORD GetMFNodeCount() const;
    virtual IMFTopologyNode* GetMFNode(DWORD nIndex) const;
    bool HasSameTransform(const CLSID& transformCLSID);
    TED_NODE_TYPE GetType();

    // Mutators //
    HRESULT CopyMediaTypes(IMFTopologyNode* pOldNode);
    HRESULT WrapTransform(CLogger* pLogger);
    HRESULT ResetD3DManager();
        
protected:
    //Non-public Helpers //
    HRESULT InitTransform(CLSID dmoCLSID);
    HRESULT InitTransform(IMFTransform* pTransform);
    HRESULT InitTransform(IMFActivate* pTransformActivate);
    
private:
    CLSID m_clsid;
    CComPtr<IMFTopologyNode> m_spTransformNode;
    CComPtr<IMFTransform> m_spTransform;
};

///////////////////////////////////////////////////////////////////////////////
//
class CTedTeeNode : public CTedTopologyNode
{
public:
    // Initialization //
    HRESULT Init();
    HRESULT Init(CTedTeeMemo* pMemo);

    // Accessors //
    HRESULT GetPin(DWORD nInput, CTedTopologyPin & Pin, bool inputPin);
    CTedNodeMemo* CreateMemo() const;
    virtual DWORD GetMFNodeCount() const;
    virtual IMFTopologyNode* GetMFNode(DWORD nIndex) const;
    TED_NODE_TYPE GetType();
    
    // Mutators //
    void NotifyConnection();
    
private:
    CComPtr<IMFTopologyNode> m_spTeeNode;
    DWORD m_nNextOutputIndex;
};

///////////////////////////////////////////////////////////////////////////////
//

class CTedEditorVisualObjectEventHandler : public CVisualObjectEventHandler
{
public:
    CTedEditorVisualObjectEventHandler(CTedTopologyEditor* pEditor);
    
    void NotifyObjectDeleted(CVisualObject* pVisualObj);

private:
    CTedTopologyEditor* m_pEditor;
};

class CTedNodeCreator
{
public:
    ~CTedNodeCreator();

    static CTedNodeCreator* GetSingleton();
    
    HRESULT CreateSource(const CAtlStringW& strSourceURL, IMFMediaSource* pFromSource, CTedSourceNode** ppSourceNode);
    HRESULT CreateCaptureSource(IMFMediaSource* pSource, CTedSourceNode** ppSourceNode);
    HRESULT CreateSAR(CTedAudioOutputNode** ppSAR);
    HRESULT CreateEVR(ITedVideoWindowHandler* pVideoHandler, CTedVideoOutputNode** ppEVR);
    HRESULT CreateCustomSink(IMFMediaSink* pSink, CTedCustomOutputNode** ppSinkNode);
    HRESULT CreateCustomSink(GUID gidCustomSinkID, CTedCustomOutputNode** ppSinkNode);
    HRESULT CreateTransform(CLSID clsidDMO, const CAtlStringW& name, CTedTransformNode** ppTransformNode);
    HRESULT CreateTransform(IMFActivate* pTransformActivate, CTedTransformNode** ppTransformNode);
    HRESULT CreateTee(CTedTeeNode** ppTee);

protected:
    CTedNodeCreator();
private:
    static CTedNodeCreator m_Singleton;
};

///////////////////////////////////////////////////////////////////////////////
// provides all functionality for editing the topology
class CTedTopologyEditor
{
public:
    // Initialization //
    CTedTopologyEditor();
    ~CTedTopologyEditor();
    HRESULT Init(ITedVideoWindowHandler* pVideoCallback, ITedPropertyController* pPropertyCallback, ITedTopoEventHandler* pEventCallback, CTopoViewerWindow * pView);
    HRESULT NewTopology();

    // Accessors - General //
    bool IsSaved() { return m_fSaved; }
    HRESULT GetTopology(IMFTopology** ppTopo, BOOL* pfIsProtected);
    bool HasSource();
    DWORD GetNodeCount();
    CTedTopologyNode* GetNode(DWORD dwIndex);
    
    // Accessors - Find //
    CTedTopologyConnection* FindDownstreamConnection(int nBeginNodeID, int nBeginPinID);
    CTedTopologyConnection* FindUpstreamConnection(int nEndNodeID, int nEndPinID);
    CTedTopologyNode* FindNode(int nodeID);

    // Mutators - General //
    HRESULT MergeTopology(IMFTopology* pTopo);
    HRESULT SpyNodeWithVisual(CVisualObject* pVisual);
    HRESULT ShowTopology(IMFTopology* pTopo, LPCWSTR szSourceURL);
    void SetEditable(BOOL fEditable);
   
    // Mutators - Create New Node //
    HRESULT AddNode(CTedTopologyNode* pNode);

    // Mutators - Connection //
    HRESULT FullConnectNodes(CTedTopologyNode* pOutputNode, long nOutputPin, CTedTopologyNode* pInputNode, long nInputPin);
    HRESULT FullDisconnectNodes(CTedTopologyNode* pOutputNode, long nOutputPin);

    // Mutators - Removal //
    HRESULT RemoveNode(CTedTopologyNode* pNode, bool fRemoveVisual = true);
    HRESULT RemoveNodeWithVisual(CVisualObject* pVisual);
    void RemoveConnectionWithBegin(int nBeginNodeID, int nBeginPinID, int* pEndNodeID);
    void RemoveConnectionWithEnd(int nEndNodeID, int nEndPinID, int* pBeginNodeID);
    void RemoveOldConnectors(CVisualPin* pPin, CVisualPin* pOtherPin);
    void RemoveAllConnectors();
    void RemoveAllConnectorsForComponent(CVisualComponent* pComponent);
   
    // Mutators - Serialization //
    HRESULT SaveTopology(const CAtlStringW& fileName);
    HRESULT LoadTopology(const CAtlStringW& fileName);

protected:
    // Non-public Helpers - General //
    HRESULT AddComponent(CTedTopologyNode * pNode);
    HRESULT LoadTopologyObject(ITedDataLoader* pLoader, const CAtlStringW& strObjName);
    HRESULT ApplyLoadedConnections();
    HRESULT ConnectMFNodes(   CTedTopologyNode * pUpNode, 
                                            long nUpPin, 
                                            CTedTopologyNode * pDownNode, 
                                            long nDownPin);
    bool HasMultipleSources();
    HRESULT ResetD3DManagers();

    // Non-public Helpers - Merge //
    HRESULT MergeBranch(CTedTopologyNode* pBeginTedNode, long nOutputPin, IMFTopologyNode* pNextNode, long nInputPin, int xPos, int yPos, bool fMarkNew);
    HRESULT MergeTransformNode(CTedTopologyNode* pBeginTedNode, long nOutputPin, IMFTopologyNode* pNextNode, long nInputPin, int xPos, int yPos, bool fMarkNew, CTedTopologyNode** ppNewNode);
    HRESULT MergeOutputNode(CTedTopologyNode* pBeginTedNode, long nOutputPin, IMFTopologyNode* pNextNode, long nInputPin, int xPos, int yPos, CTedTopologyNode** ppNewNode);
    HRESULT IsEVR(IMFTopologyNode* pNode, bool* fIsEVR);
    HRESULT IsSAR(IMFTopologyNode* pNode, bool* fIsSAR);
    HRESULT RemoveOrphanTransforms();
    HRESULT RemoveNodesNotInTopology(IMFTopology* pTopology);

    // Non-public Helpers - Find //
    CTedTopologyNode* FindNodeWithID(int id);
    CTedSourceNode* FindSourceNode(TOPOID nodeID);
    CTedTopologyNode* FindNodeByTopoID(TOPOID nodeID);
   
private:
    const static int MARGIN_SIZE;

    CComPtr<ITedVideoWindowHandler> m_spVideoCallback;
    CComPtr<ITedPropertyController> m_spPropertyCallback;
    CComPtr<ITedTopoEventHandler> m_spEventCallback;

    // set if we are viewing full topology
    BOOL m_fReadOnly;

    int m_nProtectedSourceCount;
    
    // pointer to view
    CTopoViewerWindow* m_pView;

    // topology for edit
    CComPtr<IMFTopology> m_spTopology;

    // array of nodes 
    CAtlArray<CTedTopologyNode*> m_Nodes;
    CAtlArray<CTedTopologyConnection*> m_Connections;
    CAtlStringW m_strSourceURL;

    CMoveComponentHandler * m_pMoveHandler;
    CConnectPinHandler * m_pConnectHandler;

    CAtlArray<CLogger*> m_arrLoggers;

    bool m_fSaved;
    bool m_fShutdownSources;
    BOOL m_fEditable;
    
    CComPtr<IMFSequencerSource> m_spSequencer;
    MFSequencerElementId m_LastSeqID;

    CTedEditorVisualObjectEventHandler m_VisualEventHandler;
};

#endif

