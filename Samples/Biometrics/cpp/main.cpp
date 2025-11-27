#pragma once
#include <windows.h>
#include <winbio.h>
#include <wil/resource.h>
#include <wil/token_helpers.h>
#include <wil/win32_helpers.h>
#include <algorithm>
#include <stdio.h>

#pragma region Checking for Windows SDK version 10.0.26100.7175 or higher
template<typename, typename = void> constexpr bool is_type_complete_v = false;
template<typename T> constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;

static_assert(is_type_complete_v<struct _WINBIO_CONNECTED_SENSOR>, "This sample requires Windows SDK version 10.0.26100.7175 or higher");
#pragma endregion

void PrintHresultDetails(HRESULT hr)
{
    wchar_t errorText[512];
    errorText[0] = L'\0';
    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        errorText,
        ARRAYSIZE(errorText),
        nullptr);
    wprintf(L"HRESULT = 0x%08x: %ls", hr, errorText);
}

struct WinBioApi
{
    WinBioApi() = default;
    ~WinBioApi() = default;

    wil::unique_hmodule hModule;
    decltype(&WinBioIsDeviceEnhancedSignInSecurityCapable) WinBioIsDeviceEnhancedSignInSecurityCapable = nullptr;
    decltype(&WinBioGetEnhancedSignInSecurityStateSource) WinBioGetEnhancedSignInSecurityStateSource = nullptr;
    decltype(&WinBioIsDeviceEnhancedSignInSecurityEnabled) WinBioIsDeviceEnhancedSignInSecurityEnabled = nullptr;
    decltype(&WinBioAreEnhancedSignInSecurityRequirementsMet) WinBioAreEnhancedSignInSecurityRequirementsMet = nullptr;
    decltype(&WinBioGetConnectedSensors) WinBioGetConnectedSensors = nullptr;
    decltype(&WinBioGetEnhancedSignInSecurityEnrolledFactors) WinBioGetEnhancedSignInSecurityEnrolledFactors = nullptr;

    HRESULT Load()
    {
        hModule.reset(LoadLibraryExW(L"ext-ms-win-biometrics-winbio-core-l1.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32));
        RETURN_LAST_ERROR_IF_NULL_MSG(hModule, "Unable to load ext-ms-win-biometrics-winbio-core-l1.dll");

        WinBioIsDeviceEnhancedSignInSecurityCapable = GetProcAddressByFunctionDeclaration(hModule.get(), WinBioIsDeviceEnhancedSignInSecurityCapable);
        RETURN_LAST_ERROR_IF_NULL_MSG(WinBioIsDeviceEnhancedSignInSecurityCapable, "Unable to locate WinBioIsDeviceEnhancedSignInSecurityCapable.");

        WinBioGetEnhancedSignInSecurityStateSource = GetProcAddressByFunctionDeclaration(hModule.get(), WinBioGetEnhancedSignInSecurityStateSource);
        RETURN_LAST_ERROR_IF_NULL_MSG(WinBioGetEnhancedSignInSecurityStateSource, "Unable to locate WinBioGetEnhancedSignInSecurityStateSource.");

        WinBioIsDeviceEnhancedSignInSecurityEnabled = GetProcAddressByFunctionDeclaration(hModule.get(), WinBioIsDeviceEnhancedSignInSecurityEnabled);
        RETURN_LAST_ERROR_IF_NULL_MSG(WinBioIsDeviceEnhancedSignInSecurityEnabled, "Unable to locate WinBioIsDeviceEnhancedSignInSecurityEnabled.");

        WinBioAreEnhancedSignInSecurityRequirementsMet = GetProcAddressByFunctionDeclaration(hModule.get(), WinBioAreEnhancedSignInSecurityRequirementsMet);
        RETURN_LAST_ERROR_IF_NULL_MSG(WinBioAreEnhancedSignInSecurityRequirementsMet, "Unable to locate WinBioAreEnhancedSignInSecurityRequirementsMet.");

        WinBioGetConnectedSensors = GetProcAddressByFunctionDeclaration(hModule.get(), WinBioGetConnectedSensors);
        RETURN_LAST_ERROR_IF_NULL_MSG(WinBioGetConnectedSensors, "Unable to locate WinBioGetConnectedSensors.");

        WinBioGetEnhancedSignInSecurityEnrolledFactors = GetProcAddressByFunctionDeclaration(hModule.get(), WinBioGetEnhancedSignInSecurityEnrolledFactors);
        RETURN_LAST_ERROR_IF_NULL_MSG(WinBioGetEnhancedSignInSecurityEnrolledFactors, "Unable to locate WinBioGetEnhancedSignInSecurityEnrolledFactors.");

        return S_OK;
    }
};

WinBioApi g_winBio;

// Custom RAII type that frees the memory with WinBioFree.
template<typename T>
using unique_winbio_array_ptr = wil::unique_array_ptr<T, wil::function_deleter<decltype(&WinBioFree), WinBioFree>>;

HRESULT CheckEssCapable(BOOL* isCapable)
{
    // Assume not capable if anything goes wrong.
    *isCapable = FALSE;
    RETURN_IF_FAILED_MSG(g_winBio.WinBioIsDeviceEnhancedSignInSecurityCapable(isCapable),
        "Unable to determine whether system is ESS capable.");

    if (isCapable)
    {
        printf("The system is ESS capable.\n");
    }
    else
    {
        printf("The system is not ESS capable.\n");
    }
    return S_OK;
}

// Reports the source of the ESS state. A program can use this to guide its recommendations.
// For example, if ESS is controlled by admin policy, then the program can suggest that the
// user contact their administrator.

HRESULT CheckEssStateSource(WINBIO_POLICY_SOURCE* source)
{
    // Assume unknown if anything goes wrong.
    *source = WINBIO_POLICY_UNKNOWN;

    RETURN_IF_FAILED_MSG(g_winBio.WinBioGetEnhancedSignInSecurityStateSource(source),
        "Unable to determine the source of the ESS state.");

    switch (*source)
    {
    default:
    case WINBIO_POLICY_UNKNOWN:
        printf("ESS is enabled/disabled for unknown reason.\n");
        break;
    case WINBIO_POLICY_DEFAULT:
        printf("System configuration controls whether ESS is enabled.\n");
        break;
    case WINBIO_POLICY_ADMIN:
        printf("Admin policy controls whether ESS is enabled.\n");
        break;
    case WINBIO_POLICY_LOCAL:
        printf("User preference controls whether ESS is enabled.\n");
        break;
    }

    return S_OK;
}

HRESULT CheckEssEnabled(BOOL* isEnabled)
{
    // Assume not enabled if anything goes wrong.
    *isEnabled = FALSE;
    RETURN_IF_FAILED_MSG(g_winBio.WinBioIsDeviceEnhancedSignInSecurityEnabled(isEnabled),
        "Unable to determine whether ESS is enabled");
    if (*isEnabled)
    {
        printf("ESS is enabled.\n");
    }
    else
    {
        printf("ESS is not enabled.\n");
    }
    return S_OK;
}

HRESULT CheckEssRequirementsMet(BOOL* requirementsMet)
{
    // Assume requirements are not met if anything goes wrong.
    *requirementsMet = FALSE;
    RETURN_IF_FAILED_MSG(g_winBio.WinBioAreEnhancedSignInSecurityRequirementsMet(requirementsMet),
        "Unable to determine whether ESS requirements are met.");

    if (*requirementsMet)
    {
        printf("ESS set up requirements are satisfied.\n");
    }
    else
    {
        printf("ESS set up requirements are not satisfied. User needs to enable the Enhanced sign-in security setting under Settings > Accounts > Sign-in options > Additional settings to set up the device for ESS.\n");
    }
    return S_OK;
}

// Reports on each of the sensors connected to the system and whether they are ESS-capable.
// A program can use this to guide its recommendations. For example, if the user has a fingerprint
// sensor that is ESS-capable, and a face sensor that is not, then the program can suggest that they
// enroll with the fingerprint sensor.

HRESULT CheckEssCapableSensors(BOOL* hasCapableSensors)
{
    // Assume no ESS-capable sensors if anything goes wrong.
    *hasCapableSensors = FALSE;

    // Get a list of all the connected sensors.
    unique_winbio_array_ptr<WINBIO_CONNECTED_SENSOR> sensors;
    RETURN_IF_FAILED_MSG(g_winBio.WinBioGetConnectedSensors(sensors.size_address(), sensors.put()),
        "Failed to get connected sensors.");

    printf("Found %zu sensors.\n", sensors.size());

    // Report whether they are ESS-capable and what kind of biometrics they use.
    for (const auto& sensor : sensors)
    {
        printf("Found sensor: ");
        if (sensor.isEnhancedSignInSecurityCapable)
        {
            *hasCapableSensors = TRUE;
            printf("Is ESS capable = Yes, ");
        }
        else
        {
            printf("Is ESS capable =  No, ");
        }

        switch (sensor.biometricType)
        {
        case WINBIO_TYPE_FINGERPRINT:
            printf("Fingerprint\n");
            break;
        case WINBIO_TYPE_FACIAL_FEATURES:
            printf("Face\n");
            break;
        default:
            printf("Neither fingerprint nor face.\n");
            break;
        }
    }

    return S_OK;
}

HRESULT CheckEssEnrollment(bool* isEnrolled)
{
    // Assume not enrolled if anything goes wrong.
    *isEnrolled = false;

    wil::unique_tokeninfo_ptr<TOKEN_USER> userInfo;

    // Get SID of the current user
    RETURN_IF_FAILED_MSG(wil::get_token_information_nothrow(userInfo), "Unable to identify the current user.");

    // Get the enrolled factors for the user.
    WINBIO_IDENTITY identity = {};
    identity.Type = WINBIO_ID_TYPE_SID;
    identity.Value.AccountSid.Size = GetLengthSid(userInfo->User.Sid);
    RETURN_IF_WIN32_BOOL_FALSE_MSG(
        CopySid(sizeof(identity.Value.AccountSid.Data), identity.Value.AccountSid.Data, userInfo->User.Sid),
        "User SID too large.");

    WINBIO_BIOMETRIC_TYPE enrolledFactors = WINBIO_NO_TYPE_AVAILABLE;
    RETURN_IF_FAILED_MSG(g_winBio.WinBioGetEnhancedSignInSecurityEnrolledFactors(&identity, &enrolledFactors), "Failed to get enrolled factors.");

    // If any ESS factor is enrolled, then the user is enrolled with ESS.
    *isEnrolled = enrolledFactors != WINBIO_NO_TYPE_AVAILABLE;

    if (*isEnrolled)
    {
        printf("User is enrolled with ESS.\n");
    }
    else
    {
        printf("User is not enrolled with ESS.\n");
    }

    return S_OK;
}

// This function is called when a WIL failure occurs.
// It logs the failure to the console for diagnostic purposes.

void LogFailure(wil::FailureInfo const& info) noexcept
{
    if (info.pszMessage != nullptr)
    {
        printf("Error: %ls\n", info.pszMessage);
    }
    PrintHresultDetails(info.hr);
}

HRESULT MainProgram()
{
    HRESULT hr;

    // Load WinBio API functions. If we can't do this, then there's no point in continuing.
    RETURN_IF_FAILED_MSG(g_winBio.Load(), "winbio functions could not be loaded");

    // Most programs are interested only in whether the user is enrolled in Windows Hello with ESS.
    bool isEnrolled = false;
    hr = CheckEssEnrollment(&isEnrolled);

    // If the user is enrolled, then most programs don't need additional diagnostics regarding ESS.
    if (isEnrolled)
    {
        return S_OK;
    }

    // This sample shows how a program can learn more about the ESS capabilities of the system
    // if the user is not enrolled with ESS.

    // First, check whether the system is ESS capable.
    BOOL isCapable = FALSE;
    hr = CheckEssCapable(&isCapable);

    if (!isCapable)
    {
        // If the system is not ESS-capable, then no further diagnostics are needed.
        return S_OK;
    }

    // Check the source of the ESS state.
    WINBIO_POLICY_SOURCE source = WINBIO_POLICY_UNKNOWN;
    hr = CheckEssStateSource(&source);

    // Check whether ESS is enabled.
    BOOL isEnabled = FALSE;
    hr = CheckEssEnabled(&isEnabled);

    // Check if ESS conditions are met on this device.
    BOOL requirementsMet = false;
    hr = CheckEssRequirementsMet(&requirementsMet);

    // Check the sensors and whether they are ESS-capable.
    BOOL hasCapableSensors = FALSE;
    hr = CheckEssCapableSensors(&hasCapableSensors);

    return S_OK;
}

int main()
{
    wil::SetResultLoggingCallback(LogFailure);
    return SUCCEEDED(MainProgram()) ? EXIT_SUCCESS : EXIT_FAILURE;
}
