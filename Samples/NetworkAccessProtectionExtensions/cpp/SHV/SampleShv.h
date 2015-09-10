// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __SampleShv_H__
#define __SampleShv_H__

#pragma once

#include <windows.h>
#include <NapProtocol.h>
#include <NapSystemHealthValidator.h>
#include "resource.h"


namespace SDK_SAMPLE_SHV
{

    //
    // Main COM object class: Implement a NAP System Health Validator.
    //
    class CSampleShv;
    EXTERN_C const CLSID CLSID_SampleShv;
    
    typedef struct _asyncDataItems
    {
        //CSampleShv pointer so that threads can call member functions
        CSampleShv* pthis;
        // Pointer that stores the Callback object passed through the Validate() call. 
        INapServerCallback* piCallback;
        // Pointer that stores the SHVRequest object passed through the Validate() call. 
        INapSystemHealthValidationRequest* piShvRequest;
    } asyncDataItems;


    class __declspec(uuid("007a95d0-5a9b-4647-8adb-c8b91e64e925"))
    CSampleShv :
        public INapSystemHealthValidator
    {
    public:

        // IUnknown

        STDMETHODIMP QueryInterface(
            /* [in] */ __RPC__in const IID& iid, 
            /* [out] */ __RPC__out void** ppv);

        STDMETHODIMP_(ULONG) AddRef();
    
        STDMETHODIMP_(ULONG) Release();

        // INapSystemHealthValidator

        STDMETHOD(Validate)(
            /*[in]*/ __RPC__in_opt INapSystemHealthValidationRequest* pShvRequest,
            /*[in]*/ UINT32 hintTimeOutInMsec,
            /*[in]*/ __RPC__in_opt INapServerCallback* pCallback);

        static BOOL ThreadCountIsZeroed();

        // Constructor.
        CSampleShv();

        // Destructor
        ~CSampleShv();

    private:

        long m_cRef;

        // Handles processing of incoming SoH Request and generating SoH Response.
        HRESULT QShvRespondSHVHost(
            _In_ asyncDataItems *requestData);

        //
        // Handle inbound client SoH.
        //

        // Perform all handling of inbound SoH requests from clients.
        HRESULT HandleRequestSoH(
            _In_ INapSystemHealthValidationRequest* pShvRequest, 
            _Out_ HRESULT &complianceResult);

        //
        // We support multiple config. Used to get the Config ID. Defaults to 0 if it fails.
        //
        HRESULT GetConfigIDFromQuery(
            _In_ INapSystemHealthValidationRequest* request,
            _Out_ UINT32* configID);

        // Validate whether the client's SoH data is considered healthy.
        HRESULT CheckRequestSoHHealth(
            _In_ UINT32 uConfigID,
            _In_ INapSoHProcessor *pSohRequest,
            _Out_ HRESULT &complianceResult);

        //
        // Tell if the configuration is on or off. 
        // We have only one checkbox on our configuration UI.
        // If checked, it is on; otherwise off.
        //
        BOOL IsConfigurationTurnedOn(
            _In_ UINT32 uConfigID);

        //
        // Handle outbound response SoH.
        //

        // Perform all handling of outbound SoH responses sent back to clients.
        HRESULT HandleResponseSoH(
            _In_ HRESULT validationResult,
            _In_ INapSystemHealthValidationRequest* pShvRequest);

        // Fill the specified response SoH object, given the current client's
        // health state.
        HRESULT FillResponseSoH(
            _In_ HRESULT validationResult,
            _In_ INapSoHConstructor* pSohResponse);

        //
        // Functions for creating a separate thread that would be handling the processing of SoH Request.
        //

        // Creates thread for asynchronous handling of Validate() call.
        HRESULT QShvCreateThread(
            _In_ asyncDataItems * requestData);

        // This is the thread handler. The input parameter is the pointer to the current object (SampleShv).
        static DWORD __stdcall AsyncThreadHandler(
            _In_ LPVOID pShvRequest);

        // The thread handler calls this method that has access to member variables.
        DWORD AsyncThreadHandlerMain(
            _In_ asyncDataItems *requestData);

        // Thread counter - this object should not be unloaded until all threads have finished
        static LONG threadCount;

        //
        // General class methods.
        //

        // Assignment operator not implemented. This explicitly prevents the
        // compiler from automatically providing a default implementation of
        // the assignment operator, which isn't guaranteed to handle member
        // variables correctly.
        CSampleShv& operator=(const CSampleShv &rhs);

        // Copy constructor not implemented. This explicitly prevents the
        // compiler from automatically providing a default implementation
        // of the copy constructor, which isn't guaranteed to handle member
        // variables correctly.
        CSampleShv(const CSampleShv &rhs);

    };

}  // End "namespace SDK_SAMPLE_SHV".

#endif  // __SampleShv_H__
