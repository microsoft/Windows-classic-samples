/*

Copyright (c) 1997 - 2000 Microsoft Corporation, all rights reserved

Description:

History:

*/

#ifndef _CEAPCFG_H_
#define _CEAPCFG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <rrascfg.h>    // IEAPProviderConfig declarations

#include "resource.h"

// Define this here so that our macros will expand correctly
#define IMPL

// External references
extern const CLSID  CLSID_EapCfg;

class ATL_NO_VTABLE CEapCfg : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CEapCfg, &CLSID_EapCfg>,
    public IEAPProviderConfig
{
public:
    CEapCfg()  {}

DECLARE_REGISTRY_RESOURCEID(IDR_EAPCFG)
DECLARE_NOT_AGGREGATABLE(CEapCfg)

BEGIN_COM_MAP(CEapCfg)
    COM_INTERFACE_ENTRY(IEAPProviderConfig)
END_COM_MAP()

public:
    // This is the declaration of the UI interface members.
    // See rrascfg.idl for the prototypes
    DeclareIEAPProviderConfigMembers(IMPL);
};

#endif // _CEAPCFG_H_
