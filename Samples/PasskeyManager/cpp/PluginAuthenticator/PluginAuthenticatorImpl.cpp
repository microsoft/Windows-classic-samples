#include "pch.h"
#include "PluginAuthenticatorImpl.h"
#include <App.xaml.h>
#include <PluginManagement/PluginRegistrationManager.h>
#include <PluginManagement/PluginCredentialManager.h>
#include <include/cbor-lite/codec.h>
#include <string>
#include <iostream>
#include <fstream>
#include <helpers/buffer_read_write.h>
#include <wil/result.h>
#include <wil/resource.h>
#include <algorithm>
#include <memory>

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Microsoft::UI::Windowing;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Navigation;
    using namespace PasskeyManager;
    using namespace PasskeyManager::implementation;
    using namespace CborLite;
}

namespace winrt::PasskeyManager::implementation
{
    namespace {
        // Helper function to get request signing public key with proper error handling
        std::vector<uint8_t> GetRequestSigningPubKey()
        {
            auto curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();

            DWORD cbKeyData = 0;
            unique_plugin_public_key pbKeyData = nullptr;
            HRESULT hr = WebAuthNPluginGetOperationSigningPublicKey(
                contosoplugin_guid,
                &cbKeyData,
                &pbKeyData);

            if (SUCCEEDED(hr) && pbKeyData && cbKeyData > 0)
            {
                std::vector<BYTE> response(pbKeyData.get(), pbKeyData.get() + cbKeyData);
                return response;
            }

            return {};
        }

        /*
        * This function is used to verify the signature of a request buffer.
        * The public key is part of response to plugin registration.
        */
        HRESULT VerifySignatureHelper(
            std::span<const BYTE> dataBuffer,
            PBYTE pbKeyData,
            DWORD cbKeyData,
            PBYTE pbSignature,
            DWORD cbSignature)
        {
            // Create key provider
            wil::unique_ncrypt_prov hProvider;
            wil::unique_ncrypt_key reqSigningKey;

            // Get the provider
            RETURN_IF_FAILED(NCryptOpenStorageProvider(&hProvider, nullptr, 0));
            
            // Create a NCrypt key handle from the public key
            RETURN_IF_FAILED(NCryptImportKey(
                hProvider.get(),
                NULL,
                BCRYPT_PUBLIC_KEY_BLOB,
                nullptr,
                &reqSigningKey,
                pbKeyData,
                cbKeyData,
                0));

            // Verify the signature over the hash of dataBuffer using the hKey
            DWORD objLenSize = 0;
            DWORD bytesRead = 0;
            RETURN_IF_NTSTATUS_FAILED(BCryptGetProperty(
                BCRYPT_SHA256_ALG_HANDLE,
                BCRYPT_OBJECT_LENGTH,
                reinterpret_cast<PBYTE>(&objLenSize),
                sizeof(objLenSize),
                &bytesRead, 
                0));

            auto objLen = wil::make_unique_cotaskmem<BYTE[]>(objLenSize);
            RETURN_HR_IF_NULL(E_OUTOFMEMORY, objLen);

            wil::unique_bcrypt_hash hashHandle;
            RETURN_IF_NTSTATUS_FAILED(BCryptCreateHash(
                BCRYPT_SHA256_ALG_HANDLE,
                wil::out_param(hashHandle),
                objLen.get(),
                objLenSize,
                nullptr, 
                0, 
                0));

            RETURN_IF_NTSTATUS_FAILED(BCryptHashData(
                hashHandle.get(),
                const_cast<PUCHAR>(dataBuffer.data()),
                static_cast<ULONG>(dataBuffer.size()), 
                0));

            DWORD localHashByteCount = 0;
            RETURN_IF_NTSTATUS_FAILED(BCryptGetProperty(
                BCRYPT_SHA256_ALG_HANDLE,
                BCRYPT_HASH_LENGTH,
                reinterpret_cast<PBYTE>(&localHashByteCount),
                sizeof(localHashByteCount),
                &bytesRead, 
                0));

            auto localHashBuffer = wil::make_unique_cotaskmem<BYTE[]>(localHashByteCount);
            RETURN_HR_IF_NULL(E_OUTOFMEMORY, localHashBuffer);

            RETURN_IF_NTSTATUS_FAILED(BCryptFinishHash(
                hashHandle.get(), 
                localHashBuffer.get(), 
                localHashByteCount, 
                0));

            PVOID paddingInfo = nullptr;
            DWORD dwCngFlags = 0;
            RETURN_HR_IF(E_INVALIDARG, cbKeyData < sizeof(BCRYPT_KEY_BLOB));
            
            BCRYPT_KEY_BLOB* pKeyBlob = reinterpret_cast<BCRYPT_KEY_BLOB*>(pbKeyData);
            if (pKeyBlob->Magic == BCRYPT_RSAPUBLIC_MAGIC)
            {
                BCRYPT_PKCS1_PADDING_INFO paddingInfoStruct = {};
                paddingInfoStruct.pszAlgId = BCRYPT_SHA256_ALGORITHM;
                paddingInfo = &paddingInfoStruct;
                dwCngFlags = BCRYPT_PAD_PKCS1;
            }

            RETURN_IF_WIN32_ERROR(NCryptVerifySignature(
                reqSigningKey.get(),
                paddingInfo,
                localHashBuffer.get(),
                localHashByteCount,
                pbSignature,
                cbSignature,
                dwCngFlags));

            return S_OK;
        }

        HRESULT WaitAndCheckVaultUnlockCompleted()
        {
            try
            {
                winrt::com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();

                HANDLE handles[2] = { 
                    curApp->m_hVaultConsentComplete.get(), 
                    curApp->m_hVaultConsentFailed.get() 
                };

                DWORD cWait = ARRAYSIZE(handles);
                DWORD hIndex = 0;
                THROW_IF_FAILED(CoWaitForMultipleHandles(
                    COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, 
                    INFINITE, 
                    cWait, 
                    handles, 
                    &hIndex));

                if (hIndex == 1) // Consent failed
                {
                    return E_FAIL;
                }
                return S_OK;
            }
            catch (...)
            {
                return winrt::to_hresult();
            }
        }
    } // anonymous namespace

    HRESULT ContosoPlugin::PerformUserVerification(
        HWND hWnd,
        GUID transactionId,
        PluginOperationType operationType,
        const std::vector<BYTE>& requestBuffer,
        wil::shared_cotaskmem_string rpName,
        wil::shared_cotaskmem_string userName)
    {
        RETURN_HR_IF(E_INVALIDARG, requestBuffer.empty());
        winrt::com_ptr<winrt::PasskeyManager::implementation::App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<winrt::PasskeyManager::implementation::App>();

        try
        {
            RETURN_HR_IF(E_UNEXPECTED, !curApp->GetDispatcherQueue().TryEnqueue([curApp, hWnd, operationType, rpName, userName]()
            {
                curApp->SetPluginPerformOperationOptions(hWnd, operationType, rpName.get(), userName.get());
            }));

            // Trigger a Consent Verifier Dialog to simulate a Windows Hello unlock flow
            // This is to demonstrate a vault unlock flow using Windows Hello and is not the recommended way to secure the vault
            if (PluginCredentialManager::getInstance().GetVaultLock())
            {
                RETURN_HR_IF(E_UNEXPECTED, !curApp->GetDispatcherQueue().TryEnqueue([curApp]()
                {
                    curApp->SimulateUnLockVault();
                }));
                RETURN_IF_FAILED(WaitAndCheckVaultUnlockCompleted());
            }
            else
            {
                SetEvent(curApp->m_hVaultConsentComplete.get());
            }

            // Wait for user confirmation to proceed with the operation Create/Signin/Cancel button
            // This is a mock up for plugin requiring UI.
            {
                HANDLE handles[2] = { 
                    curApp->m_hPluginProceedButtonEvent.get(), 
                    m_hPluginCancelOperationEvent.get() 
                };
                DWORD cWait = ARRAYSIZE(handles);
                DWORD hIndex = 0;

                RETURN_IF_FAILED(CoWaitForMultipleHandles(
                    COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, 
                    INFINITE, 
                    cWait, 
                    handles, 
                    &hIndex));

                if (hIndex == 1) // Cancel button clicked
                {
                    // User cancelled the operation. NTE_USER_CANCELLED allows Windows to distinguish between user cancellation and other errors.
                    return NTE_USER_CANCELLED;
                }
            }

            // Skip user verification if the user has already performed a gesture to unlock the vault to avoid double prompting
            if (PluginCredentialManager::getInstance().GetVaultLock())
            {
                return S_OK;
            }

            // Optional Step: Get the UV count. The UV count tracks the number of times the user has performed a gesture to unlock the vault.
            DWORD uvCount = 0;
            RETURN_IF_FAILED(WebAuthNPluginGetUserVerificationCount(contosoplugin_guid, &uvCount));

            // Step 1: Get the public key.
            DWORD cbPubKeyData = 0;
            unique_plugin_public_key pbPubKeyData = nullptr;
            RETURN_IF_FAILED(WebAuthNPluginGetUserVerificationPublicKey(
                contosoplugin_guid,
                &cbPubKeyData,
                &pbPubKeyData));
            RETURN_HR_IF_NULL(E_FAIL, pbPubKeyData);

            // Step 2: Perform UV. This step uses a Windows Hello prompt to authenticate the user.
            // WEBAUTHN_PLUGIN_USER_VERIFICATION_REQUEST: This structure defines the request parameters for Windows Hello user verification.
            // Enables plugins to leverage familiar Windows Hello biometric authentication for user verification workflows.
            WEBAUTHN_PLUGIN_USER_VERIFICATION_REQUEST pluginPerformUv = {
                nullptr,      // hwnd
                transactionId, // rguidTransactionId
                nullptr,      // pwszUsername
                nullptr       // pwszDisplayHint
            };

            if (curApp->GetSilentMode())
            {
                // If the app did not display any UI, use the hwnd of the caller here. This was included in the request to the plugin. 
                // Refer: PCWEBAUTHN_PLUGIN_OPERATION_REQUEST - structure containing operation parameters
                pluginPerformUv.hwnd = hWnd;
            }
            else
            {
                // If the app displayed UI, use the hwnd of the app window here
                pluginPerformUv.hwnd = curApp->GetNativeWindowHandle();
            }

            auto localUserName = wil::make_cotaskmem_string(userName.get());
            pluginPerformUv.pwszUsername = localUserName.get();

            // pwszDisplayHint can be used to provide additional context to the user.
            // This is displayed alongside the username in the Windows Hello passkey user verification dialog.
            auto localDisplayHint = wil::make_cotaskmem_string(L"Context String");
            pluginPerformUv.pwszDisplayHint = localDisplayHint.get();

            DWORD cbResponse = 0;
            PBYTE pbResponse = nullptr;

            RETURN_IF_FAILED(WebAuthNPluginPerformUserVerification(&pluginPerformUv, &cbResponse, &pbResponse));
            auto cleanupUvResponse = wil::scope_exit([&] {
                WebAuthNPluginFreeUserVerificationResponse(pbResponse);
            });

            // Verify the signature over the hash of requestBuffer using the hKey
            auto signatureVerifyResult = VerifySignatureHelper(
                requestBuffer,
                pbPubKeyData.get(),
                cbPubKeyData,
                pbResponse,
                cbResponse);

            RETURN_HR_IF(E_UNEXPECTED, !curApp->GetDispatcherQueue().TryEnqueue([curApp, signatureVerifyResult]()
            {
                if (FAILED(signatureVerifyResult))
                {
                    curApp->m_pluginOperationStatus.uvSignatureVerificationStatus = signatureVerifyResult;
                }
            }));

            return S_OK;
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    /*
    * This function is used to create a simplified version of authenticator data for the webauthn authenticator operations.
    * Refer: https://www.w3.org/TR/webauthn-3/#authenticator-data for more details.
    */
    HRESULT CreateAuthenticatorData(
        const NCRYPT_KEY_HANDLE hKey,
        const PluginOperationType operationType,
        DWORD cbRpId,
        PBYTE pbRpId,
        DWORD& pcbPackedAuthenticatorData,
        wil::unique_hlocal_ptr<BYTE[]>& ppbpackedAuthenticatorData,
        std::vector<uint8_t>& vCredentialIdBuffer)
    {
        try
        {
            // Get the public key blob
            DWORD cbPubKeyBlob = 0;
            THROW_IF_FAILED(NCryptExportKey(
                hKey,
                NULL,
                BCRYPT_ECCPUBLIC_BLOB,
                nullptr,
                nullptr,
                0,
                &cbPubKeyBlob,
                0));

            auto pbPubKeyBlob = std::make_unique<BYTE[]>(cbPubKeyBlob);

            DWORD cbPubKeyBlobOutput = 0;
            THROW_IF_FAILED(NCryptExportKey(
                hKey,
                NULL,
                BCRYPT_ECCPUBLIC_BLOB,
                nullptr,
                pbPubKeyBlob.get(),
                cbPubKeyBlob,
                &cbPubKeyBlobOutput,
                0));

            BCRYPT_ECCKEY_BLOB* pPubKeyBlobHeader = reinterpret_cast<BCRYPT_ECCKEY_BLOB*>(pbPubKeyBlob.get());
            DWORD cbXCoord = pPubKeyBlobHeader->cbKey;
            PBYTE pbXCoord = reinterpret_cast<PBYTE>(&pPubKeyBlobHeader[1]);
            DWORD cbYCoord = pPubKeyBlobHeader->cbKey;
            PBYTE pbYCoord = pbXCoord + cbXCoord;

            // create byte span for x and y
            std::span<const BYTE> xCoord(pbXCoord, cbXCoord);
            std::span<const BYTE> yCoord(pbYCoord, cbYCoord);

            // CBOR encode the public key in this order: kty, alg, crv, x, y
            std::vector<BYTE> buffer;

#pragma warning(push)
#pragma warning(disable: 4293)
            size_t bufferSize = CborLite::encodeMapSize(buffer, 5u);
#pragma warning(pop)

            // COSE CBOR encoding format. Refer to https://datatracker.ietf.org/doc/html/rfc9052#section-7 for more details.
            constexpr int8_t ktyIndex = 1;
            constexpr int8_t algIndex = 3;
            constexpr int8_t crvIndex = -1;
            constexpr int8_t xIndex = -2;
            constexpr int8_t yIndex = -3;

            // Example values for EC2 P-256 ES256 Keys. Refer to https://www.w3.org/TR/webauthn-3/#example-bdbd14cc
            // Note that this sample authenticator only supports ES256 keys.
            constexpr int8_t kty = 2; // Key type is EC2
            constexpr int8_t crv = 1; // Curve is P-256
            constexpr int8_t alg = -7; // Algorithm is ES256

            bufferSize += CborLite::encodeInteger(buffer, ktyIndex);
            bufferSize += CborLite::encodeInteger(buffer, kty);
            bufferSize += CborLite::encodeInteger(buffer, algIndex);
            bufferSize += CborLite::encodeInteger(buffer, alg);
            bufferSize += CborLite::encodeInteger(buffer, crvIndex);
            bufferSize += CborLite::encodeInteger(buffer, crv);
            bufferSize += CborLite::encodeInteger(buffer, xIndex);
            bufferSize += CborLite::encodeBytes(buffer, xCoord);
            bufferSize += CborLite::encodeInteger(buffer, yIndex);
            bufferSize += CborLite::encodeBytes(buffer, yCoord);

            wil::unique_bcrypt_hash hashHandle;
            THROW_IF_NTSTATUS_FAILED(BCryptCreateHash(
                BCRYPT_SHA256_ALG_HANDLE,
                &hashHandle,
                nullptr,
                0,
                nullptr,
                0,
                0));

            THROW_IF_NTSTATUS_FAILED(BCryptHashData(
                hashHandle.get(), 
                reinterpret_cast<PUCHAR>(pbXCoord), 
                cbXCoord, 
                0));

            THROW_IF_NTSTATUS_FAILED(BCryptHashData(
                hashHandle.get(), 
                reinterpret_cast<PUCHAR>(pbYCoord), 
                cbYCoord, 
                0));

            DWORD cbHash = 0;
            DWORD bytesRead = 0;
            THROW_IF_NTSTATUS_FAILED(BCryptGetProperty(
                hashHandle.get(),
                BCRYPT_HASH_LENGTH,
                reinterpret_cast<PBYTE>(&cbHash),
                sizeof(cbHash),
                &bytesRead,
                0));

            wil::unique_hlocal_ptr<BYTE[]> pbCredentialId = wil::make_unique_hlocal<BYTE[]>(cbHash);

            THROW_IF_NTSTATUS_FAILED(BCryptFinishHash(
                hashHandle.get(), 
                pbCredentialId.get(), 
                cbHash, 
                0));

            // Refer to learn about packing credential data https://www.w3.org/TR/webauthn-3/#sctn-authenticator-data
            constexpr DWORD rpidsha256Size = 32; // SHA256 hash of rpId
            constexpr DWORD flagsSize = 1; // flags
            constexpr DWORD signCountSize = 4; // signCount
            DWORD cbPackedAuthenticatorData = rpidsha256Size + flagsSize + signCountSize;

            if (operationType == PluginOperationType::MakeCredential)
            {
                cbPackedAuthenticatorData += sizeof(GUID); // aaGuid
                cbPackedAuthenticatorData += sizeof(WORD); // credentialId length
                cbPackedAuthenticatorData += cbHash; // credentialId
                cbPackedAuthenticatorData += static_cast<DWORD>(buffer.size()); // public key
            }

            std::vector<BYTE> vPackedAuthenticatorData(cbPackedAuthenticatorData);
            auto writer = buffer_writer{ vPackedAuthenticatorData };

            auto rgbRpIdHash = writer.reserve_space<std::array<BYTE, rpidsha256Size>>(); // 32 bytes of rpIdHash which is SHA256 hash of rpName. https://www.w3.org/TR/webauthn-3/#sctn-authenticator-data
            DWORD cbRpIdHash = rpidsha256Size;
            THROW_IF_WIN32_BOOL_FALSE(CryptHashCertificate2(
                BCRYPT_SHA256_ALGORITHM,
                0,
                nullptr,
                pbRpId,
                cbRpId,
                rgbRpIdHash->data(),
                &cbRpIdHash));

            // Flags uv, up, be, and at are set
            if (operationType == PluginOperationType::GetAssertion)
            {
                // Refer https://www.w3.org/TR/webauthn-3/#authdata-flags
                *writer.reserve_space<uint8_t>() = 0x1d; // credential data flags of size 1 byte

                *writer.reserve_space<uint32_t>() = 0u; // Sign count of size 4 bytes is set to 0

                vCredentialIdBuffer.assign(pbCredentialId.get(), pbCredentialId.get() + cbHash);
            }
            else
            {
                // Refer https://www.w3.org/TR/webauthn-3/#authdata-flags
                *writer.reserve_space<uint8_t>() = 0x5d; // credential data flags of size 1 byte

                *writer.reserve_space<uint32_t>() = 0u; // Sign count of size 4 bytes is set to 0

                // aaGuid of size 16 bytes is set to predefined bytes in big-endian. Refer https://www.w3.org/TR/webauthn-3/#aaguid
                writer.add(std::span<const BYTE>(c_pluginAaguidBytes, sizeof(c_pluginAaguidBytes)));

                // Retrieve credential id
                WORD cbCredentialId = static_cast<WORD>(cbHash);
                WORD cbCredentialIdBigEndian = _byteswap_ushort(cbCredentialId);

                *writer.reserve_space<WORD>() = cbCredentialIdBigEndian; // Size of credential id in unsigned big endian of size 2 bytes

                writer.add(std::span<BYTE>(pbCredentialId.get(), cbHash)); // Set credential id

                vCredentialIdBuffer.assign(pbCredentialId.get(), pbCredentialId.get() + cbHash);

                writer.add(std::span<BYTE>(buffer.data(), buffer.size())); // Set CBOR encoded public key
            }

            pcbPackedAuthenticatorData = static_cast<DWORD>(vPackedAuthenticatorData.size());
            ppbpackedAuthenticatorData = wil::make_unique_hlocal<BYTE[]>(pcbPackedAuthenticatorData);

            memcpy_s(ppbpackedAuthenticatorData.get(), pcbPackedAuthenticatorData, vPackedAuthenticatorData.data(), pcbPackedAuthenticatorData);

            return S_OK;
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    /*
    * This function is invoked by the platform to request the plugin to handle a make credential operation.
    * Refer: pluginauthenticator.h/pluginauthenticator.idl
    */
    HRESULT STDMETHODCALLTYPE ContosoPlugin::MakeCredential(
        /* [in] */ __RPC__in PCWEBAUTHN_PLUGIN_OPERATION_REQUEST pPluginMakeCredentialRequest,
        /* [out] */ __RPC__out PWEBAUTHN_PLUGIN_OPERATION_RESPONSE response) noexcept
    {
        HRESULT hr = S_OK;
        try
        {
            RETURN_HR_IF_NULL(E_INVALIDARG, response);
            *response = {};
            RETURN_HR_IF_NULL(E_INVALIDARG, pPluginMakeCredentialRequest);

            DWORD hIndex = 0;
            THROW_IF_FAILED(CoWaitForMultipleHandles( // wait for app to be ready
                COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS,
                INFINITE,
                1,
                m_hAppReadyForPluginOpEvent.addressof(),
                &hIndex));

            com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();

            // Atomically check if an operation is already in progress.
            bool expected = false;
            if (!curApp->m_isOperationInProgress.compare_exchange_strong(expected, true))
            {
                return HRESULT_FROM_WIN32(ERROR_BUSY); // Another operation is running.
            }
            // Ensure the flag is cleared when the function exits, for any reason.
            auto clearOperationInProgress = wil::scope_exit([&]
            {
                curApp->m_isOperationInProgress = false;
            });

            curApp->SetPluginTransactionId(pPluginMakeCredentialRequest->transactionId);
            auto completePluginOperation = wil::SetEvent_scope_exit(m_hPluginOpCompletedEvent.get());

            // WEBAUTHN_CTAPCBOR_MAKE_CREDENTIAL_REQUEST: This structure represents the decoded CTAP make credential request.
            // Provides structured access to CBOR-encoded operation parameters for third-party plugin implementations.
            PWEBAUTHN_CTAPCBOR_MAKE_CREDENTIAL_REQUEST pDecodedMakeCredentialRequest;

            THROW_IF_FAILED(WebAuthNDecodeMakeCredentialRequest(
                pPluginMakeCredentialRequest->cbEncodedRequest,
                pPluginMakeCredentialRequest->pbEncodedRequest,
                &pDecodedMakeCredentialRequest));

            auto cleanup = wil::scope_exit([&] {
                WebAuthNFreeDecodedMakeCredentialRequest(pDecodedMakeCredentialRequest);
            });

            THROW_HR_IF(NTE_NOT_SUPPORTED, wcscmp(pDecodedMakeCredentialRequest->pRpInformation->pwszName, c_pluginRpId) == 0);
            auto rpName = wil::make_cotaskmem_string(pDecodedMakeCredentialRequest->pRpInformation->pwszName);

            auto userName = wil::make_cotaskmem_string(pDecodedMakeCredentialRequest->pUserInformation->pwszName);
            std::vector<BYTE> requestBuffer(
                pPluginMakeCredentialRequest->pbEncodedRequest,
                pPluginMakeCredentialRequest->pbEncodedRequest + pPluginMakeCredentialRequest->cbEncodedRequest);

            auto pubKeyData = GetRequestSigningPubKey();
            HRESULT requestSignResult = E_FAIL;
            if (!pubKeyData.empty())
            {
                requestSignResult = VerifySignatureHelper(
                    requestBuffer,
                    pubKeyData.data(),
                    static_cast<DWORD>(pubKeyData.size()),
                    pPluginMakeCredentialRequest->pbRequestSignature,
                    pPluginMakeCredentialRequest->cbRequestSignature);
            }

            {
                std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                curApp->m_pluginOperationStatus.requestSignatureVerificationStatus = requestSignResult;
            }

            hr = PerformUserVerification(
                pPluginMakeCredentialRequest->hWnd,
                pPluginMakeCredentialRequest->transactionId,
                PluginOperationType::MakeCredential,
                requestBuffer,
                std::move(rpName),
                std::move(userName));
            THROW_IF_FAILED(hr);

            // create a persisted key using ncrypt
            wil::unique_ncrypt_prov hProvider;
            wil::unique_ncrypt_key hKey;

            // get the provider
            THROW_IF_FAILED(NCryptOpenStorageProvider(&hProvider, nullptr, 0));

            // get the user handle as a string
            std::wstring keyNameStr = contosoplugin_key_domain;
            std::wstringstream keyNameStream;
            for (DWORD idx = 0; idx < pDecodedMakeCredentialRequest->pUserInformation->cbId; idx++)
            {
                keyNameStream << std::hex << std::setw(2) << std::setfill(L'0') <<
                    static_cast<int>(pDecodedMakeCredentialRequest->pUserInformation->pbId[idx]);
            }
            keyNameStr += keyNameStream.str();

            // create the key
            THROW_IF_FAILED(NCryptCreatePersistedKey(
                hProvider.get(),
                &hKey,
                BCRYPT_ECDH_P256_ALGORITHM,
                keyNameStr.c_str(),
                0,
                NCRYPT_OVERWRITE_KEY_FLAG));

            // set the export policy
            DWORD exportPolicy = NCRYPT_ALLOW_PLAINTEXT_EXPORT_FLAG;
            THROW_IF_FAILED(NCryptSetProperty(
                hKey.get(),
                NCRYPT_EXPORT_POLICY_PROPERTY,
                reinterpret_cast<PBYTE>(&exportPolicy),
                sizeof(exportPolicy),
                NCRYPT_PERSIST_FLAG));

            // allow both signing and encryption
            DWORD keyUsage = NCRYPT_ALLOW_SIGNING_FLAG | NCRYPT_ALLOW_DECRYPT_FLAG;
            THROW_IF_FAILED(NCryptSetProperty(
                hKey.get(),
                NCRYPT_KEY_USAGE_PROPERTY,
                reinterpret_cast<PBYTE>(&keyUsage),
                sizeof(keyUsage),
                NCRYPT_PERSIST_FLAG));

            HWND hWnd;
            if (curApp->GetSilentMode())
            {
                hWnd = curApp->m_pluginOperationOptions.hWnd;
            }
            else
            {
                hWnd = curApp->GetNativeWindowHandle();
            }

            THROW_IF_FAILED(NCryptSetProperty(
                hKey.get(),
                NCRYPT_WINDOW_HANDLE_PROPERTY,
                reinterpret_cast<PBYTE>(&hWnd),
                sizeof(HWND),
                0));

            // finalize the key
            THROW_IF_FAILED(NCryptFinalizeKey(hKey.get(), 0));

            DWORD cbPackedAuthenticatorData = 0;
            wil::unique_hlocal_ptr<BYTE[]> packedAuthenticatorData;
            std::vector<uint8_t> vCredentialIdBuffer;
            THROW_IF_FAILED(CreateAuthenticatorData(
                hKey.get(),
                PluginOperationType::MakeCredential,
                pDecodedMakeCredentialRequest->cbRpId,
                pDecodedMakeCredentialRequest->pbRpId,
                cbPackedAuthenticatorData,
                packedAuthenticatorData,
                vCredentialIdBuffer));

            WEBAUTHN_CREDENTIAL_ATTESTATION attestationResponse = {};
            attestationResponse.dwVersion = WEBAUTHN_CREDENTIAL_ATTESTATION_CURRENT_VERSION;
            attestationResponse.pwszFormatType = WEBAUTHN_ATTESTATION_TYPE_NONE;
            attestationResponse.cbAttestation = 0;
            attestationResponse.pbAttestation = nullptr;
            attestationResponse.cbAuthenticatorData = 0;
            attestationResponse.pbAuthenticatorData = nullptr;

            attestationResponse.pbAuthenticatorData = packedAuthenticatorData.get();
            attestationResponse.cbAuthenticatorData = cbPackedAuthenticatorData;

            DWORD cbAttestationBuffer = 0;
            wil::unique_cotaskmem_ptr<BYTE[]> pbAttestationBuffer;

            THROW_IF_FAILED(WebAuthNEncodeMakeCredentialResponse(
                &attestationResponse,
                &cbAttestationBuffer,
                wil::out_param(pbAttestationBuffer)));

            WEBAUTHN_CREDENTIAL_DETAILS credentialDetails = {};
            credentialDetails.dwVersion = WEBAUTHN_CREDENTIAL_DETAILS_CURRENT_VERSION;
            credentialDetails.pUserInformation = const_cast<PWEBAUTHN_USER_ENTITY_INFORMATION>(pDecodedMakeCredentialRequest->pUserInformation);
            credentialDetails.pRpInformation = const_cast<PWEBAUTHN_RP_ENTITY_INFORMATION>(pDecodedMakeCredentialRequest->pRpInformation);
            credentialDetails.cbCredentialID = static_cast<DWORD>(vCredentialIdBuffer.size());
            credentialDetails.pbCredentialID = vCredentialIdBuffer.data();

            auto& credManager = PluginCredentialManager::getInstance();

            if (!credManager.SaveCredentialMetadataToMockDB(credentialDetails))
            {
                std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                curApp->m_pluginOperationStatus.performOperationStatus = E_FAIL;
            }

            // Add newly created passkey to system cache
            credManager.ReloadCredentialManager();
            std::vector<std::vector<UINT8>> credentialIdList{ vCredentialIdBuffer };
            credManager.AddPluginCredentialById(credentialIdList);

            response->cbEncodedResponse = cbAttestationBuffer;
            response->pbEncodedResponse = pbAttestationBuffer.release();
            return S_OK;
        }
        catch (...)
        {
            hr = wil::ResultFromCaughtException();
            try
            {
                com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
                if (curApp)
                {
                    std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                    curApp->m_pluginOperationStatus.performOperationStatus = hr;
                }
            }
            catch (...)
            {
                // Ignore errors during cleanup
            }
            return hr;
        }
    }

    /*
    * This function is invoked by the platform to request the plugin to handle a get assertion operation.
    * Refer: pluginauthenticator.h/pluginauthenticator.idl
    */
    HRESULT STDMETHODCALLTYPE ContosoPlugin::GetAssertion(
        /* [in] */ __RPC__in PCWEBAUTHN_PLUGIN_OPERATION_REQUEST pPluginGetAssertionRequest,
        /* [out] */ __RPC__out PWEBAUTHN_PLUGIN_OPERATION_RESPONSE response) noexcept
    {
        HRESULT hr = S_OK;
        try
        {
            RETURN_HR_IF_NULL(E_INVALIDARG, response);
            *response = {};
            RETURN_HR_IF_NULL(E_INVALIDARG, pPluginGetAssertionRequest);

            DWORD hIndex = 0;
            THROW_IF_FAILED(CoWaitForMultipleHandles(
                COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS,
                INFINITE,
                1,
                m_hAppReadyForPluginOpEvent.addressof(),
                &hIndex));

            com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();

            // Atomically check if an operation is already in progress.
            bool expected = false;
            if (!curApp->m_isOperationInProgress.compare_exchange_strong(expected, true))
            {
                return HRESULT_FROM_WIN32(ERROR_BUSY); // Another operation is running.
            }
            // Ensure the flag is cleared when the function exits, for any reason.
            auto clearOperationInProgress = wil::scope_exit([&]
            {
                curApp->m_isOperationInProgress = false;
            });

            curApp->SetPluginTransactionId(pPluginGetAssertionRequest->transactionId);
            auto completePluginOperation = wil::SetEvent_scope_exit(this->m_hPluginOpCompletedEvent.get());

            // WEBAUTHN_CTAPCBOR_GET_ASSERTION_REQUEST: This structure represents the decoded CTAP get assertion request.
            // Provides structured access to authentication parameters including RP ID, allowed credentials, and client
            // data hash for plugin processing.
            PWEBAUTHN_CTAPCBOR_GET_ASSERTION_REQUEST pDecodedAssertionRequest;

            THROW_IF_FAILED(WebAuthNDecodeGetAssertionRequest(
                pPluginGetAssertionRequest->cbEncodedRequest, 
                pPluginGetAssertionRequest->pbEncodedRequest, 
                &pDecodedAssertionRequest));

            auto cleanup = wil::scope_exit([&] {
                WebAuthNFreeDecodedGetAssertionRequest(pDecodedAssertionRequest);
            });

            wil::shared_cotaskmem_string rpName = wil::make_cotaskmem_string(pDecodedAssertionRequest->pwszRpId);

            //load the user handle
            auto& credManager = PluginCredentialManager::getInstance();
            credManager.ReloadCredentialManager();
            credManager.ReloadRegistryValues();
            const WEBAUTHN_CREDENTIAL_DETAILS* selectedCredential = nullptr;
            // create a list of credentials
            std::vector<const WEBAUTHN_CREDENTIAL_DETAILS*> selectedCredentials;

            // Wait for credentials to be loaded with timeout
            constexpr int MAX_WAIT_ITERATIONS = 100;
            constexpr int WAIT_INTERVAL_MS = 100;
            int waitCount = 0;
            
            while (waitCount < MAX_WAIT_ITERATIONS)
            {
                Sleep(WAIT_INTERVAL_MS);
                if (credManager.IsLocalCredentialMetadataLoaded())
                {
                    credManager.GetLocalCredsByRpIdAndAllowList(pDecodedAssertionRequest->pwszRpId,
                        pDecodedAssertionRequest->CredentialList.ppCredentials,
                        pDecodedAssertionRequest->CredentialList.cCredentials,
                        selectedCredentials);
                    break;
                }
                ++waitCount;
            }

            if (selectedCredentials.empty())
            {
                {
                    std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                    curApp->m_pluginOperationStatus.performOperationStatus = NTE_NOT_FOUND;
                }
                THROW_HR(NTE_NOT_FOUND);
            }
            else if (selectedCredentials.size() == 1 && credManager.GetSilentOperation())
            {
                selectedCredential = selectedCredentials[0];
            }
            else
            {
                RETURN_HR_IF(E_UNEXPECTED, !curApp->GetDispatcherQueue().TryEnqueue([
                    curApp,
                    rpId = std::wstring(pDecodedAssertionRequest->pwszRpId),
                    selectedCredentials,
                    hWnd = pPluginGetAssertionRequest->hWnd]()
                {
                    curApp->SetMatchingCredentials(rpId, selectedCredentials, hWnd);
                }));

                hIndex = 0;
                THROW_IF_FAILED(CoWaitForMultipleHandles(
                    COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, 
                    INFINITE, 
                    1, 
                    curApp->m_hPluginCredentialSelected.addressof(), 
                    &hIndex));

                {
                    std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                    selectedCredential = curApp->m_pluginOperationOptions.selectedCredential;
                }

                // Failed to select a credential
                if (!selectedCredential ||
                    selectedCredential->cbCredentialID == 0 ||
                    selectedCredential->pbCredentialID == nullptr ||
                    selectedCredential->pUserInformation == nullptr ||
                    selectedCredential->pUserInformation->pwszName == nullptr)
                {
                    {
                        std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                        curApp->m_pluginOperationStatus.performOperationStatus = NTE_NOT_FOUND;
                    }
                    THROW_HR(NTE_NOT_FOUND);
                }
            }

            wil::shared_cotaskmem_string userName = wil::make_cotaskmem_string(selectedCredential->pUserInformation->pwszName);

            std::vector<BYTE> requestBuffer(
                pPluginGetAssertionRequest->pbEncodedRequest,
                pPluginGetAssertionRequest->pbEncodedRequest + pPluginGetAssertionRequest->cbEncodedRequest);

            auto pubKeyData = GetRequestSigningPubKey();
            HRESULT requestSignResult = E_FAIL;
            if (!pubKeyData.empty())
            {
                requestSignResult = VerifySignatureHelper(
                    requestBuffer,
                    pubKeyData.data(),
                    static_cast<DWORD>(pubKeyData.size()),
                    pPluginGetAssertionRequest->pbRequestSignature,
                    pPluginGetAssertionRequest->cbRequestSignature);
            }

            {
                std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                curApp->m_pluginOperationStatus.requestSignatureVerificationStatus = requestSignResult;
            }

            hr = PerformUserVerification(
                pPluginGetAssertionRequest->hWnd,
                pPluginGetAssertionRequest->transactionId,
                PluginOperationType::GetAssertion,
                requestBuffer,
                rpName,
                userName);
            THROW_IF_FAILED(hr);

            // convert user handle to a string
            std::wstring keyNameStr = contosoplugin_key_domain;
            std::wstringstream keyNameStream;
            for (DWORD idx = 0; idx < selectedCredential->pUserInformation->cbId; idx++)
            {
                keyNameStream << std::hex << std::setw(2) << std::setfill(L'0') <<
                    static_cast<int>(selectedCredential->pUserInformation->pbId[idx]);
            }
            keyNameStr += keyNameStream.str();

            //open the key using ncrypt and sign the data
            wil::unique_ncrypt_prov hProvider;
            wil::shared_ncrypt_key hKey;

            // get the provider
            THROW_IF_FAILED(NCryptOpenStorageProvider(&hProvider, nullptr, 0));

            // open the key
            THROW_IF_FAILED(NCryptOpenKey(hProvider.get(), &hKey, keyNameStr.c_str(), 0, 0));

            // set hwnd property
            HWND hWnd;
            if (curApp->GetSilentMode())
            {
                hWnd = curApp->m_pluginOperationOptions.hWnd;
            }
            else
            {
                hWnd = curApp->GetNativeWindowHandle();
            }

            THROW_IF_FAILED(NCryptSetProperty(
                hKey.get(),
                NCRYPT_WINDOW_HANDLE_PROPERTY,
                reinterpret_cast<PBYTE>(&hWnd),
                sizeof(HWND),
                0));

            // create authenticator data
            DWORD cbPackedAuthenticatorData = 0;
            wil::unique_hlocal_ptr<BYTE[]> packedAuthenticatorData;
            std::vector<uint8_t> vCredentialIdBuffer;
            THROW_IF_FAILED(CreateAuthenticatorData(
                hKey.get(),
                PluginOperationType::GetAssertion,
                pDecodedAssertionRequest->cbRpId,
                pDecodedAssertionRequest->pbRpId,
                cbPackedAuthenticatorData,
                packedAuthenticatorData,
                vCredentialIdBuffer));

            wil::unique_hlocal_ptr<BYTE[]> pbSignature = nullptr;
            DWORD cbSignature = 0;

            {
                wil::unique_bcrypt_hash hashHandle;

                THROW_IF_NTSTATUS_FAILED(BCryptCreateHash(
                    BCRYPT_SHA256_ALG_HANDLE,
                    &hashHandle,
                    nullptr,
                    0,
                    nullptr,
                    0,
                    0));

                THROW_IF_NTSTATUS_FAILED(BCryptHashData(
                    hashHandle.get(), 
                    const_cast<PUCHAR>(packedAuthenticatorData.get()), 
                    cbPackedAuthenticatorData, 
                    0));

                THROW_IF_NTSTATUS_FAILED(BCryptHashData(
                    hashHandle.get(), 
                    const_cast<PUCHAR>(pDecodedAssertionRequest->pbClientDataHash), 
                    pDecodedAssertionRequest->cbClientDataHash, 
                    0));

                DWORD bytesRead = 0;
                DWORD cbSignatureBuffer = 0;
                THROW_IF_NTSTATUS_FAILED(BCryptGetProperty(
                    hashHandle.get(),
                    BCRYPT_HASH_LENGTH,
                    reinterpret_cast<PBYTE>(&cbSignatureBuffer),
                    sizeof(cbSignatureBuffer),
                    &bytesRead,
                    0));

                wil::unique_hlocal_ptr<BYTE[]> signatureBuffer = wil::make_unique_hlocal<BYTE[]>(cbSignatureBuffer);

                THROW_IF_NTSTATUS_FAILED(BCryptFinishHash(
                    hashHandle.get(), 
                    signatureBuffer.get(), 
                    cbSignatureBuffer, 
                    0));

                // sign the data
                THROW_IF_FAILED(NCryptSignHash(
                    hKey.get(), 
                    nullptr, 
                    signatureBuffer.get(), 
                    cbSignatureBuffer, 
                    nullptr, 
                    0, 
                    &cbSignature, 
                    0));

                pbSignature = wil::make_unique_hlocal<BYTE[]>(cbSignature);

                THROW_IF_FAILED(NCryptSignHash(
                    hKey.get(), 
                    nullptr, 
                    signatureBuffer.get(), 
                    cbSignatureBuffer, 
                    pbSignature.get(), 
                    cbSignature, 
                    &cbSignature, 
                    0));

                auto encodeSignature = [](PBYTE signature, size_t signatureSize) -> std::vector<BYTE>
                    {
                        std::vector<BYTE> encodedSignature;
                        encodedSignature.push_back(0x02);   // ASN integer tag 
                        encodedSignature.push_back(static_cast<BYTE>(signatureSize)); // length of the signature
                        if (WI_IsFlagSet(signature[0], 0x80))
                        {
                            encodedSignature[encodedSignature.size() - 1]++;
                            encodedSignature.push_back(0x00); // add a padding byte if the first byte has the high bit set
                        }

                        encodedSignature.insert(encodedSignature.end(), signature, signature + signatureSize);
                        return encodedSignature;
                    };

                auto signatureR = encodeSignature(pbSignature.get(), cbSignature / 2);
                auto signatureS = encodeSignature(pbSignature.get() + cbSignature / 2, cbSignature / 2);

                std::vector<BYTE> encodedSignature;
                encodedSignature.push_back(0x30); // ASN sequence tag
                encodedSignature.push_back(static_cast<BYTE>(signatureR.size() + signatureS.size())); // length of the sequence
                encodedSignature.insert(encodedSignature.end(), signatureR.begin(), signatureR.end());
                encodedSignature.insert(encodedSignature.end(), signatureS.begin(), signatureS.end());

                cbSignature = static_cast<DWORD>(encodedSignature.size());
                pbSignature.reset();
                pbSignature = wil::make_unique_hlocal<BYTE[]>(cbSignature);

                memcpy_s(pbSignature.get(), cbSignature, encodedSignature.data(), cbSignature);
            }

            auto assertionResponse = wil::make_unique_cotaskmem<WEBAUTHN_ASSERTION>();
            THROW_HR_IF_NULL(E_OUTOFMEMORY, assertionResponse);

            assertionResponse->dwVersion = WEBAUTHN_ASSERTION_CURRENT_VERSION;

            // [1] Credential (optional)
            assertionResponse->Credential.dwVersion = WEBAUTHN_CREDENTIAL_CURRENT_VERSION;
            assertionResponse->Credential.cbId = static_cast<DWORD>(vCredentialIdBuffer.size());
            assertionResponse->Credential.pbId = vCredentialIdBuffer.data();
            assertionResponse->Credential.pwszCredentialType = WEBAUTHN_CREDENTIAL_TYPE_PUBLIC_KEY;

            // [2] AuthenticatorData
            assertionResponse->cbAuthenticatorData = cbPackedAuthenticatorData;
            assertionResponse->pbAuthenticatorData = packedAuthenticatorData.get();

            // [3] Signature
            assertionResponse->cbSignature = cbSignature;
            assertionResponse->pbSignature = pbSignature.get();

            // [4] User (optional)
            assertionResponse->cbUserId = selectedCredential->pUserInformation->cbId;
            auto userIdBuffer = wil::make_unique_cotaskmem<BYTE[]>(selectedCredential->pUserInformation->cbId);
            THROW_HR_IF_NULL(E_OUTOFMEMORY, userIdBuffer);

            memcpy_s(userIdBuffer.get(),
                selectedCredential->pUserInformation->cbId,
                selectedCredential->pUserInformation->pbId,
                selectedCredential->pUserInformation->cbId);
            assertionResponse->pbUserId = userIdBuffer.get();

            WEBAUTHN_USER_ENTITY_INFORMATION userEntityInformation = {};
            userEntityInformation.dwVersion = WEBAUTHN_USER_ENTITY_INFORMATION_CURRENT_VERSION;
            userEntityInformation.cbId = assertionResponse->cbUserId;
            userEntityInformation.pbId = assertionResponse->pbUserId;

            // WEBAUTHN_CTAPCBOR_GET_ASSERTION_RESPONSE: This structure represents the complete get assertion
            // response including WebAuthn assertion, user information, and credential count.
            // Used for encoding into CBOR format for platform consumption.
            auto ctapGetAssertionResponse = wil::make_unique_cotaskmem<WEBAUTHN_CTAPCBOR_GET_ASSERTION_RESPONSE>();
            THROW_HR_IF_NULL(E_OUTOFMEMORY, ctapGetAssertionResponse);

            ctapGetAssertionResponse->WebAuthNAssertion = *(assertionResponse.get()); // [1] Credential, [2] AuthenticatorData, [3] Signature
            ctapGetAssertionResponse->pUserInformation = &userEntityInformation; // [4] User
            ctapGetAssertionResponse->dwNumberOfCredentials = 1; // [5] NumberOfCredentials

            DWORD cbAssertionBuffer = 0;
            wil::unique_cotaskmem_ptr<BYTE[]> pbAssertionBuffer;

            // PCWEBAUTHN_CTAPCBOR_GET_ASSERTION_RESPONSE: Pointer to get assertion response
            // structure used for encoding operations. Provides const access to response data.
            THROW_IF_FAILED(WebAuthNEncodeGetAssertionResponse(
                (PCWEBAUTHN_CTAPCBOR_GET_ASSERTION_RESPONSE)(ctapGetAssertionResponse.get()),
                &cbAssertionBuffer,
                wil::out_param(pbAssertionBuffer)));

            response->cbEncodedResponse = cbAssertionBuffer;
            response->pbEncodedResponse = pbAssertionBuffer.release();
            return S_OK;
        }
        catch (...)
        {
            hr = wil::ResultFromCaughtException();
            try
            {
                winrt::com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
                if (curApp)
                {
                    std::lock_guard<std::mutex> lock(curApp->m_pluginOperationOptionsMutex);
                    curApp->m_pluginOperationStatus.performOperationStatus = hr;
                }
            }
            catch (...)
            {
                // Ignore errors during cleanup
            }
            return hr;
        }
    }

    /*
    * This function is invoked by the platform to fetch the state of the plugin's vault
    */
    HRESULT STDMETHODCALLTYPE ContosoPlugin::GetLockStatus(
        /* [out] */ __RPC__out PLUGIN_LOCK_STATUS* vaultState) noexcept
    {
        auto& credManager = PluginCredentialManager::getInstance();
        credManager.ReloadRegistryValues();
        *vaultState = credManager.GetVaultLock() ? PluginLocked : PluginUnlocked;
        return S_OK;
    }

    /*
    * This function is invoked by the platform to request the plugin to cancel an ongoing operation.
    */
    HRESULT STDMETHODCALLTYPE ContosoPlugin::CancelOperation(
        /* [out] */ __RPC__in PCWEBAUTHN_PLUGIN_CANCEL_OPERATION_REQUEST pCancelRequest)
    {
        try
        {
            RETURN_HR_IF_NULL(E_INVALIDARG, pCancelRequest);

            com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
            RETURN_HR_IF(NTE_NOT_FOUND, curApp->GetPluginTransactionId() != pCancelRequest->transactionId);

            SetEvent(m_hPluginOpCompletedEvent.get());

            RETURN_HR_IF(E_UNEXPECTED, !curApp->GetDispatcherQueue().TryEnqueue([curApp]()
            {
                curApp->PluginCancelAction();
            }));
            return S_OK;
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    /*
    * This is a sample implementation of a factory method that creates an instance of the Class that implements
    * the IPluginAuthenticator interface. The IPluginAuthenticator interface is the core COM interface that
    * third-party passkey authenticator plugins must implement for Windows. This interface enables plugins to
    * participate in WebAuthn operations by handling make credential, get assertion, lock status, and operation
    * cancellation requests.
    * Refer: pluginauthenticator.h/pluginauthenticator.idl for the interface definition.
    */
    HRESULT __stdcall ContosoPluginFactory::CreateInstance(
        ::IUnknown* outer,
        GUID const& iid,
        void** result) noexcept
    {
        try
        {
            RETURN_HR_IF_NULL(E_INVALIDARG, result);
            *result = nullptr;

            if (outer)
            {
                return CLASS_E_NOAGGREGATION;
            }

            return make<ContosoPlugin>(m_hPluginOpCompletedEvent, m_hAppReadyForPluginOpEvent, m_hPluginCancelOperationEvent)->QueryInterface(iid, result);
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    HRESULT __stdcall ContosoPluginFactory::LockServer(BOOL) noexcept
    {
        return S_OK;
    }
}
