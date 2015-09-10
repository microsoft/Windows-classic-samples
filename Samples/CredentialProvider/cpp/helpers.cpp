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


#include "helpers.h"
#include <intsafe.h>

//
// Copies the field descriptor pointed to by rcpfd into a buffer allocated
// using CoTaskMemAlloc. Returns that buffer in ppcpfd.
//
HRESULT FieldDescriptorCoAllocCopy(
    _In_ const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR &rcpfd,
    _Outptr_result_nullonfailure_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd
    )
{
    HRESULT hr;
    *ppcpfd = nullptr;
    DWORD cbStruct = sizeof(**ppcpfd);

    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR *pcpfd = (CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*)CoTaskMemAlloc(cbStruct);
    if (pcpfd)
    {
        pcpfd->dwFieldID = rcpfd.dwFieldID;
        pcpfd->cpft = rcpfd.cpft;
        pcpfd->guidFieldType = rcpfd.guidFieldType;

        if (rcpfd.pszLabel)
        {
            hr = SHStrDupW(rcpfd.pszLabel, &pcpfd->pszLabel);
        }
        else
        {
            pcpfd->pszLabel = nullptr;
            hr = S_OK;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppcpfd = pcpfd;
    }
    else
    {
        CoTaskMemFree(pcpfd);   
    }

    return hr;
}

//
// Coppies rcpfd into the buffer pointed to by pcpfd. The caller is responsible for
// allocating pcpfd. This function uses CoTaskMemAlloc to allocate memory for
// pcpfd->pszLabel.
//
HRESULT FieldDescriptorCopy(
    _In_ const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR &rcpfd,
    _Out_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR *pcpfd
    )
{
    HRESULT hr;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR cpfd;

    cpfd.dwFieldID = rcpfd.dwFieldID;
    cpfd.cpft = rcpfd.cpft;
    cpfd.guidFieldType = rcpfd.guidFieldType;

    if (rcpfd.pszLabel)
    {
        hr = SHStrDupW(rcpfd.pszLabel, &cpfd.pszLabel);
    }
    else
    {
        cpfd.pszLabel = nullptr;
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        *pcpfd = cpfd;
    }

    return hr;
}

//
// This function copies the length of pwz and the pointer pwz into the UNICODE_STRING structure
// This function is intended for serializing a credential in GetSerialization only.
// Note that this function just makes a copy of the string pointer. It DOES NOT ALLOCATE storage!
// Be very, very sure that this is what you want, because it probably isn't outside of the
// exact GetSerialization call where the sample uses it.
//
HRESULT UnicodeStringInitWithString(
    _In_ PWSTR pwz,
    _Out_ UNICODE_STRING *pus
    )
{
    HRESULT hr;
    if (pwz)
    {
        size_t lenString = wcslen(pwz);
        USHORT usCharCount;
        hr = SizeTToUShort(lenString, &usCharCount);
        if (SUCCEEDED(hr))
        {
            USHORT usSize;
            hr = SizeTToUShort(sizeof(wchar_t), &usSize);
            if (SUCCEEDED(hr))
            {
                hr = UShortMult(usCharCount, usSize, &(pus->Length)); // Explicitly NOT including NULL terminator
                if (SUCCEEDED(hr))
                {
                    pus->MaximumLength = pus->Length;
                    pus->Buffer = pwz;
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

//
// The following function is intended to be used ONLY with the Kerb*Pack functions.  It does
// no bounds-checking because its callers have precise requirements and are written to respect
// its limitations.
// You can read more about the UNICODE_STRING type at:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthn/security/unicode_string.asp
//
static void _UnicodeStringPackedUnicodeStringCopy(
    __in const UNICODE_STRING& rus,
    __in PWSTR pwzBuffer,
    __out UNICODE_STRING *pus
    )
{
    pus->Length = rus.Length;
    pus->MaximumLength = rus.Length;
    pus->Buffer = pwzBuffer;

    CopyMemory(pus->Buffer, rus.Buffer, pus->Length);
}

//
// Initialize the members of a KERB_INTERACTIVE_UNLOCK_LOGON with weak references to the
// passed-in strings.  This is useful if you will later use KerbInteractiveUnlockLogonPack
// to serialize the structure.
//
// The password is stored in encrypted form for CPUS_LOGON and CPUS_UNLOCK_WORKSTATION
// because the system can accept encrypted credentials.  It is not encrypted in CPUS_CREDUI
// because we cannot know whether our caller can accept encrypted credentials.
//
HRESULT KerbInteractiveUnlockLogonInit(
    _In_ PWSTR pwzDomain,
    _In_ PWSTR pwzUsername,
    _In_ PWSTR pwzPassword,
    _In_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    _Out_ KERB_INTERACTIVE_UNLOCK_LOGON *pkiul
    )
{
    KERB_INTERACTIVE_UNLOCK_LOGON kiul;
    ZeroMemory(&kiul, sizeof(kiul));

    KERB_INTERACTIVE_LOGON *pkil = &kiul.Logon;

    // Note: this method uses custom logic to pack a KERB_INTERACTIVE_UNLOCK_LOGON with a
    // serialized credential.  We could replace the calls to UnicodeStringInitWithString
    // and KerbInteractiveUnlockLogonPack with a single cal to CredPackAuthenticationBuffer,
    // but that API has a drawback: it returns a KERB_INTERACTIVE_UNLOCK_LOGON whose
    // MessageType is always KerbInteractiveLogon.
    //
    // If we only handled CPUS_LOGON, this drawback would not be a problem.  For
    // CPUS_UNLOCK_WORKSTATION, we could cast the output buffer of CredPackAuthenticationBuffer
    // to KERB_INTERACTIVE_UNLOCK_LOGON and modify the MessageType to KerbWorkstationUnlockLogon,
    // but such a cast would be unsupported -- the output format of CredPackAuthenticationBuffer
    // is not officially documented.

    // Initialize the UNICODE_STRINGS to share our username and password strings.
    HRESULT hr = UnicodeStringInitWithString(pwzDomain, &pkil->LogonDomainName);
    if (SUCCEEDED(hr))
    {
        hr = UnicodeStringInitWithString(pwzUsername, &pkil->UserName);
        if (SUCCEEDED(hr))
        {
            hr = UnicodeStringInitWithString(pwzPassword, &pkil->Password);
            if (SUCCEEDED(hr))
            {
                // Set a MessageType based on the usage scenario.
                switch (cpus)
                {
                case CPUS_UNLOCK_WORKSTATION:
                    pkil->MessageType = KerbWorkstationUnlockLogon;
                    hr = S_OK;
                    break;

                case CPUS_LOGON:
                    pkil->MessageType = KerbInteractiveLogon;
                    hr = S_OK;
                    break;

                case CPUS_CREDUI:
                    pkil->MessageType = (KERB_LOGON_SUBMIT_TYPE)0; // MessageType does not apply to CredUI
                    hr = S_OK;
                    break;

                default:
                    hr = E_FAIL;
                    break;
                }

                if (SUCCEEDED(hr))
                {
                    // KERB_INTERACTIVE_UNLOCK_LOGON is just a series of structures.  A
                    // flat copy will properly initialize the output parameter.
                    CopyMemory(pkiul, &kiul, sizeof(*pkiul));
                }
            }
        }
    }

    return hr;
}

//
// WinLogon and LSA consume "packed" KERB_INTERACTIVE_UNLOCK_LOGONs.  In these, the PWSTR members of each
// UNICODE_STRING are not actually pointers but byte offsets into the overall buffer represented
// by the packed KERB_INTERACTIVE_UNLOCK_LOGON.  For example:
//
// rkiulIn.Logon.LogonDomainName.Length = 14                                    -> Length is in bytes, not characters
// rkiulIn.Logon.LogonDomainName.Buffer = sizeof(KERB_INTERACTIVE_UNLOCK_LOGON) -> LogonDomainName begins immediately
//                                                                              after the KERB_... struct in the buffer
// rkiulIn.Logon.UserName.Length = 10
// rkiulIn.Logon.UserName.Buffer = sizeof(KERB_INTERACTIVE_UNLOCK_LOGON) + 14   -> UNICODE_STRINGS are NOT null-terminated
//
// rkiulIn.Logon.Password.Length = 16
// rkiulIn.Logon.Password.Buffer = sizeof(KERB_INTERACTIVE_UNLOCK_LOGON) + 14 + 10
//
// THere's more information on this at:
// http://msdn.microsoft.com/msdnmag/issues/05/06/SecurityBriefs/#void
//

HRESULT KerbInteractiveUnlockLogonPack(
    _In_ const KERB_INTERACTIVE_UNLOCK_LOGON &rkiulIn,
    _Outptr_result_bytebuffer_(*pcb) BYTE **prgb,
    _Out_ DWORD *pcb
    )
{
    HRESULT hr;

    const KERB_INTERACTIVE_LOGON *pkilIn = &rkiulIn.Logon;

    // alloc space for struct plus extra for the three strings
    DWORD cb = sizeof(rkiulIn) +
        pkilIn->LogonDomainName.Length +
        pkilIn->UserName.Length +
        pkilIn->Password.Length;

    KERB_INTERACTIVE_UNLOCK_LOGON *pkiulOut = (KERB_INTERACTIVE_UNLOCK_LOGON*)CoTaskMemAlloc(cb);
    if (pkiulOut)
    {
        ZeroMemory(&pkiulOut->LogonId, sizeof(pkiulOut->LogonId));

        //
        // point pbBuffer at the beginning of the extra space
        //
        BYTE *pbBuffer = (BYTE*)pkiulOut + sizeof(*pkiulOut);

        //
        // set up the Logon structure within the KERB_INTERACTIVE_UNLOCK_LOGON
        //
        KERB_INTERACTIVE_LOGON *pkilOut = &pkiulOut->Logon;

        pkilOut->MessageType = pkilIn->MessageType;

        //
        // copy each string,
        // fix up appropriate buffer pointer to be offset,
        // advance buffer pointer over copied characters in extra space
        //
        _UnicodeStringPackedUnicodeStringCopy(pkilIn->LogonDomainName, (PWSTR)pbBuffer, &pkilOut->LogonDomainName);
        pkilOut->LogonDomainName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
        pbBuffer += pkilOut->LogonDomainName.Length;

        _UnicodeStringPackedUnicodeStringCopy(pkilIn->UserName, (PWSTR)pbBuffer, &pkilOut->UserName);
        pkilOut->UserName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
        pbBuffer += pkilOut->UserName.Length;

        _UnicodeStringPackedUnicodeStringCopy(pkilIn->Password, (PWSTR)pbBuffer, &pkilOut->Password);
        pkilOut->Password.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);

        *prgb = (BYTE*)pkiulOut;
        *pcb = cb;

        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

//
// This function packs the string pszSourceString in pszDestinationString
// for use with LSA functions including LsaLookupAuthenticationPackage.
//
static HRESULT _LsaInitString(
    __out PSTRING pszDestinationString,
    __in PCSTR pszSourceString
    )
{
    size_t cchLength = strlen(pszSourceString);
    USHORT usLength;
    HRESULT hr = SizeTToUShort(cchLength, &usLength);
    if (SUCCEEDED(hr))
    {
        pszDestinationString->Buffer = (PCHAR)pszSourceString;
        pszDestinationString->Length = usLength;
        pszDestinationString->MaximumLength = pszDestinationString->Length+1;
        hr = S_OK;
    }
    return hr;
}

//
// Retrieves the 'negotiate' AuthPackage from the LSA. In this case, Kerberos
// For more information on auth packages see this msdn page:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthn/security/msv1_0_lm20_logon.asp
//
HRESULT RetrieveNegotiateAuthPackage(_Out_ ULONG *pulAuthPackage)
{
    HRESULT hr;
    HANDLE hLsa;

    NTSTATUS status = LsaConnectUntrusted(&hLsa);
    if (SUCCEEDED(HRESULT_FROM_NT(status)))
    {
        ULONG ulAuthPackage;
        LSA_STRING lsaszKerberosName;
        _LsaInitString(&lsaszKerberosName, NEGOSSP_NAME_A);

        status = LsaLookupAuthenticationPackage(hLsa, &lsaszKerberosName, &ulAuthPackage);
        if (SUCCEEDED(HRESULT_FROM_NT(status)))
        {
            *pulAuthPackage = ulAuthPackage;
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_NT(status);
        }
        LsaDeregisterLogonProcess(hLsa);
    }
    else
    {
        hr = HRESULT_FROM_NT(status);
    }

    return hr;
}

//
// Return a copy of pwzToProtect encrypted with the CredProtect API.
//
// pwzToProtect must not be NULL or the empty string.
//
static HRESULT _ProtectAndCopyString(
    _In_ PCWSTR pwzToProtect,
    _Outptr_result_nullonfailure_ PWSTR *ppwzProtected
    )
{
    *ppwzProtected = nullptr;

    // pwzToProtect is const, but CredProtect takes a non-const string.
    // So, make a copy that we know isn't const.
    PWSTR pwzToProtectCopy;
    HRESULT hr = SHStrDupW(pwzToProtect, &pwzToProtectCopy);
    if (SUCCEEDED(hr))
    {
        // The first call to CredProtect determines the length of the encrypted string.
        // Because we pass a NULL output buffer, we expect the call to fail.
        //
        // Note that the third parameter to CredProtect, the number of characters of pwzToProtectCopy
        // to encrypt, must include the NULL terminator!
        DWORD cchProtected = 0;
        if (!CredProtectW(FALSE, pwzToProtectCopy, (DWORD)wcslen(pwzToProtectCopy)+1, nullptr, &cchProtected, nullptr))
        {
            DWORD dwErr = GetLastError();

            if ((ERROR_INSUFFICIENT_BUFFER == dwErr) && (0 < cchProtected))
            {
                // Allocate a buffer long enough for the encrypted string.
                PWSTR pwzProtected = (PWSTR)CoTaskMemAlloc(cchProtected * sizeof(wchar_t));
                if (pwzProtected)
                {
                    // The second call to CredProtect actually encrypts the string.
                    if (CredProtectW(FALSE, pwzToProtectCopy, (DWORD)wcslen(pwzToProtectCopy)+1, pwzProtected, &cchProtected, nullptr))
                    {
                        *ppwzProtected = pwzProtected;
                        hr = S_OK;
                    }
                    else
                    {
                        CoTaskMemFree(pwzProtected);

                        dwErr = GetLastError();
                        hr = HRESULT_FROM_WIN32(dwErr);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }

        CoTaskMemFree(pwzToProtectCopy);
    }

    return hr;
}

//
// If pwzPassword should be encrypted, return a copy encrypted with CredProtect.
//
// If not, just return a copy.
//
HRESULT ProtectIfNecessaryAndCopyPassword(
    _In_ PCWSTR pwzPassword,
    _In_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    _Outptr_result_nullonfailure_ PWSTR *ppwzProtectedPassword
    )
{
    *ppwzProtectedPassword = nullptr;

    HRESULT hr;

    // ProtectAndCopyString is intended for non-empty strings only.  Empty passwords
    // do not need to be encrypted.
    if (pwzPassword && *pwzPassword)
    {
        // pwzPassword is const, but CredIsProtected takes a non-const string.
        // So, ake a copy that we know isn't const.
        PWSTR pwzPasswordCopy;
        hr = SHStrDupW(pwzPassword, &pwzPasswordCopy);
        if (SUCCEEDED(hr))
        {
            bool bCredAlreadyEncrypted = false;
            CRED_PROTECTION_TYPE protectionType;

            // If the password is already encrypted, we should not encrypt it again.
            // An encrypted password may be received through SetSerialization in the
            // CPUS_LOGON scenario during a Terminal Services connection, for instance.
            if (CredIsProtectedW(pwzPasswordCopy, &protectionType))
            {
                if (CredUnprotected != protectionType)
                {
                    bCredAlreadyEncrypted = true;
                }
            }

            // Passwords should not be encrypted in the CPUS_CREDUI scenario.  We
            // cannot know if our caller expects or can handle an encryped password.
            if (CPUS_CREDUI == cpus || bCredAlreadyEncrypted)
            {
                hr = SHStrDupW(pwzPasswordCopy, ppwzProtectedPassword);
            }
            else
            {
                hr = _ProtectAndCopyString(pwzPasswordCopy, ppwzProtectedPassword);
            }

            CoTaskMemFree(pwzPasswordCopy);
        }
    }
    else
    {
        hr = SHStrDupW(L"", ppwzProtectedPassword);
    }

    return hr;
}

//
// Unpack a KERB_INTERACTIVE_UNLOCK_LOGON *in place*.  That is, reset the Buffers from being offsets to
// being real pointers.  This means, of course, that passing the resultant struct across any sort of
// memory space boundary is not going to work -- repack it if necessary!
//
void KerbInteractiveUnlockLogonUnpackInPlace(
    _Inout_updates_bytes_(cb) KERB_INTERACTIVE_UNLOCK_LOGON *pkiul,
    DWORD cb
    )
{
    if (sizeof(*pkiul) <= cb)
    {
        KERB_INTERACTIVE_LOGON *pkil = &pkiul->Logon;

        // Sanity check: if the range described by each (Buffer + MaximumSize) falls within the total bytecount,
        // we can be pretty confident that the Buffers are actually offsets and that this is a packed credential.
        if (((ULONG_PTR)pkil->LogonDomainName.Buffer + pkil->LogonDomainName.MaximumLength <= cb) &&
            ((ULONG_PTR)pkil->UserName.Buffer + pkil->UserName.MaximumLength <= cb) &&
            ((ULONG_PTR)pkil->Password.Buffer + pkil->Password.MaximumLength <= cb))
        {
            pkil->LogonDomainName.Buffer = pkil->LogonDomainName.Buffer
                ? (PWSTR)((BYTE*)pkiul + (ULONG_PTR)pkil->LogonDomainName.Buffer)
                : nullptr;

            pkil->UserName.Buffer = pkil->UserName.Buffer
                ? (PWSTR)((BYTE*)pkiul + (ULONG_PTR)pkil->UserName.Buffer)
                : nullptr;

            pkil->Password.Buffer = pkil->Password.Buffer
                ? (PWSTR)((BYTE*)pkiul + (ULONG_PTR)pkil->Password.Buffer)
                : nullptr;
        }
    }
}

//
// Use the CredPackAuthenticationBuffer and CredUnpackAuthenticationBuffer to convert a 32 bit WOW
// cred blob into a 64 bit native blob by unpacking it and immediately repacking it.
//
HRESULT KerbInteractiveUnlockLogonRepackNative(
    _In_reads_bytes_(cbWow) BYTE *rgbWow,
    _In_ DWORD cbWow,
    _Outptr_result_bytebuffer_(*pcbNative) BYTE **prgbNative,
    _Out_ DWORD *pcbNative
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    PWSTR pszDomainUsername = nullptr;
    DWORD cchDomainUsername = 0;
    PWSTR pszPassword = nullptr;
    DWORD cchPassword = 0;

    *prgbNative = nullptr;
    *pcbNative = 0;

    // Unpack the 32 bit KERB structure
    CredUnPackAuthenticationBufferW(CRED_PACK_WOW_BUFFER, rgbWow, cbWow, pszDomainUsername, &cchDomainUsername, nullptr, nullptr, pszPassword, &cchPassword);
    if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
    {
        pszDomainUsername = (PWSTR) LocalAlloc(0, cchDomainUsername * sizeof(wchar_t));
        if (pszDomainUsername)
        {
            pszPassword = (PWSTR) LocalAlloc(0, cchPassword * sizeof(wchar_t));
            if (pszPassword)
            {
                if (CredUnPackAuthenticationBufferW(CRED_PACK_WOW_BUFFER, rgbWow, cbWow, pszDomainUsername, &cchDomainUsername, nullptr, nullptr, pszPassword, &cchPassword))
                {
                    hr = S_OK;
                }
                else
                {
                    hr = GetLastError();
                }
            }
        }
    }

    // Repack native
    if (SUCCEEDED(hr))
    {
        hr = E_OUTOFMEMORY;
        CredPackAuthenticationBufferW(0, pszDomainUsername, pszPassword, *prgbNative, pcbNative);
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
            *prgbNative = (BYTE*) LocalAlloc(LMEM_ZEROINIT, *pcbNative);
            if (*prgbNative)
            {
                if (CredPackAuthenticationBufferW(0, pszDomainUsername, pszPassword, *prgbNative, pcbNative))
                {
                    hr = S_OK;
                }
                else
                {
                    LocalFree(*prgbNative);
                }
            }
        }
    }

    LocalFree(pszDomainUsername);
    if (pszPassword)
    {
        SecureZeroMemory(pszPassword, cchPassword * sizeof(wchar_t));
        LocalFree(pszPassword);
    }
    return hr;
}

// Concatonates pwszDomain and pwszUsername and places the result in *ppwszDomainUsername.
HRESULT DomainUsernameStringAlloc(
    _In_ PCWSTR pwszDomain,
    _In_ PCWSTR pwszUsername,
    _Outptr_result_nullonfailure_ PWSTR *ppwszDomainUsername
    )
{
    HRESULT hr;
    *ppwszDomainUsername = nullptr;
    size_t cchDomain = wcslen(pwszDomain);
    size_t cchUsername = wcslen(pwszUsername);
    // Length of domain, 1 character for '\', length of Username, plus null terminator.
    size_t cbLen = sizeof(wchar_t) * (cchDomain + 1 + cchUsername +1);
    PWSTR pwszDest = (PWSTR)HeapAlloc(GetProcessHeap(), 0, cbLen);
    if (pwszDest)
    {
        hr = StringCbPrintfW(pwszDest, cbLen, L"%s\\%s", pwszDomain, pwszUsername);
        if (SUCCEEDED(hr))
        {
            *ppwszDomainUsername = pwszDest;
        }
        else
        {
            HeapFree(GetProcessHeap(), 0, pwszDest);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT SplitDomainAndUsername(_In_ PCWSTR pszQualifiedUserName, _Outptr_result_nullonfailure_ PWSTR *ppszDomain, _Outptr_result_nullonfailure_ PWSTR *ppszUsername)
{
    HRESULT hr = E_UNEXPECTED;
    *ppszDomain = nullptr;
    *ppszUsername = nullptr;
    PWSTR pszDomain;
    PWSTR pszUsername;
    const wchar_t *pchWhack = wcschr(pszQualifiedUserName, L'\\');
    const wchar_t *pchEnd = pszQualifiedUserName + wcslen(pszQualifiedUserName) - 1;

    if (pchWhack != nullptr)
    {
        const wchar_t *pchDomainBegin = pszQualifiedUserName;
        const wchar_t *pchDomainEnd = pchWhack - 1;
        const wchar_t *pchUsernameBegin = pchWhack + 1;
        const wchar_t *pchUsernameEnd = pchEnd;

        size_t lenDomain = pchDomainEnd - pchDomainBegin + 1; // number of actual chars, NOT INCLUDING null terminated string
        pszDomain = static_cast<PWSTR>(CoTaskMemAlloc(sizeof(wchar_t) * (lenDomain + 1)));
        if (pszDomain != nullptr)
        {
            hr = StringCchCopyN(pszDomain, lenDomain + 1, pchDomainBegin, lenDomain);
            if (SUCCEEDED(hr))
            {
                size_t lenUsername = pchUsernameEnd - pchUsernameBegin + 1; // number of actual chars, NOT INCLUDING null terminated string
                pszUsername = static_cast<PWSTR>(CoTaskMemAlloc(sizeof(wchar_t) * (lenUsername + 1)));
                if (pszUsername != nullptr)
                {
                    hr = StringCchCopyN(pszUsername, lenUsername + 1, pchUsernameBegin, lenUsername);
                    if (SUCCEEDED(hr))
                    {
                        *ppszDomain = pszDomain;
                        *ppszUsername = pszUsername;
                    }
                    else
                    {
                        CoTaskMemFree(pszUsername);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            if (FAILED(hr))
            {
                CoTaskMemFree(pszDomain);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}