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
// In this sample, we have decided to show two tiles, one for
// Administrator and one for Guest.  You will need to decide what
// tiles make sense for your situation.  Can you enumerate the
// users who will use your method to log on?  Or is it better
// to provide a tile where they can type in their username?
// Does the user need to interact with something other than the
// keyboard before you can recognize which user it is (such as insert 
// a smartcard)?  We call these "event driven" credential providers.  
// We suggest that such credential providers first provide one basic tile which
// tells the user what to do ("insert your smartcard").  Once the
// user performs the action, then you can callback into LogonUI to
// tell it that you have new tiles, and include a tile that is specific
// to the user that the user can then interact with if necessary.

#include <credentialprovider.h>
#include "CSampleProvider.h"
#include "CSampleCredential.h"
#include "guid.h"

// CSampleProvider ////////////////////////////////////////////////////////

CSampleProvider::CSampleProvider():
    _cRef(1),
    _pkiulSetSerialization(NULL),
    _dwNumCreds(0),
    _bAutoSubmitSetSerializationCred(false),
    _dwSetSerializationCred(CREDENTIAL_PROVIDER_NO_DEFAULT)
{
    DllAddRef();

    ZeroMemory(_rgpCredentials, sizeof(_rgpCredentials));
}

CSampleProvider::~CSampleProvider()
{
    for (size_t i = 0; i < _dwNumCreds; i++)
    {
        if (_rgpCredentials[i] != NULL)
        {
            _rgpCredentials[i]->Release();
        }
    }

    DllRelease();
}

void CSampleProvider::_CleanupSetSerialization()
{
    if (_pkiulSetSerialization)
    {
        KERB_INTERACTIVE_LOGON* pkil = &_pkiulSetSerialization->Logon;
        SecureZeroMemory(_pkiulSetSerialization,
                         sizeof(*_pkiulSetSerialization) +
                         pkil->LogonDomainName.MaximumLength +
                         pkil->UserName.MaximumLength +
                         pkil->Password.MaximumLength);
        HeapFree(GetProcessHeap(),0, _pkiulSetSerialization);
    }
}



// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.  
//
// This sample only handles the logon and unlock scenarios as those are the most common.
HRESULT CSampleProvider::SetUsageScenario(
    __in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    __in DWORD dwFlags
    )
{
    UNREFERENCED_PARAMETER(dwFlags);
    HRESULT hr;

    static bool s_bCredsEnumerated = false;

    // Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
    // that we're not designed for that scenario.
    switch (cpus)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:       
        // A more advanced credprov might only enumerate tiles for the user whose owns the locked
        // session, since those are the only creds that wil work
        if (!s_bCredsEnumerated)
        {
            _cpus = cpus;

            hr = this->_EnumerateCredentials();
            s_bCredsEnumerated = true;
        }
        else
        {
            hr = S_OK;
        }
        break;

    case CPUS_CREDUI:
    case CPUS_CHANGE_PASSWORD:
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
// SetSerialization takes the serialization and uses it to create a credential.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// Since this sample doesn't support CPUS_CREDUI, we have not implemented the credui specific
// pieces of this function.  For information on that, please see the credUI sample.
HRESULT CSampleProvider::SetSerialization(
    __in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
    )
{
    HRESULT hr = E_INVALIDARG;

    if ((CLSID_CSample == pcpcs->clsidCredentialProvider))
    {
        // Get the current AuthenticationPackageID that we are supporting
        ULONG ulAuthPackage;
        hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);

        if (SUCCEEDED(hr))
        {
            if ((ulAuthPackage == pcpcs->ulAuthenticationPackage) &&
                (0 < pcpcs->cbSerialization && pcpcs->rgbSerialization))
            {
                KERB_INTERACTIVE_UNLOCK_LOGON* pkil = (KERB_INTERACTIVE_UNLOCK_LOGON*) pcpcs->rgbSerialization;
                if (KerbInteractiveLogon == pkil->Logon.MessageType)
                {
                    BYTE* rgbSerialization;
                    rgbSerialization = (BYTE*)HeapAlloc(GetProcessHeap(), 0, pcpcs->cbSerialization);
                    hr = rgbSerialization ? S_OK : E_OUTOFMEMORY;

                    if (SUCCEEDED(hr))
                    {
                        CopyMemory(rgbSerialization, pcpcs->rgbSerialization, pcpcs->cbSerialization);
                        KerbInteractiveUnlockLogonUnpackInPlace((KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization, pcpcs->cbSerialization);

                        if (_pkiulSetSerialization)
                        {
                            HeapFree(GetProcessHeap(), 0, _pkiulSetSerialization);
                            
                            // For this sample, we know that _dwSetSerializationCred is always in the last slot
                            if (_dwSetSerializationCred != CREDENTIAL_PROVIDER_NO_DEFAULT && _dwSetSerializationCred == _dwNumCreds - 1)
                            {
                                _rgpCredentials[_dwSetSerializationCred]->Release();
                                _rgpCredentials[_dwSetSerializationCred] = NULL;
                                _dwNumCreds--;
                                _dwSetSerializationCred = CREDENTIAL_PROVIDER_NO_DEFAULT;
                            }
                        }
                        _pkiulSetSerialization = (KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization;
                        hr = S_OK;
                    }
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

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
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

// Gets the field descriptor for a particular field
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
//
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default tile the behavior is the last used cred
// prov gets to specify the default tile to be displayed
//
// If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call GetSerialization
// on the credential you've specified as the default and will submit that credential
// for authentication without showing any further UI.
HRESULT CSampleProvider::GetCredentialCount(
    __out DWORD* pdwCount,
    __out_range(<,*pdwCount) DWORD* pdwDefault,
    __out BOOL* pbAutoLogonWithDefault
    )
{
    HRESULT hr = S_OK;

    if (_pkiulSetSerialization && _dwSetSerializationCred == CREDENTIAL_PROVIDER_NO_DEFAULT)
    {
        //haven't yet made a cred from the SetSerialization info
        _EnumerateSetSerialization();  //ignore failure, we can still produce our other tiles
    }
    
    *pdwCount = _dwNumCreds; 
    if (*pdwCount > 0)
    {
        if (_dwSetSerializationCred != CREDENTIAL_PROVIDER_NO_DEFAULT)
        {
            *pdwDefault = _dwSetSerializationCred;
        }
        else
        {
            // if we had reason to believe that one of our normal tiles should be the default
            // (like it was the last logged in user), we could set it to be the default here.  But
            // in our case we won't for now
            *pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
        }
        *pbAutoLogonWithDefault = _bAutoSubmitSetSerializationCred;
    }
    else
    {
        // no tiles, clear out out params
        *pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
        *pbAutoLogonWithDefault = FALSE;
        hr = E_FAIL;
    }

    return hr;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT CSampleProvider::GetCredentialAt(
    __in DWORD dwIndex, 
    __deref_out ICredentialProviderCredential** ppcpc
    )
{
    HRESULT hr;

    // Validate parameters.
    if((dwIndex < _dwNumCreds) && ppcpc)
    {
        hr = _rgpCredentials[dwIndex]->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
    }
    else
    {
        hr = E_INVALIDARG;
    }
        
    return hr;
}

// Creates a Credential with the SFI_USERNAME field's value set to pwzUsername.
HRESULT CSampleProvider::_EnumerateOneCredential(
    __in DWORD dwCredentialIndex,
    __in PCWSTR pwzUsername
    )
{
    HRESULT hr;

    // Allocate memory for the new credential.
    CSampleCredential* ppc = new CSampleCredential();
    
    if (ppc)
    {
        // Set the Field State Pair and Field Descriptors for ppc's fields
        // to the defaults (s_rgCredProvFieldDescriptors, and s_rgFieldStatePairs) and the value of SFI_USERNAME
        // to pwzUsername.
        hr = ppc->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, pwzUsername);
        
        if (SUCCEEDED(hr))
        {
            _rgpCredentials[dwCredentialIndex] = ppc;
            _dwNumCreds++;
        }
        else
        {
            // Release the pointer to account for the local reference.
            ppc->Release();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

// Sets up all the credentials for this provider. Since we always show the same tiles, 
// we just set it up once.
HRESULT CSampleProvider::_EnumerateCredentials()
{
    HRESULT hr = _EnumerateOneCredential(0, L"Administrator");
    if (SUCCEEDED(hr))
    {
        hr = _EnumerateOneCredential(1, L"Guest");
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

// This enumerates a tile for the info in _pkiulSetSerialization.  See the SetSerialization function comment for
// more information.
HRESULT CSampleProvider::_EnumerateSetSerialization()
{
    KERB_INTERACTIVE_LOGON* pkil = &_pkiulSetSerialization->Logon;

    _bAutoSubmitSetSerializationCred = false;

    // Since this provider only enumerates local users (not domain users) we are ignoring the domain passed in.
    // However, please note that if you receive a serialized cred of just a domain name, that domain name is meant 
    // to be the default domain for the tiles (or for the empty tile if you have one).  Also, depending on your scenario,
    // the presence of a domain other than what you're expecting might be a clue that you shouldn't handle
    // the SetSerialization.  For example, in this sample, we could choose to not accept a serialization for a cred
    // that had something other than the local machine name as the domain.

    // Use a "long" (MAX_PATH is arbitrary) buffer because it's hard to predict what will be
    // in the incoming values.  A DNS-format domain name, for instance, can be longer than DNLEN.
    WCHAR wszUsername[MAX_PATH] = {0};
    WCHAR wszPassword[MAX_PATH] = {0};

    // since this sample assumes local users, we'll ignore domain.  If you wanted to handle the domain
    // case, you'd have to update CSampleCredential::Initialize to take a domain.
    HRESULT hr = StringCbCopyNW(wszUsername, sizeof(wszUsername), pkil->UserName.Buffer, pkil->UserName.Length);

    if (SUCCEEDED(hr))
    {
        hr = StringCbCopyNW(wszPassword, sizeof(wszPassword), pkil->Password.Buffer, pkil->Password.Length);

        if (SUCCEEDED(hr))
        {
            CSampleCredential* pCred = new CSampleCredential();

            if (pCred)
            {
                hr = pCred->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, wszUsername, wszPassword);

                if (SUCCEEDED(hr))
                {
                    _rgpCredentials[_dwNumCreds] = pCred;  //array takes ref
                    _dwSetSerializationCred = _dwNumCreds;
                    _dwNumCreds++;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            // If we were passed all the info we need (in this case username & password), we're going to automatically submit this credential.
            if (SUCCEEDED(hr) && (0 < wcslen(wszPassword)))
            {
                _bAutoSubmitSetSerializationCred = true;
            }
        }
    }


    return hr;
}

