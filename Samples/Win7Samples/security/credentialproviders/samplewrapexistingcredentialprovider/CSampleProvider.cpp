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
// In this sample, we are wrapping the default password provider with
// an extra small text and combobox. We pass nearly all requests to the
// wrapped provider, except for the ones that are for fields we're
// responsible for ourselves. As far as the owner is concerned, we are a
// unique provider, so they never know we're wrapping another provider.

#include <credentialprovider.h>
#include "CSampleProvider.h"
#include "CSampleCredential.h"
#include "guid.h"

// CSampleProvider ////////////////////////////////////////////////////////

CSampleProvider::CSampleProvider():
    _cRef(1)
{
    DllAddRef();

    _rgpCredentials = NULL;
    _dwCredentialCount = 0;

    _pWrappedProvider = NULL;
    _dwWrappedDescriptorCount = 0;
}

CSampleProvider::~CSampleProvider()
{
    _CleanUpAllCredentials();
    
    if (_pWrappedProvider)
    {
        _pWrappedProvider->Release();
    }

    DllRelease();
}

// Cleans up all credentials, including the memory used to allocate the array.
void CSampleProvider::_CleanUpAllCredentials()
{
    // Iterate and clean up the array, if it exists.
    if (_rgpCredentials != NULL)
    {
        for (DWORD lcv = 0; lcv < _dwCredentialCount; lcv++)
        {
            if (_rgpCredentials[lcv] != NULL)
            {
                _rgpCredentials[lcv]->Release();
                _rgpCredentials[lcv] = NULL;
            }
        }
        delete [] _rgpCredentials;
        _rgpCredentials = NULL;
    }
}

// Ordinarily we would look at the CPUS and decide whether or not we support this scenario.
// However, in this scenario we're going to create our internal provider and let it answer
// questions like this for us.
HRESULT CSampleProvider::SetUsageScenario(
    __in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    __in DWORD dwFlags
    )
{
    HRESULT hr;

    // Create the password credential provider and query its interface for an
    // ICredentialProvider we can use. Once it's up and running, ask it about the 
    // usage scenario being provided.
    IUnknown *pUnknown = NULL;
    hr = CoCreateInstance(CLSID_PasswordCredentialProvider, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pUnknown));
    if (SUCCEEDED(hr))
    {
        hr = pUnknown->QueryInterface(IID_PPV_ARGS(&(_pWrappedProvider)));
        if (SUCCEEDED(hr))
        {
            hr = _pWrappedProvider->SetUsageScenario(cpus, dwFlags);
        }
        pUnknown->Release();
    }
    if (FAILED(hr))
    {
        if (_pWrappedProvider != NULL)
        {
            _pWrappedProvider->Release();
            _pWrappedProvider = NULL;
        }
    }

    return hr;
}

// We pass this along to the wrapped provider.
HRESULT CSampleProvider::SetSerialization(
    __in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
    )
{
    HRESULT hr = E_UNEXPECTED;
    
    if (_pWrappedProvider != NULL)
    {
        hr = _pWrappedProvider->SetSerialization(pcpcs);
    }

    return hr;
}

// Called by LogonUI to give you a callback. We pass this along to the wrapped provider.
HRESULT CSampleProvider::Advise(
    __in ICredentialProviderEvents* pcpe,
    __in UINT_PTR upAdviseContext
    )
{
    HRESULT hr = E_UNEXPECTED;
    if (_pWrappedProvider != NULL)
    {
        hr = _pWrappedProvider->Advise(pcpe, upAdviseContext);
    }
    return hr;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid. 
// We pass this along to the wrapped provider.
HRESULT CSampleProvider::UnAdvise()
{
    HRESULT hr = E_UNEXPECTED;
    if (_pWrappedProvider != NULL)
    {
        hr = _pWrappedProvider->UnAdvise();
    }
    return hr;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors. We pass this along to the wrapped provider and then append
// our own credential count.
HRESULT CSampleProvider::GetFieldDescriptorCount(
    __out DWORD* pdwCount
    )
{
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedProvider != NULL)
    {
        hr = _pWrappedProvider->GetFieldDescriptorCount(&(_dwWrappedDescriptorCount));
        if (SUCCEEDED(hr))
        {
            // Note that we need to add our own credential count to the wrapped credential's
            // total count.
            *pdwCount = _dwWrappedDescriptorCount + SFI_NUM_FIELDS;
        }
    }

    return hr;
}

// Gets the field descriptor for a particular field. If this descriptor refers to one owned
// by our wrapped provider, we'll pass it along. Otherwise we provide our own.
HRESULT CSampleProvider::GetFieldDescriptorAt(
    __in DWORD dwIndex, 
    __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    )
{    
    HRESULT hr = E_UNEXPECTED;

    if (_pWrappedProvider != NULL)
    {
        if (ppcpfd != NULL)
        {
            // If this field maps to one in the wrapped provider, hand it off.
            if (dwIndex < _dwWrappedDescriptorCount)
            {
                hr = _pWrappedProvider->GetFieldDescriptorAt(dwIndex, ppcpfd);
            }
            // Otherwise, check to see if it's ours and then handle it here.
            else
            {
                // Offset into the descriptor count so we can index our own fields.
                dwIndex -= _dwWrappedDescriptorCount;

                // Verify dwIndex is still a valid field.
                if (dwIndex < SFI_NUM_FIELDS)
                {
                    hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
                    (*ppcpfd)->dwFieldID += _dwWrappedDescriptorCount;
                }
                else
                { 
                    hr = E_INVALIDARG;
                }
            }
        }
        else
        { 
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default tile the last cred prov used can select
// the default tile. 
// If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call GetSerialization
// on the credential you've specified as the default and will submit that credential
// for authentication without showing any further UI.
// While we're here, we'll create credentials to wrap each of the credentials created by
// our wrapped provider. The key is to make everything transparent to the owner.
HRESULT CSampleProvider::GetCredentialCount(
    __out DWORD* pdwCount,
    __out_range(<,*pdwCount) DWORD* pdwDefault,
    __out BOOL* pbAutoLogonWithDefault
    )
{
    HRESULT hr = E_UNEXPECTED;
    DWORD dwDefault = 0;
    BOOL bAutoLogonWithDefault = FALSE;

    // Make sure we've created the provider.
    if (_pWrappedProvider != NULL)
    {
        // This probably shouldn't happen, but in the event that this gets called after
        // we've already been through once, we want to clean up everything before 
        // allocating new stuff all over again.
        if (_rgpCredentials != NULL)
        {
            _CleanUpAllCredentials();
        }

        // We need to know how many fields each credential has in order to initialize
        // our wrapper credentials, so we might as well do that here before anything else.
        DWORD count;
        hr = GetFieldDescriptorCount(&(count));

        if (SUCCEEDED(hr))
        {
            // Grab the credential count of the wrapped provider. We'll simply wrap each.
            hr = _pWrappedProvider->GetCredentialCount(&(_dwCredentialCount), &(dwDefault), &(bAutoLogonWithDefault));

            if (SUCCEEDED(hr))
            {
                // Create an array of credentials for use.
                _rgpCredentials = new CSampleCredential*[_dwCredentialCount];
                if (_rgpCredentials != NULL)
                {
                    // Iterate each credential and make a wrapper.
                    for (DWORD lcv = 0; SUCCEEDED(hr) && (lcv < _dwCredentialCount); lcv++)
                    {
                        // Allocate memory for the new credential.
                        _rgpCredentials[lcv] = new CSampleCredential();
                        if (_rgpCredentials[lcv] != NULL)
                        {
                            ICredentialProviderCredential *pCredential;
                            hr = _pWrappedProvider->GetCredentialAt(lcv, &(pCredential));
                            if (SUCCEEDED(hr))
                            {
                                // Set the Field State Pair and Field Descriptors for ppc's 
                                // fields to the defaults (s_rgCredProvFieldDescriptors, 
                                // and s_rgFieldStatePairs) and the value of SFI_USERNAME
                                // to pwzUsername.
                                hr = _rgpCredentials[lcv]->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, pCredential, _dwWrappedDescriptorCount);
                                if (FAILED(hr))
                                {
                                    // If initialization failed, clean everything up.
                                    for (lcv = 0; lcv < _dwCredentialCount; lcv++)
                                    {
                                        if (_rgpCredentials[lcv] != NULL)
                                        {
                                            // Release the pointer to account for the local reference.
                                            _rgpCredentials[lcv]->Release();
                                            _rgpCredentials[lcv] = NULL;
                                        }
                                    }
                                }
                                pCredential->Release();
                            } // (End if _pWrappedProvider->GetCredentialAt succeeded.)
                        } // (End if allocating _rgpCredentials[lcv] succeeded.)
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        } 
                    } // (End of _rgpCredentials allocation loop.)
                } // (End if for allocating _rgpCredentials succeeded.)
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            } // (End if _pWrappedProvider->GetCredentialCount succeeded.)
        } // (End if GetFieldDescriptorCount succeeded.)
    }

    if (FAILED(hr))
    {
        // Clean up.
        if (_rgpCredentials != NULL)
        {
            delete _rgpCredentials;
            _rgpCredentials = NULL;
        }
    }
    else
    {
        *pdwCount = _dwCredentialCount;
        *pdwDefault = dwDefault;
        *pbAutoLogonWithDefault = bAutoLogonWithDefault;
    }

    return hr;
}

// Returns the credential at the index specified by dwIndex. This function is called by 
// logonUI to enumerate the tiles.
HRESULT CSampleProvider::GetCredentialAt(
    __in DWORD dwIndex, 
    __in ICredentialProviderCredential** ppcpc
    )
{
    HRESULT hr;

    // Validate parameters.
    if ((dwIndex < _dwCredentialCount) && 
        (ppcpc != NULL) &&
        (_rgpCredentials != NULL) &&
        (_rgpCredentials[dwIndex] != NULL))
    {
        hr = _rgpCredentials[dwIndex]->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
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
