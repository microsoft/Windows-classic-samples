// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// GestureHandler.cpp : Implementation of CGestureHandler

#include "stdafx.h"
#include "GestureHandler.h"


// CGestureHandler
// IStylusAsyncPlugin Interface implementation

STDMETHODIMP CGestureHandler::Packets(
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

STDMETHODIMP CGestureHandler::StylusDown(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPacket,
            /* [out][in] */ LONG **ppInOutPkt)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::StylusUp(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const StylusInfo *pStylusInfo,
            /* [in] */ ULONG cPropCountPerPkt,
            /* [size_is][in] */ LONG *pPacket,
            /* [out][in] */ LONG **ppInOutPkt)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::Error(
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

STDMETHODIMP CGestureHandler::DataInterest(
            /* [retval][out] */ RealTimeStylusDataInterest *pDataInterest)
{
    *pDataInterest = (RealTimeStylusDataInterest)(RTSDI_CustomStylusDataAdded | RTSDI_Error);
    return S_OK;
}



/// The remaining interface methods are not used in this sample application
STDMETHODIMP CGestureHandler::RealTimeStylusEnabled(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::RealTimeStylusDisabled(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ ULONG cTcidCount,
            /* [size_is][in] */ const TABLET_CONTEXT_ID *pTcids)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::StylusInRange(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::StylusOutOfRange(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::StylusButtonDown(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::StylusButtonUp(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ const GUID *pGuidStylusButton,
            /* [out][in] */ POINT *pStylusPos)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::InAirPackets(
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

STDMETHODIMP CGestureHandler::CustomStylusDataAdded(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ const GUID *pGuidId,
            /* [in] */ ULONG cbData,
            /* [in] */ const BYTE *pbData)
{
    // Did we get passed gesture data?
    if (*pGuidId == GUID_GESTURE_DATA)
    {
        /* See if the data pointer actually points to something.
           If more than one alternate is detected ignore the
           lower alternates */
        if ((pbData != NULL) && (cbData % sizeof(GESTURE_DATA)))
        {
            // Access the data coming as a GESTURE_DATA structure
            GESTURE_DATA* pGD = (GESTURE_DATA*)pbData;

            CString strStatus;
            CString strGestureId;

            // Helper function that maps the gesture ID to a string value
            SetGestureString(pGD->gestureId, &strGestureId);

            strStatus.Format(L"Gesture=%s\tConfidence=%d\tStrokes=%d", strGestureId, pGD->recoConfidence, pGD->strokeCount);
            m_pStatusControl->SetWindowTextW(strStatus);
        }
        else
        {
            m_pStatusControl->SetWindowTextW(L"Not gesture data.");
        }
    }
    else
    {
        m_pStatusControl->SetWindowTextW(L"Not gesture data.");
    }

    return S_OK;
}

STDMETHODIMP CGestureHandler::SystemEvent(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ TABLET_CONTEXT_ID tcid,
            /* [in] */ STYLUS_ID sid,
            /* [in] */ SYSTEM_EVENT event,
            /* [in] */ SYSTEM_EVENT_DATA eventdata)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::TabletAdded(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ IInkTablet *piTablet)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::TabletRemoved(
            /* [in] */ IRealTimeStylus *piRtsSrc,
            /* [in] */ LONG iTabletIndex)
{
    return S_OK;
}

STDMETHODIMP CGestureHandler::UpdateMapping(
            /* [in] */ IRealTimeStylus *piRtsSrc)
{
    return S_OK;
}

// Helper functions

// Sets a string identifying the gesture
void CGestureHandler::SetGestureString(int gestureID, CString* strResult)
{
    switch (gestureID)
    {
        case IAG_AllGestures:
         strResult->SetString(L"AllGestures", sizeof("AllGestures"));
         break;

        case IAG_NoGesture:
            strResult->SetString(L"NoGesture", sizeof("NoGesture"));
            break;

        case IAG_Scratchout:
            strResult->SetString(L"Scratchout", sizeof("Scratchout"));
            break;

        case IAG_Triangle:
            strResult->SetString(L"Triangle", sizeof("Triangle"));
            break;

        case IAG_Square:
            strResult->SetString(L"Square", sizeof("Square"));
            break;

        case IAG_Star:
            strResult->SetString(L"Star", sizeof("Star"));
            break;

        case IAG_Check:
            strResult->SetString(L"Check", sizeof("Check"));
            break;

        case IAG_Curlicue:
            strResult->SetString(L"Curlicue", sizeof("Curlicue"));
            break;

        case IAG_DoubleCurlicue:
            strResult->SetString(L"DoubleCurlicue", sizeof("DoubleCurlicue"));
            break;

        case IAG_Circle:
            strResult->SetString(L"Circle", sizeof("Circle"));
            break;

        case IAG_DoubleCircle:
            strResult->SetString(L"DoubleCircle", sizeof("DoubleCircle"));
            break;

        case IAG_SemiCircleLeft:
            strResult->SetString(L"SemiCircleLeft", sizeof("SemiCircleLeft"));
            break;

        case IAG_SemiCircleRight:
            strResult->SetString(L"SemiCircleRight", sizeof("SemiCircleRight"));
            break;

        case IAG_ChevronUp:
            strResult->SetString(L"ChevronUp", sizeof("ChevronUp"));
            break;

        case IAG_ChevronDown:
            strResult->SetString(L"ChevronDown", sizeof("ChevronDown"));
            break;

        case IAG_ChevronLeft:
            strResult->SetString(L"ChevronLeft", sizeof("ChevronLeft"));
            break;

        case IAG_ChevronRight:
            strResult->SetString(L"ChevronRight", sizeof("ChevronRight"));
            break;

        case IAG_ArrowUp:
            strResult->SetString(L"ArrowUp", sizeof("ArrowUp"));
            break;

        case IAG_ArrowDown:
            strResult->SetString(L"ArrowDown", sizeof("ArrowDown"));
            break;

        case IAG_ArrowLeft:
            strResult->SetString(L"ArrowLeft", sizeof("ArrowLeft"));
            break;

        case IAG_ArrowRight:
            strResult->SetString(L"ArrowRight", sizeof("ArrowRight"));
            break;

        case IAG_Up:
            strResult->SetString(L"Up", sizeof("Up"));
            break;

        case IAG_Down:
            strResult->SetString(L"Down", sizeof("Down"));
            break;

        case IAG_Left:
            strResult->SetString(L"Left", sizeof("Left"));
            break;

        case IAG_Right:
            strResult->SetString(L"Right", sizeof("Right"));
            break;

        case IAG_UpDown:
            strResult->SetString(L"UpDown", sizeof("UpDown"));
            break;

        case IAG_DownUp:
            strResult->SetString(L"UpDown", sizeof("UpDown"));
            break;

        case IAG_LeftRight:
            strResult->SetString(L"UpDown", sizeof("UpDown"));
            break;

        case IAG_RightLeft:
            strResult->SetString(L"RightLeft", sizeof("RightLeft"));
            break;

        case IAG_UpLeftLong:
            strResult->SetString(L"UpLeftLong", sizeof("UpLeftLong"));
            break;

        case IAG_UpRightLong:
            strResult->SetString(L"UpRightLong", sizeof("UpRightLong"));
            break;

        case IAG_DownLeftLong:
            strResult->SetString(L"DownLeftLong", sizeof("DownLeftLong"));
            break;

        case IAG_DownRightLong:
            strResult->SetString(L"DownRightLong", sizeof("DownRightLong"));
            break;

        case IAG_UpLeft:
            strResult->SetString(L"UpLeft", sizeof("UpLeft"));
            break;

        case IAG_UpRight:
            strResult->SetString(L"UpRight", sizeof("UpRight"));
            break;

        case IAG_DownLeft:
            strResult->SetString(L"DownLeft", sizeof("DownLeft"));
            break;

        case IAG_DownRight:
            strResult->SetString(L"DownRight", sizeof("DownRight"));
            break;

        case IAG_LeftUp:
            strResult->SetString(L"LeftUp", sizeof("LeftUp"));
            break;

        case IAG_LeftDown:
            strResult->SetString(L"LeftDown", sizeof("LeftDown"));
            break;

        case IAG_RightUp:
            strResult->SetString(L"RightUp", sizeof("RightUp"));
            break;

        case IAG_RightDown:
            strResult->SetString(L"RightDown", sizeof("RightDown"));
            break;

        case IAG_Exclamation:
            strResult->SetString(L"Exclamation", sizeof("Exclamation"));
            break;

        case IAG_Tap:
            strResult->SetString(L"Tap", sizeof("Tap"));
            break;

        case IAG_DoubleTap:
            strResult->SetString(L"DoubleTap", sizeof("DoubleTap"));
            break;

        default:
            strResult->SetString(L"Unknown", sizeof("Unknown"));
            break;
    }
}
