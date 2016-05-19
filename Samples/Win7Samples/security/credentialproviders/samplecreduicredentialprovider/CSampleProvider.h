//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once

#include <credentialprovider.h>
#include <windows.h>
#include <strsafe.h>

#include "CSampleCredential.h"
#include "helpers.h"

#define MAX_CREDENTIALS 3

class CSampleProvider : public ICredentialProvider
{
  public:
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return ++_cRef;
    }
    
    IFACEMETHODIMP_(ULONG) Release()
    {
        LONG cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CSampleProvider, ICredentialProvider), // IID_ICredentialProvider
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

  public:
    IFACEMETHODIMP SetUsageScenario(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, __in DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(__in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    IFACEMETHODIMP Advise(__in ICredentialProviderEvents* pcpe, __in UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(__out DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(__in DWORD dwIndex,  __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    IFACEMETHODIMP GetCredentialCount(__out DWORD* pdwCount,
                                      __out_range(<,*pdwCount) DWORD* pdwDefault,
                                      __out BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(__in DWORD dwIndex, 
                                   __deref_out ICredentialProviderCredential** ppcpc);

    friend HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv);

  protected:
    CSampleProvider();
    __override ~CSampleProvider();
    
  private:
    
    HRESULT _EnumerateOneCredential(__in DWORD dwCredentialIndex,
                                    __in PCWSTR pwzUsername);

    // Create/free enumerated credentials.
    HRESULT _CreateEnumeratedCredentials();
    void _ReleaseEnumeratedCredentials();
    
    HRESULT _EnumerateCredentials(__in bool bAlreadyHaveSetSerializationCred = false); //this enumerates the normal set of 2 creds
    HRESULT _EnumerateSetSerialization(); //this will enumerate one tile with the contents of _pkiulSetSerialization

private:
    LONG              _cRef;
    CSampleCredential *_rgpCredentials[MAX_CREDENTIALS]; // Pointers to the credentials which will be enumerated by 
                                                         // this Provider.
    KERB_INTERACTIVE_UNLOCK_LOGON *     _pkiulSetSerialization;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO  _cpus;
    DWORD                               _dwCredUIFlags;
    bool                                _bRecreateEnumeratedCredentials;
    bool                                _bAutoSubmitSetSerializationCred;
    bool                                _bDefaultToFirstCredential;
};
