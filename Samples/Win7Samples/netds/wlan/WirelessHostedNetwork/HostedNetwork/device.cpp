// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "resource.h"

CImageList CWlanDevice::m_ImageList;

int CWlanDevice::m_DeviceImageIndex[device_type_invalid];

DEVICE_TYPE MatchPredefinedDeviceType(DOT11_MAC_ADDRESS& MacAddress)
{
    //to be added: method of detecting the device type.
    return device_type_default;
}

//
// Static functions
//
VOID
CWlanDevice::InitDeviceImageList(CWinApp * App)
{
    //
    // Add device images
    //
    m_ImageList.Create(48, 48, ILC_COLOR32, 5, 2);
    // set background color
    m_ImageList.SetBkColor(RGB(255,255,255));

    // default device
    m_DeviceImageIndex[device_type_default] = m_ImageList.Add(App->LoadIcon(IDR_DEFAULTDEVICE));
    // camera
    m_DeviceImageIndex[device_type_camera] = m_ImageList.Add(App->LoadIcon(IDR_CAMERA));
    // printer
    m_DeviceImageIndex[device_type_printer] = m_ImageList.Add(App->LoadIcon(IDR_PRINTER));
    // computer
    m_DeviceImageIndex[device_type_computer] = m_ImageList.Add(App->LoadIcon(IDR_COMPUTER));
    // telephone
    m_DeviceImageIndex[device_type_telephone] = m_ImageList.Add(App->LoadIconW(IDR_TELEPHONE));
    // zune device
    m_DeviceImageIndex[device_type_zune] = m_ImageList.Add(App->LoadIconW(IDR_ZUNEDEVICE));
}

CImageList *
CWlanDevice::GetDeviceImageList()
{
    return &m_ImageList;
}

// CWlanDevice
CWlanDevice::CWlanDevice(
    DOT11_MAC_ADDRESS& MacAddress
    )
{
    // copy MAC address
    memcpy(m_MacAddress, MacAddress, sizeof(DOT11_MAC_ADDRESS));

    // By default, the friendly name is the MAC address
    GetDisplayMacAddress(m_FriendlyName);

    m_Type = MatchPredefinedDeviceType(MacAddress);
}

CWlanDevice::~CWlanDevice()
{
}

BOOL 
CWlanDevice::operator==(
    const CWlanDevice& Other
    )
{
    // only match the MAC address
    return *this == Other.m_MacAddress;
}

BOOL 
CWlanDevice::operator==(
    const DOT11_MAC_ADDRESS MacAddress
    )
{
    // only match MAC address
    return memcmp(m_MacAddress, MacAddress, sizeof(DOT11_MAC_ADDRESS)) == 0;
}

VOID 
CWlanDevice::GetDisplayMacAddress(
    CAtlString& strMacAddress
    )
{
    WCHAR strDisplayName[WLAN_MAX_NAME_LENGTH] = {0}; 
    DWORD szDisplayName = WLAN_MAX_NAME_LENGTH;

    StringCchPrintf(
        strDisplayName,
        szDisplayName,
        L"%02X-%02X-%02X-%02X-%02X-%02X",
        m_MacAddress[0],
        m_MacAddress[1],
        m_MacAddress[2],
        m_MacAddress[3],
        m_MacAddress[4],
        m_MacAddress[5]
        );

    strMacAddress = strDisplayName;
}

int
CWlanDevice::GetImageIndex()
{
    return m_DeviceImageIndex[m_Type];   // default
}