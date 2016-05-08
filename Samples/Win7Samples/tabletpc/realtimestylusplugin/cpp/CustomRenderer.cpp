// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// CustomRenderer.cpp : Implementation of CCustomRenderer

#include "stdafx.h"
#include "CustomRenderer.h"

// CCustomRenderer

// Helper functions
void CCustomRenderer::HiMetricToPixel(LONG* pfX, LONG* pfY)
{
    // Retrieve pixels/inch for screen width
    int xPixelsPerInch = GetDeviceCaps(m_hDC, LOGPIXELSX);

    if (0 == xPixelsPerInch)
    {
        // System call failed, use default
        xPixelsPerInch = 96;
    }

    // Retrieve pixels/inch for screen height from system
    int yPixelsPerInch = GetDeviceCaps(m_hDC, LOGPIXELSY);

    if (0 == yPixelsPerInch)
    {
        // System call failed, use default
        yPixelsPerInch = 96;
    }

    *pfX = *pfX/((LONG)(2540.0/xPixelsPerInch));
    *pfY = *pfY/((LONG)(2540.0/yPixelsPerInch));
}

STDMETHODIMP CCustomRenderer::SetHDC(HDC hDC)
{
    m_hDC = hDC;

    // Create a green pen
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 127, 0));

    // Create a brush the color of the dialog background
    HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    // Select the new pen and brush for drawing
    SelectObject(m_hDC, hPen);
    SelectObject(m_hDC, hBrush);

    return S_OK;
}

// Interface implementation
STDMETHODIMP CCustomRenderer::Packets(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPktCount,
            /* [in] */ ULONG cPktBuffLength,
            /* [size_is][in] */ LONG *pPackets,
            /* [out][in] */ ULONG *pcInOutPkts,
            /* [out][in] */ LONG **ppInOutPkts)
{
    ULONG cPropertyCount = cPktBuffLength/cPktCount;

    // For each new packet received, extract the X,Y data
    // and draw a small circle around the result
    for (ULONG i = 0; i < cPktCount; i += cPropertyCount)
    {
        // Packet data always has X followed
        // by Y followed by the rest
        LONG x = pPackets[i];
        LONG y = pPackets[i+1];

        // Since the packet data is in ink space coordinates,
        // we need to convert to pixels...
        HiMetricToPixel(&x, &y);

        // Draw a circle corresponding to the packet
        Ellipse(m_hDC, x-2, y-2, x+2, y+2);
    }
    return S_OK;
}

STDMETHODIMP CCustomRenderer::StylusDown(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPacket,
            /* [out][in] */ LONG **ppInOutPkt)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::StylusUp(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPacket,
            /* [out][in] */ LONG **ppInOutPkt)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::Error(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ IStylusPlugin *piPlugin,
            /* [in] */ RealTimeStylusDataInterest dataInterest,
            /* [in] */ HRESULT hrErrorCode,
            /* [out][in] */ LONG_PTR *lptrKey)
{
    CString strError;
    strError.Format(L"An error occured. Error code: %d", hrErrorCode);
    TRACE(strError);
    return S_OK;
}

STDMETHODIMP CCustomRenderer::DataInterest(
            /* [retval][out] */ RealTimeStylusDataInterest *pDataInterest)
{
    *pDataInterest = (RealTimeStylusDataInterest)(RTSDI_Packets | RTSDI_Error);
    return S_OK;
}



/// The remaining interface methods are not used in this sample application
STDMETHODIMP CCustomRenderer::RealTimeStylusEnabled(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::RealTimeStylusDisabled(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::StylusInRange(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::StylusOutOfRange(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::StylusButtonDown(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::StylusButtonUp(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::InAirPackets(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPktCount,
            /* [in] */ ULONG cPktBuffLength,
            /* [size_is][in] */ LONG *pPackets,
            /* [out][in] */ ULONG *pcInOutPkts,
            /* [out][in] */ LONG **ppInOutPkts)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::CustomStylusDataAdded(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const GUID *pGuidId,
            /* [in] */ ULONG cbData,
            /* [in] */ const BYTE *pbData)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::SystemEvent(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ SYSTEM_EVENT event,
            /* [in] */ SYSTEM_EVENT_DATA eventdata)
{
    return S_OK;
}
STDMETHODIMP CCustomRenderer::TabletAdded(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ IInkTablet *piTablet)
{
    return S_OK;
}
STDMETHODIMP CCustomRenderer::TabletRemoved(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ LONG iTabletIndex)
{
    return S_OK;
}

STDMETHODIMP CCustomRenderer::UpdateMapping(
            /* [in] */ IRealTimeStylus *piRtsSrc)
{
    return S_OK;
}
