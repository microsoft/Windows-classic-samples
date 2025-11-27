#pragma once
#include "pch.h"
#include <webauthn.h>
#include <webauthnplugin.h>


#pragma region Checking for Windows SDK version 10.0.26100.7175 or higher
template<typename, typename = void> constexpr bool is_type_complete_v = false;
template<typename T> constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;

static_assert(is_type_complete_v<struct _WEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_OPTIONS>, "This sample requires Windows SDK version 10.0.26100.7175 or higher");
#pragma endregion


namespace {
    inline HMODULE GetWebAuthnDll()
    {
        static wil::unique_hmodule s_dll(LoadLibraryExW(L"webauthn.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32));
        return s_dll.get();
    }
}

// Delay-loaded WebAuthn API wrappers
// These match the signatures from webauthn.h and webauthnplugin.h

//
// Plugin Management APIs
//

// This Plugin Management API allows third-party developers to register their passkey authenticator plugin
// with the Windows platform on devices running Windows. This API is part of the plugin management system
// that enables plugins to participate in web authentication flows after proper COM class registration and
// user enablement in Settings.
inline HRESULT WINAPI WebAuthNPluginAddAuthenticator(
    _In_ PCWEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_OPTIONS pPluginAddAuthenticatorOptions,
    _Outptr_result_maybenull_ PWEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_RESPONSE* ppPluginAddAuthenticatorResponse)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginAddAuthenticator);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(pPluginAddAuthenticatorOptions, ppPluginAddAuthenticatorResponse);
}

// This cleanup API frees memory allocated by the platform for add authenticator responses.
inline void WINAPI WebAuthNPluginFreeAddAuthenticatorResponse(
    _In_opt_ PWEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_RESPONSE pPluginAddAuthenticatorResponse)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginFreeAddAuthenticatorResponse);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(pPluginAddAuthenticatorResponse);
}

// This Plugin Management API allows third-party passkey authenticator plugins to unregister themselves
// from the Windows platform. This removes the plugin from the system and prevents it from participating
// in future web authentication flows until re-registered.
inline HRESULT WINAPI WebAuthNPluginRemoveAuthenticator(
    _In_ REFCLSID rclsid)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginRemoveAuthenticator);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid);
}

// This Plugin Management API allows registered third-party passkey authenticator plugins to update their
// registration details such as name, logos, authenticator information, and supported RP IDs without full
// re-registration.
inline HRESULT WINAPI WebAuthNPluginUpdateAuthenticatorDetails(
    _In_ PCWEBAUTHN_PLUGIN_UPDATE_AUTHENTICATOR_DETAILS pPluginUpdateAuthenticatorDetails)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginUpdateAuthenticatorDetails);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(pPluginUpdateAuthenticatorDetails);
}

// This Plugin Management API retrieves the current state of a registered plugin authenticator.
// This allows plugins to check if they are properly registered and enabled by the user in the Windows Settings app.
// The state indicates whether the plugin can participate in web authentication flows.
// At registration time, the plugin is in the disabled state. The plugin may show up in web authentication flows,
// but will require the user to see an additional prompt to enable the plugin.
// Irrespective of the state, the plugin will be shown in the Windows Settings app and can add or remove credentials
// to the windows credential store.
inline HRESULT WINAPI WebAuthNPluginGetAuthenticatorState(
    _In_ REFCLSID rclsid,
    _Out_ AUTHENTICATOR_STATE* pluginAuthenticatorState)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginGetAuthenticatorState);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid, pluginAuthenticatorState);
}

// Registers a callback to handle plugin authenticator state change events for a plugin.
inline HRESULT WINAPI WebAuthNPluginRegisterStatusChangeCallback(
    _In_ WEBAUTHN_PLUGIN_STATUS_CHANGE_CALLBACK callback,
    _In_ void* context,
    _In_ REFCLSID rclsid,
    _Out_ DWORD* pdwRegister)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginRegisterStatusChangeCallback);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(callback, context, rclsid, pdwRegister);
}

// Unregisters a callback that was registered with WebAuthNPluginRegisterStatusChangeCallback.
inline HRESULT WINAPI WebAuthNPluginUnregisterStatusChangeCallback(
    _In_ DWORD* pdwRegister)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginUnregisterStatusChangeCallback);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(pdwRegister);
}

// This Plugin Management API retrieves the operation signing public key used by the platform to sign operation requests.
// Third-party passkey authenticator plugins use this key to verify that operations originated from the trusted Windows platform.
// Part of the security framework for Windows plugins.
inline HRESULT WINAPI WebAuthNPluginGetOperationSigningPublicKey(
    _In_ REFCLSID rclsid,
    _Out_ DWORD* pcbOpSignPubKey,
    _Outptr_result_buffer_maybenull_(*pcbOpSignPubKey) PBYTE* ppbOpSignPubKey) // Free using WebAuthNPluginFreePublicKeyResponse
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginGetOperationSigningPublicKey);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid, pcbOpSignPubKey, ppbOpSignPubKey);
}

// This cleanup API frees memory allocated by the platform for operation signing public key responses.
// Essential for proper resource management in plugin implementations.
inline void WINAPI WebAuthNPluginFreePublicKeyResponse(
    _In_opt_ PBYTE pbOpSignPubKey)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginFreePublicKeyResponse);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(pbOpSignPubKey);
}

// This Windows Hello User Verification API retrieves the current count of user verification attempts.
// Enables plugins to track and potentially limit authentication attempts as part of security policy enforcement.
inline HRESULT WINAPI WebAuthNPluginGetUserVerificationCount(
    _In_ REFCLSID rclsid,
    _Out_ DWORD* pdwVerificationCount)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginGetUserVerificationCount);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid, pdwVerificationCount);
}

// This Windows Hello User Verification API retrieves the public key used for user verification signatures.
// Enables plugins to validate that user verification responses came from the trusted Windows Hello subsystem.
inline HRESULT WINAPI WebAuthNPluginGetUserVerificationPublicKey(
    _In_ REFCLSID rclsid,
    _Out_ DWORD* pcbPublicKey,
    _Outptr_result_bytebuffer_(*pcbPublicKey) PBYTE* ppbPublicKey) // Free using WebAuthNPluginFreePublicKeyResponse
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginGetUserVerificationPublicKey);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid, pcbPublicKey, ppbPublicKey);
}

// This Windows Hello User Verification API performs actual user authentication using Windows Hello biometric capabilities.
// Provides seamless integration with platform security features while maintaining plugin flexibility.
inline HRESULT WINAPI WebAuthNPluginPerformUserVerification(
    _In_ PCWEBAUTHN_PLUGIN_USER_VERIFICATION_REQUEST pPluginUserVerification,
    _Out_ DWORD* pcbResponse,
    _Outptr_result_bytebuffer_maybenull_(*pcbResponse) PBYTE* ppbResponse)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginPerformUserVerification);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(pPluginUserVerification, pcbResponse, ppbResponse);
}

inline void WINAPI WebAuthNPluginFreeUserVerificationResponse(
    _In_opt_ PBYTE ppbResponse)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginFreeUserVerificationResponse);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(ppbResponse);
}

// This decoding API converts CBOR-encoded CTAP make credential requests into structured format.
// Enables plugins to easily process operation parameters without implementing custom CBOR parsing logic.
inline _Success_(return == S_OK) HRESULT WINAPI WebAuthNDecodeMakeCredentialRequest(
    _In_ DWORD cbEncoded,
    _In_reads_bytes_(cbEncoded) const BYTE* pbEncoded,
    _Outptr_ PWEBAUTHN_CTAPCBOR_MAKE_CREDENTIAL_REQUEST* ppMakeCredentialRequest)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNDecodeMakeCredentialRequest);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(cbEncoded, pbEncoded, ppMakeCredentialRequest);
}

inline void WINAPI WebAuthNFreeDecodedMakeCredentialRequest(
    _In_opt_ PWEBAUTHN_CTAPCBOR_MAKE_CREDENTIAL_REQUEST pMakeCredentialRequest)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNFreeDecodedMakeCredentialRequest);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(pMakeCredentialRequest);
}

// This encoding API converts structured attestation responses into CBOR-encoded CTAP format.
// Ensures plugins return properly formatted responses that comply with platform expectations.
inline _Success_(return == S_OK) HRESULT WINAPI WebAuthNEncodeMakeCredentialResponse(
    _In_ PCWEBAUTHN_CREDENTIAL_ATTESTATION pCredentialAttestation,
    _Out_ DWORD* pcbResp,
    _Outptr_result_buffer_maybenull_(*pcbResp) BYTE** ppbResp)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNEncodeMakeCredentialResponse);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(pCredentialAttestation, pcbResp, ppbResp);
}

// This decoding API converts CBOR-encoded CTAP get assertion requests into structured format.
// Enables plugins to process authentication operations without implementing custom CBOR parsing logic.
inline _Success_(return == S_OK) HRESULT WINAPI WebAuthNDecodeGetAssertionRequest(
    _In_ DWORD cbEncoded,
    _In_reads_bytes_(cbEncoded) const BYTE* pbEncoded,
    _Outptr_ PWEBAUTHN_CTAPCBOR_GET_ASSERTION_REQUEST* ppGetAssertionRequest)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNDecodeGetAssertionRequest);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(cbEncoded, pbEncoded, ppGetAssertionRequest);
}

inline void WINAPI WebAuthNFreeDecodedGetAssertionRequest(
    _In_opt_ PWEBAUTHN_CTAPCBOR_GET_ASSERTION_REQUEST pGetAssertionRequest)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNFreeDecodedGetAssertionRequest);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(pGetAssertionRequest);
}

// This encoding API converts structured get assertion responses into CBOR format.
// Ensures plugins return properly formatted CTAP responses that comply with FIDO Alliance specifications.
// Refer: https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html#message-encoding.
inline _Success_(return == S_OK) HRESULT WINAPI WebAuthNEncodeGetAssertionResponse(
    _In_ PCWEBAUTHN_CTAPCBOR_GET_ASSERTION_RESPONSE pGetAssertionResponse,
    _Out_ DWORD* pcbResp,
    _Outptr_result_buffer_maybenull_(*pcbResp) BYTE** ppbResp)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNEncodeGetAssertionResponse);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(pGetAssertionResponse, pcbResp, ppbResp);
}

//
// Credential Metadata Management APIs for third-party passkey authenticator plugins on Windows
//

// This Credential Metadata Management API allows plugin authenticators to add credential metadata to the platform database,
// enabling WebAuthN clients to retrieve a combined list of credentials across all authenticators on the platform.
// Note: Credentials must be added to the platform database before they can be used for WebAuthn operations.
inline HRESULT WINAPI WebAuthNPluginAuthenticatorAddCredentials(
    _In_ REFCLSID rclsid,
    _In_ DWORD cCredentialDetails,
    _In_reads_(cCredentialDetails) PCWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS pCredentialDetails)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginAuthenticatorAddCredentials);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid, cCredentialDetails, pCredentialDetails);
}

// This Credential Metadata Management API allows plugin authenticators to remove all their credential
// metadata from the platform database, effectively clearing the shared credential list for this plugin.
inline HRESULT WINAPI WebAuthNPluginAuthenticatorRemoveAllCredentials(
    _In_ REFCLSID rclsid)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginAuthenticatorRemoveAllCredentials);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid);
}

// This Credential Metadata Management API allows plugin authenticators to selectively remove specific
// credentials from the platform database.
inline HRESULT WINAPI WebAuthNPluginAuthenticatorRemoveCredentials(
    _In_ REFCLSID rclsid,
    _In_ DWORD cCredentialDetails,
    _In_reads_(cCredentialDetails) PCWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS pCredentialDetails)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginAuthenticatorRemoveCredentials);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid, cCredentialDetails, pCredentialDetails);
}

// This Credential Metadata Management API retrieves all credential metadata currently stored in the platform database
// for this plugin authenticator, enabling autofill scenarios and credential discovery.
inline HRESULT WINAPI WebAuthNPluginAuthenticatorGetAllCredentials(
    _In_ REFCLSID rclsid,
    _Out_ DWORD* pcCredentialDetails,
    _Outptr_result_buffer_maybenull_(*pcCredentialDetails) PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS* ppCredentialDetailsArray)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginAuthenticatorGetAllCredentials);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(rclsid, pcCredentialDetails, ppCredentialDetailsArray);
}

// This cleanup API frees the credential details array allocated by the platform
inline void WINAPI WebAuthNPluginAuthenticatorFreeCredentialDetailsArray(
    _In_ DWORD cCredentialDetails,
    _In_reads_(cCredentialDetails) PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS pCredentialDetailsArray)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNPluginAuthenticatorFreeCredentialDetailsArray);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(cCredentialDetails, pCredentialDetailsArray);
}

//
// webauthn.h API wrappers
//

inline HRESULT WINAPI WebAuthNGetPlatformCredentialList(
    _In_    PCWEBAUTHN_GET_CREDENTIALS_OPTIONS                  pGetCredentialsOptions,
    _Outptr_result_maybenull_ PWEBAUTHN_CREDENTIAL_DETAILS_LIST *ppCredentialDetailsList)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNGetPlatformCredentialList);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(pGetCredentialsOptions, ppCredentialDetailsList);
}

inline void WINAPI WebAuthNFreePlatformCredentialList(
    _In_ PWEBAUTHN_CREDENTIAL_DETAILS_LIST  pCredentialDetailsList)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNFreePlatformCredentialList);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(pCredentialDetailsList);
}

inline HRESULT WINAPI WebAuthNAuthenticatorMakeCredential(
    _In_        HWND                                                hWnd,
    _In_        PCWEBAUTHN_RP_ENTITY_INFORMATION                    pRpInformation,
    _In_        PCWEBAUTHN_USER_ENTITY_INFORMATION                  pUserInformation,
    _In_        PCWEBAUTHN_COSE_CREDENTIAL_PARAMETERS               pPubKeyCredParams,
    _In_        PCWEBAUTHN_CLIENT_DATA                              pWebAuthNClientData,
    _In_opt_    PCWEBAUTHN_AUTHENTICATOR_MAKE_CREDENTIAL_OPTIONS    pWebAuthNMakeCredentialOptions,
    _Outptr_result_maybenull_ PWEBAUTHN_CREDENTIAL_ATTESTATION      *ppWebAuthNCredentialAttestation)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNAuthenticatorMakeCredential);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(hWnd, pRpInformation, pUserInformation, pPubKeyCredParams, pWebAuthNClientData, pWebAuthNMakeCredentialOptions, ppWebAuthNCredentialAttestation);
}

inline void WINAPI WebAuthNFreeCredentialAttestation(
    _In_opt_ PWEBAUTHN_CREDENTIAL_ATTESTATION pWebAuthNCredentialAttestation)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNFreeCredentialAttestation);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(pWebAuthNCredentialAttestation);
}

inline HRESULT WINAPI WebAuthNAuthenticatorGetAssertion(
    _In_        HWND                                                hWnd,
    _In_        LPCWSTR                                             pwszRpId,
    _In_        PCWEBAUTHN_CLIENT_DATA                              pWebAuthNClientData,
    _In_opt_    PCWEBAUTHN_AUTHENTICATOR_GET_ASSERTION_OPTIONS      pWebAuthNGetAssertionOptions,
    _Outptr_result_maybenull_ PWEBAUTHN_ASSERTION                   *ppWebAuthNAssertion)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNAuthenticatorGetAssertion);
    RETURN_HR_IF_NULL(E_NOTIMPL, s_fn);
    return s_fn(hWnd, pwszRpId, pWebAuthNClientData, pWebAuthNGetAssertionOptions, ppWebAuthNAssertion);
}

inline void WINAPI WebAuthNFreeAssertion(
    _In_ PWEBAUTHN_ASSERTION pWebAuthNAssertion)
{
    static auto s_fn = GetProcAddressByFunctionDeclaration(GetWebAuthnDll(), WebAuthNFreeAssertion);
    THROW_HR_IF_NULL(E_NOTIMPL, s_fn);
    s_fn(pWebAuthNAssertion);
}
