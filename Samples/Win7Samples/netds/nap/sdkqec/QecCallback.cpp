// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#include <napprotocol.h>
#include <NapMicrosoftVendorIds.h>
#include <NapError.h>
#include <NapUtil.h>

#include <stdio.h>

#include "QecCallback.h"

using namespace SDK_SAMPLE_QEC;
using namespace SDK_SAMPLE_COMMON;


const GUID CONN_ID =
{ 0xfab24754, 0x440, 0x439b, { 0xa2, 0x23, 0xb1, 0xd3, 0x60, 0x6, 0x2d, 0x00 } };


// Given a byte buffer, prints out dwByteCount bytes in the format of:
//      Each line prefixed with 'dwIndentCount' blank spaces
//      xxyyzzaabbccmmnn-xxyyzzaabbccmmnn
//      xxyyzzaabbccmmnn-xxyyzzaabb...

void PrintByte(BYTE* pByte, DWORD dwByteCount, DWORD dwIndentCount) throw()

{
   DWORD i = 0, j = 0, k = 0;
   if (pByte == NULL || dwByteCount == 0)
   {
      wprintf(L"It is empty\n");
      goto Cleanup;
   }

   for( ; i < dwByteCount; )
   {
      for( j = 0; j < dwIndentCount; ++j)
         wprintf( L" " );
      for( ; i < dwByteCount && k < 8; ++i, ++k )
         wprintf( L"%2.2X ", pByte[i] );
      if( i < dwByteCount )
         wprintf( L"- " );
      for( ; i < dwByteCount && k < 16; ++i, ++k )
         wprintf( L"%2.2X ", pByte[i] );
      wprintf( L"\n" );
      k = 0;
   }
Cleanup:
   return;
}





// Create instance of the QEC Callback
IQecCallbackPtr QecCallback::CreateInstance(INapEnforcementClientBinding* pBinding) throw()
{
    CComObject<QecCallback>* pTemp = NULL;
    HRESULT hr = CComObject<QecCallback>::CreateInstance(&pTemp);
    if (FAILED(hr) || (!pTemp))
    {
        wprintf(L"\nQecCallback::CreateInstance: Failed to create Callback instance (error = %x) \n", hr);
        goto Cleanup;
    }
    pTemp->m_pBinding = pBinding;
    Cleanup:
        return IQecCallbackPtr(pTemp);
}


// Constructor
QecCallback::QecCallback() throw ():m_pBinding(NULL)
{
    m_retriggerHint = FALSE;
    m_pConnection = NULL;
}



// Destructor
QecCallback::~QecCallback() throw ()
{
}


// NapAgent informs the enforcement client of SoH changes
// using this method.
//
// See MSDN for latest documentation on this function.
//
STDMETHODIMP QecCallback::NotifySoHChange() throw()
{
    HRESULT hr = S_OK;
    NetworkSoHRequest *networkSohRequest = NULL;
    wprintf(L"\nQA called QecCallback::NotifySoHChange()\n");

    //
    // SDK Note:
    // Tasks a QEC should perform when NotifySoHChange is called:
    //      1) get the SoH Request for each active connection from NAPAgent
    //          via Binding::GetSoHRequest
    //                  (this task MAY be performed on the same thread as NotifySoHChange)
    //      2) send the SoH Request to the NAP Server using the method the QEC uses
    //          (DHCP, VPN, 802.1x, etc)
    //      3) wait for the SoH Response communication
    //      4) call Binding::ProcessSoHResponse to pass the SoH Response data up to NAPAgent
    //      5) enforce the network access state indicated
    //          if the SoH is valid, the NAPAgent will indicate the connection state to be enforced by
    //          setting the decision on the passed connection object before ProcessSoHResponse ends
    //
    //  Typically, these tasks will be performed by worker threads, so as not to block
    //  the callbacks made by the NAPAgent.
    //

    // very simple example - connections would not typically be created here
    // If a connection doesn't exist already, create one.
    // Must have a connection to retrieve an SoH
    hr = CreateConnectionObject();
    if (FAILED(hr))
    {
        wprintf(L"\nQecCallback::NotifySoHChange(): call to CreateConnectionObject() failed (error = 0x%08x)\n",hr);
        goto Cleanup;
    }
    
    //  Call GetSohRequest.
    hr = m_pBinding->GetSoHRequest(m_pConnection, &m_retriggerHint);
    if (FAILED(hr))
    {
        wprintf(L"\nQecCallback::NotifySoHChange(): GetSoHRequest failed (error = 0x%08x)\n",hr);
        goto Cleanup;
    }

    //  Obtain SOHRequest and print it
    hr = m_pConnection->GetSoHRequest(&networkSohRequest);
    if (FAILED(hr))
    {
        wprintf(L"\nQecCallback::NotifySoHChange(): GetSoHRequest on connection failed (error = 0x%08x)\n",hr);
        goto Cleanup;
    }

    if (!networkSohRequest)
    {
        wprintf(L"\nQecCallback::NotifySoHChange(): networkSohRequest pointer invalid\n");
        goto Cleanup;
    }

        wprintf(L"\nQecCallback::NotifySoHChange(): Rcvd NetworkSohReq of Size: %#x\n",networkSohRequest->size);
        PrintByte(networkSohRequest->data, networkSohRequest->size, 2);

Cleanup:
    wprintf(L"\nQecCallback::NotifySoHChange() Exit\n");
    return hr;
}



// To update the system isolation state, the NapAgent
// needs to know all the connections maintained by
// enforcers. NapAgent uses this method to get connection
// information. Enforcement clients must provide only a
// list of connections that are UP, and may provide a list
// of zero (0) connections, depending on the enforcement
// technology.
//
// See MSDN for latest documentation on this function.
//
// Parameters:-
// connectionList: list of connection objects.
STDMETHODIMP QecCallback::GetConnections(/* out */ Connections** connections) throw()
{
// Allocate & Construct a list of connections and pass it back
    HRESULT hr = S_OK;

    wprintf(L"\nQA called QecCallback::GetConnections()\n");

    // before allocate, ensure valid pointer
    if (NULL == connections)
    {
        hr = E_INVALIDARG;
        wprintf(L"\nQecCallback::GetConnections(): bad connections pointer passed in (error = 0x%08x)\n", hr);
        goto Cleanup;
    }
    *connections = NULL;

    // do we already have connections to return?
    if (NULL == m_pConnection)
    {
        //no connections to return, allocating empty structure

        hr = AllocConnections(connections, 0);
        if (FAILED(hr))
        {
            wprintf(L"\nQecCallback::GetConnections(): AllocConnections(0) failed (error = 0x%08x)\n",hr);
            goto Cleanup;
        }

        wprintf(L"\nQecCallback::GetConnections(): no connections exist, returning list of 0");
        (*connections)->count = 0;
        (*connections)->connections = NULL;

    }
    else  // m_pConnection exists
    {
        // private connection already created.
        // since this sample QEC only supports 1 connection,
        // only allocating and returning 1 connection
        hr = AllocConnections(connections, 1);
        if (FAILED(hr))
        {
            wprintf(L"\nQecCallback::GetConnections(): AllocConnections(1) failed (error = 0x%08x)\n",hr);
            goto Cleanup;
        }

        (*connections)->count = 1;
        (*connections)->connections[0] = m_pConnection;

        // add a reference to the connection prior to passing it out
        m_pConnection->AddRef();
    }
    
Cleanup:
    wprintf(L"\nQecCallback::GetConnections() Exit\n");
    return hr;
}




HRESULT QecCallback::CreateConnectionObject () throw()
{
    HRESULT hr = S_OK;

    if (!m_pConnection)
    {
        hr = m_pBinding->CreateConnection(&m_pConnection);
        if (FAILED(hr))
        {
            wprintf(L"\n QecCallback::CreateConnectionObject(): CreateConnection failed (error = 0x%08x)\n",hr);
            goto Cleanup;
        }

        hr = m_pConnection->SetConnectionId((ConnectionId *)&CONN_ID);
        if (FAILED(hr))
        {
            wprintf(L"\n QecCallback::CreateConnectionObject(): SetConnectionId failed (error = 0x%08x)\n",hr);
            goto Cleanup;
        }
        wprintf(L"\n QecCallback::CreateConnectionObject(): Connection object created \n");
    }
    else // m_pConnection is already set
    {
        wprintf(L"\n QecCallback::CreateConnectionObject(): Connection object already exists\n");
    }

Cleanup:
    wprintf(L"\nQecCallback::CreateConnectionObject() Exit\n");
    return hr;
}



