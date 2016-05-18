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

#include "FaxAccountNotify.h"
#include "FaxNotify.h"

//+---------------------------------------------------------------------------
//
//  function:   _CFaxAccountNotify
//
//  Synopsis:   _CFaxAccountNotify constructor 
//
//  Arguments:  none
//
//  Returns:    void
//
//----------------------------------------------------------------------------
_CFaxAccountNotify::_CFaxAccountNotify()
{
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
//  function:   ~_CFaxAccountNotify
//
//  Synopsis:   _CFaxAccountNotify destructor 
//
//  Arguments:  none
//
//  Returns:    void
//
//----------------------------------------------------------------------------
_CFaxAccountNotify::~_CFaxAccountNotify()
{
    InternalRelease(); // defined in CComObjectRootEx
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingJobAdded
//
//  Synopsis:   Handles out job added event
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrJobId  : Job id of the added job
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxAccountNotify::OnOutgoingJobAdded(
    IFaxAccount *pFaxAccount, 
    BSTR bstrJobId)
{
    ValidateFaxAccount(pFaxAccount);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: JobId in OnOutgoingJobAdded is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: OUTGOING JOB ADDED: %s \n"), bstrJobId);
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingJobRemoved
//
//  Synopsis:   Handles out job removed event
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrJobId  : Job id of the removed job
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
STDMETHODIMP _CFaxAccountNotify::OnOutgoingJobRemoved(
    IFaxAccount *pFaxAccount, 
    BSTR bstrJobId)
{
    ValidateFaxAccount(pFaxAccount);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: JobId in OnOutgoingJobRemoved is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: OUTGOING JOB REMOVED: %s \n"), bstrJobId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingJobChanged
//
//  Synopsis:   Handles out job changed event
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrJobId  : Job id of the changed job
//              pJobStatus : Job status of the changed job
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------     
STDMETHODIMP _CFaxAccountNotify::OnOutgoingJobChanged(
    IFaxAccount *pFaxAccount, 
    BSTR bstrJobId, 
    IFaxJobStatus *pJobStatus)
{    
    ValidateFaxAccount(pFaxAccount);   
    if (bstrJobId == NULL || pJobStatus == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: JobId or JobStatus in OnOutgoingJobChanged is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: OUTGOING JOB CHANGED: %s \n"), bstrJobId);
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
STDMETHODIMP _CFaxAccountNotify::OnServerShutDown(IFaxServer2 *pFaxServer)
{    
    ValidateFaxServer(pFaxServer);
    _tprintf( TEXT("FaxAccountNotify: SERVER SHUTDOWN \n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingJobAdded
//
//  Synopsis:   Handles Incoming Job added event. 
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrJobId  : Job id of the added job
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxAccountNotify::OnIncomingJobAdded(
    IFaxAccount *pFaxAccount, 
    BSTR bstrJobId)
{
    ValidateFaxAccount(pFaxAccount);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: JobId in OnIncomingJobAdded is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: INCOMING JOB ADDED: %s \n"), bstrJobId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingJobRemoved
//
//  Synopsis:   Handles Incoming Job removed event. 
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrJobId  : Job id of the added job
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------  
STDMETHODIMP _CFaxAccountNotify::OnIncomingJobRemoved(
    IFaxAccount *pFaxAccount, 
    BSTR bstrJobId)
{
    ValidateFaxAccount(pFaxAccount);
    if (bstrJobId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: JobId in OnIncomingJobRemoved is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: INCOMING JOB REMOVED: %s \n"), bstrJobId);  
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingJobChanged
//
//  Synopsis:   Handles Incoming Job changed event. 
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrJobId  : Job id of the removed job
//              pJobStatus : Job status of the changed job
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxAccountNotify::OnIncomingJobChanged(
    IFaxAccount *pFaxAccount, 
    BSTR bstrJobId, 
    IFaxJobStatus *pJobStatus)
{
    ValidateFaxAccount(pFaxAccount);
    if (bstrJobId == NULL || pJobStatus == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: JobId or JobStatus in OnIncomingJobChanged is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: INCOMING JOB CHANGED: %s \n"), bstrJobId);
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
//  Arguments:  pFaxAccount : Fax Account object
//              bstrMessageId  : Id of the added message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxAccountNotify::OnIncomingMessageAdded(
    IFaxAccount *pFaxAccount, 
    BSTR bstrMessageId,
    VARIANT_BOOL fAddedToReceiveFolder)
{
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: Id in OnIncomingMessageAdded is NULL \n"));
        return E_INVALIDARG;
    }
	if(fAddedToReceiveFolder == VARIANT_TRUE)
		_tprintf( TEXT("FaxAccountNotify: Message added to Server Inbox \n"));
    else
        _tprintf( TEXT("FaxAccountNotify: Message is added to User Inbox \n"));

    _tprintf( TEXT("FaxAccountNotify: INCOMING MESSAGE ADDED: %s \n"), bstrMessageId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnIncomingMessageRemoved
//
//  Synopsis:   Handles Incoming Message removed event. 
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrMessageId  : Id of the removed message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxAccountNotify::OnIncomingMessageRemoved(
    IFaxAccount *pFaxAccount, 
    BSTR bstrMessageId,
    VARIANT_BOOL fRemovedFromReceiveFolder)
{
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: Id in OnIncomingMessageRemovd is NULL \n"));
        return E_INVALIDARG;
    }       
	if(fRemovedFromReceiveFolder == VARIANT_TRUE) 
		_tprintf( TEXT("FaxAccountNotify: Message is removed from Server Inbox \n"));
    else
        _tprintf( TEXT("FaxAccountNotify: Message is removed from User Inbox \n"));
    _tprintf( TEXT("FaxAccountNotify: INCOMING MESSAGE REMOVED: %s \n"), bstrMessageId);
    return S_OK;    
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingMessageAdded
//
//  Synopsis:   Handles Outgoing Message Added event. 
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrMessageId  : Id of the added message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxAccountNotify::OnOutgoingMessageAdded(
    IFaxAccount *pFaxAccount, 
    BSTR bstrMessageId)
{
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: Id in OnOutgoingMessageAdded is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: OUTGOING MESSAGE ADDED: %s \n"), bstrMessageId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  function:   OnOutgoingMessageRemoved
//
//  Synopsis:   Handles Outgoing Message Removed event. 
//
//  Arguments:  pFaxAccount : Fax Account object
//              bstrMessageId  : Id of the removed message
//
//  Returns:    S_OK if successful
//
//---------------------------------------------------------------------------- 
STDMETHODIMP _CFaxAccountNotify::OnOutgoingMessageRemoved(
    IFaxAccount *pFaxAccount, 
    BSTR bstrMessageId)
{
    if (bstrMessageId == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: Id in OnOutgoingMessageRemovd is NULL \n"));
        return E_INVALIDARG;
    }        
    _tprintf( TEXT("FaxAccountNotify: OUTGOING MESSAGE REMOVED: %s \n"), bstrMessageId);
    return S_OK;   
}

//+---------------------------------------------------------------------------
//
//  function:   ValidateFaxAccount
//
//  Synopsis:   Checks if the FaxAccount object is valid 
//
//  Arguments:  pFaxAccount : Fax Account object
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
inline HRESULT ValidateFaxAccount(
    CComPtr<IFaxAccount> pFaxAccount)
{
    if(pFaxAccount == NULL)
    {
        _tprintf( TEXT("FaxAccountNotify: pFaxAccount is null \n"));
        return E_INVALIDARG;
    }
    CComBSTR bstrAccountName = NULL;
    HRESULT hr = S_OK;
    GET_SIMPLE_PROPERTY(pFaxAccount, get_AccountName, bstrAccountName, hr, exit);
exit:
    return hr;
}

