// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __Callback_H__
#define __Callback_H__

#pragma once

#include <NapSystemHealthAgent.h>
#include <NapProtocol.h>
#include <NapManagement.h>
#include "Messages.h"

#include "SDKCommon.h"
#include "sdkshamodule.h"


// This is a pseudo-COM object, i.e. not co-create-able and not in registry.
class ShaCallback :
     public INapSystemHealthAgentCallback
{
public:

    // IUnknown

    STDMETHODIMP QueryInterface(
        __RPC__in REFIID riid , 
        __RPC__out void **ppObj);

    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();

    // INapSystemHealthAgentCallback

    STDMETHOD(GetSoHRequest)(
        /* out */ __RPC__in_opt INapSystemHealthAgentRequest* pRequest);

    STDMETHOD(ProcessSoHResponse)(
        /* out */ __RPC__in_opt INapSystemHealthAgentRequest* pIRequest);

    STDMETHOD(NotifySystemIsolationStateChange)(void);

    STDMETHOD(GetFixupInfo)(
        /* out */ __RPC__deref_out_opt FixupInfo** ppstatus);

    STDMETHOD(CompareSoHRequests)(
       /* in */ __RPC__in const SoHRequest* lhs,
       /* in */ __RPC__in const SoHRequest* rhs,
       /* out */ __RPC__out BOOL* isEqual);

    STDMETHOD(NotifyOrphanedSoHRequest)(
       /* in */ __RPC__in const CorrelationId* correlationId);

public:

    // Create instance
	static INapSystemHealthAgentCallback* CreateInstance(
        /* in */ _In_ INapSystemHealthAgentBinding* binding);

protected:

    // Constructor
    ShaCallback();

    // Destructor
    ~ShaCallback();


private:

    long m_cRef;

    INapSystemHealthAgentBinding* m_pBinding;

    HRESULT FillSoHRequest(
        _In_ INapSoHConstructor* pISohRequest);

    HRESULT HandleSoHResponse(
        _In_ INapSoHProcessor* pSohProcessor,
        _In_ BOOL doFixup);

    // Assignment operator not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation of
    // the assignment operator, which isn't guaranteed to handle member
    // variables correctly.
    ShaCallback& operator= (const ShaCallback&rhs);

    // Copy constructor not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation
    // of the copy constructor, which isn't guaranteed to handle member
    // variables correctly.
    ShaCallback(const ShaCallback &rhs);

};

#endif

