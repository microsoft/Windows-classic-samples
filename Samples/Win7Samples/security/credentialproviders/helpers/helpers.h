//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Helper functions for copying parameters and packaging the buffer
// for GetSerialization.

#pragma once
#include <credentialprovider.h>
#include <ntsecapi.h>
#define SECURITY_WIN32
#include <security.h>
#include <intsafe.h>

#include <windows.h>
#include <strsafe.h>

#pragma warning(push)
#pragma warning(disable : 4995)
#include <shlwapi.h>
#pragma warning(pop)

//makes a copy of a field descriptor using CoTaskMemAlloc
HRESULT FieldDescriptorCoAllocCopy(
    __in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
    __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    );

//makes a copy of a field descriptor on the normal heap
HRESULT FieldDescriptorCopy(
    __in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
    __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd
    );

//creates a UNICODE_STRING from a NULL-terminated string
HRESULT UnicodeStringInitWithString(
    __in PWSTR pwz, 
    __out UNICODE_STRING* pus
    );

//initializes a KERB_INTERACTIVE_UNLOCK_LOGON with weak references to the provided credentials
HRESULT KerbInteractiveUnlockLogonInit(
    __in PWSTR pwzDomain,
    __in PWSTR pwzUsername,
    __in PWSTR pwzPassword,
    __in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    __out KERB_INTERACTIVE_UNLOCK_LOGON* pkiul
    );

//packages the credentials into the buffer that the system expects
HRESULT KerbInteractiveUnlockLogonPack(
    __in const KERB_INTERACTIVE_UNLOCK_LOGON& rkiulIn,
    __deref_out_bcount(*pcb) BYTE** prgb,
    __out DWORD* pcb
    );

//get the authentication package that will be used for our logon attempt
HRESULT RetrieveNegotiateAuthPackage(
    __out ULONG * pulAuthPackage
    );

//encrypt a password (if necessary) and copy it; if not, just copy it
HRESULT ProtectIfNecessaryAndCopyPassword(
    __in PCWSTR pwzPassword,
    __in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    __deref_out PWSTR* ppwzProtectedPassword
    );

HRESULT KerbInteractiveUnlockLogonRepackNative(
    __in_bcount(cbWow) BYTE* rgbWow,
    __in DWORD cbWow,
    __deref_out_bcount(*pcbNative) BYTE** prgbNative,
    __out DWORD* pcbNative
    );

void KerbInteractiveUnlockLogonUnpackInPlace(
    __inout_bcount(cb) KERB_INTERACTIVE_UNLOCK_LOGON* pkiul,
    __in DWORD cb
    );

HRESULT DomainUsernameStringAlloc(
    __in PCWSTR pwszDomain,
    __in PCWSTR pwszUsername,
    __deref_out PWSTR* ppwszDomainUsername
    );
