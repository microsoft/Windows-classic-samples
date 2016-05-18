#ifndef __COM_FAXSERVERNOTIFY_SAMPLE
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------


#define __COM_FAXSERVERNOTIFY_SAMPLE


#include "FaxNotify.h"

class ATL_NO_VTABLE _CFaxServerNotify : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispEventImpl<0, _CFaxServerNotify, &DIID_IFaxServerNotify,&LIBID_FAXCOMEXLib, 1, 0>
{
public:
    _CFaxServerNotify();

    ~_CFaxServerNotify();

BEGIN_COM_MAP(_CFaxServerNotify)
END_COM_MAP()

BEGIN_SINK_MAP(_CFaxServerNotify)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 1, OnIncomingJobAdded)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 2, OnIncomingJobRemoved)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 3, OnIncomingJobChanged)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 4, OnOutgoingJobAdded)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 5, OnOutgoingJobRemoved)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 6, OnOutgoingJobChanged)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 7, OnIncomingMessageAdded)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 8, OnIncomingMessageRemoved)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 9, OnOutgoingMessageAdded)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 10, OnOutgoingMessageRemoved)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 11, OnReceiptOptionsChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 12, OnActivityLoggingConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 13, OnSecurityConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 14, OnEventLoggingConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 15, OnOutgoingQueueConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 16, OnOutgoingArchiveConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 17, OnIncomingArchiveConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 18, OnDevicesConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 19, OnOutboundRoutingGroupsConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 20, OnOutboundRoutingRulesConfigChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 21, OnServerActivityChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 22, OnQueuesStatusChange)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 23, OnNewCall)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 24, OnServerShutDown)
    SINK_ENTRY_EX(0, DIID_IFaxServerNotify, 25, OnDeviceStatusChange)
END_SINK_MAP()

    //
    // Start of IFaxServerNotify2 methods
    //

    STDMETHOD(OnIncomingJobAdded)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrJobId);

    STDMETHOD(OnIncomingJobRemoved)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrJobId);

    STDMETHOD(OnIncomingJobChanged)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrJobId, 
        /*[in]*/ IFaxJobStatus *pJobStatus);

    STDMETHOD(OnOutgoingJobAdded)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrJobId);

    STDMETHOD(OnOutgoingJobRemoved)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrJobId);

    STDMETHOD(OnOutgoingJobChanged)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrJobId, 
        /*[in]*/ IFaxJobStatus *pJobStatus);

    STDMETHOD(OnIncomingMessageAdded)(
        /*[in]*/ IFaxServer2 *pFaxServer,
        /*[in]*/ BSTR bstrMessageId);

    STDMETHOD(OnIncomingMessageRemoved)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrMessageId);

    STDMETHOD(OnOutgoingMessageAdded)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ BSTR bstrMessageId);

    STDMETHOD(OnOutgoingMessageRemoved)(
        /*[in]*/ IFaxServer2 *pFaxServer,
        /*[in]*/ BSTR bstrMessageId);

    STDMETHOD(OnReceiptOptionsChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnActivityLoggingConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnSecurityConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnEventLoggingConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnOutgoingQueueConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnOutgoingArchiveConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnIncomingArchiveConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnDevicesConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnOutboundRoutingGroupsConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnOutboundRoutingRulesConfigChange)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnServerActivityChange)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ long lIncomingMessages,
        /*[in]*/ long lRoutingMessages, 
        /*[in]*/ long lOutgoingMessages, 
        /*[in]*/ long lQueuedMessages);

    STDMETHOD(OnQueuesStatusChange)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ VARIANT_BOOL bOutgoingQueueBlocked, 
        /*[in]*/ VARIANT_BOOL bOutgoingQueuePaused,
        /*[in]*/ VARIANT_BOOL bIncomingQueueBlocked);

    STDMETHOD(OnNewCall)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ long lCallId, 
        /*[in]*/ long lDeviceId, 
        /*[in]*/ BSTR bstrCallerId);

    STDMETHOD(OnServerShutDown)(/*[in]*/ IFaxServer2 *pFaxServer);

    STDMETHOD(OnDeviceStatusChange)(
        /*[in]*/ IFaxServer2 *pFaxServer, 
        /*[in]*/ long lDeviceId, 
        /*[in]*/ VARIANT_BOOL bPoweredOff, 
        /*[in]*/ VARIANT_BOOL bSending, 
        /*[in]*/ VARIANT_BOOL bReceiving,
        /*[in]*/ VARIANT_BOOL bRinging);

    STDMETHOD(OnGeneralServerConfigChanged)(/*[in]*/ IFaxServer2 *pFaxServer);
    //
    // End of IFaxServerNotify2 methods
    //
};

    
// _CFaxServerNotify is an abstract class because the IUnknown methods
// aren't implemented. Make use of CComObject & with the help of
// typedef a new (concrete) class CFaxServerNotify is created!!
typedef CComObject<_CFaxServerNotify> CFaxServerNotify;

#endif

