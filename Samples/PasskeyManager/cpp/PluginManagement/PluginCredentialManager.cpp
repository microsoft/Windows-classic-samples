#include "pch.h"
#include "PluginCredentialManager.h"
#include <CorError.h>
#include <Credential.h>
#include <wil/win32_helpers.h>
#include <wil/registry.h>
#include <wil/registry_helpers.h>
#include <wil/filesystem.h>
#include <filesystem>
#include <windows.storage.h>


namespace winrt::PasskeyManager::implementation
{

    PluginCredentialManager::PluginCredentialManager() : m_webAuthnDll(LoadLibraryExW(L"webauthn.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32))
    {
        Initialize();
    }

    PluginCredentialManager::~PluginCredentialManager()
    {
    }

    HRESULT PluginCredentialManager::AddAllPluginCredentials()
    {
        // Get the function pointer of WebAuthNPluginAuthenticatorAddCredentials
        auto webAuthNPluginAuthenticatorAddCredentials = GetProcAddressByFunctionDeclaration(m_webAuthnDll.get(), WebAuthNPluginAuthenticatorAddCredentials);
        RETURN_HR_IF_NULL(E_FAIL, webAuthNPluginAuthenticatorAddCredentials);

        std::vector<unique_plugin_credential_details> credentialDetailList{};
        for (auto& iter : m_pluginLocalCredentialMetadataMap)
        {
            auto& savedCred = iter.second;
            unique_plugin_credential_details pluginCred (new WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS());
            pluginCred->pwszRpId = _wcsdup(savedCred->pRpInformation->pwszId);
            pluginCred->pwszRpName = _wcsdup(savedCred->pRpInformation->pwszName);
            pluginCred->pwszUserName = _wcsdup(savedCred->pUserInformation->pwszName);
            pluginCred->pwszUserDisplayName = _wcsdup(savedCred->pUserInformation->pwszDisplayName);
            pluginCred->cbUserId = savedCred->pUserInformation->cbId;
            pluginCred->pbUserId = wil::make_unique_nothrow<BYTE[]>(pluginCred->cbUserId).release();
            memcpy_s(pluginCred->pbUserId, pluginCred->cbUserId, savedCred->pUserInformation->pbId, savedCred->pUserInformation->cbId);
            pluginCred->cbCredentialId = savedCred->cbCredentialID;
            pluginCred->pbCredentialId = wil::make_unique_nothrow<BYTE[]>(pluginCred->cbCredentialId).release();
            memcpy_s(pluginCred->pbCredentialId, pluginCred->cbCredentialId, savedCred->pbCredentialID, savedCred->cbCredentialID);
            credentialDetailList.push_back(std::move(pluginCred));
        }

        RETURN_HR_IF(E_FAIL, credentialDetailList.empty());
        WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS_LIST pluginCredentialList;
        pluginCredentialList.cCredentialDetails = static_cast<DWORD>(credentialDetailList.size());
        auto credentialDetailPtrList = std::vector<PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS>{};
        for (auto& credential : credentialDetailList)
        {
            credentialDetailPtrList.push_back(credential.get());
        }
        pluginCredentialList.pCredentialDetails = credentialDetailPtrList.data();
        auto pluginClsId = wil::make_unique_string<wil::unique_cotaskmem_string>(c_pluginClsid);
        pluginCredentialList.pwszPluginClsId = pluginClsId.get();
        RETURN_IF_FAILED(webAuthNPluginAuthenticatorAddCredentials(&pluginCredentialList));
        return S_OK;
    }

    HRESULT PluginCredentialManager::AddPluginCredentialById(const std::vector<std::vector<UINT8>>& credentialIdList)
    {
        RETURN_HR_IF(E_FAIL, credentialIdList.empty());

        // Get the function pointer of WebAuthNPluginAuthenticatorAddCredentials
        auto webAuthNPluginAuthenticatorAddCredentials = GetProcAddressByFunctionDeclaration(m_webAuthnDll.get(), WebAuthNPluginAuthenticatorAddCredentials);
        RETURN_HR_IF_NULL(E_FAIL, webAuthNPluginAuthenticatorAddCredentials);

        std::vector<unique_plugin_credential_details> credentialDetailList{};
        for (auto& credentialId : credentialIdList)
        {
            std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
            auto iter = m_pluginLocalCredentialMetadataMap.find(credentialId);
            if (iter != m_pluginLocalCredentialMetadataMap.end())
            {
                auto& savedCred = iter->second;
                unique_plugin_credential_details pluginCred (new WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS ());
                pluginCred->pwszRpId = _wcsdup(savedCred->pRpInformation->pwszId);
                pluginCred->pwszRpName = _wcsdup(savedCred->pRpInformation->pwszName);
                pluginCred->pwszUserName = _wcsdup(savedCred->pUserInformation->pwszName);
                pluginCred->pwszUserDisplayName = _wcsdup(savedCred->pUserInformation->pwszDisplayName);
                pluginCred->cbUserId = savedCred->pUserInformation->cbId;
                pluginCred->pbUserId = wil::make_unique_nothrow<BYTE[]>(pluginCred->cbUserId).release();
                memcpy_s(pluginCred->pbUserId, pluginCred->cbUserId, savedCred->pUserInformation->pbId, savedCred->pUserInformation->cbId);
                pluginCred->cbCredentialId = savedCred->cbCredentialID;
                pluginCred->pbCredentialId = wil::make_unique_nothrow<BYTE[]>(pluginCred->cbCredentialId).release();
                memcpy_s(pluginCred->pbCredentialId, pluginCred->cbCredentialId, savedCred->pbCredentialID, savedCred->cbCredentialID);
                credentialDetailList.push_back(std::move(pluginCred));
            }
        }

        if (!credentialDetailList.empty())
        {
            WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS_LIST pluginCredentialList;
            pluginCredentialList.cCredentialDetails = static_cast<DWORD>(credentialDetailList.size());
            std::vector<PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS> credentialDetailPtrList{};
            for (auto& credential : credentialDetailList)
            {
                credentialDetailPtrList.push_back(credential.get());
            }
            pluginCredentialList.pCredentialDetails = credentialDetailPtrList.data();
            auto pluginClsId = wil::make_unique_string<wil::unique_cotaskmem_string>(c_pluginClsid);
            pluginCredentialList.pwszPluginClsId = pluginClsId.get();
            RETURN_IF_FAILED(webAuthNPluginAuthenticatorAddCredentials(&pluginCredentialList));
        }
        return S_OK;
    }

    HRESULT PluginCredentialManager::DeleteAllPluginCredentials()
    {
        // Get the function pointer of WebAuthNPluginAuthenticatorAddCredentials
        auto webAuthNPluginAuthenticatorRemoveAllCredentials = GetProcAddressByFunctionDeclaration(
            m_webAuthnDll.get(),
            WebAuthNPluginAuthenticatorRemoveAllCredentials);
        RETURN_HR_IF_NULL(E_FAIL, webAuthNPluginAuthenticatorRemoveAllCredentials);

        // Convert c_pluginClsid to a CLSID
        CLSID CLSID_PluginAuthenticator;
        RETURN_IF_FAILED(CLSIDFromString(c_pluginClsid, &CLSID_PluginAuthenticator));

        RETURN_HR(webAuthNPluginAuthenticatorRemoveAllCredentials(CLSID_PluginAuthenticator));
    }

    HRESULT PluginCredentialManager::DeletePluginCredentialById(std::vector<std::vector<UINT8>> const& credentialIdList, bool deleteEverywhere)
    {
        RETURN_HR_IF(E_FAIL, credentialIdList.empty());

        // Get the function pointer of WebAuthNPluginAddAuthenticator
        auto webAuthNPluginRemoveCredentials = GetProcAddressByFunctionDeclaration(
                m_webAuthnDll.get(),
                WebAuthNPluginAuthenticatorRemoveCredentials);
        RETURN_HR_IF_NULL(E_FAIL, webAuthNPluginRemoveCredentials);

        std::vector<unique_plugin_credential_details> credentialDetailList{};

        for (auto& credentialId : credentialIdList)
        {
            std::lock_guard<std::mutex> lock(m_pluginCachedCredentialsOperationMutex);
            auto savedCred = m_pluginCachedCredentialMetadataMap.find(credentialId);
            if (savedCred != m_pluginCachedCredentialMetadataMap.end())
            {
                auto& credToDelete = savedCred->second;
                credentialDetailList.push_back(std::move(credToDelete));
                m_pluginCachedCredentialMetadataMap.erase(savedCred);
            }
        }

        if (!credentialDetailList.empty())
        {
            WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS_LIST pluginCredentialList;
            pluginCredentialList.cCredentialDetails = static_cast<DWORD>(credentialDetailList.size());
            std::vector<PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS> credentialDetailPtrList{};
            for (auto& credential : credentialDetailList)
            {
                credentialDetailPtrList.push_back(credential.get());
            }
            pluginCredentialList.pCredentialDetails = credentialDetailPtrList.data();
            auto pluginClsId = wil::make_unique_string<wil::unique_cotaskmem_string>(c_pluginClsid);
            pluginCredentialList.pwszPluginClsId = pluginClsId.get();
            RETURN_IF_FAILED(webAuthNPluginRemoveCredential(&pluginCredentialList));
        }

        if (deleteEverywhere)
        {
            for (auto& credentialId : credentialIdList)
            {
                {
                    std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
                    m_pluginLocalCredentialMetadataMap.erase(credentialId);
                }
            }
            RecreateCredentialMetadataFile();
        }
        return S_OK;
    }

    HRESULT PluginCredentialManager::RefreshAutofillPluginCredentialsList()
    {
        std::lock_guard<std::mutex> lock(m_pluginCachedCredentialsOperationMutex);
        m_cachedCredentialsLoaded = false;
        m_pluginCachedCredentialMetadataMap.clear();
        // Get the function pointer of WebAuthNPluginAuthenticatorGetAllCredentials
        auto webAuthNPluginAuthenticatorGetAllCredentials = GetProcAddressByFunctionDeclaration(m_webAuthnDll.get(),
            WebAuthNPluginAuthenticatorGetAllCredentials);
        RETURN_HR_IF_NULL(E_FAIL, webAuthNPluginAuthenticatorGetAllCredentials);

        PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS_LIST localCredentialDetailsList = nullptr;
        HRESULT hr = webAuthNPluginAuthenticatorGetAllCredentials(c_pluginClsid, &localCredentialDetailsList);
        RETURN_HR_IF_EXPECTED(S_OK, hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

        for (DWORD i = 0; i < localCredentialDetailsList->cCredentialDetails; i++)
        {
            PWEBAUTHN_PLUGIN_CREDENTIAL_DETAILS credentialDetailsPtr = localCredentialDetailsList->pCredentialDetails[i];
            std::vector<UINT8> credentialId(credentialDetailsPtr->pbCredentialId, credentialDetailsPtr->pbCredentialId + credentialDetailsPtr->cbCredentialId);

            // Create a copy of the credential details
            unique_plugin_credential_details credentialDetailsCopy(new WEBAUTHN_PLUGIN_CREDENTIAL_DETAILS());
            credentialDetailsCopy->cbCredentialId = credentialDetailsPtr->cbCredentialId;
            credentialDetailsCopy->pbCredentialId = wil::make_unique_nothrow<BYTE[]>(credentialDetailsPtr->cbCredentialId).release();
            if (credentialDetailsCopy->pbCredentialId == nullptr || credentialDetailsPtr->cbCredentialId == 0 || credentialDetailsPtr->pbCredentialId == nullptr)
            {
                continue;
            }
            memcpy_s(credentialDetailsCopy->pbCredentialId,
                credentialDetailsCopy->cbCredentialId,
                credentialDetailsPtr->pbCredentialId,
                credentialDetailsPtr->cbCredentialId);
            credentialDetailsCopy->pwszRpId = _wcsdup(credentialDetailsPtr->pwszRpId);
            credentialDetailsCopy->pwszRpName = _wcsdup(credentialDetailsPtr->pwszRpName);
            credentialDetailsCopy->cbUserId = credentialDetailsPtr->cbUserId;
            credentialDetailsCopy->pbUserId = wil::make_unique_nothrow<BYTE[]>(credentialDetailsPtr->cbUserId).release();
            if (credentialDetailsCopy->pbUserId == nullptr || credentialDetailsPtr->cbUserId == 0 || credentialDetailsPtr->pbUserId == nullptr)
            {
                continue;
            }
            memcpy_s(
                credentialDetailsCopy->pbUserId,
                credentialDetailsCopy->cbUserId,
                credentialDetailsPtr->pbUserId,
                credentialDetailsPtr->cbUserId);
            credentialDetailsCopy->pwszUserName = _wcsdup(credentialDetailsPtr->pwszUserName);
            credentialDetailsCopy->pwszUserDisplayName = _wcsdup(credentialDetailsPtr->pwszUserDisplayName);

            // Insert the copy into the map
            m_pluginCachedCredentialMetadataMap.emplace(credentialId, std::move(credentialDetailsCopy));
        }
        m_cachedCredentialsLoaded = true;
        return S_OK;
    }

    /*
    * Helper function to prepare the DataModel to diplay the list of credentials
    */
    std::vector<winrt::com_ptr<Credential>> PluginCredentialManager::GetCredentialListViewModel()
    {
        std::vector< winrt::com_ptr<Credential>> credentialViewList;
        for (const auto& mapItem : m_pluginLocalCredentialMetadataMap)
        {
            auto& savedCredential = mapItem.second;
            auto& savedCredentialId = mapItem.first;
            auto credentialOptions = CredentialOptionFlags::MetadataValid;

            if (IsPluginCredentialIdAutofillSupported(savedCredential->cbCredentialID, savedCredential->pbCredentialID))
            {
                credentialOptions |= CredentialOptionFlags::AutofillCapable;
            }
            auto writer = winrt::Windows::Storage::Streams::DataWriter();
            auto credIdSpan = std::span(savedCredentialId);
            writer.WriteBytes(credIdSpan);
            auto credentialIdBuffer = writer.DetachBuffer();

            auto credentialViewListItem = winrt::make_self<PasskeyManager::implementation::Credential>(
                savedCredential->pUserInformation->pwszName, savedCredential->pRpInformation->pwszName, credentialIdBuffer, credentialOptions);
            credentialViewList.emplace_back(credentialViewListItem);
        }
        for (const auto& mapItem : m_pluginCachedCredentialMetadataMap)
        {
            const auto& cachedCredential = mapItem.second;
            const auto& cachedCredentialId = mapItem.first;
            bool isValidCredential = m_pluginLocalCredentialMetadataMap.find(cachedCredentialId) != m_pluginLocalCredentialMetadataMap.end();
            if (isValidCredential)
            {
                // Skip the credentials that are already in the local credential list
                continue;
            }
            auto credentialOptions = CredentialOptionFlags::AutofillCapable;
            auto buffer = winrt::Windows::Storage::Streams::DataWriter();
            auto credIdSpan = std::span(cachedCredentialId);
            buffer.WriteBytes(credIdSpan);
            auto credentialIdBuffer = buffer.DetachBuffer();
            auto credentialViewListItem = winrt::make_self<PasskeyManager::implementation::Credential>(
                cachedCredential->pwszUserName, cachedCredential->pwszRpName, credentialIdBuffer, credentialOptions);
            credentialViewList.emplace_back(credentialViewListItem);
        }
        return credentialViewList;
    }

    bool PluginCredentialManager::IsPluginCredentialIdAutofillSupported(DWORD cbCredentialId, PBYTE pbCredentialId) const
    {
        return m_pluginCachedCredentialMetadataMap.find(
            std::vector<UINT8>(pbCredentialId, pbCredentialId + cbCredentialId)) !=
                m_pluginCachedCredentialMetadataMap.end();
    }

    DWORD readBytesFromFile(std::ifstream& file, PBYTE* buffer, const DWORD maxSize)
    {
        DWORD bytesToRead = 0;
        if (file.is_open() && buffer != nullptr)
        {
            while (!file.eof())
            {
                file.read((char*)(&bytesToRead), sizeof(DWORD));
                if (maxSize && bytesToRead > maxSize)
                {
                    return 0;
                }
                if (bytesToRead == 0)
                {
                    return 0;
                }
                if (*buffer == nullptr)
                {
                    // allocate memory for the buffer
                    *buffer = wil::make_unique_nothrow<BYTE[]>(bytesToRead).release();
                    memset(*buffer, 0, bytesToRead);
                }
                file.read((char*)*buffer, bytesToRead);
                return bytesToRead;
            }
        }
        return 0;
    }


    bool PluginCredentialManager::LoadSavedCredentialsFromMockDatabase()
    {
        std::ifstream file{};
        std::wstring filePath;
        if (!GetCredentialStorageFilePath(filePath))
        {
            return false;
        }
        // open file in binary mode
        file.open(filePath, std::ifstream::in | std::ifstream::binary);
        if (file.is_open())
        {
            std::lock_guard<std::mutex> lock(m_pluginLocalCredentialsOperationMutex);
            m_localCredentialsLoaded = false;
            m_pluginLocalCredentialMetadataMap.clear();
            while (!file.eof())
            {
                unique_credential_details savedCredential(new WEBAUTHN_CREDENTIAL_DETAILS());
                //WEBAUTHN_CREDENTIAL_DETAILS savedCredential{};
                savedCredential->pUserInformation = wil::make_unique_nothrow<WEBAUTHN_USER_ENTITY_INFORMATION>().release();
                savedCredential->pRpInformation = wil::make_unique_nothrow<WEBAUTHN_RP_ENTITY_INFORMATION>().release();

                if (savedCredential->pUserInformation == nullptr || savedCredential->pRpInformation == nullptr)
                {
                    break;
                }

                // read the user id
                savedCredential->pUserInformation->pbId = nullptr;
                savedCredential->pUserInformation->cbId = 0;
                savedCredential->pUserInformation->cbId = readBytesFromFile(file, &savedCredential->pUserInformation->pbId, savedCredential->pUserInformation->cbId);
                if (savedCredential->pUserInformation->cbId == 0)
                {
                    break;
                }
                // read the username and display name
                PBYTE buffer = nullptr;
                buffer = wil::make_unique_nothrow<BYTE[]>(maxWebAuthnWStringSize).release();
                readBytesFromFile(file, &buffer, maxWebAuthnWStringSize);
                savedCredential->pUserInformation->pwszName = (WCHAR*)buffer;
                buffer = wil::make_unique_nothrow<BYTE[]>(maxWebAuthnWStringSize).release();
                readBytesFromFile(file, &buffer, maxWebAuthnWStringSize);
                savedCredential->pUserInformation->pwszDisplayName = (WCHAR*)buffer;
                buffer = nullptr;

                // read credential id
                savedCredential->pbCredentialID = nullptr;
                savedCredential->cbCredentialID = 0;
                savedCredential->cbCredentialID = readBytesFromFile(file, &savedCredential->pbCredentialID, savedCredential->cbCredentialID);

                // read rp name and id
                buffer = wil::make_unique_nothrow<BYTE[]>(maxWebAuthnWStringSize).release();
                readBytesFromFile(file, &buffer, maxWebAuthnWStringSize);
                savedCredential->pRpInformation->pwszId = (WCHAR*)buffer;
                buffer = wil::make_unique_nothrow<BYTE[]>(maxWebAuthnWStringSize).release();
                readBytesFromFile(file, &buffer, maxWebAuthnWStringSize);
                savedCredential->pRpInformation->pwszName = (WCHAR*)buffer;
                buffer = nullptr;
                std::vector<UINT8> localCredId(savedCredential->pbCredentialID, savedCredential->pbCredentialID + savedCredential->cbCredentialID);
                m_pluginLocalCredentialMetadataMap.emplace(localCredId, std::move(savedCredential));
            }
            file.close();
            m_localCredentialsLoaded = true;
        }
        return true;
    }

    bool PluginCredentialManager::GetCredentialStorageFilePath(std::wstring& filePath)
    {

        Windows::Storage::ApplicationData appData = Windows::Storage::ApplicationData::Current();
        Windows::Storage::StorageFolder localFolder = appData.LocalFolder();
        std::wstring localFolderPath = localFolder.Path().c_str();
        std::wstring directoryPath = localFolderPath + L"\\" + c_pluginLocalAppDataDBDir;

        if (!std::filesystem::exists(directoryPath))
        {
            THROW_IF_WIN32_BOOL_FALSE(std::filesystem::create_directory(directoryPath) ? TRUE: FALSE);
        }

        filePath = directoryPath + L"\\" + c_credentialsFileName;
        return true;
    }

    bool PluginCredentialManager::SaveCredentialMetadataToMockDB(const WEBAUTHN_CREDENTIAL_DETAILS& newCredential)
    {
        if (newCredential.cbCredentialID == 0 || newCredential.pbCredentialID == nullptr)
        {
            return true;
        }

        std::wstring filePath;
        if (!GetCredentialStorageFilePath(filePath))
        {
            return false;
        }

        std::ofstream file{};
        file.open(filePath, std::ofstream::out | std::ofstream::binary | std::ofstream::app);
        if (file.is_open())
        {
            DWORD len = newCredential.pUserInformation->cbId;
            file.write((const char*)(&len), sizeof(DWORD));
            file.write((const char*)newCredential.pUserInformation->pbId, len);

            len = static_cast<DWORD>(wcslen(newCredential.pUserInformation->pwszName));
            len *= sizeof(WCHAR);
            file.write((const char*)(&len), sizeof(DWORD));
            file.write((const char*)newCredential.pUserInformation->pwszName, len);
            len = static_cast<DWORD>(wcslen(newCredential.pUserInformation->pwszDisplayName));
            len *= sizeof(WCHAR);
            file.write((const char*)(&len), sizeof(DWORD));
            file.write((const char*)newCredential.pUserInformation->pwszDisplayName, len);

            len = newCredential.cbCredentialID;
            file.write((const char*)(&len), sizeof(DWORD));
            file.write((const char*)newCredential.pbCredentialID, len);

            len = static_cast<DWORD>(wcslen(newCredential.pRpInformation->pwszId));
            len *= sizeof(WCHAR);
            file.write((const char*)(&len), sizeof(DWORD));
            file.write((const char*)newCredential.pRpInformation->pwszId, len);
            len = static_cast<DWORD>(wcslen(newCredential.pRpInformation->pwszName));
            len *= sizeof(WCHAR);
            file.write((const char*)(&len), sizeof(DWORD));
            file.write((const char*)newCredential.pRpInformation->pwszName, len);

            file.close();
        }
        else
        {
            return false;
        }
        return true;
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

}
