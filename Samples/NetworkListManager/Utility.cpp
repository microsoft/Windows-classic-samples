// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#include "stdafx.h"
#pragma comment(lib, "winhttp.lib")

namespace Utility
{
    enum class CostGuidance
    {
        Normal,
        OptIn,
        Conservative
    };

    CostGuidance GetNetworkCostGuidance(NLM_CONNECTION_COST cost)
    {
        if (WI_IsAnyFlagSet(cost, NLM_CONNECTION_COST_ROAMING | NLM_CONNECTION_COST_OVERDATALIMIT))
        {
            if (WI_IsFlagSet(cost, NLM_CONNECTION_COST_ROAMING))
            {
                std::wcout << "Connection is roaming; using the connection may result in additional charge." << std::endl;
            }
            else
            {
                std::wcout << "Connection has exceeded the usage cap limit." << std::endl;
            }
            return CostGuidance::OptIn;
        }
        else if (WI_IsAnyFlagSet(cost, NLM_CONNECTION_COST_FIXED | NLM_CONNECTION_COST_VARIABLE))
        {
            if (WI_IsFlagSet(cost, NLM_CONNECTION_COST_FIXED))
            {
                std::wcout << "Connection has limited allowed usage." << std::endl;
            }
            else
            {
                std::wcout << "Connection is charged based on usage." << std::endl;
            }
            return CostGuidance::Conservative;
        }
        else
        {
            if (WI_IsFlagSet(cost, NLM_CONNECTION_COST_UNRESTRICTED))
            {
                std::wcout << "Connection cost is unrestricted." << std::endl;
            }
            else
            {
                std::wcout << "Connection cost is unknown." << std::endl;
            }
            return CostGuidance::Normal;
        }
    }

    bool ShouldAttemptToConnectToInternet(NLM_CONNECTIVITY connectivity, INetworkListManager* networkListManager)
    {
        // check internet connectivity
        if (WI_IsAnyFlagSet(connectivity, NLM_CONNECTIVITY_IPV4_INTERNET | NLM_CONNECTIVITY_IPV6_INTERNET))
        {
            std::wcout << "Machine has internet connectivity." << std::endl;
            return true;
        }
        else if (WI_IsAnyFlagSet(connectivity, NLM_CONNECTIVITY_IPV4_LOCALNETWORK | NLM_CONNECTIVITY_IPV6_LOCALNETWORK))
        {
            // we are local connected, check if we're behind a captive portal before attempting to connect to the Internet.
            //
            // note: being behind a captive portal means connectivity is local and there is at least one interface(network)
            // behind a captive portal.

            bool localConnectedBehindCaptivePortal = false;
            wil::com_ptr<IEnumNetworks> enumConnectedNetworks;
            THROW_IF_FAILED(
                networkListManager->GetNetworks(
                    NLM_ENUM_NETWORK_CONNECTED,
                    enumConnectedNetworks.put()));

            // Enumeration returns S_FALSE when there are no more items.
            wil::com_ptr<INetwork> networkConnection;
            while (THROW_IF_FAILED(enumConnectedNetworks->Next(1, networkConnection.put(), nullptr)) == S_OK)
            {
                wil::com_ptr<IPropertyBag> networkProperties = networkConnection.query<IPropertyBag>();

                // these might fail if there's no value
                wil::unique_variant variantInternetConnectivityV4;
                networkProperties->Read(NA_InternetConnectivityV4, variantInternetConnectivityV4.addressof(), nullptr);
                wil::unique_variant variantInternetConnectivityV6;
                networkProperties->Read(NA_InternetConnectivityV6, variantInternetConnectivityV6.addressof(), nullptr);

                // read the VT_UI4 from the VARIANT and cast it to a NLM_INTERNET_CONNECTIVITY
                // If there is no value, then assume no special treatment.
                NLM_INTERNET_CONNECTIVITY v4Connectivity = static_cast<NLM_INTERNET_CONNECTIVITY>(variantInternetConnectivityV6.vt == VT_UI4 ? variantInternetConnectivityV4.ulVal : 0);
                NLM_INTERNET_CONNECTIVITY v6Connectivity = static_cast<NLM_INTERNET_CONNECTIVITY>(variantInternetConnectivityV6.vt == VT_UI4 ? variantInternetConnectivityV6.ulVal : 0);

                if (WI_IsFlagSet(v4Connectivity, NLM_INTERNET_CONNECTIVITY_WEBHIJACK) || WI_IsFlagSet(v6Connectivity, NLM_INTERNET_CONNECTIVITY_WEBHIJACK))
                {
                    // at least one connected interface is behind a captive portal
                    // we should assume that the device is behind it
                    localConnectedBehindCaptivePortal = true;
                }
            }

            if (!localConnectedBehindCaptivePortal)
            {
                std::wcout << "Machine has local connectivity and not behind a captive portal." << std::endl;
                return true;
            }
            else
            {
                std::wcout << "Machine is behind a captive portal." << std::endl;
            }
        }
        else
        {
            std::wcout << "Machine is not connected." << std::endl;
        }
        return false;
    }

    void SendHttpGetRequest()
    {
        wil::unique_winhttp_hinternet session(
            WinHttpOpen(L"NetworkListManagerSample.exe", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));

        if (!session)
        {
            std::wcout << "WinHttpOpen failed to get a session handle with error code " << GetLastError() << std::endl;
            return;
        }

        wil::unique_winhttp_hinternet connect(WinHttpConnect(session.get(), L"www.msftconnecttest.com", INTERNET_DEFAULT_HTTP_PORT, 0));
        if (!connect)
        {
            std::wcout << "WinHttpConnect failed to get a connect handle with error code " << GetLastError() << std::endl;
            return;
        }

        wil::unique_winhttp_hinternet request(WinHttpOpenRequest(
            connect.get(), L"GET", L"/connecttest.txt", nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0));
        if (!request)
        {
            std::wcout << "WinHttpOpenRequest failed with error code " << GetLastError() << std::endl;
            return;
        }

        if (!WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        {
            std::wcout << "WinHttpSendRequest failed with error code " << GetLastError() << std::endl;
            return;
        }

        if (!WinHttpReceiveResponse(request.get(), nullptr))
        {
            std::wcout << "WinHttpReceiveResponse failed with error code " << GetLastError() << std::endl;
            return;
        }

        DWORD statusCode{0};
        DWORD headerBytes = sizeof(statusCode);
        if (WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &statusCode, &headerBytes, WINHTTP_NO_HEADER_INDEX))
        {
            if (statusCode >= 200 && statusCode < 300)
            {
                std::wcout << "Http request succeeded with status code " << statusCode << std::endl;

                DWORD bytesRead{0};
                if (WinHttpQueryDataAvailable(request.get(), &bytesRead) && bytesRead > 0)
                {
                    std::unique_ptr<BYTE[]> readBuffer = std::make_unique<BYTE[]>(bytesRead);
                    if (WinHttpReadData(request.get(), readBuffer.get(), bytesRead, &bytesRead))
                    {
                        std::wcout << "Received " << bytesRead << " bytes in response." << std::endl;
                        return;
                    }
                    else
                    {
                        std::wcout << "WinHttpReadData failed with error code " << GetLastError() << std::endl;
                        return;
                    }
                }
                else
                {
                    std::wcout << "WinHttpQueryDataAvailable failed with error code " << GetLastError() << std::endl;
                    return;
                }
            }
            else
            {
                std::wcout << "Http request completed with status code " << statusCode << std::endl;
            }
        }
        else
        {
            std::wcout << "WinHttpQueryHeaders failed with error code " << GetLastError() << std::endl;
        }
    }

    void EvaluateCostAndConnect(bool optedIn, INetworkListManager* networkListManager)
    {
        wil::com_ptr<INetworkCostManager> netCostManager = wil::com_query<INetworkCostManager>(networkListManager);

        DWORD cost{0};
        THROW_IF_FAILED(netCostManager->GetCost(&cost, nullptr));
        const auto nlmConnectionCost = static_cast<NLM_CONNECTION_COST>(cost);
        Utility::CostGuidance costGuidance = Utility::GetNetworkCostGuidance(nlmConnectionCost);

        switch (costGuidance)
        {
        case Utility::CostGuidance::OptIn:
        {
            std::cout << "Network access cost is significantly higher." << std::endl;
            if (optedIn)
            {
                std::cout << "User has opted into network usage while roaming. Connecting." << std::endl;
                SendHttpGetRequest();
            }
            else
            {
                std::cout << "User has not opted into network usage while roaming. Not connecting." << std::endl;
            }
            break;
        }
        case Utility::CostGuidance::Conservative:
        {
            std::wcout << "Attempt connecting to the Internet for critical requests." << std::endl;
            SendHttpGetRequest();
            break;
        }
        case Utility::CostGuidance::Normal:
        default:
            std::wcout << "Attempt connecting to the Internet." << std::endl;
            SendHttpGetRequest();
            break;
        }
    }

    void EvaluateAndReportConnectivity(bool optedIn, NLM_CONNECTIVITY connectivity, INetworkListManager* networkListManager)
    {
        if (Utility::ShouldAttemptToConnectToInternet(connectivity, networkListManager))
        {
            Utility::EvaluateCostAndConnect(optedIn, networkListManager);
        }
        else
        {
            std::wcout << "Not attempting to connect to the Internet." << std::endl;
        }
    }

    void EvaluateAndReportConnectionCost(NLM_CONNECTION_COST connectionCost)
    {
        const auto costGuidance = Utility::GetNetworkCostGuidance(connectionCost);

        switch (costGuidance)
        {
        case Utility::CostGuidance::OptIn:
            // In opt-in scenarios, apps handle cases where the network access cost is significantly higher than the plan cost.
            // For example, when a user is roaming, a mobile carrier may charge a higher rate data usage.
            std::wcout << "Apps should implement opt-in behavior." << std::endl;
            break;
        case Utility::CostGuidance::Conservative:
            // In conservative scenarios, apps implement restrictions for optimizing network usage to handle transfers over metered networks.
            std::wcout << "Apps should implement conservative behavior." << std::endl;
            break;
        case Utility::CostGuidance::Normal:
        default:
            // In normal scenarios, apps do not implement restrictions. Apps treat the connection as unlimited in cost.
            std::wcout << "Apps should implement normal behavior." << std::endl;
            break;
        }
    }
}

