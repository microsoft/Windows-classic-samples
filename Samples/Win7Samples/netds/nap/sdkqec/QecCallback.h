// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __QecCallback_H__
#define __QecCallback_H__

#pragma once

#include <Atlbase.h>
#include <Atlcom.h>

#include <assert.h>

#include <NapEnforcementClient.h>
#include <NapProtocol.h>
#include <NapManagement.h>

#include "SDKCommon.h"
using namespace SDK_SAMPLE_COMMON;

namespace SDK_SAMPLE_QEC
{
    static const UINT16 NUMBER_OF_HRESULTS = 1;

    typedef CComPtr<INapEnforcementClientCallback> IQecCallbackPtr;
}


using namespace SDK_SAMPLE_QEC;

// This is a pseudo-COM object, i.e. not co-create-able and not in registry.
class __declspec(novtable) QecCallback
   : public CComObjectRootEx<CComMultiThreadModelNoCS>,
     public INapEnforcementClientCallback

{
public:

    // Create instance
    static IQecCallbackPtr CreateInstance(
                            INapEnforcementClientBinding* binding
                            ) throw();

    STDMETHOD(NotifySoHChange)(void) throw();


    STDMETHOD(GetConnections)(/* out */ Connections** connections) throw();

protected:

    // Constructor
    QecCallback() throw ();

    // Destructor
    ~QecCallback() throw ();


private:


    INapEnforcementClientBinding* m_pBinding;
    INapEnforcementClientConnection* m_pConnection;
    BOOL m_retriggerHint;

    HRESULT CreateConnectionObject () throw();


    // Assignment operator not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation of
    // the assignment operator, which isn't guaranteed to handle member
    // variables correctly.

    QecCallback& operator=
        (const QecCallback&rhs) throw();


    // Copy constructor not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation
    // of the copy constructor, which isn't guaranteed to handle member
    // variables correctly.

    QecCallback(
        const QecCallback &rhs) throw();

    DECLARE_NO_REGISTRY()

    DECLARE_NOT_AGGREGATABLE(QecCallback)

    BEGIN_COM_MAP(QecCallback)
      COM_INTERFACE_ENTRY(INapEnforcementClientCallback)
   END_COM_MAP()
};

#endif

