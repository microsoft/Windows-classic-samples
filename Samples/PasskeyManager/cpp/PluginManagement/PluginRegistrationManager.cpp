#include "pch.h"
#include "MainPage.xaml.h"
#include "PluginRegistrationManager.h"
#include <CorError.h>
#include <wil/safecast.h>

namespace winrt::PasskeyManager::implementation {
    PluginRegistrationManager::PluginRegistrationManager() :
        m_pluginRegistered(false),
        m_initialized(false),
        // AUTHENTICATOR_STATE: Enum representing the state of a plugin authenticator in the Windows
        // third-party passkey plugin system. This state indicates whether the plugin is enabled or disabled.
        m_pluginState(AUTHENTICATOR_STATE::AuthenticatorState_Disabled)
    {
        Initialize();
    }

    PluginRegistrationManager::~PluginRegistrationManager()
    {
    }

    HRESULT PluginRegistrationManager::Initialize()
    {
        HRESULT hr = RefreshPluginState();
        RETURN_HR_IF_EXPECTED(S_OK, hr == NTE_NOT_FOUND);
        ReloadRegistryValues();
        RETURN_HR(hr);
    }

    HRESULT PluginRegistrationManager::RegisterPlugin()
    {
        /*
        * This section creates a sample authenticatorInfo blob to include in the registration
        * request. This blob must CBOR encoded using the format defined
        * in https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html#authenticatorGetInfo
        *
        * 'AAGUID' maybe used to fetch information about the authenticator from the FIDO Metadata Service and other sources.
        * Refer: https://fidoalliance.org/metadata/
        *
        * 'extensions' field is used to perform feature detection on the authenticator
        * and maybe used to determine if the authenticator is filtered out.
        */
        std::string tempAaguidStr{ c_pluginAaguidString };
        tempAaguidStr.erase(std::remove(tempAaguidStr.begin(), tempAaguidStr.end(), L'-'), tempAaguidStr.end());
        std::transform(tempAaguidStr.begin(), tempAaguidStr.end(), tempAaguidStr.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        // The following hex strings represent the encoding of
        // {1: ["FIDO_2_0", "FIDO_2_1"], 2: ["prf", "hmac-secret"], 3: h'/* AAGUID */', 4: {"rk": true, "up": true, "uv": true}, 
        // 9: ["internal"], 10: [{"alg": -7, "type": "public-key"}]}
        std::string authenticatorInfoStrPart1 = "A60182684649444F5F325F30684649444F5F325F310282637072666B686D61632D7365637265740350";
        std::string authenticatorInfoStrPart2 = "04A362726BF5627570F5627576F5098168696E7465726E616C0A81A263616C672664747970656A7075626C69632D6B6579";
        std::string fullAuthenticatorInfoStr = authenticatorInfoStrPart1 + tempAaguidStr + authenticatorInfoStrPart2;
        std::vector<BYTE> authenticatorInfo = hexStringToBytes(fullAuthenticatorInfoStr);

        // WEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_OPTIONS: Structure containing options for registering a plugin authenticator
        // with the Windows platform. This includes authenticator name, class ID, supported RP IDs, logo data, and
        // CBOR-encoded authenticator information for FIDO compliance.
        WEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_OPTIONS addOptions{
            .pwszAuthenticatorName = c_pluginName,
            .rclsid = contosoplugin_guid,
            .pwszPluginRpId = c_pluginRpId,
            .pwszLightThemeLogoSvg = L"PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSIgdmlld0JveD0iMzAgMCA1MCA4NSIgc3R5bGU9ImZpbGwtcnVsZTpldmVub2RkOyBjbGlwLXJ1bGU6ZXZlbm9kZDsgc2hhcGUtcmVuZGVyaW5nOmdlb21ldHJpY1ByZWNpc2lvbjsgdGV4dC1yZW5kZXJpbmc6Z2VvbWV0cmljUHJlY2lzaW9uOyBpbWFnZS1yZW5kZXJpbmc6b3B0aW1pemVRdWFsaXR5OyI+PGRlZnM+PGxpbmVhckdyYWRpZW50IGlkPSJncmFkMSIgeDE9IjAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIwJSIgc3R5bGU9InN0b3AtY29sb3I6IzRiZTBmYzsgc3RvcC1vcGFjaXR5OjEiIC8+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjxsaW5lYXJHcmFkaWVudCBpZD0iZ3JhZDIiIHgxPSIxMDAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojNGJlMGZjOyBzdG9wLW9wYWNpdHk6MSIgLz48c3RvcCBvZmZzZXQ9IjAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjwvZGVmcz48Zz48cG9seWdvbiBwb2ludHM9IjQ4LDI0IDU4LDM2IDQ0LDY3IDMyLDYwIiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjMyLDYwIDQ0LDY3IDMyLjk0LDY4Ljg5IiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjQ0LDY3IDQ3LjE1LDYwIDQ4LDY1LjUiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxwb2x5Z29uIHBvaW50cz0iNDcuMTUsNjAgNTAuMzAsNTMgNTEuMTUsNTguNSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNTUiIGN5PSIyNSIgcj0iMTgiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxjaXJjbGUgY3g9IjcyIiBjeT0iMjUiIHI9IjE4IiBmaWxsPSJ3aGl0ZSIgLz48L2c+PGc+PHJlY3QgeD0iNzAiIHk9IjMwIiB3aWR0aD0iMTYiIGhlaWdodD0iNDUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iNzgsODEgNzAsNzUgODYsNzUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iODYsNjcgODYsNzUgODguNSw3MSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PHBvbHlnb24gcG9pbnRzPSI4Niw2NyA4Niw1OSA4OC41LDYzIiBmaWxsPSJ1cmwoI2dyYWQxKSIgLz48Y2lyY2xlIGN4PSI3NyIgY3k9IjI1IiByPSIxOCIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNzciIGN5PSIyMyIgcj0iMyIgZmlsbD0id2hpdGUiIC8+PC9nPjwvc3ZnPg==",
            .pwszDarkThemeLogoSvg = L"PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSIgdmlld0JveD0iMzAgMCA1MCA4NSIgc3R5bGU9ImZpbGwtcnVsZTpldmVub2RkOyBjbGlwLXJ1bGU6ZXZlbm9kZDsgc2hhcGUtcmVuZGVyaW5nOmdlb21ldHJpY1ByZWNpc2lvbjsgdGV4dC1yZW5kZXJpbmc6Z2VvbWV0cmljUHJlY2lzaW9uOyBpbWFnZS1yZW5kZXJpbmc6b3B0aW1pemVRdWFsaXR5OyI+PGRlZnM+PGxpbmVhckdyYWRpZW50IGlkPSJncmFkMSIgeDE9IjAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIwJSIgc3R5bGU9InN0b3AtY29sb3I6IzRiZTBmYzsgc3RvcC1vcGFjaXR5OjEiIC8+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjxsaW5lYXJHcmFkaWVudCBpZD0iZ3JhZDIiIHgxPSIxMDAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojNGJlMGZjOyBzdG9wLW9wYWNpdHk6MSIgLz48c3RvcCBvZmZzZXQ9IjAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjwvZGVmcz48Zz48cG9seWdvbiBwb2ludHM9IjQ4LDI0IDU4LDM2IDQ0LDY3IDMyLDYwIiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjMyLDYwIDQ0LDY3IDMyLjk0LDY4Ljg5IiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjQ0LDY3IDQ3LjE1LDYwIDQ4LDY1LjUiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxwb2x5Z29uIHBvaW50cz0iNDcuMTUsNjAgNTAuMzAsNTMgNTEuMTUsNTguNSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNTUiIGN5PSIyNSIgcj0iMTgiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxjaXJjbGUgY3g9IjcyIiBjeT0iMjUiIHI9IjE4IiBmaWxsPSJ3aGl0ZSIgLz48L2c+PGc+PHJlY3QgeD0iNzAiIHk9IjMwIiB3aWR0aD0iMTYiIGhlaWdodD0iNDUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iNzgsODEgNzAsNzUgODYsNzUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iODYsNjcgODYsNzUgODguNSw3MSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PHBvbHlnb24gcG9pbnRzPSI4Niw2NyA4Niw1OSA4OC41LDYzIiBmaWxsPSJ1cmwoI2dyYWQxKSIgLz48Y2lyY2xlIGN4PSI3NyIgY3k9IjI1IiByPSIxOCIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNzciIGN5PSIyMyIgcj0iMyIgZmlsbD0id2hpdGUiIC8+PC9nPjwvc3ZnPg==",
            .cbAuthenticatorInfo = static_cast<DWORD>(authenticatorInfo.size()),
            .pbAuthenticatorInfo = authenticatorInfo.data(),
            .cSupportedRpIds = 0,
            .ppwszSupportedRpIds = nullptr
        };

        // PWEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_RESPONSE: Response structure returned by the plugin registration API, containing
        // the operation signing public key that will be used to verify signed plugin operation requests from the platform.
        PWEBAUTHN_PLUGIN_ADD_AUTHENTICATOR_RESPONSE addResponse;

        // Call the plugin registration API
        RETURN_IF_FAILED(WebAuthNPluginAddAuthenticator(&addOptions, &addResponse));

        // Ensure the response is freed when it goes out of scope
        auto cleanup = wil::scope_exit([&] {
            WebAuthNPluginFreeAddAuthenticatorResponse(addResponse);
        });

        // The response from plugin contains the public key used to sign plugin operation requests. Stash it for later use.
        wil::unique_hkey hKey;
        RETURN_IF_WIN32_ERROR(RegCreateKeyEx(
            HKEY_CURRENT_USER,
            c_pluginRegistryPath,
            0,
            nullptr,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            nullptr,
            &hKey,
            nullptr));

        RETURN_IF_WIN32_ERROR(RegSetValueEx(
            hKey.get(),
            c_windowsPluginRequestSigningKeyRegKeyName,
            0,
            REG_BINARY,
            addResponse->pbOpSignPubKey,
            addResponse->cbOpSignPubKey));
        return S_OK;
    }

    HRESULT PluginRegistrationManager::UnregisterPlugin()
    {
        // Call the plugin unregistration API with the plugin's class ID
        RETURN_HR(WebAuthNPluginRemoveAuthenticator(contosoplugin_guid));
    }

    HRESULT PluginRegistrationManager::UpdatePlugin()
    {
        /*
        * This section creates a sample authenticatorInfo blob to include in the registration
        * request. This blob must CBOR encoded using the format defined
        * in https://fidoalliance.org/specs/fido-v2.0-ps-20190130/fido-client-to-authenticator-protocol-v2.0-ps-20190130.html#authenticatorGetInfo
        *
        * 'AAGUID' maybe used to fetch information about the authenticator from the FIDO Metadata Service and other sources.
        * Refer: https://fidoalliance.org/metadata/
        *
        * 'extensions' field is used to perform feature detection on the authenticator
        * and maybe used to determine if the authenticator is filtered out by the platform during web authentication
        * if the client has requested specific support for a extension that the authenticator does not support.
        */
        std::string tempAaguidStr{ c_pluginAaguidString };
        tempAaguidStr.erase(std::remove(tempAaguidStr.begin(), tempAaguidStr.end(), L'-'), tempAaguidStr.end());
        std::transform(tempAaguidStr.begin(), tempAaguidStr.end(), tempAaguidStr.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        // The following hex strings represent the encoding of
        // {1: ["FIDO_2_0", "FIDO_2_1"], 2: ["prf", "hmac-secret"], 3: h'/* AAGUID */', 4: {"rk": true, "up": true, "uv": true}, 
        // 9: ["internal"], 10: [{"alg": -7, "type": "public-key"}]}
        std::string authenticatorInfoStrPart1 = "A60182684649444F5F325F30684649444F5F325F310282637072666B686D61632D7365637265740350";
        std::string authenticatorInfoStrPart2 = "04A362726BF5627570F5627576F5098168696E7465726E616C0A81A263616C672664747970656A7075626C69632D6B6579";
        std::string fullAuthenticatorInfoStr = authenticatorInfoStrPart1 + tempAaguidStr + authenticatorInfoStrPart2;
        std::vector<BYTE> authenticatorInfo = hexStringToBytes(fullAuthenticatorInfoStr);

        // WEBAUTHN_PLUGIN_UPDATE_AUTHENTICATOR_DETAILS: Structure containing updated plugin information for an already
        // registered authenticator, including potentially new class IDs, names, logos, and authenticator information.
        WEBAUTHN_PLUGIN_UPDATE_AUTHENTICATOR_DETAILS updateDetails{
            .pwszAuthenticatorName = c_pluginName,
            .rclsid = contosoplugin_guid,
            .rclsidNew = contosoplugin_guid,
            .pwszLightThemeLogoSvg = L"PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSIgd2lkdGg9IjkwcHgiIGhlaWdodD0iOTBweCIgdmlld0JveD0iMzAgMCA1MCA4NSIgc3R5bGU9ImZpbGwtcnVsZTpldmVub2RkOyBjbGlwLXJ1bGU6ZXZlbm9kZDsgc2hhcGUtcmVuZGVyaW5nOmdlb21ldHJpY1ByZWNpc2lvbjsgdGV4dC1yZW5kZXJpbmc6Z2VvbWV0cmljUHJlY2lzaW9uOyBpbWFnZS1yZW5kZXJpbmc6b3B0aW1pemVRdWFsaXR5OyI+PGRlZnM+PGxpbmVhckdyYWRpZW50IGlkPSJncmFkMSIgeDE9IjAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIwJSIgc3R5bGU9InN0b3AtY29sb3I6IzRiZTBmYzsgc3RvcC1vcGFjaXR5OjEiIC8+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjxsaW5lYXJHcmFkaWVudCBpZD0iZ3JhZDIiIHgxPSIxMDAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojNGJlMGZjOyBzdG9wLW9wYWNpdHk6MSIgLz48c3RvcCBvZmZzZXQ9IjAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjwvZGVmcz48Zz48cG9seWdvbiBwb2ludHM9IjQ4LDI0IDU4LDM2IDQ0LDY3IDMyLDYwIiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjMyLDYwIDQ0LDY3IDMyLjk0LDY4Ljg5IiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjQ0LDY3IDQ3LjE1LDYwIDQ4LDY1LjUiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxwb2x5Z29uIHBvaW50cz0iNDcuMTUsNjAgNTAuMzAsNTMgNTEuMTUsNTguNSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNTUiIGN5PSIyNSIgcj0iMTgiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxjaXJjbGUgY3g9IjcyIiBjeT0iMjUiIHI9IjE4IiBmaWxsPSJ3aGl0ZSIgLz48L2c+PGc+PHJlY3QgeD0iNzAiIHk9IjMwIiB3aWR0aD0iMTYiIGhlaWdodD0iNDUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iNzgsODEgNzAsNzUgODYsNzUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iODYsNjcgODYsNzUgODguNSw3MSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PHBvbHlnb24gcG9pbnRzPSI4Niw2NyA4Niw1OSA4OC41LDYzIiBmaWxsPSJ1cmwoI2dyYWQxKSIgLz48Y2lyY2xlIGN4PSI3NyIgY3k9IjI1IiByPSIxOCIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNzciIGN5PSIyMyIgcj0iMyIgZmlsbD0id2hpdGUiIC8+PC9nPjwvc3ZnPg==",
            .pwszDarkThemeLogoSvg = L"PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSIgd2lkdGg9IjkwcHgiIGhlaWdodD0iOTBweCIgdmlld0JveD0iMzAgMCA1MCA4NSIgc3R5bGU9ImZpbGwtcnVsZTpldmVub2RkOyBjbGlwLXJ1bGU6ZXZlbm9kZDsgc2hhcGUtcmVuZGVyaW5nOmdlb21ldHJpY1ByZWNpc2lvbjsgdGV4dC1yZW5kZXJpbmc6Z2VvbWV0cmljUHJlY2lzaW9uOyBpbWFnZS1yZW5kZXJpbmc6b3B0aW1pemVRdWFsaXR5OyI+PGRlZnM+PGxpbmVhckdyYWRpZW50IGlkPSJncmFkMSIgeDE9IjAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIwJSIgc3R5bGU9InN0b3AtY29sb3I6IzRiZTBmYzsgc3RvcC1vcGFjaXR5OjEiIC8+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjxsaW5lYXJHcmFkaWVudCBpZD0iZ3JhZDIiIHgxPSIxMDAlIiB5MT0iMTAwJSIgeDI9IjEwMCUiIHkyPSIwJSI+PHN0b3Agb2Zmc2V0PSIxMDAlIiBzdHlsZT0ic3RvcC1jb2xvcjojNGJlMGZjOyBzdG9wLW9wYWNpdHk6MSIgLz48c3RvcCBvZmZzZXQ9IjAlIiBzdHlsZT0ic3RvcC1jb2xvcjojMTY5NmUxOyBzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjwvZGVmcz48Zz48cG9seWdvbiBwb2ludHM9IjQ4LDI0IDU4LDM2IDQ0LDY3IDMyLDYwIiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjMyLDYwIDQ0LDY3IDMyLjk0LDY4Ljg5IiBmaWxsPSJ1cmwoI2dyYWQyKSIgLz48cG9seWdvbiBwb2ludHM9IjQ0LDY3IDQ3LjE1LDYwIDQ4LDY1LjUiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxwb2x5Z29uIHBvaW50cz0iNDcuMTUsNjAgNTAuMzAsNTMgNTEuMTUsNTguNSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNTUiIGN5PSIyNSIgcj0iMTgiIGZpbGw9InVybCgjZ3JhZDEpIiAvPjxjaXJjbGUgY3g9IjcyIiBjeT0iMjUiIHI9IjE4IiBmaWxsPSJ3aGl0ZSIgLz48L2c+PGc+PHJlY3QgeD0iNzAiIHk9IjMwIiB3aWR0aD0iMTYiIGhlaWdodD0iNDUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iNzgsODEgNzAsNzUgODYsNzUiIGZpbGw9InVybCgjZ3JhZDIpIiAvPjxwb2x5Z29uIHBvaW50cz0iODYsNjcgODYsNzUgODguNSw3MSIgZmlsbD0idXJsKCNncmFkMSkiIC8+PHBvbHlnb24gcG9pbnRzPSI4Niw2NyA4Niw1OSA4OC41LDYzIiBmaWxsPSJ1cmwoI2dyYWQxKSIgLz48Y2lyY2xlIGN4PSI3NyIgY3k9IjI1IiByPSIxOCIgZmlsbD0idXJsKCNncmFkMSkiIC8+PGNpcmNsZSBjeD0iNzciIGN5PSIyMyIgcj0iMyIgZmlsbD0id2hpdGUiIC8+PC9nPjwvc3ZnPg==",
            .cbAuthenticatorInfo = static_cast<DWORD>(authenticatorInfo.size()),
            .pbAuthenticatorInfo = authenticatorInfo.data(),
            .cSupportedRpIds = 0,
            .ppwszSupportedRpIds = nullptr
        };

        // Call the plugin update API
        RETURN_IF_FAILED(WebAuthNPluginUpdateAuthenticatorDetails(&updateDetails));

        return S_OK;
    }

    HRESULT PluginRegistrationManager::RefreshPluginState()
    {
        // Reset the plugin state and registration status
        m_pluginRegistered = false;
        // Reset to disabled state
        m_pluginState = AUTHENTICATOR_STATE::AuthenticatorState_Disabled;

        // AUTHENTICATOR_STATE: Enum representing various operational
        // states of a plugin authenticator (enabled, disabled, etc.)
        AUTHENTICATOR_STATE localPluginState;

        // Query the platform for the current plugin state using the get state API
        RETURN_IF_FAILED(WebAuthNPluginGetAuthenticatorState(contosoplugin_guid, &localPluginState));

        // If the WebAuthNPluginGetAuthenticatorState function succeeded, that indicates the plugin is registered and localPluginState is the valid plugin state
        m_pluginRegistered = true;
        m_pluginState = localPluginState;
        return S_OK;
    }

    HRESULT PluginRegistrationManager::CreateVaultPasskey(HWND hWnd)
    {
        WEBAUTHN_GET_CREDENTIALS_OPTIONS webAuthNGetCredentialsOptions = {};
        webAuthNGetCredentialsOptions.dwVersion = WEBAUTHN_GET_CREDENTIALS_OPTIONS_CURRENT_VERSION;
        webAuthNGetCredentialsOptions.pwszRpId = c_pluginRpId;

        unique_webauthn_credential_details_list pCredentialDetailsList = nullptr;
        HRESULT hr = WebAuthNGetPlatformCredentialList(
            &webAuthNGetCredentialsOptions,
            &pCredentialDetailsList);
        if (SUCCEEDED(hr) && pCredentialDetailsList && pCredentialDetailsList.get()->cCredentialDetails > 0)
        {
            return NTE_EXISTS;
        }

        // populate the input structures
        WEBAUTHN_RP_ENTITY_INFORMATION rpEntity = {};
        rpEntity.dwVersion = WEBAUTHN_RP_ENTITY_INFORMATION_CURRENT_VERSION;
        rpEntity.pwszName = c_rpName;
        rpEntity.pwszId = c_pluginRpId;
        WEBAUTHN_USER_ENTITY_INFORMATION userEntity = {};
        userEntity.dwVersion = WEBAUTHN_USER_ENTITY_INFORMATION_CURRENT_VERSION;
        userEntity.pwszName = c_userName;
        userEntity.pwszDisplayName = c_userDisplayName;
        userEntity.pbId = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(c_userId));
        userEntity.cbId = static_cast<DWORD>(wcslen(c_userId) * sizeof(wchar_t));

        WEBAUTHN_COSE_CREDENTIAL_PARAMETER credentialParameter = {};
        credentialParameter.dwVersion = WEBAUTHN_COSE_CREDENTIAL_PARAMETER_CURRENT_VERSION;
        credentialParameter.pwszCredentialType = WEBAUTHN_CREDENTIAL_TYPE_PUBLIC_KEY;
        credentialParameter.lAlg = WEBAUTHN_COSE_ALGORITHM_ECDSA_P256_WITH_SHA256;

        WEBAUTHN_COSE_CREDENTIAL_PARAMETERS credentialParameters = {};
        credentialParameters.cCredentialParameters = 1;
        credentialParameters.pCredentialParameters = &credentialParameter;

        WEBAUTHN_CREDENTIAL rgExcludeCredential[] = { 0 }; // TestFree(.pbId)

        WEBAUTHN_CLIENT_DATA clientData = {};
        clientData.dwVersion = WEBAUTHN_CLIENT_DATA_CURRENT_VERSION;
        clientData.pwszHashAlgId = WEBAUTHN_HASH_ALGORITHM_SHA_256;
        std::wstring clientDataStr = L"{\"challenge\":\"eyJjaGFsbGVuZ2UiOiAiY2hhbGxlbmdlIn0\"}";
        clientData.pbClientDataJSON = reinterpret_cast<BYTE*>(const_cast<PWSTR>(clientDataStr.c_str()));
        clientData.cbClientDataJSON = static_cast<DWORD>(clientDataStr.size() * sizeof(wchar_t));

        WEBAUTHN_AUTHENTICATOR_MAKE_CREDENTIAL_OPTIONS webAuthNCredentialOptions = {
            WEBAUTHN_AUTHENTICATOR_MAKE_CREDENTIAL_OPTIONS_CURRENT_VERSION,
            180,          // dwTimeoutSeconds
            // WEBAUTHN_CREDENTIALS CredentialList
            {
                0,
                NULL
            },
            // WEBAUTHN_EXTENSIONS Extensions
            {
                0,
                NULL
            },
            WEBAUTHN_AUTHENTICATOR_ATTACHMENT_ANY,
            true,
            WEBAUTHN_USER_VERIFICATION_REQUIREMENT_REQUIRED,
            WEBAUTHN_ATTESTATION_CONVEYANCE_PREFERENCE_ANY,
            0,          // dwFlags
        };
        webAuthNCredentialOptions.bEnablePrf = true;

        std::array<uint8_t, WEBAUTHN_CTAP_ONE_HMAC_SECRET_LENGTH> prfInput{};

        RETURN_IF_NTSTATUS_FAILED(BCryptGenRandom(
            nullptr,
            prfInput.data(),
            wil::safe_cast<ULONG>(prfInput.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG));

        WEBAUTHN_HMAC_SECRET_SALT secretSalt = {};
        secretSalt.cbFirst = wil::safe_cast<DWORD>(prfInput.size());
        secretSalt.pbFirst = const_cast<uint8_t*>(prfInput.data());
        webAuthNCredentialOptions.pPRFGlobalEval = &secretSalt;

        unique_webauthn_credential_attestation pCredentialAttestation = nullptr;
        hr = WebAuthNAuthenticatorMakeCredential(
            hWnd,
            &rpEntity,
            &userEntity,
            &credentialParameters,
            &clientData,
            &webAuthNCredentialOptions,
            &pCredentialAttestation);
        if (SUCCEEDED(hr))
        {
            if (pCredentialAttestation.get()->pHmacSecret == nullptr)
            {
                return NTE_NOT_SUPPORTED; // The chosen authenticator does not support PRF.
            }

            RETURN_IF_FAILED(SetHMACSecret(std::vector<BYTE>(prfInput.begin(), prfInput.end())));
            DATA_BLOB vaultData = {
                .cbData = static_cast<DWORD>(wcslen(c_dummySecretVault) * sizeof(wchar_t)),
                .pbData = reinterpret_cast<BYTE*>(const_cast<PWSTR>(c_dummySecretVault))
            };
            DATA_BLOB entropy = {
                .cbData = pCredentialAttestation.get()->pHmacSecret->cbFirst,
                .pbData = pCredentialAttestation.get()->pHmacSecret->pbFirst
            };
            DATA_BLOB cipherText = {};
            RETURN_IF_WIN32_BOOL_FALSE(CryptProtectData(
                &vaultData,
                nullptr,
                &entropy,
                nullptr,
                nullptr,
                CRYPTPROTECT_UI_FORBIDDEN,
                &cipherText));

            // Use RAII to ensure cipherText.pbData is always freed, even on early returns
            auto cipherTextCleanup = wil::scope_exit([&] {
                if (cipherText.pbData)
                {
                    LocalFree(cipherText.pbData);
                }
            });

            RETURN_IF_FAILED(WriteEncryptedVaultData(std::vector<BYTE>(cipherText.pbData, cipherText.pbData + cipherText.cbData)));
        }
        return hr;
    }

    HRESULT PluginRegistrationManager::SetHMACSecret(std::vector<BYTE> hmacSecret)
    {
        // This function saves the random HMAC secret generated in plain text.
        // In a real application, the HMAC secret is either retrieved from the server or may be user supplied
        // or saved in encrypted form.
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        if (m_hmacSecret != hmacSecret)
        {
            wil::unique_hkey hKey;
            RETURN_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CURRENT_USER, c_pluginRegistryPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr));
            RETURN_IF_WIN32_ERROR(RegSetValueEx(hKey.get(), c_pluginHMACSecretInput, 0, REG_BINARY, reinterpret_cast<PBYTE>(hmacSecret.data()), wil::safe_cast<DWORD>(hmacSecret.size())));
            m_hmacSecret = hmacSecret;
        }
        return S_OK;
    }

    HRESULT PluginRegistrationManager::WriteEncryptedVaultData(std::vector<BYTE> cipherText)
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        wil::unique_hkey hKey;
        RETURN_IF_WIN32_ERROR(RegCreateKeyEx(HKEY_CURRENT_USER, c_pluginRegistryPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr));
        RETURN_IF_WIN32_ERROR(RegSetValueEx(hKey.get(), c_pluginEncryptedVaultData, 0, REG_BINARY, reinterpret_cast<PBYTE>(cipherText.data()), wil::safe_cast<DWORD>(cipherText.size())));
        return S_OK;
    }

    HRESULT PluginRegistrationManager::ReadEncryptedVaultData(std::vector<BYTE>& cipherText)
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationConfigMutex);
        wil::unique_hkey hKey;
        auto opt = wil::reg::try_get_value_binary(HKEY_CURRENT_USER, c_pluginRegistryPath, c_pluginEncryptedVaultData, REG_BINARY);
        if (!opt)
        {
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
        cipherText = opt.value();
        return S_OK;
    }
}
