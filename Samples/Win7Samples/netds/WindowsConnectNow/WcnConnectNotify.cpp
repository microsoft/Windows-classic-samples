// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// Module Name:
//    WcnConnectNotify.cpp
//
// Abstract:
//  The WcnConnectNotify handles setting up an instance of the IWCNConnectNotification interface which 
//  will inform the caller when a WCN configuration has completed with either a success or failure.


#include "WcnConnectNotify.h"



//Use this interface to receive a success or failure notification when a Windows Connect Now connect session completes.
WcnConnectNotification::WcnConnectNotification() :
   connectEndEvent(0),
   connectSucceededCallBackInvoked(FALSE),
   connectFailedCallBackInvoked(FALSE)
   {
   }


//Initialize an event that the ConnectSucceeded and ConnectFailed call back can use
HRESULT WcnConnectNotification::Init()
{
	connectEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if(connectEndEvent == 0)
	{	
		wprintf(L"\nERROR: WcnConnectNotification::CreateEvent() failed with the following error[0x%p]",connectEndEvent);
		return E_FAIL;
	}
	return S_OK;
}

WcnConnectNotification::~WcnConnectNotification()
{

	if(connectEndEvent)
	{
		CloseHandle(connectEndEvent);
		connectEndEvent = 0;
	}
}


//The IWCNConnectNotify::ConnectSucceeded callback method that indicates a successful IWCNDevice::Connect operation.
HRESULT STDMETHODCALLTYPE WcnConnectNotification::ConnectSucceeded()
{
	connectSucceededCallBackInvoked = TRUE;

	// Tell the main thread to stop waiting; the connect has completed.
	SetEvent(connectEndEvent);

	return S_OK; // WCN ignores the return value
}


//A callback method that indicates a IWCNDevice::Connect operation failure.
HRESULT STDMETHODCALLTYPE WcnConnectNotification::ConnectFailed(HRESULT hrFailure)
{
	//
	// This sample doesn't attempt to use the specific error code, but you can 
	// look at the hrFailure code to help determine what went wrong.
	//
	// If the value is HRESULT_FROM_WIN32(ERROR_NOT_FOUND) or WCN_E_PEER_NOT_FOUND,
	// then the device didn't respond to the connection request.
	//
	// If the value is WCN_E_AUTHENTICATION_FAILED, then the device used an incorrect
	// password.
	//
	// If the value is WCN_E_CONNECTION_REJECTED, then the other device send a NACK,
	// and you can inspect the WCN_TYPE_CONFIGURATION_ERROR integer to see if it
	// sent an error code (if the code is WCN_VALUE_CE_DEVICE_PASSWORD_AUTH_FAILURE,
	// then the device detected that our password was incorrect).  However, not all
	// devices send an error code correctly, so be prepared to handle the case where
	// the code is WCN_VALUE_CE_NO_ERROR, even though there was actually  an error.
	//
	// If the value is WCN_E_SESSION_TIMEDOUT or HRESULT_FROM_WIN32(ERROR_TIMEOUT),
	// then the device took too long to respond.  Note that this sample does impose
	// its own connect timeout (in addition to the timeout built-in to the WCN API).
	//
	UNREFERENCED_PARAMETER(hrFailure);

	connectFailedCallBackInvoked = TRUE;

	// Tell the main thread to stop waiting; the connect has completed.
	SetEvent(connectEndEvent);   

	return S_OK; // WCN ignores the return value
}

//wait for the ConnectSucceeded or ConnectFailed call back to get called
HRESULT WcnConnectNotification::WaitForConnectionResult()
{
	HRESULT hr = S_OK;

	DWORD dwIndex = 0;
	hr = CoWaitForMultipleHandles(
									COWAIT_WAITALL, 
									CONNECT_TIME_OUT, 
									1, 
									&connectEndEvent, 
									&dwIndex);
	
	if (hr == S_OK && dwIndex == 0)
	{
		ResetEvent(connectEndEvent);
	}
	else
	{
		wprintf(L"\nDiscovery timeout (after waiting %ums).", CONNECT_TIME_OUT);
	}

	return hr;
}
