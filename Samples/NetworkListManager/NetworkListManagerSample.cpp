 // THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#include "stdafx.h"

#pragma region COM helpers
using unique_connectionpoint_token = wil::unique_com_token<IConnectionPoint, DWORD, decltype(&IConnectionPoint::Unadvise), &IConnectionPoint::Unadvise>;

unique_connectionpoint_token FindConnectionPointAndAdvise(REFIID itf, IUnknown* source, IUnknown* sink)
{
    wil::com_ptr<IConnectionPointContainer> container = wil::com_query<IConnectionPointContainer>(source);
    wil::com_ptr<IConnectionPoint> connectionPoint;
    THROW_IF_FAILED(container->FindConnectionPoint(itf, connectionPoint.put()));

    unique_connectionpoint_token token{ connectionPoint.get() };
    THROW_IF_FAILED(connectionPoint->Advise(sink, token.put()));
    return token;
}

// typename T is the connection point interface we are connecting to.
template <typename T>
unique_connectionpoint_token FindConnectionPointAndAdvise(IUnknown* source, IUnknown* sink)
{
    return FindConnectionPointAndAdvise(__uuidof(T), source, sink);
}
#pragma endregion

#pragma region QueryCurrentNetworkConnectivitySample

void QueryCurrentNetworkConnectivitySample(bool optedIn)
{
    std::wcout << L"Querying current network connectivity." << std::endl;
    if (optedIn)
    {
        std::wcout << L"User has opted into possible network usage charges." << std::endl;
    }
    else
    {
        std::wcout << L"User has not opted into possible network usage charges." << std::endl;
    }

    const wil::com_ptr<INetworkListManager> networkListManager = wil::CoCreateInstance<NetworkListManager, INetworkListManager>();
    // Checks machine level connectivity via ipv4 or ipv6 or both.
    NLM_CONNECTIVITY connectivity{ NLM_CONNECTIVITY_DISCONNECTED };
    THROW_IF_FAILED(networkListManager->GetConnectivity(&connectivity));

    Utility::EvaluateAndReportConnectivity(optedIn, connectivity, networkListManager.get());
}

#pragma endregion

#pragma region QueryCurrentNetworkCostSample

void QueryCurrentNetworkCostSample()
{
    std::wcout << "Querying current network cost information." << std::endl;

    // Use INetworkCostManager to query machine-wide cost associated with
    // a network connection used for machine-wide Internet connectivity.
    const wil::com_ptr<INetworkCostManager> networkCostManager = wil::CoCreateInstance<NetworkListManager, INetworkCostManager>();

    DWORD cost{ 0 };
    THROW_IF_FAILED(networkCostManager->GetCost(&cost, nullptr));
    const auto connectionCost = static_cast<NLM_CONNECTION_COST>(cost);

    Utility::EvaluateAndReportConnectionCost(connectionCost);
}

#pragma endregion

#pragma region ListenToNetworkConnectivityChangesSample

class NetworkConnectivityListener final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, INetworkListManagerEvents>
{
public:
    NetworkConnectivityListener(bool optedIn, INetworkListManager* networkListManager)
        : m_optedIn(optedIn), m_networkListManager(networkListManager) {}

    NetworkConnectivityListener(const NetworkConnectivityListener&) = delete;
    NetworkConnectivityListener& operator=(const NetworkConnectivityListener&) = delete;

    IFACEMETHODIMP ConnectivityChanged(NLM_CONNECTIVITY connectivity) noexcept override try
    {
        std::wcout << L"INetworkListManagerEvents::ConnectivityChanged" << std::endl;
        Utility::EvaluateAndReportConnectivity(m_optedIn, connectivity, m_networkListManager.get());
        return S_OK;
    }
    CATCH_RETURN();

private:
    wil::com_ptr<INetworkListManager> m_networkListManager;
    bool m_optedIn;
};

void ListenToNetworkConnectivityChangesSample(bool optedIn)
{
    std::wcout << "Listening to network connectivity changes." << std::endl;
    if (optedIn)
    {
        std::wcout << L"User has opted into possible network usage charges." << std::endl;
    }
    else
    {
        std::wcout << L"User has not opted into possible network usage charges." << std::endl;
    }

    const wil::com_ptr<INetworkListManager> networkListManager = wil::CoCreateInstance<NetworkListManager, INetworkListManager>();

    unique_connectionpoint_token token = FindConnectionPointAndAdvise<INetworkListManagerEvents>(
        networkListManager.get(),
        Microsoft::WRL::Make<NetworkConnectivityListener>(optedIn, networkListManager.get()).Get());

    std::wcout << "Press Enter to stop." << std::endl;
    static_cast<void>(getchar());
}

#pragma endregion

#pragma region ListenToNetworkCostChangesSample

class NetworkCostListener final
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, INetworkCostManagerEvents>
{
public:
    NetworkCostListener(INetworkCostManager* networkCostManager) : m_networkCostManager(networkCostManager) {}

    NetworkCostListener(const NetworkCostListener&) = delete;
    NetworkCostListener& operator=(const NetworkCostListener&) = delete;

    IFACEMETHODIMP CostChanged(DWORD cost, __RPC__in_opt NLM_SOCKADDR* /*pDestAddr*/) noexcept override try
    {
        const auto connectionCost = static_cast<NLM_CONNECTION_COST>(cost);
        std::wcout << L"INetworkCostManagerEvents::CostChanged" << std::endl;

        Utility::EvaluateAndReportConnectionCost(connectionCost);

        return S_OK;
    }
    CATCH_RETURN();

    IFACEMETHODIMP DataPlanStatusChanged(__RPC__in_opt NLM_SOCKADDR* /*pDestAddr*/) noexcept override
    {
        // This event is not used by the sample.
        return S_OK;
    }

private:
    wil::com_ptr<INetworkCostManager> m_networkCostManager;
};
void ListenToNetworkCostChangesSample()
{
    std::wcout << "Listening to network cost changes." << std::endl;

    const wil::com_ptr<INetworkCostManager> networkCostManager = wil::CoCreateInstance<NetworkListManager, INetworkCostManager>();

    unique_connectionpoint_token token = FindConnectionPointAndAdvise<INetworkCostManagerEvents>(
        networkCostManager.get(),
        Microsoft::WRL::Make<NetworkCostListener>(networkCostManager.get()).Get());

    std::wcout << "Press Enter to stop." << std::endl;
    static_cast<void>(getchar());
}

#pragma endregion

int __cdecl wmain(int argc, wchar_t** argv)
try
{
    auto coinit = wil::CoInitializeEx_failfast(COINIT_MULTITHREADED);
    int scenario = argc == 2 ? _wtoi(argv[1]) : 0;
    switch (scenario)
    {
    case 1:
    case 2:
        QueryCurrentNetworkConnectivitySample(scenario == 2);
        break;

    case 3:
        QueryCurrentNetworkCostSample();
        break;

    case 4:
    case 5:
        ListenToNetworkConnectivityChangesSample(scenario == 5);
        break;

    case 6:
        ListenToNetworkCostChangesSample();
        break;

    default:
        std::wcout << "NetworkListManager.exe sample demonstrates how to use network list manager APIs." << std::endl
            << "  [parameter]" << std::endl
            << "    1: Query current network connectivity: User has not opted into possible network usage charges." << std::endl
            << "    2: Query current network connectivity: User has opted into possible network usage charges." << std::endl
            << "    3: Query current network cost information." << std::endl
            << "    4: Listen to network connectivity changes: User has not opted into possible network usage charges." << std::endl
            << "    5: Listen to network connectivity changes: User has opted into possible network usage charges." << std::endl
            << "    6: Listen to network cost changes." << std::endl;
        break;
    }
    return 0;
}
CATCH_RETURN();
