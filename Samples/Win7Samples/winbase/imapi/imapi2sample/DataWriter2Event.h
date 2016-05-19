/* Copyright (c) Microsoft Corporation. All rights reserved. */

#pragma once
#ifndef _IMAPI2TEST_DATAWRITER2_EVENTS_
#define _IMAPI2TEST_DATAWRITER2_EVENTS_

#include "common.h"


class  CTestDataWriter2Event;

// CTestCDWriteEngine2Event definition
class  CTestDataWriter2Event :
    public ::ATL::CComObjectRootEx<::ATL::CComSingleThreadModel>,
    public ::ATL::CComCoClass<CTestDataWriter2Event>,
    public ::ATL::IDispatchImpl<    IDispatch,             &IID_DDiscFormat2DataEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>,
    public ::ATL::IDispEventImpl<1, CTestDataWriter2Event, &IID_DDiscFormat2DataEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>
{
    // This is neccessary for any non-aggregatable class
    DECLARE_NOT_AGGREGATABLE(CTestDataWriter2Event)

    // This is the list of all interfaces supported by the object
    BEGIN_COM_MAP(CTestDataWriter2Event)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    BEGIN_SINK_MAP(CTestDataWriter2Event)
        SINK_ENTRY_EX(1, IID_DDiscFormat2DataEvents, DISPID_DDISCFORMAT2DATAEVENTS_UPDATE, &Update)
    END_SINK_MAP()

public: // DDiscFormat2DataEvents

    // DDiscFormat2DataEvents
    STDMETHOD_(VOID, Update)(IDispatch* object, IDispatch* progress);

};

#endif // _IMAPI2TEST_DATAWRITER2_EVENTS_
