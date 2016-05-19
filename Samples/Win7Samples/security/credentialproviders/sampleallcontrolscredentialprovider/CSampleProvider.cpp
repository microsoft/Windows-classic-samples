//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CSampleProvider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// In this sample, we will display one tile that uses each of the nine
// available UI controls.

#include <credentialprovider.h>
#include "CSampleProvider.h"
#include "CSampleCredential.h"
#include "guid.h"

// CSampleProvider ////////////////////////////////////////////////////////

CSampleProvider::CSampleProvider():
    _cRef(1)
{
    DllAddRef();

    _pCredential = NULL;
}

CSampleProvider::~CSampleProvider()
{
    if (_pCredential != NULL)
    {
        _pCredential->Release();
        _pCredential = NULL;
    }

    DllRelease();
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
HRESULT CSampleProvider::SetUsageScenario(
    __in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    __in DWORD dwFlags
    )
{
    UNREFERENCED_PARAMETER(dwFlags);
    HRESULT hr;

    // Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
    // that we're not designed for that scenario.
    switch (cpus)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:       
        _cpus = cpus;

        // Create and initialize our credential.
        // A more advanced credprov might only enumerate tiles for the user whose owns the locked
        // session, since those are the only creds that wil work
        _pCredential = new CSampleCredential();
        if (_pCredential != NULL)
        {
            hr = _pCredential->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs);
            if (FAILED(hr))
            {
                _pCredential->Release();
                _pCredential = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        break;

    case CPUS_CHANGE_PASSWORD:
    case CPUS_CREDUI:
        hr = E_NOTIMPL;
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// If you wish to see an example of SetSerialization, please see either the SampleCredentialProvider
// sample or the SampleCredUICredentialProvider sample.  [The logonUI team says, "The original sample that
// this was built on top of didn't have SetSerialization.  And when we decided SetSerialization was
// important enough to have in the sample, it ended up being a non-trivial amount of work to integrate
// it into the main sample.  We felt it was more important to get these samples out to you quickly than to
// hold them in order to do the work to integrate the SetSerialization changes from SampleCredentialProvider 
// into this sample.]
HRESULT CSampleProvider::SetSerialization(
    __in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
    )
{
    UNREFERENCED_PARAMETER(pcpcs);
    return E_NOTIMPL;
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated.
HRESULT CSampleProvider::Advise(
    __in ICredentialProviderEvents* pcpe,
    __in UINT_PTR upAdviseContext
    )
{
    UNREFERENCED_PARAMETER(pcpe);
    UNREFERENCED_PARAMETER(upAdviseContext);

    return E_NOTIMPL;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CSampleProvider::UnAdvise()
{
    return E_NOTIMPL;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors.
HRESULT CSampleProvider::GetFieldDescriptorCount(
    __out DWORD* pdwCount
    )
{
    *pdwCount = SFI_NUM_FIELDS;
    return S_OK;
}

// Gets the field descriptor for a particular field.
HRESULT CSampleProvider::GetFieldDescriptorAt(
    __in DWORD dwIndex, 
    __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    )
{    
    HRESULT hr;

    // Verify dwIndex is a valid field.
    if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
    {
        hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
    }
    else
    { 
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default the last used cred prov gets to pick 
// the default. If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call 
// GetSerialization on the credential you've specified as the default and will submit 
// that credential for authentication without showing any further UI.
HRESULT CSampleProvider::GetCredentialCount(
    __out DWORD* pdwCount,
    __out_range(<,*pdwCount) DWORD* pdwDefault,
    __out BOOL* pbAutoLogonWithDefault
    )
{
    *pdwCount = 1;
    *pdwDefault = 0;
    *pbAutoLogonWithDefault = FALSE;
    return S_OK;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT CSampleProvider::GetCredentialAt(
    __in DWORD dwIndex, 
    __deref_out ICredentialProviderCredential** ppcpc
    )
{
    HRESULT hr;
    if((dwIndex == 0) && ppcpc)
    {
        hr = _pCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Boilerplate code to create our provider.
HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv)
{
    HRESULT hr;

    CSampleProvider* pProvider = new CSampleProvider();

    if (pProvider)
    {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}
