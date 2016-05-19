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

#include "FaxServerNotify.h"
#include "FaxNotify.h"


//+---------------------------------------------------------------------------
//
//  function:   _CFaxServerNotify
//
//  Synopsis:   _CFaxServerNotify constructor 
//
//  Arguments:  none
//
//  Returns:    void
//
//----------------------------------------------------------------------------
_CFaxServerNotify::_CFaxServerNotify()
{
    //
    // We do one add ref on construction so that we have full
    // control on the life-time of this object.
    // Without this, if an AddRef & Release is done on this object,
    // CComObject is going to delete the object. AddRef & Release
    // may happen indirectly when we pass a pointer to this object
    // to Advise & UnAdvise calls respectively
    //
    InternalAddRef(); // defined in CComObjectRootEx
}

//+---------------------------------------------------------------------------
//
//  function:   ~_CFaxServerNotify
//
//  Synopsis:   _CFaxServerNotify destructor 
//
//  Arguments:  none
//
//  Returns:    void
//
//----------------------------------------------------------------------------
_CFaxServerNotify::~_CFaxServerNotify()
{
    InternalRelease(); // defined in CComObjectRootEx
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingJobAdded
//
//  Synopsis:   Handles out job added event
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrJobId  : Job id of the added job
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnOutgoingJobAdded(
    IFaxServer2 *pFaxServer, 
    BSTR bstrJobId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: JobId in OnOutgoingJobAdded is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: OUTGOING JOB ADDED: %s \n"), bstrJobId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingJobRemoved
//
//  Synopsis:   Handles out job removed event
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrJobId  : Job id of the removed job
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnOutgoingJobRemoved(
    IFaxServer2 *pFaxServer, 
    BSTR bstrJobId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: JobId in OnOutgoingJobRemoved is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: OUTGOING JOB REMOVED: %s \n"), bstrJobId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingJobChanged
//
//  Synopsis:   Handles out job changed event
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrJobId  : Job id of the changed job
//              pJobStatus : Job status of the changed job
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------    
STDMETHODIMP _CFaxServerNotify::OnOutgoingJobChanged(
    IFaxServer2 *pFaxServer, 
    BSTR bstrJobId, 
    IFaxJobStatus *pJobStatus)
 {
    ValidateFaxServer(pFaxServer);
    if (bstrJobId == NULL || pJobStatus == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: JobId or JobStatus in OnOutgoingJobChanged is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: OUTGOING JOB CHANGED: %s \n"), bstrJobId);
    if (pJobStatus != NULL)
    {
        DisplayJobStatus(pJobStatus);
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnServerShutDown
//
//  Synopsis:   Handles server shutdown event. 
//              This is raised when fax service is stopped.  
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnServerShutDown(IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: SERVER SHUTDOWN \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingJobAdded
//
//  Synopsis:   Handles Incoming Job added event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrJobId  : Job id of the added job
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnIncomingJobAdded(
    IFaxServer2 *pFaxServer, 
    BSTR bstrJobId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: JobId in OnIncomingJobAdded is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: INCOMING JOB ADDED: %s \n"), bstrJobId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingJobRemoved
//
//  Synopsis:   Handles Incoming Job removed event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrJobId  : Job id of the removed job
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnIncomingJobRemoved(
    IFaxServer2 *pFaxServer, 
    BSTR bstrJobId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: JobId in OnIncomingJobRemoved is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: INCOMING JOB REMOVED: %s \n"), bstrJobId);  
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingJobChanged
//
//  Synopsis:   Handles Incoming Job changed event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrJobId  : Job id of the removed job
//              pJobStatus : Job status of the changed job
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnIncomingJobChanged(
    IFaxServer2 *pFaxServer, 
    BSTR bstrJobId, 
    IFaxJobStatus *pJobStatus)
{
   ValidateFaxServer(pFaxServer);
    if (bstrJobId == NULL || pJobStatus == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: JobId or JobStatus in OnIncomingJobChanged is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: INCOMING JOB CHANGED: %s \n"), bstrJobId);
    if (pJobStatus != NULL)
    {
        DisplayJobStatus(pJobStatus);
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingMessageAdded
//
//  Synopsis:   Handles Incoming Message added event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrMessageId  : Id of the added message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnIncomingMessageAdded(
    IFaxServer2 *pFaxServer, 
    BSTR bstrMessageId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: Id in OnIncomingMessageAdded is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: INCOMING MESSAGE ADDED: %s \n"), bstrMessageId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingMessageRemoved
//
//  Synopsis:   Handles Incoming Message removed event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrMessageId  : Id of the removed message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnIncomingMessageRemoved(
    IFaxServer2 *pFaxServer, 
    BSTR bstrMessageId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: Id in OnIncomingMessageRemovd is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: INCOMING MESSAGE REMOVED: %s \n"), bstrMessageId);
    return S_OK;    
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingMessageAdded
//
//  Synopsis:   Handles Outgoing Message Added event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrMessageId  : Id of the added message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnOutgoingMessageAdded(
    IFaxServer2 *pFaxServer, 
    BSTR bstrMessageId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: Id in OnOutgoingMessageAdded is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: OUTGOING MESSAGE ADDED: %s \n"), bstrMessageId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingMessageRemoved
//
//  Synopsis:   Handles Outgoing Message Removed event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bstrMessageId  : Id of the removed message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnOutgoingMessageRemoved(
    IFaxServer2 *pFaxServer, 
    BSTR bstrMessageId)
{
    ValidateFaxServer(pFaxServer);
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: Id in OnOutgoingMessageRemovd is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxServerNotify: OUTGOING MESSAGE REMOVED: %s \n"), bstrMessageId);
    return S_OK;   
}

//+---------------------------------------------------------------------------
//
//  function:   OnReceiptOptionsChange
//
//  Synopsis:   Handles Receipts Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxServerNotify::OnReceiptOptionsChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: RECEIPT OPTIONS CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnActivityLoggingConfigChange
//
//  Synopsis:   Handles Activity Logging Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnActivityLoggingConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: ACTIVITY LOGGING CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnSecurityConfigChange
//
//  Synopsis:   Handles Security Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnSecurityConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: SECURITY CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnEventLoggingConfigChange
//
//  Synopsis:   Handles Event Logging Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnEventLoggingConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: EVENT LOGGING CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingQueueConfigChange
//
//  Synopsis:   Handles Outgoing Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnOutgoingQueueConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: OUTGOING QUEUE CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingArchiveConfigChange
//
//  Synopsis:   Handles Outgoing Archive Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnOutgoingArchiveConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: OUTGOING ARCHIVE CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingArchiveConfigChange
//
//  Synopsis:   Handles Imcoming Archive Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnIncomingArchiveConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxServerNotify: INCOMING ARCHIVE CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnDevicesConfigChange
//
//  Synopsis:   Handles Devices Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnDevicesConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: DEVICES CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutboundRoutingGroupsConfigChange
//
//  Synopsis:   Handles Outbound Routing Groups Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnOutboundRoutingGroupsConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: OUTBOUND ROUTING GROUPS CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutboundRoutingRulesConfigChange
//
//  Synopsis:   Handles Outbound Routing Rules Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnOutboundRoutingRulesConfigChange(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: OUTBOUND ROUTING RULES CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnServerActivityChange
//
//  Synopsis:   Handles Server Activity change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              lIncomingMessages : Number of incoming messages
//              lRoutingMessages  : Number of Routing messages
//              lOutgoingMessages : Number of outgoing messages
//              lQueuedMessages   : Number of Queued messages
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnServerActivityChange(
    IFaxServer2 *pFaxServer, 
    long lIncomingMessages,
    long lRoutingMessages, 
    long lOutgoingMessages, 
    long lQueuedMessages)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: SERVER ACTIVITY CHANGE \n"));
    _tprintf(TEXT("FaxServerNotify: Incoming: %d, Routing: %d, Outgoing: %d, Queued: %d \n"),
    lIncomingMessages, lRoutingMessages, lOutgoingMessages, lQueuedMessages);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnQueuesStatusChange
//
//  Synopsis:   Handles Queue Status change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              bOutgoingQueueBlocked : Is outgoing queue blocked?
//              bOutgoingQueuePaused  : Is outgoing queue paused?
//              bIncomingQueueBlocked : Is incoming queue blocked?
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnQueuesStatusChange(
    IFaxServer2 *pFaxServer, 
    VARIANT_BOOL bOutgoingQueueBlocked, 
    VARIANT_BOOL bOutgoingQueuePaused,
    VARIANT_BOOL bIncomingQueueBlocked)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: QUEUE STATUS CHANGE \n"));
    _tprintf(TEXT("FaxServerNotify: Out Queue Blocked: %d, Out Queue Paused: %d, In Queue Blocked:%d \n"), 
        bOutgoingQueueBlocked, bOutgoingQueuePaused, bIncomingQueueBlocked);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnNewCall
//
//  Synopsis:   Handles new call event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              lCallId    : Specifies the new call's id
//              lDeviceId  : Specifies the device id of the device receiving 
//                           the incoming call
//              bstrCallerId : Null terminated string that identifies the 
//                             calling device for the new call
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnNewCall(
    IFaxServer2 *pFaxServer, 
    long lCallId, 
    long lDeviceId, 
    BSTR bstrCallerId)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: NEW CALL \n"));
    _tprintf(TEXT("FaxServerNotify: Call ID: %d, Device ID: %d, Caller ID: %s \n"), lCallId, lDeviceId, bstrCallerId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnDeviceStatusChange
//
//  Synopsis:   Handles device status change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//              lDeviceId  : Specifies the device id of the device 
//              bPoweredOff: True if the device powered off?
//              bSending   : True if device is sending a fax
//              bReceiving : True if device is receiving a fax
//              bRinging   : True if the device is in ringing state
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnDeviceStatusChange(
    IFaxServer2 *pFaxServer, 
    long lDeviceId, 
    VARIANT_BOOL bPoweredOff, 
    VARIANT_BOOL bSending, 
    VARIANT_BOOL bReceiving,
    VARIANT_BOOL bRinging)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: DEVICE STATUS CHANGE \n"));
    _tprintf( TEXT("FaxServerNotify: Device ID: %d, PoweredOff: %d, Send: %d, Receive: %d, Ringing: %d \n"),
        lDeviceId, bPoweredOff, bSending, bReceiving, bRinging);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnGeneralServerConfigChanged
//
//  Synopsis:   Handles Server Config change event. 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxServerNotify::OnGeneralServerConfigChanged(
    IFaxServer2 *pFaxServer)
{
    ValidateFaxServer(pFaxServer);
    _tprintf(TEXT("FaxServerNotify: GENERAL SERVER CONFIG CHANGE \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   ValidateFaxServer
//
//  Synopsis:   Checks if the FaxServer object is valid 
//
//  Arguments:  pFaxServer : Fax Server object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
inline HRESULT ValidateFaxServer(
    CComPtr<IFaxServer2> pFaxServer)
{
    if(pFaxServer == NULL)
    {
        _tprintf( TEXT("FaxServerNotify: pFaxServer is null \n"));
        return E_INVALIDARG;
    }
    FAX_SERVER_APIVERSION_ENUM apiVersion;
    HRESULT hr = S_OK;
    GET_SIMPLE_PROPERTY(pFaxServer, get_APIVersion, apiVersion, hr, exit);
exit:
    return hr;
}

