/* Copyright (c) Microsoft Corporation. All rights reserved. */

#pragma once
#ifndef _IMAPI2TEST_AUDIO_EVENTS_
#define _IMAPI2TEST_AUDIO_EVENTS_

#include "common.h"


class  CAudioEvent;

// CTestCDWriteEngine2Event definition
class  CAudioEvent :
    public ::ATL::CComObjectRootEx<::ATL::CComMultiThreadModel>,
    public ::ATL::CComCoClass<CAudioEvent>,
    public ::ATL::IDispatchImpl<    IDispatch,            &IID_DDiscFormat2TrackAtOnceEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>,
    public ::ATL::IDispEventImpl<1, CAudioEvent, &IID_DDiscFormat2TrackAtOnceEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>
{
    // This is neccessary for any non-aggregatable class
    DECLARE_NOT_AGGREGATABLE(CAudioEvent)

    // This is the list of all interfaces supported by the object
    BEGIN_COM_MAP(CAudioEvent)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    BEGIN_SINK_MAP(CAudioEvent)
        SINK_ENTRY_EX(1, IID_DDiscFormat2TrackAtOnceEvents, DISPID_DDISCFORMAT2TAOEVENTS_UPDATE, &Update)
    END_SINK_MAP()

public: // DDiscFormat2TrackAtOnceEvents
    STDMETHOD_(VOID, Update)(IDispatch* object, IDispatch* progress);

};

#endif // _IMAPI2TEST_AUDIO_EVENTS_
