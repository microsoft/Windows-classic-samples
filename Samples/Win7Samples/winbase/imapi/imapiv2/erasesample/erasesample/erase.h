/*--

Copyright (C) Microsoft Corporation, 2006

Main header file

--*/

#pragma once
#include "stdafx.h"
#include "resource.h"

class CMsftEraseSample;

// the list of media types supported...
static const IMAPI_MEDIA_PHYSICAL_TYPE g_EraseSupportedMediaTypes[] = 
{
    IMAPI_MEDIA_TYPE_CDRW,
    IMAPI_MEDIA_TYPE_DVDRAM,
    IMAPI_MEDIA_TYPE_DVDPLUSRW,
    IMAPI_MEDIA_TYPE_DVDDASHRW,
    IMAPI_MEDIA_TYPE_DISK,
};
static const ULONG g_EraseSupportedMediaTypesCount = RTL_NUMBER_OF(g_EraseSupportedMediaTypes);

// This structure is used locally during "standard" erase
// when using READ_DISC_INFO to estimate time to completion
typedef struct _ERASE_UPDATE_CALLBACK_CONTEXT {

    CMsftEraseSample*   Erase;    
    CTaskTimeEstimator* BlankTime;

    // need to track old value...
    ULONG   LastProgress;
    // 
    BOOLEAN IsDecreasing;
    // some drives cycle 3x around....
    ULONG   CycleCount;
    ULONG   ExpectedCycleCount;

} ERASE_UPDATE_CALLBACK_CONTEXT, *PERASE_UPDATE_CALLBACK_CONTEXT;

template <class T>
class CProxy_DEraseSampleEvents :
    public ::ATL::IConnectionPointImpl<T, &IID_DEraseSampleEvents, ::ATL::CComDynamicUnkArray>
{
public:
    VOID Fire_Update(LONG elapsedSeconds, LONG estimatedTotalSeconds)
    {
        T* pT = static_cast<T*>(this);

        // setup the dispatch parameters to be used repeatedly
        DISPPARAMS disp; RtlZeroMemory(&disp, sizeof(disp));
        VARIANT args[3];
        VariantInit(&args[2]);
        VariantInit(&args[1]);
        VariantInit(&args[0]);
        //
        // Yes, arguments are place last-to-first for IDispatch.
        //
        args[2].vt = VT_DISPATCH;
        args[2].pdispVal = static_cast<IDispatch*>(pT);
        args[1].vt = VT_I4;
        args[1].lVal = elapsedSeconds;
        args[0].vt = VT_I4;
        args[0].lVal = estimatedTotalSeconds;
        disp.rgvarg = &(args[0]);
        disp.cArgs = 3;

        // now loop and fire the actual events
        int nConnections = m_vec.GetSize();
        for (int nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
        {
            IDispatch* pDispatch = NULL;
            pT->Lock();
            IUnknown* sp = m_vec.GetAt(nConnectionIndex);
            if (sp != NULL) { sp->QueryInterface(&pDispatch); }
            pT->Unlock();
            if (pDispatch != NULL)
            {
                // throw away the results on failure....
                pDispatch->Invoke(
                    DISPID_IERASESAMPLEEVENTS_UPDATE,
                    IID_NULL,
                    LOCALE_SYSTEM_DEFAULT,
                    DISPATCH_METHOD,
                    &disp,
                    NULL, // [out,retval]
                    NULL, // exception info
                    NULL // which arg is incorrect
                    );
                pDispatch->Release();
            }
        }
        return;
    }
};

// Most of the events generated/handled pass two IDispatch* parameters (object, details)
// The next typedef allows less typing to support IWriteEngine2Event callbacks
#define ERASE_WRITE_ENGINE_SOURCE 0
typedef ::ATL::IDispEventSimpleImpl<ERASE_WRITE_ENGINE_SOURCE,
                                    CMsftEraseSample, 
                                    &IID_DWriteEngine2Events> EraseWriteEngineEventSimpleImpl;

class CMsftEraseSample :
    // CComObjectRootEx provide helpers for IUknown
    public ::ATL::CComObjectRootEx<::ATL::CComMultiThreadModel>,
    // CComCoClass provides helpers for creating the object
    public ::ATL::CComCoClass<CMsftEraseSample, &CLSID_MsftEraseSample>,
    // IDispatchImpl should provide most IDispatch support
    public ::ATL::IDispatchImpl<IEraseSample, &IID_IEraseSample, &LIBID_EraseSampleLib, /*Major*/1, /*Minor*/0>,
    // IConn...ContainerImpl handles finding/using our events via connection points
    public ::ATL::IConnectionPointContainerImpl<CMsftEraseSample>,
    // allows simpler/quicker use of events by clients
    public ::ATL::IProvideClassInfo2Impl<&CLSID_MsftEraseSample, &IID_DEraseSampleEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>,
    // Use my custom proxy template class for this (similiar to VS wizard code)
    public CProxy_DEraseSampleEvents<CMsftEraseSample>,
    // Need to implement events for the IWriteEngine use (erase of randomly writable media)
    public EraseWriteEngineEventSimpleImpl,
    // ISupportErrorInfo is Yet Another Interface to be supported
    public ISupportErrorInfo
{

public:
    // This allows usage of .RGS files for registration
    // could also use DECLARE_NO_REGISTRY() for LH-only binary
    DECLARE_REGISTRY_RESOURCEID(IDR_ERASESAMPLE)

    // This is neccessary for any non-aggregatable class
    DECLARE_NOT_AGGREGATABLE(CMsftEraseSample)

    // This is the list of all interfaces supported by the object
    BEGIN_COM_MAP(CMsftEraseSample)
        COM_INTERFACE_ENTRY(IEraseSample)
        COM_INTERFACE_ENTRY(IDiscFormat2)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CMsftEraseSample)
        CONNECTION_POINT_ENTRY(IID_DEraseSampleEvents)
    END_CONNECTION_POINT_MAP()

    BEGIN_SINK_MAP(CMsftEraseSample)
        SINK_ENTRY_INFO(ERASE_WRITE_ENGINE_SOURCE, IID_DWriteEngine2Events, DISPID_DWRITEENGINE2EVENTS_UPDATE, &WriteEngineUpdate, &g_GenericDualIDispatchEventInfo)
    END_SINK_MAP()

public: // IDispatch for write engine updates
    STDMETHOD_(VOID, WriteEngineUpdate)(IDispatch* object, IDispatch* progress);

public: // ISupportErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public: // IDiscFormat2
    STDMETHOD(IsRecorderSupported)(IDiscRecorder2* recorder, VARIANT_BOOL* value);
    STDMETHOD(IsCurrentMediaSupported)(IDiscRecorder2* recorder, VARIANT_BOOL* value);
    STDMETHOD(get_SupportedMediaTypes)(SAFEARRAY** value);
    STDMETHOD(get_MediaPhysicallyBlank)(VARIANT_BOOL* value);
    STDMETHOD(get_MediaHeuristicallyBlank)(VARIANT_BOOL* value);

public: // IEraseSample
    STDMETHOD(put_Recorder)(IDiscRecorder2*  value);
    STDMETHOD(get_Recorder)(IDiscRecorder2** value);
    STDMETHOD(put_FullErase)(VARIANT_BOOL   value);
    STDMETHOD(get_FullErase)(VARIANT_BOOL* value);
    STDMETHOD(get_CurrentPhysicalMediaType)(IMAPI_MEDIA_PHYSICAL_TYPE* value);
    STDMETHOD(put_ClientName)(BSTR   value);
    STDMETHOD(get_ClientName)(BSTR* value);

    STDMETHOD(EraseMedia)();

    // THE FOLLOWING ROUTINES ARE NOT AVAILABLE VIA COM:
    HRESULT Init();

private:
    HRESULT UpdateProgress(IN ULONG ElapsedSeconds, IN ULONG EstimatedTotalSeconds);
    HRESULT SendEraseCommand();
    HRESULT EraseByWrite(IMAPI_MEDIA_PHYSICAL_TYPE mediaType);

    static HRESULT GetDiscInformation(
                        __in                              IDiscRecorder2Ex* recorder,
                        __deref_out_bcount(*DiscInfoSize) DISC_INFORMATION ** DiscInfo,
                        __out                             ULONG * DiscInfoSize,
                                                          ULONG RequiredSize
                        );
    static HRESULT GetDiscCapabilities(
                        __in                                  IDiscRecorder2Ex* recorder,
                        __deref_out_bcount(*CapabilitiesSize) CDVD_CAPABILITIES_PAGE ** Capabilities,
                        __out                                 ULONG * CapabilitiesSize,
                                                              ULONG RequiredSize
                        );

    HRESULT WaitForReadDiscInfoToWorkAfterBlank(ULONG EstimatedMillisecondsToCompletion);
    BOOLEAN CheckRecorderSupportsErase(__in IDiscRecorder2Ex * DiscRecorder, BOOLEAN CheckCurrentMedia);
    void static EraseUpdateCallBack(__in PVOID Object, __in SENSE_DATA* Sense);


    ////////////////////////////////////////////////////////////////////////////////
    IDiscRecorder2*   m_Recorder;
    IDiscRecorder2Ex* m_RecorderEx;
    VARIANT_BOOL      m_FullErase;
    BSTR              m_ClientName;

    // The following items are required for proper
    // event notification during erase-by-write
    static const ULONG DefaultSectorsPerWriteForQuickErase = ((2 * 1024 * 1024) / 2048); // 2MB

    // Timekeepers for erase-by-write
    CTaskTimeEstimator* m_TimeKeeperWrite;

    // Additional state for erase-by-write
    ULONG             m_SectorsFirstWrite;
    ULONG             m_SectorsSecondWrite;
    BOOLEAN           m_WritingFirstSection;


public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    HRESULT FinalConstruct();
    VOID    FinalRelease();

    CMsftEraseSample() :
        m_Recorder(NULL),
        m_RecorderEx(NULL),
        m_FullErase(VARIANT_FALSE),
        m_ClientName(NULL),
        m_TimeKeeperWrite(NULL),
        m_SectorsFirstWrite(DefaultSectorsPerWriteForQuickErase),
        m_SectorsSecondWrite(DefaultSectorsPerWriteForQuickErase),
        m_WritingFirstSection(FALSE)
    {
    }
};

OBJECT_ENTRY_AUTO(__uuidof(MsftEraseSample), CMsftEraseSample)

