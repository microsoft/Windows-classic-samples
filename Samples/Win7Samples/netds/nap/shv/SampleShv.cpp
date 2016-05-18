// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#include <NapUtil.h>
#include <NapTypes.h>
#include <NapProtocol.h>
#include <NapMicrosoftVendorIds.h>
#include <NapSystemHealthValidator.h>
#include <NapError.h>
#include <strsafe.h>

#include "napcommon.h"
#include "SampleShv.h"
#include "DebugHelper.h"


#include "SdkCommon.h"
using namespace SDK_SAMPLE_COMMON;


namespace SDK_SAMPLE_SHV
{
// Initialize static class member.
LONG CSampleShv::threadCount = 0;

// Constructor.
CSampleShv::CSampleShv()
{
}


    //
    // Implementation of INapSystemHealthValidator::Validate().
    //
    STDMETHODIMP CSampleShv::Validate(
        /*[in]*/ INapSystemHealthValidationRequest* pShvRequest,
        /*[in]*/ UINT32 hintTimeOutInMsec,
        /*[in]*/ INapServerCallback* pCallback) throw()
    {
        HRESULT hr = S_OK;
        asyncDataItems* requestData = NULL;
        LONG t_count = 0;

        // SDK Note:
        // The method implementing IQuarSystemHealthValidator::Validate() should
        // rarely abort early on error. In most cases, it's still responsible for
        // generating an SoH Response (containing any errors in the Compliance
        // Result Codes SoH attribute) and returning success; in rare cases, it
        // can return a true error, in which case the client request will be
        // dropped completely.

        DebugPrintfW(L"\n\n----------------------------------------------------------------------------------\n");
        DebugPrintfW(L" --- SdkShv::SampleShv::Validate(IRequest*(0x%08x), Timeout(%d), ICallback*(0x%08x))",
                     pShvRequest, hintTimeOutInMsec, pCallback);

        // Sanity check: a valid pShvRequest pointer should always be passed in. 
        // If this pointer is NULL, the SHV will be unable to get any information about the 
        // client from the SHV Host.)
        //
        if (! pShvRequest)
        {
            DebugPrintfW(L" --- SdkShv::SampleShv::Validate(): The input Request object pointer is NULL!  This object is required for the SHV to be able to perform any processing.");
            hr = E_INVALIDARG;
            goto Cleanup;
        }

        // Sanity check: a valid pCallback pointer should always be passed in. 
        // If this pointer is NULL, the SHV will be unable to call OnComplete() function.
        //
        if (! pCallback)
        {
            DebugPrintfW(L" --- SdkShv::SampleShv::Validate(): The input Callback object pointer is NULL!  This object is required for the SHV to be able to perform any processing.");
            hr = E_INVALIDARG;
            goto Cleanup;
        }

        // SDK Note:
        // If a SoH validation code determines that it needs to contact an
        // external server to assist with validation, then it must start a
        // separate helper thread to contact the server, and return E_PENDING in
        // this thread. When that occurs, this thread must exit to the SHV Host,
        // returning the E_PENDING result; the helper thread should independently
        // determine the final validation result, generate the response SoH, and
        // call the SHV Host's Callback interface.
        //
        // SDK Note:
        // If a SHV does not require processing in a separate thread but can give back
        // the response immmediately, it should call the function QShvRespondSHVHost()
        // just below this comment.
        //
        //hr = QShvRespondSHVHost();
        //if (FAILED(hr))
        //{
        //    DebugPrintfW(L" ---   SdkShv:SampleShv:: Validate():  QShvRespondSHVHost returned error %#x!",hr);
        //    goto Cleanup;
        //}
        //else
        //{
        //    DebugPrintfW(L" ---   SdkShv:SampleShv:: Validate():  QShvRespondSHVHost returned success!");
        //    goto Cleanup;
        //}

        // SDK Note:
        // Creating a separate helper thread is done whenever SHV requires need to contact the 
        // external server. Therefore, the following code of creating a thread should be done where
        // and when the need arises.
        //
        // This SHV handles the Validate() call asynchronously.
        // Validate() method return with hr = E_PENDING and does the processing of 
        // SoH request and generating SoHResponse in a separate thread.
        //
        // Here we assume that the SHV will always contact an external server. Therefore, create 
        // the thread at very start and do all the processing in the helper thread.


        // NAP Server will quit sending Validate requests if it is shutting down, so the 
        // only thing we need to ensure is that we don't already have too many outstanding
        // threads.  Should do this before allocating extra resources

        // increment the thread counter
        t_count = InterlockedIncrement(&threadCount);

        // if this increment puts us over the max, then decrement and exit
        if ( t_count > SDK_SHV_MAX_THREADS )
        {
            DebugPrintfW(L" --- SdkShv::SampleShv::Validate(): Too many threads already exist - dropping request");
            InterlockedDecrement(&threadCount);
            hr = HRESULT_FROM_WIN32(ERROR_TOO_MANY_THREADS);
            goto Cleanup;
        }

        DebugPrintfW(L" --- SdkShv::SampleShv::Validate(): Threadcount OK, preparing to start thread [%d]", t_count);

        // Allocate space to store copies of the pointers; needed because local and member
        // variables won't work in the multi-threaded case
        //
        requestData = (asyncDataItems*)CoTaskMemAlloc(sizeof(asyncDataItems));
        if (!requestData)
        {
            // Failed to allocate memory
            DebugPrintfW(L" --- SdkShv::SampleShv::Validate(): Failed to allocate memory(error = 0x%08x)",hr);
            hr = E_OUTOFMEMORY;
            // Decrement the thread counter to finish releasing the resources that were used
            InterlockedDecrement(&threadCount);
            goto Cleanup;
        }
        ZeroMemory(requestData, sizeof(asyncDataItems));


        // AddRef the request and callback pointers prior to the attempt to create the thread
        pShvRequest->AddRef();
        pCallback->AddRef();

        // Store copies of the callback and request pointers to pass into the thread
        requestData->pthis = this;
        requestData->piShvRequest = pShvRequest;
        requestData->piCallback = pCallback;

        // Attempt to create a Thread to process the SoHRequest and reply with SoHResponse.
        hr = QShvCreateThread( requestData );
        if ( FAILED(hr) )
        {
            DebugPrintfW(L" ---  SdkShv::Validate(): QShvCreateThread() returned error %#x! ",hr);
            // Release the references to the request and callback pointers,
            // as well as our copies, since the thread failed to create
            pShvRequest->Release();
            pCallback->Release();
            CoTaskMemFree(requestData);
            // Decrement the thread counter to finish releasing the resources that were used
            InterlockedDecrement(&threadCount);
            goto Cleanup;
        }

        // Thread creation successful
        DebugPrintfW(L" ---  SdkShv::Validate(): QShvCreateThread() returned S_OK: thread successfully started. Returning E_PENDING to QSHVhost");
        hr = E_PENDING;

    Cleanup:
        return hr;
    }



    // Creates thread for asynchronous handling of Validate() call.
    HRESULT CSampleShv::QShvCreateThread( /*in*/ asyncDataItems* requestData ) throw()
    {
         HRESULT hr = S_OK;
         HANDLE threadHandle = NULL;
         DWORD threadId = 0;
        BOOL retCode = 0;

         if (!requestData)
         {
            hr = E_POINTER;
            goto Cleanup;
         }

     // Windows API to create a thread.
         threadHandle = (HANDLE) CreateThread(
                                    NULL,
                                    0, 
                                    AsyncThreadHandler,
                                    (LPVOID) requestData,
                                    0, 
                                    &threadId);
         if(threadHandle == NULL)
         {
            DebugPrintfW(L" ---  SdkShv:SampleShv::QShvCreateThread(): CreateThread() returned error");
            hr = HRESULT_FROM_WIN32(GetLastError());
            // cleanup of requestData and releasing of object references handled by caller
            goto Cleanup;
         }

        // Close the threadhandle now, so that when the thread returns,
        // it will be ended and unloaded
        retCode = CloseHandle(threadHandle);
        if(!retCode)
            DebugPrintfW(L" ---   SdkShv:SampleShv:: QShvCreateThread():  CloseHandle failed -LEAKED HANDLE (thread) !");
        
         // Thread created successfully
         hr = S_OK;
    
         Cleanup:
         return hr;
    }



    // This is the thread handler. The input parameter is the pointer to an asynDataItems struct.
    DWORD __stdcall CSampleShv::AsyncThreadHandler(LPVOID lpParameter) throw()
    {
        DWORD errCode = ERROR_SUCCESS;
        asyncDataItems * requestData = NULL;
        long t_count = 0;
        
        requestData = reinterpret_cast<asyncDataItems*>(lpParameter);
        if ( (!requestData) ||
                (!requestData->pthis) ||
                (!requestData->piShvRequest) ||
                (!requestData->piCallback) )
        {
            //can't use the passed values
            DebugPrintfW(L" ---    SdkShv:SampleShv::AsyncThreadHandler(): Invalid data passed in lpParameter");
            // can't Release the objects, but we can decrement the thread counter
            InterlockedDecrement(&threadCount);
            goto Cleanup;
        }

        errCode = requestData->pthis->AsyncThreadHandlerMain(requestData);
        if(errCode != ERROR_SUCCESS)
        {
            DebugPrintfW(L" ---    SdkShv:SampleShv::AsyncThreadHandler(): AsyncThreadHandlerMain() returned error %d!",errCode);
        }

        // Release the request and callback object references
        // regardless of the result, we are done.with them
        requestData->piShvRequest->Release();
        requestData->piCallback->Release();

        // also decrement the thread counter, since this thread is finishing
        t_count = InterlockedDecrement(&threadCount);
        DebugPrintfW(L" ---  SdkShv::SampleShv::AsyncThreadHandler() Thread finishing. After decrement, threadCount = [%d]", t_count);


    Cleanup:
        //also release the requestData struct space
        CoTaskMemFree(requestData);
        return errCode;
    }


    
    // The thread handler calls this method that has access to member variables.
    DWORD  CSampleShv::AsyncThreadHandlerMain(asyncDataItems *requestData) throw()
    {
        DWORD errCode = ERROR_SUCCESS;
        HRESULT hr = S_OK;

        hr = QShvRespondSHVHost(requestData);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---   SdkShv:SampleShv:: AsyncThreadHandlerMain():  QShvRespondSHVHost returned error %#x!",hr);
        }
        else
        {
            DebugPrintfW(L" ---   SdkShv:SampleShv:: AsyncThreadHandlerMain():  QShvRespondSHVHost returned success!");
        }

        errCode = HRESULT_CODE(hr);
        return errCode;        
    }


    
    // Handles processing of incoming SoH Request and generating SoH Response.
    HRESULT CSampleShv::QShvRespondSHVHost(asyncDataItems *requestData) throw()
    {
        HRESULT hr = S_OK;
        HRESULT validationResult = QUAR_E_NOTPATCHED;

        // requestData is checked before it is passed in by QShvCreateThread()
        
        hr = HandleRequestSoH(requestData->piShvRequest, validationResult);
        if (FAILED(hr))
        {
            DebugPrintfW(L" --- SdkShv::SampleShv::QShvRespondSHVHost(): HandleRequestSoH() returned error %#x!",hr);
            // an error has occurred reading and validatating the SoH data
            // calling OnComplete to notify NAP Server of our inability to evaluate health
            // NAP Server will then map this failure code into one of the FailureCategory types
            hr = requestData->piCallback->OnComplete(requestData->piShvRequest, hr);
            goto Cleanup;
        }

        //
        // Generate an outbound SoH response, to be sent to the client.
        // In this example, the hr is used as the value passed.
        //
        hr = HandleResponseSoH(validationResult, requestData->piShvRequest);
        if (FAILED(hr))
        {
            DebugPrintfW(L" --- SdkShv::SampleShv::QShvRespondSHVHost(): HandleResponseSoH() returned error %#x! Calling OnComplete() to notify QSHVHost.",hr);
            // Failed to set response. Calling OnComplete() to notify QSHVHost of inability to validate.
            hr = requestData->piCallback->OnComplete(requestData->piShvRequest, hr);
        }
        else
        {
            DebugPrintfW(L" --- SdkShv::SampleShv::QShvRespondSHVHost(): HandleResponseSoH() returned success. Calling OnComplete() to notify QSHVHost.");
            // SoH Response is now set. Calling OnComplete() notifies setting of SoHResponse.
            hr = requestData->piCallback->OnComplete(requestData->piShvRequest, hr);
        }


    Cleanup:
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---SdkShv::SampleShv::QShvRespondSHVHost(): OnComplete() returned error %#x!",hr);
        }

        return hr;    
    }


    // Perform all handling of inbound SoH requests from clients.
    // Possible return values:
    //          NAP_E_MISSING_SOH
    //          NAP_E_INVALID_PACKET
    //          QUAR_E_NOTPATCHED
    //          Other COM errors
    //      
    HRESULT CSampleShv::HandleRequestSoH(INapSystemHealthValidationRequest* pShvRequest, HRESULT &complianceResult) throw()
    {
        HRESULT hr = S_OK;

        SoH                  *pSohRequest    = NULL;
        INapSoHProcessor    *pSohProcessor  = NULL;
        SystemHealthEntityId  systemHealthId = 0;
        BOOL napAgentGenerated = FALSE;

        complianceResult = QUAR_E_NOTPATCHED;

        //
        // Get the client's SoH request data.
        //

        // Obtain the client's SoH data from the SHV Host.
        hr = pShvRequest->GetSoHRequest(&pSohRequest, &napAgentGenerated);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::HandleRequestSoH(): pShvRequest->GetSoHRequest() returned error %#x!", hr);
            goto Cleanup;
        }

        //
        // SDK Note:
        // If the napAgentGenerated value is TRUE, it indicates that the SoH was generated
        // by the NAPAgent on behalf of the SHA, and usually indicates an SHA failure.
        // In this case, the SHV should expect an SoH packet with the following 3 TLVs :
        //      napAttributeTypeSystemHealthId,
        //      napAttributeTypeFailureCategory,
        //      napAttributeTypeErrorCodes
        //

        //
        // In this sample we assume that napAgentGenerated is always FALSE to keep the code simple.
        //
        if (! pSohRequest)
        {
            DebugPrintfW(L" ---         SampleShv::HandleRequestSoH(): The client did not send an SoH for this SHV.");
            hr = NAP_E_MISSING_SOH;
            goto Cleanup;
        }

        DebugPrintfW(L" ---         SampleShv::HandleRequestSoH(): Received SoH request for this SHV.");

        // Wrap it inside an INapSoHProcessor object.
        hr = CreateInputSoHProcessor(pSohProcessor, systemHealthId,
                                     SOH_REQUEST,   pSohRequest);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::HandleRequestSoH(): Couldn't create an SoH Parser object! (error %#x)", hr);
            goto Cleanup;
        }

        //
        // We support multiple config.  Get the Config ID.
        //
        UINT32 uConfigID = 0 ;
        GetConfigIDFromQuery(pShvRequest, &uConfigID);

        //
        // Validate whether the client's SoH request data is considered "healthy".
        //
        hr = CheckRequestSoHHealth(systemHealthId, uConfigID, pSohProcessor, complianceResult);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::HandleRequestSoH(): ValidateRequestSoH() returned error %#x!", hr);
            goto Cleanup;
        }

     Cleanup:
        // Release the SoH handler object.
        ReleaseObject(pSohProcessor);
        // Free the temporary SoH struct.
        FreeSoH(pSohRequest);

        return hr;
    }

    // Get the configuration ID.
    HRESULT CSampleShv::GetConfigIDFromQuery(
                INapSystemHealthValidationRequest* request,
                UINT32* configID
                )
    {
        HRESULT hr = E_FAIL;
        UINT32 id;
        INapSystemHealthValidationRequest2 *r2 = NULL;

        if (!request || !configID)
        {
            hr = E_INVALIDARG;
            goto Cleanup;
        }

        *configID = 0; //defaults to 0.

        hr = request->QueryInterface(
                                __uuidof(INapSystemHealthValidationRequest2),
                                (LPVOID*)&r2
                                );
        if (FAILED(hr))
        {
            if (E_NOINTERFACE == hr)
            {
                // Interface not supported, it might be Longhorn, so there's only
                // config 0
                hr = S_OK;
            }
            goto Cleanup;
        }

        hr = r2->GetConfigID(&id);
        if (SUCCEEDED(hr))
        {
            *configID = id;
        }
        
    Cleanup:
        if (r2)
        {
            r2->Release();
        }
        return hr;
    }

    // Validate whether the client's SoH data is considered healthy.
    HRESULT CSampleShv::CheckRequestSoHHealth(SystemHealthEntityId  systemHealthId,
                                              UINT32 uConfigID,
                                              INapSoHProcessor    *pSohRequest,
                                              HRESULT &complianceResult ) throw()
    {
        HRESULT hr = S_OK;

        //
        // SDK Note:
        // Each vendor should update this method & its helper functions,
        // based upon their business logic.
        //

        UINT16 index = 0;

        SoHAttributeType   attrType   = (SoHAttributeType) 0;
        SoHAttributeValue *pAttrValue = NULL;


        // Secure by default -- assume the client is unhealthy, until it
        // proves otherwise.
        complianceResult = QUAR_E_NOTPATCHED;
        
        // Sanity check -- if no SoH data was passed in, leave client data unset.
        if (! pSohRequest)
        {
            //
            // SDK Note:
            //  - If this SHV is acting as an intrusion detection system, it
            //    should use its specific business logic to determine whether
            //    the client is trusted or malicious, and should create an
            //    SoH Response containing the appropriate error code to
            //    convey the result to the SHV Host.
            //  - All other SHVs should return NAP_E_MISSING_SOH.

            DebugPrintfW(L" ---         SampleShv::CheckRequestSoHHealth(): No SoH to evaluate.");
            hr = NAP_E_MISSING_SOH;
            goto Cleanup;
        }


        // Check for the existence & values for each attribute expected from
        // the client.

        //
        // SDK Note:
        //      This SHV is very simple, and only handles 1 single attribute value within
        //      the SoH passed.  Most SHVs will need to handle multiple attributes, and will
        //      need to correctly handle them in whatever order they appear within the SoH.
        //

        // In this sample, we expect only 1 attribute passed back from SHA, so we are searching
        // for first instance of sohAttributeTypeVendorSpecific
        hr = pSohRequest->FindNextAttribute(0, sohAttributeTypeVendorSpecific, &index);
        if (FAILED(hr))
        {
           DebugPrintfW(L" ---         SampleShv::Validate(): Error: hit error %#x while searching for the client's Vendor-Specific Data attribute!", hr);
           goto Cleanup;
        }

        // Get the attribute located at the returned index
        hr = pSohRequest->GetAttribute(index, &attrType, &pAttrValue);
        if (FAILED(hr))
        {
           DebugPrintfW(L" ---         SampleShv::Validate(): Hit error %#x while reading attribute #%d!", hr, index);
           goto Cleanup;
        }

        // Sanity check -- ignore attributes whose value pointer is NULL.
        if (!pAttrValue)
        {
           DebugPrintfW(L" ---         SampleShv::Validate(): Hit error %#x while reading attribute #%d!", hr, index);
            hr = NAP_E_INVALID_PACKET;
           goto Cleanup;
        }

        // Check the configuration to see if it is enabled.
        if (!IsConfigurationTurnedOn(uConfigID))
        {
            DebugPrintfW(L" ---         SampleShv::Validate(): The configuration is off, evaluating to Compliant");
            complianceResult = QUAR_E_COMPLIANT;
        }
        // Verify the attribute's value.
        else if ( (pAttrValue->vendorSpecificVal.size   == SDK_CLIENT_VENDOR_DATA_SIZE) &&
            (pAttrValue->vendorSpecificVal.vendorId == QuarSampleSystemHealthId) &&
            (memcmp(pAttrValue->vendorSpecificVal.vendorSpecificData,
                    SDK_CLIENT_VENDOR_DATA_HEALTHY,
                    pAttrValue->vendorSpecificVal.size) == 0) )
        {
            // This client's data is the 'healthy' value.
            DebugPrintfW(L" ---         SampleShv::Validate(): This client's SoH data has evaluated to Compliant");
            complianceResult = QUAR_E_COMPLIANT;
        }
        else if ( (pAttrValue->vendorSpecificVal.size   == SDK_CLIENT_VENDOR_DATA_SIZE) &&
            (pAttrValue->vendorSpecificVal.vendorId == QuarSampleSystemHealthId) &&
            (memcmp(pAttrValue->vendorSpecificVal.vendorSpecificData,
                    SDK_CLIENT_VENDOR_DATA_UNHEALTHY,
                    pAttrValue->vendorSpecificVal.size) == 0) )
        {
            // This client's data is the 'unhealthy' value.
            DebugPrintfW(L" ---         SampleShv::Validate(): This client's SoH data has evaluated to NOT Compliant");
            complianceResult = QUAR_E_NOTPATCHED;
        }
        else
        {
            // data did not match size, ID and/or healthy/unhealthy values
            hr = NAP_E_INVALID_PACKET;
            DebugPrintfW(L" ---         SampleShv::Validate(): This client's SoH data was not a recognized value");
        }

        // Free the returned attribute object before leaving.
        FreeSoHAttributeValue(attrType, pAttrValue);


     Cleanup:
        if ( SUCCEEDED(hr) )
        {
            DebugPrintfW(L" ---         SampleShv::Validate(): This client's SoH data successfully evaluated.");
        }
        else 
        {
            DebugPrintfW(L" ---         SampleShv::Validate(): This client's SoH data not successfully validated. (code = 0x%08x", hr);
        }

        return hr;
    }

    BOOL CSampleShv::IsConfigurationTurnedOn(UINT32 uConfigID)
    {
        BOOL bIsConfigurationTurnedOn = TRUE;

        INapComponentConfig3* pServer = NULL;
        HRESULT hr = CoCreateInstance(
                        CLSID_SDK_SHV_UI,
                        NULL,               // pUnkOuter
                        CLSCTX_LOCAL_SERVER,
                        IID_INapComponentConfig3,
                        (VOID **) &pServer);
        
        if (SUCCEEDED(hr))
        {
            UINT16 bCount = 0;
            BYTE* pOutdata = NULL;
            hr = pServer->GetConfigFromID(uConfigID, &bCount, &pOutdata);
            if (SUCCEEDED(hr))
            {
                if (*pOutdata == 0)
                {
                    bIsConfigurationTurnedOn = FALSE;
                }

                CoTaskMemFree(pOutdata);
            }
            pServer->Release();
        }

        return bIsConfigurationTurnedOn;
    }

    // Perform all handling of outbound SoH responses sent back to clients.
    HRESULT CSampleShv::HandleResponseSoH(HRESULT          validationResult,
                                          INapSystemHealthValidationRequest* pShvRequest) throw()
    {
        HRESULT hr = S_OK;

        INapSoHConstructor *pSohConstructor = NULL;
        SoH                 *pSohResponse    = NULL;

        //
        // Create an empty SoH response.
        //

        hr = CreateOutputSoHConstructor(pSohConstructor,
                                        QuarSampleSystemHealthId, SOH_RESPONSE);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::HandleResponseSoH(): CreateResponseSoH() returned error %#x!",
                         hr);
            goto Cleanup;
        }


        //
        // Populate the SoH response based on the client's health state.
        //

        hr = FillResponseSoH(validationResult, pSohConstructor);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::HandleResponseSoH(): FillResponseSoH() returned error %#x!",
                         hr);
            goto Cleanup;
        }


        //
        // Save the SoH response into the SHV Host.
        //

        // Get portable SoH data.
        hr = pSohConstructor->GetSoH(&pSohResponse);

        if (! pSohResponse)
            hr = E_POINTER;

        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::HandleResponseSoH(): Couldn't get the final Response SoH data! (error %#x, pointer = 0x%08x)",
                         hr, pSohResponse);
            goto Cleanup;
        }

        // Pass it along to the SHV Host.
        hr = pShvRequest->SetSoHResponse(pSohResponse);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::HandleResponseSoH(): SetSoHResponse() returned error %#x!",
                         hr);
            goto Cleanup;
        }


     Cleanup:
        // Release the SoH handler object.
        ReleaseObject(pSohConstructor);

        // Free the temporary SoH struct.
        FreeSoH(pSohResponse);

        return hr;
    }


    // Fill the specified response SoH object, given the current client's health
    // state.
    HRESULT CSampleShv::FillResponseSoH(HRESULT               validationResult,
                                        INapSoHConstructor* &pSohResponse) throw()
    {
        HRESULT hr = S_OK;

        SoHAttributeValue value = {0};


        //
        // Append the manditory attribute: Compliance Result Codes.
        // If SHV was not able to contact the external server (Network Failure, Server Down...)
        // SHV should return Failure Category attribute instead of Compliance Result.
        // (with FailureCategory = failureCategoryServerCommunication)
        // 
        // Here we assume there is no such failure.

        value.codesVal.count    = 1;
        value.codesVal.results = &validationResult;

        hr = pSohResponse->AppendAttribute(sohAttributeTypeComplianceResultCodes,
                                           &value);
        if (FAILED(hr))
        {
            DebugPrintfW(L" ---         SampleShv::FillResponseSoH(): SoHResponse->AppendAttribute() returned error %#x!", hr);
            goto Cleanup;
        }

        ZeroMemory(&value, sizeof(value));


        //
        // Append any optional attributes.
        //

        //
        // SDK Note:
        // Append any other attributes that are appropriate for the given
        // business logic. (I.e., Vendor-Specific-Data, IPv4-Fixup-Servers,
        // Health-Class, Time-of-Last-Update, Software-Version, etc.)

    Cleanup:
        return hr;
    }


    BOOL CSampleShv::ThreadCountIsZeroed() throw()
    {
        // if threadCount > 0, return false
        // if threadCount <= 0, returns true
        // side-effect - if threadCount == 0, then exchange it with 0
        return ( 0 <= InterlockedCompareExchange( &threadCount, 0, 0) );

    }



}  // End "namespace SDK_SAMPLE_SHV".

