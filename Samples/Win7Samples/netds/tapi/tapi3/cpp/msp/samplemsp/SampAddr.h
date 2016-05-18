/*++

Copyright (c) 1999 - 2000 Microsoft Corporation

Module Name:

    sampaddr.h

Abstract:

    Declaration of the CSampleMSP
--*/

#ifndef __SAMPADDR_H_
#define __SAMPADDR_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSampleMSP
/////////////////////////////////////////////////////////////////////////////
class CSampleMSP : 
    public CMSPAddress,
    public CComCoClass<CSampleMSP, &CLSID_SampleMSP>
    //  if you want to allow your object to be used in scripting apps
    //  it has to expose IObjectSafety interface. if your object is safe for 
    //  scripting on all of its interfaces, you can use the implementation
    //  of IObjectSafety from MSPUtils.h
    //, public CMSPObjectSafetyImpl
{
public:
    CSampleMSP();
    virtual ~CSampleMSP();

    virtual ULONG MSPAddressAddRef(void);
    virtual ULONG MSPAddressRelease(void);

DECLARE_REGISTRY_RESOURCEID(IDR_SampleMSP)
DECLARE_POLY_AGGREGATABLE(CSampleMSP)

// To add extra interfaces to this class, use the following:
// BEGIN_COM_MAP(CSampleMSP)
//     COM_INTERFACE_ENTRY( YOUR_INTERFACE_HERE )
//     // if you want to allow your object to be used in scripting apps
//     // it has to expose IObjectSafety interface
//     // COM_INTERFACE_ENTRY( IObjectSafety )
//     COM_INTERFACE_ENTRY_CHAIN(CMSPAddress)
// END_COM_MAP()

public:
    STDMETHOD (CreateMSPCall) (
        IN      MSP_HANDLE     htCall,
        IN      DWORD          dwReserved,
        IN      DWORD          dwMediaType,
        IN      IUnknown    *  pOuterUnknown,
        OUT     IUnknown   **  ppMSPCall
        );

    STDMETHOD (ShutdownMSPCall) (
        IN      IUnknown *          pMSPCall
        );

protected:

    DWORD GetCallMediaTypes(void);
};

#endif //__SAMPADDR_H_
