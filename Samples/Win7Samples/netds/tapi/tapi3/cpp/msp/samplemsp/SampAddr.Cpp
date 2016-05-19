/*++

Copyright (c) 1999 - 2000  Microsoft Corporation

Module Name:

    sampaddr.cpp 

Abstract:

    This module contains the implementation of CSampleMSP.

--*/

#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

CSampleMSP::CSampleMSP()
{
    LOG((MSP_TRACE, "CSampleMSP::CSampleMSP entered."));
    LOG((MSP_TRACE, "CSampleMSP::CSampleMSP exited."));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

CSampleMSP::~CSampleMSP()
{
    LOG((MSP_TRACE, "CSampleMSP::~CSampleMSP entered."));
    LOG((MSP_TRACE, "CSampleMSP::~CSampleMSP exited."));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

ULONG CSampleMSP::MSPAddressAddRef(void)
{
    return MSPAddRefHelper(this);
}

ULONG CSampleMSP::MSPAddressRelease(void)
{
    return MSPReleaseHelper(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CSampleMSP::CreateMSPCall(
    IN      MSP_HANDLE      htCall,
    IN      DWORD           dwReserved,
    IN      DWORD           dwMediaType,
    IN      IUnknown     *  pOuterUnknown,
    OUT     IUnknown    **  ppMSPCall
    )
{
    LOG((MSP_TRACE, "CSampleMSP::CreateMSPCall - enter"));

    CSampleMSPCall * pCSampleMSPCall;

    HRESULT hr = CreateMSPCallHelper<CSampleMSPCall>(this,
                                                   htCall,
                                                   dwReserved,
                                                   dwMediaType,
                                                   pOuterUnknown,
                                                   ppMSPCall,
                                                   &pCSampleMSPCall);

    //
    // pCSampleMSPCall is not addrefed; no need to release.
    //

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSP::CreateMSPCall - template helper returned"
            "0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CSampleMSP::CreateMSPCall - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CSampleMSP::ShutdownMSPCall (
    IN      IUnknown *          pMSPCall
    )
{
    LOG((MSP_TRACE, "CSampleMSP::ShutdownMSPCall - enter"));

    CSampleMSPCall * pCSampleMSPCall;

    HRESULT hr = ShutdownMSPCallHelper<CSampleMSPCall>(pMSPCall,
                                                     &pCSampleMSPCall);

    //
    // pCSampleMSPCall is not addrefed; no need to release.
    //

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CSampleMSP::ShutdownMSPCall - template helper returned"
            "0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CSampleMSP::ShutdownMSPCall - exit S_OK"));

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Mandatory CMSPAddress override. This indicates the media types that
// we support. For this sample MSP we only support audio.
//

DWORD CSampleMSP::GetCallMediaTypes(void)
{
    return (DWORD) TAPIMEDIATYPE_AUDIO;
}

// eof
