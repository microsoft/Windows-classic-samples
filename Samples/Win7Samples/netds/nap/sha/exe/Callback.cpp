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

#include "SDKCommon.h"
#include "Callback.h"

using namespace SDK_SAMPLE_COMMON;
using namespace SDK_SAMPLE_SHA;

SDK_SHA_FIXUPSTATE g_SdkShaHealthState = NOFIXESNEEDED;
BOOL g_setHealthySoh = true;


// Create instance
IShaCallbackPtr ShaCallback::CreateInstance(INapSystemHealthAgentBinding* pBinding) throw ()
{
    CComObject<ShaCallback>* pTemp = NULL;
    HRESULT hr = CComObject<ShaCallback>::CreateInstance(&pTemp);
    if (FAILED(hr) || (!pTemp))
    {
        wprintf(L"\nShaCallback::CreateInstance: Failed to create Callback instance (error = %x) \n", hr);
        goto Cleanup;
    }
    pTemp->m_pBinding = pBinding;
    Cleanup:
        return IShaCallbackPtr(pTemp);
}


// Constructor
ShaCallback::ShaCallback() throw ():m_pBinding(NULL)
{
}



// Destructor
ShaCallback::~ShaCallback() throw ()
{
}

//
// SDK Note:
//      This callback is used by the NAPAgent to request a Statement of Health (SoH) from
//      the System Health Agent (SHA).  An SHA must implement this interface, and within it,
//      it must construct its own SoH 'blob' and pass it back to the NAPAgent
//      This data will then be communicated to the NAP Server for evaluation.
// See MSDN for latest documentation on this function.
//
STDMETHODIMP ShaCallback::
    GetSoHRequest(INapSystemHealthAgentRequest* pRequest) throw ()
{
    HRESULT hr = S_OK;
    wprintf(L"\nShaCallback::GetSoHRequest(): called by NAPAgent\n");

    INapSoHConstructor *pISohConstructor = NULL;
    SoHRequest *pSohRequest = NULL;

    // sanity check - ensure that the passed request pointer is not NULL
    if (!pRequest)
    {
        hr = E_POINTER;
        wprintf(L"\nShaCallback::GetSoHRequest : received NULL Request pointer\n");
        goto Cleanup;
    }

    //
    // Create an SoH constructor interface.
    // Passing in our SystemHealth ID, and the type of SoH we want to create (request vs response)
    //
    hr = CreateOutputSoHConstructor(pISohConstructor, QuarSampleSystemHealthId, SOH_REQUEST);
    if (FAILED(hr))
    {
        wprintf(L"\nShaCallback::GetSoHRequest: CreateOutputSoHConstructor() failed (error = 0x%08x)\n", hr);
        goto Cleanup;
    }


    //
    // Populate the data into the SoH Constructor
    //
    hr = FillSoHRequest(&pISohConstructor);
    if (FAILED(hr))
    {
        wprintf(L"\nShaCallback::GetSoHRequest(): FillSoHRequest() failed (error = 0x%08x)\n", hr);
        goto Cleanup;
    }


    //
    // Get the constructed SoH data from the SoH Constructor
    //
    hr = pISohConstructor->GetSoH(&pSohRequest);
    if (FAILED(hr))
    {
        wprintf(L"\nShaCallback::GetSoHRequest(): SoHConstructor->GetSoH failed (error = 0x%08x)\n", hr);
        goto Cleanup;
    }

    //
    // Ensure that NULL was not passed back
    //
    if (! pSohRequest)
    {
        hr = E_POINTER;
        wprintf(L"\nShaCallback::GetSoHRequest(): Final Request SoH data pointer was NULL!\n");
        goto Cleanup;
    }

    //
    // Pass the SoH data out via the Request object that was passed in
    //
    hr = pRequest->SetSoHRequest(pSohRequest, TRUE);
    if (FAILED(hr))
    {
        wprintf(L"\nShaCallback::GetSoHRequest(): pShaRequest->SetSoHRequest failed (error = 0x%08x)\n", hr);
    }


    Cleanup:
        ReleaseObject(pISohConstructor);
		::FreeSoH(pSohRequest);
        return hr;
}


//
// SDK Note:
// This callback is used by the NAP Agent to pass a received SoH Response to the
// SHA for processing.  It is called whenever the NAP Agent has an SoH response that
// is destined for this SHA.
// See MSDN for latest documentation on this function.
STDMETHODIMP ShaCallback::
    ProcessSoHResponse(INapSystemHealthAgentRequest* pResponse) throw ()
{
    wprintf(L"\nShaCallback::ProcessSoHResponse() called by NAPAgent\n");
    HRESULT hr = S_OK;
    SoHResponse* pSohResponse = NULL;
    INapSoHProcessor *pSohProcessor = NULL;
    SystemHealthEntityId systemHealthId = 0;
    UINT8 flags = 0;
    BOOL doFixup = FALSE;

    // sanity check - ensure that the passed response pointer is not NULL
    if (!pResponse)
    {
       hr = E_POINTER;
       wprintf(L"\nShaCallback::ProcessSoHResponse  NULL INapSystemHealthAgentRequest pointer\n");
       goto Cleanup;
    }

    //
    // Get the response SoH from NAPAgent, as well as the flags (see below)
    //
    hr = pResponse->GetSoHResponse(&pSohResponse, &flags);
    if (FAILED(hr))
    {
       // Couldn't get the response Log an error and return
       wprintf(L"\nShaCallback::ProcessSoHResponse():pResponse->GetSoHResponse failed (error = 0x%08x)\n", hr);
       goto Cleanup;
    }

    // Determine whether to attempt auto-remediation if fixes are needed
    if ((flags & shaFixup) == shaFixup)
    {
        //Instructed to do fixup
        doFixup = TRUE;
    }
    else
    {
        //not instructed to do fixup
        doFixup = FALSE;
    }

    //
    // Wrap it inside an INapSoHProcessor object.
    // This will handle parsing of the SoH data for us, making it easier to retrieve the
    // desired data from the SoH.
    //
    hr = CreateInputSoHProcessor(pSohProcessor,
                                                    systemHealthId,
                                                    SOH_RESPONSE, pSohResponse);
    if (FAILED(hr))
    {
       wprintf(L"\nShaCallback::ProcessSoHResponse: CreateInputSoHProcessor(): Couldn't create an SoH Parser object! (error %#x)\n", hr);
       goto Cleanup;
    }


    //
    // SDK Note:
    // In this sample, we set some flags to notify what action this SHA needs to take, then
    // other callbacks use these flags to generate notification output to the console.
    // In a real SHA, one would want to somehow notify a worker thread that updates are
    // needed so that that worker can perform the updates
    //
    hr = HandleSoHResponse(pSohProcessor, systemHealthId, doFixup);
    if (FAILED(hr))
    {
       wprintf(L" ShaCallback::ProcessSoHResponse():HandleSoHResponse failed (error = 0x%08x)\n", hr);
       goto Cleanup;
    }


Cleanup:
    ReleaseObject(pSohProcessor);
    FreeSoH(pSohResponse);
    return hr;
}


//
// SDK Note:
// This callback is used by the NAPAgent to notify SHAs that the system's NAP 
// isolation state has changed.  If an SHA has activities to be done when state
// changes, it could be triggered here
// See MSDN for latest documentation on this function.
//
STDMETHODIMP ShaCallback::NotifySystemIsolationStateChange() throw ()
{
    wprintf(L"ShaCallback::NotifySystemIsolationStateChange() called by NAPAgent\n");

    HRESULT hr = S_OK;
    IsolationInfo* pStatus = NULL;
    BOOL unknownConnections = FALSE;

    // ensure that the Binding object is set before attempting to use it
    if (!m_pBinding)
    {
        wprintf(L" ShaCallback::NotifySystemIsolationStateChange():m_pBinding is NULL");
        goto Cleanup;
    }

    //
    // SDK Note:
    // The health agent can query the system quarantine state
    // using the Binding::GetSystemIsolationInfo()
    //
    hr = m_pBinding->GetSystemIsolationInfo(&pStatus, &unknownConnections);
    if (FAILED(hr))
    {
       wprintf(L" ShaCallback::NotifySystemIsolationStateChange():m_pBinding->GetSystemIsolationInfo() failed (error = 0x%08x)\n",
                   hr);
       goto Cleanup;
    }

    //
    // for this example, we'll just output the current state to the console
    //
    wprintf(L"System quar state = %u, prob time = %x %x, url = %s, any_connections_in_unknown_state? = %u\n",
                 pStatus->isolationState,
                 pStatus->probEndTime.dwHighDateTime, pStatus->probEndTime.dwLowDateTime,
                 (pStatus->failureUrl.string == 0) ? 0 : pStatus->failureUrl.string,
                 unknownConnections);

    Cleanup:
        FreeIsolationInfo(pStatus);
        return hr;
}

//
// SDK Note:
// This callback is used by the NAPAgent to query the SHA for its current fixup status
// while that SHA is processing an SoH Response
// See MSDN for latest documentation on this function.
//
STDMETHODIMP ShaCallback::GetFixupInfo(FixupInfo** ppStatus) throw ()
{
    wprintf(L"\nShaCallback::GetFixupInfo() called by NAPAgent\n");
    HRESULT hr = S_OK;

    if (ppStatus == NULL)
    {
        hr = E_POINTER;
        wprintf(L"\nShaCallback::GetFixupInfo() bad pointer for ppStatus passed in\n");
        goto Cleanup;
    }

    // The caller should free this memory using CoTaskMemFree
    hr = AllocFixupInfo(ppStatus, NUMBER_OF_HRESULTS);
    if (FAILED(hr))
    {
       goto Cleanup;
    }

    // Fill ppStatus
    // SDK Note:
    // This should be filled according to the current Fix up status
    // This is a simple example and only returns 1 result code
    switch (g_SdkShaHealthState)
    {
        case NOFIXESNEEDED:
                // In this example, this case will be hit when the client is healthy
                wprintf(L"ShaCallback::GetFixupInfo: Returning HealthState of NOFIXESNEEDED");
                (*ppStatus)->fixupMsgId = SDK_SAMPLE_GENERIC_FIXUP_SUCCESS_MSG_ID;
                (*ppStatus)->percentage = 100;
                (*ppStatus)->state = fixupStateSuccess;
                break;

        case FIXESINPROGRESS:
                // In this example, this case will be hit when unhealthy, but only when the
                //'auto-remediate' flag is passed back from the NAP Server
                //
                // SDK Note:
                // The 'auto-remediate' flag indicates that the SHA should
                // automatically attempt to fix the client's health state, if possible.
                //
                wprintf(L"ShaCallback::GetFixupInfo: Returning HealthState of FIXESINPROGRESS");
               (*ppStatus)->fixupMsgId = SDK_SAMPLE_GENERIC_FIXUP_INPROGRESS_MSG_ID;
               (*ppStatus)->percentage = 50;
               (*ppStatus)->state = fixupStateInProgress;
               break;

        case FIXESNEEDED:
        default:
                // In this example, this case will be hit when unhealthy and 'auto-remediate'
                // is NOT indicated by NAP Server
                //
                // SDK Note:
                // The 'auto-remediate' flag indicates that the SHA should
                // automatically attempt to fix the client's health state, if possible.  If it is NOT
                // set, then the SHA should not update the client, but should leave it in its
                // current state.
                //
                wprintf(L"ShaCallback::GetFixupInfo: Returning HealthState of FIXESNEEDED");
               (*ppStatus)->fixupMsgId = SDK_SAMPLE_CLIENT_NOT_PATCHED_MSG_ID;
               (*ppStatus)->percentage = percentageNotSupported;
               (*ppStatus)->state = fixupStateCouldNotUpdate;
                break;
    }
    
    (*ppStatus)->resultCodes.count = NUMBER_OF_HRESULTS;   // 1 HRESULT
    *((*ppStatus)->resultCodes.results) = S_OK;


    Cleanup:
        return hr;
}


// Fill the SoH with the parameters
HRESULT ShaCallback::
    FillSoHRequest(INapSoHConstructor **  ppISohRequest) throw()
{
    HRESULT hr = S_OK;
    SoHAttributeValue value = {0};

    //
    // Set up the SDK SHA's Vendor Specific data attribute
    //
    value.vendorSpecificVal.vendorId = QuarSampleSystemHealthId;
    value.vendorSpecificVal.size = SDK_CLIENT_VENDOR_DATA_SIZE;

    //
    // SDK Note:
    // Append any optional attributes appropriate for the given
    // business logic. (I.e., Health-Class, Software-Version,
    // Time-of-Last-Update, etc.)
    // The data sent will be specific to the Health Validator/Agent pair in question

    // if sending healthy SoH, set healthy data, otherwise set unhealthy data
    if (g_setHealthySoh)
    {
        wprintf(L"\nShaCallback::FillSoHRequest(): Setting outbound SoH data: Healthy\n");
        value.vendorSpecificVal.vendorSpecificData = (PBYTE)&SDK_CLIENT_VENDOR_DATA_HEALTHY;
    }
    else
    {
        wprintf(L"\nShaCallback::FillSoHRequest(): Setting outbound SoH data: Unhealthy\n");
        value.vendorSpecificVal.vendorSpecificData = (PBYTE)&SDK_CLIENT_VENDOR_DATA_UNHEALTHY;
    }

    //
    // add the data to the passed in SoH Constructor
    //
    hr = (*ppISohRequest)->AppendAttribute(sohAttributeTypeVendorSpecific,&value);
    if (FAILED(hr))
    {
        wprintf(L"\nShaCallback::FillSoHRequest(): pISohRequest->AppendVendorSpecificAttribute(sohAttributeTypeVendorSpecific) (error = 0x%08x)\n", hr);
    }


   return hr;
}


// Called from within this implementation of ProcessSoHResponse, this function wraps up
// the actual evaluation of the SoH data to determine the health state as indicated by
// the NAP Server's response
HRESULT ShaCallback::
    HandleSoHResponse(
                                                INapSoHProcessor * pSohProcessor,
                                                SystemHealthEntityId  systemHealthId,
                                                BOOL doFixup
                                                ) throw()
{
    HRESULT hr = S_OK;

    UINT16        index      = 0;
    SoHAttributeType   attrType   = (SoHAttributeType) 0;
    SoHAttributeValue *pAttrValue = NULL;

    //
    // SDK Note:
    // Each vendor should implement this section based upon their business logic and
    // on the state indicated by the SoH Response data
    //

    // sanity check - Ensure that an SoH Processor object was passed in
    if (! pSohProcessor)
    {
        // don't change our state if we can't read the SoH
        hr = NAP_E_MISSING_SOH;
        wprintf(L"\nShaCallback::HandleSoHResponse missing SoH Processor");
        goto Cleanup;
    }

    hr = pSohProcessor->FindNextAttribute(0,
                                     sohAttributeTypeComplianceResultCodes,
                                     &index);
    if (FAILED(hr))
    {
        // don't change our state if we can't read the SoH
        wprintf(L"\nShaCallback::HandleSoHResponse(): Failed to get sohAttributeTypeComplianceResultCodes (error = 0x%08x)\n", hr);
        goto Cleanup;
    }


    hr = pSohProcessor->GetAttribute(index, &attrType, &pAttrValue);
    if (FAILED(hr))
    {
        // don't change our state if we can't read the SoH
        wprintf(L"\nShaCallback::HandleSoHResponse(): GetAttribute failed (error = 0x%08x)\n", hr);
        goto Cleanup;
    }

    // for this sample, only reading 1 attribute
    // if more or less attributes are returned from the NAP Server via NAP Agent, then
    // this sample SHA will fail out.
    if ( NUMBER_OF_HRESULTS != pAttrValue->codesVal.count )
    {
        hr = NAP_E_INVALID_PACKET;
        wprintf(L"\nShaCallback::HandleSoHResponse(): Incorrect number of results returned from NAP Server\n");
        goto Cleanup;
    }

    // SDK Note:
    // in this example will handle the first result
    // each vendor can have as many results as needed
    // The logic will vary from vendor to another, so according to the result they should act
    // for example if the result code = QUAR_E_NOTPATCHED and auto-remediate is set,
    // SHA may attempt to install patches, etc
    HRESULT resultCode = pAttrValue->codesVal.results[0];
    if (FAILED(resultCode))
    {
        //
        // Update a flag that fixes are required
        // Fixes should be handled by a separate thread, so as not to block
        // the NAP system.
        // The NAPAgent will query fixup state via the GetFixupInfo callback.
        //
        wprintf(L"Client needs fixes (Result code = %x)\n", resultCode);
        g_SdkShaHealthState = FIXESNEEDED;

        if (TRUE == doFixup)
        {
            wprintf(L"Client needs fixes and instructed to auto-remediate (Result code = %x)\n", resultCode);
            g_SdkShaHealthState = FIXESINPROGRESS;
        }

    }
    else
    {
        wprintf(L"Compliant machine (Result code = %x)\n", resultCode);
        g_SdkShaHealthState = NOFIXESNEEDED;
    }


    //
    // SDK Note:
    // This method must not hold references to the request object once
    // this call has completed.
    //
    // After saving the SoHResponse,  ProcessSoHResponse() should notify a
    // worker thread, and return immediately
    //
    // The worker thread would be responsible for all fixup work
    // Currently, this sample SHA is single-threaded, so it acts as a
    // synchronous, blocking call, which returns to the QA once it has
    // completed all necessary fixup work.
    //
    // When writing a multithreaded SHA, the behavior is somewhat different:
    // When the NAP Agent calls into this SHA's ProcessSoHResponse(),
    // the SHA will start a worker thread, then exit immediately.
    // The worker thread will perform all fixup steps, and should
    // indicate its percentage-complete status in a FixupStatus value.  The
    // NAP Agent will call <GetFixupInfo> to query this status, to determine
    // what to display to the user in the NAP Status window.
    //

    Cleanup:
        FreeSoHAttributeValue(attrType,pAttrValue);
        return hr;
}


//
// SDK Note:
// This callback is used by the NAPAgent when determining whether a new NAP transaction
// is required due to a system state change.
// The SHA should compare the SoHs and return isEqual as TRUE if the SoHs are semantically
// equal.
// See MSDN for latest documentation on this function.
//
STDMETHODIMP  ShaCallback::CompareSoHRequests(
   /* in */ const SoHRequest* lhs,
   /* in */ const SoHRequest* rhs,
   /* out */ BOOL* isEqual
   ) throw ()
{
    HRESULT hr = S_OK;
    wprintf(L"\nShaCallback::CompareSoHRequests() called by NAPAgent\n");


    // SDK Note:
    // If SHAs have put incremental IDs or time-stamps into their SoH, then 2 SoHs may
    // be semantically equal (i.e. they may convey the same health information), but they may
    // be byte-wise unequal. SHAs should be careful to implement this function such that they
    // check for semantic equality on SoHs.
    //
    // If SHAs have not put any time-stamps or Ids into their SoH, they may choose to not
    // implement this function and return E_NOTIMPL.  In this case, the NapAgent performs
    // a byte-wise comparison on the SoHRequests.

    UNREFERENCED_PARAMETER(lhs);
    UNREFERENCED_PARAMETER(rhs);
    UNREFERENCED_PARAMETER(isEqual);
    
    wprintf(L"ShaCallback::CompareSoHRequest: SHA does not support CompareSoHRequests API, so QA will compare bytes on its own\n");
    hr = E_NOTIMPL;

    return hr;
}


//
// This callback is used by the NAPAgent to notify the SHA that an outstanding SoHRequest
// did not receive a corresponding SoHResponse.  The SHA may use this to clean up state
// However, this is a best-effort notification, and may not be called in all cases.  Therefore,
// an SHA should not rely entirely upon this callback for cleaning up state.
// See MSDN for latest documentation on this function.
//
STDMETHODIMP  ShaCallback::NotifyOrphanedSoHRequest(
   /* in */ const CorrelationId* correlationId
   ) throw ()
{
   HRESULT hr = S_OK;

   wprintf(L"NapAgent notified about OrphanedSohRequest (no response received for previously generated SoHRequest)\n");
   UNREFERENCED_PARAMETER(correlationId);

   return hr;
}
