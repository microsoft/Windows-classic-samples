// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef _DEVICE_H_
#define _DEVICE_H_

typedef enum DEVICE_TYPE
{
    device_type_default = 0,
    device_type_camera,
    device_type_printer,
    device_type_computer,
    device_type_telephone,
    device_type_zune,
    device_type_invalid
} DEVICE_TYPE;

class CWlanDevice : public CRefObject
{
private:
    DOT11_MAC_ADDRESS m_MacAddress;
    
    // Don't allow to create an empty CWlanDevice object
    CWlanDevice() {};

    // Friendly hame
    CAtlString m_FriendlyName;

    DEVICE_TYPE m_Type;

    static CImageList m_ImageList;     // Images for the devices

    // Image index
    static int m_DeviceImageIndex[device_type_invalid];
public:
    CWlanDevice(DOT11_MAC_ADDRESS&);
    ~CWlanDevice();

    BOOL operator==(const CWlanDevice &);
    BOOL operator==(const DOT11_MAC_ADDRESS);

    VOID GetFriendlyName(CAtlString& strName) {strName = m_FriendlyName;};

    VOID SetFriendlyName(__in LPWSTR strName) {m_FriendlyName = strName;};

    VOID SetType(DEVICE_TYPE Type) {m_Type = Type;};

    VOID GetDisplayMacAddress(CAtlString&);

    VOID GetMacAddress(DOT11_MAC_ADDRESS & MacAddress) {memcpy(MacAddress, m_MacAddress, sizeof(DOT11_MAC_ADDRESS));};

    int GetImageIndex();

    static VOID InitDeviceImageList(CWinApp * App);

    static CImageList* GetDeviceImageList();
};

#endif  // _DEVICE_H_
