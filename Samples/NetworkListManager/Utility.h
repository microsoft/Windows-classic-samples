// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once
namespace Utility
{
    void EvaluateAndReportConnectivity(bool optedIn, NLM_CONNECTIVITY connectivity, INetworkListManager* networkListManager);
    void EvaluateAndReportConnectionCost(NLM_CONNECTION_COST connectionCost);
}
