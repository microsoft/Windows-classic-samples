// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      This module declares message structures for a 
//      simple discovery protocol.

#pragma once

enum TMessageType
{
    Hello,
    Bye,
    Query,
    Reply
};

struct TDeviceInfo
{
    WCHAR szDeviceCategory[129];
    WCHAR szFriendlyName[81];
    WCHAR szManufacturer[81];
    WCHAR szManufacturerUrl[129];
    WCHAR szModelName[81];
    WCHAR szModelNumber[31];
    WCHAR szModelUrl[129];
    WCHAR szUpc[13];
    WCHAR szFirmwareVersion[13];
    WCHAR szSerialNumber[13];
    WCHAR szPresentationUrl[81];
    WCHAR szPnPHardwareId[200];
};

struct TMessage
{
    TMessageType MessageType;
};

struct THelloMessage
{
    TMessageType MessageType;
    GUID DeviceId;
    TDeviceInfo DeviceInfo;
};

struct TByeMessage
{
    TMessageType MessageType;
    GUID DeviceId;
};

struct TQueryMessage
{
    TMessageType MessageType;
    WCHAR szDeviceCategory[31];
};

struct TReplyMessage
{
    TMessageType MessageType;
    GUID DeviceId;
    TDeviceInfo DeviceInfo;
};

// Declare the multicast socket for the discovery protocol.
// These values are chosen arbitarily for this sample.
const WCHAR szMulticastAddress[] = L"FF02::73F7";
const WCHAR szMulticastPort[] = L"45394";
const LONGLONG QueryTimeout= 10000;  // 10 seconds
const ULONG MaxMessageSize = sizeof(THelloMessage);