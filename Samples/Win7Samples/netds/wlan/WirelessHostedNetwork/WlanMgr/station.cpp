// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

// CWlanStation
CWlanStation::CWlanStation(
    const CWlanStation& Other
    )
{
    // copy MAC address
    memcpy(m_MacAddress, Other.m_MacAddress, sizeof(DOT11_MAC_ADDRESS));
}

CWlanStation::CWlanStation(
    const WLAN_HOSTED_NETWORK_PEER_STATE& StationState
    )
{
    // copy MAC address
    memcpy(m_MacAddress, StationState.PeerMacAddress, sizeof(DOT11_MAC_ADDRESS));
}

CWlanStation::~CWlanStation()
{
}

BOOL 
CWlanStation::operator==(
    const CWlanStation& Other
    )
{
    // only match the MAC address
    return *this == Other.m_MacAddress;
}

BOOL 
CWlanStation::operator==(
    const DOT11_MAC_ADDRESS MacAddress
    )
{
    // only match MAC address
    return memcmp(m_MacAddress, MacAddress, sizeof(DOT11_MAC_ADDRESS)) == 0;
}

VOID 
CWlanStation::GetMacAddress(
    DOT11_MAC_ADDRESS& MacAddress
    ) const
{
    // copy MAC address
    memcpy(MacAddress, m_MacAddress, sizeof(DOT11_MAC_ADDRESS));
}
