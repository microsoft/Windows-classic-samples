#include "pch.h"
#include "PluginCredentialManager.h"
#include <CorError.h>
#include <Credential.h>
#include <wil/registry_helpers.h>
#include <wil/safecast.h>
#include <wil/filesystem.h>
#include <wil/result.h>
#include <filesystem>
#include <windows.storage.h>
#include <algorithm>
#include <memory>

namespace winrt::PasskeyManager::implementation
{
    PluginCredentialManager::PluginCredentialManager()
    {
        Initialize();
    }

    PluginCredentialManager::~PluginCredentialManager()
    {
        // Destructor cleanup is handled by RAII patterns
    }

    HRESULT PluginCredentialManager::AddAllPluginCredentials()
    {
        // WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS: This structure represents credential metadata
        // that can be shared with the platform to enable various user experience scenarios.
        std::vector<WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS> credentialDetailList;

        {
            std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
            credentialDetailList.reserve(m_pluginLocalCredentialMetadataMap.size()); // Reserve space to avoid reallocations
            for (const auto& [credentialId, savedCred] : m_pluginLocalCredentialMetadataMap)
            {
                if (!savedCred || !savedCred->pRpInformation || !savedCred->pUserInformation)
                {
                    continue; // Skip invalid credentials
                }

                // Create WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS structure with credential metadata
                // for sharing with the platform's credential database
                WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS pluginCred = {};
                pluginCred.cbCredentialId = savedCred->cbCredentialID;
                pluginCred.pbCredentialId = savedCred->pbCredentialID;
                pluginCred.pwszRpId = savedCred->pRpInformation->pwszId;
                pluginCred.pwszRpName = savedCred->pRpInformation->pwszName;
                pluginCred.cbUserId = savedCred->pUserInformation->cbId;
                pluginCred.pbUserId = savedCred->pUserInformation->pbId;
                pluginCred.pwszUserName = savedCred->pUserInformation->pwszName;
                pluginCred.pwszUserDisplayName = savedCred->pUserInformation->pwszDisplayName;

                credentialDetailList.push_back(pluginCred);
            }
        }

        RETURN_HR_IF(E_FAIL, credentialDetailList.empty());

        // Call the API to add credentials to the platform database
        RETURN_IF_FAILED(WebAuthNPluginAuthenticatorAddCredentials(
            contosoplugin_guid,
            static_cast<DWORD>(credentialDetailList.size()),
            credentialDetailList.data()));

        return S_OK;
    }

    HRESULT PluginCredentialManager::AddPluginCredentialById(const std::vector<std::vector<UINT8>>& credentialIdList)
    {
        RETURN_HR_IF(E_INVALIDARG, credentialIdList.empty());

        // WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS: Creating credential detail structures for the
        // selected credential IDs to be added to the platform's shared credential database.
        std::vector<WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS> credentialDetailList;
        credentialDetailList.reserve(credentialIdList.size());

        {
            std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
            for (const auto& credentialId : credentialIdList)
            {
                auto iter = m_pluginLocalCredentialMetadataMap.find(credentialId);
                if (iter != m_pluginLocalCredentialMetadataMap.end())
                {
                    const auto& savedCred = iter->second;
                    if (!savedCred || !savedCred->pRpInformation || !savedCred->pUserInformation)
                    {
                        continue; // Skip invalid credentials
                    }

                    // Populate WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS with credential metadata
                    WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS pluginCred = {};
                    pluginCred.cbCredentialId = savedCred->cbCredentialID;
                    pluginCred.pbCredentialId = savedCred->pbCredentialID;
                    pluginCred.pwszRpId = savedCred->pRpInformation->pwszId;
                    pluginCred.pwszRpName = savedCred->pRpInformation->pwszName;
                    pluginCred.cbUserId = savedCred->pUserInformation->cbId;
                    pluginCred.pbUserId = savedCred->pUserInformation->pbId;
                    pluginCred.pwszUserName = savedCred->pUserInformation->pwszName;
                    pluginCred.pwszUserDisplayName = savedCred->pUserInformation->pwszDisplayName;

                    credentialDetailList.push_back(pluginCred);
                }
            }
        }

        if (!credentialDetailList.empty())
        {
            // Use API to add selected credentials to platform database
            RETURN_IF_FAILED(WebAuthNPluginAuthenticatorAddCredentials(
                contosoplugin_guid,
                static_cast<DWORD>(credentialDetailList.size()),
                credentialDetailList.data()));
        }

        return S_OK;
    }

    HRESULT PluginCredentialManager::DeleteAllPluginCredentials()
    {
        // Call API to remove all credentials from platform database
        RETURN_IF_FAILED(WebAuthNPluginAuthenticatorRemoveAllCredentials(contosoplugin_guid));

        {
            std::lock_guard<std::mutex> lock(m_pluginCachedCredentialsOperationMutex);
            m_pluginCachedCredentialMetadataMap.clear();
        }

        return S_OK;
    }

    HRESULT PluginCredentialManager::DeletePluginCredentialById(const std::vector<std::vector<UINT8>>& credentialIdList, bool deleteEverywhere)
    {
        RETURN_HR_IF(E_INVALIDARG, credentialIdList.empty());

        // WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS: Preparing credential structures for the credentials
        // that need to be removed from the platform database.
        std::vector<WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS> credentialDetailList;
        credentialDetailList.reserve(credentialIdList.size());

        {
            std::lock_guard<std::mutex> lock(m_pluginCachedCredentialsOperationMutex);
            for (const auto& credentialId : credentialIdList)
            {
                auto savedCred = m_pluginCachedCredentialMetadataMap.find(credentialId);
                if (savedCred != m_pluginCachedCredentialMetadataMap.end())
                {
                    const auto& credDetails = savedCred->second;
                    if (credDetails)
                    {
                        // Create WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS for removal operation
                        WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS pluginCred = {};
                        pluginCred.cbCredentialId = credDetails->cbCredentialId;
                        pluginCred.pbCredentialId = credDetails->pbCredentialId;
                        pluginCred.pwszRpId = credDetails->pwszRpId;
                        pluginCred.pwszRpName = credDetails->pwszRpName;
                        pluginCred.cbUserId = credDetails->cbUserId;
                        pluginCred.pbUserId = credDetails->pbUserId;
                        pluginCred.pwszUserName = credDetails->pwszUserName;
                        pluginCred.pwszUserDisplayName = credDetails->pwszUserDisplayName;

                        credentialDetailList.push_back(pluginCred);
                    }
                }
            }
        }

        if (!credentialDetailList.empty())
        {
            // Use API to remove specified credentials from platform database
            RETURN_IF_FAILED(WebAuthNPluginAuthenticatorRemoveCredentials(
                contosoplugin_guid,
                static_cast<DWORD>(credentialDetailList.size()),
                credentialDetailList.data()));
        }

        // Remove from caches
        {
            std::lock_guard<std::mutex> lockCache(m_pluginCachedCredentialsOperationMutex);
            for (const auto& credentialId : credentialIdList)
            {
                m_pluginCachedCredentialMetadataMap.erase(credentialId);
            }
        }

        if (deleteEverywhere)
        {
            {
                std::lock_guard<std::mutex> lockLocal(m_pluginLocalCredentialsOperationMutex);
                for (const auto& credentialId : credentialIdList)
                {
                    m_pluginLocalCredentialMetadataMap.erase(credentialId);
                }
            }
            if (!RecreateCredentialMetadataFile())
            {
                return E_FAIL;
            }
        }

        return S_OK;
    }

    HRESULT PluginCredentialManager::RefreshAutofillPluginCredentialsList()
    {
        {
            std::lock_guard<std::mutex> lock(m_pluginCachedCredentialsOperationMutex);
            m_cachedCredentialsLoaded = false;
            m_pluginCachedCredentialMetadataMap.clear();
        }

        DWORD cCredentialDetails = 0;
        // PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS: Pointer to array of credential detail structures
        // returned by the platform database query.
        PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS ppCredentialDetailsArray = nullptr;

        // Call API to retrieve all credentials from platform database
        HRESULT hr = WebAuthNPluginAuthenticatorGetAllCredentials(contosoplugin_guid, &cCredentialDetails, &ppCredentialDetailsArray);
        RETURN_HR_IF_EXPECTED(S_OK, hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
        RETURN_IF_FAILED(hr);

        // Ensure cleanup even on early return using free function
        auto cleanup = wil::scope_exit([&] {
            WebAuthNPluginAuthenticatorFreeCredentialDetailsArray(cCredentialDetails, ppCredentialDetailsArray);
        });

        {
            std::lock_guard<std::mutex> lock(m_pluginCachedCredentialsOperationMutex);
            for (DWORD i = 0; i < cCredentialDetails; i++)
            {
                // Process each PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS returned by the platform
                PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS credentialDetailsPtr = &ppCredentialDetailsArray[i];
                
                // Validate credential data first
                if (!credentialDetailsPtr->pbCredentialId || credentialDetailsPtr->cbCredentialId == 0 ||
                    !credentialDetailsPtr->pbUserId || credentialDetailsPtr->cbUserId == 0 ||
                    !credentialDetailsPtr->pwszRpId || !credentialDetailsPtr->pwszRpName ||
                    !credentialDetailsPtr->pwszUserName || !credentialDetailsPtr->pwszUserDisplayName)
                {
                    continue;
                }

                std::vector<UINT8> credentialId(credentialDetailsPtr->pbCredentialId, 
                    credentialDetailsPtr->pbCredentialId + credentialDetailsPtr->cbCredentialId);

                // Use RAII for automatic cleanup
                try {
                    // Start directly with the typed unique_ptr with custom deleter
                    auto managedCredential = unique_plugin_credential_details(new WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS());
                    ZeroMemory(managedCredential.get(), sizeof(WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS));

                    // Copy credential ID
                    managedCredential->cbCredentialId = credentialDetailsPtr->cbCredentialId;
                    auto credIdBuffer = std::make_unique<BYTE[]>(credentialDetailsPtr->cbCredentialId);
                    memcpy_s(credIdBuffer.get(), managedCredential->cbCredentialId,
                        credentialDetailsPtr->pbCredentialId, credentialDetailsPtr->cbCredentialId);
                    managedCredential->pbCredentialId = credIdBuffer.release();

                    // Copy strings using proper allocation methods
                    managedCredential->pwszRpId = wil::make_cotaskmem_string(credentialDetailsPtr->pwszRpId).release();
                    managedCredential->pwszRpName = wil::make_cotaskmem_string(credentialDetailsPtr->pwszRpName).release();
                    managedCredential->pwszUserName = wil::make_cotaskmem_string(credentialDetailsPtr->pwszUserName).release();
                    managedCredential->pwszUserDisplayName = wil::make_cotaskmem_string(credentialDetailsPtr->pwszUserDisplayName).release();

                    // Copy user ID
                    managedCredential->cbUserId = credentialDetailsPtr->cbUserId;
                    auto userIdBuffer = std::make_unique<BYTE[]>(credentialDetailsPtr->cbUserId);
                    memcpy_s(userIdBuffer.get(), managedCredential->cbUserId,
                        credentialDetailsPtr->pbUserId, credentialDetailsPtr->cbUserId);
                    managedCredential->pbUserId = userIdBuffer.release();

                    // Directly move the managed credential with custom deleter
                    m_pluginCachedCredentialMetadataMap.emplace(std::move(credentialId), std::move(managedCredential));
                }
                catch (...) {
                    // Continue processing other credentials on allocation failure
                    continue;
                }
            }
            m_cachedCredentialsLoaded = true;
        }

        return S_OK;
    }

    std::vector<winrt::com_ptr<Credential>> PluginCredentialManager::GetCredentialListViewModel()
    {
        std::vector<winrt::com_ptr<Credential>> credentialViewList;
        
        // Reserve space to avoid reallocations - estimate based on typical usage
        constexpr size_t ESTIMATED_CREDENTIALS = 10;
        credentialViewList.reserve(ESTIMATED_CREDENTIALS);
        
        // Process local credentials first
        {
            std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
            credentialViewList.reserve(credentialViewList.size() + m_pluginLocalCredentialMetadataMap.size());
            
            for (const auto& [credentialId, savedCredential] : m_pluginLocalCredentialMetadataMap)
            {
                if (!savedCredential || !savedCredential->pUserInformation || !savedCredential->pRpInformation)
                {
                    continue;
                }

                auto credentialOptions = CredentialOptionFlags::MetadataValid;
                if (IsPluginCredentialIdAutofillSupported(savedCredential->cbCredentialID, savedCredential->pbCredentialID))
                {
                    credentialOptions |= CredentialOptionFlags::AutofillCapable;
                }

                // Optimize buffer creation - reuse DataWriter for better performance
                auto writer = winrt::Windows::Storage::Streams::DataWriter();
                writer.WriteBytes(credentialId); // Direct vector write
                auto credentialIdBuffer = writer.DetachBuffer();

                auto credentialViewListItem = winrt::make_self<PasskeyManager::implementation::Credential>(
                    savedCredential->pUserInformation->pwszName, 
                    savedCredential->pRpInformation->pwszName, 
                    credentialIdBuffer, 
                    credentialOptions);
                credentialViewList.emplace_back(std::move(credentialViewListItem));
            }
        }

        // Process cached credentials that aren't already in local storage
        // These are credentials retrieved from the platform database via APIs
        {
            std::lock_guard<std::mutex> lock1(m_pluginCachedCredentialsOperationMutex);
            std::lock_guard<std::mutex> lock2(m_pluginLocalCredentialsOperationMutex);

            for (const auto& [cachedCredentialId, cachedCredential] : m_pluginCachedCredentialMetadataMap)
            {
                if (!cachedCredential)
                {
                    continue;
                }

                // Skip if already processed in local credentials
                if (m_pluginLocalCredentialMetadataMap.find(cachedCredentialId) != m_pluginLocalCredentialMetadataMap.end())
                {
                    continue;
                }

                auto credentialOptions = CredentialOptionFlags::AutofillCapable;

                // Reuse writer pattern
                auto writer = winrt::Windows::Storage::Streams::DataWriter();
                writer.WriteBytes(cachedCredentialId);
                auto credentialIdBuffer = writer.DetachBuffer();

                // Create credential view item from cached credential data
                auto credentialViewListItem = winrt::make_self<PasskeyManager::implementation::Credential>(
                    cachedCredential->pwszUserName, 
                    cachedCredential->pwszRpName, 
                    credentialIdBuffer, 
                    credentialOptions);
                credentialViewList.emplace_back(std::move(credentialViewListItem));
            }
        }

        return credentialViewList;
    }

    bool PluginCredentialManager::IsPluginCredentialIdAutofillSupported(DWORD cbCredentialId, PBYTE pbCredentialId) const
    {
        if (!pbCredentialId || cbCredentialId == 0)
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_pluginCachedCredentialsOperationMutex);
        std::vector<UINT8> credentialId(pbCredentialId, pbCredentialId + cbCredentialId);
        // Check if credential exists in cached metadata retrieved from platform database via APIs
        return m_pluginCachedCredentialMetadataMap.find(credentialId) != m_pluginCachedCredentialMetadataMap.end();
    }

    namespace {
        using unique_file_buffer = std::unique_ptr<BYTE[]>;

        DWORD readBytesFromFile(std::ifstream& file, unique_file_buffer& buffer, const DWORD maxSize)
        {
            DWORD bytesToRead = 0;
            buffer.reset(); // Ensure clean state
            
            if (!file.is_open())
            {
                return 0;
            }
            
            // Read the size first
            file.read(reinterpret_cast<char*>(&bytesToRead), sizeof(bytesToRead));
            if (!file.good() || bytesToRead == 0)
            {
                return 0;
            }
            
            // Check size limits
            if (maxSize > 0 && bytesToRead > maxSize)
            {
                return 0;
            }
            
            // Allocate buffer with RAII
            try {
                auto rawBuffer = std::make_unique<BYTE[]>(bytesToRead);
                buffer.reset(rawBuffer.release());
            }
            catch (const std::bad_alloc&) {
                return 0;
            }

            // Initialize buffer
            ZeroMemory(buffer.get(), bytesToRead);

            // Read the actual data
            file.read(reinterpret_cast<char*>(buffer.get()), bytesToRead);
            if (!file.good())
            {
                buffer.reset(); // Clean up on failure
                return 0;
            }
            
            return bytesToRead;
        }
    }

    bool PluginCredentialManager::LoadSavedCredentialsFromMockDatabase()
    {
        std::wstring filePath;
        if (!GetCredentialStorageFilePath(filePath))
        {
            return false;
        }

        std::ifstream file(filePath, std::ifstream::in | std::ifstream::binary);
        if (!file.is_open())
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
        m_localCredentialsLoaded = false;
        m_pluginLocalCredentialMetadataMap.clear();

        while (file.good() && !file.eof())
        {
            try {
                auto savedCredential = std::make_unique<WEBAUTHN_CREDENTIAL_DETAILS>();
                ZeroMemory(savedCredential.get(), sizeof(WEBAUTHN_CREDENTIAL_DETAILS));
                
                auto userInfo = std::make_unique<WEBAUTHN_USER_ENTITY_INFORMATION>();
                auto rpInfo = std::make_unique<WEBAUTHN_RP_ENTITY_INFORMATION>();
                ZeroMemory(userInfo.get(), sizeof(WEBAUTHN_USER_ENTITY_INFORMATION));
                ZeroMemory(rpInfo.get(), sizeof(WEBAUTHN_RP_ENTITY_INFORMATION));

                // Read user ID
                unique_file_buffer userIdBuffer;
                DWORD userIdSize = readBytesFromFile(file, userIdBuffer, 0);
                if (userIdSize == 0 || !userIdBuffer)
                {
                    break; // End of file or error
                }
                userInfo->pbId = userIdBuffer.release();
                userInfo->cbId = userIdSize;

                // Read username
                unique_file_buffer userNameBuffer;
                DWORD nameSize = readBytesFromFile(file, userNameBuffer, maxWebAuthnWStringSize * sizeof(WCHAR));
                if (nameSize == 0 || !userNameBuffer)
                {
                    break;
                }
                userInfo->pwszName = reinterpret_cast<WCHAR*>(userNameBuffer.release());

                // Read display name
                unique_file_buffer displayNameBuffer;
                DWORD displayNameSize = readBytesFromFile(file, displayNameBuffer, maxWebAuthnWStringSize * sizeof(WCHAR));
                if (displayNameSize == 0 || !displayNameBuffer)
                {
                    break;
                }
                userInfo->pwszDisplayName = reinterpret_cast<WCHAR*>(displayNameBuffer.release());

                // Read credential ID
                unique_file_buffer credentialIdBuffer;
                DWORD credentialIdSize = readBytesFromFile(file, credentialIdBuffer, 0);
                if (credentialIdSize == 0 || !credentialIdBuffer)
                {
                    break;
                }
                savedCredential->pbCredentialID = credentialIdBuffer.release();
                savedCredential->cbCredentialID = credentialIdSize;

                // Read RP ID
                unique_file_buffer rpIdBuffer;
                DWORD rpIdSize = readBytesFromFile(file, rpIdBuffer, maxWebAuthnWStringSize * sizeof(WCHAR));
                if (rpIdSize == 0 || !rpIdBuffer)
                {
                    break;
                }
                rpInfo->pwszId = reinterpret_cast<WCHAR*>(rpIdBuffer.release());

                // Read RP name
                unique_file_buffer rpNameBuffer;
                DWORD rpNameSize = readBytesFromFile(file, rpNameBuffer, maxWebAuthnWStringSize * sizeof(WCHAR));
                if (rpNameSize == 0 || !rpNameBuffer)
                {
                    break;
                }
                rpInfo->pwszName = reinterpret_cast<WCHAR*>(rpNameBuffer.release());

                // Transfer ownership
                savedCredential->pUserInformation = userInfo.release();
                savedCredential->pRpInformation = rpInfo.release();

                // Create credential ID vector for map key
                std::vector<UINT8> localCredId(savedCredential->pbCredentialID, 
                    savedCredential->pbCredentialID + savedCredential->cbCredentialID);

                // Transfer to managed container
                unique_credential_details managedCredential(savedCredential.release());
                m_pluginLocalCredentialMetadataMap.emplace(std::move(localCredId), std::move(managedCredential));
            }
            catch (const std::exception&) {
                break;
            }
        }

        file.close();
        m_localCredentialsLoaded = true;
        return true;
    }

    bool PluginCredentialManager::GetCredentialStorageFilePath(std::wstring& filePath)
    {
        try
        {
            Windows::Storage::ApplicationData appData = Windows::Storage::ApplicationData::Current();
            Windows::Storage::StorageFolder localFolder = appData.LocalFolder();
            std::wstring localFolderPath = localFolder.Path().c_str();
            std::wstring directoryPath = localFolderPath + L"\\" + c_pluginLocalAppDataDBDir;

            // Use std::filesystem for better error handling
            std::error_code ec;
            if (!std::filesystem::exists(directoryPath, ec))
            {
                if (!std::filesystem::create_directories(directoryPath, ec) || ec)
                {
                    return false;
                }
            }

            filePath = directoryPath + L"\\" + c_credentialsFileName;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool PluginCredentialManager::SaveCredentialMetadataToMockDB(const WEBAUTHN_CREDENTIAL_DETAILS& newCredential)
    {
        // Validate input parameters
        if (newCredential.cbCredentialID == 0 || !newCredential.pbCredentialID ||
            !newCredential.pUserInformation || !newCredential.pRpInformation ||
            !newCredential.pUserInformation->pwszName || !newCredential.pUserInformation->pwszDisplayName ||
            !newCredential.pRpInformation->pwszId || !newCredential.pRpInformation->pwszName)
        {
            return false;
        }

        std::wstring filePath;
        if (!GetCredentialStorageFilePath(filePath))
        {
            return false;
        }

        try {
            std::ofstream file(filePath, std::ofstream::out | std::ofstream::binary | std::ofstream::app);
            if (!file.is_open())
            {
                return false;
            }

            // Use RAII for automatic file closure
            auto fileGuard = wil::scope_exit([&file] { file.close(); });

            // Write user ID
            DWORD len = newCredential.pUserInformation->cbId;
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(newCredential.pUserInformation->pbId), len);

            // Write username
            len = static_cast<DWORD>((wcslen(newCredential.pUserInformation->pwszName) + 1) * sizeof(WCHAR));
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(newCredential.pUserInformation->pwszName), len);

            // Write display name
            len = static_cast<DWORD>((wcslen(newCredential.pUserInformation->pwszDisplayName) + 1) * sizeof(WCHAR));
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(newCredential.pUserInformation->pwszDisplayName), len);

            // Write credential ID
            len = newCredential.cbCredentialID;
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(newCredential.pbCredentialID), len);

            // Write RP ID
            len = static_cast<DWORD>((wcslen(newCredential.pRpInformation->pwszId) + 1) * sizeof(WCHAR));
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(newCredential.pRpInformation->pwszId), len);

            // Write RP name
            len = static_cast<DWORD>((wcslen(newCredential.pRpInformation->pwszName) + 1) * sizeof(WCHAR));
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(newCredential.pRpInformation->pwszName), len);

            return file.good();
        }
        catch (...) {
            return false;
        }
    }

    void PluginCredentialManager::GetLocalCredsByRpIdAndAllowList(PCWSTR rpId, PWEBAUTHN_CREDENTIAL_EX* ppCredentialList, DWORD pcCredentials, std::vector<const WEBAUTHN_CREDENTIAL_DETAILS *>& matchingCredentials)
    {
        std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
        for (const auto& mapItem : m_pluginLocalCredentialMetadataMap)
        {
            auto& cred = mapItem.second;
            if (lstrcmpW(cred->pRpInformation->pwszId, rpId) == 0)
            {
                if (pcCredentials > 0)
                {
                    for (DWORD i = 0; i < pcCredentials; i++)
                    {
                        if (cred->cbCredentialID == ppCredentialList[i]->cbId &&
                            memcmp(cred->pbCredentialID, ppCredentialList[i]->pbId, cred->cbCredentialID) == 0)
                        {
                            matchingCredentials.push_back(cred.get());
                            break;
                        }
                    }
                }
                else
                {
                    matchingCredentials.push_back(cred.get());
                }
            }
        }
    }

    bool PluginCredentialManager::RecreateCredentialMetadataFile()
    {
        std::wstring filePath;
        if (!GetCredentialStorageFilePath(filePath))
        {
            return false;
        }

        std::ofstream file{};
        file.open(filePath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        if (file.is_open())
        {
            file.close();
        }
        else
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
        // iterating over all the saved credentials and writing them to the file
        for (const auto& credEntry : m_pluginLocalCredentialMetadataMap)
        {
            SaveCredentialMetadataToMockDB(*credEntry.second.get());
        }
        return true;
    }

    bool PluginCredentialManager::ResetLocalCredentialsStore()
    {
        {
            std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
            m_pluginLocalCredentialMetadataMap.clear();
        }
        return RecreateCredentialMetadataFile();
    }

    HRESULT PluginCredentialManager::SetVaultLock(bool lock)
    {
        std::lock_guard<std::mutex> lockguard(m_pluginOperationConfigMutex);
        if (m_vaultLocked != lock)
        {
            wil::unique_hkey hKey;
            RETURN_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CURRENT_USER, c_pluginRegistryPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr));
            RETURN_IF_WIN32_ERROR(RegSetValueEx(hKey.get(), c_windowsPluginVaultLockedRegKeyName, 0, REG_DWORD, reinterpret_cast<PBYTE>(&lock), sizeof(lock)));
            m_vaultLocked = lock;
        }
        return S_OK;
    }

    bool PluginCredentialManager::GetVaultLock()
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        return m_vaultLocked;
    }

    HRESULT PluginCredentialManager::SetSilentOperation(bool silent)
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        if (m_silentOperation != silent)
        {
            wil::unique_hkey hKey;
            RETURN_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CURRENT_USER, c_pluginRegistryPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr));
            RETURN_IF_WIN32_ERROR(RegSetValueEx(hKey.get(), c_windowsPluginSilentOperationRegKeyName, 0, REG_DWORD, reinterpret_cast<PBYTE>(&silent), sizeof(silent)));
            m_silentOperation = silent;
        }
        return S_OK;
    }

    bool PluginCredentialManager::GetSilentOperation()
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        return m_silentOperation;
    }

    HRESULT PluginCredentialManager::SetVaultUnlockMethod(VaultUnlockMethod method)
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        if (m_vaultUnlockMethod != method)
        {
            wil::unique_hkey hKey;
            RETURN_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CURRENT_USER, c_pluginRegistryPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr));
            RETURN_IF_WIN32_ERROR(RegSetValueEx(hKey.get(), c_windowsPluginVaultUnlockMethodRegKeyName, 0, REG_DWORD, reinterpret_cast<PBYTE>(&method), sizeof(method)));
            m_vaultUnlockMethod = method;
        }
        return S_OK;
    }

    VaultUnlockMethod PluginCredentialManager::GetVaultUnlockMethod()
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        return m_vaultUnlockMethod;
    }

    // This function simulates using a webauthn PRF to secure the vault data.
    // In a real implementation, the encrypted vault data would be
    // returned by ReadEncryptedVaultData() and would be decrypted using
    // the HMAC secret returned by the authenticator.
    // In case of a cloud based authenticator, the HMAC secret would be
    // returned by the cloud service and the encrypted vault data would
    // be stored in the cloud.
    HRESULT PluginCredentialManager::UnlockCredentialVaultWithPasskey(HWND hwnd) try
    {
        WEBAUTHN_CLIENT_DATA clientData = {};
        clientData.dwVersion = WEBAUTHN_CLIENT_DATA_CURRENT_VERSION;
        clientData.pwszHashAlgId = WEBAUTHN_HASH_ALGORITHM_SHA_256;
        // This is a dummy challenge, a real challenge may be randomly generated or
        // is received from the cloud service in case of cloud based authenticators.
        std::string clientDataStr = "{\"challenge\":\"eyJjaGFsbGVuZ2eciOiEiY2hhbGxlbmdlIn0\"}";
        clientData.pbClientDataJSON = reinterpret_cast<PBYTE>(clientDataStr.data());
        clientData.cbClientDataJSON = static_cast<DWORD>(clientDataStr.size());

        PluginRegistrationManager::getInstance().ReloadRegistryValues();
        std::vector<BYTE> hmacSecretValue = PluginRegistrationManager::getInstance().GetHMACSecret();

        WEBAUTHN_HMAC_SECRET_SALT hmacSecretSalt = {};
        hmacSecretSalt.cbFirst = wil::safe_cast<DWORD>(hmacSecretValue.size());
        hmacSecretSalt.pbFirst = hmacSecretValue.data();

        WEBAUTHN_HMAC_SECRET_SALT_VALUES hmacSecretSaltValues = {};
        hmacSecretSaltValues.pGlobalHmacSalt = &hmacSecretSalt;

        WEBAUTHN_AUTHENTICATOR_GET_ASSERTION_OPTIONS webAuthNGetAssertionOptions = {};
        webAuthNGetAssertionOptions.dwVersion = WEBAUTHN_AUTHENTICATOR_GET_ASSERTION_OPTIONS_CURRENT_VERSION;
        webAuthNGetAssertionOptions.dwTimeoutMilliseconds = 600 * 1000;
        webAuthNGetAssertionOptions.dwAuthenticatorAttachment = WEBAUTHN_AUTHENTICATOR_ATTACHMENT_ANY;
        webAuthNGetAssertionOptions.dwUserVerificationRequirement = WEBAUTHN_USER_VERIFICATION_REQUIREMENT_REQUIRED;
        webAuthNGetAssertionOptions.pHmacSecretSaltValues = &hmacSecretSaltValues;

        unique_webauthn_assertion pAssertion = nullptr;
        RETURN_IF_FAILED(WebAuthNAuthenticatorGetAssertion(
            hwnd,
            c_pluginRpId,
            &clientData,
            &webAuthNGetAssertionOptions,
            &pAssertion));

        if (pAssertion.get()->pHmacSecret == nullptr)
        {
            return NTE_NOT_SUPPORTED; // The chosen authenticator does not support PRF.
        }

        DATA_BLOB entropy = {
            .cbData = pAssertion.get()->pHmacSecret->cbFirst,
            .pbData = pAssertion.get()->pHmacSecret->pbFirst
        };

        std::vector<uint8_t> cipherText;
        RETURN_IF_FAILED(PluginRegistrationManager::getInstance().ReadEncryptedVaultData(cipherText));
        DATA_BLOB cipherTextBlob = {
            .cbData = static_cast<DWORD>(cipherText.size()),
            .pbData = cipherText.data()
        };

        DATA_BLOB decryptedData = {};

        RETURN_IF_WIN32_BOOL_FALSE(CryptUnprotectData(
            &cipherTextBlob,
            nullptr,
            &entropy,
            nullptr,
            nullptr,
            CRYPTPROTECT_UI_FORBIDDEN,
            &decryptedData));

        // scope exit for freeing the decrypted data
        auto decryptedDataCleanup = wil::scope_exit([&] {
            if (decryptedData.pbData)
            {
                LocalFree(decryptedData.pbData);
            }
        });

        if (decryptedData.cbData != wcslen(c_dummySecretVault) * sizeof(wchar_t) || memcmp(decryptedData.pbData, c_dummySecretVault, decryptedData.cbData) != 0)
        {
            return E_FAIL;
        }

        return S_OK;
    }
    CATCH_RETURN()
}
