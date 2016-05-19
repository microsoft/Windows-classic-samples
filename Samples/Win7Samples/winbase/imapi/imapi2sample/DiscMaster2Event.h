/* Copyright (c) Microsoft Corporation. All rights reserved. */

#pragma once
#ifndef _IMAPI2TEST_DM2_EVENTS_
#define _IMAPI2TEST_DM2_EVENTS_

#include "common.h"

// CMsftDiscMaster2 definition
class CTestDiscMaster2Event;



class CTestDiscMaster2Event :
    public ::ATL::CComObjectRootEx<::ATL::CComSingleThreadModel>,
    public ::ATL::CComCoClass<CTestDiscMaster2Event>,
    public ::ATL::IDispatchImpl<    IDispatch,             &IID_DDiscMaster2Events, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>,
    public ::ATL::IDispEventImpl<1, CTestDiscMaster2Event, &IID_DDiscMaster2Events, &LIBID_IMAPILib2, IMAPILib2_MajorVersion, IMAPILib2_MinorVersion>
{
public:

    // This is neccessary for any non-aggregatable class
    DECLARE_NOT_AGGREGATABLE(CTestDiscMaster2Event)

    // This is the list of all interfaces supported by the object
    BEGIN_COM_MAP(CTestDiscMaster2Event)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    BEGIN_SINK_MAP(CTestDiscMaster2Event)
        SINK_ENTRY_EX(1, IID_DDiscMaster2Events, DISPID_DDISCMASTER2EVENTS_DEVICEADDED,   &NotifyDeviceAdded  )
        SINK_ENTRY_EX(1, IID_DDiscMaster2Events, DISPID_DDISCMASTER2EVENTS_DEVICEREMOVED, &NotifyDeviceRemoved)
    END_SINK_MAP()

public: // DDiscMaster2Events
    STDMETHOD_(VOID, NotifyDeviceAdded)(IDispatch* object, BSTR value);
    STDMETHOD_(VOID, NotifyDeviceRemoved)(IDispatch* object, BSTR value);

};

#endif // _IMAPI2TEST_DM2_EVENTS_
