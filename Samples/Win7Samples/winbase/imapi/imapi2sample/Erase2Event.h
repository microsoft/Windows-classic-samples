/* Copyright (c) Microsoft Corporation. All rights reserved. */

#pragma once
#ifndef _IMAPI2TEST_ERASE2_EVENTS_
#define _IMAPI2TEST_ERASE2_EVENTS_

#include "common.h"

// Erase takes an IDispatch* then two VT_I4 (object, elapsed, total) as arguments
static ::ATL::_ATL_FUNC_INFO g_EraseIDispatchEventInfo = { CC_STDCALL, VT_EMPTY, 3, { VT_DISPATCH, VT_I4, VT_I4 } };

class  CTestErase2Event;

// CTestCDWriteEngine2Event definition
class  CTestErase2Event :
    public ::ATL::CComObjectRootEx<::ATL::CComSingleThreadModel>,
    public ::ATL::CComCoClass<CTestErase2Event>,
    public ::ATL::IDispatchImpl<    IDispatch,        &IID_DDiscFormat2EraseEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>,
    public ::ATL::IDispEventImpl<1, CTestErase2Event, &IID_DDiscFormat2EraseEvents, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>
{
    // This is neccessary for any non-aggregatable class
    DECLARE_NOT_AGGREGATABLE(CTestErase2Event)

    // This is the list of all interfaces supported by the object
    BEGIN_COM_MAP(CTestErase2Event)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    BEGIN_SINK_MAP(CTestErase2Event)
        SINK_ENTRY_INFO(1, IID_DDiscFormat2EraseEvents, DISPID_IDISCFORMAT2ERASEEVENTS_UPDATE, &Update, &g_EraseIDispatchEventInfo)
    END_SINK_MAP()

public: // DDiscFormat2EraseEvents

    // DDiscFormat2EraseEvents
    STDMETHOD_(VOID, Update)(IDispatch* object, LONG elapsedSeconds, LONG expectedSeconds);

};

#endif // _IMAPI2TEST_ERASE2_EVENTS_