// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:       
//      EventSinks.h
//
// Description:
//      This file contains the definitions of the event sink templates,
//      instantiated in the CAdvRecoApp class.
//
//      The event source interfaces used are: 
//      _IInkEvents, _IInkRecognitionEvent, _IInkCollectorEvents
//   
//--------------------------------------------------------------------------

#pragma once

// The DISPID's of the events 
#ifndef DISPID_CEStroke
    #define DISPID_CEStroke                     0x00000001
#endif
#ifndef DISPID_RERecognition
    #define DISPID_RERecognition                0x00000002
#endif

// IDispEventSimpleImpl requires a constant as a sink id; define it here
#define SINK_ID  1

/////////////////////////////////////////////////////////
//                                          
// IInkRecognitionEventsImpl
// 
// The template is derived from the ATL's IDispEventSimpleImpl 
// to implement a sink for the IInkRecognitionEvents, fired by 
// the InkRecoContext object
//
/////////////////////////////////////////////////////////

template <class T>
class ATL_NO_VTABLE IInkRecognitionEventsImpl :
	public IDispEventSimpleImpl<SINK_ID, IInkRecognitionEventsImpl<T>, 
                                &DIID__IInkRecognitionEvents>
{
public:
    // ATL structure with the type information for the event, 
    // handled in this template. (Initialized in the AdvReco.cpp)
    static const _ATL_FUNC_INFO mc_AtlFuncInfo;

BEGIN_SINK_MAP(IInkRecognitionEventsImpl)
    SINK_ENTRY_INFO(SINK_ID, 
                    DIID__IInkRecognitionEvents, 
                    DISPID_RERecognition,
                    Recognition, 
                    const_cast<_ATL_FUNC_INFO*>(&mc_AtlFuncInfo))
END_SINK_MAP()

    HRESULT __stdcall Recognition(BSTR bstrRecognizedString, 
                                    VARIANT vCustomParam, 
                                    InkRecognitionStatus RecognitionStatus)
    {
		T* pT = static_cast<T*>(this);
        return pT->OnRecognition(bstrRecognizedString, vCustomParam, RecognitionStatus);
    }
};


/////////////////////////////////////////////////////////
//                                          
// IInkCollectorEventsImpl
// 
// The template is derived from the ATL's IDispEventSimpleImpl 
// to implement a sink for the IInkCollectorEvents, fired by 
// the InkCollector object
// Since the IDispEventSimpleImpl doesn't require to supply 
// implementation code for every event, this template has a handler
// for the Gesture event only.
//
/////////////////////////////////////////////////////////

template <class T>
class ATL_NO_VTABLE IInkCollectorEventsImpl :
	public IDispEventSimpleImpl<SINK_ID, 
                                IInkCollectorEventsImpl<T>, 
                                &DIID__IInkCollectorEvents>
{
public:
    // ATL structure with the type information of Stroke events, 
    // handled in this template.(Initialized in the AdvReco.cpp)
    static const _ATL_FUNC_INFO mc_AtlFuncInfo;

BEGIN_SINK_MAP(IInkCollectorEventsImpl)
    SINK_ENTRY_INFO(SINK_ID, 
                    DIID__IInkCollectorEvents, 
                    DISPID_CEStroke, 
                    Stroke, 
                    const_cast<_ATL_FUNC_INFO*>(&mc_AtlFuncInfo))
END_SINK_MAP()

    HRESULT __stdcall Stroke(IInkCursor* pIInkCursor, IInkStrokeDisp* pInkStroke, 
                             VARIANT_BOOL* pbCancel)
    {
		T* pT = static_cast<T*>(this);
        return pT->OnStroke(pIInkCursor, pInkStroke, pbCancel);
    }
};

