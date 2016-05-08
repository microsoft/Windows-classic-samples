/* Copyright (c) Microsoft Corporation. All rights reserved. */

#pragma once
#ifndef _IMAPI2TEST_RAWWRITER2_EVENTS_
#define _IMAPI2TEST_RAWWRITER2_EVENTS_

#include "common.h"


class  CTestRawWriter2Event;

// CTestRawWriter2Event definition
class  CTestRawWriter2Event :
    public ::ATL::CComObjectRootEx<::ATL::CComSingleThreadModel>,
    public ::ATL::CComCoClass<CTestRawWriter2Event>,
    public ::ATL::IDispatchImpl<    IDispatch,            &IID_DDiscFormat2RawCDEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>,
    public ::ATL::IDispEventImpl<1, CTestRawWriter2Event, &IID_DDiscFormat2RawCDEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>
{
    // This is neccessary for any non-aggregatable class
    DECLARE_NOT_AGGREGATABLE(CTestRawWriter2Event)

    // This is the list of all interfaces supported by the object
    BEGIN_COM_MAP(CTestRawWriter2Event)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    BEGIN_SINK_MAP(CTestRawWriter2Event)
        SINK_ENTRY_EX(1, IID_DDiscFormat2RawCDEvents, DISPID_DDISCFORMAT2RAWCDEVENTS_UPDATE, &Update)
    END_SINK_MAP()

public: // CTestRawWriter2Event

    // CTestRawWriter2Event
    STDMETHOD_(VOID, Update)(IDispatch* object, IDispatch* progress);

};

#endif // _IMAPI2TEST_RAWWRITER2_EVENTS_
