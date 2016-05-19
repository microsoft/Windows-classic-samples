// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// GestureHandler.h : Declaration of the CGestureHandler

#pragma once
#include "resource.h"       // main symbols

#include "COMRTS.h"
#include "rtscom.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


// CGestureHandler

class ATL_NO_VTABLE CGestureHandler :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGestureHandler, &CLSID_GestureHandler>,
	public IGestureHandler,
	public IStylusAsyncPlugin
{
public:
	CGestureHandler()
	{
		m_pUnkMarshaler = NULL;
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_GESTUREHANDLER)


	BEGIN_COM_MAP(CGestureHandler)
		COM_INTERFACE_ENTRY(IGestureHandler)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
		COM_INTERFACE_ENTRY(IStylusAsyncPlugin)
	END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()
	DECLARE_GET_CONTROLLING_UNKNOWN()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

private:
	CStatic* m_pStatusControl;

// Helper methods
public:
	HRESULT SetStatusWindow(CStatic* staticGestureStatus) { m_pStatusControl = staticGestureStatus; return S_OK; }
	void SetGestureString(int gestureID, CString* strResult);

// IStylusAsyncPlugin Methods
public:
	STDMETHODIMP RealTimeStylusEnabled( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids);

	STDMETHODIMP RealTimeStylusDisabled( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids);

	STDMETHODIMP StylusInRange( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid);

	STDMETHODIMP StylusOutOfRange( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid);

	STDMETHODIMP StylusDown( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPackets,
            /* [out][in] */ LONG **ppInOutPkts);

	STDMETHODIMP StylusUp( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPackets,
            /* [out][in] */ LONG **ppInOutPkts);

	STDMETHODIMP StylusButtonDown( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos);

	STDMETHODIMP StylusButtonUp( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos);

	STDMETHODIMP InAirPackets( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPktCount,
            /* [in] */ ULONG cPktBuffLength,
            /* [size_is][in] */ LONG *pPackets,
            /* [out][in] */ ULONG *pcInOutPkts,
            /* [out][in] */ LONG **ppInOutPkts);

	STDMETHODIMP Packets( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPktCount,
            /* [in] */ ULONG cPktBuffLength,
            /* [size_is][in] */ LONG *pPackets,
            /* [out][in] */ ULONG *pcInOutPkts,
            /* [out][in] */ LONG **ppInOutPkts);

	STDMETHODIMP CustomStylusDataAdded( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const GUID *pGuidId,
            /* [in] */ ULONG cbData,
            /* [in] */ const BYTE *pbData);

	STDMETHODIMP SystemEvent( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ SYSTEM_EVENT event,
            /* [in] */ SYSTEM_EVENT_DATA eventdata);

	STDMETHODIMP TabletAdded( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ IInkTablet *piTablet);

	STDMETHODIMP TabletRemoved( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ LONG iTabletIndex);

	STDMETHODIMP Error( 
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ IStylusPlugin *piPlugin,
            /* [in] */ RealTimeStylusDataInterest dataInterest,
            /* [in] */ HRESULT hrErrorCode,
            /* [out][in] */ LONG_PTR *lptrKey);

	STDMETHODIMP UpdateMapping( 
            /* [in] */ IRealTimeStylus *piRtsSrc);

	STDMETHODIMP DataInterest( 
            /* [retval][out] */ RealTimeStylusDataInterest *pDataInterest);
};

