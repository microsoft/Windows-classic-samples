//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File:        module.h
//
// Contents:    CCertManagePolicyModuleSample definition
//
//---------------------------------------------------------------------------

#include "certpsam.h"
#include "resource.h"       // main symbols


class CCertManagePolicyModuleSample: 
    public CComDualImpl<ICertManageModule, &IID_ICertManageModule, &LIBID_CERTPOLICYSAMPLELib>, 
    public CComObjectRoot,
    public CComCoClass<CCertManagePolicyModuleSample, &CLSID_CCertManagePolicyModuleSample>
{
public:
    CCertManagePolicyModuleSample() {}
    ~CCertManagePolicyModuleSample() {}

BEGIN_COM_MAP(CCertManagePolicyModuleSample)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertManageModule)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertManagePolicyModuleSample) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation.  The default is to support it

// UNDONE UNDONE
DECLARE_REGISTRY(
    CCertManagePolicyModuleSample,
    wszCLASS_CERTMANAGEPOLICYMODULESAMPLE TEXT(".1"),
    wszCLASS_CERTMANAGEPOLICYMODULESAMPLE,
    IDS_CERTMANAGEPOLICYMODULE_DESC,    
    THREADFLAGS_BOTH)

// ICertManageModule
public:
    STDMETHOD (GetProperty) (
            /* [in] */ const BSTR strConfig,
            /* [in] */ BSTR strStorageLocation,
            /* [in] */ BSTR strPropertyName,
            /* [in] */ LONG dwFlags,
            /* [retval][out] */ VARIANT __RPC_FAR *pvarProperty);
        
    STDMETHOD (SetProperty)(
            /* [in] */ const BSTR strConfig,
            /* [in] */ BSTR strStorageLocation,
            /* [in] */ BSTR strPropertyName,
            /* [in] */ LONG dwFlags,
            /* [in] */ VARIANT const __RPC_FAR *pvarProperty);
        
    STDMETHOD (Configure)( 
            /* [in] */ const BSTR strConfig,
            /* [in] */ BSTR strStorageLocation,
            /* [in] */ LONG dwFlags);
};
