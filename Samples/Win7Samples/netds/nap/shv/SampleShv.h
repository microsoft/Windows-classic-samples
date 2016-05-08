// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


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
    
    typedef struct _asyncDataItems
    {
        //CSampleShv pointer so that threads can call member functions
        CSampleShv* pthis;
        // Pointer that stores the Callback object passed through the Validate() call. 
       INapServerCallback* piCallback;
       // Pointer that stores the SHVRequest object passed through the Validate() call. 
       INapSystemHealthValidationRequest* piShvRequest;
    } asyncDataItems;


    class __declspec(novtable)
          __declspec(uuid("007a95d0-5a9b-4647-8adb-c8b91e64e925"))
        CSampleShv :
            public CComObjectRootEx<CComMultiThreadModelNoCS>,
            public CComCoClass<CSampleShv, &__uuidof(CSampleShv)>,
            public INapSystemHealthValidator
    {

      public:

        DECLARE_NO_REGISTRY()

        DECLARE_NOT_AGGREGATABLE(CSampleShv)

        BEGIN_COM_MAP(CSampleShv)
            COM_INTERFACE_ENTRY(INapSystemHealthValidator)
        END_COM_MAP()



        //
        // Implementation of INapSystemHealthValidator::Validate().
        //

        STDMETHOD(Validate(
                      /*[in]*/ INapSystemHealthValidationRequest* request,
                      /*[in]*/ UINT32 hintTimeOutInMsec,
                      /*[in]*/ INapServerCallback* callback)) throw();


        static BOOL ThreadCountIsZeroed() throw();

      protected:

        // Constructor.
        CSampleShv()  throw();

        // Destructor (empty).
        ~CSampleShv() throw()  { };



      private:

     // Handles processing of incoming SoH Request and generating SoH Response.
         HRESULT QShvRespondSHVHost(asyncDataItems *requestData) throw();


        //
        // Handle inbound client SoH.
        //

        // Perform all handling of inbound SoH requests from clients.
        HRESULT HandleRequestSoH(INapSystemHealthValidationRequest* pShvRequest, HRESULT &complianceResult) throw();

        //
        // We support multiple config. Used to get the Config ID. Defaults to 0 if it fails.
        //
        HRESULT GetConfigIDFromQuery(
                                INapSystemHealthValidationRequest* request,
                                UINT32* configID
                                );

        // Validate whether the client's SoH data is considered healthy.
        HRESULT CheckRequestSoHHealth(SystemHealthEntityId  systemHealthId,
                                      UINT32 uConfigID,
                                      INapSoHProcessor    *pSohRequest,
                                      HRESULT &complianceResult) throw();


        //
        // Tell if the configuration is on or off. 
        // We have only one checkbox on our configuration UI.
        // If checked, it is on; otherwise off.
        //
        BOOL IsConfigurationTurnedOn(UINT32 uConfigID);

        //
        // Handle outbound response SoH.
        //

        // Perform all handling of outbound SoH responses sent back to clients.
        HRESULT HandleResponseSoH(HRESULT          validationResult,
                                  INapSystemHealthValidationRequest* pShvRequest) throw();


        // Fill the specified response SoH object, given the current client's
        // health state.
        HRESULT FillResponseSoH(HRESULT               validationResult,
                                INapSoHConstructor* &pSohResponse) throw();


     //
     // Functions for creating a separate thread that would be handling the processing of SoH Request.
     //

     // Creates thread for asynchronous handling of Validate() call.
        HRESULT QShvCreateThread( asyncDataItems * requestData );


         // This is the thread handler. The input parameter is the pointer to the current object (SampleShv).
         static DWORD __stdcall AsyncThreadHandler (LPVOID pShvRequest);


         // The thread handler calls this method that has access to member variables.
         DWORD AsyncThreadHandlerMain(asyncDataItems *requestData);



        // Thread counter - this object should not be unloaded until all threads have finished
        static LONG threadCount;

        //
        // General class methods.
        //

        // Assignment operator not implemented. This explicitly prevents the
        // compiler from automatically providing a default implementation of
        // the assignment operator, which isn't guaranteed to handle member
        // variables correctly.
        CSampleShv& operator=(const CSampleShv &rhs) throw();

        // Copy constructor not implemented. This explicitly prevents the
        // compiler from automatically providing a default implementation
        // of the copy constructor, which isn't guaranteed to handle member
        // variables correctly.
        CSampleShv(const CSampleShv &rhs) throw();

    };

    OBJECT_ENTRY_AUTO(__uuidof(CSampleShv), CSampleShv)


}  // End "namespace SDK_SAMPLE_SHV".

#endif  // __SampleShv_H__
