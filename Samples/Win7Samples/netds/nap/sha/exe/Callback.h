// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __Callback_H__
#define __Callback_H__

#pragma once

#include <Atlbase.h>
#include <Atlcom.h>

#include <assert.h>

#include <NapSystemHealthAgent.h>
#include <NapProtocol.h>
#include <NapManagement.h>
#include "Messages.h"

#include "SDKCommon.h"
#include "sdkshamodule.h"

//using namespace SDK_SAMPLE_COMMON;


// This is a pseudo-COM object, i.e. not co-create-able and not in registry.
class __declspec(novtable) ShaCallback
   : public CComObjectRootEx<CComMultiThreadModelNoCS>,
     public INapSystemHealthAgentCallback

{
public:

    // Create instance
	static SDK_SAMPLE_SHA::IShaCallbackPtr CreateInstance(
                            INapSystemHealthAgentBinding* binding
                            ) throw ();

    STDMETHOD(GetSoHRequest)(
       IN INapSystemHealthAgentRequest* pRequest) throw ();


    STDMETHOD(ProcessSoHResponse)(
       IN INapSystemHealthAgentRequest* pIRequest) throw ();


    STDMETHOD(NotifySystemIsolationStateChange)(void) throw ();


    STDMETHOD(GetFixupInfo)(
       OUT FixupInfo** ppstatus) throw ();

    STDMETHOD(CompareSoHRequests)(
       /* in */ const SoHRequest* lhs,
       /* in */ const SoHRequest* rhs,
       /* out */ BOOL* isEqual
       ) throw ();

    STDMETHOD(NotifyOrphanedSoHRequest)(
       /* in */ const CorrelationId* correlationId
       ) throw ();

protected:

    // Constructor
    ShaCallback() throw ();

    // Destructor
    ~ShaCallback() throw ();


private:


    INapSystemHealthAgentBinding* m_pBinding;

    HRESULT CreateSoHConstructor(INapSoHConstructor** ppISohRequest) throw();


    HRESULT FillSoHRequest(INapSoHConstructor** ppISohRequest) throw();


    HRESULT HandleSoHResponse(INapSoHProcessor * pSohProcessor,
                                    SystemHealthEntityId  systemHealthId,
                                    BOOL doFixup) throw();



    // Assignment operator not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation of
    // the assignment operator, which isn't guaranteed to handle member
    // variables correctly.

    ShaCallback& operator=
        (const ShaCallback&rhs) throw();


    // Copy constructor not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation
    // of the copy constructor, which isn't guaranteed to handle member
    // variables correctly.

    ShaCallback(
        const ShaCallback &rhs) throw();



    DECLARE_NO_REGISTRY()

    DECLARE_NOT_AGGREGATABLE(ShaCallback)

    BEGIN_COM_MAP(ShaCallback)
      COM_INTERFACE_ENTRY(INapSystemHealthAgentCallback)
   END_COM_MAP()
};

#endif

