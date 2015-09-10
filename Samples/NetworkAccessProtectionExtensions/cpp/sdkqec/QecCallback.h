// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __QecCallback_H__
#define __QecCallback_H__

#pragma once

#include <NapEnforcementClient.h>
#include <NapProtocol.h>
#include <NapManagement.h>

#include "SDKCommon.h"
using namespace SDK_SAMPLE_COMMON;

namespace SDK_SAMPLE_QEC
{
    static const UINT16 NUMBER_OF_HRESULTS = 1;
}


using namespace SDK_SAMPLE_QEC;

// This is a pseudo-COM object, i.e. not co-create-able and not in registry.
class QecCallback
   : public INapEnforcementClientCallback

{
public:

    // IUnknown

    STDMETHODIMP QueryInterface(
        /* [in] */ __RPC__in const IID& iid, 
        /* [out] */ __RPC__out void** ppv);

    STDMETHODIMP_(ULONG) AddRef();
    
    STDMETHODIMP_(ULONG) Release();

    // INapEnforcementClientCallback

    STDMETHOD(NotifySoHChange)(void);

    STDMETHOD(GetConnections)(
        /* [out] */ __RPC__deref_out_opt Connections** connections);

public:

    // Create instance
    static INapEnforcementClientCallback* CreateInstance(
        _In_ INapEnforcementClientBinding* binding);

protected:

    // Constructor
    QecCallback();

    // Destructor
    ~QecCallback();


private:

    long m_cRef;

    INapEnforcementClientBinding* m_pBinding;
    INapEnforcementClientConnection* m_pConnection;
    BOOL m_retriggerHint;

    HRESULT CreateConnectionObject ();

    // Assignment operator not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation of
    // the assignment operator, which isn't guaranteed to handle member
    // variables correctly.
    QecCallback& operator=(const QecCallback&rhs);

    // Copy constructor not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation
    // of the copy constructor, which isn't guaranteed to handle member
    // variables correctly.
    QecCallback(const QecCallback &rhs);
};

#endif

