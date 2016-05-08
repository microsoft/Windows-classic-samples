/*--

Copyright (C) Microsoft Corporation, 2006

Events class

--*/

#pragma once
#ifndef _ERASE_SAMPLE_TEST_EVENTS_
#define _ERASE_SAMPLE_TEST_EVENTS_

// Erase takes an IDispatch* then two VT_I4 (object, elapsed, total) as arguments
static ::ATL::_ATL_FUNC_INFO g_EraseIDispatchEventInfo = { CC_STDCALL, VT_EMPTY, 3, { VT_DISPATCH, VT_I4, VT_I4 } };

class  CTestEraseEvent :
    public ::ATL::CComObjectRootEx<::ATL::CComSingleThreadModel>,
    public ::ATL::CComCoClass<CTestEraseEvent>,
    public ::ATL::IDispatchImpl<    IDispatch,       &IID_DEraseSampleEvents, &LIBID_EraseSampleLib, 1, 0>,
    public ::ATL::IDispEventImpl<1, CTestEraseEvent, &IID_DEraseSampleEvents, &LIBID_EraseSampleLib, 1, 0>
{
    // This is neccessary for any non-aggregatable class
    DECLARE_NOT_AGGREGATABLE(CTestEraseEvent)

    // This is the list of all interfaces supported by the object
    BEGIN_COM_MAP(CTestEraseEvent)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    BEGIN_SINK_MAP(CTestEraseEvent)
        SINK_ENTRY_INFO(1, IID_DEraseSampleEvents, DISPID_IERASESAMPLEEVENTS_UPDATE, &Update, &g_EraseIDispatchEventInfo)
    END_SINK_MAP()

public: // DDEraseSampleEvents

    // DDEraseSampleEvents
    STDMETHOD_(VOID, Update)(IDispatch* object, LONG elapsedSeconds, LONG expectedSeconds);

};
#endif // _ERASE_SAMPLE_TEST_EVENTS_
