// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// PackMod.cpp : Implementation of CPacketModifier

#include "stdafx.h"
#include "PackMod.h"
#include "msinkaut.h"

// CPacketModifier

// Helper function to set the area
// to which the packets are restricted
STDMETHODIMP CPacketModifier::SetRectangle(HDC hDC, RECT rect)
{
    m_filterRect.top = rect.top;
    m_filterRect.left = rect.left;
    m_filterRect.bottom = rect.bottom;
    m_filterRect.right = rect.right;

    // Do all calculations in high metric
    PixelToHiMetric(hDC, &m_filterRect.left, &m_filterRect.top);
    PixelToHiMetric(hDC, &m_filterRect.right, &m_filterRect.bottom);
    return S_OK;
}

// Helper function to convert from Pixel to HiMetric space
void CPacketModifier::PixelToHiMetric(HDC hdc, LONG* pfX, LONG* pfY)
{
    // Retrieve pixels/inch for screen width
    int xPixelsPerInch = GetDeviceCaps(hdc, LOGPIXELSX);

    if (0 == xPixelsPerInch)
    {
        // System call failed, use default
        xPixelsPerInch = 96;
    }

    // Retrieve pixels/inch for screen height
    int yPixelsPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
    if (0 == yPixelsPerInch)
    {
        yPixelsPerInch = 96;
    }

    *pfX = (LONG)((*pfX * 2540.0f)/(float)xPixelsPerInch);
    *pfY = (LONG)((*pfY * 2540.0f)/(float)yPixelsPerInch);
}

// Helper method to modify a single packet
// Called from StylusDown() and StylusUp()
HRESULT CPacketModifier::ModifyPacket(
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPacket,
            /* [out][in] */ LONG **ppInOutPkt)
{
    // Pointer to a buffer to hold changed packet values
    LONG* pTempOutPkt = NULL;

    // X and Y come first (0 and 1),
    // other properties follow
    ULONG iOtherProps = 2;

    if (cPropCountPerPkt > 0)
    {
        pTempOutPkt = reinterpret_cast<LONG*>(CoTaskMemAlloc(sizeof(ULONG) * cPropCountPerPkt));

        if (NULL != pTempOutPkt)
        {
            // Packet data always has x followed by y followed by the rest.
            LONG x = pPacket[0];
            LONG y = pPacket[1];

            // In the packet data, check whether
            // its X,Y values fall outside of the specified rectangle.
            // If so, replace them with the nearest point that still
            // falls within the rectangle.
            x = (x < m_filterRect.left ? m_filterRect.left : x);
            x = (x > m_filterRect.right ? m_filterRect.right : x);
            y = (y < m_filterRect.top ? m_filterRect.top : y);
            y = (y > m_filterRect.bottom ? m_filterRect.bottom : y);

            // If necessary, modify the x,y packet data
            if ((x != pPacket[0]) || (y != pPacket[1]))
            {
                pTempOutPkt[0] = x;
                pTempOutPkt[1] = y;

                // Copy the properties that we haven't modified
                while (iOtherProps < cPropCountPerPkt)
                {
                    pTempOutPkt[iOtherProps] = pPacket[iOtherProps++];
                }

                *ppInOutPkt = pTempOutPkt;
            }
            else
            {
                CoTaskMemFree(pTempOutPkt);
            }
        }
    }

    return S_OK;
}


// Interface implementation

STDMETHODIMP CPacketModifier::DataInterest(
        /* [retval][out] */ RealTimeStylusDataInterest *pDataInterest)
{
    *pDataInterest = (RealTimeStylusDataInterest)(RTSDI_StylusDown | RTSDI_Packets |
                                                  RTSDI_StylusUp | RTSDI_Error);
    return S_OK;
}

STDMETHODIMP CPacketModifier::StylusDown(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPacket,
            /* [out][in] */ LONG **ppInOutPkt)
{
    return ModifyPacket(cPropCountPerPkt, pPacket, ppInOutPkt);
}

STDMETHODIMP CPacketModifier::Packets(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPktCount,
            /* [in] */ ULONG cPktBuffLength,
            /* [size_is][in] */ LONG *pPackets,
            /* [out][in] */ ULONG *pcInOutPkts,
            /* [out][in] */ LONG **ppInOutPkts)
{
    BOOL fModified = FALSE;                             // Did we change the packet data?
    ULONG cPropertyCount = cPktBuffLength/cPktCount;    // # of properties in a packet
    ULONG iOtherProps = 0;                                // Properties other than X and Y

    // Allocate memory for modfied packets
    LONG* pTempOutPkts = reinterpret_cast<LONG*>(CoTaskMemAlloc(sizeof(ULONG) * cPktBuffLength));

    // For each packet in the packet data, check whether
    // its X,Y values fall outside of the specified rectangle.
    // If so, replace them with the nearest point that still
    // falls within the rectangle.
    for (ULONG i = 0; i < cPktCount; i += cPropertyCount)
    {
        // Packet data always has X followed by Y
        // followed by the rest
        LONG x = pPackets[i];
        LONG y = pPackets[i+1];

        // Constrain points to the input rectangle
        x = (x < m_filterRect.left ? m_filterRect.left : x);
        x = (x > m_filterRect.right ? m_filterRect.right : x);
        y = (y < m_filterRect.top ? m_filterRect.top : y);
        y = (y > m_filterRect.bottom ? m_filterRect.bottom : y);

        // If necessary, modify the X,Y packet data
        if ((x != pPackets[i]) || (y != pPackets[i+1]))
        {
            pTempOutPkts[i] = x;
            pTempOutPkts[i+1] = y;
            iOtherProps = i+2;

            // Copy the properties that we haven't modified
            while (iOtherProps < (i + cPropertyCount))
            {
                pTempOutPkts[iOtherProps] = pPackets[iOtherProps++];
            }

            fModified = TRUE;
        }
    }

    if (fModified)
    {
        // Set the [out] pointer to the
        // memory we allocated and updated
        *ppInOutPkts = pTempOutPkts;
        *pcInOutPkts = cPktCount;
    }
    else
    {
        // Nothing modified, release the memory we allocated
        CoTaskMemFree(pTempOutPkts);
    }

    return S_OK;
}

STDMETHODIMP CPacketModifier::StylusUp(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPacket,
            /* [out][in] */ LONG **ppInOutPkt)
{
    return ModifyPacket(cPropCountPerPkt, pPacket, ppInOutPkt);
}

STDMETHODIMP CPacketModifier::Error(
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


/// The remaining interface methods are not used in this sample application
STDMETHODIMP CPacketModifier::RealTimeStylusEnabled(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::RealTimeStylusDisabled(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::StylusInRange(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::StylusOutOfRange(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::StylusButtonDown(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::StylusButtonUp(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::InAirPackets(
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

STDMETHODIMP CPacketModifier::CustomStylusDataAdded(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const GUID *pGuidId,
            /* [in] */ ULONG cbData,
            /* [in] */ const BYTE *pbData)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::SystemEvent(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ SYSTEM_EVENT event,
            /* [in] */ SYSTEM_EVENT_DATA eventdata)
{
    return S_OK;
}
STDMETHODIMP CPacketModifier::TabletAdded(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ IInkTablet *piTablet)
{
    return S_OK;
}
STDMETHODIMP CPacketModifier::TabletRemoved(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ LONG iTabletIndex)
{
    return S_OK;
}

STDMETHODIMP CPacketModifier::UpdateMapping(
            /* [in] */ IRealTimeStylus *piRtsSrc)
{
    return S_OK;
}
