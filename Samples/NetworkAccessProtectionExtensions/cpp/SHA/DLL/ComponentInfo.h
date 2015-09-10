// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// ComponentInfo.h : Declaration of the CComponentInfo

#pragma once
#include "resource.h"       // main symbols
#include "napcommon.h"

EXTERN_C const CLSID CLSID_ComponentInfo;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

static const WORD LANG_ID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

// CComponentInfo

class  __declspec(uuid("E19DDEC2-3FBE-4C3B-9317-679760C13AAE")) 
CComponentInfo :
	public INapComponentInfo
{
public:
	CComponentInfo();

	~CComponentInfo();

public:

    // IUnknown

    STDMETHODIMP QueryInterface(
        /* [in] */ __RPC__in const IID& iid, 
        /* [out] */ __RPC__out void** ppv);

    STDMETHODIMP_(ULONG) AddRef();
    
    STDMETHODIMP_(ULONG) Release();

    // INapComponentInfo

    STDMETHODIMP GetFriendlyName(
        /* [out] */ __RPC__out MessageId * friendlyName);

    STDMETHODIMP GetDescription(
        /* [out] */ __RPC__out MessageId * description);

    STDMETHODIMP GetVendorName(
        /* [out] */ __RPC__out MessageId * vendorName);

    STDMETHODIMP GetVersion(
        /* [out] */ __RPC__out MessageId * version);

    STDMETHODIMP GetIcon(
        /* [out] */ __RPC__deref_out CountedString ** dllFilePath, 
        /* [out] */ __RPC__out UINT32 * iconResourceId);

    STDMETHODIMP ConvertErrorCodeToMessageId(
        /* [in] */  __RPC__in HRESULT errorCode, 
        /* [out] */ __RPC__out MessageId * msgId);

    STDMETHODIMP GetLocalizedString(
        /* [in] */  MessageId msgId, 
        /* [out] */ __RPC__deref_out CountedString ** string);

private:

    long m_cRef;

};

